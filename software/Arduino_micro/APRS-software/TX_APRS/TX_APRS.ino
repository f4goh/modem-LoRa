//pro micro
#define LORA_SCK 15   // SX1276 SCK
#define LORA_MISO 14  // SX1276 MISO
#define LORA_MOSI 16  // SX1276 MOSI
#define LORA_CS 10    // SX1276 CS
#define LORA_RST 9    // SX1276 RST
#define LORA_IRQ 7    // SX1276 IO0
#define LED_TX 5  //used for TX
#define LED_RX 6  //used for RX

#include <Arduino.h>
#include <LoRa.h>
#include <Position.h>  // Trame Position APRS
#include <LoraAprs.h>
#include <Message.h>
#include <Weather.h>




Position pos("F4KMN-7", "APLT00", "WIDE1-1", 48.010237, 0.206267, "Ballon", '/', 'O');  // icon ballon
Message mes("F4GOH-9", "APLT00", "WIDE1-1", "F4KMN    ", "Hello");
Weather wea("F4GOH-4", "APLT00", "WIDE1-1", 48.010237, 0.206267);


LoraAprs lora;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(LED_TX, OUTPUT);
  pinMode(LED_RX, OUTPUT);
  delay(5000);
  lora.setup();
}

void loop() {
  char c;
  char *pdu;
  if (lora.checkMessage()) {
    pdu = lora.getMessage();
    Serial.println(pdu);
  }
  if (Serial.available() > 0) {
    c = Serial.read();
    digitalWrite(LED_TX, HIGH);
    switch (c) {
      case 'm':
        Serial.println(F("Send a message"));
        mes.setRecipient("F4KMN    ");
        mes.setComment("i like LoRa");
        pdu = mes.getPduMes(false);
        Serial.println(pdu);
        lora.send(pdu, mes.getPduLength());
        break;
      case 'p':
        Serial.println(F("Send a position"));
        pos.setLatitude(48.010237); 
        pos.setLongitude(0.206267);
        pos.setSymbol('O');
        pos.setAltitude(80);
        pdu = pos.getPduAprs(false);
        Serial.println(pdu);
        lora.send(pdu, pos.getPduLength());
        break;
      case 'w':
        Serial.println(F("Send weather report"));
        wea.setWind(90);    // Direction du vent
        wea.setSpeed(2.5);  // Vitesse moyenne du vent en m/s
        //wea.setGust(5.1);         // Vitesse en rafale en m/s
        wea.setTemp(17.5);        // Température en degré celsius
        wea.setRain(0);           // Pluie en mm/1h
        wea.setRain24(0);         // Pluie en mm/24h
        wea.setRainMid(0);        // Pluie en mm/depuit minuit
        wea.setHumidity(60);      // Hmidité relative en %
        wea.setPressure(1008.5);  // Pression barométrique en hPa
        wea.setComment("Weather");
        pdu = wea.getPduWx();
        Serial.println(pdu);
        lora.send(pdu, wea.getPduLength());
        break;
    }
    digitalWrite(LED_TX, LOW);
  }
}
