/*
With repeated message
need callback, more harder

Some help
https://groups.google.com/g/nanopb/c/0Xd3eTq5q74
https://jpa.kapsi.fi/nanopb/docs/reference.html
https://github.com/nanopb/nanopb/tree/master/examples/
https://protogen.marcgravell.com/decode


syntax = "proto2";

message Data {
  required float temperature     = 1;
  required int32 humidity        = 2;
  required int32 air_pressure    = 3;
  required int32 wind_speed      = 4;
  required int32 wind_direction  = 5;
}

message DataHistory {
  repeated Data data = 1;
}

*/

#include "weather.pb.h"
#include "pb_common.h"
#include "pb.h"
#include "pb_encode.h"
#include "pb_decode.h"

void setup() {
  Serial.begin(115200);
  uint8_t buffer[128];

  // Création d'une instance de DataHistory
  DataHistory history = DataHistory_init_zero;
  
  // Création de deux données Data
  Data data_entries[2] = {Data_init_zero, Data_init_zero};
  
  data_entries[0].temperature = 18.5;
  data_entries[0].humidity = 60;
  data_entries[0].air_pressure = 1015;
  data_entries[0].wind_speed = 15;
  data_entries[0].wind_direction = 260;

  data_entries[1].temperature = 20.1;
  data_entries[1].humidity = 55;
  data_entries[1].air_pressure = 1012;
  data_entries[1].wind_speed = 10;
  data_entries[1].wind_direction = 270;

  // Utilisation d'un callback pour gérer le repeated Data
  history.data.funcs.encode = &data_encode_callback;
  history.data.arg = &data_entries;
  
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
  
  bool status = pb_encode(&stream, DataHistory_fields, &history);
  if (!status) {
      Serial.println("Failed to encode");
      return;
  }
  
  Serial.print("Message Length: ");
  Serial.println(stream.bytes_written);
  Serial.println("Encoded Message:");
  char hexBuf[3];

  for (int i = 0; i < stream.bytes_written; i++) {
    sprintf (hexBuf, "%02x", buffer[i]);
    Serial.print(hexBuf);
  }
  Serial.println();
  
  // Décodage
  DataHistory decoded_history = DataHistory_init_zero;
  Data decoded_entries[2]; // Stocke les entrées décodées
  decoded_history.data.funcs.decode = &data_decode_callback;
  decoded_history.data.arg = &decoded_entries;
  
  pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);
  
  status = pb_decode(&istream, DataHistory_fields, &decoded_history);
  if (!status) {
      Serial.println("Failed to decode");
      return;
  }
  
  Serial.println("Decoded DataHistory:");
  for (int i = 0; i < 2; i++) { // On sait qu'on a 2 éléments
      Serial.print("Entry "); Serial.println(i);
      Serial.println(decoded_entries[i].temperature);
      Serial.println(decoded_entries[i].humidity);
      Serial.println(decoded_entries[i].air_pressure);
      Serial.println(decoded_entries[i].wind_speed);
      Serial.println(decoded_entries[i].wind_direction);
  }
}

void loop() {}

// Callback d'encodage pour repeated Data
bool data_encode_callback(pb_ostream_t *stream, const pb_field_iter_t *field, void * const *arg) {
    Data *data_array = (Data *)(*arg);
    for (int i = 0; i < 2; i++) { // Encode toujours 2 éléments ici
        if (!pb_encode_tag_for_field(stream, field)){
          const char * error = PB_GET_ERROR(stream);
            printf("SimpleMessage_encode error: %s", error);
            return false;
        }
        if (!pb_encode_submessage(stream, Data_fields, &data_array[i])){
           const char * error = PB_GET_ERROR(stream);
            printf("SimpleMessage error: %s", error);
            return false;
        }
    }
    return true;
}

bool data_decode_callback(pb_istream_t *stream, const pb_field_iter_t *field, void **arg) {
    static int index = 0;
    Data *data_array = (Data *)(*arg);

    if (index >= 2) return false; // Empêche d'aller au-delà du tableau de taille 2

    // Décodage d'un message Data
    if (!pb_decode(stream, Data_fields, &data_array[index])) {
        Serial.println("Decoding failed");
        return false;
    }

    index++; // Incrémenter l'index après chaque décodage
    return true;
}


