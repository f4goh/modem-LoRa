//pro micro
#define LORA_SCK 15   // SX1276 SCK
#define LORA_MISO 14  // SX1276 MISO
#define LORA_MOSI 16  // SX1276 MOSI
#define LORA_CS 10    // SX1276 CS
#define LORA_RST 9    // SX1276 RST
#define LORA_IRQ 7    // SX1276 IO0
#define LED_TX 5      //used for TX
#define LED_RX 6      //used for RX

#include <Arduino.h>
#include <LoRa.h>
#include <Position.h>  // Trame Position APRS
#include <LoraAprs.h>
#include <Message.h>
#include <Weather.h>




Position pos("F4KMN-12", "APLT00", "WIDE1-1", 47.99531, 0.20502, "Ballon", '/', 'O');  // icon ballon
//Message mes("F4GOH-9", "APLT00", "WIDE1-1", "F4KMN    ", "Hello");
//Weather wea("F4GOH-4", "APLT00", "WIDE1-1", 48.010237, 0.206267);


LoraAprs lora;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial1.begin(9600);
  pinMode(LED_TX, OUTPUT);
  pinMode(LED_RX, OUTPUT);
  delay(5000);
  lora.setup();
}

void loop() {
  String payload;
  char *pdu;
  bool tx = false;
  //pas de réception
  /*
  if (lora.checkMessage()) {
    pdu = lora.getMessage();
    Serial.println(pdu);
  }
  */
  if (Serial.available() > 0) {           //via liaison série USB 115200
    payload = Serial.readStringUntil(10);
    tx=true;

  }
  if (Serial1.available() > 0) {        //via liaison série MAX232 9600
    payload = Serial1.readStringUntil(10);
    tx=true;
  }

  if (tx) {
    //c = Serial.read();
    digitalWrite(LED_TX, HIGH);
    Serial.println(F("Send a position"));
    //pos.setLatitude(47.99531);
    //pos.setLongitude(0.20502);
    //pos.setSymbol('O');
    pos.setAltitude(80);
    pos.setComment(payload);
    pdu = pos.getPduAprs(false);
    Serial.println(pdu);
    lora.send(pdu, pos.getPduLength());
    digitalWrite(LED_TX, LOW);
    tx = false;
  }
}
