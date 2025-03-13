/*
With repeated int32
need callback, more harder

Some help
https://groups.google.com/g/nanopb/c/0Xd3eTq5q74
https://jpa.kapsi.fi/nanopb/docs/reference.html
https://stackoverflow.com/questions/45979984/creating-callbacks-and-structs-for-repeated-field-in-a-protobuf-message-in-nanop
https://github.com/nanopb/nanopb/tree/master/examples/
https://protogen.marcgravell.com/decode

syntax = "proto2";

message SimpleMessage {
    repeated int32 number = 1;
}

Encoded size: 12
Encoded Message:
08f60108900708aa0c088008
Bytes decoded: 12
NB numbers decode: 4
123
456
789
512

*/

#include "SimpleMessage.pb.h"
#include "pb_common.h"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"


#define MAX_NUMBERS 32

typedef struct {
  int32_t numbers[MAX_NUMBERS];
  int32_t numbers_count;
} IntList;

// add a number to the int list
void IntList_add_number(IntList *list, int32_t number) {
  if (list->numbers_count < MAX_NUMBERS) {
    list->numbers[list->numbers_count] = number;
    list->numbers_count++;
  }
}


void setup() {
  Serial.begin(115200);
  uint8_t buffer[128];
  size_t total_bytes_encoded = 0;
  IntList actualData = { 0 };
  IntList_add_number(&actualData, 123);
  IntList_add_number(&actualData, 456);
  IntList_add_number(&actualData, 789);
  IntList_add_number(&actualData, 512);

  // prepare the nanopb ENCODING callback
  SimpleMessage msg = SimpleMessage_init_zero;
  msg.number.arg = &actualData;
  msg.number.funcs.encode = SimpleMessage_encode_numbers;

  // call nanopb encoding
  pb_ostream_t ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  if (!pb_encode(&ostream, SimpleMessage_fields, &msg)) {
    const char *error = PB_GET_ERROR(&ostream);
    Serial.print("Failed to encode error :");
    Serial.println(error);
    return;
  }

  total_bytes_encoded = ostream.bytes_written;
  Serial.print("Encoded size: ");
  Serial.println(total_bytes_encoded);

  Serial.println("Encoded Message:");
  char hexBuf[3];

  for (int i = 0; i < total_bytes_encoded; i++) {
    sprintf(hexBuf, "%02x", buffer[i]);
    Serial.print(hexBuf);
  }
  Serial.println();

  // empty array for decoding
  IntList decodedData = { 0 };

  // prepare the nanopb DECODING callback
  SimpleMessage msg_decode = SimpleMessage_init_zero;
  msg_decode.number.arg = &decodedData;
  msg_decode.number.funcs.decode = SimpleMessage_decode_single_number;

  // call nanopb decoding
  pb_istream_t istream = pb_istream_from_buffer(buffer, total_bytes_encoded);
  if (!pb_decode(&istream, SimpleMessage_fields, &msg_decode)) {
    const char *error = PB_GET_ERROR(&istream);
    Serial.print("Failed to encode error :");
    Serial.println(error);
    return;
  }
  Serial.print("Bytes decoded: ");
  Serial.println(total_bytes_encoded - istream.bytes_left);
  Serial.print("NB numbers decode: ");
  Serial.println(decodedData.numbers_count);
  for (int i = 0; i < decodedData.numbers_count; i++) {
    Serial.println(decodedData.numbers[i]);
  }
}

void loop() {}

// Callback d'encodage pour repeated Data
bool SimpleMessage_encode_numbers(pb_ostream_t *ostream, const pb_field_t *field, void *const *arg) {
  IntList *source = (IntList *)(*arg);

  // encode all numbers
  for (int i = 0; i < source->numbers_count; i++) {
    if (!pb_encode_tag_for_field(ostream, field)) {
      const char *error = PB_GET_ERROR(ostream);
      printf("SimpleMessage_encode_numbers error: %s", error);
      return false;
    }

    if (!pb_encode_svarint(ostream, source->numbers[i])) {
      const char *error = PB_GET_ERROR(ostream);
      printf("SimpleMessage_encode_numbers error: %s", error);
      return false;
    }
  }

  return true;
}

bool SimpleMessage_decode_single_number(pb_istream_t *istream, const pb_field_t *field, void **arg) {
  IntList *dest = (IntList *)(*arg);

  // decode single number
  int64_t number;
  if (!pb_decode_svarint(istream, &number)) {
    const char *error = PB_GET_ERROR(istream);
    printf("SimpleMessage_decode_single_number error: %s", error);
    return false;
  }

  // add to destination list
  IntList_add_number(dest, (int32_t)number);
  return true;
}