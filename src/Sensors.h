//
// Created by jedartois on 19/10/18.
//

#ifndef ESP8266_THERMOSTAT_SENSORS_H
#define ESP8266_THERMOSTAT_SENSORS_H

#include <NTPClient.h> // get it here https://github.com/arduino-libraries/NTPClient
#define  MAX_SENSORS 15 //#FIXME c'est pas le max mais indice

class Node {
public:
    unsigned long lastUpdate;
    int temperature;
    int humidity;
    int battery;
    int rssi;
    bool active;
};


typedef enum
{
    SENSOR_TYPE_HUMIDITY     =      (0),
    SENSOR_TYPE_TEMPERATURE  =      (1),
    SENSOR_TYPE_VOLTAGE      =      (2),
    SENSOR_TYPE_RSSI         =      (3),
    SENSOR_TYPE_POWER        =      (4)
} sensors_type_t;


class Sensors {
public:
    Sensors(NTPClient &ntpClient);
    ~Sensors();
    Node readSensor(int id);
    void updateValue(int id, int sensors_type, int value);

private:
    NTPClient *ntpClient;
    Node nodes[MAX_SENSORS];
};
#endif //ESP8266_THERMOSTAT_SENSORS_H
