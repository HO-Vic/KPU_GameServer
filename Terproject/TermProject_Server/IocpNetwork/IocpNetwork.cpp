#include "stdafx.h"
#include <chrono>
#include "IocpNetwork.h"
#include "../ExpOver/ExpOver.h"
#include "../GameObject/PlayerObject/PlayerObject.h"
#include "../GameObject/NPC_Object/NPC_OBJECT.h"
#include "../GameObject/GameObject.h"
#include "../MapSession/MapSession.h"
#include "../Packet/PacketManager.h"
#include "../Logic/Logic.h"
#include "../DB/DB_Event.h"
#include "../Metric/Metric.h"

extern array<GameObject *, MAX_USER + MAX_NPC> g_clients;
extern array < array<MapSession, 100>, 100> g_gameMap;
extern array<int, 11> g_levelExp;
extern std::array<int, 11> g_levelMaxHp;
extern std::array<int, 11> g_levelAttackDamage;


IocpNetwork::IocpNetwork(){
	m_acceptExpOver = new ExpOver(OP_CODE::OP_ACCEPT);
	ZeroMemory(m_acceptBuffer, BUF_SIZE);
	InitIocp();
}

IocpNetwork::~IocpNetwork(){
	delete m_acceptExpOver;
}

void IocpNetwork::ExecuteAccept(){
	m_acceptExpOver->ResetOverlapped();
	int addr_size = sizeof(SOCKADDR_IN);
	m_clientSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	AcceptEx(m_listenSocket, m_clientSocket, m_acceptBuffer, 0, addr_size + 16, addr_size + 16, 0, reinterpret_cast<WSAOVERLAPPED *>( m_acceptExpOver ));
}

void IocpNetwork::InitIocp(){
	WSADATA WSAData;
	if(WSAStartup(MAKEWORD(2, 2), &WSAData) != 0){
		cerr << "wsaStartUp Error\n";
		WSACleanup();
		exit(-1);
	}
}

void IocpNetwork::Start(){
	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT_NUM);
	serverAddr.sin_addr.S_un.S_addr = INADDR_ANY;

	m_listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

	bind(m_listenSocket, reinterpret_cast<sockaddr *>( &serverAddr ), sizeof(serverAddr));
	listen(m_listenSocket, SOMAXCONN);

	m_iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	CreateIoCompletionPort(reinterpret_cast<HANDLE>( m_listenSocket ), m_iocpHandle, 9999, 0);
	ExecuteAccept();

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();

	for(int i = 0; i < num_threads; ++i)
		worker_threads.emplace_back([&](){WorkerThread(); });

	using namespace std::chrono;
	steady_clock::time_point prevTime = steady_clock::now();
	while(true){
		steady_clock::time_point currentTime = steady_clock::now();
		if(duration_cast<milliseconds>(currentTime - prevTime).count() < 1000){//1초에 한번씩 로깅할 수 있게
			continue;
		}
		prevTime = currentTime;

		auto & metric = Metric::GetInstance().SwapAndLoad();
		auto total = metric.totalGridElapsedTime.load();
		auto cnt = metric.totalProcessCnt.load();
		uint64_t avgTime = 0;
		if(cnt != 0){
			avgTime = total / cnt;
		}
		auto currentUserCnt = Metric::GetInstance().m_currentUserCnt.load();
		std::cout << "Grid ElpaseTime: " << avgTime;
		std::cout << "(micros)\nCnt: " << cnt;
		std::cout << "\ncurrentUserCnt: " << currentUserCnt << '\n';
	}

	for(auto & th : worker_threads)
		th.join();

	closesocket(m_listenSocket);
	WSACleanup();
}

const HANDLE & IocpNetwork::GetIocpHandle(){
	return m_iocpHandle;
}

void IocpNetwork::WorkerThread(){
	DWORD ioByte;
	ULONG_PTR key;
	volatile WSAOVERLAPPED * over = nullptr;
	while(true){
		BOOL ret = GetQueuedCompletionStatus(m_iocpHandle, &ioByte, &key, (LPOVERLAPPED *)&over, INFINITE);

		ExpOver * exOver = reinterpret_cast<ExpOver *>( (LPOVERLAPPED)over );
		OP_CODE currentOpCode = exOver->GetOpCode();
		if(FALSE == ret){
			if(currentOpCode == OP_ACCEPT){
				//cout << "Accept Error";
			}
			//cout << "GQCS Error on client[" << key << "]\n";
			Logic::DisconnectClient(key);
			ExpOverMgr::DeleteExpOver(exOver);
			over = nullptr;
		} else{
			switch(currentOpCode){
			case OP_ACCEPT:
			{
				int newClientId = Logic::GetNewClientId();
				if(newClientId != -1){
					CreateIoCompletionPort(reinterpret_cast<HANDLE>( m_clientSocket ), m_iocpHandle, newClientId, 0);
					PlayerObject * playerObject = dynamic_cast<PlayerObject *>( g_clients[newClientId] );
					playerObject->RegistGameObject(newClientId, m_clientSocket);
					ExecuteAccept();
				} else{
					closesocket(m_clientSocket);
					ExecuteAccept();
					//cout << "Max user exceeded.\n";
				}
			}
			break;
			case OP_RECV:
			{
				dynamic_cast<PlayerObject *>( g_clients[key] )->RecvPacket(ioByte);
			}
			break;
			case OP_SEND:
			{
				ExpOverMgr::DeleteExpOver(exOver);
			}
			break;
			case OP_NPC_MOVE:
			{
				Logic::NPCMove(key);
				ExpOverMgr::DeleteExpOver(exOver);
			}
			break;
			case OP_NPC_CHASE_MOVE:
			{
				ExpOverBuffer * bufferOver = reinterpret_cast<ExpOverBuffer *>( exOver );
				int * targetId = reinterpret_cast<int *>( bufferOver->GetBufferData() );

				Logic::NPCMove(key, *targetId);
				ExpOverMgr::DeleteExpOver(exOver);
			}
			break;
			case OP_DB_GET_PLAYER_INFO:
			{
				char * dbData = reinterpret_cast<ExpOverBuffer *>( exOver )->GetBufferData();
				Logic::PlayerIngameState(dbData);
				Metric::GetInstance().m_currentUserCnt++;
				ExpOverMgr::DeleteExpOver(exOver);
			}
			break;

			case OP_DB_AUTO_SAVE_PLAYER:
			{
				Logic::AutoSaveAllPlayers();
				ExpOverMgr::DeleteExpOver(exOver);
			}
			break;
			case OP_PLAYER_LOGIN_FAIL:
			{
				dynamic_cast<PlayerObject *>( g_clients[key] )->SendLoginFailPacket();
				ExpOverMgr::DeleteExpOver(exOver);
			}
			break;

			case OP_NPC_RESPAWN:
			{
				Logic::RespawnNPC(key);
				ExpOverMgr::DeleteExpOver(exOver);
			}
			break;
			}
		}
	}
}

