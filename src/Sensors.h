//
// Created by jedartois on 19/10/18.
//

#ifndef ESP8266_THERMOSTAT_SENSORS_H
#define ESP8266_THERMOSTAT_SENSORS_H

#include <NTPClient.h> // get it here https://github.com/arduino-libraries/NTPClient
#define  MAX_SENSORS 10

class Node {
public:
    int lastUpdate;
    int temperature;
    int humidity;
};

class Sensors {
public:
    Sensors(NTPClient &ntpClient);
    ~Sensors();

    Node readSensor(int id);
    void updateSensor(int id, int temperature, int humidity);
    void updateTemperature(int id, int temperature);
    void updateHumidity(int id, int humidity);

private:
    NTPClient *ntpClient;
    Node nodes[MAX_SENSORS];
};
#endif //ESP8266_THERMOSTAT_SENSORS_H
