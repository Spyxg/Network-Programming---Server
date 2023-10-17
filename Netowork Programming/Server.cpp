#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <map>
#include <fstream>
#include <thread>
#include "json.hpp"


using namespace std;


void SaveUserData(const map<string, string>& users) {
    ofstream userFile("users.txt");
    if (userFile.is_open()) {
        for (const auto& entry : users) {
            userFile << entry.first << " " << entry.second << endl;
        }
        userFile.close();
    }
}



int main() {
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);


    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        cout << "WSAStartup failed with error: " << wsaerr << endl;
        return 1;
    }

    cout << "The Winsock DLL found!" << endl;
    cout << "The Status: " << wsaData.szSystemStatus << endl;

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in service;
    service.sin_family = AF_INET;
    service.sin_addr.s_addr = INADDR_ANY;
    service.sin_port = htons(8080);

    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        cout << "bind() failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        cout << "listen() failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }
    else {
        cout << "listen() is good, waiting for connections..." << endl;
    }

    map<string, string> users;
    thread saveThread([&users] { SaveUserData(users); });
    while (true) {
        SOCKET acceptSocket = accept(serverSocket, NULL, NULL);
        if (acceptSocket == INVALID_SOCKET) {
            cout << "Accept failed: " << WSAGetLastError() << endl;
            continue;
        }

        int choice;
        int byteCount = recv(acceptSocket, (char*)&choice, sizeof(choice), 0);

        if (byteCount <= 0) {
            if (byteCount == 0) {
            cout << "Client disconnected." << endl;
            }
            else {
                cout << "recv failed with error: " << WSAGetLastError() << endl;
            }
            closesocket(acceptSocket);
            continue;
        }

        if (choice == 1) {
            char username[100];
            char password[100];

            byteCount = recv(acceptSocket, username, sizeof(username) - 1, 0);
            if (byteCount <= 0) {
                cout << "Client disconnected during registration." << endl;
                closesocket(acceptSocket);
                continue;
            }
            username[byteCount] = '\0';

            byteCount = recv(acceptSocket, password, sizeof(password) - 1, 0);
            if (byteCount <= 0) {
                cout << "Client disconnected during registration." << endl;
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
                cout << "Client disconnected during login." << endl;
                closesocket(acceptSocket);
                continue;
            }
            username[byteCount] = '\0';

            byteCount = recv(acceptSocket, password, sizeof(password) - 1, 0);
            if (byteCount <= 0) {
                cout << "Client disconnected during login." << endl;
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
    }

    saveThread.join(); // Join the saveThread before exiting
    SaveUserData(users);

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}