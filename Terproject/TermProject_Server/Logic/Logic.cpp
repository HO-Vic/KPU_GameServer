#include "stdafx.h"
#include <chrono>
#include "Logic.h"
#include "../GameObject/GameObject.h"
#include "../GameObject/PlayerObject/PlayerObject.h"
#include "../GameObject/NPC_Object/NPC_Object.h"
#include "../GameObject/NPC_Object/AggroNPC.h"
#include "../GameObject/NPC_Object/PeaceNPC.h"
#include "../MapSession/MapSession.h"
#include "../Timer/Timer.h"
#include "../Packet/PacketManager.h"
#include "../DB/DB_Event.h"
#include "../Metric/Metric.h"

using namespace std;

extern array<GameObject *, MAX_USER + MAX_NPC> g_clients;
extern array < array<MapSession, 100>, 100> g_gameMap;
extern Timer g_Timer;
extern std::array<std::pair<short, short>, 31> g_mapObstacle;
extern std::array<std::pair<short, short>, 29> g_vilageObstacle;
extern array<int, 11> g_levelExp;
extern std::array<int, 11> g_levelMaxHp;
extern std::array<int, 11> g_levelAttackDamage;

concurrency::concurrent_queue<int> m_restIdQueue;

void Logic::InitGameMap(){
	std::cout << "init Game Map begin\n";
	for(int i = 0; i < 100; i++){
		for(int j = 0; j < 100; j++){
			g_gameMap[i][j].SetPos(i, j);
			// initialize Obj on Local Map
		}
	}
	std::cout << "init Game Map end\n";
}

void Logic::InitGameObjects(){
	for(int i = 0; i < MAX_USER; ++i)
		g_clients[i] = new PlayerObject(i);
	for(int i = MAX_USER; i < MAX_USER + ( MAX_NPC / 2 ); ++i)
		g_clients[i] = new AggroNPC(i);
	for(int i = MAX_USER + ( MAX_NPC / 2 ); i < MAX_USER + MAX_NPC; ++i)
		g_clients[i] = new PeaceNPC(i);
}

void Logic::InitNPC(){
	cout << "NPC intialize begin.\n";

	for(int i = MAX_USER; i < MAX_USER + ( MAX_NPC / 2 ); ++i){
		g_clients[i]->InitSetting(i, 250, 250, 70, 70);
		wstring name = L"AGRO";
		name.append(std::to_wstring(i));
		g_clients[i]->SetName(name);
	}
	for(int i = MAX_USER + ( MAX_NPC / 2 ); i < MAX_USER + MAX_NPC; ++i){
		g_clients[i]->InitSetting(i, 600, 600, 120, 120);
		wstring name = L"PEACE";
		name.append(std::to_wstring(i));
		g_clients[i]->SetName(name);
	}
	cout << "NPC initialize end.\n";
}

void Logic::InitAstarLoad(){
	g_mapObstacle[0] = ( make_pair(5, 1) );
	g_mapObstacle[1] = ( make_pair(6, 1) );
	g_mapObstacle[2] = ( make_pair(7, 1) );
	g_mapObstacle[3] = ( make_pair(2, 3) );
	g_mapObstacle[4] = ( make_pair(5, 3) );
	g_mapObstacle[5] = ( make_pair(14, 3) );
	g_mapObstacle[6] = ( make_pair(17, 4) );
	g_mapObstacle[7] = ( make_pair(2, 6) );
	g_mapObstacle[8] = ( make_pair(10, 6) );
	g_mapObstacle[9] = ( make_pair(6, 8) );
	g_mapObstacle[10] = ( make_pair(7, 8) );
	g_mapObstacle[11] = ( make_pair(8, 17) );
	g_mapObstacle[12] = ( make_pair(16, 17) );
	g_mapObstacle[13] = ( make_pair(10, 8) );
	g_mapObstacle[14] = ( make_pair(13, 8) );
	g_mapObstacle[15] = ( make_pair(6, 9) );
	g_mapObstacle[16] = ( make_pair(7, 9) );
	g_mapObstacle[17] = ( make_pair(9, 8) );
	g_mapObstacle[18] = ( make_pair(9, 9) );
	g_mapObstacle[19] = ( make_pair(10, 9) );
	g_mapObstacle[20] = ( make_pair(17, 10) );
	g_mapObstacle[21] = ( make_pair(2, 11) );
	g_mapObstacle[22] = ( make_pair(15, 13) );
	g_mapObstacle[23] = ( make_pair(13, 14) );
	g_mapObstacle[24] = ( make_pair(2, 15) );
	g_mapObstacle[25] = ( make_pair(6, 15) );
	g_mapObstacle[26] = ( make_pair(16, 15) );
	g_mapObstacle[27] = ( make_pair(16, 16) );
	g_mapObstacle[28] = ( make_pair(7, 17) );
	g_mapObstacle[29] = ( make_pair(8, 8) );
	g_mapObstacle[30] = ( make_pair(8, 9) );

	for(int i = 0; i < g_vilageObstacle.size(); ++i)
		g_vilageObstacle[i] = g_mapObstacle[i];
}

bool Logic::IsPlayer(int id){
	return id < MAX_USER;
}

int Logic::GetNewClientId(){
	/*for (int i = 0; i < MAX_USER; i++) {
		if (g_clients[i]->GetPlayerState() == ST_PLAYER_FREE)
			return i;
	}*/
	static int i = 0;
	if(i < MAX_USER) return i++;
	int id;
	if(m_restIdQueue.try_pop(id)){
		return id;
	}
	return -1;
}

void Logic::PlayerIngameState(char * dbPlayerData){
	DB::DB_PlayerInfo * playerData = reinterpret_cast<DB::DB_PlayerInfo *>(dbPlayerData);

	PlayerObject * playerObject = reinterpret_cast<PlayerObject *>( g_clients[playerData->m_id] );
	playerObject->InitSetting(playerData->m_id, playerData->m_hp, playerData->m_maxHp, playerData->m_attackDamage, playerData->m_exp, playerData->m_posX, playerData->m_posY);
	playerObject->SetName(playerData->m_playerName);
	playerObject->SetLoginId(playerData->m_playerId);
	playerObject->SetMaxExp(g_levelExp[playerData->m_level - 1]);
	playerObject->SetLevel(playerData->m_level);
	auto pos = playerObject->GetPosition();
	playerObject->SendLoginInfoPacket();
	unordered_set<int> nullSet;
	unordered_set<int> newViewList = Logic::UpdateNearList(playerData->m_id);
	auto mapIdx = Logic::PlayerPositionToMapSession(pos);
	Logic::InsertObjectIdMapSession(playerData->m_id, mapIdx);
	ProccessViewList(playerData->m_id, nullSet, newViewList);
}

void Logic::DisconnectClient(int disconnectId){
	if(!IsPlayer(disconnectId))return;
	if(g_clients[disconnectId]->GetPlayerState() != ST_INGAME)return;
	//view List 정리 해주고, => remove Player
	auto playerPosition = g_clients[disconnectId]->GetPosition();
	RemovePlayerOnMap(disconnectId, playerPosition);
	dynamic_cast<PlayerObject *>( g_clients[disconnectId] )->Disconnect();
	m_restIdQueue.push(disconnectId);

}

bool Logic::MoveDirection(char direction, std::pair<short, short> & position){
	switch(direction){
	case 1: if(position.second > 0) position.second--; break;
	case 2: if(position.second < W_HEIGHT - 1) position.second++; break;
	case 3: if(position.first > 0) position.first--; break;
	case 4: if(position.first < W_WIDTH - 1) position.first++; break;
	}
	auto mapIdx = Logic::PlayerPositionToMapSession(make_pair(position.first, position.second));
	return !g_gameMap[mapIdx.first][mapIdx.second].CollisionObject(position);
	//map Object Collision
}

void Logic::NPCMove(int npcId){
	NPC_Object * npc = dynamic_cast<NPC_Object *>(g_clients[npcId]);
	npc->RandMove();
}

void Logic::NPCMove(int npcId, int targetId){
	NPC_Object * npc = dynamic_cast<NPC_Object *>( g_clients[npcId] );
	npc->ChaseMove(targetId);
}

void Logic::MoveGameObject(int playerId, std::pair<short, short> & prevPosition, std::pair<short, short> & position){
	g_clients[playerId]->SetPosition(position);

	//player On Map Index UpdatePlayers
	auto mapPosition = Logic::PlayerPositionToMapSession(position);
	auto prevMapPosition = Logic::PlayerPositionToMapSession(prevPosition);
	if(mapPosition != prevMapPosition){
		g_gameMap[mapPosition.first][mapPosition.second].InsertPlayer(playerId);
		g_gameMap[prevMapPosition.first][prevMapPosition.second].DeletePlayer(playerId);
	}
	//Concurrency::concurrent_unordered_set<int>& prevViewList = g_clients[playerId]->GetViewList();
	unordered_set<int> prevViewList = g_clients[playerId]->GetViewList();
	//뷰 리스트 업데이트를 위한 new near List 생성
	unordered_set<int> newViewList;
	if(IsPlayer(playerId))
		newViewList = Logic::UpdateNearList(playerId);
	else newViewList = Logic::NPC_UpdateNearList(playerId);
	//send self MovePacket
	g_clients[playerId]->MovePlayer(playerId);
	ProccessViewList(playerId, prevViewList, newViewList);
}

bool Logic::CheckInMap(std::pair<short, short> & position){
	if(position.first < 0)
		return false;
	if(position.first > 1999)
		return false;
	if(position.second < 0)
		return false;
	if(position.second > 1999)
		return false;
	return true;
}

bool Logic::CheckInMap(short x, short y){
	if(x < 0)
		return false;
	if(x > 1999)
		return false;
	if(y < 0)
		return false;
	if(y < 1999)
		return false;
	return true;
}

void Logic::RemovePlayerOnMap(int objectId, std::pair<short, short> position){
	auto mapIdx = PlayerPositionToMapSession(position);
	g_gameMap[mapIdx.first][mapIdx.second].DeletePlayer(objectId);
}


std::unordered_set<int> Logic::UpdateNearList(int playerId){
	using namespace std::chrono;

	//{
	//	steady_clock::time_point startTime = steady_clock::now();
	//	std::unordered_set<int> newUpdateViewList;
	//	for(int i = 0; i < MAX_USER + MAX_NPC; ++i){
	//		if(playerId == i){
	//			continue;
	//		}
	//		if(g_clients[i]->GetPlayerState() != ST_INGAME) continue;
	//		if(ViewInRange(playerId, i)){
	//			newUpdateViewList.emplace(i);
	//		}
	//	}
	//	steady_clock::time_point endTime = steady_clock::now();
	//	auto elapsedTime = duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	//	auto & metric = Metric::GetInstance().GetCurrentMetric();
	//	metric.totalGridElapsedTime.fetch_add(elapsedTime);
	//	metric.totalProcessCnt.fetch_add(1);
	//	return newUpdateViewList;
	//}

	steady_clock::time_point startTime = steady_clock::now();
	pair<short, short> playerPosition = g_clients[playerId]->GetPosition();
	pair<short, short> playerMapSessionIdx = PlayerPositionToMapSession(playerPosition);

	std::unordered_set<int> newUpdateViewList;
	Logic::GetNearList(playerId, newUpdateViewList, playerMapSessionIdx);

	if(playerPosition.first % 20 < 7){
		//left Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second);
		if(playerPosition.second % 20 < 7){
			//top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//left top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second - 1);
		} else if(playerPosition.second % 20 < 13){
			//bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//left bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second + 1);
		}
	} else if(playerPosition.first % 20 > 13){
		//right Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second);
		if(playerPosition.second % 20 < 7){
			//top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//right top Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second - 1);
		} else if(playerPosition.second % 20 < 13){
			//bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//right bottom Session
			GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second + 1);
		}
	} else if(playerPosition.second % 20 < 7){
		//top Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
	} else if(playerPosition.second % 20 > 13){
		//bottom Session
		GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
	}
	steady_clock::time_point endTime = steady_clock::now();
	auto elapsedTime = duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
	auto & metric = Metric::GetInstance().GetCurrentMetric();
	metric.totalGridElapsedTime.fetch_add(elapsedTime);
	metric.totalProcessCnt.fetch_add(1);

	return newUpdateViewList;
}

std::unordered_set<int> Logic::NPC_UpdateNearList(int playerId){
	using namespace std::chrono;
	//{
	//	steady_clock::time_point startTime = steady_clock::now();
	//	std::unordered_set<int> newUpdateViewList;
	//	for(int i = 0; i < MAX_USER; ++i){
	//		if(playerId == i){
	//			continue;
	//		}
	//		if(!IsPlayer(i))continue;
	//		if(g_clients[i]->GetPlayerState() != ST_INGAME) continue;
	//		if(ViewInRange(playerId, i)){
	//			newUpdateViewList.emplace(i);
	//		}
	//	}
	//	steady_clock::time_point endTime = steady_clock::now();
	//	auto elapsedTime = duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
	//	auto & metric = Metric::GetInstance().GetCurrentMetric();
	//	metric.totalGridElapsedTime.fetch_add(elapsedTime);
	//	metric.totalProcessCnt.fetch_add(1);
	//	return newUpdateViewList;
	//}
	steady_clock::time_point startTime = steady_clock::now();
	auto playerPosition = g_clients[playerId]->GetPosition();
	auto playerMapSessionIdx = PlayerPositionToMapSession(playerPosition);

	std::unordered_set<int> newUpdateViewList;// = g_clients[playerId]->GetViewList();
	Logic::NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx);

	if(playerPosition.first % 20 < 7){
		//left Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second);
		if(playerPosition.second % 20 < 7){
			//top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//left top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second - 1);
		} else if(playerPosition.second % 20 < 13){
			//bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//left bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first - 1, playerMapSessionIdx.second + 1);
		}
	} else if(playerPosition.first % 20 > 13){
		//right Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second);
		if(playerPosition.second % 20 < 7){
			//top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
			//right top Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second - 1);
		} else if(playerPosition.second % 20 < 13){
			//bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
			//right bottom Session
			NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first + 1, playerMapSessionIdx.second + 1);
		}
	} else if(playerPosition.second % 20 < 7){
		//top Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second - 1);
	} else if(playerPosition.second % 20 > 13){
		//bottom Session
		NPC_GetNearList(playerId, newUpdateViewList, playerMapSessionIdx.first, playerMapSessionIdx.second + 1);
	}
	steady_clock::time_point endTime = steady_clock::now();
	auto elapsedTime = duration_cast<std::chrono::microseconds>( endTime - startTime ).count();
	auto & metric = Metric::GetInstance().GetCurrentMetric();
	metric.totalGridElapsedTime.fetch_add(elapsedTime);
	metric.totalProcessCnt.fetch_add(1);
	return newUpdateViewList;
}

void Logic::GetNearList(int playerId, std::unordered_set<int> & newViewList, std::pair<short, short> mapSessionId){
	if(mapSessionId.first < 0 || mapSessionId.first > 99 || mapSessionId.second < 0 || mapSessionId.second > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId.first][mapSessionId.second].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for(auto & localPlayerId : currentLocalPlayer){
		if(localPlayerId == playerId) continue;
		if(g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if(ViewInRange(currentPlayerPos, localPlayerId)){
			newViewList.insert(localPlayerId);
		}
	}
}

void Logic::GetNearList(int playerId, std::unordered_set<int> & newViewList, int mapSessionId_x, int mapSessionId_y){
	if(mapSessionId_x < 0 || mapSessionId_x > 99 || mapSessionId_y < 0 || mapSessionId_y > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId_x][mapSessionId_y].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for(auto & localPlayerId : currentLocalPlayer){
		if(localPlayerId == playerId) continue;
		if(g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if(ViewInRange(currentPlayerPos, localPlayerId)){
			newViewList.insert(localPlayerId);
		}
	}
}

void Logic::NPC_GetNearList(int playerId, std::unordered_set<int> & newViewList, std::pair<short, short> mapSessionId){
	if(mapSessionId.first < 0 || mapSessionId.first > 99 || mapSessionId.second < 0 || mapSessionId.second > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId.first][mapSessionId.second].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for(auto & localPlayerId : currentLocalPlayer){
		if(!IsPlayer(localPlayerId))continue;
		if(g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if(ViewInRange(currentPlayerPos, localPlayerId)){
			newViewList.insert(localPlayerId);
		}
	}
}

void Logic::NPC_GetNearList(int playerId, std::unordered_set<int> & newViewList, int mapSessionId_x, int mapSessionId_y){
	if(mapSessionId_x < 0 || mapSessionId_x > 99 || mapSessionId_y < 0 || mapSessionId_y > 99) return;
	std::unordered_set<int> currentLocalPlayer = g_gameMap[mapSessionId_x][mapSessionId_y].GetPlayer();
	auto currentPlayerPos = g_clients[playerId]->GetPosition();
	for(auto & localPlayerId : currentLocalPlayer){
		if(!IsPlayer(localPlayerId))continue;
		if(g_clients[localPlayerId]->GetPlayerState() != ST_INGAME) continue;
		if(ViewInRange(currentPlayerPos, localPlayerId)){
			newViewList.insert(localPlayerId);
		}
	}
}

void Logic::ProccessViewList(int objectId, const std::unordered_set<int> & prevViewList, const std::unordered_set<int> & newViewList){
	//send other Player MovePacket, and apply current move Client view List
	for(auto & viewListPlayer : newViewList){
		if(objectId == viewListPlayer)continue;
		if(!IsPlayer(viewListPlayer)){
			if(!dynamic_cast<NPC_Object *>( g_clients[viewListPlayer] )->GetIsArrive()) continue;
		}
		bool isExist = g_clients[viewListPlayer]->IsExistViewList(objectId);//뷰리스트에 추가된 플레이어 대해서 현재 내가 존재 하는지
		if(isExist){
			g_clients[viewListPlayer]->MovePlayer(objectId);//존재 한다면 내가 움직인걸 알리고
		} else{
			g_clients[viewListPlayer]->AddViewListPlayer(objectId);//없다면 해당 플레이어에게 나를 추가 함		
		}
		if(prevViewList.count(viewListPlayer) == 0){// 이전 리스트에 상대 클라가 없다면
			g_clients[objectId]->AddViewListPlayer(viewListPlayer);//실제 내 클라이언트에 새로 추가된 클라이언트 정보를 보내주자
		}
	}
	for(auto & prevPlayerId : prevViewList) // 이전 리스트 중에
	{
		if(!newViewList.count(prevPlayerId)){
			if(g_clients[objectId]->IsExistViewList(prevPlayerId)){//=>현재는 존재하지 않음
				g_clients[objectId]->RemoveViewListPlayer(prevPlayerId);//remove하라고 명령
			}
			if(g_clients[prevPlayerId]->IsExistViewList(objectId)){//=>현재는 존재하지 않음
				g_clients[prevPlayerId]->RemoveViewListPlayer(objectId);//remove하라고 명령
			}
		}
	}
}

bool Logic::ViewInRange(int from, int to){
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if(abs(player1.first - player2.first) > VIEW_RANGE)
		return false;
	if(abs(player1.second - player2.second) > VIEW_RANGE)
		return false;
	return true;
}

bool Logic::ViewInRange(pair<short, short> & fromPosition, int to){
	auto player2 = g_clients[to]->GetPosition();
	if(abs(fromPosition.first - player2.first) > VIEW_RANGE)
		return false;
	if(abs(fromPosition.second - player2.second) > VIEW_RANGE)
		return false;
	return true;
}

bool Logic::AttackInRange(pair<short, short> & fromPosition, pair<short, short> & toPosition){
	if((int)abs(fromPosition.first - toPosition.first) > Attack_RANGE)
		return false;
	if((int)abs(fromPosition.second - toPosition.second) > Attack_RANGE)
		return false;
	return true;
}

bool Logic::AttackInRange(int from, int to){
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if((int)abs(player1.first - player2.first) > Attack_RANGE)
		return false;
	if((int)abs(player1.second - player2.second) > Attack_RANGE)
		return false;
	return true;
}

bool Logic::NPC_AttackInRange(pair<short, short> & fromPosition, pair<short, short> & toPosition){
	if(fromPosition.first == toPosition.first && fromPosition.second == toPosition.second)
		return true;
	return false;
}

bool Logic::NPC_AttackInRange(int from, int to){
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if(player1.first == player2.first && player1.second == player2.second)
		return true;
	return false;
}

bool Logic::NPC_AgroInRange(int from, int to){
	auto player1 = g_clients[from]->GetPosition();
	auto player2 = g_clients[to]->GetPosition();
	if((int)abs(player1.first - player2.first) > AGRO_RANGE)
		return false;
	if((int)abs(player1.second - player2.second) > AGRO_RANGE)
		return false;
	return true;
}

bool Logic::NPC_AgroInRange(pair<short, short> & fromPosition, pair<short, short> & toPosition){
	if((int)abs(fromPosition.first - toPosition.first) > AGRO_RANGE)
		return false;
	if((int)abs(fromPosition.second - toPosition.second) > AGRO_RANGE)
		return false;
	return true;
}

pair<short, short> Logic::PlayerPositionToMapSession(pair<short, short> playerPosition){
	return { playerPosition.first / 20 , playerPosition.second / 20 };
}

pair<short, short> Logic::PlayerPositionToMapSession(short x, short y){
	return { x / 20 , y / 20 };
}

void Logic::InsertObjectIdMapSession(int objId, pair<short, short> mapIdx){
	g_gameMap[mapIdx.first][mapIdx.second].InsertPlayer(objId);
}

void Logic::PlayerAttackExecute(int playerId){
	if(!g_clients[playerId]->IsAbleAttack()) return;
	PlayerObject * playerObject = dynamic_cast<PlayerObject *>( g_clients[playerId] );
	auto viewList = g_clients[playerId]->GetViewList();
	playerObject->ResetLastAttack();
	playerObject->SendSkillExecutePacket(viewList);
	for(auto & id : viewList){
		if(!IsPlayer(id)){
			Attack(playerId, id);
		}
	}
	if(g_clients[playerId]->IsAbleLevelUp()){
		short level = g_clients[playerId]->GetLevel();
		if(level == g_levelExp.size()) return;
		short restExp = g_clients[playerId]->GetExp() - g_levelExp[level - 1];
		g_clients[playerId]->SetExp(restExp);
		g_clients[playerId]->SetMaxExp(g_levelExp[level]);
		g_clients[playerId]->LevelUp();
		g_clients[playerId]->SetHp(g_levelMaxHp[level]);
		g_clients[playerId]->SetMaxHp(g_levelMaxHp[level]);
		g_clients[playerId]->SetAttackDamage(g_levelAttackDamage[level]);
		playerObject->SaveData();
	}
	PacketManager::SendStatPacketSelf(playerId);
}

void Logic::Attack(int attackObjId, int attackedObjId){
	if(IsPlayer(attackedObjId))return;
	if(!AttackInRange(attackObjId, attackedObjId))return;
	short getExp = g_clients[attackedObjId]->AttackedDamage(attackObjId, g_clients[attackObjId]->GetAttackDamage());
	auto viewList = g_clients[attackedObjId]->GetViewList();
	PacketManager::SendStatPacketInViewList(viewList, attackedObjId);
	if(getExp != 0){
		dynamic_cast<PlayerObject *>( g_clients[attackObjId] )->ConsumeExp(getExp);
		//die npc info send
		g_clients[attackedObjId]->ClearViewList();
		InsertRespawnNPC(attackedObjId);
	}
}

void Logic::NPC_Attack(int attackObjId, int attackedObjId){
	if(!IsPlayer(attackedObjId))return;
	if(g_clients[attackObjId]->GetPlayerState() != ST_INGAME)return;
	if(!g_clients[attackObjId]->IsAbleAttack())return;
	if(!NPC_AttackInRange(attackObjId, attackedObjId))return;
	g_clients[attackedObjId]->AttackedDamage(attackObjId, g_clients[attackObjId]->GetAttackDamage());
	auto viewList = g_clients[attackedObjId]->GetViewList();
	PacketManager::SendStatPacketInViewList(viewList, attackedObjId);
	g_clients[attackObjId]->ResetLastAttack();
}

void Logic::InsertRespawnNPC(int npcId){
	g_Timer.InsertTimerQueue(EV_RESPAWN_NPC, npcId, -1, 1min);
}

void Logic::RespawnNPC(int npcId){
	NPC_Object * npc = dynamic_cast<NPC_Object *>( g_clients[npcId] );
	if(npc->RespawnNpc()){
		npc->RespawnData();
		unordered_set<int> nullSet;
		unordered_set<int> newViewList = Logic::NPC_UpdateNearList(npcId);
		ProccessViewList(npcId, nullSet, newViewList);
	}
}

float Logic::GetDistance(pair<short, short> & p1, pair<short, short> & p2){
	//return sqrt(pow((float)(p1.first - p2.first), 2) + pow((float)(p1.second - p2.second), 2));
	return pow(( p1.first - p2.first ), 2) + pow(( p1.second - p2.second ), 2);
	//return abs(p1.first - p2.first) + abs(p1.second - p2.second);
}

float Logic::GetDistance(pair<short, short> & p1, short x, short y){
	//return sqrt(pow((float)(p1.first - x), 2) + pow((float)(p1.second - y), 2));
	return pow(( p1.first - x ), 2) + pow(( p1.second - y ), 2);
	//return abs(p1.first - x) + abs(p1.second - y);
}

void Logic::InsertOpenList(const std::unordered_map<int, AstarNode> & closeList, std::unordered_map<int, AstarNode> & openList, pair<short, short> & targetNode, pair<short, short> & parentNode, short nextNodeX, short nextNodeY){
	pair<short, short> nextNode = make_pair(nextNodeX, nextNodeY);
	bool checkInMap = CheckInMap(nextNode);
	if(!checkInMap)
		return;

	auto nextgameMapIdx = Logic::PlayerPositionToMapSession(nextNode);
	bool isCollideMap = g_gameMap[nextgameMapIdx.first][nextgameMapIdx.second].CollisionObject(nextNode);
	if(isCollideMap)
		return;

	float distance = GetDistance(targetNode, nextNode);

	if(closeList.count(nextNodeY * 20 + nextNodeX)) return;
	if(openList.count(nextNodeY * 20 + nextNodeX)){
		openList[nextNodeY * 20 + nextNodeX].ModifyDistance(distance, parentNode);
	} else openList.try_emplace(nextNodeY * 20 + nextNodeX, nextNode, parentNode, distance);
}

void Logic::SendMess(int from, int to, wchar_t * mess){
	if(IsPlayer(to))
		dynamic_cast<PlayerObject *>( g_clients[to] )->SendMess(from, mess);
}

void Logic::BroadCastMessInViewList(int playerId, wchar_t * mess){
	auto viewList = g_clients[playerId]->GetViewList();
	for(const auto & id : viewList){
		SendMess(playerId, id, mess);
	}
}

std::list<pair<short, short>> Logic::GetAstarList(int npcId, int targetId){
	auto npcPosition = g_clients[npcId]->GetPosition();
	auto targetPosition = g_clients[targetId]->GetPosition();
	if(!Logic::NPC_AgroInRange(npcPosition, targetPosition))return std::list<pair<short, short>>{};

	std::unordered_map<int, AstarNode> openListMap;
	std::unordered_map<int, AstarNode> closeListMap;

	closeListMap.try_emplace(npcPosition.second * 20 + npcPosition.first, npcPosition, npcPosition, 0);
	auto currentNode = npcPosition;

	while(true){
		//auto currentgameMapIdx = Logic::PlayerPositionToMapSession(currentNode);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first - 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first + 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second - 1);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second + 1);

		float minDistace = _FMAX;
		int minIdx = -1;
		for(auto & node : openListMap){
			if(minDistace > node.second.GetDistance()){
				minDistace = node.second.GetDistance();
				minIdx = node.first;
			}
		}
		///////////////////////
		if(-1 == minIdx){
			return std::list<pair<short, short>>{};
		}
		if(-1 != minIdx){
			currentNode = openListMap[minIdx].GetNodePos();
			closeListMap.try_emplace(minIdx, openListMap[minIdx]);
			openListMap.erase(minIdx);
		}

		if(currentNode == targetPosition){
			std::list<pair<short, short>> resList;
			int resIdx = minIdx;
			while(true){
				AstarNode & currentNode = closeListMap[resIdx];
				resList.emplace_front(currentNode.GetNodePos());
				if(currentNode.GetNodePos() == npcPosition){
					return resList;
				}
				resIdx = currentNode.GetParentNodePos().first + currentNode.GetParentNodePos().second * 20;
			}
		}
	}
}

std::list<pair<short, short>> Logic::GetAstarList(int npcId, pair<short, short> & targetPos){
	auto npcPosition = g_clients[npcId]->GetPosition();
	auto targetPosition = targetPos;
	if(!Logic::NPC_AgroInRange(npcPosition, targetPosition))return std::list<pair<short, short>>{};
	if(targetPosition == npcPosition)
		return std::list<pair<short, short>>{targetPosition};
	std::unordered_map<int, AstarNode> openListMap;
	std::unordered_map<int, AstarNode> closeListMap;

	closeListMap.try_emplace(npcPosition.second * 20 + npcPosition.first, npcPosition, npcPosition, 0);
	auto currentNode = npcPosition;

	while(true){
		//auto currentgameMapIdx = Logic::PlayerPositionToMapSession(currentNode);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first - 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first + 1, currentNode.second);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second - 1);
		Logic::InsertOpenList(closeListMap, openListMap, targetPosition, currentNode, currentNode.first, currentNode.second + 1);

		float minDistace = _FMAX;
		int minIdx = -1;
		for(auto & node : openListMap){
			if(minDistace > node.second.GetDistance()){
				minDistace = node.second.GetDistance();
				minIdx = node.first;
			}
		}
		///////////////////////
		if(-1 == minIdx){
			return std::list<pair<short, short>>{};
		}
		if(-1 != minIdx){
			currentNode = openListMap[minIdx].GetNodePos();
			closeListMap.try_emplace(minIdx, openListMap[minIdx]);
			openListMap.erase(minIdx);
		}

		if(currentNode == targetPosition){
			std::list<pair<short, short>> resList;
			int resIdx = minIdx;
			while(true){
				AstarNode & resCurrentNode = closeListMap[resIdx];
				if(resCurrentNode.GetNodePos() == npcPosition){
					return resList;
				}
				resList.emplace_front(resCurrentNode.GetNodePos());
				resIdx = resCurrentNode.GetParentNodePos().first + resCurrentNode.GetParentNodePos().second * 20;
			}
		}
	}
}

void Logic::AutoSaveAllPlayers(){
	for(int i = 0; i < MAX_USER; i++){
		if(g_clients[i]->GetName().find(L"Test") != wstring::npos)continue;
		if(ST_INGAME != g_clients[i]->GetPlayerState())continue;
		PlayerObject * pObject = dynamic_cast<PlayerObject *>(g_clients[i]);
		pObject->SaveData();
	}
	TIMER_EVENT ev{ EV_AUTO_SAVE, -1, -1, system_clock::now() + 5min };
	g_Timer.InsertTimerQueue(ev);
}

