#include "stdafx.h"
#include "ExpOver.h"
#include "../Packet/PacketManager.h"

void RecvExpOverBuffer::DoRecv(SOCKET& socket)
{
	DWORD recv_flag = 0;
	ZeroMemory(&m_overlapped, sizeof(WSAOVERLAPPED));
	m_wsaBuf.len = BUF_SIZE - m_remainData;
	m_wsaBuf.buf = m_buffer + m_remainData;
	WSARecv(socket, &m_wsaBuf, 1, 0, &recv_flag, &m_overlapped, 0);
}

void RecvExpOverBuffer::RecvPacket(int id, int ioByte)
{
	m_remainData = PacketManager::ProccessPacket(id, ioByte, m_remainData, m_buffer);
}

void RecvExpOverBuffer::Clear()
{
	ZeroMemory(&m_overlapped, sizeof(WSAOVERLAPPED));
	m_remainData = 0;
	m_wsaBuf.len = BUF_SIZE;
	m_wsaBuf.buf = m_buffer;
}

void ExpOverWsaBuffer::DoSend(SOCKET& socket)
{
	WSASend(socket, &m_wsaBuf, 1, 0, 0, &m_overlapped, 0);
}

ExpOver* ExpOverMgr::CreateExpOver(const OP_CODE&& opCode)
{
	return new ExpOver(opCode);
}

ExpOver* ExpOverMgr::CreateExpOverBuffer(const OP_CODE&& opCode, char* data)
{
	return new ExpOverBuffer(opCode, data);
}

ExpOver* ExpOverMgr::CreateExpOverBuffer(const OP_CODE&& opCode, char* data, int dataSize)
{
	return new ExpOverBuffer(opCode, data, dataSize);
}

ExpOverWsaBuffer* ExpOverMgr::CreateExpOverWsaBuffer(const OP_CODE&& opCode, char* data)
{
	return new ExpOverWsaBuffer(opCode, data);
}

void ExpOverMgr::DeleteExpOver(ExpOver* delExpOver)
{
	if (delExpOver->GetOpCode() == OP_ACCEPT || delExpOver->GetOpCode() == OP_RECV) return;
	delete delExpOver;
}
