#pragma once
#include<sqlext.h>
#include <string>
using namespace std;
class DB_OBJ
{
private:
	SQLHENV henv;
	SQLHDBC hdbc;
	SQLHSTMT hstmt;
public:
	DB_OBJ();
	~DB_OBJ()
	{
		// Process data  

		SQLCancel(hstmt);///종료
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);//리소스 해제

		//disconnet
		SQLDisconnect(hdbc);

	}
	bool GetPlayerInfo(wstring PlayerLoginId, wstring& outputPlayerName, short& pos_X, short& pos_Y, short& level, short& Exp, short& hp);
	void SetPlayerPosition(wstring PlayerLoginId, short pos_X, short pos_Y);
	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);
};
