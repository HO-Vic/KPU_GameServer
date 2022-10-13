#include"include/glm/glm.hpp"
#include"include/glm/ext.hpp"
#include"include/glm/gtc/matrix_transform.hpp"
#include<iostream>
#include<unordered_map>
#include<WS2tcpip.h>
#include"SocketSection.h"

#pragma comment(lib,"ws2_32")


#define SERVER_PORT 9000

using namespace std;

unordered_map<int, SocketSection> ClientSockets;
int board[400][400] = { 0 };

int main(int argc, char** argv)
{	
	wcout.imbue(std::locale("korean"));
	WSADATA WSAData;

	if (WSAStartup(MAKEWORD(2, 2), &WSAData)) {
		cout << "Error: WSAStartUp fail" << endl;
		display_Err(WSAGetLastError());
	}

	SOCKET mainSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (mainSocket == INVALID_SOCKET) {
		cout << "Error: socket Err" << endl;
		display_Err(WSAGetLastError());
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(mainSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(SOCKADDR_IN)) != 0) {
		cout << "Error: bind() Err" << endl;
		display_Err(WSAGetLastError());
	}

	if (listen(mainSocket, SOMAXCONN) != 0) {
		cout << "Error: listen() Err" << endl;
		display_Err(WSAGetLastError());
	}

	for(int i=0; ; ++i) {
		int addrLen = sizeof(SOCKADDR_IN);
		SOCKET clientSocket = WSAAccept(mainSocket, reinterpret_cast<sockaddr*>(&serverAddr), &addrLen, NULL, NULL);		
		
		if (clientSocket == INVALID_SOCKET) {
			cout << "Error: InvalidSocket" << endl;
			display_Err(WSAGetLastError());
			continue;
		}
		ClientSockets.try_emplace(i, i, clientSocket);
		ClientSockets[i].firstLocal();
		ClientSockets[i].spreadMyChessPeice();
		ClientSockets[i].prsentDiffChessPeice();
		
	}
	closesocket(mainSocket);
	WSACleanup();
}
