//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_SPIFFSHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_SPIFFSHANDLER_H

#include <FS.h>   // Include the SPIFFS library

class SpiffsHandler
{
public:
    SpiffsHandler();
    ~SpiffsHandler();

    void setup();
    void update();

private:
    String formatBytes(size_t bytes);

};


#endif //ESP8266_RELAY_BOARD_ARDUINO_SPIFFSHANDLER_H
