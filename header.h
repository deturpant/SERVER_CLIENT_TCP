#pragma once
#include <string>
#include <winsock2.h> 
#pragma comment(lib, "ws2_32.lib")  // Link with ws2_32.lib

#pragma pack(push, 1)  
struct Header {
    byte version;
    char identity[3];
    short ID_packet;
    char packetType;
    char command[30];
    int dataSize;
};

#pragma pack(pop) 
struct Message {
    Header header;
    char data[1024];
};

enum PacketType {
    ERROR_RESPONSE = 1,
    OK_RESPONSE,
    OK_REQUEST,
    ERROR_REQUEST
};
