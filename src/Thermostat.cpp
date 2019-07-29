//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "Thermostat.h"
#include "Gsender.h"

Thermostat::Thermostat(NTPClient &ntpClient,Sensors &sensors, Relay &relay)
{
    this->relay = &relay;
    this->sensors = &sensors;
    this->ntpClient = &ntpClient;
    turnOff();
    this->thermostatMode = THERMOSTAT_MANUAL;
    thermostatManualsetpoint=2100;
    selectedSensor = 0;
    lastUpdate=millis();
}
Thermostat::~Thermostat() {


}

void Thermostat::setManualSetPoint(int temp) {
    thermostatManualsetpoint = temp;
    Serial.print("ManualSetPoint");
    Serial.println(thermostatManualsetpoint);
}

int Thermostat::getMode(){
    return thermostatMode;
}

void Thermostat::setMode(int mode){

    if(mode >= THERMOSTAT_MANUAL && mode <= THEMOSTAT_IA){
        thermostatMode = mode;
        Serial.print("ThermostatMode=");
        Serial.println(mode);
    }

}
int Thermostat::getState(){
    return thermostatState;
}

int Thermostat::getManuelSetPoint(){
    return thermostatManualsetpoint;
}
void Thermostat::setHysteresisLow(int temp){
    this->thermostatHysteresisLow = temp;
}
void Thermostat::setHysteresisHigh(int temp){
    this->thermostatHysteresisHigh = temp;
}

int Thermostat::getSelectedSensor(){
    return this->selectedSensor;
}
void Thermostat::setSelectedSensor(int id){
    if(id >=0 && id <= MAX_SENSORS){
        this->selectedSensor = id;
    }
}


void Thermostat::turnOn(){
    thermostatState = THERMOSTAT_ON;
    Serial.println("ON");
}
void Thermostat::turnOff(){
    relay->turnOff();
    thermostatState = THERMOSTAT_OFF;
    Serial.println("OFF");
}

void Thermostat::update() {
    if((millis() > lastUpdate + 20000))
    {
        lastUpdate = millis();
        Serial.println(ntpClient->getFormattedTime());

        unsigned long epoch = ntpClient->getEpochTime();
        int dow=(ntpClient->getDay()+6)%7;
        unsigned int hour=ntpClient->getHours();
        int min = ntpClient->getMinutes();
        int minadj = (min*100/60);
        int currtime = hour*100+minadj;
        Serial.printf("DOW=%d currtime=%d \n",dow, currtime);

        if(this->sensors->readSensor(getSelectedSensor()).active){

            int Treading=this->sensors->readSensor(getSelectedSensor()).temperature;

            if(thermostatState == THERMOSTAT_OFF)
            {
                Serial.println("Thermostat switched off, abandoning routine.");
            }
            else
            {

                if(epoch - this->sensors->readSensor(getSelectedSensor()).lastUpdate  >= 1000){

                    Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance

                    if(gsender->Subject("Stale Measurement!")->Send("jedartois@gmail.com", "The selected sensor value is outdated")) {
                        Serial.println("Message send.");
                    } else {
                        Serial.print("Error sending message: ");
                        Serial.println(gsender->getError());
                    }
                }

                if(thermostatMode == THERMOSTAT_MANUAL) {

                    handle(Treading, thermostatManualsetpoint);

                }else if(thermostatMode ==THERMOSTAT_SCHEDULE) {

                    for(int sched=0; sched<8 && thermostatSchedule.weekSched[dow].daySched[sched].active==1; sched++) {

                        if(currtime >= thermostatSchedule.weekSched[dow].daySched[sched].start && currtime < thermostatSchedule.weekSched[dow].daySched[sched].end) {

                            Serial.printf("Current schedule (%d) setpoint is: %d %d %d\n",sched,thermostatSchedule.weekSched[dow].daySched[sched].setpoint, thermostatSchedule.weekSched[dow].daySched[sched].start,thermostatSchedule.weekSched[dow].daySched[sched].end);
                            handle(Treading, thermostatSchedule.weekSched[dow].daySched[sched].setpoint);
                        }
                    }


                }else if(thermostatMode ==THEMOSTAT_IA) {

                    // TODO
                }
            }

        } else
        {
            Serial.println("Thermostat switched off, no valid Temperature.");
            Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance

            if(gsender->Subject("Stale Measurement!")->Send("jedartois@gmail.com", "The selected sensor value is outdated")) {
                Serial.println("Message send.");
            } else {
                Serial.print("Error sending message: ");
                Serial.println(gsender->getError());
            }
        }

    }

}

void Thermostat::handle(int current_t, int setpoint) {
    if(current_t < setpoint - thermostatHysteresisLow ) {
        Serial.printf("Current reading (%d) is less than the setpoint (%d)\n",current_t, setpoint);
        this->relay->turnOn();
    } else if(current_t > setpoint + thermostatHysteresisHigh ) {
        Serial.printf("Current reading (%d) is more than the setpoint (%d).\n",current_t, setpoint);
        this->relay->turnOff();

    }

}

bool Thermostat::loadConfiguration(){


    File configThermo = SPIFFS.open("/thermostat.json", "r");
    if (!configThermo) {
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configThermo.size();
    if (size > 200) {
        Serial.printf("Config file size is too large %d\n ", size);
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> bufConfig(new char[size]);
    configThermo.readBytes(bufConfig.get(), size);

    DynamicJsonBuffer jsonBufferConfig;
    JsonObject& config = jsonBufferConfig.parseObject(bufConfig.get());
    int mode = config["mode"];
    int manualsetpoint = config["manualsetpoint"];
    int state = config["state"];
    int relay = config["relay"];
    int hysteresLow = config["hysteresLow"];
    int hysteresHigh = config["hysteresHigh"];
    int selectedSensor = config["selectedSensor"];

    setMode(mode);
    setManualSetPoint(manualsetpoint);
    setHysteresisHigh(hysteresHigh);
    setHysteresisLow(hysteresLow);
    setSelectedSensor(selectedSensor);

    if(state == 1){
        turnOn();
    }else{
        turnOff();
    }

    if(relay == 1){
        this->relay->turnOn();
    }else{
        this->relay->turnOff();
    }

    File configFile = SPIFFS.open("/schedule.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }

    size = configFile.size();
    if (size > 1800) {
        Serial.printf("Schedule Config file size is too large %f\n ", size);
        return false;
    }


    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& scheduleJson = jsonBuffer.parseObject(buf.get());

    if (!scheduleJson.success()) {
        Serial.println("Failed to parse config file");
        return false;
    }

    for(int dow=0; dow<7; dow++) {
        JsonArray& schedule = scheduleJson[String(dow)].asArray();
        int i=0;
        for (auto& row : schedule) {
            // TODO: check type
            int start = row["s"];
            int end = row["e"];
            int sp = row["sp"];

            if(i < 8){
                thermostatSchedule.weekSched[dow].daySched[i].start=start; //0am
                thermostatSchedule.weekSched[dow].daySched[i].end=end; //6am, hours are * 100
                thermostatSchedule.weekSched[dow].daySched[i].setpoint=sp; //10.0*C
                thermostatSchedule.weekSched[dow].daySched[i].active=1;
                i++;
            }

        }
    }
    return true;
}
bool Thermostat::saveConfiguration(int target){

    // TODO: only update in case of changes to save FLASH

    if(target == SCHEDULE_CONFIG){

        DynamicJsonBuffer jsonBufferThermostatSchedule;
        JsonObject& jsonThermostatSchedule = jsonBufferThermostatSchedule.createObject();

        for(int dow=0; dow<7; dow++) {
            JsonArray& jsonDow = jsonThermostatSchedule.createNestedArray(String(dow));

            for(int sched=0; sched<8 && thermostatSchedule.weekSched[dow].daySched[sched].active==1; sched++) {
                JsonObject& zone = jsonDow.createNestedObject();
                zone["s"] = thermostatSchedule.weekSched[dow].daySched[sched].start;
                zone["e"] = thermostatSchedule.weekSched[dow].daySched[sched].end;
                zone["sp"] = thermostatSchedule.weekSched[dow].daySched[sched].setpoint;
            }
        }
        File scheduleFile = SPIFFS.open("/schedule.json", "w");
        jsonThermostatSchedule.printTo(scheduleFile);
        scheduleFile.close();
        return true;
    }else if(target == THERMOSTAT_CONFIG) {

        StaticJsonBuffer<500> jsonBuffer;
        JsonObject &thermoConfig = jsonBuffer.createObject();

        thermoConfig["state"] = getState();
        thermoConfig["manualsetpoint"] = getManuelSetPoint();
        thermoConfig["mode"] = getMode();
        thermoConfig["hysteresLow"] = getHysteresisLow();
        thermoConfig["hysteresHigh"] = getHysteresisHigh();
        thermoConfig["selectedSensor"] = getSelectedSensor();

        File thermostatFile = SPIFFS.open("/thermostat.json", "w");
        thermoConfig.printTo(thermostatFile);
        thermostatFile.close();
        return true;

    }
    return false;
}

int Thermostat::getHysteresisLow(){
    return thermostatHysteresisLow;
}
int Thermostat::getHysteresisHigh(){
    return thermostatHysteresisHigh;
}




