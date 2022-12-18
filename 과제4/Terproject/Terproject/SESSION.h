#pragma once
#include <WS2tcpip.h>
#include <wtypes.h>
#include <mutex>
#include <utility>
#include <unordered_set>
#include "protocol_2022.h"


using namespace std;

enum S_STATE { ST_FREE, ST_ALLOC, ST_INGAME };
enum COMP_TYPE { OP_ACCEPT, OP_RECV, OP_SEND, OP_DB_GET_PLAYER_INFO, OP_NPC_MOVE, OP_NPC_CHASE_MOVE};

class LUA_OBJECT;
class EXP_OVER
{
public:
	WSAOVERLAPPED _over;
	WSABUF _wsabuf;
	char _send_buf[BUF_SIZE];
	COMP_TYPE _comp_type;
	EXP_OVER()
	{
		_wsabuf.len = BUF_SIZE;
		_wsabuf.buf = _send_buf;
		_comp_type = OP_RECV;
		ZeroMemory(&_over, sizeof(_over));
	}
	EXP_OVER(char* packet)
	{
		_wsabuf.len = packet[0];
		_wsabuf.buf = _send_buf;
		ZeroMemory(&_over, sizeof(_over));
		_comp_type = OP_SEND;
		memcpy(_send_buf, packet, packet[0]);
	}
};

class SESSION {
	EXP_OVER _recv_over;
public:
	mutex _s_lock;
	S_STATE _state;
	int _id;
	SOCKET _socket;
	short	x, y;
	char	_name[NAME_SIZE];
	int		_prev_remain;
	unordered_set <int> _view_list;//이 클라의 뷰 리스트
	mutex	_vl; // 뷰 리스트 전용 락
	
	pair<int, int> myLocalSectionIndex = make_pair(0, 0); // 현재 위치한 땅의 인덱스

	short level = 0;
	short exp = 0;
	short hp = 0;
	short maxHp = 0;
	short attackDamage = 0;
	LUA_OBJECT* myLua = nullptr;

private:
	bool ableAttack = true;
public:
	SESSION();

	~SESSION();

	void do_recv();

	void do_send(void* packet);
	void send_login_info_packet();
	void send_move_packet(int c_id, std::array<SESSION, MAX_USER + MAX_NPC>& clients);
	void send_add_player_packet(int c_id, std::array<SESSION, MAX_USER + MAX_NPC>& clients);
	void send_remove_player_packet(int c_id);
	
	void SetAbleAttack(bool b)
	{
		ableAttack = b;
	}
	bool GetAbleAttack() { return ableAttack; }
};

extern array<SESSION, MAX_USER + MAX_NPC> clients;
