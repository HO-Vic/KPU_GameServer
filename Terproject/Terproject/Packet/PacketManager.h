#pragma once
#include "../PCH/stdafx.h"
#include <winsock.h>

class PacketManager
{
public:
	static void SendLoginPacket(SOCKET& socket,wstring& ingameName, int id, short x, short y, short hp, short mHp, short level, short exp);
	static void SendLoginPacket(SOCKET& socket,wstring& ingameName, int id, pair<short, short>& pos, short hp, short mHp, short level, short exp);
	static void SendRemoveObjectPacket(SOCKET& socket, int removePlayerId);
	static void SendMoveObjectPacket(SOCKET& socket, int movePlayerId);
	static void SendAddObjectPacket(SOCKET& socket, int addPlayerId);
	static void SendStatPacketInViewList(std::unordered_set<int>& viewList, int objId);
	static void SendStatPacketSelf(int playerId);
public:
	static void SendPacket(SOCKET& socket, char* data);
public:
	static int ProccessPacket(int playerId, int ioByte, int currentRemainData, char* buf);
	static void ExecutePacket(int playerId, char* packet);
};

