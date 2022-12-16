#pragma once
#include <array>
#include <atomic>
#include "protocol_2022.h"
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
class LUA_OBJECT
{
private:	
	std::atomic_bool isActive = false;
	lua_State* myLuaState = nullptr;
public:
	NPC_TYPE type = AGRO;
	LUA_OBJECT() {}
	LUA_OBJECT(int id, NPC_TYPE t);

	LUA_OBJECT(int id, const char* luaName);

	~LUA_OBJECT() {}

	bool ActiveNPC();
	bool InActiveNPC();
};
