
#define LORA_SCK 13      // SX1276 SCK
#define LORA_MISO 12    // SX1276 MISO
#define LORA_MOSI 11    // SX1276 MOSI
#define LORA_CS 10      // SX1276 CS
#define LORA_RST 9     // SX1276 RST
#define LORA_IRQ 2     // SX1276 IO0

#define LORA_IO0 LORA_IRQ  // alias

#define FREQUENCY 433775000
#define SPREADING_FACTOR 12
#define SIGNAL_BANDWIDTH 125000
#define CODING_RATE_5 5
#define TX_POWER 20


#include <SPI.h>
#include <LoRa.h>

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE_5);
  LoRa.enableCrc();

  LoRa.setTxPower(TX_POWER);//Config.lora.power);
  Serial.println("LoRa init done!");    


}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
