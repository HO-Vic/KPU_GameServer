#pragma once
#include <array>
#include <atomic>
#include <list>
#include <utility>
#include <mutex>
extern "C"
{
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}
#pragma comment(lib, "lua54.lib")

enum NPC_TYPE {
	PEACE,
	AGRO
};
class SESSION;
struct AStarNode {
	float fScore;
	float gScore;
	float hScore;
	std::pair<int, int> myNode;
	std::pair<int, int> parentNode;
	constexpr bool operator < (const AStarNode& L) const
	{
		return (fScore < L.fScore);
	}
};
class LUA_OBJECT
{
private:
	std::atomic_bool isActive = false;
	std::atomic_bool isChase = false;
	std::atomic_bool isArrive = true;
	lua_State* myLuaState = nullptr;
	int chaseId = -1;
	//std::list<AStarNode> npcNavigateList;
	std::mutex GetNodeLock;
public:
	NPC_TYPE type = AGRO;
	LUA_OBJECT() {}
	LUA_OBJECT(int id, NPC_TYPE t);

	LUA_OBJECT(int id, const char* luaName);

	~LUA_OBJECT() {}

	bool ActiveNPC();
	bool InActiveNPC();
	bool ActiveChase();
	bool InActiveChase();
	bool DieNpc()
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
	bool GetArrive() { return isArrive; }
	std::pair<int, int> AStarLoad(int StartX, int startY, int destinyX, int destinyY);
	std::pair<int, int> GetNextNode() 
	{
		/*GetNodeLock.lock();
		if (!npcNavigateList.empty()) {
			std::pair<int, int> retVal = npcNavigateList.begin()->myNode;		
			if(!npcNavigateList.empty())
				npcNavigateList.pop_front();
			GetNodeLock.unlock();
			return retVal;
		}
		GetNodeLock.unlock();
		return std::make_pair(-100, -100);*/
	}
	int GetChaseId() { return chaseId; }
	void SetChaseId(int cId) { chaseId = cId; }
};
