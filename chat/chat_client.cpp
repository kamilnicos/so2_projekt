#include <iostream>
#include <thread>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <chrono>
#include <iomanip>
#include <sstream>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

bool isRunning = true;

string currentTime() {
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    tm local_tm;
    localtime_s(&local_tm, &now_c);
    stringstream ss;
    ss << put_time(&local_tm, "%H:%M:%S");
    return ss.str();
}

void receiveMessages(SOCKET sock) {
    char buffer[1024];
    while (isRunning) {
        memset(buffer, 0, sizeof(buffer));
        int bytes = recv(sock, buffer, sizeof(buffer), 0);
        if (bytes <= 0) {
            cout << "Disconnected from server." << endl;
            isRunning = false;
            break;
        }
        cout << buffer;
    }
}

void sendMessages(SOCKET sock) {
    string message;
    while (isRunning) {
        getline(cin, message);

        if (message == "/exit") {
            isRunning = false;
            shutdown(sock, SD_SEND);
            break;
        }

        if (message == "/users") {
            send(sock, "__USERS__", strlen("__USERS__"), 0);
            continue;
        }

        string fullMessage = "[" + currentTime() + "] " + message;
        send(sock, fullMessage.c_str(), fullMessage.size(), 0);
    }
}


int main() {
    cout << "Simple Chat Client with Nicknames (Windows, C++20)" << endl;
    cout << "Kamil Nicos 263753" << endl;

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return 1;
    }

    // Create socket
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed!" << endl;
        WSACleanup();
        return 1;
    }

    // Set server address
    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    // Connect to server
    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Connection failed!" << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    // Make socket non-blocking temporarily
    u_long nonBlocking = 1;
    ioctlsocket(clientSocket, FIONBIO, &nonBlocking);

    // Try to read "Chat is full" message immediately
    char initialBuffer[1024] = {0};
    int initialBytes = recv(clientSocket, initialBuffer, sizeof(initialBuffer), 0);

    // Restore socket to blocking mode
    nonBlocking = 0;
    ioctlsocket(clientSocket, FIONBIO, &nonBlocking);

    if (initialBytes > 0) {
        cout << initialBuffer << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 0;
    }

    // Send nickname
    cout << "Enter your nickname: ";
    string nickname;
    getline(cin, nickname);
    send(clientSocket, nickname.c_str(), nickname.size(), 0);

    cout << "Connected to server. Type messages and press ENTER. Type '/exit' to quit. Type '/users' to view the list of users" << endl;

    thread tRecv(receiveMessages, clientSocket);
    thread tSend(sendMessages, clientSocket);

    tRecv.join();
    tSend.join();

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}