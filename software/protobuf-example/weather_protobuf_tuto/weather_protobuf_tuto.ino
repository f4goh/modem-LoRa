/*
https://github.com/nanopb/nanopb/tree/master/examples/simple

syntax = "proto2";

message Data {
  required float temperature     = 1;
  required int32 humidity        = 2;
  required int32 air_pressure    = 3;
  required int32 wind_speed      = 4;
  required int32 wind_direction  = 5;
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
 
  Data message = Data_init_zero;
 
  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
 
  message.temperature = 18.5;
  message.humidity = 60;
  message.air_pressure = 1015;
  message.wind_speed = 15;
  message.wind_direction = 260;
  
  bool status = pb_encode(&stream, Data_fields, &message);
 
  if (!status)
  {
      Serial.println("Failed to encode");
      return;
  }
 
  Serial.print("Message Length: ");
  Serial.println(stream.bytes_written);
 
  Serial.println("Message: ");
 
  for(int i = 0; i<stream.bytes_written; i++){
    Serial.print(buffer[i],HEX);
    Serial.print(',');
  }
 
  // Décodage
  Data decoded_message = Data_init_zero;
  
  pb_istream_t istream = pb_istream_from_buffer(buffer, stream.bytes_written);

  status = pb_decode(&istream, Data_fields, &decoded_message);

  if (!status) {
      Serial.println("Failed to decode");
      return;
  }

  Serial.println("Decoded message: ");
  Serial.println(decoded_message.temperature);
  Serial.println(decoded_message.humidity);
  Serial.println(decoded_message.air_pressure);
  Serial.println(decoded_message.wind_speed);
  Serial.println(decoded_message.wind_direction);

}
 
void loop() {}