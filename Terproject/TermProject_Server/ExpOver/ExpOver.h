#pragma once
#include "../PCH/stdafx.h"
class ExpOver
{
protected:
	WSAOVERLAPPED m_overlapped;
	OP_CODE m_opCode;
public:
	ExpOver(OP_CODE opCode)
	{
		m_opCode = opCode;
		ZeroMemory(&m_overlapped, sizeof(WSAOVERLAPPED));
	}
	void ResetOverlapped()
	{
		ZeroMemory(&m_overlapped, sizeof(WSAOVERLAPPED));
	}
	void ResetOverlapped(OP_CODE opCode)
	{
		m_opCode = opCode;
		ZeroMemory(&m_overlapped, sizeof(WSAOVERLAPPED));
	}
	OP_CODE GetOpCode()
	{
		return m_opCode;
	}
};

class ExpOverBuffer :public ExpOver
{
protected:
	char m_buffer[BUF_SIZE] = { 0 };
public:
	ExpOverBuffer(OP_CODE opCode) : ExpOver(opCode)
	{
		ZeroMemory(m_buffer, BUF_SIZE);
	}

	ExpOverBuffer(OP_CODE opCode, char* data) : ExpOver(opCode)
	{
		ZeroMemory(m_buffer, BUF_SIZE);
		strcpy_s(m_buffer, data);
	}
	ExpOverBuffer(OP_CODE opCode, char* data, int dataSize) : ExpOver(opCode)
	{
		ZeroMemory(m_buffer, BUF_SIZE);
		memcpy(m_buffer, data, dataSize);
	}
	char* GetBufferData()
	{
		return m_buffer;
	}
};

class ExpOverWsaBuffer : public ExpOverBuffer
{
protected:
	WSABUF m_wsaBuf;
public:
	ExpOverWsaBuffer(OP_CODE opCode) : ExpOverBuffer(opCode)
	{
		ZeroMemory(m_buffer, BUF_SIZE);
		m_wsaBuf.len = BUF_SIZE;
		m_wsaBuf.buf = m_buffer;
	}

	ExpOverWsaBuffer(OP_CODE opCode, char* data) : ExpOverBuffer(opCode)
	{
		ZeroMemory(m_buffer, BUF_SIZE);
		m_wsaBuf.len = (unsigned char)data[0];
		m_wsaBuf.buf = m_buffer;
		memcpy(m_buffer, data, m_wsaBuf.len);
	}
public:
	void DoSend(SOCKET& socket);
};

class RecvExpOverBuffer : public ExpOverWsaBuffer
{
protected:
	int m_remainData;
public:
	RecvExpOverBuffer() : ExpOverWsaBuffer(OP_RECV), m_remainData{ 0 } { }
	RecvExpOverBuffer(char* data) : ExpOverWsaBuffer(OP_RECV), m_remainData{ 0 } { }
public:
	void DoRecv(SOCKET& socket);
	void RecvPacket(int id, int ioByte);
	void Clear();
};

class ExpOverMgr
{
public:
	static ExpOver* CreateExpOver(const OP_CODE&& opCode);
	static ExpOver* CreateExpOverBuffer(const OP_CODE&& opCode, char* data);
	static ExpOver* CreateExpOverBuffer(const OP_CODE&& opCode, char* data, int dataSize);
	static ExpOverWsaBuffer* CreateExpOverWsaBuffer(const OP_CODE&& opCode, char* data);

	static void DeleteExpOver(ExpOver* delExpOver);
};