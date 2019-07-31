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
#include "Gsender.h"

WifiHandler wifiHandler;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
Sensors sensors(timeClient);
Relay relay;
Thermostat thermostat(timeClient,sensors,relay);

SpiffsHandler spiffsHandler;

//NOTE: consider using singletons
WebSocketHandler webSocketHandler(sensors,spiffsHandler,thermostat,relay);
MqttHandler mqttHandler(sensors,wifiHandler,thermostat);
RF69Handler rf69Handler(sensors, mqttHandler);
WebServerHandler webServerHandler;
OtaHandler otaHandler;

void ledblink(int times, int lengthms, int pinnum) {
    for (int x = 0; x < times; x++) {
        digitalWrite(pinnum, HIGH);
        delay(lengthms);
        digitalWrite(pinnum, LOW);
        delay(lengthms);
    }
}

#define  PIN_BUTTON 0
int current;         // Current state of the button
long millis_held;    // How long the button was held (milliseconds)
long secs_held;      // How long the button was held (seconds)
long prev_secs_held; // How long the button was held in the previous check
byte previous = HIGH;
unsigned long firstTime; // how long since the button was first pressed


void setup(void){
    Serial.begin(115200);
    digitalWrite(PIN_BUTTON, HIGH);
    spiffsHandler.setup();
    timeClient.begin();

    spiffsHandler.loadConfiguration();
    thermostat.loadConfiguration();

    timeClient.setTimeOffset(spiffsHandler.tz); /// GMT +1 = 3600; GMT+2 7200, ..

    wifiHandler.setup(spiffsHandler.ssid,spiffsHandler.wifiPassword);
    rf69Handler.setup();
    otaHandler.setup();

    relay.setup();
    webServerHandler.setup();
    mqttHandler.setup(spiffsHandler.mqttServer,
                      spiffsHandler.mqttUser,
                      spiffsHandler.mqttPassword,
                      spiffsHandler.mqttPort);
    webSocketHandler.setup();

    Gsender *gsender = Gsender::Instance();

    gsender->setup(spiffsHandler.mailServer,
                   spiffsHandler.mailUser,
                   spiffsHandler.mailPassword,
                   spiffsHandler.mailfrom,
                   spiffsHandler.mailto,
                   spiffsHandler.mailPort);

}



void loop(void) {
    timeClient.update();
    wifiHandler.update();
    webSocketHandler.update();                           // constantly check for websocket events
    webServerHandler.update();                      // run the server
    otaHandler.update();
    rf69Handler.update();
    mqttHandler.update();
    thermostat.update();

    current = digitalRead(PIN_BUTTON);

    // if the button state changes to pressed, remember the start time
    if (current == LOW && previous == HIGH && (millis() - firstTime) > 200) {
        firstTime = millis();
    }

    millis_held = (millis() - firstTime);
    secs_held = millis_held / 1000;

    if (millis_held > 50) {

        if (current == LOW && secs_held > prev_secs_held) {
            ledblink(1, 50, 16);
        }
        // check if the button was released since we last checked
        if (current == HIGH && previous == LOW) {
            if (secs_held >= 5) {

                Serial.print("Setting soft-AP ... ");
                Serial.println(WiFi.softAP("CELSIUS") ? "Ready" : "Failed!");
                webServerHandler.setup();

            }
        }
        previous = current;
        prev_secs_held = secs_held;
    }
}
