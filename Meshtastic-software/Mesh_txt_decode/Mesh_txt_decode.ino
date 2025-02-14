/*
Meshtastic AES-128 decrypt in CTR mode
raw mesh text message 0xFF, 0xFF, 0xFF, 0xFF, 0x88, 0x1B, 0xC2, 0x25, 0xA5, 0x72, 0xA4, 0xAB, 0x63, 0x8, 0x0, 0x0, 0x2D, 0x73, 0xFE, 0xA3, 0x70, 0x6E, 0xBF, 0x6A, 0x16, 0xE0
key =  0xd4, 0xf1, 0xbb, 0x3a, 0x20, 0x29, 0x07, 0x59, 0xf0, 0xbc, 0xff, 0xab, 0xcf, 0x4e, 0x69, 0x01
plaintext = 0x08, 0x01, 0x12, 0x04, 0x54, 0x65, 0x73, 0x74, 0x48, 0x00 //payload decrypted
ciphertext = 0x2d, 0x73, 0xfe, 0xa3, 0x70, 0x6e, 0xbf, 0x6a, 0x16, 0xe0  //payload crypted
iv = 0xa5, 0x72, 0xa4, 0xab, 0x00, 0x00, 0x00, 0x00,0x88, 0x1b, 0xc2, 0x25, 0x00, 0x00, 0x00, 0x00 //cypher init
size = 10
Message received : Test
*/


#include <Crypto.h>
#include <AES.h>
#include <CTR.h>
#include "base64.h"
#include <SPI.h>
#include <LoRa.h>

#define MAX_PLAINTEXT_SIZE 36
#define MAX_CIPHERTEXT_SIZE 36

#define LORA_SCK 13   // SX1276 SCK
#define LORA_MISO 12  // SX1276 MISO
#define LORA_MOSI 11  // SX1276 MOSI
#define LORA_CS 10    // SX1276 CS
#define LORA_RST 9    // SX1276 RST
#define LORA_IRQ 2    // SX1276 IO0

#define LORA_IO0 LORA_IRQ  // alias

#define CONFIG0 A1
#define CONFIG1 A2
#define CONFIG2 A3
#define LED1 5
#define LED2 6
#define POTAR A0
#define RGB 8
#define DSONEWIRE 4

//meshtastic
#define FREQUENCY 433875000
#define SPREADING_FACTOR 11
#define SIGNAL_BANDWIDTH 250000
#define CODING_RATE_5 5
#define TX_POWER 20

typedef struct {
  uint8_t key[16];
  uint8_t plaintext[MAX_PLAINTEXT_SIZE];
  uint8_t ciphertext[MAX_CIPHERTEXT_SIZE];
  uint8_t iv[16];
  size_t size;
} aesVector;


// Test vectors for AES-128 in CTR mode from RFC 3686.
aesVector decodeAES128CTR = {
  .key = { 0 },
  .plaintext = { 0 },
  .ciphertext = { 0 },
  .iv = { 0 },
  .size = 0
};


char meshtasticFullKeyBase64[] = "1PG7OiApB1nwvP+rz05pAQ==";


CTR<AES128> ctraes128;

#define packetDataLength 30

typedef struct {
  uint32_t destination;
  uint32_t sender;
  uint32_t packetID;
  uint8_t hopLimit : 3;
  uint8_t wantAck : 1;
  uint8_t viaMQTT : 1;
  uint8_t hopStart : 3;
  uint8_t channelHash;
  uint16_t reserved;
  uint8_t packetData[packetDataLength];
  uint8_t size;
} frameStr;

frameStr frame;


void cipherDecrypt(Cipher* cipher, aesVector* decodeAES128CTR, frameStr* frame) {
  cipher->setKey(decodeAES128CTR->key, cipher->keySize());
  cipher->setIV(decodeAES128CTR->iv, cipher->ivSize());
  cipher->decrypt(decodeAES128CTR->plaintext, frame->packetData, decodeAES128CTR->size);  //dest,source,size
}

void printHex(uint8_t* tab, uint8_t len) {
  for (int i = 0; i < len; i++) {
    Serial.print(tab[i], HEX);
    Serial.print(',');
  }
  Serial.println();
}

void results(frameStr frame, aesVector* decodeAES128CTR) {
  Serial.print(F("Frame struct Size: "));
  Serial.println(sizeof(frameStr));
  Serial.print(F("Destination: "));
  Serial.println(frame.destination, HEX);
  Serial.print(F("Sender: "));
  Serial.println(frame.sender, HEX);
  Serial.print(F("PacketID: "));
  Serial.println(frame.packetID, HEX);
  Serial.print(F("Crypted payload: "));
  printHex(frame.packetData, frame.size);
  Serial.print(F("AES key : "));
  printHex(decodeAES128CTR->key, 16);
  Serial.print(F("AES IV : "));
  printHex(decodeAES128CTR->iv, 16);
  Serial.print(F("Decrypted payload: "));
  printHex(decodeAES128CTR->plaintext, decodeAES128CTR->size);
  if (decodeAES128CTR->plaintext[1] == 0x01) {  //a revoir dans une structure message
    Serial.print(F("Message received: "));
    for (int n = 4; n < 4 + decodeAES128CTR->plaintext[3]; n++) {
      Serial.print((char)decodeAES128CTR->plaintext[n]);
      Serial1.print((char)decodeAES128CTR->plaintext[n]);
      switch ((char)decodeAES128CTR->plaintext[4 + 3]) {
        case '0':
          digitalWrite(LED1, LOW);
          break;
        case '1':
          digitalWrite(LED1, HIGH);
          break;
      }
    }
  } else {
    Serial.print(F("Frame unknown"));
  }
  Serial.println();
}

void decode() {
  // cpy iv init
  *((uint32_t*)&decodeAES128CTR.iv + 2) = frame.sender;
  *((uint32_t*)&decodeAES128CTR.iv) = frame.packetID;

  cipherDecrypt(&ctraes128, &decodeAES128CTR, &frame);
  results(frame, &decodeAES128CTR);
}


void setup() {
  Serial.begin(115200);
  Serial1.begin(9600);
  while (!Serial)
    ;
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);

  if (!LoRa.begin(FREQUENCY)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }

  LoRa.setSpreadingFactor(SPREADING_FACTOR);
  LoRa.setSignalBandwidth(SIGNAL_BANDWIDTH);
  LoRa.setCodingRate4(CODING_RATE_5);
  LoRa.enableCrc();
  LoRa.setPreambleLength(16);  //mesh
  LoRa.setSyncWord(0x2b);      //mesh
  LoRa.setTxPower(TX_POWER);   //Config.lora.power);
  Serial.println("LoRa init done!");

  //init key
  base64_decode(meshtasticFullKeyBase64, decodeAES128CTR.key);  //source, dest
}

void loop() {
  // try to parse packet
  int i;
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");
    memset(frame.packetData, 0, packetDataLength);
    i = 0;
    // read packet
    while (LoRa.available()) {
      *((uint8_t*)&frame + i) = (uint8_t*)LoRa.read();
      i++;
    }
    frame.size = i;
    decodeAES128CTR.size = i - 0x10;
    if (decodeAES128CTR.size > 5) {
      decode();
    }
    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
