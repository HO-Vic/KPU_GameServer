#include "stdafx.h"
#include "IocpNetwork/IocpNetwork.h"
#include "MapSession/MapSession.h"
#include "GameObject/GameObject.h"
#include "Logic/Logic.h"
#include "Timer/Timer.h"
#include "DB/DB_OBJ.h"

using namespace std;



array<int, 11> g_levelExp = { 0, 100, 200, 300, 400 ,500, 600, 700, 800, 900, 1200 };
std::array<int, 11> g_levelMaxHp = { 100, 200, 500, 600 ,700, 900, 1000, 1100, 1200, 1500, 1700 };
std::array<int, 11> g_levelAttackDamage = { 50, 70, 100, 120 ,150, 200, 220, 250, 300, 330, 400 };


array<GameObject*, MAX_USER + MAX_NPC> g_clients;
array < array<MapSession, 100>, 100> g_gameMap;
std::array<std::pair<short, short>, 31> g_mapObstacle;


random_device g_rd;
default_random_engine g_dre(g_rd());
uniform_int_distribution<int> g_npcRandDir(0, 3); // inclusive
uniform_int_distribution<int> g_npcRandPostion(0, 1999); // inclusive

IocpNetwork g_iocpNetwork;
Timer g_Timer;
DB_OBJ g_DB;

int main()
{
	locale("Korean");
	Logic::InitAstarLoad();
	Logic::InitGameMap();
	Logic::InitGameObjects();
	Logic::InitNPC();
	g_iocpNetwork.Start();
}

//1. PlayerData저장 - DB Save Player Info
//2. Client쪽 Skill CoolTime 조정
//3. Timer - 5min Save All Palyer Info
//4. Chat 추가
