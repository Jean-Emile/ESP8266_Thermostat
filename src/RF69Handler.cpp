//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "RF69Handler.h"


RF69Handler::RF69Handler(Sensors &sensors,MqttHandler &mqttHandler) {
    this->sensors = &sensors;
    this->mqttHandler = &mqttHandler;

    radio = new RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);
    ackCount=0;
    packetCount = 0;
}

RF69Handler::~RF69Handler() {
    //delete radio;
}

void RF69Handler::setup() {
    Serial.println(F("[INFO] RF69Handler setup()"));
    // Hard Reset the RFM module
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, HIGH);
    delay(100);
    digitalWrite(RFM69_RST, LOW);
    delay(100);
    // Initialize radio
    if (!radio->initialize(FREQUENCY,NODEID,NETWORKID)) {
        Serial.println(F("radio.initialize failed!"));
    }
    if (IS_RFM69HCW) {
        radio->setHighPower(true);    // Only for RFM69HCW & HW!
    }

    radio->encrypt(ENCRYPTKEY);
    radio->promiscuous(false);
    //radio->setPowerLevel(10);

    pinMode(LED, OUTPUT);
    Serial.print(F("\nFreq at "));
    Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
    Serial.println(F(" MHz"));
    Serial.print(F("Network ")); Serial.print(NETWORKID);
    Serial.print(F(" Node ")); Serial.println(NODEID); Serial.println();
}

void RF69Handler::update() {


    if (radio->receiveDone())
    {
        //  Serial.print('[');Serial.print(radio->SENDERID, DEC);Serial.print("] ");
        //    Serial.print(" [RX_RSSI:");Serial.print(radio->readRSSI());Serial.print("]");
//        if (promiscuousMode)
//        {
//            Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
//        }
        //    Serial.print(radio->DATALEN);
        //     Serial.print(" ");
        //   Serial.println(sizeof(Payload));

        if (radio->DATALEN != sizeof(Payload))
            Serial.print(F("Invalid payload received, not matching Payload struct!"));
        else
        {
            theData = *(Payload*)radio->DATA; //assume radio.DATA actually contains our struct and not something else
            Serial.print(F(" nodeId="));
            Serial.print(theData.nodeId);
            // Serial.print(" uptime=");
            //Serial.print(theData.uptime);
            Serial.print(F(" temp="));
            Serial.print(theData.temp);
            Serial.print(F(" hum="));
            Serial.print(theData.humidity);
            Serial.print(F(" bat="));
            Serial.print(theData.battery);
            Serial.print(F(" pulse="));
            Serial.print(theData.pulsecount);

            sensors->updateValue(theData.nodeId,SENSOR_TYPE_TEMPERATURE,(int)theData.temp);
            sensors->updateValue(theData.nodeId,SENSOR_TYPE_HUMIDITY,(int)theData.humidity);
            sensors->updateValue(theData.nodeId,SENSOR_TYPE_VOLTAGE, (int)theData.battery);
            sensors->updateValue(theData.nodeId,SENSOR_TYPE_RSSI,(int)radio->readRSSI());

            // TODO mqtt adapater
            char str_temp[6];
            char str_topic[128];

            dtostrf(theData.temp/100.0, 4, 2, str_temp);
            sprintf(str_topic,"emon/emonth%d/%s",(int)theData.nodeId,"temperature");
            mqttHandler->publish(str_topic, str_temp);

            dtostrf(theData.humidity/100.0, 4, 2, str_temp);
            sprintf(str_topic,"emon/emonth%d/%s",(int)theData.nodeId,"humidity");
            mqttHandler->publish(str_topic, str_temp);

            dtostrf(theData.battery/100.0, 4, 2, str_temp);
            sprintf(str_topic,"emon/emonth%d/%s",(int)theData.nodeId,"battery");
            mqttHandler->publish(str_topic, str_temp);

            sprintf("%d",str_temp, (const char*)radio->readRSSI());
            sprintf(str_topic,"emon/emonth%d/%s",(int)theData.nodeId,"rssi");
            mqttHandler->publish(str_topic, str_temp);

        }

        if (radio->ACKRequested())
        {
            // byte theNodeID = radio->SENDERID;
            radio->sendACK();
            // Serial.print(" - ACK sent.");

            // When a node requests an ACK, respond to the ACK
            // and also send a packet requesting an ACK (every 3rd one only)
            // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
//            if (ackCount++%3==0)
//            {
//                Serial.print(" Pinging node ");
//                Serial.print(theNodeID);
//                Serial.print(" - ACK...");
//                delay(3); //need this when sending right after reception .. ?
//                if (radio->sendWithRetry(theNodeID, "ACK", 8, 0))  // 0 = only 1 attempt, no retries
//                    Serial.print("ok!");
//                else Serial.print("nothing");
//            }
        }
        Serial.println();
        //Blink(LED,3);
    }
}