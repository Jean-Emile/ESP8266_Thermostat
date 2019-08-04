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
        Serial.println("[CRITICAL] Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 450) {
        Serial.println("[CRITICAL] Config file size is too large");
        return false;
    }
    Serial.println(size);

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);
    DynamicJsonDocument config(1024);
    DeserializationError error = deserializeJson(config, buf.get());


    if (error) {
        Serial.println("[WARNING] Failed to parse config file");
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
    // SMTP
    strcpy(this->mailServer,config["mail"]["server"]);
    strcpy(this->mailUser,config["mail"]["login"]);
    strcpy(this->mailPassword,config["mail"]["password"]);
    strcpy(this->mailfrom,config["mail"]["from"]);
    strcpy(this->mailfrom,config["mail"]["to"]);
    this->mailPort = config["mail"]["port"];



    return true;
}

bool SpiffsHandler::saveConfiguration() {
    StaticJsonDocument<200> json;
    json["ssid"] = "";
    json["wpwd"] = "";

    // TODO
    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    serializeJson(json, configFile);
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
    Serial.println("[INFO] WebSocketHandler setup()");
    Serial.println("SPIFFS started. Contents:");
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