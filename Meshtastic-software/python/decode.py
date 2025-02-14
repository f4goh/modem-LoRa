"""
longueurde la clé 32
meshtasticFullKeyHex d4f1bb3a20290759f0bcffabcf4e6901
##### PACKET DATA START #####
dest ffffffff sender 25c21b88
id 38757527
flags 63
chanhash 08
data 2d73fea3706ebf6a16e0
##### PACKET DATA END #####
{'dest': b'\xff\xff\xff\xff', 'sender': b'\x88\x1b\xc2%', 'packetID': b'\xa5r\xa4\xab', 'flags': b'c', 'channelHash': b'\x08', 'reserved': b'\x00\x00', 'data': b'-s\xfe\xa3pn\xbfj\x16\xe0'}
AES nonce is:  a572a4ab00000000881bc22500000000
AES key used:  b'1PG7OiApB1nwvP+rz05pAQ=='
Nonce length is: 16
dec: 08011204546573744800
b'\x08\x01\x12\x04TestH\x00'
data.portnum 1
TEXT_MESSAGE_APP b'\x88\x1b\xc2%' -> b'\xff\xff\xff\xff' Test
liste
https://buf.build/meshtastic/protobufs/file/8a1d0af7af794354a2e7ed04ed4a50ec:meshtastic/portnums.proto

remote-hardware on doit recompiler pour activer le module
https://meshtastic.org/docs/configuration/module/remote-hardware/

"""


import base64
from base64 import b64encode, b64decode
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes
from cryptography.hazmat.backends import default_backend
from meshtastic import protocols, mesh_pb2, admin_pb2, portnums_pb2, telemetry_pb2, mqtt_pb2

##### START FUNCTIONS BLOCK #####

# Takes in a string encoded as hex, and emits them as a bytes encoded of the same hex representation

def hexStringToBinary(hexString):
    binString = bytes.fromhex(hexString)
    return binString

def bytesToHexString(byteString):
    hexString = byteString.hex()
    return hexString

def msb2lsb(msb):
    #string version of this. ONLY supports 32 bit from the sender/receiver ID. Hacky
    lsb = msb[6] + msb[7] + msb[4] + msb[5] + msb[2] + msb[3] + msb[0] + msb[1]
    return lsb

##### END FUNCTIONS BLOCK #####

##### START AES KEY ASSIGNMENT BLOCK #####
#decode depuis la base 64
#https://base64.guru/converter/decode/hex
#https://www.rapidtables.com/web/tools/base64-decode.html

def parseAESKey():
    meshtasticFullKeyBase64 = "1PG7OiApB1nwvP+rz05pAQ=="
    aesKeyLength = len(base64.b64decode(meshtasticFullKeyBase64).hex())
    print("longueurde la clé",aesKeyLength)
    return base64.b64decode(meshtasticFullKeyBase64.encode('ascii'))
    
##### END AES KEY ASSIGNMENT BLOCK #####


##### START DATA EXTRACTION BLOCK #####

def dataExtractor(data):

    # Now we split the data into the appropriate meshtastic packet structure using https://meshtastic.org/docs/overview/mesh-algo/
    # NOTE: The data coming out of GnuRadio is MSB or big endian. We have to reverse byte order after this step.

    # destination : 4 bytes 
    # sender      : 4 bytes
    # packetID    : 4 bytes
    # flags       : 1 byte
    # channelHash : 1 byte
    # reserved    : 2 bytes
    # data        : 0-237 bytes

    meshPacketHex = {
        'dest' : hexStringToBinary(data[0:8]),
        'sender' : hexStringToBinary(data[8:16]),
        'packetID' : hexStringToBinary(data[16:24]),
        'flags' : hexStringToBinary(data[24:26]),
        'channelHash' : hexStringToBinary(data[26:28]),
        'reserved' : hexStringToBinary(data[28:32]),
        'data' : hexStringToBinary(data[32:len(data)])
    }
    print("##### PACKET DATA START #####")
    print("dest "   + msb2lsb(str(meshPacketHex['dest'].hex())) + " sender " + msb2lsb(str(meshPacketHex['sender'].hex())) )
    print("id "     + msb2lsb(str(int(meshPacketHex['packetID'].hex(),16))) )
    print("flags "  + str(meshPacketHex['flags'].hex()))
    print("chanhash "  + str(meshPacketHex['channelHash'].hex()))
    print("data "   + str(meshPacketHex['data'].hex()))
    print("##### PACKET DATA END #####")
    return meshPacketHex

##### END DATA EXTRACTION BLOCK #####
#valide
#https://cryptii.com/pipes/aes-encryption 


def dataDecryptor(meshPacketHex, aesKey):

    # Build the nonce. This is (packetID)+(00000000)+(sender)+(00000000) for a total of 128bit
    # Even though sender is a 32 bit number, internally its used as a 64 bit number.
    # Needs to be a bytes array for AES function.

    aesNonce = meshPacketHex['packetID'] + b'\x00\x00\x00\x00' + meshPacketHex['sender'] + b'\x00\x00\x00\x00'

    print("AES nonce is: ", aesNonce.hex())
    print("AES key used: ", str(b64encode(aesKey)))
    print("Nonce length is:", len(aesNonce) )


    # Initialize the cipher
    cipher = Cipher(algorithms.AES(meshtasticFullKeyHex), modes.CTR(aesNonce), backend=default_backend())
    decryptor = cipher.decryptor()

    # Do the decryption. Note, that this cipher is reversible, so running the cipher on encrypted gives decrypted, and running the cipher on decrypted gives encrypted.
    decryptedOutput = decryptor.update(meshPacketHex['data']) + decryptor.finalize()
    
    print("dec: "+ decryptedOutput.hex())
    print(decryptedOutput)
    return decryptedOutput

###### END DECRYPTION PROCESS #####

##### START PROTOBUF DECODER #####

def decodeProtobuf(packetData, sourceID, destID):

    data = mesh_pb2.Data()
    try:
        data.ParseFromString(packetData)
    except:
        data = "INVALID PROTOBUF"
        return data
    print("data.portnum",data.portnum)
    match data.portnum :
        case 0 : # UNKNOWN_APP
            data = "UNKNOWN_APP To be implemented"
        case 1 : # TEXT_MESSAGE_APP
            text_payload = data.payload.decode('utf-8')
            #print(text_payload)
            data = "TEXT_MESSAGE_APP " + str(sourceID) + " -> " + str(destID) + " " + str(text_payload)
        case 2 : # REMOTE_HARDWARE_APP
            data = "REMOTE_HARDWARE_APP To be implemented"
        case 3 : # POSITION_APP
            pos = mesh_pb2.Position()
            pos.ParseFromString(data.payload)
            latitude = pos.latitude_i * 1e-7
            longitude = pos.longitude_i * 1e-7
            data="POSITION_APP " + str(sourceID) + " -> " + str(destID) + " " + str(latitude) +"," + str(longitude)
        case 4 : # NODEINFO_APP
            info = mesh_pb2.User()
            try:
                info.ParseFromString(data.payload)
            except:
                print("Unknown Nodeinfo_app parse error")
            data = "NODEINFO_APP " + str(info)
        case 5 : # ROUTING_APP
            rtng = mesh_pb2.Routing()
            rtng.ParseFromString(data.payload)
            data = "TELEMETRY_APP "  + str(rtng)
        case 6 : # ADMIN_APP
            admn = admin_pb2.AdminMessage()
            admn.ParseFromString(data.payload)
            data = "ADMIN_APP " + str(admn) 
        case 7 : # TEXT_MESSAGE_COMPRESSED_APP
            data = "TEXT_MESSAGE_COMPRESSED_APP To be implemented"
        case 10 : # DETECTION_SENSOR_APP
            data = "DETECTION_SENSOR_APP To be implemented"
        case 32 : # REPLY_APP
            data = "REPLY_APP To be implemented"
        case 33 : # IP_TUNNEL_APP
            data = "IP_TUNNEL_APP To be implemented"
        case 34 : # PAXCOUNTER_APP
            data = "PAXCOUNTER_APP To be implemented"
        case 64 : # SERIAL_APP
            print(" ")
        case 65 : # STORE_FORWARD_APP
            sfwd = mesh_pb2.StoreAndForward()
            sfwd.ParseFromString(data.payload)
            data = "STORE_FORWARD_APP " + str(sfwd)
        case 67 : # TELEMETRY_APP
            env = telemetry_pb2.Telemetry()
            env.ParseFromString(data.payload)
            data = "TELEMETRY_APP " + str(env)
        case 68 : # ZPS_APP
            z_info = mesh_pb2.zps()
            z_info.ParseFromString(data.payload)
            data = "ZPS_APP " + str(z_info)
        case 69 : # SIMULATOR_APP
            data = "SIMULATOR_APP To be implemented"
        case 70 : # TRACEROUTE_APP
            trct= mesh_pb2.RouteDiscovery()
            trct.ParseFromString(data.payload)
            data = "TRACEROUTE_APP " + str(sourceID) + " -> " + str(destID) + " " + str(trct) 
        case 71 : # NEIGHBORINFO_APP
            ninfo = mesh_pb2.NeighborInfo()
            ninfo.ParseFromString(data.payload)
            data = "NEIGHBORINFO_APP " + str(ninfo)
        case 72 : # ATAK_PLUGIN
            data = "ATAK_PLUGIN To be implemented"
        case 73 : # MAP_REPORT_APP
            mrpt = mesh_pb2.MapReport()
            mrpt.ParseFromString(data.payload)
            data = "MAP_REPORT_APP " + str(mrpt) 
        case 74 : # POWERSTRESS_APP
            data = "POWERSTRESS_APP To be implemented"
        case 256 : # PRIVATE_APP
            data = "PRIVATE_APP To be implemented"
        case 257 : # ATAK_FORWARDER
            data = "ATAK_FORWARD To be implemented"
        case _:
            data = "UNKNOWN PROTOBUF"

    return data

##### END PROTOBUF DECODER #####


if __name__ == "__main__":
    meshtasticFullKeyHex = parseAESKey()
    print("meshtasticFullKeyHex",meshtasticFullKeyHex.hex())
    frame="FFFFFFFF881BC225A572A4AB630800002D73FEA3706EBF6A16E0"
    meshPacketHex = dataExtractor(frame)
    print(meshPacketHex)
    decryptedData = dataDecryptor(meshPacketHex, meshtasticFullKeyHex)    
    protobufMessage = decodeProtobuf(decryptedData,meshPacketHex['sender'],meshPacketHex['dest'])
    if protobufMessage == "INVALID PROTOBUF:" :
        print("INVALID PROTOBUF: ", end = '')
        print(decryptedData)
    else:
        print(protobufMessage)

    