//
// Created by jedartois on 19/10/18.
//

#include "Sensors.h"


Sensors::Sensors(NTPClient &ntpClient){
    this->ntpClient = &ntpClient;
    for(int id=0; id < MAX_SENSORS;id++){
        nodes[id].temperature = 0;
        nodes[id].humidity = 0;
        nodes[id].lastUpdate = 0;
    }
}
Sensors::~Sensors(){

}

Node Sensors::readSensor(int id){
    if(id >=0 && id <= MAX_SENSORS){
        return nodes[id];
    }
}

void Sensors::updateTemperature(int id, int temperature){
    if(id >=0 && id <= MAX_SENSORS){
        nodes[id].temperature = temperature;
        nodes[id].lastUpdate = ntpClient->getEpochTime();
    }
}
void Sensors::updateHumidity(int id, int humidity){
    if(id >=0 && id <= MAX_SENSORS){
        nodes[id].humidity = humidity;
        nodes[id].lastUpdate = ntpClient->getEpochTime();
    }
}

void Sensors::updateSensor(int id, int temperature, int humidity){
    if(id >=0 && id <= MAX_SENSORS){
        nodes[id].temperature = temperature;
        nodes[id].humidity = humidity;
        nodes[id].lastUpdate = ntpClient->getEpochTime();
    }
}