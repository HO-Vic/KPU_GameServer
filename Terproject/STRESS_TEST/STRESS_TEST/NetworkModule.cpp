#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <WinSock2.h>
#include <winsock.h>
#include <Windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <unordered_set>
#include <mutex>
#include <atomic>
#include <chrono>
#include <queue>
#include <array>
#include <memory>
#include "Metric.h"

using namespace std;
using namespace chrono;

extern HWND		hWnd;

const static int MAX_TEST = 6000;
const static int MAX_CLIENTS = MAX_TEST * 2;
const static int INVALID_ID = -1;
const static int MAX_PACKET_SIZE = 255;
const static int MAX_BUFF_SIZE = 255;

#pragma comment (lib, "ws2_32.lib")

#include "protocol_2022.h"

HANDLE g_hiocp;

enum OPTYPE{ OP_SEND, OP_RECV, OP_DO_MOVE };

high_resolution_clock::time_point last_connect_time;

struct OverlappedEx{
	WSAOVERLAPPED over;
	WSABUF wsabuf;
	unsigned char IOCP_buf[MAX_BUFF_SIZE];
	OPTYPE event_type;
	int event_target;
};

struct CLIENT{
	int id;
	int x;
	int y;
	atomic_bool connected = true;

	SOCKET client_socket = NULL;
	OverlappedEx recv_over;
	unsigned char packet_buf[MAX_PACKET_SIZE] = { 0 };
	int prev_packet_data = 0;
	int curr_packet_size = 0;
	high_resolution_clock::time_point last_move_time;
	high_resolution_clock::time_point last_delay_time;
	volatile bool m_isSendAbleDelayCheck = true;
	int prevLatencyTime = 0;

	CLIENT() : last_move_time(high_resolution_clock::now()), last_delay_time(high_resolution_clock::now()){}
};

array<int, MAX_CLIENTS> client_map;
array<CLIENT, MAX_CLIENTS + MAX_NPC> g_clients;
atomic_int num_connections;
atomic_int client_to_close;
atomic_int active_clients;

atomic_int			global_delay;				// ms단위, 1000이 넘으면 클라이언트 증가 종료
volatile uint64_t			avg_delay;				// 평균 딜레이

vector <thread *> worker_threads;
thread test_thread;

float point_cloud[MAX_TEST * 2];

// 나중에 NPC까지 추가 확장 용
struct ALIEN{
	int id;
	int x, y;
	int visible_count;
};

void error_display(const char * msg, int err_no){
	WCHAR * lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, err_no,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf, 0, NULL);
	std::cout << msg;
	std::wcout << L"에러" << lpMsgBuf << std::endl;

	//MessageBox(hWnd, lpMsgBuf, L"ERROR", 0);
	LocalFree(lpMsgBuf);
	// while (true);
}

void DisconnectClient(int ci){
	bool status = true;
	if(true == atomic_compare_exchange_strong(&g_clients[ci].connected, &status, false)){
		closesocket(g_clients[ci].client_socket);
		//global_delay -= g_clients[ci].prevLatencyTime;
		active_clients--;
	}
	// cout << "Client [" << ci << "] Disconnected!\n";
}

void SendPacket(int cl, void * packet){
	int psize = reinterpret_cast<unsigned char *>( packet )[0];
	int ptype = reinterpret_cast<unsigned char *>( packet )[1];
	OverlappedEx * over = new OverlappedEx;
	over->event_type = OP_SEND;
	memcpy(over->IOCP_buf, packet, psize);
	ZeroMemory(&over->over, sizeof(over->over));
	over->wsabuf.buf = reinterpret_cast<CHAR *>( over->IOCP_buf );
	over->wsabuf.len = psize;
	int ret = WSASend(g_clients[cl].client_socket, &over->wsabuf, 1, NULL, 0,
										&over->over, NULL);
	if(0 != ret){
		int err_no = WSAGetLastError();
		if(WSA_IO_PENDING != err_no)
			error_display("Error in SendPacket:", err_no);
	}
	// std::cout << "Send Packet [" << ptype << "] To Client : " << cl << std::endl;
}

void ProcessPacket(int ci, unsigned char packet[]){
	switch(packet[1]){
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET * move_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET *>( packet );
		if(move_packet->id < MAX_CLIENTS){
			int my_id = client_map[move_packet->id];
			if(-1 != my_id){
				g_clients[my_id].x = move_packet->x;
				g_clients[my_id].y = move_packet->y;
			}
			if(ci == my_id){
				if(0 != move_packet->move_time){
					if(!g_clients[ci].connected.load())return;
					/*duration_cast<milliseconds>(g_clients[ci].last_move_time.time_since_epoch()).count();

					duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();*/
					//auto d_ms = duration_cast<milliseconds>( high_resolution_clock::now().time_since_epoch() ).count() - duration_cast<milliseconds>( g_clients[ci].last_move_time.time_since_epoch() ).count();
					////d_ms /= 2;
					////if (ci == 5) {
					////	cout << "d_ms: " << d_ms << endl;
					////	cout << "prevLat: " << g_clients[ci].prevLatencyTime << endl;
					////}
					//global_delay += ( d_ms - g_clients[ci].prevLatencyTime );
					//if(global_delay < 0)global_delay = 0;
					///*if (global_delay < d_ms)
					//	global_delay++;
					//else if (global_delay > d_ms)
					//	global_delay--;*/
					//g_clients[ci].prevLatencyTime = d_ms;
				}
			}
		}
	}
	break;
	case SC_ADD_OBJECT: break;
	case SC_REMOVE_OBJECT: break;
	case SC_LOGIN_INFO:
	{
		g_clients[ci].connected = true;
		active_clients++;
		SC_LOGIN_INFO_PACKET * login_packet = reinterpret_cast<SC_LOGIN_INFO_PACKET *>( packet );
		int my_id = ci;
		client_map[login_packet->id] = my_id;
		g_clients[my_id].id = login_packet->id;
		g_clients[my_id].x = login_packet->x;
		g_clients[my_id].y = login_packet->y;

		//cs_packet_teleport t_packet;
		//t_packet.size = sizeof(t_packet);
		//t_packet.type = CS_TELEPORT;
		//SendPacket(my_id, &t_packet);
	}
	break;
	case SC_CHAT:
	case SC_STAT_CHANGE:
		break;
	case SC_DELAY:
	{
		if(g_clients[ci].connected){
			auto d_ms = duration_cast<milliseconds>( high_resolution_clock::now() - g_clients[ci].last_delay_time ).count();
							//d_ms /= 2;
							//if (ci == 5) {
							//	cout << "d_ms: " << d_ms << endl;
							//	cout << "prevLat: " << g_clients[ci].prevLatencyTime << endl;
							//}
			auto & currentMetric = Metric::GetInstance().GetCurrentMetric();
			currentMetric.totalDelayCnt++;
			currentMetric.totalDelayTime += d_ms;
			//global_delay += ( d_ms - g_clients[ci].prevLatencyTime );
			//if(global_delay < 0)global_delay = 0;
			/*if (global_delay < d_ms)
				global_delay++;
			else if (global_delay > d_ms)
				global_delay--;*/
			g_clients[ci].prevLatencyTime = d_ms;
			g_clients[ci].m_isSendAbleDelayCheck = true;
		}
	}
	break;
	default: MessageBox(hWnd, L"Unknown Packet Type", L"ERROR", 0);
		while(true);
	}
}

void Worker_Thread(){
	while(true){
		DWORD io_size;
		unsigned long long ci;
		OverlappedEx * over;
		BOOL ret = GetQueuedCompletionStatus(g_hiocp, &io_size, &ci,
																				 reinterpret_cast<LPWSAOVERLAPPED *>( &over ), INFINITE);
																			 // std::cout << "GQCS :";
		int client_id = static_cast<int>( ci );
		if(FALSE == ret){
			int err_no = WSAGetLastError();
			if(64 == err_no) DisconnectClient(client_id);
			else{
				// error_display("GQCS : ", WSAGetLastError());
				DisconnectClient(client_id);
			}
			if(OP_SEND == over->event_type) delete over;
		}
		if(0 == io_size){
			DisconnectClient(client_id);
			continue;
		}
		if(OP_RECV == over->event_type){
			//std::cout << "RECV from Client :" << ci;
			//std::cout << "  IO_SIZE : " << io_size << std::endl;
			unsigned char * buf = g_clients[ci].recv_over.IOCP_buf;
			unsigned psize = g_clients[ci].curr_packet_size;
			unsigned pr_size = g_clients[ci].prev_packet_data;
			while(io_size > 0){
				if(0 == psize) psize = buf[0];
				if(io_size + pr_size >= psize){
					// 지금 패킷 완성 가능
					unsigned char packet[MAX_PACKET_SIZE];
					memcpy(packet, g_clients[ci].packet_buf, pr_size);
					memcpy(packet + pr_size, buf, psize - pr_size);
					ProcessPacket(static_cast<int>( ci ), packet);
					io_size -= psize - pr_size;
					buf += psize - pr_size;
					psize = 0; pr_size = 0;
				} else{
					memcpy(g_clients[ci].packet_buf + pr_size, buf, io_size);
					pr_size += io_size;
					io_size = 0;
				}
			}
			g_clients[ci].curr_packet_size = psize;
			g_clients[ci].prev_packet_data = pr_size;
			DWORD recv_flag = 0;
			int ret = WSARecv(g_clients[ci].client_socket,
												&g_clients[ci].recv_over.wsabuf, 1,
												NULL, &recv_flag, &g_clients[ci].recv_over.over, NULL);
			if(SOCKET_ERROR == ret){
				int err_no = WSAGetLastError();
				if(err_no != WSA_IO_PENDING){
					//error_display("RECV ERROR", err_no);
					DisconnectClient(client_id);
				}
			}
		} else if(OP_SEND == over->event_type){
			if(io_size != over->wsabuf.len){
				// std::cout << "Send Incomplete Error!\n";
				DisconnectClient(client_id);
			}
			delete over;
		} else if(OP_DO_MOVE == over->event_type){
			// Not Implemented Yet
			delete over;
		} else{
			std::cout << "Unknown GQCS event!\n";
			while(true);
		}
	}
}

constexpr int DELAY_LIMIT = 50;
constexpr int DELAY_LIMIT2 = 150;
constexpr int CONN_DELAY = 50;

void Adjust_Number_Of_Client(){
	static int delay_multiplier = 1;
	static int max_limit = MAXINT;
	static bool increasing = true;

	static high_resolution_clock::time_point metricTime = high_resolution_clock::now();

	auto nowTime = high_resolution_clock::now();
	if(metricTime + 500ms < nowTime){
		auto & currentMetric = Metric::GetInstance().SwapAndLoad();
		auto totalDelay = currentMetric.totalDelayTime.load();
		auto delayCnt = currentMetric.totalDelayCnt.load();
		if(delayCnt != 0){
			avg_delay = totalDelay / delayCnt;
		}
		metricTime = nowTime;
	}

	auto connDuration = duration_cast<milliseconds>(nowTime - last_connect_time).count();

	if(CONN_DELAY * delay_multiplier > connDuration){
		return;
	}

	int activeClients = active_clients;

	if(DELAY_LIMIT2 < avg_delay){//평균 딜레이가 150초과
		if(true == increasing){
			max_limit = activeClients;
			increasing = false;
		}
		if(100 > activeClients) return;

		if(CONN_DELAY * 3 > connDuration){
			return;
		}

		last_connect_time = nowTime;
		DisconnectClient(client_to_close);
		client_to_close++;
		return;
	} else{
		if(DELAY_LIMIT < avg_delay){//100ms초과의 딜레이라면
			delay_multiplier = 10;//커넥트 속도 줄이기
			return;
		}
	}
	if(max_limit * 0.8 < activeClients){//아까 전체의 20%덜어내기 전에는 conn못하게
		return;
	}

	increasing = true;
	last_connect_time = nowTime;

	if(activeClients >= MAX_TEST) return;
	if(num_connections >= MAX_CLIENTS) return;

	g_clients[num_connections].client_socket = WSASocketW(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	SOCKADDR_IN ServerAddr;
	ZeroMemory(&ServerAddr, sizeof(SOCKADDR_IN));
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(PORT_NUM);
	ServerAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


	int Result = WSAConnect(g_clients[num_connections].client_socket, (sockaddr *)&ServerAddr, sizeof(ServerAddr), NULL, NULL, NULL, NULL);
	if(0 != Result){
		error_display("WSAConnect : ", GetLastError());
	} else{
		g_clients[num_connections].curr_packet_size = 0;
		g_clients[num_connections].prev_packet_data = 0;
		ZeroMemory(&g_clients[num_connections].recv_over, sizeof(g_clients[num_connections].recv_over));
		g_clients[num_connections].recv_over.event_type = OP_RECV;
		g_clients[num_connections].recv_over.wsabuf.buf =
			reinterpret_cast<CHAR *>( g_clients[num_connections].recv_over.IOCP_buf );
		g_clients[num_connections].recv_over.wsabuf.len = sizeof(g_clients[num_connections].recv_over.IOCP_buf);

		DWORD recv_flag = 0;
		CreateIoCompletionPort(reinterpret_cast<HANDLE>( g_clients[num_connections].client_socket ), g_hiocp, num_connections, 0);

		CS_LOGIN_PACKET l_packet;

		int temp = num_connections;
		sprintf_s(l_packet.loginId, "test_%d", temp);
		l_packet.size = sizeof(l_packet);
		l_packet.type = CS_LOGIN;
		SendPacket(num_connections, &l_packet);
		int ret = WSARecv(g_clients[num_connections].client_socket, &g_clients[num_connections].recv_over.wsabuf, 1,
											NULL, &recv_flag, &g_clients[num_connections].recv_over.over, NULL);
		if(SOCKET_ERROR == ret){
			int err_no = WSAGetLastError();
			if(err_no != WSA_IO_PENDING){
				error_display("RECV ERROR", err_no);
				goto fail_to_connect;
			}
		}
		num_connections++;
	}


fail_to_connect:
	return;
}

void Test_Thread(){
	while(true){
		//Sleep(max(20, global_delay));
		Adjust_Number_Of_Client();

		for(int i = 0; i < num_connections; ++i){
			if(false == g_clients[i].connected) continue;
			auto nowTime = high_resolution_clock::now();
			if(g_clients[i].last_move_time + 1s < nowTime){
				CS_MOVE_PACKET my_packet;
				my_packet.size = sizeof(my_packet);
				my_packet.type = CS_MOVE;
				switch(rand() % 4){
				case 0: my_packet.direction = 0; break;
				case 1: my_packet.direction = 1; break;
				case 2: my_packet.direction = 2; break;
				case 3: my_packet.direction = 3; break;
				}
				my_packet.move_time = static_cast<unsigned>(duration_cast<milliseconds>(nowTime.time_since_epoch()).count());
				SendPacket(i, &my_packet);
				g_clients[i].last_move_time = nowTime;
			}
			if(g_clients[i].m_isSendAbleDelayCheck && g_clients[i].last_delay_time + 2s < nowTime){
				g_clients[i].m_isSendAbleDelayCheck = false;
				CS_DELAY_PACKET delayPacket;
				delayPacket.size = sizeof(CS_DELAY_PACKET);
				delayPacket.type = CS_DELAY;
				g_clients[i].last_delay_time = nowTime;
				SendPacket(i, &delayPacket);
			}

		}
	}
}

void InitializeNetwork(){
	for(auto & cl : g_clients){
		cl.connected = false;
		cl.id = INVALID_ID;
	}

	for(auto & cl : client_map) cl = -1;
	num_connections = 0;
	last_connect_time = high_resolution_clock::now();

	WSADATA	wsadata;
	WSAStartup(MAKEWORD(2, 2), &wsadata);

	g_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, NULL, 0);

	for(int i = 0; i < 6; ++i)
		worker_threads.push_back(new std::thread{ Worker_Thread });

	test_thread = thread{ Test_Thread };
}

void ShutdownNetwork(){
	test_thread.join();
	for(auto pth : worker_threads){
		pth->join();
		delete pth;
	}
}

void Do_Network(){
	return;
}

void GetPointCloud(int * size, float ** points){
	int index = 0;
	for(int i = 0; i < num_connections; ++i)
		if(true == g_clients[i].connected){
			point_cloud[index * 2] = static_cast<float>(g_clients[i].x);
			point_cloud[index * 2 + 1] = static_cast<float>(g_clients[i].y);
			index++;
		}

	*size = index;
	*points = point_cloud;
}

