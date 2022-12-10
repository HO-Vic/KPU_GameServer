#include <iostream>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include "SESSION.h"
#include "LOCAL_SESSION.h"
#include "DB_OBJ.h"
#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

array<SESSION, MAX_USER> clients;
array < array<LOCAL_SESSION, 100>, 100> gameMap;
SOCKET listenSocket;
SOCKET clientSocket;
EXP_OVER acceptOver;
HANDLE g_iocpHandle;


//main func
bool can_see(int from, int to);
int get_new_client_id();
void process_packet(int c_id, char* packet);
void disconnect(int c_id);
void worker_thread(DB_OBJ dbObj);

//lua func
void InitializeNPC();


constexpr int VIEW_RANGE = 15;
int main()
{
	cout << "initialize Map" << endl;
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			gameMap[i][j].SetPos(i, j);
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

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();

	for (int i = 0; i < num_threads; ++i) {
		DB_OBJ dbObj;
		worker_threads.emplace_back(worker_thread, dbObj);
	}

	for (auto& th : worker_threads)
		th.join();

	closesocket(listenSocket);
	WSACleanup();
}

bool can_see(int from, int to)
{
	if (abs(clients[from].x - clients[to].x) > VIEW_RANGE) return false;
	return abs(clients[from].y - clients[to].y) <= VIEW_RANGE;
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
	case CS_LOGIN: {
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		strcpy_s(clients[c_id]._name, p->name);
		//DB 추가 해야됨
		/*clients[c_id].send_login_info_packet();
		{
			lock_guard<mutex> ll{ clients[c_id]._s_lock };
			clients[c_id]._state = ST_INGAME;
		}
		for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._s_lock);
				if (ST_INGAME != pl._state) continue;
			}
			if (pl._id == c_id) continue;
			if (false == can_see(c_id, pl._id))
				continue;
			clients[c_id]._vl.lock();
			clients[c_id]._view_list.insert(pl._id);
			clients[c_id]._vl.unlock();
			pl.send_add_player_packet(c_id);
			clients[c_id].send_add_player_packet(pl._id);
		}*/
		break;
	}
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
		clients[c_id].x = x;
		clients[c_id].y = y;

		gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].UpdatePlayers(clients[c_id], gameMap);//현재 로컬 최신화

		//뷰 리스트 업데이트를 위한 new near List 생성
		unordered_set<int> near_list;
		clients[c_id]._vl.lock();
		unordered_set<int> old_vlist = clients[c_id]._view_list;
		clients[c_id]._vl.unlock();

		//all clients search - 원본
		/*for (auto& cl : clients) {
			if (cl._state != ST_INGAME) continue;
			if (cl._id == c_id) continue;
			if (can_see(c_id, cl._id))
				near_list.insert(cl._id);
		}*/

		for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second].GetPlayer()) { // current my local
			if (clients[id]._state != ST_INGAME) continue;
			if (clients[id]._id == c_id) continue;
			if (can_see(c_id, id))
				near_list.insert(id);
		}
		//근첩한 local 탐색
		if (clients[c_id].x % 20 < 7) { // 좌로 붙은 섹션
			if (clients[c_id].y % 20 < 7) {// 위로 붙은 부분
				if (clients[c_id].myLocalSectionIndex.second > 0 && clients[c_id].myLocalSectionIndex.first > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second - 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							near_list.insert(id);
					}
				}
			}
			else if (clients[c_id].y % 20 > 13) { // 아래로 붙은 부분
				if (clients[c_id].myLocalSectionIndex.second < 19 && clients[c_id].myLocalSectionIndex.first > 0) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							near_list.insert(id);
					}
				}
			}
			if (clients[c_id].myLocalSectionIndex.first > 0) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first - 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id) continue;
					if (can_see(c_id, id))
						near_list.insert(id);
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
							near_list.insert(id);
					}
				}
			}
			else if (clients[c_id].y % 20 > 13) {
				if (clients[c_id].myLocalSectionIndex.second < 19 && clients[c_id].myLocalSectionIndex.first < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							near_list.insert(id);
					}
				}
			}
			if (clients[c_id].myLocalSectionIndex.first < 19) {
				for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first + 1][clients[c_id].myLocalSectionIndex.second].GetPlayer()) {
					if (clients[id]._state != ST_INGAME) continue;
					if (clients[id]._id == c_id) continue;
					if (can_see(c_id, id))
						near_list.insert(id);
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
							near_list.insert(id);
					}
				}
			}
			else if (clients[c_id].y % 20 > 13) {
				if (clients[c_id].myLocalSectionIndex.second < 19) {
					for (auto& id : gameMap[clients[c_id].myLocalSectionIndex.first][clients[c_id].myLocalSectionIndex.second + 1].GetPlayer()) {
						if (clients[id]._state != ST_INGAME) continue;
						if (clients[id]._id == c_id) continue;
						if (can_see(c_id, id))
							near_list.insert(id);
					}
				}
			}
		}


		clients[c_id].send_move_packet(c_id, clients);

		for (auto& pl : near_list) {
			auto& cpl = clients[pl];
			cpl._vl.lock();
			if (cpl._view_list.count(c_id) > 0) {
				cpl._vl.unlock();
				cpl.send_move_packet(c_id, clients);
			}
			else {
				cpl._vl.unlock();
				clients[pl].send_add_player_packet(c_id, clients);
			}

			if (old_vlist.count(pl) == 0)
				clients[c_id].send_add_player_packet(pl, clients);
		}

		for (auto& pl : old_vlist)
			if (0 == near_list.count(pl)) {
				clients[c_id].send_remove_player_packet(pl);
				clients[pl].send_remove_player_packet(c_id);
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

void worker_thread(DB_OBJ dbObj)
{
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
		}
	}
}

void InitializeNPC()
{
	cout << "NPC intialize begin.\n";
	for (int i = MAX_USER; i < MAX_USER + MAX_NPC; ++i) {
		clients[i].x = rand() % W_WIDTH;
		clients[i].y = rand() % W_HEIGHT;
		clients[i]._id = i;
		sprintf_s(clients[i]._name, "NPC%d", i);
		clients[i]._state = ST_INGAME;

		auto L = clients[i].myLuaState = luaL_newstate();
		luaL_openlibs(L);
		luaL_loadfile(L, "npc.lua");
		lua_pcall(L, 0, 0, 0);

		lua_getglobal(L, "set_uid");
		lua_pushnumber(L, i);
		lua_pcall(L, 1, 0, 0);
		// lua_pop(L, 1);// eliminate set_uid from stack after call

		/*lua_register(clients[i]._L, "SendHelloMessage", API_helloSendMessage);
		lua_register(clients[i]._L, "SendByeMessage", API_ByeSendMessage);
		lua_register(L, "API_get_x", API_get_x);
		lua_register(L, "API_get_y", API_get_y);*/
	}
	cout << "NPC initialize end.\n";
}
