/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   LoraAprs.h
 * Author: ale
 *
 * Created on 19 mars 2022, 18:04
 * TRX SX1278 pinout is in pins_arduino.h file (board environement)
 * /home/ale/.platformio/packages/framework-arduinoespressif32/variants/tbeam
 * // SPI LoRa Radio
 */

#ifndef LORAAPRS_H
#define LORAAPRS_H

#include <LoRa.h>
#include <Arduino.h>

//pro micro
#define LORA_SCK 15      // SX1276 SCK
#define LORA_MISO 14    // SX1276 MISO
#define LORA_MOSI 16    // SX1276 MOSI
#define LORA_CS 10      // SX1276 CS
#define LORA_RST 9     // SX1276 RST
#define LORA_IRQ 7     // SX1276 IO0


#define LORA_IO0 LORA_IRQ  // alias

#define FREQUENCY 433775000
#define SPREADING_FACTOR 12
#define SIGNAL_BANDWIDTH 125000
#define CODING_RATE_4 5
#define TX_POWER 20

#define MSG_MAX_LENGTH 100


class LoraAprs : public LoRaClass {
public:
    LoraAprs(long _frequency=FREQUENCY,
            int _spreadingFactor=SPREADING_FACTOR,
            long _signalBandwith=SIGNAL_BANDWIDTH,
            int _codingRate=CODING_RATE_4,
            int _txPower=TX_POWER);
    LoraAprs(const LoraAprs& orig);
    virtual ~LoraAprs();
    void setup();
    void send(char* msg,int length);
	bool checkMessage();  
	char * getMessage();
	


private:
  
  char msg[MSG_MAX_LENGTH];
  long frequency;
  int spreadingFactor;
  long signalBandwith;
  int codingRate;
  int txPower;
  
};

#endif /* LORAAPRS_H */

