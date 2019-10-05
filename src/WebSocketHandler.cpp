//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "WebSocketHandler.h"

WebSocketHandler::WebSocketHandler(Sensors &sensors, SpiffsHandler &spiffsHandler, Thermostat &thermostat, Relay &relay)
{
    this->spiffsHandler = &spiffsHandler;
    this->relay = &relay;
    this->sensors = &sensors;
    this->thermostat = &thermostat;
    webSocket = new WebSocketsServer(81); // create a websocket server on port 81
}

WebSocketHandler::~WebSocketHandler()
{
    webSocket->close();
    delete webSocket;
}

void WebSocketHandler::setup()
{
    Serial.println(F("[INFO] WebSocketHandler setup()"));
    // TODO load/save config
    //  webSocket->setAuthorization("esp8266", "Led*ZInqsdf9");
    webSocket->begin(); // start the websocket server
    webSocket->onEvent(std::bind(&WebSocketHandler::onWsEvent, this,
                                 std::placeholders::_1,
                                 std::placeholders::_2,
                                 std::placeholders::_3,
                                 std::placeholders::_4)); // if there's an incomming websocket message, go to function 'webSocketEvent'

    Serial.println(F("[INF0] WebSocket server started."));
    //webSocket->setReconnectInterval(5000);
   // webSocket->enableHeartbeat(15000, 3000, 2);
}

void WebSocketHandler::update()
{
    webSocket->loop();
}

void WebSocketHandler::onWsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t lenght)
{ // When a WebSocket message is received
    switch (type)
    {
    case WStype_DISCONNECTED: // if the websocket is disconnected
        Serial.printf("[%u] Disconnected!\n", num);
        break;
    case WStype_CONNECTED:
    { // if a new websocket connection is established
        IPAddress ip = webSocket->remoteIP(num);
    }
    break;
    case WStype_TEXT: // if new text data is received
        // Serial.printf("[%u] get Text: %s\n", num, payload);

        DynamicJsonDocument ev_cmd(1024);
        printf("Free Heap: %d Free Stack %d\n", ESP.getFreeHeap(), ESP.getFreeContStack());

        auto error = deserializeJson(ev_cmd, &payload[0]);

        if (error)
        {
            Serial.println(F("parseObject() failed"));
            ev_cmd.clear();
        }
        else
        {
            String target = ev_cmd["event"];

            if (target.equals("relay"))
            {

                if (ev_cmd.containsKey("state") && ev_cmd["state"].is<bool>())
                {
                    bool state = ev_cmd["state"];
                    if (state == true)
                    {
                        relay->turnOn();
                    }
                    else if (state == false)
                    {
                        relay->turnOff();
                    }
                    thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                }
            }
            else if (target.equals("restart"))
            {
                ESP.restart();
            }
            else if (target.equals("freememory"))
            {
                DynamicJsonDocument free_memory(64);
                free_memory["event"] = "freememory";
                free_memory["freeheap"] = ESP.getFreeHeap();
                free_memory["freestack"] =  ESP.getFreeContStack();
                String retJson;
                serializeJson(free_memory, retJson);
                webSocket->sendTXT(num, retJson);
                free_memory.clear();
            }
            else if (target.equals("time"))
            {
                DynamicJsonDocument timeJson(1024);
                timeJson["event"] = "time";
                timeJson["currtime"] = this->thermostat->ntpClient->getFormattedTime();
                String retJson;
                serializeJson(timeJson, retJson);
                webSocket->sendTXT(num, retJson);
                timeJson.clear();
            }
            else if (target.equals("do_wifi_scan"))
            {
                DynamicJsonDocument wifiroot(1024);
                wifiroot["event"] = "wifi_scan";
                byte n = WiFi.scanNetworks();
                JsonArray networks = wifiroot.createNestedArray("networks");

                for (int i = 0; i < n; i++)
                {
                    JsonObject network = networks.createNestedObject();
                    network["ssid"] = WiFi.SSID(i);
                    network["encryption"] = WiFi.encryptionType(i);
                    network["rssi"] = WiFi.RSSI(i);
                    network["bssid"] = WiFi.BSSIDstr(i);
                    network["channel"] = WiFi.channel(i);
                    network["isHidden"] = WiFi.isHidden(i);
                }

                String retJson;
                serializeJson(wifiroot, retJson);
                webSocket->sendTXT(num, retJson);
                wifiroot.clear();
            }
            else if (target.equals("thermostat_state"))
            {

                if (ev_cmd.containsKey("state") && ev_cmd["state"].is<int>())
                {
                    bool state = ev_cmd["state"];
                    if (state == true)
                    {
                        thermostat->turnOn();
                    }
                    else if (state == false)
                    {
                        thermostat->turnOff();
                    }
                    thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                }
            }
            else if (target.equals("thermostat_manualsetpoint"))
            {

                if (ev_cmd.containsKey("temperature") && ev_cmd["temperature"].is<int>())
                {
                    int temperature = ev_cmd["temperature"];
                    thermostat->setManualSetPoint(temperature);

                    thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                }
            }
            else if (target.equals("thermostat_schedule"))
            {

                if (ev_cmd.containsKey("day") && ev_cmd.containsKey("schedule") && ev_cmd["day"].is<int>())
                {
                    int dow = ev_cmd["day"];
                    if (dow >= 0 && dow <= 7)
                    {

                        JsonArray scheduleRoot = ev_cmd["schedule"];

                        if (scheduleRoot.size() < MAXIMUM_SCHEDULE_PER_DAY)
                        {

                            if (thermostat->validateSchedule(scheduleRoot))
                            {
                                int i = 0;
                                for (auto row : scheduleRoot)
                                {

                                    if (row.containsKey("s") && row.containsKey("e") && row.containsKey("sp"))
                                    {

                                        if (row["s"].is<int>() && row["e"].is<int>() && row["sp"].is<int>())
                                        {

                                            int start = row["s"];
                                            int end = row["e"];
                                            int sp = row["sp"];

                                            if (i < MAXIMUM_SCHEDULE_PER_DAY)
                                            {

                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].start = start; //0am
                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].end = end;     //6am, hours are * 100
                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].setpoint = sp; //10.0*C
                                                thermostat->thermostatSchedule.weekSched[dow].daySched[i].active = 1;
                                                i++;
                                            }
                                        }
                                    }
                                }
                                if (i < MAXIMUM_SCHEDULE_PER_DAY)
                                {
                                    thermostat->thermostatSchedule.weekSched[dow].daySched[i].active = 0; //mark the next schedule as inactive
                                }
                                thermostat->saveDaySchedule(dow);
                            }
                            else
                            {
                                Serial.println(F("[WARNING] the schedule is not compliant"));
                            }
                        }
                    }
                }
            }
            else if (target.equals("thermostat_mode"))
            {

                if (ev_cmd.containsKey("mode") && ev_cmd["mode"].is<int>())
                {
                    int mode = ev_cmd["mode"];
                    thermostat->setMode(mode);
                    thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                }
            }
            else if (target.equals("get_thermostat_states"))
            {
                // NOTE the size have to aligned withMAX_SENSORS
                DynamicJsonDocument root_thermostat_states(1024); // https://arduinojson.org/v5/assistant/
                root_thermostat_states["event"] = "thermostat_states";

                JsonArray jsonSensors = root_thermostat_states.createNestedArray("sensors");

                for (int i = 0; i < MAX_SENSORS; i++)
                {
                    if (sensors->readSensor(i).active == true)
                    {
                        JsonObject sensor = jsonSensors.createNestedObject();
                        sensor["id"] = i;
                        sensor["temp"] = sensors->readSensor(i).temperature;
                        sensor["hum"] = sensors->readSensor(i).humidity;
                        sensor["batt"] = sensors->readSensor(i).battery;
                        sensor["rssi"] = sensors->readSensor(i).rssi;
                        sensor["lastUp"] = sensors->readSensor(i).lastUpdate;
                    }
                }

                root_thermostat_states["relayState"] = relay->getState();
                root_thermostat_states["state"] = thermostat->getState();
                root_thermostat_states["manualsetpoint"] = thermostat->getManuelSetPoint();
                root_thermostat_states["mode"] = thermostat->getMode();
                root_thermostat_states["boardtime"] = this->thermostat->ntpClient->getFormattedTime();
                root_thermostat_states["selected"] = thermostat->getSelectedSensor();
                root_thermostat_states["hysteresis_h"] = thermostat->getHysteresisHigh();
                root_thermostat_states["hysteresis_l"] = thermostat->getHysteresisLow();
                root_thermostat_states["free_stack"] = ESP.getFreeContStack();
                String output;
                serializeJson(root_thermostat_states, output);

                webSocket->sendTXT(num, output);
                root_thermostat_states.clear();
   
            }
            else if (target.equals("thermostat_selected"))
            {

                if (ev_cmd.containsKey("sensorid"))
                {
                    int sensorID = ev_cmd["sensorid"];
                    thermostat->setSelectedSensor(sensorID);
                    thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                }
            }
            else if (target.equals("set_hysteresis"))
            {

                if (ev_cmd.containsKey("hysteresis_h") && ev_cmd.containsKey("hysteresis_l"))
                {
                    if (ev_cmd["hysteresis_h"].is<int>() && ev_cmd["hysteresis_l"].is<int>())
                    {
                        int hysteresis_h = ev_cmd["hysteresis_h"];
                        int hysteresis_l = ev_cmd["hysteresis_l"];

                        thermostat->setHysteresisHigh(hysteresis_h);
                        thermostat->setHysteresisLow(hysteresis_l);
                        thermostat->saveConfiguration(THERMOSTAT_CONFIG);
                    }
                }
            }
            else if (target.equals("get_schedule"))
            {
                // thermostat_schedule
                DynamicJsonDocument jsonThermostatSchedule(5000);
                jsonThermostatSchedule["event"] = "thermostat_schedule";

                for (int dow = 0; dow < 7; dow++)
                {
                    // TODO: remove this array   
                    const char *days[7] = {"sun","mon", "tue", "wed", "thu", "fri", "sat"};

                    JsonArray jsonDow = jsonThermostatSchedule.createNestedArray(days[dow]);

                    for (int sched = 0; sched < MAXIMUM_SCHEDULE_PER_DAY && thermostat->thermostatSchedule.weekSched[dow].daySched[sched].active == 1; sched++)
                    {
                        JsonObject zone = jsonDow.createNestedObject();
                        zone["s"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].start;
                        zone["e"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].end;
                        zone["sp"] = thermostat->thermostatSchedule.weekSched[dow].daySched[sched].setpoint;
                    }
                }
                String output2;
                serializeJson(jsonThermostatSchedule, output2);
                webSocket->sendTXT(num, output2);
                jsonThermostatSchedule.clear();
            }

        }
        ev_cmd.clear();
        break;
    }
}