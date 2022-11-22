// SQLBindCol_ref.cpp  
// compile with: odbc32.lib  
#include <windows.h>  
#include <iostream>  

#include <sqlext.h>  

#define NAME_LEN 50  


void show_error() {
    printf("error\n");
}

int main() {
    SQLHENV henv;
    SQLHDBC hdbc;
    SQLHSTMT hstmt = 0; // sql����� �����ϰ� �ڵ��� �̿��ؼ� �����ϱ� ���� �ڵ�
    SQLRETURN retcode;
    SQLWCHAR szName[NAME_LEN];
    SQLINTEGER szLevel, szId;
    SQLLEN cbName = 0, cbLevel= 0, cbId = 0;

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
                retcode = SQLConnect(hdbc, (SQLWCHAR*)L"gameServer", SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);

                // Allocate statement handle  
                if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                    retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);

                    //retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"SELECT Name, User_ID, [Level] FROM Table_1", SQL_NTS);
                    retcode = SQLExecDirect(hstmt, (SQLWCHAR*)L"EXEC select_user_over 10", SQL_NTS);

                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {

                        // Bind columns 1, 2, and 3  
                        /*retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, &szName, NAME_LEN, &cbName);
                        retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &szId, 4, &cbId);
                        retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &szLevel, 4, &cbLevel);*/

                        retcode = SQLBindCol(hstmt, 1, SQL_C_WCHAR, szName, NAME_LEN, &cbName);
                        retcode = SQLBindCol(hstmt, 2, SQL_C_LONG, &szId, 4, &cbId);
                        retcode = SQLBindCol(hstmt, 3, SQL_C_LONG, &szLevel, 4, &cbLevel);

                        // Fetch and print each row of data. On an error, display a message and exit.  
                        for (int i = 0; ; i++) {
                            retcode = SQLFetch(hstmt);
                            if (retcode == SQL_ERROR || retcode == SQL_SUCCESS_WITH_INFO)
                                show_error();
                            if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO)
                            {
                                //replace wprintf with printf
                                //%S with %ls
                                //warning C4477: 'wprintf' : format string '%S' requires an argument of type 'char *'
                                //but variadic argument 2 has type 'SQLWCHAR *'
                                //wprintf(L"%d: %S %S %d\n", i + 1, szLevel, szName, szId);
                                printf("%d: %d %ls %d\n", i + 1, szLevel, szName, szId);
                                //std::cout << szName << " " << szId << " " << szLevel << std::endl;
                                //printf("%d: %ls %ls %ls\n", i + 1, sCustID, szName, szPhone);
                            }
                            else
                                break;
                        }
                    }
                    else {
                        if (retcode == SQL_ERROR) {
                            std::cout << "error" << std::endl;
                        }
                    }
                    // Process data  
                    if (retcode == SQL_SUCCESS || retcode == SQL_SUCCESS_WITH_INFO) {
                        SQLCancel(hstmt);
                        SQLFreeHandle(SQL_HANDLE_STMT, hstmt);
                    }

                    SQLDisconnect(hdbc);
                }

                SQLFreeHandle(SQL_HANDLE_DBC, hdbc);
            }
        }
        SQLFreeHandle(SQL_HANDLE_ENV, henv);
    }
}