#include<unordered_map>
#include<random>
#include"SocketSection.h"

using namespace std;

void CALLBACK recv_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK send_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void disconnect(int id);
void processPacket(int id);

extern unordered_map<int, SocketSection> ClientSockets;
extern int board[400][400];

SocketSection::SocketSection(int id, SOCKET& clientSocket) :clientSocket(clientSocket) {
	clientInfo.id = id;
	ZeroMemory(&recvWSABuf, sizeof(WSABUF));
	ZeroMemory(&sendWSABuf, sizeof(WSABUF));
	sendWSABuf.buf = sendBuf;		sendWSABuf.len = BUF_SIZE;
	recvWSABuf.buf = recvBuf;		recvWSABuf.len = BUF_SIZE;
}

void SocketSection::doRecv()
{
	DWORD flag = 0x0;
	overlapped.hEvent = reinterpret_cast<HANDLE>(clientInfo.id);
	if (WSARecv(clientSocket, &recvWSABuf, 1, NULL, &flag, &overlapped, recv_Callback) != 0) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			//cout << "ID: " << clientInfo.id << "	" << "doRecv() - Error" << endl;
			display_Err(WSAGetLastError());
		}
	}
}

void SocketSection::doSend()
{
	overlapped.hEvent = reinterpret_cast<HANDLE>(clientInfo.id);
	if (WSASend(clientSocket, &sendWSABuf, 1, &sendByte, 0, &overlapped, send_Callback) != 0) {
		if (WSAGetLastError() != WSA_IO_PENDING) {
			cout << "doSend() - send fail" << endl;
			display_Err(WSAGetLastError());
		}
	}
	//cout << "SendByte: " << sendByte << endl;
}

void SocketSection::firstLocal()
{
		std::random_device rd;
		std::default_random_engine dre(rd());
		std::uniform_int_distribution<int> uid(0, 399);

		int x = uid(dre);
		int y = uid(dre);

		SC_LOGIN_INFO_PACKET sendPosPacket;
		board[x][y] = 1;
		sendPosPacket.id = clientInfo.id;
		clientInfo.pos = glm::vec3(x - 3.5, 0, 3.5 - y);
		sendPosPacket.x = clientInfo.pos.x;
		sendPosPacket.y = clientInfo.pos.z;
		sendPosPacket.type = SC_LOGIN_INFO;
		sendPosPacket.size = sizeof(sendPosPacket);
		memcpy(this->sendWSABuf.buf, &sendPosPacket, sendPosPacket.size);
		this->sendWSABuf.len = sendPosPacket.size;
		doSend();
}

void processPacket(int id)
{
	switch (ClientSockets[id].recvWSABuf.buf[2])
	{
	case CS_LOGIN:
	{
		cout << "type: SC_LOGIN_INFO, Recv Login Info Packet" << endl;
		CS_LOGIN_PACKET* loginPacket = reinterpret_cast<CS_LOGIN_PACKET*>(ClientSockets[id].recvWSABuf.buf);
		memcpy(ClientSockets[id].clientInfo.name, loginPacket + 2, loginPacket->size - 2);
		ClientSockets[id].firstLocal();
		ClientSockets[id].spreadMyChessPeice();
		ClientSockets[id].prsentDiffChessPeice();
	}
	break;
	case CS_MOVE:
	{
		cout << "type: SC_MOVE_PLAYER, Recv Direction Packet" << endl;
		CS_MOVE_PACKET* directionPacket = reinterpret_cast<CS_MOVE_PACKET*>(ClientSockets[id].recvWSABuf.buf);
		ClientSockets[id].moveChessPiece(directionPacket->direction);
	}
	break;
	default:
		break;
	}
}

void SocketSection::moveChessPiece(char& direction)
{
	switch (direction)
	{
	case DIRECTION_FRONT:
		if (clientInfo.pos.z > -3.5f) {
			board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5) + 1] = 1;
			clientInfo.pos += glm::vec3(0, 0, -1);
			SC_MOVE_PLAYER_PACKET sendPacket;
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.type = SC_MOVE_PLAYER;
			sendPacket.size = sizeof(SC_MOVE_PLAYER_PACKET);
			ZeroMemory(sendWSABuf.buf, BUF_SIZE);
			memcpy(sendBuf, &sendPacket, sendPacket.size);
			sendWSABuf.len = sendPacket.size;
			doSend();
			spreadMyChessPeice();
		}
		else doRecv();
		break;
	case DIRECTION_BACK:
		if (clientInfo.pos.z < 3.5f) {
			board[(int)(clientInfo.pos.x + 3.5f)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5) - 1] = 1;
			clientInfo.pos += glm::vec3(0, 0, 1);
			SC_MOVE_PLAYER_PACKET sendPacket;
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.type = SC_MOVE_PLAYER;
			sendPacket.size = sizeof(SC_MOVE_PLAYER_PACKET);
			ZeroMemory(sendWSABuf.buf, BUF_SIZE);
			memcpy(sendBuf, &sendPacket, sendPacket.size);
			sendWSABuf.len = sendPacket.size;
			doSend();
			spreadMyChessPeice();
		}
		else doRecv();
		break;
	case DIRECTION_LEFT:
		if (clientInfo.pos.x > -3.5f) {
			board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5) - 1][-(int)(clientInfo.pos.z - 3.5)] = 1;
			clientInfo.pos += glm::vec3(-1, 0, 0);
			SC_MOVE_PLAYER_PACKET sendPacket;
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.type = SC_MOVE_PLAYER;
			sendPacket.size = sizeof(SC_MOVE_PLAYER_PACKET);
			ZeroMemory(sendWSABuf.buf, BUF_SIZE);
			memcpy(sendBuf, &sendPacket, sendPacket.size);
			sendWSABuf.len = sendPacket.size;
			doSend();
			spreadMyChessPeice();
		}
		else doRecv();
		break;
	case DIRECTION_RIGHT:
		if (clientInfo.pos.x < 3.5f) {
			board[(int)(clientInfo.pos.x + 3.5)][-(int)(clientInfo.pos.z - 3.5)] = 0;
			board[(int)(clientInfo.pos.x + 3.5) + 1][-(int)(clientInfo.pos.z - 3.5)] = 1;
			clientInfo.pos += glm::vec3(1, 0, 0);
			SC_MOVE_PLAYER_PACKET sendPacket;
			sendPacket.x = clientInfo.pos.x;
			sendPacket.y = clientInfo.pos.z;
			sendPacket.type = SC_MOVE_PLAYER;
			sendPacket.size = sizeof(SC_MOVE_PLAYER_PACKET);
			ZeroMemory(sendWSABuf.buf, BUF_SIZE);
			memcpy(sendBuf, &sendPacket, sendPacket.size);
			sendWSABuf.len = sendPacket.size;
			doSend();
			spreadMyChessPeice();
		}
		else doRecv();
		break;
	default:
		break;
	}
}

void SocketSection::spreadMyChessPeice()
{
	for (auto& client : ClientSockets) {
		if (client.first != clientInfo.id) {
			//cout << "multiCast Client Info Id:" << client.first << endl;
			SC_ADD_PLAYER_PACKET sendPakcet;
			sendPakcet.id = clientInfo.id;
			sendPakcet.x = clientInfo.pos.x;
			sendPakcet.y = clientInfo.pos.z;
			memcpy(sendPakcet.name, clientInfo.name, NAME_SIZE);
			sendPakcet.type = SC_ADD_PLAYER;
			sendPakcet.size = sizeof(SC_ADD_PLAYER_PACKET);

			ZeroMemory(client.second.sendBuf, BUF_SIZE);
			memcpy(client.second.sendBuf, &sendPakcet, sendPakcet.size);

			client.second.sendWSABuf.len = sendPakcet.size;
			client.second.doSend();
		}
	}
}

void SocketSection::prsentDiffChessPeice()
{
	for (auto& client : ClientSockets) {
		if (client.first != clientInfo.id) {
			//cout << "presendDiffChessPeice[" << client.first << "]" << endl;
			SC_ADD_PLAYER_PACKET sendPakcet;
			sendPakcet.id = client.first;
			sendPakcet.x = clientInfo.pos.x;
			sendPakcet.y = clientInfo.pos.z;
			memcpy(sendPakcet.name, clientInfo.name, NAME_SIZE);
			sendPakcet.type = SC_ADD_PLAYER;
			sendPakcet.size = sizeof(SC_ADD_PLAYER_PACKET);

			ZeroMemory(sendBuf, BUF_SIZE);
			memcpy(sendBuf, &sendPakcet, sendPakcet.size);

			sendWSABuf.len = sendPakcet.size;
			doSend();
		}
	}
}

void SocketSection::LoginClient()
{
}

void disconnect(int id)
{
	for (auto& client : ClientSockets) {
		if (client.first != id) {
			SC_REMOVE_PLAYER_PACKET sendPakcet;
			sendPakcet.id = id;
			sendPakcet.type = SC_REMOVE_PLAYER;
			sendPakcet.size = sizeof(SC_REMOVE_PLAYER_PACKET);

			ZeroMemory(client.second.sendBuf, BUF_SIZE);
			memcpy(client.second.sendBuf, &sendPakcet, sendPakcet.size);
			client.second.sendWSABuf.len = sendPakcet.size;
			client.second.doSend();
		}
	}
	ClientSockets.erase(id);
}

/* CALLBACK Params
	IN DWORD dwError, // 작업의 성공 유무
	IN DWORD cbTransferred, // 바이트
	IN LPWSAOVERLAPPED lpOverlapped, // 오버랩 구조체
	IN DWORD dwFlags // 플래그
*/


void CALLBACK recv_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{

	//cout << "recv_Callback() - RecvByte" << (int)cbTransferred << endl;
	int id = reinterpret_cast<int>(lpOverlapped->hEvent);
	if (ClientSockets.find(id) != ClientSockets.end()) {
		if (dwError == 0) {
			processPacket(id);
		}
		else if (dwError == WSAECONNRESET) {
			cout << "removePlayer ID: " << id << endl;
			//ClientSockets[recvOverlapped->clientInfo.id].disconnect();
			board[(int)(ClientSockets[id].clientInfo.pos.x + 3.5)][-(int)(ClientSockets[id].clientInfo.pos.z - 3.5)] = 0;
			disconnect(id);
		}
	}

}

void CALLBACK send_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	//cout << "send_Callback() - SendByte" << (int)cbTransferred << endl;
	//ClientSockets[reinterpret_cast<int>(lpOverlapped->hEvent)].doRecv();

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
