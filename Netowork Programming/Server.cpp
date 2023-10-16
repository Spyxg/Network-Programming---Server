#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

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

    // Create a socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }
    else {
        cout << "socket() is good" << endl;
    }

    // Bind the socket
    sockaddr_in service;
    service.sin_family = AF_INET;
    InetPton(AF_INET, L"127.0.0.1", &service.sin_addr.s_addr);
    service.sin_port = htons(8080);
    if (bind(serverSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR) {
        cout << "bind() failed: " << WSAGetLastError() << endl;
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 1) == SOCKET_ERROR)
        cout << "listen(): Error listening on socket " << WSAGetLastError() << endl;
    else
        cout << "listen() is good, waiting for connections..." << endl;

    // Accept a client connection
    SOCKET acceptSocket = accept(serverSocket, NULL, NULL);
    if (acceptSocket == INVALID_SOCKET) {
        cout << "Accept failed: " << WSAGetLastError() << endl;
    }

    // Receiving
    char receiveBuffer[200] = "";
    int byteCount = recv(acceptSocket, receiveBuffer, 200, 0);
    if (byteCount < 0) {
        printf("Receive error %ld.\n", WSAGetLastError());
    }
    else {
        printf("Received data: %s \n", receiveBuffer);
    }

    char buffer[200];
    printf("Enter your Message: ");
    cin.getline(buffer, 200);
    int sendByteCount = send(acceptSocket, buffer, 200, 0);
    if (sendByteCount == SOCKET_ERROR) {
        printf("Send error %ld.\n", WSAGetLastError());
    }
    else {
        printf("Sent %d bytes \n", sendByteCount);
    }

    closesocket(acceptSocket); // Close the accepted socket
    closesocket(serverSocket);  // Close the server socket
    WSACleanup(); // Cleanup Winsock when done
    return 0;
}
