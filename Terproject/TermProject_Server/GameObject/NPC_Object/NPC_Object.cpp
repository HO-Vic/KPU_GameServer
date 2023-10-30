#include "stdafx.h"
#include "NPC_Object.h"
#include "../../Logic/Logic.h"
#include "../../Timer/Timer.h"

extern Timer g_Timer;
extern random_device g_rd;
extern default_random_engine g_dre;
extern uniform_int_distribution<int> g_npcRandDir; // inclusive
extern uniform_int_distribution<short> g_npcRandPostion; // inclusive


NPC_Object::NPC_Object() : GameObject()
{
	m_chaseRoad.clear();
	SetRandPosition();
	pair<short, short> mapIdx = Logic::PlayerPositionToMapSession(m_position);
	Logic::InsertObjectIdMapSession(m_id, mapIdx);
	m_state = S_STATE::ST_INGAME;
}

NPC_Object::NPC_Object(int id) : GameObject(id)
{
	m_chaseRoad.clear();
	SetRandPosition();
	pair<short, short> mapIdx = Logic::PlayerPositionToMapSession(m_position);
	Logic::InsertObjectIdMapSession(m_id, mapIdx);
	m_state = S_STATE::ST_INGAME;
}

NPC_Object::~NPC_Object()
{
}

void NPC_Object::SetRandPosition()
{
	SetPosition(g_npcRandPostion(g_dre), g_npcRandPostion(g_dre));
}

void NPC_Object::RemoveViewListPlayer(int removePlayerId)
{
	if (!Logic::IsPlayer(removePlayerId))return;
	m_viewListLock.lock();
	int existElement = m_viewList.erase(removePlayerId);//exist ret 1, not exist ret 0
	m_viewListLock.unlock();
	if (!existElement) return;
}

void NPC_Object::MovePlayer(int movePlayerId)
{
	//NPC는 무브를 알 필요가 없음.
}

void NPC_Object::AddViewListPlayer(int addPlayerId)
{
	if (!Logic::IsPlayer(addPlayerId))return;
	m_viewListLock.lock();
	m_viewList.insert(addPlayerId);
	m_viewListLock.unlock();
	if (ActiveNPC()) {
		g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 5ms);
	}
}

S_STATE NPC_Object::GetPlayerState()
{
	return m_state;
}

void NPC_Object::RespawnData()
{
	m_hp = m_maxHp;
	auto mapIdx = Logic::PlayerPositionToMapSession(m_position);
	Logic::InsertObjectIdMapSession(m_id, mapIdx);
}

bool NPC_Object::ActiveNPC()
{
	bool old_state = false;
	if (atomic_compare_exchange_strong(&m_isActive, &old_state, true))
		return true;
	return false;
}

bool NPC_Object::InActiveNPC()
{
	bool old_state = true;
	if (atomic_compare_exchange_strong(&m_isActive, &old_state, false))
		return true;
	return false;
}

bool NPC_Object::DieNpc()
{
	bool old_state = true;
	if (atomic_compare_exchange_strong(&m_isArrive, &old_state, false))
		return true;
	return false;
}

bool NPC_Object::RespawnNpc()
{
	bool old_state = false;
	if (atomic_compare_exchange_strong(&m_isArrive, &old_state, true))
		return true;
	return false;
}

bool NPC_Object::GetIsArrive()
{
	return m_isArrive;
}

bool NPC_Object::FindRoad(int targetId)
{
	auto moveNode = Logic::GetAstarList(m_id, targetId);
	if (moveNode.empty()) return false;
	m_chaseRoadLock.lock();
	m_chaseRoad = moveNode;
	m_chaseRoadLock.unlock();
	m_lastFindRoadTime = system_clock::now();
	return true;
}

bool NPC_Object::FindRoad(int targetId, pair<short, short>& targetPosition)
{
	auto moveNode = Logic::GetAstarList(m_id, targetPosition);
	if (moveNode.empty()) return false;
	m_chaseRoadLock.lock();
	m_chaseRoad = moveNode;
	m_chaseRoadLock.unlock();
	m_lastFindRoadTime = system_clock::now();
	return true;
}

bool NPC_Object::MoveChaseRoad()
{
	m_chaseRoadLock.lock();
	pair<short, short> nextMoveNode;
	if (!m_chaseRoad.empty()) {
		nextMoveNode = m_chaseRoad.front();
		m_chaseRoad.pop_front();
		m_chaseRoadLock.unlock();
		Logic::MoveGameObject(m_id, m_position, nextMoveNode);
		return true;

	}
	m_chaseRoadLock.unlock();
	return false;
}

bool NPC_Object::IsAbleFindRoadTime()
{
	return m_lastFindRoadTime + 5s < system_clock::now();
}

void NPC_Object::Attacked(int attackPlayerId)
{
	//Logic::Attack(attackPlayerId, m_id);
}

short NPC_Object::AttackedDamage(int attackId, short damage)
{
	if (!GetIsArrive())return 0;
	m_hp -= damage;
	if (m_hp <= 0) {
		DieNpc();
		pair<short, short> mapIdx = Logic::PlayerPositionToMapSession(m_position);
		Logic::RemovePlayerOnMap(m_id, mapIdx);
		m_hp = 0;
		return m_exp;
	}
	return 0;
}

bool NPC_Object::IsAbleAttack()
{

	return m_lastAttackTime + 1s < std::chrono::system_clock::now();
}
