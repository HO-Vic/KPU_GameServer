#pragma once
#include "../../PCH/stdafx.h"
#include "../GameObject.h"
#include "../../Logic/Astar/AstarNode.h"

class NPC_Object : public GameObject
{
private://NPC Data
	std::atomic_bool m_isActive = false;
	std::atomic_bool m_isArrive = true;
	int m_chaseId = -1;

	std::mutex m_chaseRoadLock;
	std::list<pair<short, short>> m_chaseRoad;
	std::chrono::system_clock::time_point m_lastFindRoadTime = std::chrono::system_clock::now();
public:
	NPC_Object();
	NPC_Object(int id);
	virtual ~NPC_Object();

public:
	virtual void RemoveViewListPlayer(int removePlayerId) override;
	virtual void MovePlayer(int movePlayerId) override;
	virtual void AddViewListPlayer(int addPlayerId) override;
public:
	virtual S_STATE GetPlayerState() override;

	bool ActiveNPC();
	bool InActiveNPC();

	bool RespawnNpc();
	bool GetIsArrive();
private:
	bool DieNpc();
	void SetRandPosition();
public:

	bool FindRoad(int targetId);
	bool FindRoad(int targetId, pair<short, short>& targetPosition);
	bool MoveChaseRoad();
	bool IsAbleFindRoadTime();
	void Attacked(int attackPlayerId);
	virtual short AttackedDamage(short damage) override;
	virtual bool IsAbleAttack() override;

};

/*bool DieNpc()
{
	bool old_state = true;
	if (atomic_compare_exchange_strong(&isArrive, &old_state, false))
		return true;
	return false;
}
bool RespawnNpc()
{
	bool old_state = false;
	if (atomic_compare_exchange_strong(&isArrive, &old_state, true))
		return true;
	return false;
}
bool GetArrive() { return isArrive; }*/

/*void AStarLoad(int StartX, int startY, int destinyX, int destinyY);
std::pair<int, int> GetNextNode()
{
	GetNodeLock.lock();
	if (!npcNavigateList.empty()) {
		std::pair<int, int> retVal = npcNavigateList.begin()->myNode;
		if (!npcNavigateList.empty())
			npcNavigateList.pop_front();
		GetNodeLock.unlock();
		return retVal;
	}
	GetNodeLock.unlock();
	return std::make_pair(-100, -100);
}*/
