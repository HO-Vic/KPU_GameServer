#pragma once
#include "../PCH/stdafx.h"
#include "DB_Event.h"

using namespace std;

class DB_OBJ
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
private:
	concurrency::concurrent_queue<DB_Event> m_eventQueue;

	atomic_bool m_isRunning = false;
	std::thread m_dbThread;
private:
	void DB_ThreadFunc();
public:
	DB_OBJ();
	~DB_OBJ();

private:	
	bool GetPlayerInfo(int playerId, wchar_t* PlayerLoginId);
	void SavePlayerInfo(wstring PlayerLoginId, short& pos_X, short& pos_Y, short& level, short& Exp, short& hp, short& maxHp, short& attackDamage);
	void AddUser(wstring PlayerLoginId);
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
public:
	void Insert_DBEvent(DB_Event& event);
};
