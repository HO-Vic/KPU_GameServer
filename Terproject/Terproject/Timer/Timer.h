#pragma once
#include "../PCH/stdafx.h"
#include "TIMER_EVENT.h"
class Timer
{
private:
	atomic_bool m_isRunning = false;
	std::thread m_TimerThread;
private:
	std::priority_queue <TIMER_EVENT> m_priorityTimerQueue;
	std::mutex m_timerQueueLock;
public:
	Timer();
	~Timer();
private:
	void TimerThreadFunc();
public:
	void InsertTimerQueue(TIMER_EVENT& ev);
	void InsertTimerQueue(EVENT_TYPE type, int ownerId, int targetId, std::chrono::milliseconds afterTime);
};
