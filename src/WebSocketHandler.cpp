//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "WebSocketHandler.h"


void WebSocketHandler::onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
    switch (type) {
        case WStype_DISCONNECTED:             // if the websocket is disconnected
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED: {              // if a new websocket connection is established
            IPAddress ip = webSocket->remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

            StaticJsonBuffer<237> jsonBuffer; // https://arduinojson.org/v5/assistant/
            JsonObject& root = jsonBuffer.createObject();
            root["id"] = "thermostat_state";
            root["temperature"] = "23";
            root["humidity"] = "55.0";
            root["relayState"] = relay->getState();
            root["opmode"] = thermostat->getThermostatState();
            root["manualsetpoint"] = thermostat->getThermostatManuelSetPoint();
            root["mode"]= thermostat->getThermostatMode();
            String output;
            root.printTo(output);

            webSocket->sendTXT(num, output);

        }
            break;
        case WStype_TEXT:                     // if new text data is received
            Serial.printf("[%u] get Text: %s\n", num, payload);
            String _payload = String((char *) &payload[0]);

            String command = (_payload.substring(_payload.indexOf("#")+1,_payload.indexOf("=")));
            String data = (_payload.substring(_payload.indexOf("=")+1,_payload.length()));
            if (command.equals("relay")) {
                if(data.toInt() == 1){
                    relay->turnOn();
                }else if(data.toInt() == 0)
                {
                    relay->turnOff();
                }else
                {
                    Serial.println("Hug huhu?");
                }


            }else if(command.equals("restart")){
                if(data.toInt() == 1){
                    ESP.restart();
                }

            }


            break;
    }
}

WebSocketHandler::WebSocketHandler(Thermostat &thermostat, Relay &relay) {
    this->relay = &relay;
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