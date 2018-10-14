//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "MqttHandler.h"

MqttHandler::MqttHandler(char* mqttServer,char *mqttUser, char *mqttPassword,int mqttPort, WifiHandler &_wifiHandler){
    this->wifiHandler = &_wifiHandler;
    this->mqttServer = mqttServer;
    this->mqttUser = mqttUser;
    this->mqttPassword = mqttPassword;
    this->mqttPort = mqttPort;
}

MqttHandler::~MqttHandler(){
    if (client)
        delete  client;
}
void MqttHandler::reconnect() {
    // Loop until we're reconnected
    while (!client->connected()) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client->connect(clientId.c_str(), mqttUser,mqttPassword)) {
            Serial.println("connected");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client->state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
            // TODO RESTART ESP when several retry has failed
        }
    }

}

void MqttHandler::setup() {
    // check if null
    client = new PubSubClient( this->wifiHandler->getClient());
    client->setServer(this->mqttServer, this->mqttPort);
    client->setCallback(std::bind(&MqttHandler::callback, this,
            std::placeholders::_1,
            std::placeholders::_2,
            std::placeholders::_3));
}


void MqttHandler::update() {
    if (!client->connected()) {
        reconnect();
    }
    client->loop();
}

void MqttHandler::callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
        digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
        // but actually the LED is on; this is because
        // it is active low on the ESP-01)
    } else {
        digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    }

}
