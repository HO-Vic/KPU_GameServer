#pragma once
extern "C"
{
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}
#pragma comment(lib, "lua54.lib")

class LUA_OBJECT
{
private:
	lua_State* myLuaState;
public:
	LUA_OBJECT() {
		myLuaState = lua_newstate();
	}
	~LUA_OBJECT() {}
};

