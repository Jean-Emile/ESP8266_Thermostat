//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_OTAHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_OTAHANDLER_H

#include <Arduino.h>
#include <ArduinoOTA.h>

class OtaHandler
{
public:
    OtaHandler();
    ~OtaHandler();

    void setup();
    void update();

private:
};


#endif //ESP8266_RELAY_BOARD_ARDUINO_OTAHANDLER_H
