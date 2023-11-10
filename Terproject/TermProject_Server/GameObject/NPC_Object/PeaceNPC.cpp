#include "stdafx.h"
#include "PeaceNPC.h"
#include "../../Timer/Timer.h"
#include "../../Logic/Logic.h"

extern random_device g_rd;
extern default_random_engine g_dre;
extern uniform_int_distribution<int> g_npcRandDir; // inclusive
extern uniform_int_distribution<int> g_npcRandPostion; // inclusive

extern Timer g_Timer;

bool PeaceNPC::AcitiveAggro()
{
	bool old_state = false;
	if (atomic_compare_exchange_strong(&m_isAggro, &old_state, true))
		return true;
	return false;
}

bool PeaceNPC::InacitiveAggro()
{
	bool old_state = true;
	if (atomic_compare_exchange_strong(&m_isAggro, &old_state, false))
		return true;
	return false;
}

void PeaceNPC::RandMove()
{
	if (!GetIsArrive())return;
	if (m_isAggro) return;
	auto viewList = GetViewList();
	if (viewList.empty()) {//inactive npc
		InActiveNPC();
		return;
	}
	//npc rand Move
	auto position = GetPosition();
	auto prevPosition = position;
	bool moveRes = Logic::MoveDirection(g_npcRandDir(g_dre), position);
	if (moveRes)
		Logic::MoveGameObject(m_id, prevPosition, position);
	g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 1000ms);
}

void PeaceNPC::ChaseMove(int targetId)
{
	if (!GetIsArrive())return;

	bool isAgroRange = Logic::NPC_AgroInRange(m_id, targetId);
	if (!isAgroRange) {//어그로 대상이 없다면
		auto viewList = GetViewList();
		if (viewList.empty()) {//inactive npc
			InActiveNPC();
			return;
		}
		if (InacitiveAggro())
			g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 5ms);//일반 랜덤 무브로 변경
		return;
	}
	//Astar Road Move
	Logic::NPC_Attack(m_id, targetId);//내부에서 공격할 수 있는지 판단

	bool chaseRes = MoveChaseRoad();
	if (!chaseRes) {
		bool findRoadRes = FindRoad(targetId);
		if (!findRoadRes) {
			if (InacitiveAggro())
				g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 5ms);
		}
		else
			g_Timer.InsertTimerQueue(EV_CHASE_MOVE, m_id, targetId, 5ms);
		return;
	}
	if (IsAbleFindRoadTime()) {
		bool findRoadRes = FindRoad(targetId);
		if (!findRoadRes) {
			if (InacitiveAggro())
				g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 5ms);
			return;
		}
	}
	g_Timer.InsertTimerQueue(EV_CHASE_MOVE, m_id, targetId, 1000ms);
}

short PeaceNPC::AttackedDamage(int attackId, short damage)
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
	if (AcitiveAggro()) {
		g_Timer.InsertTimerQueue(EV_CHASE_MOVE, m_id, attackId, 1000ms);
	}
	return 0;
}

void PeaceNPC::RespawnData()
{
	NPC_Object::RespawnData();
	InacitiveAggro();
}
