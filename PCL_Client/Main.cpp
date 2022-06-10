#include <WS2tcpip.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <thread>
#pragma comment(lib, "ws2_32.lib")

// DEF UNICODE
#ifdef _UNICODE
#define tstring std::wstring
#define tcout std::wcout
#define tcin std::wcin
#else
#define tstring std::string
#define tcout std::cout
#define tcin std::cin
#endif

#define BUFFSIZE 1024

void main() {
  tstring ipAddress;
  tstring port;
  tcout << _T("Adress: ");
  getline(tcin, ipAddress);
  tcout << _T("Port: ");
  getline(tcin, port);
  int portInt = stoi(port);

  tcout << _T("Adress: ") << ipAddress << _T(" Port: ") << port << _T("\n");

  // Initialize WinSock
  WSAData data;
  WORD ver = MAKEWORD(2, 2);
  int wsResult = WSAStartup(ver, &data);
  if (wsResult != 0) {
    std::cerr << "Can't start Winsock, Err #" << wsResult << '\n';
    return;
  }

  // Create socket
  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == INVALID_SOCKET) {
    std::cerr << "Can't create socket, Err #" << WSAGetLastError() << '\n';
    WSACleanup();
    return;
  }

  // Fill in a hint structure
  sockaddr_in hint;
  hint.sin_family = AF_INET;
  hint.sin_port = htons((u_short)portInt);

  InetPton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

  // Connect to server
  int connResult = connect(sock, (sockaddr *)&hint, sizeof(hint));
  if (connResult == SOCKET_ERROR) {
    std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << '\n';
    closesocket(sock);
    WSACleanup();
    return;
  }
  tcout << "Connected!\n";
  tstring userInput;
  bool canQuit = false;

  while (!canQuit) {
    getline(tcin, userInput);

    if (userInput.size() <= 0)
      continue;
    int sendResult = send(sock, (char *)userInput.c_str(),
                          sizeof(TCHAR) * userInput.size() + 1, 0);

    if (sendResult == SOCKET_ERROR) {
      break;
    }

    if (userInput == _T("quit")) {
      tcout << "Quitting...!\n";
      break;
    }

    int nrOfBytes, maxBytes;
    int bytesReceived = recv(sock, (char *)&nrOfBytes, sizeof(int), 0);

    if (bytesReceived <= 0)
      break;

    maxBytes = nrOfBytes;

    char *message = new char[nrOfBytes];
    char *currentPosition = message;

    while (nrOfBytes > 0) {
      bytesReceived = recv(sock, currentPosition, min(BUFFSIZE, nrOfBytes), 0);
      nrOfBytes -= bytesReceived;
      currentPosition = currentPosition + bytesReceived;
    }
    tstring tmessage = (LPTSTR)message;

    tmessage.at(maxBytes / sizeof(TCHAR)) = _T('\0');

    for (size_t index = 0; index < maxBytes / sizeof(TCHAR); index++) {
      tcout << tmessage[index];
    }

    delete[] message;
  }

  closesocket(sock);
  WSACleanup();
}
