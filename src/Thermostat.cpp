//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "Thermostat.h"

Thermostat::Thermostat(NTPClient &ntpClient, Sensors &sensors, Relay &relay)
{
    this->relay = &relay;
    this->sensors = &sensors;
    this->ntpClient = &ntpClient;
    turnOff();
    this->thermostatMode = THERMOSTAT_MANUAL;
    thermostatManualsetpoint = 2100;
    selectedSensor = 0;
    lastUpdate = millis();
}
Thermostat::~Thermostat()
{
}

void Thermostat::setManualSetPoint(int temp)
{
    thermostatManualsetpoint = temp;
    Serial.print(F("ManualSetPoint"));
    Serial.println(thermostatManualsetpoint);
}

int Thermostat::getMode()
{
    return thermostatMode;
}

void Thermostat::setMode(int mode)
{

    if (mode >= THERMOSTAT_MANUAL && mode <= THEMOSTAT_IA)
    {
        thermostatMode = mode;
        Serial.print(F("ThermostatMode="));
        Serial.println(mode);
    }
}
int Thermostat::getState()
{
    return thermostatState;
}

int Thermostat::getManuelSetPoint()
{
    return thermostatManualsetpoint;
}
void Thermostat::setHysteresisLow(int temp)
{
    this->thermostatHysteresisLow = temp;
}
void Thermostat::setHysteresisHigh(int temp)
{
    this->thermostatHysteresisHigh = temp;
}

int Thermostat::getSelectedSensor()
{
    return this->selectedSensor;
}
void Thermostat::setSelectedSensor(int id)
{
    if (id >= 0 && id <= MAX_SENSORS)
    {
        this->selectedSensor = id;
    }
}

void Thermostat::turnOn()
{
    thermostatState = THERMOSTAT_ON;
    Serial.println(F("ON"));
}
void Thermostat::turnOff()
{
    relay->turnOff();
    thermostatState = THERMOSTAT_OFF;
    Serial.println(F("OFF"));
}

void Thermostat::update()
{
    if ((millis() > lastUpdate + 20000))
    {
        printf("Free Heap: %d Free Stack %d\n", ESP.getFreeHeap(), ESP.getFreeContStack());
        lastUpdate = millis();
        Serial.println(ntpClient->getFormattedTime());

        unsigned long epoch = ntpClient->getEpochTime();
        int dow = ntpClient->getDay();
        unsigned int hour = ntpClient->getHours();
        int min = ntpClient->getMinutes();
        int minadj = (min * 100 / 60);
        int currtime = hour * 100 + minadj;
        Serial.printf("DOW=%d currtime=%d \n", dow, currtime);

        if (this->sensors->readSensor(getSelectedSensor()).active)
        {

            int Treading = this->sensors->readSensor(getSelectedSensor()).temperature;

            if (thermostatState == THERMOSTAT_OFF)
            {
                Serial.println(F("Thermostat switched off, abandoning routine."));
            }
            else
            {

                if (epoch - this->sensors->readSensor(getSelectedSensor()).lastUpdate >= 6000)
                {
                        Serial.println(F("Stale sensor, abandoning routine."));
                    /*
                    Gsender *gsender = Gsender::Instance(); // Getting pointer to class instance

                    if (gsender->Subject("Stale Measurement!")->Send("", "The selected sensor value is outdated"))
                    {
                        Serial.println("Message send.");
                    }
                    else
                    {
                        Serial.print("Error sending message: ");
                        Serial.println(gsender->getError());
                    } 
                    */
                    this->relay->turnOff();
                }else {
                    if (thermostatMode == THERMOSTAT_MANUAL)
                    {

                            handle(Treading, thermostatManualsetpoint);
                    } else if (thermostatMode == THERMOSTAT_SCHEDULE){

                            for (int sched = 0; sched < 8 && thermostatSchedule.weekSched[dow].daySched[sched].active == 1; sched++)
                            {

                                if (currtime >= thermostatSchedule.weekSched[dow].daySched[sched].start && currtime < thermostatSchedule.weekSched[dow].daySched[sched].end)
                                {

                                    Serial.printf("Current schedule (%d) setpoint is: %d  start=%d end=%d\n", sched, thermostatSchedule.weekSched[dow].daySched[sched].setpoint, thermostatSchedule.weekSched[dow].daySched[sched].start, thermostatSchedule.weekSched[dow].daySched[sched].end);
                                    handle(Treading, thermostatSchedule.weekSched[dow].daySched[sched].setpoint);
                                }
                            }
                    }else if (thermostatMode == THEMOSTAT_IA)
                    {

                            // TODO
                    }

                }

            
            }
        }
        else
        {
            Serial.println(F("Thermostat switched off, no valid Temperature."));
            /*
             Gsender *gsender = Gsender::Instance(); // Getting pointer to class instance

            if (gsender->Subject("Stale Measurement!")->Send("", "The selected sensor value is outdated"))
             {
                 Serial.println("Message send.");
             }
             else
             {
                 Serial.print("Error sending message: ");
                 Serial.println(gsender->getError());
             } */
             this->relay->turnOff();
        }
    }
}

void Thermostat::handle(int current_t, int setpoint)
{
    if (current_t < setpoint - thermostatHysteresisLow)
    {
        Serial.printf("Current reading (%d) is less than the setpoint (%d)\n", current_t, setpoint);
        this->relay->turnOn();
    }
    else if (current_t > setpoint + thermostatHysteresisHigh)
    {
        Serial.printf("Current reading (%d) is more than the setpoint (%d).\n", current_t, setpoint);
        this->relay->turnOff();
    }
}

bool Thermostat::validateSchedule(JsonArray &scheduleRoot)
{
    if (scheduleRoot.size() < MAXIMUM_SCHEDULE_PER_DAY)
    {
        int lastEnd = 0;
        for (auto row : scheduleRoot)
        {

            if (row.containsKey("s") && row.containsKey("e") && row.containsKey("sp"))
            {

                if (row["s"].is<int>() && row["e"].is<int>() && row["sp"].is<int>())
                {
                    int start = row["s"];
                    int end = row["e"];
                    int sp = row["sp"];

                    if ((sp < 0 && sp > 50) || start < 0 || start > 2400 || end < 0 || end > 2400)
                    {
                        return false;
                    }
                    if (start != lastEnd)
                    {
                        return false;
                    }

                    lastEnd = end;
                }
                else
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
    return true;
}

bool Thermostat::loadConfiguration()
{

    File configThermo = SPIFFS.open("/thermostat.json", "r");
    if (!configThermo)
    {
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configThermo.size();
    if (size > 200)
    {
        Serial.printf("Config file size is too large %d\n ", size);
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> bufConfig(new char[size]);
    configThermo.readBytes(bufConfig.get(), size);

    DynamicJsonDocument config(1024);
    DeserializationError error = deserializeJson(config, bufConfig.get());
    if (error)
    {
        Serial.print(F("deserializeJson() failed with code "));
        Serial.println(error.c_str());
        configThermo.close();
        return false;
    }

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

    if (state == 1)
    {
        turnOn();
    }
    else
    {
        turnOff();
    }

    if (relay == 1)
    {
        this->relay->turnOn();
    }
    else
    {
        this->relay->turnOff();
    }

    for (int dow = 0; dow < 7; dow++)
    {
        for (int sched = 0; sched < MAXIMUM_SCHEDULE_PER_DAY; sched++)
        {
            char fileName [8];
            sprintf (fileName, "/%d_%d.bin", dow, sched);
            File scheduleFile = SPIFFS.open(fileName, "r");
            if (scheduleFile)
            {
                scheduleFile.read((byte *)&thermostatSchedule.weekSched[dow].daySched[sched], sizeof(dayScheduleElement));
                scheduleFile.close();
            }else{
                printf("Schedule not found %s\n",fileName);
            }
        }
    }
    config.clear();
    configThermo.close();

    return true;
}
bool Thermostat::saveSchedule(int dow, int sched)
{
    // Saving a specific dow/sched
    char fileName [8];
    sprintf (fileName, "/%d_%d.bin", dow, sched);
    File scheduleFile = SPIFFS.open(fileName, "w");
    printf("Saving dow=%d sched=%d => %s\r\n",dow,sched, fileName);
    scheduleFile.write((byte *)&thermostatSchedule.weekSched[dow].daySched[sched], sizeof(dayScheduleElement));
    scheduleFile.flush();
    scheduleFile.close();
    return true;
}
bool Thermostat::saveDaySchedule(int dow)
{
    for (int sched = 0; sched < MAXIMUM_SCHEDULE_PER_DAY; sched++)
    {
        saveSchedule(dow, sched);
    }
    return true;
}
bool Thermostat::saveConfiguration(int target)
{


    if (target == SCHEDULE_CONFIG)
    {
        // Saving the whole scheduling 
        for (int dow = 0; dow < 7; dow++)
        {
            saveDaySchedule(dow);
        }

        return true;
    }
    else if (target == THERMOSTAT_CONFIG)
    {

        DynamicJsonDocument thermoConfig(500);

        thermoConfig["state"] = getState();
        thermoConfig["manualsetpoint"] = getManuelSetPoint();
        thermoConfig["mode"] = getMode();
        thermoConfig["hysteresLow"] = getHysteresisLow();
        thermoConfig["hysteresHigh"] = getHysteresisHigh();
        thermoConfig["selectedSensor"] = getSelectedSensor();

        File thermostatFile = SPIFFS.open("/thermostat.json", "w");
        serializeJson(thermoConfig, thermostatFile);
        thermostatFile.close();
        thermoConfig.clear();

        return true;
    }
    return false;
}

int Thermostat::getHysteresisLow()
{
    return thermostatHysteresisLow;
}
int Thermostat::getHysteresisHigh()
{
    return thermostatHysteresisHigh;
}
