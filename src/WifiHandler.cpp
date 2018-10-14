//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "WifiHandler.h"



WifiHandler::WifiHandler(char *ssid, char * password) {
    this->ssid = ssid;
    this->password = password;
    espClient = new WiFiClient();
}

WifiHandler::~WifiHandler() {


}


void WifiHandler::setup() {

    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.begin(this->ssid,this->password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    if (mdns.begin("esp8266", WiFi.localIP())) {
        Serial.println("MDNS responder started");
    }
}

void WifiHandler::update() {

}


WiFiClient& WifiHandler::getClient() {

    return *(this->espClient);
}