    /***********************************************************
    * Base64 library implementation                            *
    * @author Ahmed Elzoughby adapted for arduino by F4GOH     *
    * @date July 23, 2017                                      *
    ***********************************************************/

#include "base64.h"


void base64_decode(char* cipher,uint8_t * plain) {

  uint8_t counts = 0;
  uint8_t buffer[4];
  int i = 0, p = 0;

  for (i = 0; cipher[i] != '\0'; i++) {
    uint8_t k;
    for (k = 0; k < 64 && base64_map[k] != cipher[i]; k++);
      ;
    buffer[counts++] = k;
    if (counts == 4) {
      plain[p++] = (buffer[0] << 2) + (buffer[1] >> 4);
      if (buffer[2] != 64)
        plain[p++] = (buffer[1] << 4) + (buffer[2] >> 2);
      if (buffer[3] != 64)
        plain[p++] = (buffer[2] << 6) + buffer[3];
      counts = 0;
    }
  }

//  plain[p] = '\0'; /* string padding character */
}
