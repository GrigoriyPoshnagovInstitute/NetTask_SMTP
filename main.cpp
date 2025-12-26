#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

void checkResponse(SOCKET sock, const std::string& expectedCode) {
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0) {
        std::cout << "Server: " << buffer;
        std::string response(buffer);
        if (response.substr(0, 3) != expectedCode) {
            std::cerr << "Error: Expected code " << expectedCode << " but got " << response.substr(0, 3) << std::endl;
        }
    } else {
        std::cerr << "Error receiving data" << std::endl;
    }
}

void sendCommand(SOCKET sock, const std::string& command) {
    std::cout << "Client: " << command;
    send(sock, command.c_str(), command.length(), 0);
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return 1;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(25);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) != 0) {
        std::cerr << "Connection failed" << std::endl;
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    checkResponse(sock, "220");

    sendCommand(sock, "HELO localhost\r\n");
    checkResponse(sock, "250");

    sendCommand(sock, "MAIL FROM: <sender@example.com>\r\n");
    checkResponse(sock, "250");

    sendCommand(sock, "RCPT TO: <recipient@example.com>\r\n");
    checkResponse(sock, "250");

    sendCommand(sock, "DATA\r\n");
    checkResponse(sock, "354");

    std::string body = "Subject: Test Email\r\n\r\nHello, this is a test email sent from C++ socket.\r\n.\r\n";
    sendCommand(sock, body);
    checkResponse(sock, "250");

    sendCommand(sock, "QUIT\r\n");
    checkResponse(sock, "221");

    closesocket(sock);
    WSACleanup();
    return 0;
}