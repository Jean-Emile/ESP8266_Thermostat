//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_MQTTHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_MQTTHANDLER_H

#include <PubSubClient.h> // get it here: https://github.com/knolleary/pubsubclient.git

#include "WifiHandler.h"

class MqttHandler
{
public:
    MqttHandler(char* mqttServer,char *mqttUser, char *mqttPassword,int mqttPort, WifiHandler &wifiHandler);
    ~MqttHandler();

    void setup();
    void reconnect();
    void update();

private:
PubSubClient *client;
WifiHandler *wifiHandler;
char* mqttServer;
int mqttPort;
char* mqttUser;
char* mqttPassword;
void callback(char* topic, byte* payload, unsigned int length);

};

#endif //ESP8266_RELAY_BOARD_ARDUINO_MQTTHANDLER_H
