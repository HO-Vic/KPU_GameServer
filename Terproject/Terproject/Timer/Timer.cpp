#include "stdafx.h"
#include "Timer.h"
#include "../ExpOver/ExpOver.h"
#include "../IocpNetwork/IocpNetwork.h"

extern IocpNetwork g_iocpNetwork;

Timer::Timer() : m_isRunning(true)
{
	m_TimerThread = std::thread([&]() {TimerThreadFunc(); });
	TIMER_EVENT ev{ EV_AUTO_SAVE, -1, -1, system_clock::now() + 5min };
	InsertTimerQueue(ev);
}

Timer::~Timer()
{
	m_isRunning = false;
	m_TimerThread.join();
}

void Timer::TimerThreadFunc()
{
	while (m_isRunning) {
		TIMER_EVENT ev;

		m_timerQueueLock.lock();
		if (!m_priorityTimerQueue.empty()) {
			ev = m_priorityTimerQueue.top();
		}
		else {
			m_timerQueueLock.unlock();
			this_thread::yield();
			continue;
		}
		m_timerQueueLock.unlock();

		auto currentTime = chrono::system_clock::now();
		if (!ev.IsWakeUpTime(currentTime)) {
			this_thread::yield();
			continue;
		}
		else
		{
			m_timerQueueLock.lock();
			m_priorityTimerQueue.pop();
			m_timerQueueLock.unlock();
		}

		switch (ev.GetEventType())
		{
		case EV_RANDOM_MOVE:
		{
			ExpOver* ov = ExpOverMgr::CreateExpOver(OP_NPC_MOVE);
			PostQueuedCompletionStatus(g_iocpNetwork.GetIocpHandle(), 1, ev.GetOwner(), reinterpret_cast<WSAOVERLAPPED*>(ov));
		}
		break;
		case EV_CHASE_MOVE:
		{
			int targetId = ev.GetTarget();
			ExpOver* ov = ExpOverMgr::CreateExpOverBuffer(OP_NPC_CHASE_MOVE, reinterpret_cast<char*>(&targetId), sizeof(int));
			PostQueuedCompletionStatus(g_iocpNetwork.GetIocpHandle(), 1, ev.GetOwner(), reinterpret_cast<WSAOVERLAPPED*>(ov));
		}
		break;
		case EV_RESPAWN_NPC:
		{			
			ExpOver* ov = ExpOverMgr::CreateExpOver(OP_NPC_RESPAWN);
			PostQueuedCompletionStatus(g_iocpNetwork.GetIocpHandle(), 1, ev.GetOwner(), reinterpret_cast<WSAOVERLAPPED*>(ov));
		}
		break;
		case EV_AUTO_SAVE:
		{
			ExpOver* ov = ExpOverMgr::CreateExpOver(OP_DB_AUTO_SAVE_PLAYER);
			PostQueuedCompletionStatus(g_iocpNetwork.GetIocpHandle(), -1, -1, reinterpret_cast<WSAOVERLAPPED*>(ov));
		}
		break;	
		default:
			break;
		}
	}
}

void Timer::InsertTimerQueue(TIMER_EVENT& ev)
{
	m_timerQueueLock.lock();
	m_priorityTimerQueue.push(ev);
	m_timerQueueLock.unlock();
}

void Timer::InsertTimerQueue(EVENT_TYPE type, int ownerId, int targetId, std::chrono::milliseconds afterTime)
{
	TIMER_EVENT ev{ type, ownerId, targetId, system_clock::now() + afterTime };
	m_timerQueueLock.lock();
	m_priorityTimerQueue.push(ev);
	m_timerQueueLock.unlock();
}
