#include <iostream>
#include <winsock2.h>

using std::cout;
using std::endl;

// Initialize WSA - WSAStartup().

int main() {
    WSADATA wsaData;
    int wsaerr;
    WORD wVersionRequested = MAKEWORD(2, 2);
    wsaerr = WSAStartup(wVersionRequested, &wsaData);
    if (wsaerr != 0) {
        cout << "The Winsock DLL not found!" << endl;
        return 1;
    }
    else {
        cout << "The Winsock DLL found!" << endl;
        cout << "The Status: " << wsaData.szSystemStatus << endl;
        WSACleanup();
    }
    return 0;
}
