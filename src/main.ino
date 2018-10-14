//
// Created by jean-emile.dartois (jedartois@gmail.com) on 10/10/18.
//

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <NTPClient.h> // get it here https://github.com/arduino-libraries/NTPClient
#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPIFlash.h>      //get it here: https://www.github.com/lowpowerlab/spiflash

#include <SPI.h> //included with Arduino IDE install (www.arduino.cc)
#include <FS.h>   // Include the SPIFFS library
#include <PubSubClient.h> // get it here: https://github.com/knolleary/pubsubclient.git
#include <WebSocketsServer.h> // https://github.com/Links2004/arduinoWebSockets
#include <Hash.h>
#include <ArduinoOTA.h>

#include "Thermostat.h"
#include "Relay.h"
#include "RF69Handler.h"
#include "WebSocketHandler.h"
#include "WifiHandler.h"
#include "MqttHandler.h"
#include "WebServerHandler.h"
#include "OtaHandler.h"
#include "SpiffsHandler.h"

WifiHandler wifiHandler("","");
Relay relay;
Thermostat thermostat(relay);
RF69Handler rf69Handler;
WebSocketHandler webSocketHandler(thermostat,relay);
MqttHandler mqttHandler("", "","", 8883, wifiHandler);
WebServerHandler webServerHandler;
OtaHandler otaHandler;
SpiffsHandler spiffsHandler;

void Blink(byte PIN, int DELAY_MS)
{
    pinMode(PIN, OUTPUT);
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
}


void setup(void){
    Serial.begin(115200);
    wifiHandler.setup();
    rf69Handler.setup();
    otaHandler.setup();
    spiffsHandler.setup();
    relay.setup();
    webServerHandler.setup();
    mqttHandler.setup();
    webSocketHandler.setup();
}


void loop(void){
    wifiHandler.update();
    webSocketHandler.update();                           // constantly check for websocket events
    webServerHandler.update();                      // run the server
    otaHandler.update();
    rf69Handler.update();
    mqttHandler.update();
    spiffsHandler.update();
    thermostat.update();
}