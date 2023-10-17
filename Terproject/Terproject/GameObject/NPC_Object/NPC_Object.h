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
	void RespawnData();
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
