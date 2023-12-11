#pragma once

#include <winsock2.h>  // Include the Windows socket header
#pragma comment(lib, "ws2_32.lib")  // Link with ws2_32.lib

#pragma pack(push, 1)  // Pack structure tightly
struct Header {
    byte version;
    char identity[3];
    short ID_packet;
    char packetType;
    char command[16];
};
#pragma pack(pop)  // Restore default packing

enum PacketType {
    ERROR_RESPONSE = 1,
    OK_RESPONSE,
    OK_REQUEST,
    ERROR_REQUEST
};

