/*
https://techtutorialsx.com/2018/10/19/esp32-esp8266-arduino-protocol-buffers/

syntax = "proto2";
 
message TestMessage {
    required int32 test_number = 1;
}
https://protobuf-decoder.netlify.app/
Results in the serial console
Message Length: 3
Message: 
089c04
Decoded test_number: 540

*/

#include "test.pb.h"

#include "pb_common.h"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

void setup() {

  Serial.begin(115200);

  uint8_t buffer[128];

  TestMessage message = TestMessage_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  message.test_number = 540;

  bool status = pb_encode(&stream, TestMessage_fields, &message);

  if (!status) {
    Serial.println("Failed to encode");
    return;
  }

  Serial.print("Message Length: ");
  Serial.println(stream.bytes_written);

  Serial.println("Message: ");

  char hexBuf[3];

  for (int i = 0; i < stream.bytes_written; i++) {
    sprintf(hexBuf, "%02x", buffer[i]);
    Serial.print(hexBuf);
  }
  Serial.println();
  // Décodage
  TestMessage decoded_message = TestMessage_init_zero;

  pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);

  status = pb_decode(&istream, TestMessage_fields, &decoded_message);

  if (!status) {
    Serial.println("Failed to decode");
    return;
  }

  Serial.print("Decoded test_number: ");
  Serial.println(decoded_message.test_number);
}

void loop() {}