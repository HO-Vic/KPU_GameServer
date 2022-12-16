#include "LUA_OBJECT.h"
#include "SESSION.h"
#include <math.h>
#include <list>

#include <vector>
//lua func
int API_get_x(lua_State* L);
int API_get_y(lua_State* L);

//AStart
extern pair<int, int> AStartObstacle[31];
bool CollideObstacle(int inX, int inY);
float LocalDistance(int destinyX, int destinyY, int startX, int startY);
struct AStarNode {
	float fScore;
	float gScore;
	float hScore;
	pair<int, int> myNode;
	pair<int, int> parentNode;
	constexpr bool operator < (const AStarNode& L) const
	{
		return (fScore < L.fScore);
	}
	constexpr bool operator== (const AStarNode& L) const
	{
		return myNode == L.myNode;
	}
};

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

bool LUA_OBJECT::ActiveNPC()
{
	bool old_state = false;
	if (false == atomic_compare_exchange_strong(&isActive, &old_state, true))
		return true;
	return false;
}

bool LUA_OBJECT::InActiveNPC()
{
	bool old_state = true;
	if (false == atomic_compare_exchange_strong(&isActive, &old_state, false))
		return true;
	return false;
}

void LUA_OBJECT::AStart(int DestinyId, int npcId)
{
	std::list<AStarNode> open;
	std::list<AStarNode> close;
	int startX = clients[npcId].x;
	int startY = clients[npcId].y;
	close.push_back(AStarNode{ 0,0,0,make_pair(startX, startY), make_pair(startX, startY) });

	int destinyX = clients[DestinyId].x;
	int destinyY = clients[DestinyId].y;

	pair<int, int> currentNode = make_pair(destinyX, destinyY);
	while (true) {
		/*std::vector<pair<int, int>> nextStep;
		if(!CollideObstacle(startX-1, startY))
			nextStep.push_back(make_pair(startX - 1, startY));
		if (!CollideObstacle(startX + 1, startY))
			nextStep.push_back(make_pair(startX + 1, startY));
		if (!CollideObstacle(startX, startY - 1))
			nextStep.push_back(make_pair(startX, startY - 1));
		if (!CollideObstacle(startX, startY + 1))
			nextStep.push_back(make_pair(startX, startY + 1));*/

		if (!CollideObstacle(startX - 1, startY)) {
			AStarNode node;
			node.hScore = LocalDistance(currentNode.first, currentNode.second, startX - 1, startY);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(startX - 1, startY);
			node.parentNode = make_pair(startX, startY);

			auto findRetVal = find(open.begin(), open.end(), node);
			if (findRetVal != open.end()) {
				(*findRetVal) = node;
			}
			else {
				auto openNode = open.begin();
				for (; openNode != open.end(); openNode++) {
					if (openNode->fScore > node.fScore)
						break;
				}
				open.insert(openNode, node);
			}
		}
		if (!CollideObstacle(startX + 1, startY)) {
			AStarNode node;
			node.hScore = LocalDistance(currentNode.first, currentNode.second, startX + 1, startY);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(startX + 1, startY);
			node.parentNode = make_pair(startX, startY);
			auto findRetVal = find(open.begin(), open.end(), node);
			if (findRetVal != open.end()) {
				(*findRetVal) = node;
			}
			else {
				auto openNode = open.begin();
				for (; openNode != open.end(); openNode++) {
					if (openNode->fScore > node.fScore)
						break;
				}
				open.insert(openNode, node);
			}
		}
		if (!CollideObstacle(startX, startY - 1)) {
			AStarNode node;
			node.hScore = LocalDistance(currentNode.first, currentNode.second, startX, startY - 1);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(startX, startY - 1);
			node.parentNode = make_pair(startX, startY);
			auto findRetVal = find(open.begin(), open.end(), node);
			if (findRetVal != open.end()) {
				(*findRetVal) = node;
			}
			else {
				auto openNode = open.begin();
				for (; openNode != open.end(); openNode++) {
					if (openNode->fScore > node.fScore)
						break;
				}
				open.insert(openNode, node);
			}
		}
		if (!CollideObstacle(startX, startY + 1)) {
			AStarNode node;
			node.hScore = LocalDistance(currentNode.first, currentNode.second, startX, startY + 1);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(startX, startY + 1);
			node.parentNode = make_pair(startX, startY);
			auto findRetVal = find(open.begin(), open.end(), node);
			if (findRetVal != open.end()) {
				(*findRetVal) = node;
			}
			else {
				auto openNode = open.begin();
				for (; openNode != open.end(); openNode++) {
					if (openNode->fScore > node.fScore)
						break;
				}
				open.insert(openNode, node);
			}
		}

		AStarNode GetNode = *open.begin();
		open.erase(open.begin());
		close.push_back(GetNode);
		currentNode = GetNode.myNode;
		if (GetNode.myNode.first == destinyX && GetNode.myNode.second == destinyY)
			return;
	}
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

bool CollideObstacle(int inX, int inY)
{
	for (int i = 0; i < 31; i++)
		if (inX == AStartObstacle[i].first && inY == AStartObstacle[i].second)
			return true;
	return false;
}

float LocalDistance(int destinyX, int destinyY, int startX, int startY)
{
	return floorf(
		(destinyX - startX) ^ 2 +
		(destinyY - startY) ^ 2
	);
}

