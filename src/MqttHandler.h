//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_MQTTHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_MQTTHANDLER_H

#include <PubSubClient.h> // get it here: https://github.com/knolleary/pubsubclient.git

#include "WifiHandler.h"
#include "Thermostat.h"
#include "Sensors.h"

class MqttHandler
{
public:
    MqttHandler(Sensors &sensors,WifiHandler &wifiHandler,Thermostat &thermostat);
    ~MqttHandler();

    void setup(const char* mqttServer,const char *mqttUser, const char *mqttPassword,int mqttPort);
    void reconnect();
    void update();

private:
    PubSubClient *client;
    WifiHandler *wifiHandler;
    Thermostat *thermostat;
    Sensors *sensors;
    const char* mqttServer;
    int mqttPort;
    const char* mqttUser;
    const char* mqttPassword;
    void callback(char* topic, byte* payload, unsigned int length);
    double timeRetry;

};

#endif //ESP8266_RELAY_BOARD_ARDUINO_MQTTHANDLER_H
