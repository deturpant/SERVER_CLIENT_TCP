#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "header.h"

#include <iostream>
#include <cstring>
#include <algorithm>

#include <winsock2.h>  // Include the Windows socket header
#pragma comment(lib, "ws2_32.lib")  // Link with ws2_32.lib
#include <thread>
#undef min

const char* identify_num = "\xAF\xAA\xAF";

bool isPacketValid(Header header) {
    for (int i = 0; i < 3; i++) {
        if (header.identity[i] != identify_num[i]) return false;
    }
    if (header.version != 1) return false;
    return true;
}

void printMenu() {
    std::cout << "GET_INFO -> get some infromation from server\n";
    std::cout << "GET_RANDOM_VALUE -> get random value from server\n";
    std::cout << "GET_CURRENT_TIME -> get current time from server\n";
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


void receiveMessages(SOCKET serverSocket) {
    Message message;
    while (true) {
        int bytesReceived = recv(serverSocket, reinterpret_cast<char*>(&message.header), sizeof(message.header), 0);
        if (bytesReceived == sizeof(message.header) && isPacketValid(message.header)) {
            // Receive the data
            if (message.header.dataSize > 0) {
                int bytesRead = recv(serverSocket, message.data, (message.header.dataSize < sizeof(message.data)) ? message.header.dataSize : sizeof(message.data), 0);
                if (bytesRead > 0) {
                    std::string receivedData(message.data, bytesRead);
                    std::cout << "\nReceived packet from server: Type " << getStrOfPacketType((PacketType)message.header.packetType)
                        << ", ID " << message.header.ID_packet << ", Command: " << message.header.command << std::endl;

                    std::cout << "Received data from server: " << receivedData << std::endl;
                }
            }
            else {
            }
        }
    }
}




int main(int argc, char* argv[]) {
    // Check if IP address and port are provided
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <server_ip> <server_port>" << std::endl;
        return 1;
    }

    const char* serverIp = argv[1];
    const int serverPort = std::stoi(argv[2]);

    // Initialize Winsock
    WSADATA wsData;
    if (WSAStartup(MAKEWORD(2, 2), &wsData) != 0) {
        std::cerr << "Failed to initialize Winsock." << std::endl;
        return 1;
    }

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Failed to create socket." << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in server_data;
    server_data.sin_family = AF_INET;
    server_data.sin_port = htons(serverPort);
    server_data.sin_addr.s_addr = inet_addr(serverIp);

    if (connect(clientSocket, reinterpret_cast<struct sockaddr*>(&server_data), sizeof(server_data)) == SOCKET_ERROR) {
        std::cerr << "Failed to connect to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Start a separate thread to receive messages from the server
    std::thread(receiveMessages, clientSocket).detach();
    printMenu();
    while (true) {
        // Read a message from the console
        std::string message;
        std::cout << "Enter command: ";
        std::getline(std::cin, message);

        // Prepare the packet
        Header header;
        int randIDPacket = rand() % 30001;
        strcpy(header.identity, identify_num);
        header.version = 1;
        header.ID_packet = randIDPacket;
        header.packetType = PacketType::OK_REQUEST;
        strncpy(header.command, message.c_str(), sizeof(header.command) - 1);
        header.command[sizeof(header.command) - 1] = '\0';

        // Send the header
        send(clientSocket, reinterpret_cast<char*>(&header), sizeof(header), 0);

        // Send the message
        send(clientSocket, message.c_str(), message.size(), 0);
    }

    // Cleanup Winsock
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
