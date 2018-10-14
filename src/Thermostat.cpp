//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "Thermostat.h"


Thermostat::Thermostat(Relay &relay)
{
    this->relay = &relay;
    timeClient= new NTPClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
    timeClient->setTimeOffset(7200); /// GMT +1 = 3600; GMT+2 7200, ...
    timeClient->begin();

    // TODO load
    thermostatManualsetpoint=2100;

}
Thermostat::~Thermostat() {

    delete timeClient;
}

void Thermostat::setManualTemperature(int temp) {
    thermostatManualsetpoint = temp;
}

int Thermostat::getThermostatMode(){
    return thermostatMode;
}

int Thermostat::getThermostatState(){
    return thermostatState;
}

int Thermostat::getThermostatManuelSetPoint(){
    return thermostatManualsetpoint;
}

void Thermostat::update() {
    timeClient->update();


    float Treading=0;

    if(thermostatState == THERMOSTAT_OFF) {
        Serial.println("Thermostat switched off, abandoning routine.");
        return;
    }
    if(thermostatMode == THERMOSTAT_MANUAL) {
        handle(Treading, thermostatManualsetpoint);
        return;
    }



}

void Thermostat::handle(float current_t, float setpoint) {
    if(current_t < setpoint - thermostatHysteresisLow ) {
        Serial.printf("Current reading (%d) is less than the setpoint.\n",current_t);
        this->relay->turnOn();
    } else if(current_t > setpoint + thermostatHysteresisHigh ) {
        Serial.printf("Current reading (%d) is more than the setpoint.\n",current_t);
        this->relay->turnOff();

    }

}