#include "stdafx.h"
#include "PacketManager.h"
#include "protocol_2022.h"
#include "../ExpOver/ExpOver.h"
#include "../GameObject/GameObject.h"
#include "../GameObject/PlayerObject/PlayerObject.h"
#include "../GameObject/NPC_Object/NPC_OBJECT.h"
#include "../Logic/Logic.h"
#include "../IocpNetwork/IocpNetwork.h"
#include "../DB/DB_OBJ.h"

using namespace std;
using namespace chrono;


extern random_device g_rd;
extern default_random_engine g_dre;
extern uniform_int_distribution<short> g_npcRandDir; // inclusive
extern uniform_int_distribution<short> g_npcRandPostion; // inclusive

extern array<GameObject*, MAX_USER + MAX_NPC> g_clients;
extern std::array<int, 11> g_levelExp;

extern IocpNetwork g_iocpNetwork;
extern DB_OBJ g_DB;

void PacketManager::SendLoginPacket(SOCKET& socket, wstring& ingameName, int id, short x, short y, short hp, short mHp, short level, short exp)
{
	SC_LOGIN_INFO_PACKET p;
	p.id = id;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_LOGIN_INFO;
	p.x = x;
	p.y = y;
	p.hp = hp;
	p.max_hp = hp;
	p.exp = exp;
	p.level = level;
	p.max_exp = g_levelExp[level];
	wcsncpy_s(p.name, ingameName.c_str(), ingameName.size() > 19 ? 19 : ingameName.size());
	SendPacket(socket, reinterpret_cast<char*>(&p));
}

void PacketManager::SendLoginPacket(SOCKET& socket, wstring& ingameName, int id, pair<short, short>& pos, short hp, short mHp, short level, short exp)
{
	SC_LOGIN_INFO_PACKET p;
	p.id = id;
	p.size = sizeof(SC_LOGIN_INFO_PACKET);
	p.type = SC_LOGIN_INFO;
	p.x = pos.first;
	p.y = pos.second;
	p.hp = hp;
	p.max_hp = hp;
	p.exp = exp;
	p.level = level;
	p.max_exp = g_levelExp[level];
	wcsncpy_s(p.name, ingameName.c_str(), ingameName.size() > 19 ? 19 : ingameName.size());
	SendPacket(socket, reinterpret_cast<char*>(&p));
}

void PacketManager::SendLoginFailPacket(SOCKET& socket)
{
	SC_LOGIN_FAIL_PACKET p;
	p.size = sizeof(SC_LOGIN_FAIL_PACKET);
	p.type = SC_LOGIN_FAIL;
	SendPacket(socket, reinterpret_cast<char*>(&p));
}

void PacketManager::SendRemoveObjectPacket(SOCKET& socket, int removePlayerId)
{
	SC_REMOVE_OBJECT_PACKET p;
	p.id = removePlayerId;
	p.size = sizeof(p);
	p.type = SC_REMOVE_OBJECT;
	SendPacket(socket, reinterpret_cast<char*>(&p));
}

void PacketManager::SendMoveObjectPacket(SOCKET& socket, int movePlayerId)
{
	SC_MOVE_OBJECT_PACKET p;
	p.id = movePlayerId;
	p.size = sizeof(SC_MOVE_OBJECT_PACKET);
	p.type = SC_MOVE_OBJECT;
	p.x = g_clients[movePlayerId]->GetPosition().first;
	p.y = g_clients[movePlayerId]->GetPosition().second;
	p.move_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	SendPacket(socket, reinterpret_cast<char*>(&p));
}

void PacketManager::SendAddObjectPacket(SOCKET& socket, int addPlayerId)
{
	SC_ADD_OBJECT_PACKET p;
	p.id = addPlayerId;
	wcscpy_s(p.name, g_clients[addPlayerId]->GetName().c_str());
	p.size = sizeof(p);
	p.type = SC_ADD_OBJECT;
	p.x = g_clients[addPlayerId]->GetPosition().first;
	p.y = g_clients[addPlayerId]->GetPosition().second;
	p.hp = g_clients[addPlayerId]->GetHp();
	p.max_hp = g_clients[addPlayerId]->GetMaxHp();
	SendPacket(socket, reinterpret_cast<char*>(&p));
}

void PacketManager::SendStatPacketInViewList(std::unordered_set<int>& viewList, int objId)
{
	SC_STAT_CHANGEL_PACKET p;
	p.type = SC_STAT_CHANGE;
	p.size = sizeof(SC_STAT_CHANGEL_PACKET);
	p.id = objId;
	p.hp = g_clients[objId]->GetHp();
	p.max_hp = g_clients[objId]->GetMaxHp();
	p.max_exp = g_clients[objId]->GetMaxExp();
	p.exp = g_clients[objId]->GetExp();
	p.level = g_clients[objId]->GetLevel();
	if (Logic::IsPlayer(objId))
		dynamic_cast<PlayerObject*>(g_clients[objId])->SendPacket(reinterpret_cast<char*>(&p));
	for (auto& id : viewList)
		if (Logic::IsPlayer(id))
			dynamic_cast<PlayerObject*>(g_clients[id])->SendPacket(reinterpret_cast<char*>(&p));
}

void PacketManager::SendSkillExecuteTImePacket(SOCKET& socket, int playerId, unordered_set<int>& viewList, system_clock::time_point& t)
{
	SC_ATTACK_PACKET p;
	p.size = sizeof(SC_ATTACK_PACKET);
	p.type = SC_ATTACK;
	p.skillExecuteTime = t;
	p.id = playerId;
	if (Logic::IsPlayer(playerId))
		SendPacket(socket, reinterpret_cast<char*>(&p));
	for (const auto& otherId : viewList)
		if (Logic::IsPlayer(otherId))
			dynamic_cast<PlayerObject*>(g_clients[otherId])->SendPacket(reinterpret_cast<char*>(&p));
}

void PacketManager::SendMessPacket(SOCKET& socket, int sendId, wchar_t* mess)
{
	SC_CHAT_PACKET p;
	p.type = SC_CHAT;
	p.size = sizeof(SC_CHAT_PACKET);
	p.id = sendId;
	wcscpy_s(p.mess, mess);
	SendPacket(socket, reinterpret_cast<char*>(&p));
}

void PacketManager::SendStatPacketSelf(int playerId)
{
	if (!Logic::IsPlayer(playerId))return;
	SC_STAT_CHANGEL_PACKET p;
	p.type = SC_STAT_CHANGE;
	p.size = sizeof(SC_STAT_CHANGEL_PACKET);
	p.id = playerId;
	p.hp = g_clients[playerId]->GetHp();
	p.max_hp = g_clients[playerId]->GetMaxHp();
	p.max_exp = g_clients[playerId]->GetMaxExp();
	p.exp = g_clients[playerId]->GetExp();
	p.level = g_clients[playerId]->GetLevel();
	dynamic_cast<PlayerObject*>(g_clients[playerId])->SendPacket(reinterpret_cast<char*>(&p));

}

void PacketManager::SendPacket(SOCKET& socket, char* data)
{
	ExpOverWsaBuffer* sendOver = ExpOverMgr::CreateExpOverWsaBuffer(OP_CODE::OP_SEND, data);
	sendOver->DoSend(socket);
}

int PacketManager::ProccessPacket(int playerId, int ioByte, int currentRemainData, char* buf)
{
	int remainSize = ioByte + currentRemainData;
	char* currentPacketPosition = buf;


	while (true) {
		unsigned char packetSize = currentPacketPosition[0];
		if (packetSize == 0)break;
		if (packetSize <= remainSize) {
			ExecutePacket(playerId, currentPacketPosition);

			memcpy(g_clients[playerId]->prevPacketData, currentPacketPosition, packetSize);
			g_clients[playerId]->prevPacketSize = packetSize;

			currentPacketPosition = currentPacketPosition + packetSize;
			remainSize = remainSize - packetSize;
		}
		else break;
	}
	if (remainSize > 0)
		std::memcpy(buf, currentPacketPosition, remainSize);
	ZeroMemory(buf + remainSize, BUF_SIZE - remainSize);
	return remainSize;
}

void PacketManager::ExecutePacket(int playerId, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		std::string nameString = p->loginId;
		if (std::string::npos != nameString.find("Test")) {//동접 테스트를 위한
			PlayerObject* currentPlayer = reinterpret_cast<PlayerObject*>(g_clients[playerId]);
			currentPlayer->InitSetting(playerId, 1000, 1000, 0, 0, g_npcRandPostion(g_dre), g_npcRandPostion(g_dre));
			currentPlayer->SetName(p->loginId);//name보다는 로그인 id임
			currentPlayer->SendLoginInfoPacket();
			auto viewList = Logic::UpdateNearList(playerId);
			for (auto& notifyPlayerId : viewList) {
				g_clients[notifyPlayerId]->AddViewListPlayer(playerId);
				currentPlayer->AddViewListPlayer(notifyPlayerId);
			}
		}
		else {
			DB::DB_PlayerId dbData = DB::DB_PlayerId(playerId, p->loginId);
			DB_Event ev = DB_Event(EV_GET_PLAYER_INFO, playerId, &dbData);
			g_DB.Insert_DBEvent(ev);
		}
	}
	break;
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		auto prevPosition = g_clients[playerId]->GetPosition();
		auto position = prevPosition;

		Logic::MoveDirection(p->direction, position);
		if (position == prevPosition) return;
		Logic::MoveGameObject(playerId, prevPosition, position);
	}
	break;
	case CS_ATTACK:
	{
		Logic::PlayerAttackExecute(playerId);
	}
	break;
	case CS_LOGOUT:
	{
		DB_Event dbData{ EV_SAVE_PLAYER_INFO, playerId };
		g_DB.Insert_DBEvent(dbData);
		//disconnect
		Logic::DisconnectClient(playerId);
	}
	case CS_CHAT:
	{
		CS_CHAT_PACKET* p = reinterpret_cast<CS_CHAT_PACKET*>(packet);
		Logic::BroadCastMessInViewList(playerId, p->mess);
	}
	break;

	default:
		std::cerr << "recv unknown Packet" << std::endl;
		break;
	}
}

void PacketManager::RemoveDisconnectClient(int disconnectedId, unordered_set<int>& disconnectPlayerViewList)
{
	for (const auto& pId : disconnectPlayerViewList)
		g_clients[pId]->RemoveViewListPlayer(disconnectedId);
}
