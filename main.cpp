#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <winsock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

std::string g_responseBuffer;

std::string receiveLine(SOCKET sock) {
    size_t pos;
    while ((pos = g_responseBuffer.find("\r\n")) == std::string::npos) {
        char temp[4096];
        int bytesReceived = recv(sock, temp, sizeof(temp) - 1, 0);
        if (bytesReceived <= 0) {
            return "";
        }
        temp[bytesReceived] = '\0';
        g_responseBuffer += temp;
    }

    std::string line = g_responseBuffer.substr(0, pos + 2);
    g_responseBuffer.erase(0, pos + 2);
    return line;
}

void checkResponse(SOCKET sock, const std::string& expectedCode) {
    std::string line;
    std::string lastCode;
    bool done = false;

    while (!done) {
        line = receiveLine(sock);
        if (line.empty()) {
            std::cerr << "Error: Connection closed or receive error" << std::endl;
            return;
        }

        std::cout << "Server: " << line;

        if (line.length() >= 3) {
            lastCode = line.substr(0, 3);
            if (line.length() < 4 || line[3] != '-') {
                done = true;
            }
        }
    }

    if (lastCode != expectedCode) {
        std::cerr << "Error: Expected code " << expectedCode << " but got " << lastCode << std::endl;
    }
}

void sendCommand(SOCKET sock, const std::string& command) {
    std::cout << "Client: " << command;
    send(sock, command.c_str(), (int)command.length(), 0);
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