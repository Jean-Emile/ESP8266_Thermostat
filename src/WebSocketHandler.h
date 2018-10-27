//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_WEBSOCKETHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_WEBSOCKETHANDLER_H

#include <Arduino.h>

#include <WebSocketsServer.h> // get it here https://github.com/Links2004/arduinoWebSockets
#include <ArduinoJson.h> //  get it here  https://github.com/bblanchon/ArduinoJson ( 5.13.3)
#include "SpiffsHandler.h"
#include "Relay.h"
#include "Thermostat.h"

class WebSocketHandler
{
public:
    WebSocketHandler(Sensors &Sensors,SpiffsHandler &spiffsHandler, Thermostat &thermostat, Relay &relay);
    ~WebSocketHandler();

    void setup();
    void update();
    void onWsEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght);

private:
    WebSocketsServer *webSocket;
    Relay *relay;
    Sensors *sensors;
    Thermostat *thermostat;
    SpiffsHandler  *spiffsHandler;

};



#endif //ESP8266_RELAY_BOARD_ARDUINO_WEBSOCKETHANDLER_H
