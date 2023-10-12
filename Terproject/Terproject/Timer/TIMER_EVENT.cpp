#include "stdafx.h"
#include "TIMER_EVENT.h"

TIMER_EVENT::TIMER_EVENT() :m_eventType(EV_NONE), m_ownerId(-1), m_targetId(-1)
{
}

TIMER_EVENT::TIMER_EVENT(EVENT_TYPE evType, int ownerId, int target, chrono::system_clock::time_point wakeupTime)
	:m_eventType(evType), m_ownerId(ownerId), m_targetId(target), m_wakeUpTime(wakeupTime)
{
}

TIMER_EVENT::TIMER_EVENT(const TIMER_EVENT& other)
	:m_eventType(other.m_eventType), m_ownerId(other.m_ownerId), m_targetId(other.m_targetId), m_wakeUpTime(other.m_wakeUpTime)
{
}

TIMER_EVENT::TIMER_EVENT(TIMER_EVENT&& other)
	:m_eventType(other.m_eventType), m_ownerId(other.m_ownerId), m_targetId(other.m_targetId), m_wakeUpTime(other.m_wakeUpTime)
{
}

bool TIMER_EVENT::IsWakeUpTime(chrono::system_clock::time_point& currentTime)
{
	return m_wakeUpTime <= currentTime;	
}

const EVENT_TYPE& TIMER_EVENT::GetEventType()
{
	// TODO: 여기에 return 문을 삽입합니다.
	return m_eventType;
}

int TIMER_EVENT::GetOwner()
{
	return m_ownerId;
}

int TIMER_EVENT::GetTarget()
{
	return m_targetId;
}

pair<int, int> TIMER_EVENT::GetIds()
{
	return { m_ownerId , m_targetId };
}

TIMER_EVENT& TIMER_EVENT::operator= (const TIMER_EVENT& other)
{
	// TODO: 여기에 return 문을 삽입합니다.
	m_eventType = other.m_eventType;
	m_ownerId = other.m_ownerId;
	m_targetId = other.m_targetId;
	m_wakeUpTime = other.m_wakeUpTime;
	return *this;
}
