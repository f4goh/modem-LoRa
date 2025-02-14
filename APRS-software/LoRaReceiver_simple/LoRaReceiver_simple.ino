//https://meshtastic.org/docs/overview/mesh-algo/
// Received packet 'FF,FF,FF,FF,88,1B,C2,25,A5,72,A4,AB,63,8,0,0,2D,73,FE,A3,70,6E,BF,6A,16,E0,' with RSSI -59
// Received packet 'FF,FF,FF,FF,88,1B,C2,25,A9,72,A4,AB,63,8,0,0,12,08,E5,63,BD,CB,AF,D7,FD,BF,' with RSSI -73

/*
FF,FF,FF,FF, Packet Header: Destination. The destination's unique NodeID. 0xFFFFFFFF for broadcast. Little Endian.
88,1B,C2,25, Packet Header: Sender. The sender's unique NodeID. Little Endian.
A5,72,A4,AB, Packet Header: The sending node's unique packet ID for this packet. Little Endian.
63, 011 0 0 011 Packet Header: Flags. See the header flags for usage.
8, Packet Header: Channel hash. Used as hint for decryption for the receiver.
0,0, Packet Header: Reserved for future use.
2D,73,FE,A3,70,6E,BF,6A,16,E0, Actual packet data. Unused bytes are not transmitted.
*/


#define LORA_SCK 13   // SX1276 SCK
#define LORA_MISO 12  // SX1276 MISO
#define LORA_MOSI 11  // SX1276 MOSI
#define LORA_CS 10    // SX1276 CS
#define LORA_RST 9    // SX1276 RST
#define LORA_IRQ 2    // SX1276 IO0

#define LORA_IO0 LORA_IRQ  // alias


//APRS
#define FREQUENCY 433775000
#define SPREADING_FACTOR 12
#define SIGNAL_BANDWIDTH 125000
#define CODING_RATE_5 5
#define TX_POWER 20
/*
//meshtastic
#define FREQUENCY 433875000
#define SPREADING_FACTOR 11
#define SIGNAL_BANDWIDTH 250000
#define CODING_RATE_5 5
#define TX_POWER 20
*/

#include <SPI.h>
#include <LoRa.h>

typedef struct {
  uint32_t destination;
  uint32_t sender;
  uint32_t packetID;
  uint8_t HopLimit : 3;
  uint8_t WantAck : 1;
  uint8_t ViaMQTT : 1;
  uint8_t HopStart : 3;
  uint16_t Reserved;
  uint8_t packetData[30];
} frameStr;



void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("LoRa Receiver");

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }

  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE_5);
  LoRa.enableCrc();
 // LoRa.setPreambleLength(16);  //mesh
 // LoRa.setSyncWord(0x2b);      //mesh



  LoRa.setTxPower(TX_POWER);  //Config.lora.power);
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
      Serial.print((int)LoRa.read(), HEX);
      Serial.print(",");
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
