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

//concurrency::concurrent_priority_queue<TIMER_EVENT> eventTimerQueue;

//void TimerWorkerThread()
//{
//	while (true) {
//		TIMER_EVENT ev;
//		auto current_time = chrono::system_clock::now();
//		if (true == eventTimerQueue.try_pop(ev)) {
//			if (ev.wakeupTime > current_time) {
//				eventTimerQueue.push(ev);		// ����ȭ �ʿ�
//				// timer_queue�� �ٽ� ���� �ʰ� ó���ؾ� �Ѵ�.
//				this_thread::sleep_for(1ms);  // ����ð��� ���� �ȵǾ����Ƿ� ��� ���
//				continue;
//			}
//			switch (ev.eventId) {
//			case EV_RANDOM_MOVE:
//			{
//				EXP_OVER* ov = new EXP_OVER;
//				ov->_comp_type = OP_NPC_MOVE;
//				PostQueuedCompletionStatus(g_iocpHandle, 1, ev.objId, &ov->_over);
//			}
//			break;
//			case EV_CHASE_MOVE:
//			{
//				EXP_OVER* ov = new EXP_OVER;
//				ov->_comp_type = OP_NPC_CHASE_MOVE;
//				PostQueuedCompletionStatus(g_iocpHandle, 1, ev.objId, &ov->_over);
//			}
//			break;
//			case EV_PLAYER_ATTACK_COOL:
//			{
//				SC_ATTACK_COOL_PACKET packet;
//				packet.size = sizeof(SC_ATTACK_COOL_PACKET);
//				packet.type = SC_ATTACK_COOL;
//				clients[ev.objId].do_send(&packet);
//				clients[ev.objId].SetAbleAttack(true);
//			}
//			break;
//			case EV_RESPAWN_NPC:
//			{
//				clients[ev.objId].myLua->RespawnNpc();
//			}
//			break;
//			case EV_AUTO_SAVE:
//			{
//				EXP_OVER* ov = new EXP_OVER;
//				ov->_comp_type = OP_DB_AUTO_SAVE_PLAYER;
//				PostQueuedCompletionStatus(g_iocpHandle, 1, ev.objId, &ov->_over);
//			}
//			break;
//			default: break;
//			}
//			continue;		// ��� ���� �۾� ������
//		}
//		this_thread::sleep_for(1ms);   // timer_queue�� ��� ������ ��� ��ٷȴٰ� �ٽ� ����
//	}
//}
