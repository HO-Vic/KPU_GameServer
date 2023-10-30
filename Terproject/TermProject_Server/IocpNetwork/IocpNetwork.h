#pragma once
#include "../PCH/stdafx.h"

class ExpOver;

class IocpNetwork
{
private:
	HANDLE m_iocpHandle;

	SOCKET m_listenSocket;
	SOCKET m_clientSocket;

	ExpOver* m_acceptExpOver;
	char m_acceptBuffer[BUF_SIZE];
public:
	IocpNetwork();
	~IocpNetwork();
private:
	void ExecuteAccept();
	void InitIocp();
public:
	const HANDLE& GetIocpHandle();
	void Start();
	void WorkerThread();

};
