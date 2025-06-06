#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <semaphore>
#include <chrono>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int PORT = 8080;
const int MAX_CLIENTS = 5;

struct ClientInfo {
    SOCKET socket;
    string nickname;
};

vector<ClientInfo> clientSockets;
mutex clientsMutex;
counting_semaphore<MAX_CLIENTS> clientSemaphore(MAX_CLIENTS);

void broadcastMessage(const string& msg, SOCKET senderSocket, const string& senderNick) {
    auto now = chrono::system_clock::now();
    time_t now_c = chrono::system_clock::to_time_t(now);
    tm local_tm;
    localtime_s(&local_tm, &now_c);

    char timeStr[10];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &local_tm);

    string fullMessage = senderNick + ": " + msg + "\n";

    lock_guard<mutex> lock(clientsMutex);
    for (const ClientInfo& client : clientSockets) {
        if (client.socket != senderSocket) {
            send(client.socket, fullMessage.c_str(), fullMessage.size(), 0);
        }
    }
}
void handleClient(SOCKET clientSocket) {
    // Receive nickname
    char nickBuffer[1024];
    memset(nickBuffer, 0, sizeof(nickBuffer));
    int nickBytes = recv(clientSocket, nickBuffer, sizeof(nickBuffer), 0);
    if (nickBytes <= 0) {
        cout << "Client failed to send nickname. Disconnecting." << endl;
        closesocket(clientSocket);
        clientSemaphore.release();
        return;
    }
    string nickname(nickBuffer);

    // Add client to list
    {
        lock_guard<mutex> lock(clientsMutex);
        clientSockets.push_back({clientSocket, nickname});
    }

    cout << "New client connected: " << nickname << " (SOCKET=" << clientSocket << ")" << endl;
    string joinMsg =  nickname + " has joined the chat\n";
    broadcastMessage(joinMsg, clientSocket, "");

    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesReceived <= 0) {
            cout << nickname << " disconnected." << endl;
            string leaveMsg = nickname + " has left the chat\n";
            broadcastMessage(leaveMsg, clientSocket, "");
            break;
        }

        string msg(buffer);

        if (msg == "__USERS__") {
            string userList = "Active users\n";
            {
                lock_guard<mutex> lock(clientsMutex);
                for (const auto& client : clientSockets) {
                    userList += client.nickname + "\n";
                }
            }
            send(clientSocket, userList.c_str(), userList.size(), 0);
        } else {
            broadcastMessage(msg, clientSocket, nickname);
            cout << nickname << ": " << msg << endl;
        }

    }

    {
        lock_guard<mutex> lock(clientsMutex);
        for (size_t i = 0; i < clientSockets.size(); ++i) {
            if (clientSockets[i].socket == clientSocket) {
                clientSockets.erase(clientSockets.begin() + i);
                break;
            }
        }
    }

    closesocket(clientSocket);
    clientSemaphore.release();
}

int main() {
    cout << "Simple Chat Server with Nicknames (Windows, C++20)" << endl;
    cout << "Kamil Nicos 263753" << endl;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed!" << endl;
        return 1;
    }

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        cerr << "Socket creation failed!" << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cerr << "Bind failed!" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "Listen failed!" << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    cout << "Server is listening on port " << PORT << "..." << endl;

    while (true) {
        sockaddr_in clientAddr{};
        int clientSize = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddr, &clientSize);

        if (clientSocket == INVALID_SOCKET) {
            cerr << "Accept failed!" << endl;
            continue;
        }

        if (!clientSemaphore.try_acquire()) {
            const char* fullMsg = "Chat is full. Try again later.\n";
            send(clientSocket, fullMsg, strlen(fullMsg), 0);
            shutdown(clientSocket, SD_SEND);
            this_thread::sleep_for(chrono::milliseconds(100));
            closesocket(clientSocket);
            continue;
        }


        thread t(handleClient, clientSocket);
        t.detach();
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}
