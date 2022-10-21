#pragma once
#include<winsock2.h>
#include"include/glm/glm.hpp"
#include<iostream>
#include "protocol.h"

#define DIRECTION_FRONT 1
#define DIRECTION_BACK	2
#define DIRECTION_LEFT	3
#define DIRECTION_RIGHT 4

using namespace std;

struct ClientInfo
{
	int id;
	glm::vec3 pos;
	glm::vec3 color;
	char name[NAME_SIZE];
};

struct OverlappedEx {
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	unsigned char IOCP_buf[BUF_SIZE];
};

class SocketSection
{
public:
	WSAOVERLAPPED overlapped;
	SOCKET clientSocket;
	WSABUF recvWSABuf;
	char recvBuf[BUF_SIZE] = { 0 };
	DWORD recvByte = 0;
	ClientInfo clientInfo;

	char prevPacket[BUF_SIZE] = { 0 };
	
	unsigned char prevPacketLastLocal = 0;

public:
	SocketSection() {}
	~SocketSection() {
		recvWSABuf.buf = nullptr;
		closesocket(clientSocket);		
	}
	

	SocketSection(int id, SOCKET& clientSocket);
	void doRecv();
	void doSend(void* packet);

	void firstLocal();
	void moveChessPiece(char& direction);
	void spreadMyChessPeice();
	void prsentDiffChessPeice();
	void LoginClient();
	void processPacket(char* completePacket);
	void constructPacket(char* inputPacket, unsigned char inputSize);

};

void display_Err(int Errcode);
