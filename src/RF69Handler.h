//
// Created by Jean-Emile Dartois (jedartois@gmail.com) on 14/10/18.
//

#ifndef ESP8266_RELAY_BOARD_ARDUINO_RF69HANDLER_H
#define ESP8266_RELAY_BOARD_ARDUINO_RF69HANDLER_H

#include <Arduino.h>

#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <RFM69_ATC.h>     //get it here: https://www.github.com/lowpowerlab/rfm69

#include "Sensors.h"
#include "MqttHandler.h"

typedef struct __attribute__((__packed__)) Payload {                                                    // RFM RF payload datastructure
    uint16_t           nodeId; //store this nodeId
    uint32_t temp;
    uint32_t temp_external;
    uint32_t humidity;
    uint32_t battery;
    unsigned long pulsecount;
};


// ESP8266 rf69
//  GPI016 >==> led4
// D8 (GPIO15) 	NSS
// D7 (GPIO13) 	MOSI
// D6 (GPIO12) 	MISO
// D5 (GPIO14) 	SCK
//D1** (GPIO4) 	DIO0

#define LED 16 // onboard blinky
#define RFM69_CS      15 //SS
#define RFM69_IRQ     4
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ)
#define RFM69_RST     2

#define NODEID        1    //unique for each node on same network
#define NETWORKID     100  //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
//#define FREQUENCY     RF69_915MHZ
#define ENCRYPTKEY    "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!

#define IS_RFM69HCW true // set to 'true' if you are using an RFM69HCW module


class RF69Handler
{
public:
    RF69Handler(Sensors &sensors,MqttHandler &mqttHandler);
    ~RF69Handler();

    void setup();
    void update();

private:
    Payload theData;
    Sensors *sensors;
    byte ackCount;
    uint32_t packetCount;
    RFM69 *radio;
    MqttHandler *mqttHandler;


};



#endif //ESP8266_RELAY_BOARD_ARDUINO_RF69HANDLER_H
