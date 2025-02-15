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
	

# Meshtastic-technical links	


```cpp
  LoRa.setPreambleLength(16);  //mesh
  LoRa.setSyncWord(0x2b);      //mesh
 ``` 


![Mestastic-inst](images/mesh_install_thonny01.png "th01")

