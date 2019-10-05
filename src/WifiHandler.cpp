//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "WifiHandler.h"
#include "user_interface.h"
#include "wpa2_enterprise.h"

     
WifiHandler::WifiHandler() {
    espClient = new WiFiClient();
}

WifiHandler::~WifiHandler() {


}


void WifiHandler::setup(const char *ssid, const char * password) {

    delay(10);
    // We start by connecting to a WiFi network
   
    Serial.print(F("Connecting to "));
    Serial.println(ssid);
    // Normal Connection starts here
    WiFi.begin(ssid,password);

    /* WPA2 PEAP

   // SSID to connect to
    const char* ssid2 = "";
    // Username for authentification
    const char* username2 = "";
    // Password for authentication
    const char* password2 = "";
    Serial.println(ssid2);

     wifi_set_opmode(STATION_MODE);
    struct station_config wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config));
    strcpy((char*)wifi_config.ssid, ssid);
    strcpy((char*)wifi_config.password, password2);
    wifi_station_set_config(&wifi_config);
    wifi_station_set_wpa2_enterprise_auth(1);

    // Clean up to be sure no old data is still inside
    wifi_station_clear_cert_key();
    wifi_station_clear_enterprise_ca_cert();
    wifi_station_clear_enterprise_identity();
    wifi_station_clear_enterprise_username();
    wifi_station_clear_enterprise_password();
    wifi_station_clear_enterprise_new_password();


    wifi_station_set_enterprise_identity((uint8*)username2, strlen(username2));
    wifi_station_set_enterprise_username((uint8*)username2, strlen(username2));
    wifi_station_set_enterprise_password((uint8*)password2, strlen(password2));
    wifi_station_connect();
    */
      
    int i=0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(F("."));
        if (i > 30){
             Serial.println(F("Reset.."));
             ESP.restart();
        }
        i++;
    }

    randomSeed(micros());

    Serial.println(F(""));
    Serial.println(F("WiFi connected"));
    Serial.println(F("IP address: "));
    Serial.println(WiFi.localIP());
    if (mdns.begin("esp8266", WiFi.localIP())) {
        Serial.println(F("MDNS responder started"));
    }
}

void WifiHandler::update() {

}


WiFiClient& WifiHandler::getClient() {

    return *(this->espClient);
}