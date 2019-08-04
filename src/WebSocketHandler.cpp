//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "WebSocketHandler.h"


WebSocketHandler::WebSocketHandler(Sensors &sensors, SpiffsHandler &spiffsHandler,Thermostat &thermostat, Relay &relay) {
    this->spiffsHandler= &spiffsHandler;
    this->relay = &relay;
    this->sensors =&sensors;
    this->thermostat = &thermostat;
    webSocket = new WebSocketsServer(81);    // create a websocket server on port 81
}

WebSocketHandler::~WebSocketHandler() {
    webSocket->close();
    delete  webSocket;
}

void WebSocketHandler::setup() {
    Serial.println("[INFO] WebSocketHandler setup()");
    // TODO load/save config
    //  webSocket->setAuthorization("esp8266", "Led*ZInqsdf9");
    webSocket->begin();                          // start the websocket server
    webSocket->onEvent(std::bind(&WebSocketHandler::onWsEvent, this,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3,
                                 std::placeholders::_4));          // if there's an incomming websocket message, go to function 'webSocketEvent'

    Serial.println("[INF0] WebSocket server started.");
}


void WebSocketHandler::update() {
    webSocket->loop();
}




void WebSocketHandler::onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
    switch (type) {
        case WStype_DISCONNECTED:             // if the websocket is disconnected
            //Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {              // if a new websocket connection is established
            IPAddress ip = webSocket->remoteIP(num);
        }
            break;
        case WStype_TEXT:                     // if new text data is received
            // Serial.printf("[%u] get Text: %s\n", num, payload);

            DynamicJsonDocument root(1024);

            auto error = deserializeJson(root, &payload[0]);

            if (error) {
                Serial.println("parseObject() failed");

            }else
            {
                String target = root["event"];

                if (target.equals("relay")) {

                    if(root.containsKey("state") && root["state"].is<bool>()) {
                        bool state = root["state"];
                        if(state == true){
                            relay->turnOn();
                        }else if(state== false)
                        {
                            relay->turnOff();
                        }
                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }


                }else if(target.equals("restart")) {
                    ESP.restart();
                }else if(target.equals("freeheap")){
                    DynamicJsonDocument freeHeap(1024);

                    freeHeap["event"] = "freeheap";
                    root["freeheap"] =  ESP.getFreeHeap();
                    String retJson;
                    serializeJson(root, retJson);
                    webSocket->sendTXT(num, retJson);
                }else if(target.equals("time")){
                    DynamicJsonDocument freeHeap(1024);

                    freeHeap["event"] = "time";
                    root["currtime"] =  this->thermostat->ntpClient->getFormattedTime();
                    String retJson;
                    serializeJson(root, retJson);
                    webSocket->sendTXT(num, retJson);
                }else if(target.equals("do_wifi_scan")){

                    Serial.println("do_wifi_scan");
                    DynamicJsonDocument root(1024);
                    root["event"] = "wifi_scan";
                    byte n = WiFi.scanNetworks();
                    JsonArray networks = root.createNestedArray("networks");

                    for(int i=0;i<n;i++) {
                        JsonObject network = networks.createNestedObject();
                        network["ssid"] = WiFi.SSID(i);
                        network["encryption"] = WiFi.encryptionType(i);
                        network["rssi"] = WiFi.RSSI(i);
                        network["bssid"] = WiFi.BSSIDstr(i);
                        network["channel"] = WiFi.channel(i);
                        network["isHidden"] = WiFi.isHidden(i);
                    }

                    String retJson;
                    serializeJson(root, retJson);
                    webSocket->sendTXT(num, retJson);
                } else if(target.equals("thermostat_state")){

                    if(root.containsKey("state") && root["state"].is<int>()) {
                        bool state = root["state"];
                        if(state == true){
                            thermostat->turnOn();
                        }else if(state== false)
                        {
                            thermostat->turnOff();
                        }
                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }

                }else if(target.equals("thermostat_manualsetpoint"))
                {

                    if(root.containsKey("temperature") && root["temperature"].is<int>()) {
                        int temperature = root["temperature"];
                        thermostat->setManualSetPoint(temperature);

                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }

                }else if(target.equals("thermostat_schedule")){

                    if (root.containsKey("day") && root.containsKey("schedule" ) && root["day"].is<int>()) {
                        int dow = root["day"];
                        if(dow >=0 && dow <=7){

                            JsonArray scheduleRoot = root["schedule"];

                            if(scheduleRoot.size() < MAXIMUM_SCHEDULE_PER_DAY){

                                int i=0;
                                for (auto row : scheduleRoot) {

                                    if(row.containsKey("s") && row.containsKey("e") &&  row.containsKey("sp")) {

                                        if(row["s"].is<int>()  && row["e"].is<int>() && row["sp"].is<int>())
                                        {

                                            int start = row["s"];
                                            int end = row["e"];
                                            int sp = row["sp"];

                                            if(i < MAXIMUM_SCHEDULE_PER_DAY){

                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].start=start; //0am
                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].end=end; //6am, hours are * 100
                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].setpoint=sp; //10.0*C
                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].active=1;
                                                i++;
                                            }

                                        }else {

                                            Serial.println("[WARNING] start/end/setpoint have to be integer");

                                        }

                                    }else {
                                        Serial.println("[WARNING] start/end/setpoint are mandatory");

                                    }


                                }
                                if(i < MAXIMUM_SCHEDULE_PER_DAY){
                                    thermostat->thermostatSchedule.weekSched[dow].daySched[i].active=0;	//mark the next schedule as inactive
                                }

                                thermostat->saveConfiguration(SCHEDULE_CONFIG);


                            }

                        }


                    }

                } else if(target.equals("thermostat_mode")){

                    if(root.containsKey("mode") && root["mode"].is<int>()) {
                        int mode = root["mode"];
                        thermostat->setMode(mode);
                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }

                }else if(target.equals("get_thermostat_states")){

                    DynamicJsonDocument root(1024); // https://arduinojson.org/v5/assistant/
                    root["event"] = "thermostat_states";

                    JsonArray jsonSensors = root.createNestedArray("sensors");

                    for(int i=0; i< MAX_SENSORS;i++){
                        if(sensors->readSensor(i).active == true){
                            JsonObject sensor = jsonSensors.createNestedObject();
                            sensor["id"] = i;
                            sensor["temp"] = sensors->readSensor(i).temperature;
                            sensor["hum"] = sensors->readSensor(i).humidity;
                            sensor["batt"] = sensors->readSensor(i).battery;
                            sensor["rssi"] = sensors->readSensor(i).rssi;
                            sensor["lastUp"] = sensors->readSensor(i).lastUpdate;
                        }

                    }

                    root["relayState"] = relay->getState();
                    root["state"] = thermostat->getState();
                    root["manualsetpoint"] = thermostat->getManuelSetPoint();
                    root["mode"]= thermostat->getMode();
                    root["boardtime"] =  this->thermostat->ntpClient->getFormattedTime();
                    root["selected"] = thermostat->getSelectedSensor();
                    root["hysteresis_h"] = thermostat->getHysteresisHigh();
                    root["hysteresis_l"] = thermostat->getHysteresisLow();

                    String output;
                    serializeJson(root, output);

                    webSocket->sendTXT(num, output);
                } else if(target.equals("thermostat_selected")) {

                    if(root.containsKey("sensorid"))
                    {
                        int sensorID = root["sensorid"];
                        thermostat->setSelectedSensor(sensorID);
                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }

                } else if(target.equals("set_hysteresis")) {

                    if(root.containsKey("hysteresis_h") && root.containsKey("hysteresis_l")){
                        if(root["hysteresis_h"].is<int>() && root["hysteresis_l"].is<int>() )
                        {
                            int hysteresis_h = root["hysteresis_h"];
                            int hysteresis_l = root["hysteresis_l"];

                            thermostat->setHysteresisHigh(hysteresis_h);
                            thermostat->setHysteresisLow(hysteresis_l);
                            thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                        }


                    }


                } else if(target.equals("get_schedule")) {
                    // thermostat_schedule
                    DynamicJsonDocument jsonThermostatSchedule(4096);
                    jsonThermostatSchedule["event"] = "thermostat_schedule";

                    for(int dow=0; dow<7; dow++) {
                        // TODO: remove this array
                        const char * days[7] = {"mon","tue","wed","thu","fri","sat","sun"};

                        JsonArray jsonDow = jsonThermostatSchedule.createNestedArray(days[dow]);

                        for(int sched=0; sched<8 && thermostat->thermostatSchedule.weekSched[dow].daySched[sched].active==1; sched++) {
                            JsonObject zone = jsonDow.createNestedObject();
                            zone["s"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].start;
                            zone["e"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].end;
                            zone["sp"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].setpoint;
                        }
                    }
                    String output2;
                    serializeJson(jsonThermostatSchedule,output2);
                    webSocket->sendTXT(num, output2);

                }


            }

            break;
    }
}