#pragma once
#include "../PCH/stdafx.h"
#include "Astar/AstarNode.h"


class Logic
{
public:
	static void InitGameMap();
	static void InitGameObjects();
	static void InitNPC();
	static void InitAstarLoad();

public:
	static bool IsPlayer(int id);
	static int GetNewClientId();
	static void PlayerIngameState(char* dbPlayerData);
	static void DisconnectClient(int disconnectId);

public:
	static bool MoveDirection(char direction, std::pair<short, short>& position);
	static void NPCMove(int npcId);
	static void NPCMove(int npcId, int targetId);

	static void MoveGameObject(int playerId, std::pair<short, short>& prevPosition, std::pair<short, short>& position);

	static bool CheckInMap(std::pair<short, short>& position);
	static bool CheckInMap(short x, short y);
	static void RemovePlayerOnMap(int objectId, std::pair<short, short> position);

	static std::unordered_set<int> UpdateNearList(int playerId);
	static std::unordered_set<int> NPC_UpdateNearList(int playerId);
private:
	static void GetNearList(int playerId, std::unordered_set<int>& newViewList, std::pair<short, short> mapSessionId);
	static void GetNearList(int playerId, std::unordered_set<int>& newViewList, int mapSessionId_x, int mapSessionId_y);
	static void NPC_GetNearList(int playerId, std::unordered_set<int>& newViewList, std::pair<short, short> mapSessionId);
	static void NPC_GetNearList(int playerId, std::unordered_set<int>& newViewList, int mapSessionId_x, int mapSessionId_y);
	static void ProccessViewList(int objectId, const std::unordered_set<int>& prevViewList, const std::unordered_set<int>& newViewList);

	static bool ViewInRange(int from, int to);
	static bool ViewInRange(pair<short, short>& fromPosition, int to);
	static bool AttackInRange(pair<short, short>& fromPosition, pair<short, short>& toPosition);
	static bool AttackInRange(int from, int to);
	static bool NPC_AttackInRange(pair<short, short>& fromPosition, pair<short, short>& toPosition);
	static bool NPC_AttackInRange(int from, int to);
	static bool NPC_AgroInRange(int from, int to);
	static bool NPC_AgroInRange(pair<short, short>& fromPosition, pair<short, short>& toPosition);

public:
	static pair<short, short> PlayerPositionToMapSession(pair<short, short> playerPosition);
	static pair<short, short> PlayerPositionToMapSession(short x, short y);
	static void InsertObjectIdMapSession(int objId, pair<short, short> mapIdx);
public:
	static void PlayerAttackExecute(int playerId);
	static void Attack(int attackObjId, int attackedObjId);
	static void NPC_Attack(int attackObjId, int attackedObjId);
	static void InsertRespawnNPC(int npcId);
	static void RespawnNPC(int npcId);
private:
	static float GetDistance(pair<short, short>& p1, pair<short, short>& p2);
	static float GetDistance(pair<short, short>& p1, short x, short y);
public:
	static std::list<pair<short, short>> GetAstarList(int npcId, int targetId);
	static std::list<pair<short, short>> GetAstarList(int npcId, pair<short, short>& targetPos);

private:
	static void InsertOpenList(const std::map<int, AstarNode>& closeList, std::map<int, AstarNode>& openList, pair<short, short>& targetNode, pair<short, short>& parentNode, short nextNodeX, short nextNodeY);
public:
	static void SendMess(int from, int to, wchar_t* mess);
	static void BroadCastMessInViewList(int playerId, wchar_t* mess);
public:
	static void AutoSaveAllPlayers();
};



//void disconnect(int c_id)
//{
//	clients[c_id]._vl.lock();
//	unordered_set <int> vl = clients[c_id]._view_list;
//	clients[c_id]._vl.unlock();
//	for (auto& p_id : vl) {
//		auto& pl = clients[p_id];
//		{
//			lock_guard<mutex> ll(pl._s_lock);
//			if (ST_INGAME != pl._state) continue;
//		}
//		if (pl._id == c_id) continue;
//		pl.send_remove_player_packet(c_id);
//	}
//	closesocket(clients[c_id]._socket);
//
//	lock_guard<mutex> ll(clients[c_id]._s_lock);
//	clients[c_id]._state = ST_FREE;
//}
