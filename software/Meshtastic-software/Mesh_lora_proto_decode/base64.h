/***********************************************************
* Base64 library                                           *
* @author Ahmed Elzoughby adapted for arduino by F4GOH     *
* @date July 23, 2017                                      *
* Purpose: decode base64 format                            *
***********************************************************/

#ifndef BASE64_H
#define BASE64_H
#include <stdint.h>
#include <Arduino.h>

const char base64_map[] PROGMEM = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                    'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/' };

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************
decodes base64 format char into bytes
@param plain encoded base64 char
@return  bytes to be encoded
***********************************************/
void base64_decode(char* cipher,uint8_t * plain);

#ifdef __cplusplus
}
#endif

#endif  //BASE64_H
