#pragma once
#include <array>
#include <atomic>
#include <list>
#include <utility>

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
	lua_State* myLuaState = nullptr;
	int chaseId = -1;
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
	void AStarLoad(int DestinyId, int npcId, std::list<AStarNode>& res);
	int GetChaseId() { return chaseId; }
	void SetChaseId(int cId) { chaseId = cId; }
};
