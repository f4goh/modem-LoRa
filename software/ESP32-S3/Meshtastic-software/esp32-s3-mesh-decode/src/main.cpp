/*
 https://github.com/jgromes/radiolib/issues/121
 * https://github.com/f4goh/modem-LoRa/blob/main/software/Meshtastic-software/Mesh_lora_proto_decode/Mesh_lora_proto_decode.ino
 
 Trame reçue (Hex) :
RSSI -73.00 dBm
Frame struct Size: 272
Destination: 0xffffffff
Sender: 0x1c0835c8
PacketID: 0x2661ab02
Crypted payload: 43,8E,8B,5C,D6,FE,FC,14,46,DC,2F,D5,7C,D4,32,65,2F,4F,3,4B,97,6                                                                   B,2,88,D8,E0,24,F7,
AES key : D4,F1,BB,3A,20,29,7,59,F0,BC,FF,AB,CF,4E,69,1,
AES IV : 2,AB,61,26,0,0,0,0,C8,35,8,1C,0,0,0,0,
Decrypted payload: 8,43,12,18,D,D0,2B,6E,67,12,11,8,65,15,27,31,88,40,1D,AA,AA,9                                                                   A,40,25,FD,88,8E,3F,
Decoded test_number: 67
Time stamp: 1735273424
Variant number: 2
Is deviceMetrics:
Battery Level: 101
Voltage: 4.26
Channel Utilization: 4.83
Air Utilization TX: 1.11
-----------------------------
Trame reçue (Hex) :
RSSI -41.00 dBm
Frame struct Size: 272
Destination: 0xffffffff
Sender: 0xbd619b4c
PacketID: 0x9074f737
Crypted payload: 2D,4A,28,FB,D5,D2,7A,46,54,A0,FC,99,92,C3,25,E2,AE,26,1D,8D,A9,99,5E,5,EC,A6,2F,F8,BD,A6,E6,6D,9F,EE,6F,54,
AES key : D4,F1,BB,3A,20,29,7,59,F0,BC,FF,AB,CF,4E,69,1,
AES IV : 37,F7,74,90,0,0,0,0,4C,9B,61,BD,0,0,0,0,
Decrypted payload: 8,43,12,1E,D,23,44,F1,67,1A,17,D,E5,AC,C9,41,15,4E,B8,2F,42,1D,6,2F,7B,44,25,D1,E7,CC,42,38,DE,1,48,0,
Decoded test_number: 67
Time stamp: 1743864867
Variant number: 3
Is environmentMetrics:
Temperature: 25.21
Relative Humidity: 43.93
Barometric Pressure: 1004.73
Gas Resistance: 102.45
IAQ: 222
 * 
 */


#include <Crypto.h>
#include <AES.h>
#include <CTR.h>
#include "base64.h"
#include <SPI.h>
#include <RadioLib.h>
#include "mesh.pb.h"
#include "telemetry.pb.h"

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

/*
size_t pb_encode_to_bytes(uint8_t *destbuf, size_t destbufsize, const pb_msgdesc_t *fields, const void *src_struct)
{
    pb_ostream_t stream = pb_ostream_from_buffer(destbuf, destbufsize);
    if (!pb_encode(&stream, fields, src_struct)) {
        //LOG_ERROR("Panic: can't encode protobuf reason='%s'", PB_GET_ERROR(&stream));
        return 0;
    } else {
        return stream.bytes_written;
    }
}
*/
/// helper function for decoding a record as a protobuf, we will return false if the decoding failed
bool pb_decode_from_bytes(const uint8_t *srcbuf, size_t srcbufsize, const pb_msgdesc_t *fields, void *dest_struct)
{
    pb_istream_t stream = pb_istream_from_buffer(srcbuf, srcbufsize);
    if (!pb_decode(&stream, fields, dest_struct)) {
        //LOG_ERROR("Can't decode protobuf reason='%s', pb_msgdesc %p", PB_GET_ERROR(&stream), fields);
        return false;
    } else {
        return true;
    }
}


void results(frameStr frame, aesVector* decodeAES128CTR) {
  Serial.printf("Frame struct Size: %d\n\r",sizeof(frameStr));
  Serial.printf("Destination: 0x%08x\n\r",frame.destination);
  Serial.printf("Sender: 0x%08x\n\r",frame.sender);
  Serial.printf("PacketID: 0x%08x\n\r",frame.packetID);
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
  //status = pb_decode_from_bytes(buffer, sizeof(buffer), meshtastic_Data_fields, &decoded_message);
  
  pb_istream_t istream = pb_istream_from_buffer(decodeAES128CTR->plaintext, decodeAES128CTR->size);
  status = pb_decode(&istream, meshtastic_Data_fields, &decoded_message);

  if (!status) {
    Serial.println("Failed to decode");
    return;
  }

  Serial.print("Decoded test_number: ");
  Serial.println(decoded_message.portnum);
  meshtastic_Data_payload_t payload = decoded_message.payload;
  if (decoded_message.portnum == 1) { //message
    //meshtastic_Data_payload_t payload = decoded_message.payload;
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
    }
    if (decoded_message.portnum == 67) { //telemetry
        meshtastic_Telemetry scratch;
        meshtastic_Telemetry *decoded = NULL;
        memset(&scratch, 0, sizeof (scratch));
        if (pb_decode_from_bytes(payload.bytes, payload.size, &meshtastic_Telemetry_msg, &scratch)) {
            decoded = &scratch;
            Serial.print(F("Time stamp: "));
            Serial.println(decoded->time);
            Serial.print(F("Variant number: "));
            Serial.println(decoded->which_variant);
            if (decoded->which_variant == 2) { // Le variant est DeviceMetrics
                meshtastic_DeviceMetrics *metrics = &(decoded->variant.device_metrics);
                Serial.println(F("Is deviceMetrics:"));
                if (metrics->has_battery_level) {
                    Serial.print(F("Battery Level: "));
                    Serial.println(metrics->battery_level);
                }

                if (metrics->has_voltage) {
                    Serial.print(F("Voltage: "));
                    Serial.println(metrics->voltage);
                }

                if (metrics->has_channel_utilization) {
                    Serial.print(F("Channel Utilization: "));
                    Serial.println(metrics->channel_utilization);
                }

                if (metrics->has_air_util_tx) {
                    Serial.print(F("Air Utilization TX: "));
                    Serial.println(metrics->air_util_tx);
                }

                if (metrics->has_uptime_seconds) {
                    Serial.print(F("Uptime Seconds: "));
                    Serial.println(metrics->uptime_seconds);
                }
            }

            if (decoded->which_variant == 3) { // Le variant est EnvironmentMetrics
                meshtastic_EnvironmentMetrics *env_metrics = &(decoded->variant.environment_metrics);
                Serial.println(F("Is environmentMetrics:"));
                if (env_metrics->has_temperature) {
                    Serial.print(F("Temperature: "));
                    Serial.println(env_metrics->temperature);
                }

                if (env_metrics->has_relative_humidity) {
                    Serial.print(F("Relative Humidity: "));
                    Serial.println(env_metrics->relative_humidity);
                }

                if (env_metrics->has_barometric_pressure) {
                    Serial.print(F("Barometric Pressure: "));
                    Serial.println(env_metrics->barometric_pressure);
                }

                if (env_metrics->has_gas_resistance) {
                    Serial.print(F("Gas Resistance: "));
                    Serial.println(env_metrics->gas_resistance);
                }

                if (env_metrics->has_voltage) {
                    Serial.print(F("Voltage: "));
                    Serial.println(env_metrics->voltage);
                }

                if (env_metrics->has_current) {
                    Serial.print(F("Current: "));
                    Serial.println(env_metrics->current);
                }

                if (env_metrics->has_iaq) {
                    Serial.print(F("IAQ: "));
                    Serial.println(env_metrics->iaq);
                }

                if (env_metrics->has_distance) {
                    Serial.print(F("Distance: "));
                    Serial.println(env_metrics->distance);
                }

                if (env_metrics->has_lux) {
                    Serial.print(F("Ambient Light Lux: "));
                    Serial.println(env_metrics->lux);
                }

                if (env_metrics->has_white_lux) {
                    Serial.print(F("White Light Lux: "));
                    Serial.println(env_metrics->white_lux);
                }

                if (env_metrics->has_ir_lux) {
                    Serial.print(F("Infrared Lux: "));
                    Serial.println(env_metrics->ir_lux);
                }

                if (env_metrics->has_uv_lux) {
                    Serial.print(F("Ultraviolet Lux: "));
                    Serial.println(env_metrics->uv_lux);
                }

                if (env_metrics->has_wind_direction) {
                    Serial.print(F("Wind Direction: "));
                    Serial.println(env_metrics->wind_direction);
                }

                if (env_metrics->has_wind_speed) {
                    Serial.print(F("Wind Speed: "));
                    Serial.println(env_metrics->wind_speed);
                }

                if (env_metrics->has_weight) {
                    Serial.print(F("Weight: "));
                    Serial.println(env_metrics->weight);
                }

                if (env_metrics->has_wind_gust) {
                    Serial.print(F("Wind Gust: "));
                    Serial.println(env_metrics->wind_gust);
                }

                if (env_metrics->has_wind_lull) {
                    Serial.print(F("Wind Lull: "));
                    Serial.println(env_metrics->wind_lull);
                }

                if (env_metrics->has_radiation) {
                    Serial.print(F("Radiation: "));
                    Serial.println(env_metrics->radiation);
                }

                if (env_metrics->has_rainfall_1h) {
                    Serial.print(F("Rainfall (1h): "));
                    Serial.println(env_metrics->rainfall_1h);
                }

                if (env_metrics->has_rainfall_24h) {
                    Serial.print(F("Rainfall (24h): "));
                    Serial.println(env_metrics->rainfall_24h);
                }
            }
        }
        Serial.println();
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