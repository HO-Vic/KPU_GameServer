#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <random>
#include <concurrent_priority_queue.h>
#include "SESSION.h"
#include "LOCAL_SESSION.h"
#include "DB_OBJ.h"
#include "TIMER_EVENT.h"
#include "LUA_OBJECT.h"
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

array<int, 10> levelExp = { 100, 200, 300, 400 ,500, 600, 700, 800, 900, 1200 };
array<int, 10> levelMaxHp = { 200, 500, 600 ,700, 900, 1000, 1100, 1200, 1500, 1700 };
array<SESSION, MAX_USER + MAX_NPC> clients;
array < array<LOCAL_SESSION, 100>, 100> gameMap;
pair<int, int> AStartObstacle[31];
SOCKET listenSocket;
SOCKET clientSocket;
EXP_OVER acceptOver;
HANDLE g_iocpHandle;

random_device npcRd;
default_random_engine npcDre(npcRd());
uniform_int_distribution<int> npcRandDirUid(0, 3); // inclusive
chrono::system_clock::time_point g_nowTime = chrono::system_clock::now();
concurrency::concurrent_priority_queue<TIMER_EVENT> eventTimerQueue;

//main func
bool isPc(int id);
bool can_see(int from, int to);
bool canAttack(int from, int to);
int get_new_client_id();
void process_packet(int c_id, char* packet);
void disconnect(int c_id);
void worker_thread();
void AttackNpc(int cId, int npcId);

void UpdateNearList(std::unordered_set<int>& newNearList, int c_id);

//NPC func
void InitializeNPC();
void WakeUpNPC(int npcId, int waker);
bool MoveRandNPC(int npcId);
bool Agro_NPC(int npcId, int cId);

//Timer
void TimerWorkerThread();

constexpr int VIEW_RANGE = 8;
constexpr int AGRO_RANGE = 6;
constexpr int Attack_RANGE = 3;
int main()
{
	cout << "initialize Map" << endl;
	{
		AStartObstacle[0] = (make_pair(5, 1));
		AStartObstacle[1] = (make_pair(6, 1));
		AStartObstacle[2] = (make_pair(7, 1));
		AStartObstacle[3] = (make_pair(2, 3));
		AStartObstacle[4] = (make_pair(5, 3));
		AStartObstacle[5] = (make_pair(14, 3));
		AStartObstacle[6] = (make_pair(17, 4));
		AStartObstacle[7] = (make_pair(2, 6));
		AStartObstacle[8] = (make_pair(10, 6));
		AStartObstacle[9] = (make_pair(6, 8));
		AStartObstacle[10] = (make_pair(7, 8));
		AStartObstacle[11] = (make_pair(8, 8));
		AStartObstacle[12] = (make_pair(9, 8));
		AStartObstacle[13] = (make_pair(10, 8));
		AStartObstacle[14] = (make_pair(13, 8));
		AStartObstacle[15] = (make_pair(6, 9));
		AStartObstacle[16] = (make_pair(7, 9));
		AStartObstacle[17] = (make_pair(8, 9));
		AStartObstacle[18] = (make_pair(9, 9));
		AStartObstacle[19] = (make_pair(10, 9));
		AStartObstacle[20] = (make_pair(17, 10));
		AStartObstacle[21] = (make_pair(2, 11));
		AStartObstacle[22] = (make_pair(15, 13));
		AStartObstacle[23] = (make_pair(13, 14));
		AStartObstacle[24] = (make_pair(2, 15));
		AStartObstacle[25] = (make_pair(6, 15));
		AStartObstacle[26] = (make_pair(16, 15));
		AStartObstacle[27] = (make_pair(16, 16));
		AStartObstacle[28] = (make_pair(7, 17));
		AStartObstacle[29] = (make_pair(8, 17));
		AStartObstacle[30] = (make_pair(16, 17));
	}
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			gameMap[i][j].SetPos(i, j);
			// initialize Obj on Local Map
		}
	}
	InitializeNPC();


	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << "wsaStartUp Error" << endl;
		WSACleanup();
		return -1;
	}

	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	bind(listenSocket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(listenSocket, SOMAXCONN);

	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);

	g_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), g_iocpHandle, 9999, 0);

	clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	acceptOver._comp_type = OP_ACCEPT;
	AcceptEx(listenSocket, clientSocket, acceptOver._send_buf, 0, addr_size + 16, addr_size + 16, 0, &acceptOver._over);

	thread timerThread = thread(TimerWorkerThread);
	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();

	for (int i = 0; i < num_threads; ++i) {

		worker_threads.emplace_back(worker_thread);
	}

	for (auto& th : worker_threads)
		th.join();
	timerThread.join();

	closesocket(listenSocket);
	WSACleanup();
}

bool isPc(int id)
{
	return id < MAX_USER;
}

bool can_see(int from, int to)
{
	if ((int)abs(clients[from].x - clients[to].x) > VIEW_RANGE)
		return false;
	if ((int)abs(clients[from].y - clients[to].y) > VIEW_RANGE)
		return false;
	return true;
}

bool canAttack(int from, int to)
{
	if ((int)abs(clients[from].x - clients[to].x) > Attack_RANGE)
		return false;
	if ((int)abs(clients[from].y - clients[to].y) > Attack_RANGE)
		return false;
	return true;
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard <mutex> ll{ clients[i]._s_lock };
		if (clients[i]._state == ST_FREE)
			return i;
	}
	return -1;
}

void process_packet(int c_id, char* packet)
{
	switch (packet[1]) {
	case CS_LOGIN:
	{
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);

		EXP_OVER* exOver = new EXP_OVER();
		exOver->_comp_type = OP_DB_GET_PLAYER_INFO;
		memcpy(exOver->_send_buf, p->name, strlen(p->name));
		exOver->_send_buf[strlen(p->name)] = 0;
		PostQueuedCompletionStatus(g_iocpHandle, strlen(p->name), c_id, &exOver->_over);
	}
	break;
	case CS_MOVE:
	{
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short x = clients[c_id].x;
		short y = clients[c_id].y;
		switch (p->direction) {
		case 1: if (y > 0) y--; break;
		case 2: if (y < W_HEIGHT - 1) y++; break;
		case 3: if (x > 0) x--; break;
		case 4: if (x < W_WIDTH - 1) x++; break;
		}

		//map Object Collision
		if (gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].CollisionObject(x, y)) {
			x = clients[c_id].x;
			y = clients[c_id].y;
		}

		clients[c_id].x = x;
		clients[c_id].y = y;

		gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].UpdatePlayers(clients[c_id], gameMap);//현재 로컬 최신화

		//뷰 리스트 업데이트를 위한 new near List 생성
		unordered_set<int> near_list;
		clients[c_id]._vl.lock();
		unordered_set<int> old_vlist = clients[c_id]._view_list;
		clients[c_id]._vl.unlock();

		UpdateNearList(near_list, c_id);

		clients[c_id].send_move_packet(c_id, clients);
		for (auto& pl : near_list) {
			auto& cpl = clients[pl];
			cpl._vl.lock();
			if (cpl._view_list.count(c_id) > 0) {
				cpl._vl.unlock();
				if (isPc(pl))
					cpl.send_move_packet(c_id, clients);
			}
			else {
				cpl._vl.unlock();
				if (isPc(pl))
					cpl.send_add_player_packet(c_id, clients);
				else {
					clients[pl]._vl.lock();
					clients[pl]._view_list.insert(c_id);
					clients[pl]._vl.unlock();
					WakeUpNPC(pl, c_id);
				}
			}

			if (old_vlist.count(pl) == 0) {
				if (isPc(pl))
					clients[pl].send_add_player_packet(c_id, clients);
				else {
					clients[pl]._vl.lock();
					clients[pl]._view_list.insert(c_id);
					clients[pl]._vl.unlock();
					WakeUpNPC(pl, c_id);
				}
				clients[c_id].send_add_player_packet(pl, clients);
			}
		}

		for (auto& pl : old_vlist)
			if (0 == near_list.count(pl)) {
				clients[c_id].send_remove_player_packet(pl);
				if (isPc(pl))
					clients[pl].send_remove_player_packet(c_id);
				else {
					clients[pl]._vl.lock();
					clients[pl]._view_list.erase(c_id);
					clients[pl]._vl.unlock();
				}
			}
	}
	break;
	case CS_ATTACK:
	{
		if (clients[c_id].GetAbleAttack()) {
			SC_ATTACK_PACKET packet;
			packet.size = sizeof(SC_ATTACK_PACKET);
			packet.type = SC_ATTACK;
			clients[c_id].do_send(&packet);

			TIMER_EVENT ev{ c_id, chrono::system_clock::now() + 1s, EV_PLAYER_ATTACK_COOL, 0 };
			eventTimerQueue.push(ev);

			for (auto& npc : clients[c_id]._view_list) {
				if (!isPc(npc) && canAttack(c_id, npc))
					AttackNpc(c_id, npc);
			}
			clients[c_id].SetAbleAttack(false);
		}
	}
	break;
	}
}

void disconnect(int c_id)
{
	clients[c_id]._vl.lock();
	unordered_set <int> vl = clients[c_id]._view_list;
	clients[c_id]._vl.unlock();
	for (auto& p_id : vl) {
		auto& pl = clients[p_id];
		{
			lock_guard<mutex> ll(pl._s_lock);
			if (ST_INGAME != pl._state) continue;
		}
		if (pl._id == c_id) continue;
		pl.send_remove_player_packet(c_id);
	}
	closesocket(clients[c_id]._socket);

	lock_guard<mutex> ll(clients[c_id]._s_lock);
	clients[c_id]._state = ST_FREE;
}

void worker_thread()
{
	DB_OBJ dbObj = DB_OBJ{};
	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(g_iocpHandle, &num_bytes, &key, &over, INFINITE);
		EXP_OVER* ex_over = reinterpret_cast<EXP_OVER*>(over);
		if (FALSE == ret) {
			if (ex_over->_comp_type == OP_ACCEPT) cout << "Accept Error";
			else {
				cout << "GQCS Error on client[" << key << "]\n";
				disconnect(static_cast<int>(key));
				if (ex_over->_comp_type == OP_SEND) delete ex_over;
				continue;
			}
		}

		switch (ex_over->_comp_type) {
		case OP_ACCEPT: {
			int client_id = get_new_client_id();
			if (client_id != -1) {
				{
					lock_guard<mutex> ll(clients[client_id]._s_lock);
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._view_list.clear();
				clients[client_id]._socket = clientSocket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), g_iocpHandle, client_id, 0);
				clients[client_id].do_recv();
				clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&acceptOver._over, sizeof(acceptOver._over));
			int addr_size = sizeof(SOCKADDR_IN);
			AcceptEx(listenSocket, clientSocket, acceptOver._send_buf, 0, addr_size + 16, addr_size + 16, 0, &acceptOver._over);
			break;
		}
		case OP_RECV: {
			int remain_data = num_bytes + clients[key]._prev_remain;
			char* p = ex_over->_send_buf;
			while (remain_data > 0) {
				int packet_size = p[0];
				if (packet_size <= remain_data) {
					process_packet(static_cast<int>(key), p);
					p = p + packet_size;
					remain_data = remain_data - packet_size;
				}
				else break;
			}
			clients[key]._prev_remain = remain_data;
			if (remain_data > 0) {
				memcpy(ex_over->_send_buf, p, remain_data);
			}
			clients[key].do_recv();
			break;
		}
		case OP_SEND:
			delete ex_over;
			break;
		case OP_DB_GET_PLAYER_INFO:
		{
			string userStr{ ex_over->_send_buf, strlen(ex_over->_send_buf) };
			wstring userId;
			userId.assign(userStr.begin(), userStr.end());
			wstring playerName;

			dbObj.GetPlayerInfo(userId, playerName, clients[key].x, clients[key].y, clients[key].level, clients[key].exp, clients[key].hp, clients[key].maxHp, clients[key].attackDamage);

			if (playerName.empty()) {
				SC_LOGIN_FAIL_PACKET failPacket;
				failPacket.size = 2;
				failPacket.type = SC_LOGIN_FAIL;
				clients[key].do_send(&failPacket);
			}
			else {
				string playerStr;
				playerStr.assign(playerName.begin(), playerName.end());
				memcpy(clients[key]._name, playerStr.c_str(), NAME_SIZE);
				clients[key].send_login_info_packet();

				clients[key].myLocalSectionIndex.first = clients[key].x / 20;
				clients[key].myLocalSectionIndex.second = clients[key].y / 20;
				{
					lock_guard<mutex> ll{ clients[key]._s_lock };
					clients[key]._state = ST_INGAME;
				}
				UpdateNearList(clients[key]._view_list, key);
				for (auto& pl : clients[key]._view_list) {

					if (isPc(pl)) clients[pl].send_add_player_packet(key, clients);
					else WakeUpNPC(pl, key);
					clients[key].send_add_player_packet(clients[pl]._id, clients);
				}
			}
		}
		break;
		case OP_NPC_MOVE:
		{
			bool keep_alive = false;
			for (auto index : clients[key]._view_list) {
				if (clients[index]._state != ST_INGAME) continue;
				if (can_see(static_cast<int>(key), index)) {
					keep_alive = true;
					break;
				}
			}
			if (true == keep_alive) {
				if (MoveRandNPC(static_cast<int>(key))) {
					TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
					eventTimerQueue.push(ev);
				}
			}
			else {
				if (clients[key].myLua != nullptr) {
					clients[key].myLua->InActiveNPC();
					if (clients[key].myLua->InActiveChase())
						clients[key].myLua->SetChaseId(-1);
				}
			}
			delete ex_over;
		}
		break;
		case OP_NPC_CHASE_MOVE:
		{
			//이미 붙어 있다면
			int chaseId = clients[key].myLua->GetChaseId();
			if (chaseId < 0);
			else if (clients[key].x == clients[chaseId].x && clients[key].y == clients[chaseId].y) {
				TIMER_EVENT ev{ key, chrono::system_clock::now() + 4s, EV_CHASE_MOVE, 0 };
				eventTimerQueue.push(ev);
			}
			else {
				//길찾기 실행
				pair<int, int> res = clients[key].myLua->GetNextNode();
				//결과 값(길 list)이 비었다면
				if (res.first < 0 || res.second < 0) {
					//새로 길 찾기 실행
					if (!Agro_NPC(key, chaseId)) {
						clients[key].myLua->InActiveChase();
						TIMER_EVENT ev{ key, chrono::system_clock::now() + 2s, EV_RANDOM_MOVE, 0 };
						eventTimerQueue.push(ev);
					}
					else {

						pair<int, int> res = clients[key].myLua->AStarLoad(clients[key].x, clients[key].y, clients[chaseId].x, clients[chaseId].y);
						//적용
						clients[key].x = res.first;
						clients[key].y = res.second;


						clients[key]._vl.lock();
						unordered_set<int> old_vlist = clients[key]._view_list;
						clients[key]._vl.unlock();

						gameMap[clients[key].myLocalSectionIndex.first][clients[key].myLocalSectionIndex.second].UpdatePlayers(clients[key], gameMap);//현재 로컬 최신화
						UpdateNearList(clients[key]._view_list, key);

						//npc not send
						//clients[npcId].send_move_packet(npcId, clients);

						for (auto& pl : clients[key]._view_list) {
							auto& cpl = clients[pl];
							cpl._vl.lock();
							if (cpl._view_list.count(key) > 0) {
								cpl._vl.unlock();
								if (isPc(pl))
									cpl.send_move_packet(key, clients);
							}
							else {
								cpl._vl.unlock();
								if (isPc(pl))
									clients[pl].send_add_player_packet(key, clients);
							}
						}

						for (auto& pl : old_vlist)
							if (0 == clients[key]._view_list.count(pl))
								if (isPc(pl))
									clients[pl].send_remove_player_packet(key);

						TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_CHASE_MOVE, 0 };
						eventTimerQueue.push(ev);
					}
				}
				else {
					//길이 존재 한다면 고고
					clients[key].x = res.first;
					clients[key].y = res.second;

					clients[key]._vl.lock();
					unordered_set<int> old_vlist = clients[key]._view_list;
					clients[key]._vl.unlock();

					gameMap[clients[key].myLocalSectionIndex.first][clients[key].myLocalSectionIndex.second].UpdatePlayers(clients[key], gameMap);//현재 로컬 최신화
					UpdateNearList(clients[key]._view_list, key);

					//npc not send
					//clients[npcId].send_move_packet(npcId, clients);

					for (auto& pl : clients[key]._view_list) {
						auto& cpl = clients[pl];
						cpl._vl.lock();
						if (cpl._view_list.count(key) > 0) {
							cpl._vl.unlock();
							if (isPc(pl))
								cpl.send_move_packet(key, clients);
						}
						else {
							cpl._vl.unlock();
							if (isPc(pl))
								clients[pl].send_add_player_packet(key, clients);
						}
					}

					for (auto& pl : old_vlist)
						if (0 == clients[key]._view_list.count(pl))
							if (isPc(pl))
								clients[pl].send_remove_player_packet(key);

					TIMER_EVENT ev{ key, chrono::system_clock::now() + 1s, EV_CHASE_MOVE, 0 };
					eventTimerQueue.push(ev);
				}
			}
		}
		break;
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

void AttackNpc(int cId, int npcId)
{
	clients[npcId].hp = clients[npcId].hp - clients[cId].attackDamage;
	if (clients[npcId].hp < 0) {
		clients[cId].exp += clients[npcId].exp;
		if (levelExp[clients[cId].level - 1] < clients[cId].exp) {
			clients[cId].exp -= levelExp[clients[npcId].level - 1];
			clients[npcId].level += 1;
			clients[cId].maxHp = levelMaxHp[clients[npcId].level - 2];
			clients[cId].hp = clients[cId].maxHp;
		}
		SC_STAT_CHANGEL_PACKET sendPakcet;
		sendPakcet.size = sizeof(SC_STAT_CHANGEL_PACKET);
		sendPakcet.hp = clients[cId].hp;
		sendPakcet.level = clients[cId].level;
		sendPakcet.max_hp = clients[cId].maxHp;
		sendPakcet.type = SC_STAT_CHANGE;
		sendPakcet.exp = clients[cId].exp;
		clients[cId].do_send(&sendPakcet);
	}
}

void UpdateNearList(std::unordered_set<int>& newNearList, int c_id)
{
	if (isPc(c_id)) {
		for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].GetPlayer()) { // current my local
			if (clients[id]._state != ST_INGAME) continue;
			if (clients[id]._id == c_id) continue;
			if (can_see(c_id, id))
				newNearList.insert(id);
		}
		//근첩한 local 탐색
		if (clients[c_id].x % 20 < 7) { // 좌로 붙은 섹션
			if (clients[c_id].y % 20 < 7) {// 위로 붙은 부분
				if (clients[c_id].myLocalSectionIndex.second > 0 && clients[c_id].myLocalSectionIndex.first > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.first > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.second > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
			}
			else if (clients[c_id].y % 20 > 13) { // 아래로 붙은 부분
				if (clients[c_id].myLocalSectionIndex.second < 19 && clients[c_id].myLocalSectionIndex.first > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.second < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.first > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
			}
			if (clients[c_id].myLocalSectionIndex.first > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
		else if (clients[c_id].x % 20 > 13) { // 우로 붙은 섹션
			if (clients[c_id].y % 20 < 7) {
				if (clients[c_id].myLocalSectionIndex.second > 0 && clients[c_id].myLocalSectionIndex.first < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.second > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.first < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
			}
			else if (clients[c_id].y % 20 > 13) {
				if (clients[c_id].myLocalSectionIndex.second < 19 && clients[c_id].myLocalSectionIndex.first < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.second < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
				else if (clients[c_id].myLocalSectionIndex.first < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
			}
			if (clients[c_id].myLocalSectionIndex.first < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
		else { // x는 내부에 잘 있고 y만 체크
			if (clients[c_id].y % 20 < 7) {
				if (clients[c_id].myLocalSectionIndex.second > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
			}
			else if (clients[c_id].y % 20 > 13) {
				if (clients[c_id].myLocalSectionIndex.second < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							newNearList.insert(id);
					}
				}
			}
		}
		return;
	}
	//npc view list -> players
	for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].GetPlayer()) { // current my local
		if (clients[id]._state != ST_INGAME) continue;
		if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
		if (can_see(c_id, id))
			newNearList.insert(id);
	}
	//근첩한 local 탐색
	if (clients[c_id].x % 20 < 7) { // 좌로 붙은 섹션
		if (clients[c_id].y % 20 < 7) {// 위로 붙은 부분
			if (clients[c_id].myLocalSectionIndex.second > 0 && clients[c_id].myLocalSectionIndex.first > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.first > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.second > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
		else if (clients[c_id].y % 20 > 13) { // 아래로 붙은 부분
			if (clients[c_id].myLocalSectionIndex.second < 19 && clients[c_id].myLocalSectionIndex.first > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.second < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.first > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
		if (clients[c_id].myLocalSectionIndex.first > 0) {
			for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
				if (clients[id]._state != ST_INGAME) continue;
				if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
				if (can_see(c_id, id))
					newNearList.insert(id);
			}
		}
	}
	else if (clients[c_id].x % 20 > 13) { // 우로 붙은 섹션
		if (clients[c_id].y % 20 < 7) {
			if (clients[c_id].myLocalSectionIndex.second > 0 && clients[c_id].myLocalSectionIndex.first < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.second > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.first < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
		else if (clients[c_id].y % 20 > 13) {
			if (clients[c_id].myLocalSectionIndex.second < 19 && clients[c_id].myLocalSectionIndex.first < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.second < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
			else if (clients[c_id].myLocalSectionIndex.first < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
		if (clients[c_id].myLocalSectionIndex.first < 19) {
			for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
				if (clients[id]._state != ST_INGAME) continue;
				if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
				if (can_see(c_id, id))
					newNearList.insert(id);
			}
		}
	}
	else { // x는 내부에 잘 있고 y만 체크
		if (clients[c_id].y % 20 < 7) {
			if (clients[c_id].myLocalSectionIndex.second > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
		else if (clients[c_id].y % 20 > 13) {
			if (clients[c_id].myLocalSectionIndex.second < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id || !isPc(clients[id]._id)) continue;
					if (can_see(c_id, id))
						newNearList.insert(id);
				}
			}
		}
	}
}

void InitializeNPC()
{
	cout << "NPC intialize begin.\n";
	for (int i = MAX_USER; i < MAX_USER + 3; ++i) {
		clients[i]._id = i;
		clients[i].myLua = new LUA_OBJECT(clients[i]._id, "lua_script\boss.lua");
		clients[i]._state = ST_INGAME;
		memcpy(clients[i]._name, "boss", 4);
		clients[i].myLocalSectionIndex = make_pair(clients[i].x / 20, clients[i].y / 20);
		gameMap[clients[i].myLocalSectionIndex.first][clients[i].myLocalSectionIndex.second].InsertPlayers(clients[i]);
		clients[i]._state = ST_INGAME;
	}
	for (int i = MAX_USER + 3; i < MAX_USER + MAX_NPC / 2; ++i) {
		clients[i]._id = i;
		clients[i]._state = ST_INGAME;
		clients[i].myLua = new LUA_OBJECT(clients[i]._id, NPC_TYPE::AGRO);
		string name = "AGRO";
		name.append(std::to_string(i));
		memcpy(clients[i]._name, name.c_str(), name.size());
		clients[i].myLocalSectionIndex = make_pair(clients[i].x / 20, clients[i].y / 20);
		gameMap[clients[i].myLocalSectionIndex.first][clients[i].myLocalSectionIndex.second].InsertPlayers(clients[i]);
	}
	for (int i = MAX_USER + MAX_NPC / 2; i < MAX_USER + MAX_NPC; ++i) {
		clients[i]._id = i;
		string name = "PEACE";
		name.append(std::to_string(i));
		memcpy(clients[i]._name, name.c_str(), name.size());
		clients[i].myLua = new LUA_OBJECT(clients[i]._id, NPC_TYPE::PEACE);
		clients[i].myLocalSectionIndex = make_pair(clients[i].x / 20, clients[i].y / 20);
		gameMap[clients[i].myLocalSectionIndex.first][clients[i].myLocalSectionIndex.second].InsertPlayers(clients[i]);
		clients[i]._state = ST_INGAME;
	}
	cout << "NPC initialize end.\n";
}

void WakeUpNPC(int npcId, int waker)
{
	if (clients[npcId].myLua->ActiveNPC()) {
		TIMER_EVENT ev{ npcId, chrono::system_clock::now() + 1s, EV_RANDOM_MOVE, 0 };
		eventTimerQueue.push(ev);
	}
}

bool MoveRandNPC(int npcId)
{
	//cout << "npc move" << endl;	
	if (clients[npcId].myLua->type == NPC_TYPE::AGRO) {
		for (auto& vlIndex : clients[npcId]._view_list) {
			if (Agro_NPC(npcId, vlIndex)) {
				if (clients[npcId].myLua->ActiveChase()) {
					clients[npcId].myLua->SetChaseId(vlIndex);
					EXP_OVER* expOver = new EXP_OVER();
					expOver->_comp_type = OP_NPC_CHASE_MOVE;
					PostQueuedCompletionStatus(g_iocpHandle, 1, npcId, &expOver->_over);
				}
				return false;
			}
		}
	}
	if (clients[npcId].myLua->InActiveChase())
		clients[npcId].myLua->SetChaseId(-1);

	int x = clients[npcId].x;
	int y = clients[npcId].y;
	switch (npcRandDirUid(npcDre)) {
	case 0: if (x < (W_WIDTH - 1)) x++; break;
	case 1: if (x > 0) x--; break;
	case 2: if (y < (W_HEIGHT - 1)) y++; break;
	case 3:if (y > 0) y--; break;
	}

	//map Object Collision
	if (gameMap[clients[npcId].myLocalSectionIndex.first][clients[npcId].myLocalSectionIndex.second].CollisionObject(x, y)) {
		x = clients[npcId].x;
		y = clients[npcId].y;
	}

	clients[npcId].x = x;
	clients[npcId].y = y;

	clients[npcId]._vl.lock();
	unordered_set<int> old_vlist = clients[npcId]._view_list;
	clients[npcId]._vl.unlock();

	gameMap[clients[npcId].myLocalSectionIndex.first][clients[npcId].myLocalSectionIndex.second].UpdatePlayers(clients[npcId], gameMap);//현재 로컬 최신화
	UpdateNearList(clients[npcId]._view_list, npcId);

	//npc not send
	//clients[npcId].send_move_packet(npcId, clients);

	for (auto& pl : clients[npcId]._view_list) {
		auto& cpl = clients[pl];
		cpl._vl.lock();
		if (cpl._view_list.count(npcId) > 0) {
			cpl._vl.unlock();
			if (isPc(pl))
				cpl.send_move_packet(npcId, clients);
		}
		else {
			cpl._vl.unlock();
			if (isPc(pl))
				clients[pl].send_add_player_packet(npcId, clients);
		}
	}

	for (auto& pl : old_vlist)
		if (0 == clients[npcId]._view_list.count(pl))
			if (isPc(pl))
				clients[pl].send_remove_player_packet(npcId);
	return true;
}

bool Agro_NPC(int npcId, int cId)
{
	if (abs(clients[npcId].x - clients[cId].x) > AGRO_RANGE)
		return false;
	if (abs(clients[npcId].y - clients[cId].y) > AGRO_RANGE)
		return false;
	return true;
}

void TimerWorkerThread()
{
	while (true) {
		TIMER_EVENT ev;
		auto current_time = chrono::system_clock::now();
		if (true == eventTimerQueue.try_pop(ev)) {
			if (ev.wakeupTime > current_time) {
				eventTimerQueue.push(ev);		// 최적화 필요
				// timer_queue에 다시 넣지 않고 처리해야 한다.
				this_thread::sleep_for(1ms);  // 실행시간이 아직 안되었으므로 잠시 대기
				continue;
			}
			switch (ev.eventId) {
			case EV_RANDOM_MOVE:
			{
				EXP_OVER* ov = new EXP_OVER;
				ov->_comp_type = OP_NPC_MOVE;
				PostQueuedCompletionStatus(g_iocpHandle, 1, ev.objId, &ov->_over);
			}
			break;
			case EV_CHASE_MOVE:
			{
				EXP_OVER* ov = new EXP_OVER;
				ov->_comp_type = OP_NPC_CHASE_MOVE;
				PostQueuedCompletionStatus(g_iocpHandle, 1, ev.objId, &ov->_over);
			}
			break;
			case EV_PLAYER_ATTACK_COOL:
			{
				SC_ATTACK_COOL_PACKET packet;
				packet.size = sizeof(SC_ATTACK_COOL_PACKET);
				packet.type = SC_ATTACK_COOL;
				clients[ev.objId].do_send(&packet);
				clients[ev.objId].SetAbleAttack(true);
			}
			break;
			default: break;
			}
			continue;		// 즉시 다음 작업 꺼내기
		}
		this_thread::sleep_for(1ms);   // timer_queue가 비어 있으니 잠시 기다렸다가 다시 시작
	}
}
