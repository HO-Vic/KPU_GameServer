#include <windows.h>
#include "DB_OBJ.h"
#include "protocol_2022.h"

DB_OBJ::DB_OBJ()
{
	SQLRETURN retcode;
	// Allocate environment handle  
	retcode = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);

	// Set the ODBC version environment attribute  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
		// Allocate connection handle  
		if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
			retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);

			// Set login timeout to 5 seconds  
			if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
				// Connect to data source  
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"2018184010", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
				}
			}
		}
	}
}

bool DB_OBJ::GetPlayerInfo(wstring PlayerLoginId, wstring& outputPlayerName, short& pos_X, short& pos_Y)
{
	SQLRETURN retcode;

	SQLHSTMT hstmt;

	SQLWCHAR szName[NAME_SIZE] = { 0 };
	SQLLEN cbName = 0;

	SQLINTEGER szPos_X = 0;
	SQLLEN cbPos_X = 0;

	SQLINTEGER szPos_Y = 0;
	SQLLEN cbPos_Y = 0;

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);


	wstring oper = L"EXEC select_user_info ";
	oper.append(PlayerLoginId);
	oper.append(L"\0");
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)oper.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, szName, NAME_SIZE, &cbName);
		retcode = SQLBindCol(hstmt, 2, SQL_C_SHORT, &szPos_X, 2, &cbPos_X);
		retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &szPos_Y, 2, &cbPos_Y);

		// Fetch and print each row of data. On an error, display a message and exit.
		retcode = SQLFetch(hstmt); // 다음 행일 가져와라 명령어
		if (retcode == SQL_ERROR/* || retcode == SQL_SUCCESS_WITH_INFO*/) {
			HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
			return false;
		}
		////show_error();
		else if (retcode == SQL_SUCCESS)
		{
			outputPlayerName.append(szName);
			pos_X = szPos_X;
			pos_Y = szPos_Y;
		}
		if (retcode == SQL_SUCCESS_WITH_INFO) {
			outputPlayerName.append(szName);
			pos_X = szPos_X;
			pos_Y = szPos_Y;
			HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		}
	}
	// Process data  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);///종료
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);//리소스 해제
	}
	return true;
	//disconnet
	//SQLDisconnect(hdbc);
}

void DB_OBJ::SetPlayerPosition(wstring PlayerLoginId, short pos_X, short pos_Y)
{
	SQLRETURN retcode;

	SQLHSTMT hstmt;

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);


	wstring oper = L"EXEC set_user_position ";
	oper.append(PlayerLoginId);
	oper.append(L", ");
	oper.append(to_wstring(pos_X));
	oper.append(L", ");
	oper.append(to_wstring(pos_Y));

	oper.append(L"\0");
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)oper.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

		// Fetch and print each row of data. On an error, display a message and exit.
		retcode = SQLFetch(hstmt); // 다음 행일 가져와라 명령어
		if (retcode == SQL_ERROR/* || retcode == SQL_SUCCESS_WITH_INFO*/) {
			HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		}
		////show_error();
		else if (retcode == SQL_SUCCESS)
		{

		}
		if (retcode == SQL_SUCCESS_WITH_INFO) {

			HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		}
	}
	// Process data  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);///종료
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);//리소스 해제
	}
}

void DB_OBJ::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) {
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError;
	
	WCHAR       wszState[SQL_SQLSTATE_SIZE + 1];
	WCHAR       wszMessage[1000];

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage,
		(SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated.. 		
		if (wcsncmp(wszState, L"01004", 5))
		{
			//fwprintf(stdout, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
		//cout << wszState << " " << (WCHAR*)wszMessage << " " << iError << endl;
	}
}
