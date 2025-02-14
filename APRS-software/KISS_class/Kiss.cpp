#include "Kiss.h"


Kiss::Kiss(Stream &serialPort)
  : serialPort(serialPort) {}

void Kiss::TNC2toAX25(char *frame) {
  char *stations[MAX_STATIONS];
  uint8_t ssid[MAX_STATIONS] = { 0 };
  char *message = NULL;
  uint8_t maxStation = 0;

  // Trouver la position de ':' pour séparer la partie stations et le message
  char *msgStart = strstr(frame, ":");
  if (msgStart) {
    *msgStart = '\0';  // Terminer la première partie de la chaîne
    msgStart += 1;     // Pointer après "::"
    message = (char *)malloc(strlen(msgStart) + 1);
    if (message) {
      strcpy(message, msgStart);
    }
  }

  // Tokenizer pour extraire les stations
  char *token = strtok(frame, ">,");
  while (token != NULL && maxStation < MAX_STATIONS) {
    stations[maxStation] = (char *)malloc(10);  // Allocation fixe de 10 octets
    if (stations[maxStation]) {
      strncpy(stations[maxStation], token, 9);
      stations[maxStation][9] = '\0';

      // Extraire le SSID dans un tableau en entier
      char *dash = strrchr(stations[maxStation], '-');
      if (dash && isdigit(dash[1])) {
        ssid[maxStation] = atoi(dash + 1);
      } else {
        ssid[maxStation] = 0;
      }

      // Supprimer le SSID si présent
      if (dash) {
        *dash = '\0';
      }

      // Recopier et formater sur 7 caractères
      snprintf(stations[maxStation], 8, "%-7s", stations[maxStation]);

      //serialPort.print("Station: ");
      //serialPort.print(stations[maxStation]);
      //serialPort.print(strlen(stations[maxStation]));

      //serialPort.print(" SSID: ");
      //serialPort.println(ssid[maxStation]);
    }
    maxStation++;
    token = strtok(NULL, ">,");
  }

  //decallage des bits pour la trame ax25
  uint8_t i, j;
  for (i = 0; i < maxStation; i++) {
    for (j = 0; j < 6; j++) {
      stations[i][j] = stations[i][j] << 1;
      //serialPort.print((uint8_t)stations[i][j], HEX);
      //serialPort.print(',');
    }
    stations[i][6] = ('0' + ssid[i]) << 1;
    //serialPort.println((uint8_t)stations[i][6], HEX);
  }
  //ajout masque fin de stations
  stations[maxStation - 1][6] = stations[maxStation - 1][6] | 1;


  // Construction de la trame AX.25 + KISS
  size_t frameSize = 2 + (maxStation * 7) + 2 + (message ? strlen(message) : 0) + 1;
  uint8_t *ax25 = (uint8_t *)malloc(frameSize);
  if (ax25) {
    uint8_t *ptr = ax25;
    *ptr++ = 0xC0;  //debut de l'encapsulation KISS ne supporte que le mode data
    *ptr++ = 0x00;

    // Ajouter les stations dans l'ordre
    if (maxStation > 1) {
      memcpy(ptr, stations[1], 7);
      ptr += 7;
    }
    if (maxStation > 0) {
      memcpy(ptr, stations[0], 7);
      ptr += 7;
    }
    for (uint8_t i = 2; i < maxStation; i++) {
      memcpy(ptr, stations[i], 7);
      ptr += 7;
    }

    *ptr++ = 0x03;  //ajout séparateurs
    *ptr++ = 0xF0;

    if (message) {  //ajoute message
      strcpy((char *)ptr, message);
      ptr += strlen(message);
    }

    *ptr++ = 0xC0;  //fin kiss

    //serialPort.print("AX.25 Frame: ");
    //Envoi vers la liaison série
    for (size_t i = 0; i < frameSize; i++) {
      serialPort.write(ax25[i]);
      //serialPort.print(ax25[i], HEX);
      //serialPort.print(" ");
    }
    //serialPort.println();

    free(ax25);
  }

  // Affichage du message récupéré
  if (message) {
    //serialPort.print("Message: ");
    //serialPort.println(message);
    free(message);
  }
  // Affichage du nombre total de stations
  //serialPort.print("maxStation: ");
  //serialPort.println(maxStation);

  // Libération de la mémoire allouée pour les stations
  for (uint8_t i = 0; i < maxStation; i++) {
    free(stations[i]);
  }
}

bool Kiss::ax25IsValid(uint8_t *ax25, size_t length) {
  if (ax25 == NULL) {
    return false;
  }
  for (size_t i = 0; i < length; i++) {
    if (ax25[i] == 0x03 && ax25[i + 1] == 0xF0) {
      return true;
    }
  }
  return false;
}

String Kiss::assembleAdr(char *str, uint8_t ssid, char carSep, bool carInsert) {
  String result = "";
  result += str;
  if (ssid > 0) {
    result += '-';
    result += String(ssid);
  }
  if (carInsert == true) {
    result += carSep;
  }
  return result;
}


String Kiss::ax252tnc(uint8_t *ax25, size_t length) {
  String result = "";
  char addresses[10][10] = { 0 };  // Tableau pour stocker les adresses décodées
  uint8_t ssid[10];
  size_t addressCount = 0;

  // Décodage des adresses/ssid et stockage dans le tableau addresses/ssid
  size_t i, j = 0;
  while (i + 6 < length && addressCount < 10) {
    for (j = 0; j < 7; j++) {
      addresses[addressCount][j] = (char)((ax25[i + j] >> 1) & 0x7F);
    }
    ssid[addressCount] = addresses[addressCount][6] - '0';
    addresses[addressCount][6] = '\0';

    if (ax25[i + 6] & 0x1) {  // Fin des adresses
      addressCount++;
      i += 7;
      break;
    } else {
      addressCount++;
    }
    i += 7;
  }

  //supprime les espaces avant construction de la trame
  for (i = 0; i < addressCount; i++) {
    for (size_t j = 0; j < 7; j++) {
      if (addresses[i][j] == ' ') {
        addresses[i][j] = '\0';
      }
    }
  }
  /*
  for (i = 0; i < addressCount; i++) {  //vérification
    Serial.print(addresses[i]);
    Serial.print(' ');
    Serial.print(ssid[i]);
    Serial.println(',');
  }
  */
  // Reconstruction des adresses dans la chaîne résultat
  result += assembleAdr(addresses[1], ssid[1], '>', true);
  result += assembleAdr(addresses[0], ssid[0], ',', true);
  for (i = 2; i < addressCount; i++) {
    if (i == addressCount - 1) {
      result += assembleAdr(addresses[i], ssid[i], ',', false);
    } else {
      result += assembleAdr(addresses[i], ssid[i], ',', true);
    }
  }

  // Ajout de ':' de séparation
  result += ':';

  // Extraction des données après 0xF0 (c'est le payload APRS)
  // recherche de l'index
  size_t dataStart = 0;
  for (i = 0; i < length; i++) {  //mod
    if (ax25[i] == 0xF0) {
      dataStart = i + 1;
      break;  //mouais...
    }
  }

  for (size_t j = dataStart; j < length; j++) {
    result += (char)ax25[j];
  }

  return result;
}

bool Kiss::tncIsValid(char *frame) {
  if (frame == NULL) {
    return false;
  }
  while (*frame) {
    if (*frame == ':') {
      return true;
    }
    frame++;
  }
  return false;
}
