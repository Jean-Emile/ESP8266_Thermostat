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
    // TODO load/save config
    //  webSocket->setAuthorization("esp8266", "Led*ZInqsdf9");
    webSocket->begin();                          // start the websocket server
    webSocket->onEvent(std::bind(&WebSocketHandler::onWsEvent, this,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3,
                                 std::placeholders::_4));          // if there's an incomming websocket message, go to function 'webSocketEvent'

    Serial.println("WebSocket server started.");
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

            // thermostat_schedule
            DynamicJsonBuffer jsonBufferThermostatSchedule;
            JsonObject& jsonThermostatSchedule = jsonBufferThermostatSchedule.createObject();
            jsonThermostatSchedule["event"] = "thermostat_schedule";

            for(int dow=0; dow<7; dow++) {
                // TODO: remove this array
                const char * days[7] = {"mon","tue","wed","thu","fri","sat","sun"};

                JsonArray& jsonDow = jsonThermostatSchedule.createNestedArray(days[dow]);

                for(int sched=0; sched<8 && thermostat->thermostatSchedule.weekSched[dow].daySched[sched].active==1; sched++) {
                    JsonObject& zone = jsonDow.createNestedObject();
                    zone["s"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].start;
                    zone["e"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].end;
                    zone["sp"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].setpoint;
                }
            }
            String output2;
            jsonThermostatSchedule.printTo(output2);
            webSocket->sendTXT(num, output2);


        }
            break;
        case WStype_TEXT:                     // if new text data is received
            // Serial.printf("[%u] get Text: %s\n", num, payload);

            DynamicJsonBuffer jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(&payload[0]);

            if (!root.success()) {
                Serial.println("parseObject() failed");

            }else
            {
                String target = root["event"].asString();

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
                    DynamicJsonBuffer jsonBuffer;
                    JsonObject& freeHeap = jsonBuffer.createObject();
                    freeHeap["event"] = "freeheap";
                    root["freeheap"] =  ESP.getFreeHeap();
                    String retJson;
                    root.printTo(retJson);
                    webSocket->sendTXT(num, retJson);
                }else if(target.equals("time")){
                    DynamicJsonBuffer jsonBuffer;
                    JsonObject& freeHeap = jsonBuffer.createObject();
                    freeHeap["event"] = "time";
                    root["currtime"] =  this->thermostat->ntpClient->getFormattedTime();
                    String retJson;
                    root.printTo(retJson);
                    webSocket->sendTXT(num, retJson);
                }else if(target.equals("do_wifi_scan")){

                    Serial.println("do_wifi_scan");

                    DynamicJsonBuffer jsonBuffer;
                    JsonObject& root = jsonBuffer.createObject();
                    root["event"] = "wifi_scan";
                    byte n = WiFi.scanNetworks();
                    JsonArray& networks = root.createNestedArray("networks");

                    for(int i=0;i<n;i++) {
                        JsonObject& network = networks.createNestedObject();
                        network["ssid"] = WiFi.SSID(i);
                        network["encryption"] = WiFi.encryptionType(i);
                        network["rssi"] = WiFi.RSSI(i);
                        network["bssid"] = WiFi.BSSIDstr(i);
                        network["channel"] = WiFi.channel(i);
                        network["isHidden"] = WiFi.isHidden(i);
                    }

                    String retJson;
                    root.printTo(retJson);
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

                    if (root.containsKey("day") && root.containsKey("schedule" ) && root.is<int>("day")) {
                        int dow = root["day"];

                        if(dow >=0 && dow <=7){

                            JsonArray& scheduleRoot = root["schedule"].asArray();
                            int i=0;
                            for (auto& row : scheduleRoot) {

//                                if(root.containsKey("s") && root["s"].is<int>() &&
//                                   root.containsKey("e") && root["e"].is<int>() &&
//                                   root.containsKey("sp") && root["sp"].is<int>()) {

                                    int start = row["s"];
                                    int end = row["e"];
                                    int sp = row["sp"];

                                    if(i < 8){
                                        thermostat->thermostatSchedule.weekSched[dow].daySched[i].start=start; //0am
                                        thermostat->thermostatSchedule.weekSched[dow].daySched[i].end=end; //6am, hours are * 100
                                        thermostat->thermostatSchedule.weekSched[dow].daySched[i].setpoint=sp; //10.0*C
                                        thermostat->thermostatSchedule.weekSched[dow].daySched[i].active=1;
                                        i++;
                                    }

                                //}



                            }
                            if(i <8){
                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].active=0;	//mark the next schedule as inactive
                            }
                        }
                        thermostat->saveConfiguration(SCHEDULE_CONFIG);

                    }

                } else if(target.equals("thermostat_mode")){

                    if(root.containsKey("mode") && root["mode"].is<int>()) {
                        int mode = root["mode"];
                        thermostat->setMode(mode);
                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }

                }else if(target.equals("get_thermostat_states")){

                    DynamicJsonBuffer jsonBuffer; // https://arduinojson.org/v5/assistant/
                    JsonObject& root = jsonBuffer.createObject();
                    root["event"] = "thermostat_states";

                    JsonArray& jsonSensors = root.createNestedArray("sensors");

                    for(int i=0; i< MAX_SENSORS;i++){
                        if(sensors->readSensor(i).active == true){
                            JsonObject& sensor = jsonSensors.createNestedObject();
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
                    root.printTo(output);

                    webSocket->sendTXT(num, output);
                } else if(target.equals("thermostat_selected")) {

                    if(root.containsKey("sensorid"))
                    {
                        int sensorID = root["sensorid"];
                        thermostat->setSelectedSensor(sensorID);

                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }

                } else if(target.equals("set_hysteresis")) {


                    Serial.print(target);
                    if(root.containsKey("hysteresis_h") && root.containsKey("hysteresis_l")){

                        if(root["hysteresis_h"].is<int>() && root["hysteresis_l"].is<int>() )
                        {

                            int hysteresis_h = root["hysteresis_h"];
                            int hysteresis_l = root["hysteresis_l"];

                            thermostat->setHysteresisHigh(hysteresis_h);
                            thermostat->setHysteresisLow(hysteresis_l);
                        }


                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }



                }


            }

            break;
    }
}