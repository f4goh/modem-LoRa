/*
TX LoRa wx message with protobuf
https://github.com/nanopb/nanopb/tree/master/examples/simple
https://protobuf-decoder.netlify.app/

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
#include "pb_encode.h"
//#include "pb_decode.h"

uint8_t buffer[64];

void setup() {

  Serial.begin(115200);
  //while (!Serial);
  Serial.println("LoRa Protobuf TX");

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

  Data message = Data_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  message.temperature = 18.5;
  message.humidity = 60;
  message.air_pressure = 1015;
  message.wind_speed = 15;
  message.wind_direction = 260;

  bool status = pb_encode(&stream, Data_fields, &message);

  if (!status) {
    Serial.println("Failed to encode");
    return;
  }

  Serial.print("Message Length: ");
  Serial.println(stream.bytes_written);

  Serial.println("Message: ");

  char hexBuf[3];

  for (int i = 0; i < stream.bytes_written; i++) {
    sprintf(hexBuf, "%02x", buffer[i]);
    Serial.print(hexBuf);
  }
  Serial.println();

  LoRa.beginPacket();
  LoRa.write((char *)buffer, stream.bytes_written);
  LoRa.endPacket();

  delay(5000);
}