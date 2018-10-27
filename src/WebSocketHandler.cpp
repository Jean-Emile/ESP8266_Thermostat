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
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {              // if a new websocket connection is established
            IPAddress ip = webSocket->remoteIP(num);

            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            StaticJsonBuffer<250> jsonBuffer; // https://arduinojson.org/v5/assistant/
            JsonObject& root = jsonBuffer.createObject();
            root["cmd"] = "thermostat_state";
            root["temperature"] = sensors->readSensor(0).temperature;
            root["humidity"] = sensors->readSensor(0).humidity;
            root["relayState"] = relay->getState();
            root["state"] = thermostat->getThermostatState();
            root["manualsetpoint"] = thermostat->getThermostatManuelSetPoint();
            root["mode"]= thermostat->getThermostatMode();
            String output;
            root.printTo(output);

            webSocket->sendTXT(num, output);

            // thermostat_schedule
            DynamicJsonBuffer jsonBufferThermostatSchedule;
            JsonObject& jsonThermostatSchedule = jsonBufferThermostatSchedule.createObject();
            jsonThermostatSchedule["cmd"] = "thermostat_schedule";

            for(int dow=0; dow<7; dow++) {
                char * days[7] = {"mon","tue","wed","thu","fri","sat","sun"};

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
            Serial.printf("[%u] get Text: %s\n", num, payload);
            DynamicJsonBuffer jsonBuffer;
            JsonObject& root = jsonBuffer.parseObject(&payload[0]);

            if (!root.success()) {
                Serial.println("parseObject() failed");

            }else
            {
                String target = root["cmd"].asString();

                if (target.equals("relay")) {
                    bool state = root["state"];
                    if(state == true){
                        relay->turnOn();
                    }else if(state== false)
                    {
                        relay->turnOff();
                    }

                }else if(target.equals("restart")) {
                    ESP.restart();
                }else if(target.equals("freeheap")){
                    StaticJsonBuffer<50> jsonBuffer;
                    JsonObject& freeHeap = jsonBuffer.createObject();
                    freeHeap["cmd"] = "freeheap";
                    root["freeheap"] =  ESP.getFreeHeap();
                    String retJson;
                    root.printTo(retJson);
                    webSocket->sendTXT(num, retJson);

                }else if(target.equals("scanwifi")){
                    Serial.println("scanwifi");
                    DynamicJsonBuffer jsonBuffer;
                    JsonObject& root = jsonBuffer.createObject();
                    root["cmd"] = "wifiscan";
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
                    bool state = root["state"];

                    if(state == true){
                        thermostat->turnOn();
                    }else if(state== false)
                    {
                        thermostat->turnOff();
                    }

                }else if(target.equals("thermostat_manualsetpoint"))
                {
                    int temperature = root["temperature"];
                    thermostat->setManualTemperature(temperature);


                }else if(target.equals("thermostat_schedule")){

                    if (root.containsKey("day") && root.containsKey("schedule" ) && root.is<int>("day")) {
                        int dow = root["day"];

                        if(dow >=0 && dow <=7){

                            JsonArray& scheduleRoot = root["schedule"].asArray();
                            int i=0;
                            for (auto& row : scheduleRoot) {
                              // TODO: check type
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

                            }
                        }

                    }

                } else if(target.equals("thermostat_mode")){
                    int mode = root["mode"];
                    thermostat->setMode(mode);
                }


            }

            break;
    }
}