#ifndef KISS_H
#define KISS_H

#include <Arduino.h>

#define MAX_STATIONS 10

class Kiss {
public:
  Kiss(Stream &serialPort);
  void TNC2toAX25(char *frame);
  String ax252tnc(uint8_t *ax25, size_t length);
  bool tncIsValid(char *frame);
  bool ax25IsValid(uint8_t *ax25, size_t length);
  String assembleAdr(char *str, uint8_t ssid, char carSep, bool carInsert);

private:
  Stream &serialPort;
};

#endif  // KISS_H
