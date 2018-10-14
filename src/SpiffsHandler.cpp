//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "SpiffsHandler.h"



SpiffsHandler::SpiffsHandler(){


}
SpiffsHandler::~SpiffsHandler() {

}

void SpiffsHandler::update() {


}

String SpiffsHandler::formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
    if (bytes < 1024) {
        return String(bytes) + "B";
    } else if (bytes < (1024 * 1024)) {
        return String(bytes / 1024.0) + "KB";
    } else if (bytes < (1024 * 1024 * 1024)) {
        return String(bytes / 1024.0 / 1024.0) + "MB";
    }
}


void SpiffsHandler::setup() {
    SPIFFS.begin();                             // Start the SPI Flash File System (SPIFFS)
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