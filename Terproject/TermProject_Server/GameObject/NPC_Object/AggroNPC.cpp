#include "stdafx.h"
#include "AggroNPC.h"
#include "../../Timer/Timer.h"
#include "../../Logic/Logic.h"

extern random_device g_rd;
extern default_random_engine g_dre;
extern uniform_int_distribution<int> g_npcRandDir; // inclusive
extern uniform_int_distribution<int> g_npcRandPostion; // inclusive

extern Timer g_Timer;

void AggroNPC::RandMove()
{
	if (!GetIsArrive())return;
	auto viewList = GetViewList();
	if (viewList.empty()) {//inactive npc
		InActiveNPC();
		return;
	}
	//player in aggro range
	for (auto& id : viewList) {
		bool isAgroRange = Logic::NPC_AgroInRange(m_id, id);
		if (isAgroRange) {
			if (FindRoad(id)) {
				ChaseMove(id);
				return;
			}
		}
	}
	//npc rand Move
	auto position = GetPosition();
	auto prevPosition = position;
	bool moveRes = Logic::MoveDirection(g_npcRandDir(g_dre), position);
	if (moveRes)
		Logic::MoveGameObject(m_id, prevPosition, position);
	g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 1000ms);
}

void AggroNPC::ChaseMove(int targetId)
{
	if (!GetIsArrive())return;

	bool isAgroRange = Logic::NPC_AgroInRange(m_id, targetId);
	if (!isAgroRange) {//어그로 대상이 없다면
		auto viewList = GetViewList();
		if (viewList.empty()) {//inactive npc
			InActiveNPC();
			return;
		}
		g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 5ms);//일반 랜덤 무브로 변경
		return;
	}
	//Astar Road Move
	Logic::NPC_Attack(m_id, targetId);//내부에서 공격할 수 있는지 판단

	bool chaseRes = MoveChaseRoad();
	if (!chaseRes) {
		bool findRoadRes = FindRoad(targetId);
		if (!findRoadRes)
			g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 5ms);
		else
			g_Timer.InsertTimerQueue(EV_CHASE_MOVE, m_id, targetId, 5ms);
		return;
	}
	if (IsAbleFindRoadTime()) {
		bool findRoadRes = FindRoad(targetId);
		if (!findRoadRes) {
			g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, m_id, -1, 5ms);
			return;
		}
	}
	g_Timer.InsertTimerQueue(EV_CHASE_MOVE, m_id, targetId, 1000ms);
}
