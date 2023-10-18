#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <map>
#include <fstream>
#include <thread>
#include "json.hpp"
#include <mutex>

void SaveUserData(const std::map<std::string, std::string>& users, std::mutex& saveMutex) {
    nlohmann::json jsonData;
    for (const auto& entry : users) {
        jsonData[entry.first] = entry.second;
    }

    std::ofstream userFile("users.json");
    if (userFile.is_open()) {
        userFile << jsonData.dump();
        userFile.close();
    }
    else {
        std::cerr << "Error saving user data to file." << std::endl;
    }

   // saveMutex.unlock();
}

std::map<std::string, std::string> LoadUserData() {
    std::map<std::string, std::string> users;
    std::ifstream userFile("users.json");
    if (userFile.is_open()) {
        nlohmann::json jsonData;
        userFile >> jsonData;

        for (auto it = jsonData.begin(); it != jsonData.end(); ++it) {
            users[it.key()] = it.value();
        }

        userFile.close();
    }
    else {
        std::cerr << "Error loading user data from file." << std::endl;
    }
    return users;
}

int main() {
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);

    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        std::cout << "WSAStartup failed with error: " << wsaerr << std::endl;
        return 1;
    }

    std::cout << "The Winsock DLL found!" << std::endl;
    std::cout << "The Status: " << wsaData.szSystemStatus << std::endl;

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cout << "Error at socket(): " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(8080);

    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        std::cout << "bind() failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        std::cout << "listen() failed: " << WSAGetLastError() << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    else {
        std::cout << "listen() is good, waiting for connections..." << std::endl;
    }

    std::map<std::string, std::string> users = LoadUserData();
    std::mutex saveMutex;  // Mutex for synchronization
    std::thread saveThread([&users, &saveMutex] { SaveUserData(users, saveMutex); });

    while (true) {
        SOCKET acceptSocket = accept(serverSocket, nullptr, nullptr);
        if (acceptSocket == INVALID_SOCKET) {
            std::cout << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        int choice;
        int byteCount = recv(acceptSocket, reinterpret_cast<char*>(&choice), sizeof(choice), 0);

        if (byteCount <= 0) {
            if (byteCount == 0) {
                std::cout << "Client disconnected." << std::endl;
            }
            else {
                std::cout << "recv failed with error: " << WSAGetLastError() << std::endl;
            }
            closesocket(acceptSocket);
            continue;
        }

        if (choice == 1) {
            char username[100];
            char password[100];

            byteCount = recv(acceptSocket, username, sizeof(username) - 1, 0);
            if (byteCount <= 0) {
                std::cout << "Client disconnected during registration." << std::endl;
                closesocket(acceptSocket);
                continue;
            }
            username[byteCount] = '\0';

            byteCount = recv(acceptSocket, password, sizeof(password) - 1, 0);
            if (byteCount <= 0) {
                std::cout << "Client disconnected during registration." << std::endl;
                closesocket(acceptSocket);
                continue;
            }
            password[byteCount] = '\0';

            if (users.find(username) != users.end()) {
                const char* response = "Username already taken";
                send(acceptSocket, response, strlen(response), 0);
            }
            else {
                users[username] = password;
                const char* response = "Registration successful";
                send(acceptSocket, response, strlen(response), 0);
            }
        }
        if (choice == 2) {
            char username[100];
            char password[100];

            byteCount = recv(acceptSocket, username, sizeof(username) - 1, 0);
            if (byteCount <= 0) {
                std::cout << "Client disconnected during login." << std::endl;
                closesocket(acceptSocket);
                continue;
            }
            username[byteCount] = '\0';

            byteCount = recv(acceptSocket, password, sizeof(password) - 1, 0);
            if (byteCount <= 0) {
                std::cout << "Client disconnected during login." << std::endl;
                closesocket(acceptSocket);
                continue;
            }
            password[byteCount] = '\0';

            if (users.find(username) != users.end() && users[username] == std::string(password)) {
                const char* response = "Authentication successful";
                send(acceptSocket, response, strlen(response), 0);
            }
            else {
                const char* response = "Authentication failed";
                send(acceptSocket, response, strlen(response), 0);
            }
        }

        closesocket(acceptSocket);
        saveMutex.lock();
        SaveUserData(users, saveMutex);
        saveMutex.unlock();
    }

    saveThread.join();
    SaveUserData(users, saveMutex);

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}