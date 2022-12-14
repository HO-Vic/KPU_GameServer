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
	lua_State* myLuaState = nullptr;
public:
	LUA_OBJECT() {}
	LUA_OBJECT(int id) {
		myLuaState = luaL_newstate();
		luaL_openlibs(myLuaState);
	/*	luaL_loadfile(myLuaState, "npc.lua");
		lua_pcall(myLuaState, 0, 0, 0);

		lua_getglobal(myLuaState, "set_uid");
		lua_pushnumber(myLuaState, id);
		lua_pcall(myLuaState, 1, 0, 0);*/
		// lua_pop(L, 1);// eliminate set_uid from stack after call

		/*lua_register(clients[i]._L, "SendHelloMessage", API_helloSendMessage);
		lua_register(clients[i]._L, "SendByeMessage", API_ByeSendMessage);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);*/
	}
	~LUA_OBJECT() {}
};

