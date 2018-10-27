//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H
#define ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H


#include <WiFiUdp.h>
#include <NTPClient.h> // get it here https://github.com/arduino-libraries/NTPClient
#include <FS.h>   // Include the SPIFFS library
#include <ArduinoJson.h>

#include "Relay.h"
#include "Sensors.h"

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

typedef struct {
    uint16_t start;
    uint16_t end;
    uint16_t setpoint;  // Degrees C in * 100 i.e. 2350=23.5*C
    uint16_t active; 	 // pad to 4 byte boundary
} dayScheduleElement;

typedef struct {
    dayScheduleElement daySched[8]; // Max 8 schedules per day
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
    void setManualTemperature(int temp);
    int getThermostatMode();
    int getThermostatState();
    int getThermostatManuelSetPoint();
    void turnOn();
    void turnOff();
    void setMode(int mode);
    bool loadConfiguration();
    bool saveConfiguration();

    WeekSchedule thermostatSchedule;
private:
    void handle(int current_t, int setpoint);

    NTPClient *ntpClient;
    Relay *relay;
    Sensors *sensors;
    int thermostatMode;
    int thermostatState;

    int thermostatManualsetpoint;
    int thermostatHysteresisLow;
    int thermostatHysteresisHigh;
    double lastUpdate;
    int temperatures[10];

};


#endif //ESP8266_RELAY_BOARD_ARDUINO_THERMOSTAT_H
