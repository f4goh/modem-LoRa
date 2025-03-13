/*
 https://github.com/jgromes/radiolib/issues/121
 * for esp32-S3 lolin_s3_mini
 */


#include <Crypto.h>
#include <AES.h>
#include <CTR.h>
#include "base64.h"
#include <SPI.h>
#include <RadioLib.h>
#include "mesh.pb.h"
#include "pb_common.h"
#include "pb.h"
//#include "pb_encode.h"
#include "pb_decode.h"

// Paramètres des broches SPI pour RA-02
#define LORA_SCK 5
#define LORA_MISO 3
#define LORA_MOSI 6
#define LORA_CS 7
#define LORA_RST 8
#define LORA_IRQ 9

#define BUFFER_SIZE 255

#define FREQUENCY 433.875f
#define SPREADING_FACTOR 11
#define SIGNAL_BANDWIDTH 250.0f
#define CODING_RATE_4 5
#define SYNC_WORD 0x2B
#define PREAMBLE_LENGTH 0x2B

#define CONFIG0 36
#define CONFIG1 34
#define CONFIG2 35
#define LED1 37
#define LED2 38
#define POTAR 1
#define DSONEWIRE 4

#define MAX_PLAINTEXT_SIZE 255
#define MAX_CIPHERTEXT_SIZE 255

#define TX_POWER 20

SPIClass newSPI(HSPI);

Module* mod;
SX1278* lora;

uint8_t buffer[BUFFER_SIZE];
volatile bool operationDone = false; // Utiliser 'volatile' pour les drapeaux manipulés dans les interruptions

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

#define packetDataLength 255

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
  Serial.printf("Frame struct Size: %d\n",sizeof(frameStr));
  Serial.printf("Destination: 0x%08x\n",frame.destination);
  Serial.printf("Sender: 0x%08x\n",frame.sender);
  Serial.printf("PacketID: 0x%08x\n",frame.packetID);
  Serial.print(F("Crypted payload: "));
  printHex(frame.packetData, decodeAES128CTR->size);
  Serial.print(F("AES key : "));
  printHex(decodeAES128CTR->key, 16);
  Serial.print(F("AES IV : "));
  printHex(decodeAES128CTR->iv, 16);
  Serial.print(F("Decrypted payload: "));
  printHex(decodeAES128CTR->plaintext, decodeAES128CTR->size);

  meshtastic_Data decoded_message meshtastic_Data_init_zero;
  bool status;
  pb_istream_t istream = pb_istream_from_buffer(decodeAES128CTR->plaintext, decodeAES128CTR->size);
  status = pb_decode(&istream, meshtastic_Data_fields, &decoded_message);

  if (!status) {
    Serial.println("Failed to decode");
    return;
  }

  Serial.print("Decoded test_number: ");
  Serial.println(decoded_message.portnum);
  if (decoded_message.portnum == 1) {
    meshtastic_Data_payload_t payload = decoded_message.payload;
    char* buf = (char*)malloc(payload.size + 1);      // because of terminating `\0`
    memcpy(buf, payload.bytes, payload.size);  // copy the message
    buf[payload.size] = '\0';                  // force '\0' termination
    Serial.println(buf);
    if (strncmp("Led", buf, 3) == 0) {
      switch ((char)buf[3]) {
        case '0':
          digitalWrite(LED1, LOW);
          break;
        case '1':
          digitalWrite(LED1, HIGH);
          break;
      }
    }
    free(buf);
  } else {
    Serial.println(F("Not a text smg"));
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

void setFlag(void) {
    operationDone = true;
}

void setup() {
    Serial.begin(115200);
    while (!Serial); // Attendre que le port série soit prêt
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    // Initialisation du bus SPI
    newSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI);

    // Initialisation du module
    mod = new Module(LORA_CS, LORA_IRQ, LORA_RST, RADIOLIB_NC, newSPI);
    lora = new SX1278(mod);

    Serial.println("Initialisation du module LoRa...");

    // Initialisation LoRa avec fréquence spécifiée
    int state = lora->begin(FREQUENCY);
    if (state == RADIOLIB_ERR_NONE) {
        Serial.println("Module LoRa initialisé avec succès !");
    } else {
        Serial.print("Erreur lors de l'initialisation : ");
        Serial.println(state);
        while (true); // Arrêt en cas d'échec critique
    }

    // Configuration supplémentaire
    lora->setCRC(true);              // Activer le CRC
    lora->setPreambleLength(PREAMBLE_LENGTH);     // Longueur du préambule
    lora->setSyncWord(SYNC_WORD);         // Mot de synchronisation
    lora->setCodingRate(CODING_RATE_4);          // Rapport de codage
    lora->setBandwidth(SIGNAL_BANDWIDTH);       // Bande passante
    lora->setSpreadingFactor(SPREADING_FACTOR);    // Facteur d'étalement

    Serial.println("Configuration des paramètres LoRa terminée.");

    // Configurer l'interruption pour réception
    lora->setDio0Action(setFlag, RISING);
    lora->startReceive();

    Serial.println("Module prêt à recevoir.");
    //init key
  base64_decode(meshtasticFullKeyBase64, decodeAES128CTR.key);  //source, dest
  
}

void loop() {
    // Vérifier si l'opération est terminée
    if (operationDone) {
        operationDone = false; // Réinitialiser le drapeau
        //String message = "";
        //int state = lora->readData(message); // Utiliser readData pour traiter les données reçues
        int state = lora->readData(buffer,BUFFER_SIZE);
        if (state == RADIOLIB_ERR_NONE) {

            Serial.print("Trame reçue (Hex) : ");
            for (size_t i = 0; i < lora->getPacketLength(true); i++) {
                //Serial.printf("%02X ", (unsigned char) buffer[i]);
                *((uint8_t*) & frame + i) = (uint8_t) buffer[i];
            }
            Serial.println();
            // print RSSI of packet
            Serial.print("RSSI ");
            Serial.print(lora->getRSSI());
            Serial.println(F(" dBm"));

            frame.size = lora->getPacketLength(true);
            decodeAES128CTR.size = frame.size - 0x10;
            if (decodeAES128CTR.size > 5) {
                decode();
                }
        } else {
            Serial.print("Erreur de réception, code : ");
            Serial.println(state);
        }

        // Reprendre la réception
        lora->startReceive();
    }
}

/*
  // Exemple d'envoi d'un message
  Serial.println("Envoi du message...");
  int state = lora.transmit("Bonjour, LoRa !");

  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Message envoyé avec succès !");
  } else {
    Serial.print("Erreur lors de l'envoi : ");
    Serial.println(state);
  }
 */