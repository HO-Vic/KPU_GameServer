#include<unordered_map>
#include<random>
#include<chrono>
#include<mutex>
#include<array>
#include"SocketSection.h"

using namespace std;

void CALLBACK recv_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK send_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void disconnect(int id);

extern array<SocketSection, MAX_USER> ClientSockets;
extern mutex mapMutex;
SocketSection::SocketSection(int id, SOCKET& clientSocket) :clientSocket(clientSocket) {
	clientInfo.id = id;
	ZeroMemory(&recvWSABuf, sizeof(WSABUF));
	recvWSABuf.buf = recvBuf;		recvWSABuf.len = BUF_SIZE;
}

void doRecv(int id)
{
	//cout << "doRecv() " << clientInfo.id << endl;	
	if (ClientSockets[id].clientSocket == SOCKET_ERROR)
		return;
	if (!ClientSockets[id].isUse)
		return;
	DWORD flag = 0x0;
	ClientSockets[id].overlapped.hEvent = reinterpret_cast<HANDLE>(id);
	if (WSARecv(ClientSockets[id].clientSocket, &ClientSockets[id].recvWSABuf, 1, NULL, &flag, &ClientSockets[id].overlapped, recv_Callback) != 0) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			disconnect(id);
			return;
		}
	}
	SleepEx(100, true);
}

void doSend(int id, void* packet)
{
	//cout << "doSend() " << clientInfo.id << endl;
	if (ClientSockets[id].clientSocket == SOCKET_ERROR)	
		return;
	if (!ClientSockets[id].isUse)
		return;
	OverlappedEx* sendOver = new OverlappedEx;
	sendOver->over.hEvent = reinterpret_cast<HANDLE>(sendOver);
	memcpy(sendOver->IOCP_buf, packet, reinterpret_cast<char*>(packet)[0]);
	sendOver->wsabuf.buf = reinterpret_cast<char*>(sendOver->IOCP_buf);
	sendOver->wsabuf.len = reinterpret_cast<char*>(packet)[0];
	if (WSASend(ClientSockets[id].clientSocket, &sendOver->wsabuf, 1, nullptr, 0, &sendOver->over, send_Callback) != 0) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			disconnect(id);
			return;
		}
	}
	SleepEx(100, true);
	//cout << "SendByte: " << sendByte << endl;
}

void SocketSection::firstLocal()
{
	std::random_device rd;
	std::default_random_engine dre(rd());
	std::uniform_int_distribution<int> uid(0, 400);

	int x = uid(dre);
	int y = uid(dre);

	SC_LOGIN_INFO_PACKET sendPosPacket;
	sendPosPacket.id = clientInfo.id;
	clientInfo.pos = glm::vec3(x, 0, y);
	sendPosPacket.x = clientInfo.pos.x;
	sendPosPacket.y = clientInfo.pos.z;
	sendPosPacket.type = SC_LOGIN_INFO;
	sendPosPacket.size = sizeof(sendPosPacket);
	doSend(clientInfo.id, &sendPosPacket);
}

void SocketSection::processPacket(char* completePacket)
{
	//cout << "processData Type" << (int)ClientSockets[id].recvWSABuf.buf[1] << endl;
	switch (completePacket[1])
	{
	case CS_LOGIN:
	{
		//cout << "type: SC_LOGIN_INFO, Recv Login Info Packet" << endl;
		CS_LOGIN_PACKET* loginPacket = reinterpret_cast<CS_LOGIN_PACKET*>(completePacket);
		memcpy(clientInfo.name, loginPacket + 2, loginPacket->size - 2);
		firstLocal();
		spreadMyChessPeice();
		prsentDiffChessPeice();
	}
	break;
	case CS_MOVE:
	{
		//cout << "type: SC_MOVE_PLAYER, Recv Direction Packet" << endl;
		CS_MOVE_PACKET* directionPacket = reinterpret_cast<CS_MOVE_PACKET*>(completePacket);
		moveChessPiece(directionPacket->direction);
	}
	break;
	default:
		cout << "unknown Packet Recv" << endl;
		break;
	}
	delete[] completePacket;
}

void SocketSection::moveChessPiece(char& direction)
{
	SC_MOVE_PLAYER_PACKET sendPacket;
	sendPacket.type = SC_MOVE_PLAYER;
	sendPacket.id = clientInfo.id;
	sendPacket.size = sizeof(SC_MOVE_PLAYER_PACKET);

	switch (direction)
	{
	case DIRECTION_FRONT:
		if (clientInfo.pos.z > 0) {
			/*board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5) + 1] = 1;*/
			clientInfo.pos += glm::vec3(0, 0, -1);
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.move_time = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
			for (auto& client : ClientSockets) {
				if (client.recvWSABuf.buf == nullptr) continue;
				doSend(client.clientInfo.id, &sendPacket);
			}
		}
		break;
	case DIRECTION_BACK:
		if (clientInfo.pos.z < 400) {
			/*board[(int)(clientInfo.pos.x + 3.5f)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5) - 1] = 1;*/
			clientInfo.pos += glm::vec3(0, 0, 1);
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.move_time = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
			for (auto& client : ClientSockets) {
				if (client.recvWSABuf.buf == nullptr) continue;
				doSend(client.clientInfo.id, &sendPacket);
			}
		}
		break;
	case DIRECTION_LEFT:
		if (clientInfo.pos.x > 0) {
			/*board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5) - 1][-(int)(clientInfo.pos.z - 3.5)] = 1;*/
			clientInfo.pos += glm::vec3(-1, 0, 0);
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.move_time = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
			for (auto& client : ClientSockets) {
				if (client.recvWSABuf.buf == nullptr) continue;
				doSend(client.clientInfo.id, &sendPacket);
			}
		}
		break;
	case DIRECTION_RIGHT:
		if (clientInfo.pos.x < 400) {
			/*board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5) + 1][-(int)(clientInfo.pos.z - 3.5)] = 1;*/
			clientInfo.pos += glm::vec3(1, 0, 0);
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.move_time = chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now().time_since_epoch()).count();
			for (auto& client : ClientSockets) {
				if (client.recvWSABuf.buf == nullptr) continue;
				doSend(client.clientInfo.id, &sendPacket);
			}
		}
		break;
	default:
		break;
	}
}

void SocketSection::spreadMyChessPeice()
{
	SC_ADD_PLAYER_PACKET sendPakcet;
	sendPakcet.id = clientInfo.id;
	sendPakcet.x = clientInfo.pos.x;
	sendPakcet.y = clientInfo.pos.z;
	memcpy(sendPakcet.name, clientInfo.name, NAME_SIZE);
	sendPakcet.type = SC_ADD_PLAYER;
	sendPakcet.size = sizeof(SC_ADD_PLAYER_PACKET);
	for (auto& client : ClientSockets) {
		if (client.clientInfo.id == clientInfo.id) continue;
		doSend(client.clientInfo.id, &sendPakcet);
	}
}

void SocketSection::prsentDiffChessPeice()
{
	for (auto& client : ClientSockets) {
		if (client.clientInfo.id == clientInfo.id || client.recvWSABuf.buf == nullptr) continue;
		//	cout << "presendDiffChessPeice[" << client.first << "]" << endl;
		SC_ADD_PLAYER_PACKET sendPakcet;
		sendPakcet.id = client.clientInfo.id;
		sendPakcet.x = clientInfo.pos.x;
		sendPakcet.y = clientInfo.pos.z;
		memcpy(sendPakcet.name, clientInfo.name, NAME_SIZE);
		sendPakcet.type = SC_ADD_PLAYER;
		sendPakcet.size = sizeof(SC_ADD_PLAYER_PACKET);
		doSend(clientInfo.id, &sendPakcet);
	}
}

void SocketSection::LoginClient()
{
}

void disconnect(int id)
{
	ClientSockets[id].isUse = false;
	closesocket(ClientSockets[id].clientSocket);
	SC_REMOVE_PLAYER_PACKET sendPakcet;
	sendPakcet.id = id;
	sendPakcet.type = SC_REMOVE_PLAYER;
	sendPakcet.size = sizeof(SC_REMOVE_PLAYER_PACKET);
	for (auto& client : ClientSockets) {
		if (ClientSockets[id].isUse)
			doSend(client.clientInfo.id, &sendPakcet);
	}

	cout << "disconect id: " << id << endl;
}

/* CALLBACK Params
	IN DWORD dwError, // 작업의 성공 유무
	IN DWORD cbTransferred, // 바이트
	IN LPWSAOVERLAPPED lpOverlapped, // 오버랩 구조체
	IN DWORD dwFlags // 플래그
*/


void CALLBACK recv_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	int id = reinterpret_cast<int>(lpOverlapped->hEvent);
	if (dwError == WSAECONNRESET) {
		disconnect(id);
		return;
	}
	ClientSockets[id].constructPacket(ClientSockets[id].recvBuf, (unsigned char)cbTransferred);
	ZeroMemory(ClientSockets[id].recvBuf, BUF_SIZE);
	doRecv(id);

}

void CALLBACK send_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	//cout << "send_Callback() - SendByte" << (int)cbTransferred << endl;
	//ClientSockets[reinterpret_cast<int>(lpOverlapped->hEvent)].doRecv();
	delete lpOverlapped->hEvent;

}

void display_Err(int Errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, Errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);
	wcout << "ErrorCode: " << Errcode << " - " << (WCHAR*)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}

void SocketSection::constructPacket(char* inputPacket, unsigned char inputSize)
{
	char* packetStart = inputPacket;
	unsigned char restSize = inputSize + prevPacketLastLocal;
	unsigned char currentPacketLocal = 0;
	unsigned char makePacketSize = 0;



	while (restSize > 0) {
		if (prevPacketLastLocal != 0)
			makePacketSize = prevPacket[0];
		else makePacketSize = inputPacket[currentPacketLocal];
		if (makePacketSize == 0) break;
		if (restSize < makePacketSize) {
			memcpy(prevPacket + prevPacketLastLocal, inputPacket + currentPacketLocal, restSize);
			prevPacketLastLocal += restSize;
			break;
		}
		else {
			memcpy(prevPacket + prevPacketLastLocal, inputPacket + currentPacketLocal, (int)(makePacketSize - prevPacketLastLocal));

			restSize -= (makePacketSize - prevPacketLastLocal);
			currentPacketLocal += (makePacketSize - prevPacketLastLocal);
			prevPacketLastLocal += (makePacketSize - prevPacketLastLocal);

			char* procPacket = new char[makePacketSize];
			ZeroMemory(procPacket, makePacketSize);
			memcpy(procPacket, prevPacket, makePacketSize);
			processPacket(procPacket);
			ZeroMemory(prevPacket, BUF_SIZE);
			prevPacketLastLocal = 0;
			if (currentPacketLocal == inputSize)
				break;
		}
	}
}
