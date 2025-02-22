#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "header.h"
#include <iostream>
#include <cstring>
#include <winsock2.h>  // Include the Windows socket header
#include <vector>
#include <thread>
#include <time.h>
#pragma comment(lib, "ws2_32.lib")  // Link with ws2_32.lib

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);

    return buf;
}

std::string getStrOfPacketType(PacketType pt) {
    switch (pt) {
    case PacketType::OK_RESPONSE: {
        return "RESPONSE_OK";
    }
    case PacketType::ERROR_RESPONSE: {
         return "RESPONSE_ERROR";
    }
    case PacketType::OK_REQUEST: {
        return "REQUEST_OK";
    }
    case PacketType::ERROR_REQUEST: {
        return "REQUEST_ERROR";
    }
    }
}

std::vector<ClientData> clients;
const char* identify_num = "\xAF\xAA\xAF";

bool isPacketValid(Header header) {
    const char* identify_num = "\xAF\xAA\xAF";
    for (int i = 0; i < 3; i++) {
        if (header.identity[i] != identify_num[i]) return false;
    }
    if (header.version != 1) return false;
    return true;
}

void sendMessageToClient(SOCKET client, std::string headerCMD, std::string message, PacketType type) {
    // Respond with information
    int randIDPacket = rand() % 30001;
    Message responseMessage;
    responseMessage.header.version = 1;
    strcpy(responseMessage.header.identity, identify_num);
    responseMessage.header.ID_packet = randIDPacket;
    responseMessage.header.packetType = type;
    strcpy(responseMessage.header.command, headerCMD.c_str());

   
    char* cstr = new char[message.length() + 1];
    strcpy(cstr, message.c_str());
    responseMessage.header.dataSize = strlen(cstr);
    std::cout << "Send message to client: " << client << " | Type: " << getStrOfPacketType(type) << std::endl;

    send(client, reinterpret_cast<char*>(&responseMessage.header), sizeof(responseMessage.header), 0);
    send(client, cstr, responseMessage.header.dataSize, 0);

}


void handleClient(SOCKET clientSocket) {
    Header header;
    while (true) {
        int bytesReceived = recv(clientSocket, reinterpret_cast<char*>(&header), sizeof(header), 0);
        if (bytesReceived == sizeof(header) && isPacketValid(header)) {
            std::cout << "Received packet from client " << clientSocket << ": Type " << getStrOfPacketType(PacketType(header.packetType)) << ", ID " << header.ID_packet << ", Command: " << header.command << std::endl;

            // Handle the command
            if (strcmp(header.command, "GET_INFO") == 0) {
                sendMessageToClient(clientSocket, "INFO_RESPONSE", "This is response to your GET_INFO command", PacketType::OK_RESPONSE);
            }
            else if (strcmp(header.command, "GET_RANDOM_VALUE") == 0) {
                std::string randomValue = std::to_string(rand());
                randomValue = "Your random value ---> " + randomValue;
                sendMessageToClient(clientSocket, "RANDOM_RESPONSE", randomValue, PacketType::OK_RESPONSE);
            }
            else if (strcmp(header.command, "GET_CURRENT_TIME") == 0) {
                sendMessageToClient(clientSocket, "CURRENT_TIME_RESPONSE", currentDateTime(), PacketType::OK_RESPONSE);
            }
            else {
                sendMessageToClient(clientSocket, "ERROR_RESPONSE", "Unknown command", PacketType::ERROR_RESPONSE);
            }
        }
    }
}


int main() {
    // Initialize Winsock
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_data;
    server_data.sin_family = AF_INET;
    server_data.sin_port = htons(12222); // port
    server_data.sin_addr.s_addr = inet_addr("127.0.0.1");

    bind(server, reinterpret_cast<struct sockaddr*>(&server_data), sizeof(server_data));
    listen(server, SOMAXCONN);

    std::cout << "Server started on 12222 port\n";

    while (true) {
        SOCKET clientSocket = accept(server, nullptr, nullptr);
        std::cout << "New client connected: " << clientSocket << std::endl;

        clients.push_back({ clientSocket });

        // Start a new thread to handle the client
        std::thread(handleClient, clientSocket).detach();
    }

    // Cleanup Winsock
    closesocket(server);
    WSACleanup();

    return 0;
}
