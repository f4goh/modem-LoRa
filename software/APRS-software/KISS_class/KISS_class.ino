//pro micro
#define LORA_SCK 15   // SX1276 SCK
#define LORA_MISO 14  // SX1276 MISO
#define LORA_MOSI 16  // SX1276 MOSI
#define LORA_CS 10    // SX1276 CS
#define LORA_RST 9    // SX1276 RST
#define LORA_IRQ 7    // SX1276 IO0
#define LED_TX 5      //used for TX
#define LED_RX 6      //used for RX

#define MAX_FRAME_SIZE 150  // Taille maximale de la trame

#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define TIME_INFO 5000

#include <Arduino.h>
#include <LoraAprs.h>
#include <Tiny4kOLED.h>

#include "Kiss.h"

uint8_t frame[MAX_FRAME_SIZE];  // Tableau pour stocker la trame
uint8_t frameIndex = 0;         // Indice actuel dans le tableau
bool frameComplete = false;     // Indicateur de trame complète
uint8_t state = 0;              // Indicateur d'état de réception d'une trame
uint32_t tempoPrec = 0;

LoraAprs lora;
Kiss *kiss;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
  pinMode(LED_TX, OUTPUT);
  pinMode(LED_RX, OUTPUT);
  lora.setup();
  Serial.flush();
  kiss = new Kiss(Serial);
  oled.begin();
  //oled.setFont(FONT8X16);
  //oled.setFontX2(FONT6X8P);
  oled.setFontX2(FONT8X16);
  oled.clear();
  oled.on();
  oled.setCursor(0, 0);
  oledPrint("F4GOH");
  tempoPrec = millis();
  /*
  //Test kiss Class
  char loraStr[] = "F4GOH-1>APIN21,WIDE1-1,WIDE2-1::F4KMN-2  :Test{1";
  if (kiss->tncIsValid(loraStr)) {
    kiss->TNC2toAX25(loraStr);
  }
   uint8_t ax25Str[] = { 0x82, 0xa0, 0x92, 0x9C, 0x64, 0x62, 0x60, 0x8C, 0x68, 0x8E, 0x9E, 0x90, 0x40, 0x62,
                        0xAE, 0x92, 0x88, 0x8A, 0x62, 0x40, 0x62, 0xAE, 0x92, 0x88, 0x8A, 0x64, 0x40, 0x63, 0x03, 0xF0,
                        0x3A, 0x46, 0x34, 0x4B, 0x4D, 0x4E, 0x2D, 0x32, 0x20, 0x20, 0x3A, 0x54, 0x65, 0x73, 0x74, 0x7B,
                        0x31 };

  String decoded = kiss->ax252tnc(ax25Str, sizeof(ax25Str));
  Serial.println(decoded);
  */
}

void oledPrint(char *text) {
  oled.clear();
  oled.setCursor(40, 0);
  oled.print(text);
}

void handleInputData(uint8_t car) {
  switch (state) {
    case 0:
      if (car == 0xC0) state = 1;
      break;
    case 1:
      if (car == 0x00) state = 2;
      else state = 0;
      break;
    case 2:
      if (car != 0xC0) {
        if (frameIndex < MAX_FRAME_SIZE) {
          frame[frameIndex++] = car;
        }
      } else frameComplete = true;
      break;
  }
  if (frameComplete) {
    if (kiss->ax25IsValid(frame, frameIndex)) {
      String decoded;
      digitalWrite(LED_TX, HIGH);
      oledPrint("TX");
      /*
      for (int n; n < frameIndex; n++) {
        Serial.write(frame[n]);
      }
      Serial.println(frameIndex);
      */
      decoded = kiss->ax252tnc(frame, frameIndex);
      //Serial.println(decoded.length());
      //Serial.println(decoded);
      char pdu[decoded.length()];
      decoded.toCharArray(pdu, decoded.length() + 1);
      lora.send(pdu, decoded.length() + 1);
      digitalWrite(LED_TX, LOW);
      //Serial.println(pdu);
    }
    // Réinitialisation du buffer
    frameIndex = 0;
    frameComplete = false;
    state = 0;
  }
}



void readFromSerial() {
  while (Serial.available() > 0) {
    char character = Serial.read();
    handleInputData(character);
  }
}


void loop() {
  char *pdu;
  char car;
  if (lora.checkMessage()) {  //lecture trame lora et conversion AX25 et envoi vers ligne serie
    digitalWrite(LED_RX, HIGH);
    oledPrint("RX");
    pdu = lora.getMessage();
    //Serial.println(pdu);
    if (kiss->tncIsValid(pdu)) {
      kiss->TNC2toAX25(pdu);
    }
    digitalWrite(LED_RX, LOW);
  }
  readFromSerial();  //lecture trame serie et envoi sur le lora

  if (millis() - tempoPrec > TIME_INFO) {
    oled.clear();
    tempoPrec = millis();
  }
}
