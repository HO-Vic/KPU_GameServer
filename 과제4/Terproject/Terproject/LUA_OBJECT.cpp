#include "LUA_OBJECT.h"
#include "SESSION.h"

//lua func
int API_get_x(lua_State* L);
int API_get_y(lua_State* L);

LUA_OBJECT::LUA_OBJECT(int id, NPC_TYPE t)
{
	type = t;
	myLuaState = luaL_newstate();
	luaL_openlibs(myLuaState);
	luaL_loadfile(myLuaState, "lua_script\npc.lua");
	lua_pcall(myLuaState, 0, 0, 0);

	lua_getglobal(myLuaState, "set_uid");
	lua_pushnumber(myLuaState, id);
	lua_pcall(myLuaState, 1, 0, 0);
	lua_pop(myLuaState, 1);// eliminate set_uid from stack after call
	lua_settop(myLuaState, 0);

	lua_register(myLuaState, "API_get_x", API_get_x);
	lua_register(myLuaState, "API_get_y", API_get_y);

}

LUA_OBJECT::LUA_OBJECT(int id, const char* luaName)
{
	myLuaState = luaL_newstate();
	luaL_openlibs(myLuaState);
	luaL_loadfile(myLuaState, luaName);
	lua_pcall(myLuaState, 0, 0, 0);

	lua_getglobal(myLuaState, "set_uid");
	lua_pushnumber(myLuaState, id);
	lua_pcall(myLuaState, 1, 0, 0);
	lua_pop(myLuaState, 1);// eliminate set_uid from stack after call
	lua_settop(myLuaState, 0);

	lua_register(myLuaState, "API_get_x", API_get_x);
	lua_register(myLuaState, "API_get_y", API_get_y);
}

int API_get_x(lua_State* L)
{
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int x = clients[user_id].x;
	lua_pushnumber(L, x);
	return 1;
}

int API_get_y(lua_State* L)
{
	int user_id = (int)lua_tointeger(L, -1);
	lua_pop(L, 2);
	int y = clients[user_id].y;
	lua_pushnumber(L, y);
	return 1;
}