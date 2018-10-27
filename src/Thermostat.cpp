//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "Thermostat.h"

// NOTE: this code is adapted from Martin Harizanov project (https://github.com/mharizanov/ESP8266_Relay_Board)

// TODO remove that
#define IS_LEAP(year) (year%4 == 0)
#define SEC_IN_NON_LEAP (86400*365)
#define SEC_IN_LEAP (86400*366)
#define SEC_IN_YEAR(year) (IS_LEAP(year) ? SEC_IN_LEAP : SEC_IN_NON_LEAP)

char buf[30];

unsigned char calendar [] = {31, 28, 31, 30,31, 30, 31, 31,30, 31, 30, 31};
unsigned char calendar_leap [] = {31, 29, 31, 30,31, 30, 31, 31,30, 31, 30, 31};

unsigned char *get_calendar(int year) {
    return IS_LEAP(year) ? calendar_leap : calendar;
}

int get_year(unsigned long *t) {
    int year=1970;
    while(*t>SEC_IN_YEAR(year)) {
        *t-=SEC_IN_YEAR(year);
        year++;
    }
    return year;
}

int get_month(unsigned long *t, int year) {
    unsigned char *cal = get_calendar(year);
    int i=0;
    while(*t > cal[i]*86400) {
        *t-=cal[i]*86400;
        i++;
    }
    return i+1;
}

static int wd(int year, int month, int day) {
    size_t JND =                                                     \
          day                                                      \
        + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5) \
        + (365 * (year + 4800 - ((14 - month) / 12)))              \
        + ((year + 4800 - ((14 - month) / 12)) / 4)                \
        - ((year + 4800 - ((14 - month) / 12)) / 100)              \
        + ((year + 4800 - ((14 - month) / 12)) / 400)              \
        - 32045;
    return (int)JND % 7;
}

Thermostat::Thermostat(NTPClient &ntpClient,Sensors &sensors, Relay &relay)
{
    this->relay = &relay;
    this->sensors = &sensors;
    this->ntpClient = &ntpClient;
    turnOff();
    this->thermostatMode = THERMOSTAT_MANUAL;
    // TODO load
    thermostatManualsetpoint=2100;
    lastUpdate=millis();





}
Thermostat::~Thermostat() {


}

void Thermostat::setManualTemperature(int temp) {
    thermostatManualsetpoint = temp;
    Serial.print("ManualSetPoint");
    Serial.println(thermostatManualsetpoint);
}

int Thermostat::getThermostatMode(){
    return thermostatMode;
}

void Thermostat::setMode(int mode){

    if(mode >= THERMOSTAT_MANUAL && mode <= THEMOSTAT_IA){
        thermostatMode = mode;
        Serial.print("ThermostatMode=");
        Serial.println(mode);
    }

}
int Thermostat::getThermostatState(){
    return thermostatState;
}

int Thermostat::getThermostatManuelSetPoint(){
    return thermostatManualsetpoint;
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
        // TODO
        int Treading=this->sensors->readSensor(0).temperature;

        if(thermostatState == THERMOSTAT_OFF) {
            Serial.println("Thermostat switched off, abandoning routine.");
        } else if(thermostatMode == THERMOSTAT_MANUAL) {
            handle(Treading, thermostatManualsetpoint);
        }else if(thermostatMode ==THERMOSTAT_SCHEDULE) {

            unsigned long epoch = ntpClient->getEpochTime();
            // TODO
            //timeClient->getHours();
            int year=get_year(&epoch);
            int month=get_month(&epoch,year);
            int day=day=1+(epoch/86400);
            int dow=wd(year,month,day);
            epoch=epoch%86400;
            unsigned int hour=epoch/3600;
            epoch%=3600;
            unsigned int min=epoch/60;
            int minadj = (min*100/60);
            int currtime = hour*100+minadj;

            for(int sched=0; sched<8 && thermostatSchedule.weekSched[dow].daySched[sched].active==1; sched++) {
                if(currtime >= thermostatSchedule.weekSched[dow].daySched[sched].start && currtime < thermostatSchedule.weekSched[dow].daySched[sched].end) {

                    Serial.printf("Current schedule (%d) setpoint is: %d\n",sched,thermostatSchedule.weekSched[dow].daySched[sched].setpoint);
                    handle(Treading, thermostatSchedule.weekSched[dow].daySched[sched].setpoint);
                }
            }

        }else if(thermostatMode ==THEMOSTAT_IA) {

            // TODO
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

    File configFile = SPIFFS.open("/schedule.json", "r");
    if (!configFile) {
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 1500) {
        Serial.printf("Schedule Config file size is too large %f\n ", size);
        return false;
    }
    Serial.println(size);

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);
    DynamicJsonBuffer jsonBuffer;
    JsonObject& config = jsonBuffer.parseObject(buf.get());

    if (!config.success()) {
        Serial.println("Failed to parse config file");
        return false;
    }


    JsonArray& days = config["schedule"].asArray();
    Serial.print(days.size());
    for (String day : days) {
        Serial.println(day);
    }
}
bool Thermostat::saveConfiguration(){

}
