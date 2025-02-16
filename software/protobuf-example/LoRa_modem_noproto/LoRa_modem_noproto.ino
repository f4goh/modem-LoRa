//LoRa modem wihtout protobuf
//send txt message via console


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

bool readyToSend = false;
uint8_t ptrBuffer = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("LoRa simple text modem");

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
  char car;
  char buffer[200];
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
  if (Serial.available()) {  //read char in a buufer until lf \n
    car = Serial.read();
    if (car == '\n') {
      readyToSend = true;   //lf received , ready to transmit
      buffer[ptrBuffer]='\0';
    } else {
      buffer[ptrBuffer++] = car;
    }
  }
  if (readyToSend == true) {  //transmit buffer and init ptr buffer
    Serial.print("message send : ");
    Serial.println(buffer);
    LoRa.beginPacket();
    LoRa.write((char *)buffer, ptrBuffer);
    LoRa.endPacket();
    readyToSend=false;
    ptrBuffer=0;
  }
}
