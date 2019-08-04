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
        nodes[id].active = false;
    }
}
Sensors::~Sensors(){

}

Node Sensors::readSensor(int id){
    if(id >=0 && id <= MAX_SENSORS){
        return nodes[id];
    }
    // TODO return
}

void Sensors::updateValue(int id, int type, int value){
    if(id >=0 && id <= MAX_SENSORS){

        switch (type){
            case SENSOR_TYPE_TEMPERATURE:
                nodes[id].temperature = value;
                break;
            case SENSOR_TYPE_HUMIDITY:
                nodes[id].humidity = value;
                break;
            case SENSOR_TYPE_RSSI:
                nodes[id].rssi = value;
                break;
            case SENSOR_TYPE_VOLTAGE:
                nodes[id].battery = value;
                break;
        }

        nodes[id].lastUpdate = ntpClient->getEpochTime();
        nodes[id].active = true;
    }else{

        // Warning
    }
}
