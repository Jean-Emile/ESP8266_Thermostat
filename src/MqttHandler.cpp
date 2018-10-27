//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "MqttHandler.h"

#define TOPIC_THERMOSTAT_MODE "home/thermostat/mode"
#define TOPIC_THERMOSTAT_STATE "home/thermostat/state"
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
void MqttHandler::reconnect() {

    if (!client->connected() &&  (millis() > timeRetry + 5000)) {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client->connect(clientId.c_str(), mqttUser,mqttPassword)) {
            Serial.println("connected");

            client->subscribe(TOPIC_THERMOSTAT_MODE);
            client->subscribe(TOPIC_THERMOSTAT_STATE);
            client->subscribe(TOPIC_THERMOSTAT_MANUAL_SET_POINT);

            //client->subscribe("emon/#");
            client->subscribe("emon/emonth6/temperature");
            client->subscribe("emon/emonth6/humidity");

        } else {
            Serial.print("failed, rc=");
            Serial.print(client->state());
            Serial.println(" try again in 5 seconds");
            timeRetry = millis();
        }
    }

}

void MqttHandler::setup(const char* mqttServer,const char *mqttUser,const char *mqttPassword,int mqttPort) {
    Serial.print(mqttServer);
    Serial.print(" ");
    Serial.print(mqttUser);
    Serial.print(" ");
    Serial.println(mqttPort);
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
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
        message_buff[i] = payload[i];
    }
    Serial.println();
    message_buff[length] = '\0';
    String data = String(message_buff);

    if (strcmp(topic,"emon/emonth6/temperature")==0){
        sensors->updateTemperature(0,(int)(data.toFloat()*100));
    }else if (strcmp(topic,"emon/emonth6/humidity") ==0) {
        sensors->updateHumidity(0,(int)(data.toFloat()*100));
    } else if (strcmp(topic,TOPIC_THERMOSTAT_MODE)==0)
    {
        if(data.equals("manual")){
            thermostat->setMode(THERMOSTAT_MANUAL);
        }else if(data.equals("schedule")){
            thermostat->setMode(THERMOSTAT_SCHEDULE);
        }
    }else if (strcmp(topic,TOPIC_THERMOSTAT_STATE)==0)
    {
        if(data.equals("on")){
            thermostat->turnOn();
        }else if(data.equals("off")){
            thermostat->turnOff();
        }
    }else if(strcmp(topic, TOPIC_THERMOSTAT_MANUAL_SET_POINT) ==0){
        //TODO check its a INT
        thermostat->setManualTemperature(data.toInt());
    }else {

        Serial.printf("The topic=%s is ignored", topic);
    }

}


