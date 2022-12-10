#pragma once
#include<sqlext.h>
#include <string>
using namespace std;
class DB_OBJ
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
public:
	DB_OBJ();
	bool GetPlayerInfo(wstring PlayerLoginId, wstring& outputPlayerName, short& pos_X, short& pos_Y);
	void SetPlayerPosition(wstring PlayerLoginId, short pos_X, short pos_Y);
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
};
