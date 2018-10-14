//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#include "RF69Handler.h"


RF69Handler::RF69Handler() {
    radio = new RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);
    ackCount=0;
    packetCount = 0;
}

RF69Handler::~RF69Handler() {
    delete radio;
}

void RF69Handler::setup() {

    // Hard Reset the RFM module
    pinMode(RFM69_RST, OUTPUT);
    digitalWrite(RFM69_RST, HIGH);
    delay(100);
    digitalWrite(RFM69_RST, LOW);
    delay(100);

    // Initialize radio
    if (!radio->initialize(FREQUENCY,NODEID,NETWORKID)) {
        Serial.println("radio.initialize failed!");
    }
    if (IS_RFM69HCW) {
        radio->setHighPower(true);    // Only for RFM69HCW & HW!
    }

    radio->encrypt(ENCRYPTKEY);
    radio->promiscuous(false);
    radio->setPowerLevel(10);

    pinMode(LED, OUTPUT);
    Serial.print("\nFreq at ");
    Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
    Serial.println(" MHz");
    Serial.print("Network "); Serial.print(NETWORKID);
    Serial.print(" Node "); Serial.println(NODEID); Serial.println();
}

void RF69Handler::update() {


    if (radio->receiveDone())
    {
        Serial.print('[');Serial.print(radio->SENDERID, DEC);Serial.print("] ");
        Serial.print(" [RX_RSSI:");Serial.print(radio->readRSSI());Serial.print("]");
//        if (promiscuousMode)
//        {
//            Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
//        }
        Serial.print(radio->DATALEN);
        Serial.print(" ");
        Serial.println(sizeof(Payload));

        if (radio->DATALEN != sizeof(Payload))
            Serial.print("Invalid payload received, not matching Payload struct!");
        else
        {
            theData = *(Payload*)radio->DATA; //assume radio.DATA actually contains our struct and not something else
            Serial.print(" nodeId=");
            Serial.print(theData.nodeId);
            // Serial.print(" uptime=");
            //Serial.print(theData.uptime);
            Serial.print(" temp=");
            Serial.print(theData.temp);
            Serial.print(" hum=");
            Serial.print(theData.humidity);
            Serial.print(" bat=");
            Serial.print(theData.battery);
            Serial.print(" pulse=");
            Serial.print(theData.pulsecount);

//            char str_temp[6];
//            dtostrf(theData.temp/10, 4, 2, str_temp);
//            client.publish("emon/emonth99/temperature", str_temp);
        }

        if (radio->ACKRequested())
        {
            byte theNodeID = radio->SENDERID;
            radio->sendACK();
            Serial.print(" - ACK sent.");

            // When a node requests an ACK, respond to the ACK
            // and also send a packet requesting an ACK (every 3rd one only)
            // This way both TX/RX NODE functions are tested on 1 end at the GATEWAY
            if (ackCount++%3==0)
            {
                Serial.print(" Pinging node ");
                Serial.print(theNodeID);
                Serial.print(" - ACK...");
                delay(3); //need this when sending right after reception .. ?
                if (radio->sendWithRetry(theNodeID, "ACK", 8, 0))  // 0 = only 1 attempt, no retries
                    Serial.print("ok!");
                else Serial.print("nothing");
            }
        }
        Serial.println();
        //Blink(LED,3);
    }
}