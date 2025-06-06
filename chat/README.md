# Chat – Client-Server (C++20, Windows)

## Project Description

This project implements a simple multi-client chat application in C++ using Winsock2 for TCP communication on Windows.  
The server can handle up to 5 clients simultaneously. Each client connects with a nickname and can send messages to all others.  
Messages are timestamped, and clients can use `/users` to list active users or `/exit` to leave the chat.  
The server handles each client in a separate thread and synchronizes access to shared data using mutexes.

- Multiple clients (up to 5)
- Nicknames
- Timestamped messages
- Message broadcasting
- Commands like `/exit` and `/users`

## Requirements

- Windows OS
- C++20 compatible compiler (MSVC)
- CMake or Visual Studio / CLion
- Linked with `ws2_32.lib`

## How to Run

### 1. Run the server

Open a terminal and run:

```bash
./chat_server.exe
```

### 2. Run clients (in separate terminals)

Open multiple terminals and run:

```bash
./chat_client.exe
```

You can connect up to 5 clients.

## Usage Instructions

- When prompted, enter your **nickname**
- Type messages and press **Enter** to send
- Supported commands:
  - `/exit` – disconnect from the server and close the client
  - `/users` – display list of currently connected users

Each message is automatically timestamped using local system time.

## Multithreading

### Server

- For each new client, a **separate thread** is launched to handle communication
- All access to the global client list is protected with a **mutex**
- A **counting semaphore** limits the number of active clients to 5
- Messages are broadcasted to all clients except the sender

### Client

- One thread handles **receiving messages**
- Another thread handles **sending input messages**

## Example

```text
[17:08:03] Hello everyone!
[17:08:05] /users
Active users:
Kamil
Mietek
```

## Author

Kamil Nicos  
Student ID: 263753  
Wrocław University of Science and Technology  
June 2025