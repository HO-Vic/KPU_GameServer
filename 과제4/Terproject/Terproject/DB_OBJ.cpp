#include <windows.h>
#include <iostream>
#include "DB_OBJ.h"
#include "protocol_2022.h"

int print_error(SQLHENV    henv,
	SQLHDBC    hdbc,
	SQLHSTMT   hstmt)
{
	SQLWCHAR     buffer[SQL_MAX_MESSAGE_LENGTH + 1];
	SQLWCHAR     sqlstate[SQL_SQLSTATE_SIZE + 1];
	SQLINTEGER  sqlcode;
	SQLSMALLINT length;


	while (SQLError(henv, hdbc, hstmt, sqlstate, &sqlcode, buffer,
		SQL_MAX_MESSAGE_LENGTH + 1, &length) == SQL_SUCCESS)
	{
		cout << "\n **** ERROR *****\n";
		cout << "         SQLSTATE: " << sqlstate << endl;
		cout << "Native Error Code: " << sqlcode << endl;
		cout << "%s" << buffer << endl;
	};
	return (0);

}

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
				retcode = SQLConnect(hdbc, (SQLWCHAR*)L"TermProject2018184010", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
				if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
					//cout << "DB Success" << endl;
				}
			}
		}
	}	
}

bool DB_OBJ::GetPlayerInfo(wstring PlayerLoginId, wstring& outputPlayerName, short& pos_X, short& pos_Y, short& level, short& Exp, short& hp, short& maxHp, short& attackDamage)
{
	SQLRETURN retcode;

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (retcode == SQL_ERROR) {
		print_error(henv, hdbc, hstmt);
	}

	SQLWCHAR szName[NAME_SIZE] = { 0 };
	SQLLEN cbName = 0;

	SQLINTEGER szPos_X = 0;
	SQLLEN cbPos_X = 0;

	SQLINTEGER szPos_Y = 0;
	SQLLEN cbPos_Y = 0;

	SQLINTEGER szLevel = 0;
	SQLLEN cbLevel = 0;

	SQLINTEGER szExp = 0;
	SQLLEN cbExp = 0;

	SQLINTEGER szHp = 0;
	SQLLEN cbHp = 0;

	SQLINTEGER szMaxHp = 0;
	SQLLEN cbMaxHp = 0;

	SQLINTEGER szAttackDamage = 0;
	SQLLEN cbAttackDamage = 0;

	wstring oper = L"EXEC select_user_Info ";
	oper.append(PlayerLoginId);
	oper.append(L"\0");
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)oper.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, szName, NAME_SIZE, &cbName);
		retcode = SQLBindCol(hstmt, 2, SQL_C_SHORT, &szPos_X, SQL_INTEGER, &cbPos_X);
		retcode = SQLBindCol(hstmt, 3, SQL_C_SHORT, &szPos_Y, SQL_INTEGER, &cbPos_Y);
		retcode = SQLBindCol(hstmt, 4, SQL_C_SHORT, &szLevel, SQL_INTEGER, &cbLevel);
		retcode = SQLBindCol(hstmt, 5, SQL_C_SHORT, &szExp, SQL_INTEGER, &cbExp);
		retcode = SQLBindCol(hstmt, 6, SQL_C_SHORT, &szHp, SQL_INTEGER, &cbHp);
		retcode = SQLBindCol(hstmt, 7, SQL_C_SHORT, &szMaxHp, SQL_INTEGER, &cbMaxHp);
		retcode = SQLBindCol(hstmt, 8, SQL_C_SHORT, &szAttackDamage, SQL_INTEGER, &cbAttackDamage);

		// Fetch and print each row of data. On an error, display a message and exit.
		retcode = SQLFetch(hstmt); // ���� ���� �����Ͷ� ��ɾ�
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
			level = szLevel;
			Exp = szExp;
			hp = szHp;
			maxHp = szMaxHp;
			attackDamage = szAttackDamage;
		}
		if (retcode == SQL_SUCCESS_WITH_INFO) {
			outputPlayerName.append(szName);
			pos_X = szPos_X;
			pos_Y = szPos_Y;
			level = szLevel;
			Exp = szExp;
			hp = szHp;
			maxHp = szMaxHp;
			attackDamage = szAttackDamage;
			HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, retcode);
		}
	}
	if (retcode == SQL_ERROR) {
		print_error(henv, hdbc, hstmt);
	}

	// Process data  
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(hstmt);///����
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);//���ҽ� ����
	}
	return true;
}

void DB_OBJ::SavePlayerInfo(wstring PlayerLoginId, short& pos_X, short& pos_Y, short& level, short& Exp, short& hp, short& maxHp, short& attackDamage)
{
	SQLRETURN retcode;

	SQLHSTMT hstmt;

	// Allocate statement handle  
	retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);


	wstring oper = L"EXEC save_user_info ";
	oper.append(PlayerLoginId);
	oper.append(L", ");
	oper.append(to_wstring(pos_X));
	oper.append(L", ");
	oper.append(to_wstring(pos_Y));
	oper.append(L", ");
	oper.append(to_wstring(level));
	oper.append(L", ");
	oper.append(to_wstring(Exp));
	oper.append(L", ");
	oper.append(to_wstring(hp));
	oper.append(L", ");
	oper.append(to_wstring(maxHp));
	oper.append(L", ");
	oper.append(to_wstring(attackDamage));
	oper.append(L"\0");
	retcode = SQLExecDirect(hstmt, (SQLWCHAR*)oper.c_str(), SQL_NTS);
	if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

		// Fetch and print each row of data. On an error, display a message and exit.
		retcode = SQLFetch(hstmt); // ���� ���� �����Ͷ� ��ɾ�
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
		SQLCancel(hstmt);///����
		SQLFreeHandle(SQL_HANDLE_STMT, hstmt);//���ҽ� ����
	}
}

void DB_OBJ::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode) {
	SQLSMALLINT iRec = 0;
	SQLINTEGER  iError = RetCode;

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
			fwprintf(stdout, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
		cout << wszState << " " << (WCHAR*)wszMessage << " " << iError << endl;
	}
}
