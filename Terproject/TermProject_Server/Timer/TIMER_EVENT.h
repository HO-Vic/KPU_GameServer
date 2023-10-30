#pragma once
#include "../PCH/stdafx.h"

using namespace std;
using namespace chrono;


class TIMER_EVENT
{
private:
	chrono::system_clock::time_point m_wakeUpTime;
	EVENT_TYPE m_eventType = EV_NONE;

	int m_ownerId;
	int m_targetId;

public:
	TIMER_EVENT();
	TIMER_EVENT(EVENT_TYPE evType, int ownerId, int target, chrono::system_clock::time_point wakeupTime);
	TIMER_EVENT(const TIMER_EVENT& other);
	TIMER_EVENT(TIMER_EVENT&& other);
public:
	constexpr bool operator < (const TIMER_EVENT& L) const
	{
		return (m_wakeUpTime > L.m_wakeUpTime);
	}

	TIMER_EVENT& operator= (const TIMER_EVENT& other);

public:
	bool IsWakeUpTime(chrono::system_clock::time_point& currentTime);
	const EVENT_TYPE& GetEventType();

	int GetOwner();
	int GetTarget();
	pair<int, int> GetIds();
};
