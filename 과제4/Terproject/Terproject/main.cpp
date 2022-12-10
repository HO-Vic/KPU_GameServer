#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <thread>
#include <vector>
#include <chrono>
#include "SESSION.h"
#include "LOCAL_SESSION.h"
extern "C"
{
#include "include\lua.h"
#include "include\lauxlib.h"
#include "include\lualib.h"
}

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
#pragma comment(lib, "lua54.lib")

using namespace std;
using namespace chrono;

array<SESSION, MAX_USER> clients;
array < array<LOCAL_SESSION, 100>, 100> gameMap;
SOCKET listenSocket;
SOCKET clientSocket;
EXP_OVER acceptOver;
HANDLE g_iocpHandle;

bool can_see(int from, int to);
int get_new_client_id();
void process_packet(int c_id, char* packet);
void disconnect(int c_id);
void worker_thread();
constexpr int VIEW_RANGE = 15;
int main()
{
	WSADATA WSAData;
	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << "wsaStartUp Error" << endl;
		WSACleanup();
		return -1;
	}

	cout << "initialize Map" << endl;
	for (int i = 0; i < 100; i++) {
		for (int j = 0; j < 100; j++) {
			gameMap[i][j].SetPos(i, j);
		}
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
		worker_threads.emplace_back(worker_thread);
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
	case CS_MOVE: {
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

		unordered_set<int> near_list;

		clients[c_id]._vl.lock();
		unordered_set<int> old_vlist = clients[c_id]._view_list;
		clients[c_id]._vl.unlock();

		for (auto& cl : clients) {
			if (cl._state != ST_INGAME) continue;
			if (cl._id == c_id) continue;
			if (can_see(c_id, cl._id))
				near_list.insert(cl._id);
		}

		clients[c_id].send_move_packet(c_id);

		for (auto& pl : near_list) {
			auto& cpl = clients[pl];
			cpl._vl.lock();
			if (cpl._view_list.count(c_id) > 0) {
				cpl._vl.unlock();
				cpl.send_move_packet(c_id);
			}
			else {
				cpl._vl.unlock();
				clients[pl].send_add_player_packet(c_id);
			}

			if (old_vlist.count(pl) == 0)
				clients[c_id].send_add_player_packet(pl);
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

void worker_thread()
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