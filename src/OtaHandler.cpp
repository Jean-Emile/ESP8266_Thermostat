//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "OtaHandler.h"


OtaHandler::OtaHandler() {

}
OtaHandler::~OtaHandler() {

}


void OtaHandler::setup() {
    Serial.println(F("[INFO] OtaHandler setup()"));

    const char *OTAName = "ESP8266";           // A name and a password for the OTA service
    const char *OTAPassword = "esp8266";

    ArduinoOTA.setHostname(OTAName);
    ArduinoOTA.setPassword(OTAPassword);

    ArduinoOTA.onStart([]() {
        Serial.println(F("Start"));
    });
    ArduinoOTA.onEnd([]() {
        Serial.println(F("\r\nEnd"));
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
        else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
        else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
        else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
        else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
    });
    ArduinoOTA.begin();
    Serial.println(F("[INFO] OTA ready"));
}

void OtaHandler::update() {
    ArduinoOTA.handle();                        // listen for OTA events
}