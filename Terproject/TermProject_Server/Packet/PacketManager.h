#pragma once
#include "../PCH/stdafx.h"

using namespace std;
using namespace chrono;

class PacketManager{
public:
	static void SendStatPacketSelf(int playerId);

	static void SendLoginPacket(SOCKET & socket, wstring & ingameName, int id, short x, short y, short hp, short mHp, short level, short exp);
	static void SendLoginPacket(SOCKET & socket, wstring & ingameName, int id, pair<short, short> & pos, short hp, short mHp, short level, short exp);
	static void SendLoginFailPacket(SOCKET & socket);
	static void SendRemoveObjectPacket(SOCKET & socket, int removePlayerId);
	static void SendMoveObjectPacket(SOCKET & socket, int movePlayerId);
	static void SendAddObjectPacket(SOCKET & socket, int addPlayerId);
	static void SendStatPacketInViewList(std::unordered_set<int> & viewList, int objId);
	static void SendSkillExecuteTImePacket(SOCKET & socket, int playerId, unordered_set<int> & viewList, system_clock::time_point & t);
	static void SendMessPacket(SOCKET & socket, int sendId, wchar_t * mess);
	static void SendDelayPacket(SOCKET socket);
public:
	static void SendPacket(SOCKET & socket, char * data);
public:
	static int ProccessPacket(int playerId, int ioByte, int currentRemainData, char * buf);
	static void ExecutePacket(int playerId, char * packet);
public:
	static void RemoveDisconnectClient(int disconnectedId, unordered_set<int> & disconnectPlayerViewList);
};

