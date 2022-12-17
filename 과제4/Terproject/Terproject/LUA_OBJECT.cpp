#include "LUA_OBJECT.h"
#include "SESSION.h"
#include <math.h>

#include <vector>
//lua func
int API_get_x(lua_State* L);
int API_get_y(lua_State* L);

//AStart
extern pair<int, int> AStartObstacle[31];
bool CollideObstacle(int inX, int inY);
float LocalDistance(int destinyX, int destinyY, int startX, int startY);


LUA_OBJECT::LUA_OBJECT(int id, NPC_TYPE t)
{
	npcNavigateList.clear();
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
	npcNavigateList.clear();
	type = NPC_TYPE::AGRO;
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

bool LUA_OBJECT::ActiveChase()
{
	bool old_state = false;
	if (false == atomic_compare_exchange_strong(&isActive, &old_state, true))
		return true;
	return false;
}

bool LUA_OBJECT::InActiveChase()
{
	chaseId = -1;
	bool old_state = true;
	if (false == atomic_compare_exchange_strong(&isActive, &old_state, false))
		return true;
	return false;
}

pair<int, int> LUA_OBJECT::AStarLoad(int DestinyId, int npcId)
{
	std::list<AStarNode> open;
	std::list<AStarNode> close;
	int startX = clients[npcId].x;
	int startY = clients[npcId].y;
	close.push_back(AStarNode{ 0,0,0,make_pair(startX, startY), make_pair(startX, startY) });

	int destinyX = clients[DestinyId].x;
	int destinyY = clients[DestinyId].y;
	pair<int, int> currentNode = make_pair(startX, startY);

	while (true) {
		if (!CollideObstacle(currentNode.first - 1, currentNode.second)) {
			AStarNode node;
			node.hScore = LocalDistance(destinyX, destinyY, currentNode.first - 1, currentNode.second);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(currentNode.first - 1, currentNode.second);
			node.parentNode = make_pair(currentNode.first, currentNode.second);

			auto findRetVal = find_if(open.begin(), open.end(), [=](const AStarNode findNode) {
				return findNode.myNode == node.myNode;
				});
			if (findRetVal != open.end()) {
				if (findRetVal->fScore > node.fScore)
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
		if (!CollideObstacle(currentNode.first + 1, currentNode.second)) {
			AStarNode node;
			node.hScore = LocalDistance(currentNode.first + 1, currentNode.second, destinyX, destinyY);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(currentNode.first + 1, currentNode.second);
			node.parentNode = make_pair(currentNode.first, currentNode.second);
			auto findRetVal = find_if(open.begin(), open.end(), [=](const AStarNode findNode) {
				return findNode.myNode == node.myNode;
				});
			if (findRetVal != open.end()) {
				if (findRetVal->fScore > node.fScore)
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
		if (!CollideObstacle(currentNode.first, currentNode.second - 1)) {
			AStarNode node;
			node.hScore = LocalDistance(currentNode.first, currentNode.second - 1, destinyX, destinyY);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(currentNode.first, currentNode.second - 1);
			node.parentNode = make_pair(currentNode.first, currentNode.second);
			auto findRetVal = find_if(open.begin(), open.end(), [=](const AStarNode findNode) {
				return findNode.myNode == node.myNode;
				});
			if (findRetVal != open.end()) {
				if (findRetVal->fScore > node.fScore)
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
		if (!CollideObstacle(currentNode.first, currentNode.second + 1)) {
			AStarNode node;
			node.hScore = LocalDistance(currentNode.first, currentNode.second + 1, destinyX, destinyY);
			node.gScore = 1;
			node.fScore = node.hScore + node.gScore;
			node.myNode = make_pair(currentNode.first, currentNode.second + 1);
			node.parentNode = make_pair(currentNode.first, currentNode.second);
			auto findRetVal = find_if(open.begin(), open.end(), [=](const AStarNode findNode) {
				return findNode.myNode == node.myNode;
				});
			if (findRetVal != open.end()) {
				if (findRetVal->fScore > node.fScore)
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

		if (GetNode.myNode.first == destinyX && GetNode.myNode.second == destinyY) {
			npcNavigateList.push_back(GetNode);
			while (true) {
				auto findIter = find_if(close.begin(), close.end(), [=](AStarNode findNode) {
					return findNode.myNode == GetNode.parentNode;
					});
				if (findIter == close.end()) {
					open.clear();
					close.clear();
					return GetNode.myNode;
				}
				if (findIter->myNode == findIter->parentNode) {
					pair<int, int> retVal = npcNavigateList.rbegin()->myNode;
					npcNavigateList.pop_back();
					open.clear();
					close.clear();
					return retVal;
				}
				npcNavigateList.push_back(*findIter);
				GetNode = *findIter;
			}
		}
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
		if (inX % 20 == AStartObstacle[i].first && inY % 20 == AStartObstacle[i].second)
			return true;
	return false;
}

float LocalDistance(int destinyX, int destinyY, int startX, int startY)
{
	return sqrt(
		pow( (float)(destinyX - startX), 2 ) + pow( (float)(destinyY - startY), 2 ) 
	);
}

