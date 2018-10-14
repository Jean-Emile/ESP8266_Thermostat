//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "OtaHandler.h"


OtaHandler::OtaHandler() {

}
OtaHandler::~OtaHandler() {

}


void OtaHandler::setup() {

    const char *OTAName = "ESP8266";           // A name and a password for the OTA service
    const char *OTAPassword = "esp8266";

    ArduinoOTA.setHostname(OTAName);
    ArduinoOTA.setPassword(OTAPassword);

    ArduinoOTA.onStart([]() {
        Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\r\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("OTA ready\r\n");
}

void OtaHandler::update() {
    ArduinoOTA.handle();                        // listen for OTA events
}