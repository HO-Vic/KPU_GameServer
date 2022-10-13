#pragma once
#include<winsock2.h>
#include"include/glm/glm.hpp"
#include<iostream>
#define BUF_SIZE 32

#define DIRECTION_FRONT 0x10
#define DIRECTION_BACK	0x11
#define DIRECTION_LEFT	0x12
#define DIRECTION_RIGHT 0x13

using namespace std;

struct ClientInfo
{
	int id;
	glm::vec3 pos;
	glm::vec3 color;
};

class SocketSection
{
public:
	WSAOVERLAPPED overlapped;
	SOCKET clientSocket;
	WSABUF sendWSABuf;
	WSABUF recvWSABuf;
	char recvBuf[BUF_SIZE] = { 0 };
	char sendBuf[BUF_SIZE] = { 0 };
	DWORD recvByte = 0;
	DWORD sendByte = 0;
	ClientInfo clientInfo;

public:
	SocketSection() {}
	~SocketSection() {
		recvWSABuf.buf = nullptr;
		closesocket(clientSocket);		
	}
	

	SocketSection(int id, SOCKET& clientSocket);
	void doRecv();
	void doSend();

	void firstLocal();
	void moveChessPiece(WORD& direction);
	void spreadMyChessPeice();
	void prsentDiffChessPeice();
};

void display_Err(int Errcode);
