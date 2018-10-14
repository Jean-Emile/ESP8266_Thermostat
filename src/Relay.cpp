//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "Relay.h"


Relay::Relay() {
    relayState=0;

}

void Relay::setup() {
    pinMode(RELAY_PIN, OUTPUT);

}


int Relay::getState() {
    return relayState;
}
void Relay::turnOff() {
    if (relayState == 1)
    {
        digitalWrite(RELAY_PIN,LOW);
        relayState=0;
    }else
    {
        Serial.println("Already turned OFF");
    }

}

void Relay::turnOn() {
    if(relayState == 0){
        digitalWrite(RELAY_PIN,HIGH);
        relayState=1;
    }else
    {
        Serial.println("Already turned ON");
    }

}