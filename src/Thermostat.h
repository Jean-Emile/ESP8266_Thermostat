//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H
#define ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H


#include <WiFiUdp.h>
#include <NTPClient.h> // get it here https://github.com/arduino-libraries/NTPClient
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>
#include <Time.h> // get it here https://github.com/PaulStoffregen/Time/tree/v1.5

#include "Relay.h"
#include "Sensors.h"

#define MAXIMUM_SCHEDULE_PER_DAY 8

enum ThermostatMode
{
    THERMOSTAT_MANUAL=0,
    THERMOSTAT_SCHEDULE=1,
    THEMOSTAT_IA=2
};

enum ThermostatState {
    THERMOSTAT_OFF=0,
    THERMOSTAT_ON=1
} ;

enum ThermostatSave {
    SCHEDULE_CONFIG=0,
    THERMOSTAT_CONFIG=1
} ;

typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t setpoint;  // Degrees C in * 100 i.e. 2350=23.5*C
    uint16_t active; 	 // pad to 4 byte boundary
} dayScheduleElement;

typedef struct {
    dayScheduleElement daySched[MAXIMUM_SCHEDULE_PER_DAY]; // Max 8 schedules per day
}  daySchedule;

typedef struct {
    daySchedule weekSched[7]; // 7 days per week
}  WeekSchedule;


class Thermostat
{
public:
    Thermostat(NTPClient &ntpClient,Sensors &sensors, Relay &relay);
    ~Thermostat();

    void update();

    void setHysteresisLow(int temp);
    void setHysteresisHigh(int temp);
    void setManualSetPoint(int temp);

    int getSelectedSensor();
    void setSelectedSensor(int id);

    int getMode();
    int getState();
    int getManuelSetPoint();
    int getHysteresisLow();
    int getHysteresisHigh();

    void turnOn();
    void turnOff();
    void setMode(int mode);
    bool loadConfiguration();
    bool saveConfiguration(int target);
    NTPClient *ntpClient;
    WeekSchedule thermostatSchedule;
private:
    void handle(int current_t, int setpoint);


    Relay *relay;
    Sensors *sensors;
    int thermostatMode;
    int thermostatState;
    int selectedSensor;

    int thermostatManualsetpoint;
    int thermostatHysteresisLow;
    int thermostatHysteresisHigh;
    unsigned long lastUpdate;

};


#endif //ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H
