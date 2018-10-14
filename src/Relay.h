//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_RELAY_H
#define ESP8266_RELAY_BOARD_ARDUINO_RELAY_H

#include <Arduino.h>
#define RELAY_PIN     5

class Relay
{
public:
    Relay();
    void setup();
    void turnOn();
    void turnOff();
    int getState();
private:
    int relayState;
};
#endif //ESP8266_RELAY_BOARD_ARDUINO_RELAY_H
