#pragma once
#include <iostream>

#include <WS2tcpip.h>
#include <MSWSock.h>

#include <thread>

#include <random>
#include <chrono>

#include <mutex>
#include <atomic>

#include <utility>
#include <algorithm>

#include <vector>
#include <array>
#include <unordered_set>
#include <list>
#include <queue>
#include <concurrent_priority_queue.h>
#include <concurrent_queue.h>
#include <string>
#include <map>

#include <math.h>

#include<sqlext.h>


extern "C"
{
#include "../include\lua.h"
#include "../include\lauxlib.h"
#include "../include\lualib.h"
}
#pragma comment(lib, "lua54.lib")


#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 255;
constexpr int NAME_SIZE = 20;
constexpr int CHAT_SIZE = 100;

constexpr int MAX_USER = 11000;
constexpr int MAX_NPC = 200000;
//constexpr int MAX_NPC = 1;

constexpr int W_WIDTH = 2000;
constexpr int W_HEIGHT = 2000;

enum S_STATE {
	ST_PLAYER_FREE,
	ST_PLAYER_ALLOC,
	ST_INGAME	
};
enum OP_CODE {
	OP_ACCEPT,
	OP_RECV,
	OP_SEND,
	OP_DB_GET_PLAYER_INFO,
	OP_NPC_MOVE,
	OP_NPC_CHASE_MOVE,
	OP_NPC_RESPAWN,
	OP_DB_AUTO_SAVE_PLAYER,
	OP_PLAYER_LOGIN_FAIL
};

enum EVENT_TYPE {
	EV_NONE,
	EV_RANDOM_MOVE,
	EV_CHASE_MOVE,
	EV_PLAYER_ATTACK_COOL,
	EV_RESPAWN_NPC,
	EV_AUTO_SAVE
};
enum DB_EVENT_TYPE {
	EV_GET_PLAYER_INFO,
	EV_ADD_NEW_USER,
	EV_SAVE_PLAYER_INFO
};

enum NPC_TYPE {
	PEACE,
	AGRO,
	BOSS
};
constexpr int VIEW_RANGE = 8;
constexpr int AGRO_RANGE = 6;
constexpr int Attack_RANGE = 1;
constexpr int NPC_Attack_RANGE = 0;
