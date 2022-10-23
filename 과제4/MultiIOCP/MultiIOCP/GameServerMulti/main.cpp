#include <iostream>
#include <array>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include<thread>
#include<vector>
#include<mutex>
#include<random>
#include<chrono>
#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")
using namespace std;
using namespace chrono;
constexpr int MAX_USER = 2000;

random_device rd;
default_random_engine dre(rd());
uniform_int_distribution<int> uid(0, 400);

enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND };//iocp���� gqgs ������ ���� Ÿ�� ����
class OVER_EXP { // 
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type; // �� �������� ��io�� recv ���� send���� ����
	OVER_EXP()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV; // �ϴ� recv�� �ʱ�ȭ
		ZeroMemory(&_over, sizeof(_over));
	}
	OVER_EXP(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND; // ������ ��ü�ϱ�
		memcpy(_send_buf, packet, packet[0]);
	}
};

enum STATE {
	ST_FREE,
	ST_ALLOC,
	ST_INGAME
};

//mutex�� ��ȣ ���ִ��� atomic operator- ���߿�
//x, y�� ������ ���� ��ȣ���� ���� // ���� �ణ �߶վ����� �ɾ�°� ��������, ������ ����
//
class SESSION {
	OVER_EXP _recv_over; // no data race // �ϳ��� ��������� recv send �ϳ��� ����

public:
	mutex _c_mutex;
	STATE _state;// data race
	//name�� ������Ʈ �Ҷ� in_use�� false�϶�

	int _id; // no data race
	SOCKET _socket;// data race - ?? // ������Ʈ�Ҷ� ���� ����??
	//socket in_use�� ���� ����

	short	x, y; // data race
	char	_name[NAME_SIZE]; // data race

	int		_prev_remain; // ��Ŷ �������� ���� - ������ ��Ŷ ����? // no data race
	
	
public:
	SESSION() :_state(ST_FREE)
	{
		_id = -1;
		_socket = 0;
		x = y = 0;
		_name[0] = 0;
		//in_use = false; // ���ڸ����� �ƴ��� �Ǵ��� ��ü
		_prev_remain = 0;
	}

	~SESSION() {}

	void do_recv()
	{
		DWORD recv_flag = 0;
		memset(&_recv_over._over, 0, sizeof(_recv_over._over));
		_recv_over._wsabuf.len = BUF_SIZE - _prev_remain;
		_recv_over._wsabuf.buf = _recv_over._send_buf + _prev_remain;
		WSARecv(_socket, &_recv_over._wsabuf, 1, 0, &recv_flag,
			&_recv_over._over, 0);
	}

	void do_send(void* packet)
	{
		OVER_EXP* sdata = new OVER_EXP{ reinterpret_cast<char*>(packet) };
		WSASend(_socket, &sdata->_wsabuf, 1, 0, 0, &sdata->_over, 0);
	}
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		/*
		{
			lock_guard<mutex> mut{};
		}*/
		p.id = _id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		x = p.x = uid(dre);
		y = p.y = uid(dre);
		do_send(&p);
	}
	void send_move_packet(int c_id);
};

array<SESSION, MAX_USER> clients; // data race //�̻�Ű�� �� ������
//�� Ŭ���̾�Ʈ�� �̵��ϸ� �ٸ� Ŭ���̾�Ʈ�� ������ ��ε�ĳ��Ʈ �ϱ� ����
//��� �ؾ��ұ�
//�����̳ʰ� ������ �ٲ�ٸ� �����̳� ��ü�� data race  => vector
//������ array�����̳ʴ� �ٲ��� ���� => data race�� ����Ű���ʴ� ������ �����̳� ���
//5õ������ 10��ۿ� �ȵ��´� => ������ ����������̰� ��� ������
//
//��Ƽ���������� ���� ����
//
//array���� �����Ҷ��� data race => mutex�� ����ؾߵ�
//���뿡 ����, map�� erase�ϸ� ���� ���µ� aray�� ������ �� => ������ϸ� ������µ� �װ� �ȵ�
//
//
//


HANDLE h_iocp; // no data race


SOCKET g_c_socket;
SOCKET g_s_socket; // no data race

OVER_EXP g_a_over; // no data race



void worker_thread();


void SESSION::send_move_packet(int c_id)
{
	SC_MOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(SC_MOVE_PLAYER_PACKET);
	p.type = SC_MOVE_PLAYER;
	p.x = clients[c_id].x;
	p.y = clients[c_id].y;
	p.move_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
	do_send(&p);
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i) {
		lock_guard<mutex> ml{ clients[i]._c_mutex };
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
		memcpy(clients[c_id]._name, p->name, NAME_SIZE);
		clients[c_id].send_login_info_packet();

		{
			lock_guard<mutex> ll(clients[c_id]._c_mutex);
			clients[c_id]._state = ST_INGAME;
		}

		SC_ADD_PLAYER_PACKET add_packet;
		add_packet.id = c_id;
		strcpy_s(add_packet.name, p->name);
		add_packet.size = sizeof(add_packet);
		add_packet.type = SC_ADD_PLAYER;
		add_packet.x = clients[c_id].x;
		add_packet.y = clients[c_id].y;
		for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._c_mutex);
				if (ST_INGAME != pl._state || pl._id == c_id) continue;
			}			
			pl.do_send(&add_packet);
		}

		for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._c_mutex);
				if (ST_INGAME != pl._state || pl._id == c_id) continue;
			}			
			SC_ADD_PLAYER_PACKET diffAdd_packet;
			diffAdd_packet.id = pl._id;
			strcpy_s(diffAdd_packet.name, pl._name);
			diffAdd_packet.size = sizeof(diffAdd_packet);
			diffAdd_packet.type = SC_ADD_PLAYER;
			diffAdd_packet.x = pl.x;
			diffAdd_packet.y = pl.y;
			clients[c_id].do_send(&diffAdd_packet);
		}
		break;
	}
	case CS_MOVE: {
		CS_MOVE_PACKET* p = reinterpret_cast<CS_MOVE_PACKET*>(packet);
		short x = clients[c_id].x;
		short y = clients[c_id].y;
		switch (p->direction) {
		case 0: if (y > 0) y--; break;
		case 1: if (y < W_HEIGHT - 1) y++; break;
		case 2: if (x > 0) x--; break;
		case 3: if (x < W_WIDTH - 1) x++; break;
		}
		clients[c_id].x = x;
		clients[c_id].y = y;
		for (auto& pl : clients) {
			{
				lock_guard<mutex> ll(pl._c_mutex);
				if (ST_INGAME != pl._state) continue;
			}
			pl.send_move_packet(c_id);
		}
		break;
	}
	}
}

void disconnect(int c_id)
{
	SC_REMOVE_PLAYER_PACKET p;
	p.id = c_id;
	p.size = sizeof(p);
	p.type = SC_REMOVE_PLAYER;
	for (auto& pl : clients) {
		{
			lock_guard<mutex> ll(pl._c_mutex);
			if (ST_INGAME != pl._state || pl._id == c_id) continue;
		}		
		pl.do_send(&p);
	}
	closesocket(clients[c_id]._socket);
	lock_guard<mutex> ll(clients[c_id]._c_mutex);
	clients[c_id]._state = ST_FREE;
}

int main()
{

	WSADATA WSAData;
	WSAStartup(MAKEWORD(2, 2), &WSAData);
	g_s_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	SOCKADDR_IN server_addr;
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT_NUM);
	server_addr.sin_addr.S_un.S_addr = INADDR_ANY;
	bind(g_s_socket, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr));
	listen(g_s_socket, SOMAXCONN);
	SOCKADDR_IN cl_addr;
	int addr_size = sizeof(cl_addr);
	int client_id = 0;

	h_iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_s_socket), h_iocp, 9999, 0);
	g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	g_a_over._comp_type = OP_ACCEPT;
	AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);


	vector<thread> worker_threads;
	int num_threads = thread::hardware_concurrency();
	for (int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back(worker_thread);
	for (auto& th : worker_threads)
		th.join();

	closesocket(g_s_socket);
	WSACleanup();
}

void worker_thread() {

	while (true) {
		DWORD num_bytes;
		ULONG_PTR key;
		WSAOVERLAPPED* over = nullptr;
		BOOL ret = GetQueuedCompletionStatus(h_iocp, &num_bytes, &key, &over, INFINITE);
		OVER_EXP* ex_over = reinterpret_cast<OVER_EXP*>(over);
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
					lock_guard<mutex> ll(clients[client_id]._c_mutex);
					clients[client_id]._state = ST_ALLOC;
				}
				clients[client_id].x = 0;
				clients[client_id].y = 0;
				clients[client_id]._id = client_id;
				clients[client_id]._name[0] = 0;
				clients[client_id]._prev_remain = 0;
				clients[client_id]._socket = g_c_socket;
				CreateIoCompletionPort(reinterpret_cast<HANDLE>(g_c_socket), h_iocp, client_id, 0);
				clients[client_id].do_recv();
				g_c_socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
			}
			else {
				cout << "Max user exceeded.\n";
			}
			ZeroMemory(&g_a_over._over, sizeof(g_a_over._over));
			SOCKADDR_IN cl_addr;
			int addr_size = sizeof(cl_addr);
			AcceptEx(g_s_socket, g_c_socket, g_a_over._send_buf, 0, addr_size + 16, addr_size + 16, 0, &g_a_over._over);
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