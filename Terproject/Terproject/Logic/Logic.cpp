#include "stdafx.h"
#include "Logic.h"
#include "../GameObject/GameObject.h"
#include "../GameObject/PlayerObject/PlayerObject.h"
#include "../GameObject/NPC_Object/NPC_Object.h"
#include "../MapSession/MapSession.h"
#include "../Timer/Timer.h"
#include "../Packet/PacketManager.h"

using namespace std;

extern array<GameObject*, MAX_USER + MAX_NPC> g_clients;
extern array < array<MapSession, 100>, 100> g_gameMap;
extern Timer g_Timer;
extern std::array<std::pair<short, short>, 31> g_mapObstacle;
extern array<int, 11> g_levelExp;
extern std::array<int, 11> g_levelMaxHp;
extern std::array<int, 11> g_levelAttackDamage;

extern random_device g_rd;
extern default_random_engine g_dre;
extern uniform_int_distribution<int> g_npcRandDir; // inclusive
extern uniform_int_distribution<int> g_npcRandPostion; // inclusive

void Logic::InitGameMap()
{
	std::cout << "init Game Map begin\n";
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			g_gameMap[i][j].SetPos(i, j);
			// initialize Obj on Local Map
		}
	}
	std::cout << "init Game Map end\n";
}

void Logic::InitGameObjects()
{
	for (int i = 0; i < MAX_USER; ++i)
		g_clients[i] = new PlayerObject(i);
	for (int i = MAX_USER; i < MAX_USER + 3; ++i)
		g_clients[i] = new NPC_Object(i);
	for (int i = MAX_USER + 3; i < MAX_USER + MAX_NPC / 2; ++i)
		g_clients[i] = new NPC_Object(i);
	for (int i = MAX_USER + MAX_NPC / 2; i < MAX_USER + MAX_NPC; ++i)
		g_clients[i] = new NPC_Object(i);
}

void Logic::InitNPC()
{
	cout << "NPC intialize begin.\n";

	//g_gameMap[clients[i].myLocalSectionIndex.first][clients[i].myLocalSectionIndex.second].InsertPlayers(clients[i]);

	//random pos같게 해야됨

	for (int i = MAX_USER; i < MAX_USER + 3; ++i) {

		g_clients[i]->InitSetting(i, 5000, 5000, 1000, 250);
		//g_clients[i].myLua = new LUA_OBJECT(clients[i]._id, "lua_script/boss.lua");
		//g_clients[i]._state = ST_INGAME;
		g_clients[i]->SetName(L"boss");
	}
	for (int i = MAX_USER + 3; i < (MAX_USER + MAX_NPC) / 2; ++i) {
		g_clients[i]->InitSetting(i, 250, 250, 70, 70);
		//g_clients[i].myLua = new LUA_OBJECT(clients[i]._id, "lua_script/boss.lua");
		//g_clients[i]._state = ST_INGAME;
		wstring name = L"AGRO";
		name.append(std::to_wstring(i));
		g_clients[i]->SetName(name);
	}
	for (int i = (MAX_USER + MAX_NPC) / 2; i < MAX_USER + MAX_NPC; ++i) {
		g_clients[i]->InitSetting(i, 600, 600, 120, 120);
		//g_clients[i].myLua = new LUA_OBJECT(clients[i]._id, "lua_script/boss.lua");
		//g_clients[i]._state = ST_INGAME;
		wstring name = L"PEACE";
		name.append(std::to_wstring(i));
		g_clients[i]->SetName(name);
	}
	cout << "NPC initialize end.\n";
	//TIMER_EVENT ev{ 1, chrono::system_clock::now() + 300s, EV_AUTO_SAVE, 0 };//5분 마다 오토 세이브 시작
	//eventTimerQueue.push(ev);
}

void Logic::InitAstarLoad()
{
	g_mapObstacle[0] = (make_pair(5, 1));
	g_mapObstacle[1] = (make_pair(6, 1));
	g_mapObstacle[2] = (make_pair(7, 1));
	g_mapObstacle[3] = (make_pair(2, 3));
	g_mapObstacle[4] = (make_pair(5, 3));
	g_mapObstacle[5] = (make_pair(14, 3));
	g_mapObstacle[6] = (make_pair(17, 4));
	g_mapObstacle[7] = (make_pair(2, 6));
	g_mapObstacle[8] = (make_pair(10, 6));
	g_mapObstacle[9] = (make_pair(6, 8));
	g_mapObstacle[10] = (make_pair(7, 8));
	g_mapObstacle[11] = (make_pair(8, 8));
	g_mapObstacle[12] = (make_pair(9, 8));
	g_mapObstacle[13] = (make_pair(10, 8));
	g_mapObstacle[14] = (make_pair(13, 8));
	g_mapObstacle[15] = (make_pair(6, 9));
	g_mapObstacle[16] = (make_pair(7, 9));
	g_mapObstacle[17] = (make_pair(8, 9));
	g_mapObstacle[18] = (make_pair(9, 9));
	g_mapObstacle[19] = (make_pair(10, 9));
	g_mapObstacle[20] = (make_pair(17, 10));
	g_mapObstacle[21] = (make_pair(2, 11));
	g_mapObstacle[22] = (make_pair(15, 13));
	g_mapObstacle[23] = (make_pair(13, 14));
	g_mapObstacle[24] = (make_pair(2, 15));
	g_mapObstacle[25] = (make_pair(6, 15));
	g_mapObstacle[26] = (make_pair(16, 15));
	g_mapObstacle[27] = (make_pair(16, 16));
	g_mapObstacle[28] = (make_pair(7, 17));
	g_mapObstacle[29] = (make_pair(8, 17));
	g_mapObstacle[30] = (make_pair(16, 17));
}

std::unordered_set<int> Logic::UpdateNearList(int playerId)
{
	std::unordered_set<int> newUpdateViewList;// = g_clients[playerId]->GetViewList();

	auto playerPosition = g_clients[playerId]->GetPosition();
	auto playerMapSessionIdx = PlayerPositionToMapSession(playerPosition);
	GetNearList(playerId, newUpdateViewList, playerMapSessionIdx);

	if (playerPosition.first % 20 < 7) {
		//left Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second);
		if (playerPosition.second % 20 < 7) {
			//top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//left top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second - 1);
		}
		else if (playerPosition.second % 20 < 13) {
			//bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//left bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second + 1);
		}
	}
	else if (playerPosition.first % 20 > 13) {
		//right Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second);
		if (playerPosition.second % 20 < 7) {
			//top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//right top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second - 1);
		}
		else if (playerPosition.second % 20 < 13) {
			//bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//right bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second + 1);
		}
	}
	else if (playerPosition.second % 20 < 7) {
		//top Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
	}
	else if (playerPosition.second % 20 > 13) {
		//bottom Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
	}
	return newUpdateViewList;
}

std::unordered_set<int> Logic::NPC_UpdateNearList(int playerId)
{
	std::unordered_set<int> newUpdateViewList;// = g_clients[playerId]->GetViewList();

	auto playerPosition = g_clients[playerId]->GetPosition();
	auto playerMapSessionIdx = PlayerPositionToMapSession(playerPosition);
	NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx);

	if (playerPosition.first % 20 < 7) {
		//left Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second);
		if (playerPosition.second % 20 < 7) {
			//top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//left top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second - 1);
		}
		else if (playerPosition.second % 20 < 13) {
			//bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//left bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second + 1);
		}
	}
	else if (playerPosition.first % 20 > 13) {
		//right Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second);
		if (playerPosition.second % 20 < 7) {
			//top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//right top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second - 1);
		}
		else if (playerPosition.second % 20 < 13) {
			//bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//right bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second + 1);
		}
	}
	else if (playerPosition.second % 20 < 7) {
		//top Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
	}
	else if (playerPosition.second % 20 > 13) {
		//bottom Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
	}
	return newUpdateViewList;
}

void Logic::MoveGameObject(int playerId, std::pair<short, short>& position)
{
	g_clients[playerId]->SetPosition(position);

	//player On Map Index UpdatePlayers
	auto mapPosition = Logic::PlayerPositionToMapSession(position);
	g_gameMap[mapPosition.first][mapPosition.second].InsertPlayer(playerId);

	unordered_set<int> prevViewList = g_clients[playerId]->GetViewList();
	//뷰 리스트 업데이트를 위한 new near List 생성
	unordered_set<int> newViewList;
	if (IsPlayer(playerId))
		newViewList = Logic::UpdateNearList(playerId);
	else newViewList = Logic::NPC_UpdateNearList(playerId);
	//send self MovePacket
	g_clients[playerId]->MovePlayer(playerId);

	//send other Player MovePacket, and apply current move Client view List
	for (auto& viewListPlayer : newViewList) {
		if (playerId == viewListPlayer)continue;
		if (!IsPlayer(viewListPlayer)) {
			if (!dynamic_cast<NPC_Object*>(g_clients[viewListPlayer])->GetIsArrive()) continue;
		}
		bool isExist = g_clients[viewListPlayer]->IsExistViewList(playerId);//뷰리스트에 추가된 플레이어 대해서 현재 내가 존재 하는지
		if (isExist) {
			g_clients[viewListPlayer]->MovePlayer(playerId);//존재 한다면 내가 움직인걸 알리고
		}
		else {
			g_clients[viewListPlayer]->AddViewListPlayer(playerId);//없다면 해당 플레이어에게 나를 추가 함
			if (!IsPlayer(viewListPlayer) && IsPlayer(playerId))
				if (dynamic_cast<NPC_Object*>(g_clients[viewListPlayer])->ActiveNPC())
					g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, viewListPlayer, -1, 5ms);
		}
		if (prevViewList.count(viewListPlayer) == 0) {// 이전 리스트에 상대 클라가 없다면
			g_clients[playerId]->AddViewListPlayer(viewListPlayer);//실제 내 클라이언트에 새로 추가된 클라이언트 정보를 보내주자
		}
	}
	for (auto& prevPlayerId : prevViewList) // 이전 리스트 중에
	{
		if (!g_clients[playerId]->IsExistViewList(prevPlayerId)) {//=>현재는 존재하지 않음
			g_clients[playerId]->RemoveViewListPlayer(prevPlayerId);//remove하라고 명령
		}
		if (!g_clients[prevPlayerId]->IsExistViewList(playerId)) {//=>현재는 존재하지 않음
			g_clients[prevPlayerId]->RemoveViewListPlayer(playerId);//remove하라고 명령
		}
	}
}

bool Logic::CheckInMap(std::pair<short, short>& position)
{
	if (position.first < 0)
		return false;
	if (position.first > 1999)
		return false;
	if (position.second < 0)
		return false;
	if (position.second > 1999)
		return false;
	return true;
}

bool Logic::CheckInMap(short x, short y)
{
	if (x < 0)
		return false;
	if (x > 1999)
		return false;
	if (y < 0)
		return false;
	if (y < 1999)
		return false;
	return true;
}

void Logic::GetNearList(int playerId, std::unordered_set<int>& newViewList, std::pair<short, short> mapSessionId)
{
	if (mapSessionId.first < 0 || mapSessionId.first > 99 || mapSessionId.second < 0 || mapSessionId.second > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId.first][mapSessionId.second].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for (auto& localPlayerId : currentLocalPlayer) {
		if (localPlayerId == playerId) continue;
		if (g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if (ViewInRange(currentPlayerPos, localPlayerId)) {
			newViewList.insert(localPlayerId);
		}
	}
}

void Logic::GetNearList(int playerId, std::unordered_set<int>& newViewList, int mapSessionId_x, int mapSessionId_y)
{
	if (mapSessionId_x < 0 || mapSessionId_x > 99 || mapSessionId_y < 0 || mapSessionId_y > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId_x][mapSessionId_y].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for (auto& localPlayerId : currentLocalPlayer) {
		if (localPlayerId == playerId) continue;
		if (g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if (ViewInRange(currentPlayerPos, localPlayerId)) {
			newViewList.insert(localPlayerId);
		}
	}
}

void Logic::NPC_GetNearList(int playerId, std::unordered_set<int>& newViewList, std::pair<short, short> mapSessionId)
{
	if (mapSessionId.first < 0 || mapSessionId.first > 99 || mapSessionId.second < 0 || mapSessionId.second > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId.first][mapSessionId.second].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for (auto& localPlayerId : currentLocalPlayer) {
		if (!IsPlayer(localPlayerId))continue;
		if (g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if (ViewInRange(currentPlayerPos, localPlayerId)) {
			newViewList.insert(localPlayerId);
		}
	}
}

void Logic::NPC_GetNearList(int playerId, std::unordered_set<int>& newViewList, int mapSessionId_x, int mapSessionId_y)
{
	if (mapSessionId_x < 0 || mapSessionId_x > 99 || mapSessionId_y < 0 || mapSessionId_y > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId_x][mapSessionId_y].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for (auto& localPlayerId : currentLocalPlayer) {
		if (!IsPlayer(localPlayerId))continue;
		if (g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if (ViewInRange(currentPlayerPos, localPlayerId)) {
			newViewList.insert(localPlayerId);
		}
	}
}

bool Logic::ViewInRange(int from, int to)
{
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if ((int)abs(player1.first - player2.first) > VIEW_RANGE)
		return false;
	if ((int)abs(player1.second - player2.second) > VIEW_RANGE)
		return false;
	return true;
}

bool Logic::ViewInRange(pair<short, short>& fromPosition, int to)
{
	auto player2 = g_clients[to]->GetPosition();
	if ((int)abs(fromPosition.first - player2.first) > VIEW_RANGE)
		return false;
	if ((int)abs(fromPosition.second - player2.second) > VIEW_RANGE)
		return false;
	return true;
}

bool Logic::AttackInRange(pair<short, short>& fromPosition, pair<short, short>& toPosition)
{
	if ((int)abs(fromPosition.first - toPosition.first) > Attack_RANGE)
		return false;
	if ((int)abs(fromPosition.second - toPosition.second) > Attack_RANGE)
		return false;
	return true;
}

bool Logic::AttackInRange(int from, int to)
{
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if ((int)abs(player1.first - player2.first) > Attack_RANGE)
		return false;
	if ((int)abs(player1.second - player2.second) > Attack_RANGE)
		return false;
	return true;
}

bool Logic::NPC_AttackInRange(pair<short, short>& fromPosition, pair<short, short>& toPosition)
{
	if ((int)abs(fromPosition.first - toPosition.first) > NPC_Attack_RANGE)
		return false;
	if ((int)abs(fromPosition.second - toPosition.second) > NPC_Attack_RANGE)
		return false;
	return true;
}

bool Logic::NPC_AttackInRange(int from, int to)
{
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if (player1.first == player2.first && player1.second == player2.second)
		return true;
	return false;
}

bool Logic::NPC_AgroInRange(int from, int to)
{
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if ((int)abs(player1.first - player2.first) > AGRO_RANGE)
		return false;
	if ((int)abs(player1.second - player2.second) > AGRO_RANGE)
		return false;
	return true;
}

bool Logic::NPC_AgroInRange(pair<short, short>& fromPosition, pair<short, short>& toPosition)
{
	if ((int)abs(fromPosition.first - toPosition.first) > AGRO_RANGE)
		return false;
	if ((int)abs(fromPosition.second - toPosition.second) > AGRO_RANGE)
		return false;
	return true;
}

pair<short, short> Logic::PlayerPositionToMapSession(pair<short, short> playerPosition)
{
	return { playerPosition.first / 20 , playerPosition.second / 20 };
}

pair<short, short> Logic::PlayerPositionToMapSession(short x, short y)
{
	return { x / 20 , y / 20 };
}

void Logic::InsertObjectIdMapSession(int objId, pair<short, short> mapIdx)
{
	g_gameMap[mapIdx.first][mapIdx.second].InsertPlayer(objId);
}

int Logic::GetNewClientId()
{
	for (int i = 0; i < MAX_USER; ++i) {
		S_STATE playerState = g_clients[i]->GetPlayerState();
		if (playerState == S_STATE::ST_PLAYER_FREE)
			return i;
	}
	return -1;
}

void Logic::PlayerAttackExecute(int playerId)
{
	if (!g_clients[playerId]->IsAbleAttack()) return;
	auto viewList = g_clients[playerId]->GetViewList();
	for (auto& id : viewList) {
		if (!IsPlayer(id)) {
			Attack(playerId, id);
		}
	}
	g_clients[playerId]->ResetLastAttack();
	if (g_clients[playerId]->IsAbleLevelUp()) {
		short level = g_clients[playerId]->GetLevel();
		if (level == g_levelExp.size()) return;
		short restExp = g_clients[playerId]->GetExp() - g_levelExp[level - 1];
		g_clients[playerId]->SetExp(restExp);
		g_clients[playerId]->SetMaxExp(g_levelExp[level]);
		g_clients[playerId]->LevelUp();
		g_clients[playerId]->SetHp(g_levelMaxHp[level]);
		g_clients[playerId]->SetMaxHp(g_levelMaxHp[level]);
		g_clients[playerId]->SetAttackDamage(g_levelAttackDamage[level]);
		dynamic_cast<PlayerObject*>(g_clients[playerId])->SaveData();
	}
	PacketManager::SendStatPacketSelf(playerId);
}

bool Logic::MoveDirection(char direction, std::pair<short, short>& position)
{
	switch (direction) {
	case 1: if (position.second > 0) position.second--; break;
	case 2: if (position.second < W_HEIGHT - 1) position.second++; break;
	case 3: if (position.first > 0) position.first--; break;
	case 4: if (position.first < W_WIDTH - 1) position.first++; break;
	}
	auto mapIdx = Logic::PlayerPositionToMapSession(make_pair(position.first, position.second));
	return !g_gameMap[mapIdx.first][mapIdx.second].CollisionObject(position);
	//map Object Collision
}

void Logic::NPCMove(int npcId)
{
	NPC_Object* npc = dynamic_cast<NPC_Object*>(g_clients[npcId]);
	if (!npc->GetIsArrive())return;

	auto viewList = npc->GetViewList();
	if (viewList.empty()) {//inactive npc
		npc->InActiveNPC();
		return;
	}
	for (auto& id : viewList) {//npc chase player
		bool isAgroRange = NPC_AgroInRange(npcId, id);
		if (isAgroRange) {
			//Astar Calculate
			auto tagetPosition = g_clients[npcId]->GetPosition();
			npc->FindRoad(id, tagetPosition);
			//Timer Event Chase
			g_Timer.InsertTimerQueue(EV_CHASE_MOVE, npcId, id, 1000ms);
			return;
		}
	}
	//npc rand Move
	auto position = npc->GetPosition();
	bool moveRes = Logic::MoveDirection(g_npcRandDir(g_dre), position);
	if (moveRes)
		Logic::MoveGameObject(npcId, position);
	g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, npcId, -1, 1000ms);
}

void Logic::NPCMove(int npcId, int targetId)
{
	NPC_Object* npc = dynamic_cast<NPC_Object*>(g_clients[npcId]);
	if (!npc->GetIsArrive())return;
	bool isAgroRange = NPC_AgroInRange(npcId, targetId);
	if (!isAgroRange) {//어그로 대상이 없다면
		auto viewList = npc->GetViewList();
		if (viewList.empty()) {//inactive npc
			npc->InActiveNPC();
			return;
		}
		g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, npcId, -1, 5ms);//일반 랜덤 무브로 변경
		return;
	}
	//Astar Road Move
	NPC_Attack(npcId, targetId);//내부에서 공격할 수 있는지 판단

	bool chaseRes = npc->MoveChaseRoad();
	if (!chaseRes) {
		auto tagetPosition = g_clients[targetId]->GetPosition();
		bool findRoadRes = npc->FindRoad(targetId, tagetPosition);
		if (!findRoadRes)
			g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, npcId, -1, 5ms);
		else
			g_Timer.InsertTimerQueue(EV_CHASE_MOVE, npcId, targetId, 5ms);
		return;
	}
	if (npc->IsAbleFindRoadTime()) {
		auto tagetPosition = g_clients[targetId]->GetPosition();
		bool findRoadRes = npc->FindRoad(targetId, tagetPosition);
		if (!findRoadRes)
			g_Timer.InsertTimerQueue(EV_RANDOM_MOVE, npcId, -1, 5ms);
		else
			g_Timer.InsertTimerQueue(EV_CHASE_MOVE, npcId, targetId, 1000ms);
		return;
	}
	g_Timer.InsertTimerQueue(EV_CHASE_MOVE, npcId, targetId, 1000ms);
}

void Logic::Attack(int attackObjId, int attackedObjId)
{
	if (!g_clients[attackObjId]->IsAbleAttack())return;
	if (!AttackInRange(attackObjId, attackedObjId))return;
	short getExp = g_clients[attackedObjId]->AttackedDamage(g_clients[attackObjId]->GetAttackDamage());
	dynamic_cast<PlayerObject*>(g_clients[attackObjId])->ConsumeExp(getExp);
	auto viewList = g_clients[attackedObjId]->GetViewList();
	PacketManager::SendStatPacketInViewList(viewList, attackedObjId);
}

void Logic::NPC_Attack(int attackObjId, int attackedObjId)
{
	if (!g_clients[attackObjId]->IsAbleAttack())return;
	if (!NPC_AttackInRange(attackObjId, attackedObjId))return;
	g_clients[attackedObjId]->AttackedDamage(g_clients[attackObjId]->GetAttackDamage());
	auto viewList = g_clients[attackedObjId]->GetViewList();
	PacketManager::SendStatPacketInViewList(viewList, attackedObjId);
	g_clients[attackObjId]->ResetLastAttack();
}

void Logic::InsertRespawnNPC(int npcId)
{
	g_Timer.InsertTimerQueue(EV_RESPAWN_NPC, npcId, -1, 1min);
}

float Logic::GetDistance(pair<short, short>& p1, pair<short, short>& p2)
{
	//return sqrt(pow((float)(p1.first - p2.first), 2) + pow((float)(p1.second - p2.second), 2));
	return pow((float)(p1.first - p2.first), 2) + pow((float)(p1.second - p2.second), 2);
}

float Logic::GetDistance(pair<short, short>& p1, short x, short y)
{
	//return sqrt(pow((float)(p1.first - x), 2) + pow((float)(p1.second - y), 2));
	return pow((float)(p1.first - x), 2) + pow((float)(p1.second - y), 2);
}

void Logic::InsertOpenList(const std::map<int, AstarNode>& closeList, std::map<int, AstarNode>& openList, pair<short, short>& targetNode, pair<short, short>& parentNode, short nextNodeX, short nextNodeY)
{
	pair<short, short> nextNode = make_pair(nextNodeX, nextNodeY);
	bool checkInMap = CheckInMap(nextNode);
	if (!checkInMap)
		return;

	auto nextgameMapIdx = Logic::PlayerPositionToMapSession(nextNode);
	bool isCollideMap = g_gameMap[nextgameMapIdx.first][nextgameMapIdx.second].CollisionObject(nextNode);
	if (isCollideMap)
		return;

	float distance = GetDistance(targetNode, nextNode);

	if (closeList.count(nextNodeY * 20 + nextNodeX)) return;
	if (openList.count(nextNodeY * 20 + nextNodeX)) {
		openList[nextNodeY * 20 + nextNodeX].ModifyDistance(distance, parentNode);
	}
	else openList.try_emplace(nextNodeY * 20 + nextNodeX, nextNode, parentNode, distance);
}

std::list<pair<short, short>> Logic::GetAstarList(int npcId, int targetId)
{
	auto npcPosition = g_clients[npcId]->GetPosition();
	auto targetPosition = g_clients[targetId]->GetPosition();
	if (!Logic::NPC_AgroInRange(npcPosition, targetPosition))return std::list<pair<short, short>>{};

	std::map<int, AstarNode> openListMap;
	std::map<int, AstarNode> closeListMap;

	closeListMap.try_emplace(npcPosition.second * 20 + npcPosition.first, npcPosition, npcPosition, 0);
	auto currentNode = npcPosition;

	while (true) {
		//auto currentgameMapIdx = Logic::PlayerPositionToMapSession(currentNode);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first - 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first + 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second - 1);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second + 1);

		float minDistace = _FMAX;
		int minIdx = -1;
		for (auto& node : openListMap) {
			if (minDistace > node.second.GetDistance()) {
				minDistace = node.second.GetDistance();
				minIdx = node.first;
			}
		}
		///////////////////////
		if (-1 == minIdx) {
			return std::list<pair<short, short>>{};
		}
		if (-1 != minIdx) {
			currentNode = openListMap[minIdx].GetNodePos();
			closeListMap.try_emplace(minIdx, openListMap[minIdx]);
			openListMap.erase(minIdx);
		}

		if (currentNode == targetPosition) {
			std::list<pair<short, short>> resList;
			int resIdx = minIdx;
			while (true) {
				AstarNode& currentNode = closeListMap[resIdx];
				resList.emplace_front(currentNode.GetNodePos());
				if (currentNode.GetNodePos() == npcPosition) {
					return resList;
				}
				resIdx = currentNode.GetParentNodePos().first + currentNode.GetParentNodePos().second * 20;
			}
		}
	}
}

std::list<pair<short, short>> Logic::GetAstarList(int npcId, pair<short, short>& targetPos)
{
	auto npcPosition = g_clients[npcId]->GetPosition();
	auto targetPosition = targetPos;
	if (!Logic::NPC_AgroInRange(npcPosition, targetPosition))return std::list<pair<short, short>>{};
	if (targetPosition == npcPosition)return std::list<pair<short, short>>{targetPosition};
	std::map<int, AstarNode> openListMap;
	std::map<int, AstarNode> closeListMap;

	closeListMap.try_emplace(npcPosition.second * 20 + npcPosition.first, npcPosition, npcPosition, 0);
	auto currentNode = npcPosition;

	while (true) {
		//auto currentgameMapIdx = Logic::PlayerPositionToMapSession(currentNode);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first - 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first + 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second - 1);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second + 1);

		float minDistace = _FMAX;
		int minIdx = -1;
		for (auto& node : openListMap) {
			if (minDistace > node.second.GetDistance()) {
				minDistace = node.second.GetDistance();
				minIdx = node.first;
			}
		}
		///////////////////////
		if (-1 == minIdx) {
			return std::list<pair<short, short>>{};
		}
		if (-1 != minIdx) {
			currentNode = openListMap[minIdx].GetNodePos();
			closeListMap.try_emplace(minIdx, openListMap[minIdx]);
			openListMap.erase(minIdx);
		}

		if (currentNode == targetPosition) {
			std::list<pair<short, short>> resList;
			int resIdx = minIdx;
			while (true) {
				AstarNode& currentNode = closeListMap[resIdx];
				resList.emplace_front(currentNode.GetNodePos());
				if (currentNode.GetNodePos() == npcPosition) {
					return resList;
				}
				resIdx = currentNode.GetParentNodePos().first + currentNode.GetParentNodePos().second * 20;
			}
		}
	}
}