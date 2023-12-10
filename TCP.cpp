#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "header.h"

#include <iostream>
#include <cstring>
#include <winsock2.h>  // Include the Windows socket header
#include <vector>
#include <thread>
#pragma comment(lib, "ws2_32.lib")  // Link with ws2_32.lib


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
    return true;
}

void sendMessageToClient(SOCKET client, std::string headerCMD, std::string message, PacketType type) {
    // Respond with information
    Header responseHeader;
    strcpy(responseHeader.identity, identify_num);
    responseHeader.ID_packet = 124;  // Example response packet ID
    responseHeader.packetType = type;   // Example response packet type
    strcpy(responseHeader.command, headerCMD.c_str());

    std::string responseData = message;
    int dataSize = responseData.size();
    std::cout << "Send message to client: " << client << " | Type: " << getStrOfPacketType(type) << std::endl;
    // Send the response header and data in a single block
    send(client, reinterpret_cast<char*>(&responseHeader), sizeof(responseHeader), 0);
    send(client, reinterpret_cast<char*>(&dataSize), sizeof(dataSize), 0);
    send(client, responseData.c_str(), dataSize, 0);
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
    listen(server, 5);

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
