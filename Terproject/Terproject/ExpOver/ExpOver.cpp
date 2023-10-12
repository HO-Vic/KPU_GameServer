#include "stdafx.h"
#include "ExpOver.h"
#include "../Packet/PacketManager.h"

void RecvExpOverBuffer::DoRecv(SOCKET& socket)
{
	DWORD recv_flag = 0;
	ZeroMemory(&m_overlapped, 0, sizeof(WSAOVERLAPPED));
	m_wsaBuf.len = BUF_SIZE - m_remainData;
	m_wsaBuf.buf = m_buffer + m_remainData;
	WSARecv(socket, &m_wsaBuf, 1, 0, &recv_flag, &m_overlapped, 0);
}

void RecvExpOverBuffer::RecvPacket(int id, int ioByte)
{
	m_remainData = PacketManager::ProccessPacket(id, ioByte, m_remainData, m_buffer);
}

void ExpOverWsaBuffer::DoSend(SOCKET& socket)
{
	WSASend(socket, &m_wsaBuf, 1, 0, 0, &m_overlapped, 0);
}
