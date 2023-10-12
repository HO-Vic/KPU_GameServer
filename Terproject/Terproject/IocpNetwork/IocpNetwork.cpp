#include "stdafx.h"
#include "IocpNetwork.h"
#include "../ExpOver/ExpOver.h"
#include "../GameObject/PlayerObject/PlayerObject.h"
#include "../GameObject/NPC_Object/NPC_OBJECT.h"
#include "../GameObject/GameObject.h"
#include "../MapSession/MapSession.h"
#include "../Packet/PacketManager.h"
#include "../Logic/Logic.h"
#include "../DB/DB_Event.h"

extern array<GameObject*, MAX_USER + MAX_NPC> g_clients;
extern array < array<MapSession, 100>, 100> g_gameMap;
extern array<int, 11> g_levelExp;
extern std::array<int, 11> g_levelMaxHp;
extern std::array<int, 11> g_levelAttackDamage;


IocpNetwork::IocpNetwork()
{
	m_acceptExpOver = new ExpOver(OP_CODE::OP_ACCEPT);
	ZeroMemory(m_acceptBuffer, BUF_SIZE);
	InitIocp();
}

IocpNetwork::~IocpNetwork()
{
	delete m_acceptExpOver;
}

void IocpNetwork::ExecuteAccept()
{
	m_acceptExpOver->ResetOverlapped();
	int addr_size = sizeof(SOCKADDR_IN);
	m_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	AcceptEx(m_listenSocket, m_clientSocket, m_acceptBuffer, 0, addr_size + 16, addr_size + 16, 0, reinterpret_cast<WSAOVERLAPPED*>(m_acceptExpOver));
}

void IocpNetwork::InitIocp()
{
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cerr << "wsaStartUp Error\n";
		WSACleanup();
		exit(-1);
	}
}

void IocpNetwork::Start()
{
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NUM);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	bind(m_listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
	listen(m_listenSocket, SOMAXCONN);

	m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_listenSocket), m_iocpHandle, 9999, 0);
	ExecuteAccept();

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();

	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back([&]() {WorkerThread(); });

	for (auto& th : worker_threads)
		th.join();

	closesocket(m_listenSocket);
	WSACleanup();
}

const HANDLE& IocpNetwork::GetIocpHandle()
{
	return m_iocpHandle;
}

void IocpNetwork::WorkerThread()
{
	while (true) {
		DWORD ioByte;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(m_iocpHandle, &ioByte, &key, &over, INFINITE);
		ExpOver* exOver = reinterpret_cast<ExpOver*>(over);
		OP_CODE currentOpCode = exOver->GetOpCode();
		if (FALSE == ret) {
			if (currentOpCode == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				//disconnect(static_cast<int>(key));
				if (currentOpCode == OP_SEND) delete exOver;
				continue;
			}
		}
		switch (currentOpCode) {
		case OP_ACCEPT:
		{
			int newClientId = Logic::GetNewClientId();
			if (newClientId != -1) {
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(m_clientSocket), m_iocpHandle, newClientId, 0);
				PlayerObject* playerObject = dynamic_cast<PlayerObject*>(g_clients[newClientId]);
				playerObject->RegistGameObject(newClientId, m_clientSocket);
				ExecuteAccept();
			}
			else {
				cout << "Max user exceeded.\n";
			}
		}
		break;
		case OP_RECV:
		{
			dynamic_cast<PlayerObject*>(g_clients[key])->RecvPacket(ioByte);
		}
		break;
		case OP_SEND:
			delete exOver;
			break;

		case OP_NPC_MOVE:
		{
			Logic::NPCMove(key);
			delete exOver;
		}
		break;
		case OP_NPC_CHASE_MOVE:
		{
			ExpOverBuffer* bufferOver = reinterpret_cast<ExpOverBuffer*>(over);
			int* volatile targetId = reinterpret_cast<int*>(bufferOver->GetBufferData());
			
			Logic::NPCMove(key, *targetId);
			delete exOver;
		}
		break;
		case OP_DB_GET_PLAYER_INFO:
		{
			DB::DB_PlayerInfo* playerData =
				reinterpret_cast<DB::DB_PlayerInfo*> (
					reinterpret_cast<ExpOverBuffer*>(exOver)->GetBufferData()
					);
			PlayerObject* playerObject = reinterpret_cast<PlayerObject*>(g_clients[playerData->m_id]);
			playerObject->InitSetting(playerData->m_id, playerData->m_hp, playerData->m_maxHp, playerData->m_attackDamage, playerData->m_exp, playerData->m_posX, playerData->m_posY);
			playerObject->SetName(playerData->m_playerName);
			playerObject->SetLoginId(playerData->m_playerId);
			playerObject->SetMaxExp(g_levelExp[playerData->m_level - 1]);
			playerObject->SetLevel(playerData->m_level);
			auto pos = playerObject->GetPosition();
			playerObject->SendLoginInfoPacket();
			Logic::MoveGameObject(key, pos);
			delete exOver;
		}
		break;

		//
		//{
		//	string userStr{ ex_over->_send_buf, strlen(ex_over->_send_buf) };
		//	wstring userId;
		//	userId.assign(userStr.begin(), userStr.end());
		//	wstring playerName;

		//	dbObj.GetPlayerInfo(userId, playerName, clients[key].x, clients[key].y, clients[key].level, clients[key].exp, clients[key].hp, clients[key].maxHp, clients[key].attackDamage);

		//	if (playerName.empty()) {
		//		string userStr{ ex_over->_send_buf, strlen(ex_over->_send_buf) };
		//		wstring userId;
		//		userId.assign(userStr.begin(), userStr.end());
		//		dbObj.AddUser(userId);
		//		SC_LOGIN_FAIL_PACKET failPacket;
		//		failPacket.size = 2;
		//		failPacket.type = SC_LOGIN_FAIL;
		//		clients[key].do_send(&failPacket);
		//	}
		//	else {
		//		string playerStr;
		//		playerStr.assign(playerName.begin(), playerName.end());
		//		std::memcpy(clients[key]._name, playerStr.c_str(), NAME_SIZE);
		//		clients[key].send_login_info_packet();

		//		clients[key].myLocalSectionIndex.first = clients[key].x / 20;
		//		clients[key].myLocalSectionIndex.second = clients[key].y / 20;
		//		{
		//			lock_guard<mutex> ll{ clients[key]._s_lock };
		//			clients[key]._state = ST_INGAME;
		//		}
		//		UpdateNearList(clients[key], key);

		//		for (auto& pl : clients[key]._view_list) {
		//			if (isPc(pl)) clients[pl].send_add_player_packet(key, clients);
		//			else WakeUpNPC(pl, key);
		//			clients[key].send_add_player_packet(clients[pl]._id, clients);
		//		}
		//	}
		//	delete ex_over;
		//}
		//break;

		//case OP_NPC_CHASE_MOVE:
		//{
		//	if (clients[key].myLua->GetArrive() && clients[key].myLua->isChase) {
		//		//이미 붙어 있다면
		//		int chaseId = clients[key].myLua->GetChaseId();
		//		if (chaseId < 0)
		//		{
		//			if (clients[key].myLua->InActiveChase()) {
		//				TIMER_EVENT ev{ key, chrono::system_clock::now() + 2s, EV_RANDOM_MOVE, 0 };
		//				eventTimerQueue.push(ev);
		//			}
		//		}
		//		else if (clients[key].x == clients[chaseId].x && clients[key].y == clients[chaseId].y) {
		//			if (AbleAttack_NPC(key, chaseId) && clients[key].myLua->GetArrive() && clients[chaseId]._name[0] != 'T') {
		//				clients[chaseId].hp = clients[chaseId].hp - clients[key].attackDamage;
		//				if (clients[chaseId].hp < 0) {
		//					clients[chaseId].exp /= 2;
		//					clients[chaseId].maxHp = levelMaxHp[clients[chaseId].level - 1];
		//					clients[chaseId].hp = clients[chaseId].maxHp;

		//					clients[chaseId].x = 0;
		//					clients[chaseId].y = 0;

		//					gameMap[clients[chaseId].myLocalSectionIndex.first][clients[chaseId].myLocalSectionIndex.second].DeletePlayers(clients[chaseId]);//현재 로컬 최신화
		//					clients[chaseId].myLocalSectionIndex = std::make_pair(0, 0);
		//					gameMap[0][0].InsertPlayers(clients[chaseId]);

		//					clients[chaseId].send_move_packet(chaseId, clients);

		//					//뷰 리스트 업데이트를 위한 new near List 생성							
		//					clients[chaseId]._vl.lock();
		//					unordered_set<int> old_vlist = clients[chaseId]._view_list;
		//					clients[chaseId]._vl.unlock();


		//					UpdateNearList(clients[chaseId], chaseId);
		//					for (auto& pl : clients[chaseId]._view_list) {
		//						auto& cpl = clients[pl];
		//						cpl._vl.lock();
		//						if (cpl._view_list.count(chaseId) > 0) {
		//							cpl._vl.unlock();
		//							if (isPc(pl))
		//								cpl.send_move_packet(chaseId, clients);
		//						}
		//						else {
		//							cpl._vl.unlock();
		//							if (isPc(pl))
		//								cpl.send_add_player_packet(chaseId, clients);
		//							else {
		//								clients[pl]._vl.lock();
		//								clients[pl]._view_list.insert(chaseId);
		//								clients[pl]._vl.unlock();
		//								WakeUpNPC(pl, chaseId);
		//							}
		//						}

		//						if (old_vlist.count(pl) == 0) {
		//							/*if (isPc(pl))
		//								clients[pl].send_add_player_packet(chaseId, clients);
		//							else {
		//								clients[pl]._vl.lock();
		//								clients[pl]._view_list.insert(chaseId);
		//								clients[pl]._vl.unlock();
		//								WakeUpNPC(pl, chaseId);
		//							}*/
		//							clients[chaseId].send_add_player_packet(pl, clients);
		//						}
		//					}

		//					for (auto& pl : old_vlist)
		//						if (0 == clients[chaseId]._view_list.count(pl)) {
		//							clients[chaseId].send_remove_player_packet(pl);
		//							if (isPc(pl))
		//								clients[pl].send_remove_player_packet(chaseId);
		//							else {
		//								clients[pl]._vl.lock();
		//								clients[pl]._view_list.erase(chaseId);
		//								clients[pl]._vl.unlock();
		//							}
		//						}

		//					SC_STAT_CHANGEL_PACKET sendPakcet;
		//					sendPakcet.size = sizeof(SC_STAT_CHANGEL_PACKET);
		//					sendPakcet.id = chaseId;
		//					sendPakcet.hp = clients[chaseId].maxHp;
		//					sendPakcet.level = clients[chaseId].level;
		//					sendPakcet.max_hp = clients[chaseId].maxHp;
		//					sendPakcet.type = SC_STAT_CHANGE;
		//					sendPakcet.exp = clients[chaseId].exp;
		//					sendPakcet.max_exp = levelExp[clients[chaseId].level];
		//					clients[chaseId].do_send(&sendPakcet);

		//					EXP_OVER* expOver = new EXP_OVER();
		//					expOver->_comp_type = OP_DB_SAVE_PLAYER;
		//					PostQueuedCompletionStatus(g_iocpHandle, 1, chaseId, &expOver->_over);
		//					if (clients[key].myLua->InActiveChase()) {
		//						TIMER_EVENT ev{ key, chrono::system_clock::now() + 2s, EV_RANDOM_MOVE, 0 };
		//						eventTimerQueue.push(ev);
		//					}
		//				}
		//				else {
		//					SC_STAT_CHANGEL_PACKET sendPakcet;
		//					sendPakcet.size = sizeof(SC_STAT_CHANGEL_PACKET);
		//					sendPakcet.id = chaseId;
		//					sendPakcet.hp = clients[chaseId].hp;
		//					sendPakcet.level = clients[chaseId].level;
		//					sendPakcet.max_hp = clients[chaseId].maxHp;
		//					sendPakcet.type = SC_STAT_CHANGE;
		//					sendPakcet.exp = clients[chaseId].exp;
		//					sendPakcet.max_exp = levelExp[clients[chaseId].level];
		//					clients[chaseId].do_send(&sendPakcet);
		//				}
		//			}
		//			TIMER_EVENT ev{ key, chrono::system_clock::now() + 2s, EV_CHASE_MOVE, 0 };
		//			eventTimerQueue.push(ev);
		//		}
		//		else {
		//			//길찾기 실행
		//			//AStarLoad(clients[key].x, clients[key].y, clients[chaseId].x, clients[chaseId].y);
		//			bool isReFind = clients[key].myLua->IsReFindRoad();
		//			pair<int, int> res = clients[key].myLua->GetNextNode();
		//			//clients[key].x = res.first;
		//			//clients[key].y = res.second;


		//			//clients[key]._vl.lock();
		//			//unordered_set<int> old_vlist = clients[key]._view_list;
		//			//clients[key]._vl.unlock();

		//			//gameMap[clients[key].myLocalSectionIndex.first][clients[key].myLocalSectionIndex.second].UpdatePlayers(clients[key], gameMap);//현재 로컬 최신화
		//			//UpdateNearList(clients[key], key);

		//			////npc not send
		//			////clients[npcId].send_move_packet(npcId, clients);

		//			//for (auto& pl : clients[key]._view_list) {
		//			//	auto& cpl = clients[pl];
		//			//	cpl._vl.lock();
		//			//	if (cpl._view_list.count(key) > 0) {
		//			//		cpl._vl.unlock();
		//			//		if (isPc(pl))
		//			//			cpl.send_move_packet(key, clients);
		//			//	}
		//			//	else {
		//			//		cpl._vl.unlock();
		//			//		if (isPc(pl))
		//			//			clients[pl].send_add_player_packet(key, clients);
		//			//	}
		//			//}

		//			//for (auto& pl : old_vlist)
		//			//	if (0 == clients[key]._view_list.count(pl))
		//			//		if (isPc(pl))
		//			//			clients[pl].send_remove_player_packet(key);

		//			//TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_CHASE_MOVE, 0 };
		//			//eventTimerQueue.push(ev);
		//			//결과 값(길 list)이 비었다면
		//			if (res.first < 0 || res.second < 0 || isReFind) {
		//				//새로 길 찾기 실행
		//				if (!Agro_NPC(key, chaseId)) {
		//					clients[key].myLua->InActiveChase();
		//					TIMER_EVENT ev{ key, chrono::system_clock::now() + 2s, EV_RANDOM_MOVE, 0 };
		//					eventTimerQueue.push(ev);
		//				}
		//				else {
		//					clients[key].myLua->AStarLoad(clients[key].x, clients[key].y, clients[chaseId].x, clients[chaseId].y);
		//					pair<int, int> res = clients[key].myLua->GetNextNode();
		//					if (res.first > 0 && res.second > 0) {
		//						clients[key].x = res.first;
		//						clients[key].y = res.second;
		//					}

		//					clients[key]._vl.lock();
		//					unordered_set<int> old_vlist = clients[key]._view_list;
		//					clients[key]._vl.unlock();

		//					gameMap[clients[key].myLocalSectionIndex.first][clients[key].myLocalSectionIndex.second].UpdatePlayers(clients[key], gameMap);//현재 로컬 최신화
		//					UpdateNearList(clients[key], key);

		//					//npc not send
		//					//clients[npcId].send_move_packet(npcId, clients);

		//					for (auto& pl : clients[key]._view_list) {
		//						auto& cpl = clients[pl];
		//						cpl._vl.lock();
		//						if (cpl._view_list.count(key) > 0) {
		//							cpl._vl.unlock();
		//							if (isPc(pl))
		//								cpl.send_move_packet(key, clients);
		//						}
		//						else {
		//							cpl._vl.unlock();
		//							if (isPc(pl))
		//								clients[pl].send_add_player_packet(key, clients);
		//						}
		//					}

		//					for (auto& pl : old_vlist)
		//						if (0 == clients[key]._view_list.count(pl))
		//							if (isPc(pl))
		//								clients[pl].send_remove_player_packet(key);

		//					TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_CHASE_MOVE, 0 };
		//					eventTimerQueue.push(ev);
		//				}
		//				clients[key].myLua->UpdateLastFindRoadTime();
		//			}
		//			else {
		//				//길이 존재 한다면 고고
		//				clients[key].x = res.first;
		//				clients[key].y = res.second;

		//				clients[key]._vl.lock();
		//				unordered_set<int> old_vlist = clients[key]._view_list;
		//				clients[key]._vl.unlock();

		//				gameMap[clients[key].myLocalSectionIndex.first][clients[key].myLocalSectionIndex.second].UpdatePlayers(clients[key], gameMap);//현재 로컬 최신화
		//				UpdateNearList(clients[key], key);

		//				//npc not send
		//				//clients[npcId].send_move_packet(npcId, clients);

		//				for (auto& pl : clients[key]._view_list) {
		//					auto& cpl = clients[pl];
		//					cpl._vl.lock();
		//					if (cpl._view_list.count(key) > 0) {
		//						cpl._vl.unlock();
		//						if (isPc(pl))
		//							cpl.send_move_packet(key, clients);
		//					}
		//					else {
		//						cpl._vl.unlock();
		//						if (isPc(pl))
		//							clients[pl].send_add_player_packet(key, clients);
		//					}
		//				}

		//				for (auto& pl : old_vlist)
		//					if (0 == clients[key]._view_list.count(pl))
		//						if (isPc(pl))
		//							clients[pl].send_remove_player_packet(key);

		//				TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_CHASE_MOVE, 0 };
		//				eventTimerQueue.push(ev);
		//			}
		//		}
		//	}
		//	delete ex_over;
		//}
		//break;
		//case OP_DB_SAVE_PLAYER:
		//{
		//	std::string idStr = clients[key].playerID;
		//	std::wstring idWstr;
		//	idWstr.assign(idStr.begin(), idStr.end());
		//	dbObj.SavePlayerInfo(idWstr, clients[key].x, clients[key].y, clients[key].level, clients[key].exp, clients[key].hp, clients[key].maxHp, clients[key].attackDamage);
		//	delete ex_over;
		//}
		//break;
		//case OP_DB_AUTO_SAVE_PLAYER:
		//{
		//	for (int i = 0; i < MAX_USER; i++) {
		//		if (clients[i]._name[0] != 'T') {
		//			std::string idStr = clients[i].playerID;
		//			std::wstring idWstr;
		//			idWstr.assign(idStr.begin(), idStr.end());
		//			dbObj.SavePlayerInfo(idWstr, clients[i].x, clients[i].y, clients[i].level, clients[i].exp, clients[i].hp, clients[i].maxHp, clients[i].attackDamage);
		//		}
		//	}
		//	TIMER_EVENT ev{ key, chrono::system_clock::now() + 300s, EV_AUTO_SAVE, 0 };//5분 마다 오토 세이브
		//	eventTimerQueue.push(ev);
		//	delete ex_over;
		//}
		//break;
		/*case OP_DB_SET_PLAYER_POSITION:
		{
			string userStr{ clients[key]._user_ID };
			wstring userId;
			userId.assign(userStr.begin(), userStr.end());
			DBmutex.lock();
			SetPlayerPosition(userId, clients[key].x, clients[key].y);
			DBmutex.unlock();
		}
		break;*/
		}

	}
}

//void AttackNpc(int cId, int npcId)
//{
//	clients[npcId].hp = clients[npcId].hp - clients[cId].attackDamage;
//	if (clients[npcId].hp > 0) {
//		SC_STAT_CHANGEL_PACKET sendPakcet;
//		sendPakcet.id = npcId;
//		sendPakcet.size = sizeof(SC_STAT_CHANGEL_PACKET);
//		sendPakcet.hp = clients[npcId].hp;
//		sendPakcet.level = clients[npcId].level;
//		sendPakcet.max_hp = clients[npcId].maxHp;
//		sendPakcet.type = SC_STAT_CHANGE;
//		sendPakcet.exp = clients[npcId].exp;
//		sendPakcet.max_exp = levelExp[clients[npcId].level];
//		clients[cId].do_send(&sendPakcet);
//	}
//	if (clients[npcId].hp < 0) {
//		if (clients[npcId].myLua->DieNpc()) {
//			clients[cId].exp += clients[npcId].exp;
//			if (clients[cId].level < 11) {
//				if (levelExp[clients[cId].level] < clients[cId].exp) {
//					clients[cId].exp -= levelExp[clients[cId].level];
//					clients[cId].level += 1;
//					clients[cId].maxHp = levelMaxHp[clients[cId].level - 1];
//					clients[cId].attackDamage = levelAttackDamage[clients[cId].level - 1];
//					clients[cId].hp = clients[cId].maxHp;
//				}
//			}
//
//			SC_STAT_CHANGEL_PACKET sendPakcet;
//			sendPakcet.id = npcId;
//			sendPakcet.size = sizeof(SC_STAT_CHANGEL_PACKET);
//			sendPakcet.hp = 0;
//			sendPakcet.level = clients[npcId].level;
//			sendPakcet.max_hp = clients[npcId].maxHp;
//			sendPakcet.type = SC_STAT_CHANGE;
//			sendPakcet.exp = clients[npcId].exp;
//			sendPakcet.max_exp = levelExp[clients[npcId].level];
//			clients[cId].do_send(&sendPakcet);
//
//			SC_STAT_CHANGEL_PACKET npcsendPakcet;
//			npcsendPakcet.size = sizeof(SC_STAT_CHANGEL_PACKET);
//			npcsendPakcet.id = cId;
//			npcsendPakcet.hp = clients[cId].hp;
//			npcsendPakcet.level = clients[cId].level;
//			npcsendPakcet.max_hp = clients[cId].maxHp;
//			npcsendPakcet.type = SC_STAT_CHANGE;
//			npcsendPakcet.exp = clients[cId].exp;
//			npcsendPakcet.max_exp = levelExp[clients[cId].level];
//			clients[cId].do_send(&npcsendPakcet);
//
//			if (clients[cId]._name[0] != 'T') {
//				EXP_OVER* expOver = new EXP_OVER();
//				expOver->_comp_type = OP_DB_SAVE_PLAYER;
//				PostQueuedCompletionStatus(g_iocpHandle, 1, cId, &expOver->_over);
//			}
//			SC_REMOVE_OBJECT_PACKET removePakcet;// 죽었으니 제거
//			removePakcet.type = SC_REMOVE_OBJECT;
//			removePakcet.size = sizeof(SC_REMOVE_OBJECT_PACKET);
//			removePakcet.id = npcId;
//
//			for (auto& vlIndex : clients[npcId]._view_list)// npc의 뷰 리스트가 가지고 있는 클라이언트들에게 전송
//				clients[vlIndex].do_send(&removePakcet);
//
//			TIMER_EVENT ev{ npcId, chrono::system_clock::now() + 30s, EV_RESPAWN_NPC, 0 };
//			eventTimerQueue.push(ev);
//		}
//	}
//}
