//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "SpiffsHandler.h"



SpiffsHandler::SpiffsHandler(){


}
SpiffsHandler::~SpiffsHandler() {
    SPIFFS.end();
}




bool SpiffsHandler::loadConfiguration() {
    File configFile = SPIFFS.open("/config.json", "r");
    if (!configFile) {
        Serial.println(F("[CRITICAL] Failed to open config file"));
        return false;
    }

    size_t size = configFile.size();
    if (size > 450) {
        Serial.println(F("[CRITICAL] Config file size is too large"));
        return false;
    }
     // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);
    DynamicJsonDocument config(1024);
    DeserializationError error = deserializeJson(config, buf.get());


    if (error) {
        Serial.println(F("[WARNING] Failed to parse config file"));
        config.clear();
        configFile.close();
        return false;
    }

    strcpy(this->ssid,config["wifi"]["ssid"]);
    strcpy(this->wifiPassword,config["wifi"]["password"]);

    // MQTT
    strcpy(this->mqttServer,config["mqtt"]["server"]);
    strcpy(this->mqttUser,config["mqtt"]["user"]);
    strcpy(this->mqttPassword,config["mqtt"]["password"]);
    this->mqttPort = config["mqtt"]["port"];

    this->tz = config["ntp"]["tz"];
    config.clear();
    configFile.close();
    return true;
}

bool SpiffsHandler::saveConfiguration() {
    DynamicJsonDocument json(200);
    json["ssid"] = "";
    json["wpwd"] = "";

    // TODO
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println(F("Failed to open config file for writing"));
        return false;
    }

    serializeJson(json, configFile);
    json.clear();
    configFile.close();
    return true;
}

String SpiffsHandler::formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
    if (bytes < 1024) {
        return String(bytes) + "B";
    } else if (bytes < (1024 * 1024)) {
        return String(bytes / 1024.0) + "KB";
    } else if (bytes < (1024 * 1024 * 1024)) {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
    return  String(bytes);
}


void SpiffsHandler::setup() {

    SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
    Serial.println(F("[INFO] WebSocketHandler setup()"));
    Serial.println(F("SPIFFS started. Contents:"));
    {
        Dir dir = SPIFFS.openDir("/");
        while (dir.next()) {                      // List the file system contents
            String fileName = dir.fileName();
            size_t fileSize = dir.fileSize();
            Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
        }
        Serial.printf("\n");
    }
}