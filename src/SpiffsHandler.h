//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_SPIFFSHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_SPIFFSHANDLER_H

#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>

class SpiffsHandler
{
public:
    SpiffsHandler();
    ~SpiffsHandler();

    void setup();

    bool loadConfiguration();
    bool saveConfiguration();

    char ssid[20];
    char wifiPassword[20];
    char mqttServer[20];
    char mqttUser[20];
    char mqttPassword[20];
    int mqttPort;
    int tz;

    char mailServer[20];
    char mailUser[32];
    char mailPassword[32];
    char mailfrom[64];
    char mailto[64];
    int mailPort;


private:
    String formatBytes(size_t bytes);

};


#endif //ESP8266_RELAY_BOARD_ARDUINO_SPIFFSHANDLER_H
