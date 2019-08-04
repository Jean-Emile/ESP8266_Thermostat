//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_WEBSERVERHANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_WEBSERVERHANDLER_H

#include <ESP8266WebServer.h>

#include <FS.h>   // Include the SPIFFS library

class WebServerHandler
{
public:
    WebServerHandler();
    ~WebServerHandler();

    void setup();
    void update();


private:
    ESP8266WebServer *server;
    void handleFileUpload();
    void handleNotFound();
    bool handleFileRead(String path);
    String getContentType(String filename);
    File fsUploadFile;                 // a File variable to temporarily store the received file


};

#endif //ESP8266_RELAY_BOARD_ARDUINO_WEBSERVERHANDLER_H
