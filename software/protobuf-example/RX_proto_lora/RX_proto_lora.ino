/*
https://github.com/nanopb/nanopb/tree/master/examples/simple

syntax = "proto2";

message Data {
  required float temperature     = 1;
  required int32 humidity        = 2;
  required int32 air_pressure    = 3;
  required int32 wind_speed      = 4;
  required int32 wind_direction  = 5;
}

*/

#define LORA_SCK 13   // SX1276 SCK
#define LORA_MISO 12  // SX1276 MISO
#define LORA_MOSI 11  // SX1276 MOSI
#define LORA_CS 10    // SX1276 CS
#define LORA_RST 9    // SX1276 RST
#define LORA_IRQ 2    // SX1276 IO0

#define LORA_IO0 LORA_IRQ  // alias

//TEST frequency
#define FREQUENCY 433675000  //QSY no APRS and no Mesh frequency
#define SPREADING_FACTOR 12
#define SIGNAL_BANDWIDTH 125000
#define CODING_RATE_4 5
#define TX_POWER 20

#include <SPI.h>
#include <LoRa.h>

#include "weather.pb.h"

#include "pb_common.h"
#include "pb.h"
//#include "pb_encode.h"
#include "pb_decode.h"

uint8_t buffer[64];
uint8_t ptrBuffer = 0;

void setup() {

  Serial.begin(115200);

  //while (!Serial);

  Serial.println("LoRa Protobuf RX");

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }

  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE_4);
  LoRa.enableCrc();
  LoRa.setTxPower(TX_POWER);  //Config.lora.power);
  Serial.println("LoRa init done!");
}

void loop() {

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    // read packet
    while (LoRa.available()) {
      buffer[ptrBuffer++] = (uint8_t)LoRa.read();
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());

    // Décodage

    Serial.println("Message: ");

    for (int i = 0; i < ptrBuffer; i++) {
      Serial.print(buffer[i], HEX);
      Serial.print(',');
    }
    Serial.println();
    Data decoded_message = Data_init_zero;

    pb_istream_t istream = pb_istream_from_buffer(buffer, ptrBuffer);

    bool status = pb_decode(&istream, Data_fields, &decoded_message);
    ptrBuffer = 0;
    if (!status) {
      Serial.println("Failed to decode");
      return;
    }

    Serial.println("Decoded message: ");
    Serial.println(decoded_message.temperature);
    Serial.println(decoded_message.humidity);
    Serial.println(decoded_message.air_pressure);
    Serial.println(decoded_message.wind_speed);
    Serial.println(decoded_message.wind_direction);
  }
}