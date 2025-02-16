# LoRa simple text modem (no protobuf)

- LoRa_modem_noproto.ino

```
LoRa simple text modem
message send : Hello
Received packet 'good morning' with RSSI -58
```

# protobuf exemples

- quick_protobuf_tuto

	- Basic from [protobuf tutorial](https://techtutorialsx.com/2018/10/19/esp32-esp8266-arduino-protocol-buffers/) (very simple)

- Transmit data with LoRa RF with protobuf exemple

	- TX_proto_lora.ino

- Receive data with LoRa RF with protobuf exemple

	- RX_proto_lora.ino



https://protobuf.dev/programming-guides/proto3/

  
|Proto Type	|Notes|
|---    |:--    |
|double	|	|
|float	|	|
|int32	|Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint32 instead.|
|int64	|Uses variable-length encoding. Inefficient for encoding negative numbers – if your field is likely to have negative values, use sint64 instead.|
|uint32	|Uses variable-length encoding.|
|uint64	|Uses variable-length encoding.|
|sint32	|Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int32s.|
|sint64	|Uses variable-length encoding. Signed int value. These more efficiently encode negative numbers than regular int64s.|
|fixed32 |	Always four bytes. More efficient than uint32 if values are often greater than 228.|
|fixed64 |	Always eight bytes. More efficient than uint64 if values are often greater than 256.|
|sfixed32 |	Always four bytes.|
|sfixed64 |	Always eight bytes.|
|bool	|	|
|string	|A string must always contain UTF-8 encoded or 7-bit ASCII text, and cannot be longer than 2<sup>32sup>.|
|bytes	|May contain any arbitrary sequence of bytes no longer than 2<sup>32sup>.|


# Meshtastic-technical links	


```cpp
  LoRa.setPreambleLength(16);  //mesh
  LoRa.setSyncWord(0x2b);      //mesh
 ``` 


```
protoc --nanopb_out=. weather.proto
```

![Mestastic-inst](images/mesh_install_thonny01.png "th01")

