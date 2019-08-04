//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_WIFIHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_WIFIHANDLER_H

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>


class WifiHandler
{
public:
    WifiHandler();
    ~WifiHandler();

    void setup(const char *ssid, const char * password);
    void update();
    WiFiClient& getClient();

private:
    MDNSResponder mdns;
    WiFiClient *espClient;
};
#endif //ESP8266_RELAY_BOARD_ARDUINO_WIFIHANDLER_H
