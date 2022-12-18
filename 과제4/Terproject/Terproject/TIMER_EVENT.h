#pragma once
#include <chrono>

using namespace std;
using namespace chrono;

enum EVENT_TYPE { EV_NONE, EV_RANDOM_MOVE, EV_CHASE_MOVE, EV_PLAYER_ATTACK_COOL };

struct TIMER_EVENT
{
	int objId;
	chrono::system_clock::time_point wakeupTime;
	EVENT_TYPE eventId = EV_NONE;
	int targetId;
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (wakeupTime > L.wakeupTime);
	}
};
