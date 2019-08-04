//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "Relay.h"


Relay::Relay() {
    relayState=false;
}

void Relay::setup() {
    Serial.println("[INFO] Relay setup()");
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN,LOW); // TODO: check if mandatory
}


bool Relay::getState() {
    return relayState;
}
void Relay::turnOff() {
    if (relayState == true)
    {
        digitalWrite(RELAY_PIN,LOW);
        relayState= false;
    }
}

void Relay::turnOn() {
    if(relayState == false){
        digitalWrite(RELAY_PIN,HIGH);
        relayState= true;
    }
}