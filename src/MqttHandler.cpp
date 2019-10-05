//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "MqttHandler.h"

#define TOPIC_THERMOSTAT_MODE "home/thermostat/mode"
#define TOPIC_THERMOSTAT_RELAY "home/thermostat/relay"
#define TOPIC_THERMOSTAT_MANUAL_SET_POINT "home/thermostat/manualsetpoint"

MqttHandler::MqttHandler(Sensors &sensors,WifiHandler &_wifiHandler, Thermostat &thermostat){
    this->wifiHandler = &_wifiHandler;
    this->thermostat = &thermostat;
    this->sensors = &sensors;
    this->client = NULL;
    timeRetry=millis();
}

MqttHandler::~MqttHandler(){
    if (client != NULL)
        delete  client;
}
boolean MqttHandler::publish(const char* topic, const char* payload){
    return client->publish(topic, payload);
}

void MqttHandler::reconnect() {

    if (!client->connected() &&  (millis() > timeRetry + 5000)) {
        Serial.print(F("Attempting MQTT connection... "));
        Serial.print(this->mqttServer);
        Serial.print(F(" "));
        Serial.print(this->mqttUser);
        Serial.print(F(" "));
//        Serial.print(this->mqttPassword);
//        Serial.print(" ");
        Serial.println(this->mqttPort);
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client->connect(clientId.c_str(), mqttUser,mqttPassword)) {
            Serial.println(F("connected"));

            client->subscribe(TOPIC_THERMOSTAT_MODE);
            client->subscribe(TOPIC_THERMOSTAT_RELAY);
            client->subscribe(TOPIC_THERMOSTAT_MANUAL_SET_POINT);

            client->subscribe("emon/#");
            counterRetry=0;

        } else {
            Serial.print(F("failed, rc="));
            Serial.print(client->state());
            Serial.println(F(" try again in 5 seconds"));
            timeRetry = millis();
            counterRetry++;
            if(counterRetry > 100){
                Serial.println(F("Reset.."));
                ESP.restart();
            }
        }
    }

}

void MqttHandler::setup(const char* mqttServer,const char *mqttUser,const char *mqttPassword,int mqttPort) {
    this->mqttServer = mqttServer;
    this->mqttUser = mqttUser;
    this->mqttPassword = mqttPassword;
    this->mqttPort = mqttPort;
    // check if null
    client = new PubSubClient( this->wifiHandler->getClient());
    client->setServer(this->mqttServer, this->mqttPort);
    client->setCallback(std::bind(&MqttHandler::callback, this,
                                  std::placeholders::_1,
                                  std::placeholders::_2,
                                  std::placeholders::_3));
}


void MqttHandler::update() {
    if (!client->connected()) {
        reconnect();
    }
    client->loop();
}

void MqttHandler::callback(char* topic, byte* payload, unsigned int length) {
    char message_buff[10];
    // Serial.print("Message arrived [");
//    Serial.print(topic);
    //  Serial.print("] ");
    for (int i = 0; i < (int)length; i++) {
        //  Serial.print((char)payload[i]);
        message_buff[i] = payload[i];
    }
    //   Serial.println();
    message_buff[length] = '\0';
    String data = String(message_buff);

    int nodeId = 0;
    char sensorId[32];

    if (sscanf(topic, "emon/emonth%d/%s", &nodeId, sensorId) == 2) {
        //NOTE: If no valid conversion could be performed because the String doesn’t start with a digit, a zero is returned. Data type: float.
        //Serial.printf("nodeID %d (%s) %s \r\n", nodeId, sensorId, message_buff);

        if (strcmp(sensorId, "temperature") == 0) {
            sensors->updateValue(nodeId, SENSOR_TYPE_TEMPERATURE, (int) (data.toFloat() * 100));
        } else if (strcmp(sensorId, "humidity") == 0) {
            sensors->updateValue(nodeId, SENSOR_TYPE_HUMIDITY, (int) (data.toFloat() * 100));
        } else if (strcmp(sensorId, "battery") == 0) {
            sensors->updateValue(nodeId, SENSOR_TYPE_VOLTAGE, (int) (data.toFloat() * 100));
        } else if (strcmp(sensorId, "rssi") == 0) {

            sensors->updateValue(nodeId, SENSOR_TYPE_RSSI, (int) (data.toInt()));
        } else {
            Serial.printf("The Sensor=%s is ignored \r\n", topic);
        }

    } else if (sscanf(topic, "emon/emontx%d/%s", &nodeId, sensorId) == 2){
         if (strcmp(sensorId, "temp1") == 0) {
            sensors->updateValue(nodeId, SENSOR_TYPE_TEMPERATURE, (int) (data.toFloat() * 100));
        } else if (strcmp(sensorId, "rssi") == 0) {
            sensors->updateValue(nodeId, SENSOR_TYPE_RSSI, (int) (data.toInt()));
        }else if (strcmp(sensorId, "power1") == 0) {
            sensors->updateValue(nodeId, SENSOR_TYPE_POWER, (int) (data.toInt()));
        }else if (strcmp(sensorId, "battery") == 0) {
            sensors->updateValue(nodeId, SENSOR_TYPE_VOLTAGE, (int) (data.toFloat() * 100));
        }
    }
    else if (strcmp(topic,TOPIC_THERMOSTAT_MODE)==0)
    {
        if(data.equals("manual")){
            thermostat->setMode(THERMOSTAT_MANUAL);
        }else if(data.equals("schedule")){
            thermostat->setMode(THERMOSTAT_SCHEDULE);
        }
    }else if (strcmp(topic,TOPIC_THERMOSTAT_RELAY)==0)
    {
        if(data.equals("on")){
            thermostat->turnOn();
        }else if(data.equals("off")){
            thermostat->turnOff();
        }
    }else if(strcmp(topic, TOPIC_THERMOSTAT_MANUAL_SET_POINT) ==0){
        thermostat->setManualSetPoint(data.toInt());
    }else {

        Serial.printf("The topic=%s is ignored \r\n", topic);
    }

}


