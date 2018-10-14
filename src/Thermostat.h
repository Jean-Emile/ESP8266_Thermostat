//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H
#define ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H


#include <WiFiUdp.h>
#include <NTPClient.h> // get it here https://github.com/arduino-libraries/NTPClient

#include "Relay.h"

typedef enum {
    THERMOSTAT_SCHEDULE,
    THERMOSTAT_MANUAL,
    THERMOSTAT_ON,
    THERMOSTAT_OFF
} thermo_state_t;


class Thermostat
{
public:
    Thermostat(Relay &relay);
    ~Thermostat();

    void update();
    void setManualTemperature(int temp);
    int getThermostatMode();
    int getThermostatState();
    int getThermostatManuelSetPoint();

private:
    void handle(float current_t, float setpoint);

    NTPClient *timeClient;
    Relay *relay;
    WiFiUDP ntpUDP;

    int thermostatMode;
    int thermostatState;

    int thermostatManualsetpoint;
    int thermostatHysteresisLow;
    int thermostatHysteresisHigh;
};


#endif //ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H
