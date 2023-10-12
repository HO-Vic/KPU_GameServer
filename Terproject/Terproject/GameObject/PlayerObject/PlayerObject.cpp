#include "stdafx.h"
#include "PlayerObject.h"
#include "../../Packet/PacketManager.h"
#include "../../ExpOver/ExpOver.h"
#include "../../DB/DB_OBJ.h"

extern DB_OBJ g_DB;

PlayerObject::PlayerObject() :GameObject()
{
	m_recvOver = new RecvExpOverBuffer();
}

PlayerObject::PlayerObject(int id) :GameObject(id)
{
	m_recvOver = new RecvExpOverBuffer();
}

PlayerObject::~PlayerObject()
{
	delete m_recvOver;
}

void PlayerObject::RegistSocket(SOCKET& sock)
{
	m_socket = sock;
}

void PlayerObject::SetLoginId(char* loginId)
{
	string str{ loginId };
	m_loginID.assign(str.begin(), str.end());
}

void PlayerObject::SetLoginId(wchar_t* loginId)
{
	m_loginID = loginId;
}

wstring PlayerObject::GetLoginId()
{
	return m_loginID;
}

void PlayerObject::DoRecv()
{
	m_recvOver->DoRecv(m_socket);
}

void PlayerObject::RecvPacket(int ioByte)
{
	m_recvOver->RecvPacket(m_id, ioByte);
	DoRecv();
}

void PlayerObject::SendLoginInfoPacket()
{
	PacketManager::SendLoginPacket(m_socket, m_inGameName, m_id, m_position, m_hp, m_maxHp, m_level, m_exp);//name ºüÁü.
	lock_guard<mutex> stateLock{ m_stateLock};
	m_state = ST_INGAME;
}

void PlayerObject::RemoveViewListPlayer(int removePlayerId)
{
	m_viewListLock.lock();
	int existElement = m_viewList.erase(removePlayerId);//exist ret 1, not exist ret 0
	m_viewListLock.unlock();
	if (!existElement) return;
	PacketManager::SendRemoveObjectPacket(m_socket, removePlayerId);
}

void PlayerObject::MovePlayer(int movePlayerId)
{
	PacketManager::SendMoveObjectPacket(m_socket, movePlayerId);
}

void PlayerObject::AddViewListPlayer(int addPlayerId)
{
	m_viewListLock.lock();
	m_viewList.insert(addPlayerId);
	m_viewListLock.unlock();
	PacketManager::SendAddObjectPacket(m_socket, addPlayerId);
}

short PlayerObject::AttackedDamage(short damage)
{
	m_hp -= damage;
	if (m_hp <= 0) {
		//10, 10À¸·Î ±ÍÈ¯ ½ÃÄÑ¾ßµÊ.

		m_exp = 0;
		return m_exp;
	}
	return 0;
}

bool PlayerObject::IsAbleAttack()
{
	auto t = duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - m_lastAttackTime);
	if (t.count() > 1000)return true;
	return false;
}

void PlayerObject::SendStatPacket(char* data)
{
	PacketManager::SendPacket(m_socket, data);
}

void PlayerObject::SaveData()
{
	DB::DB_PlayerInfo dbData{m_id, m_loginID, m_inGameName,
		m_position.first, m_position.second,
		m_level, m_exp, m_hp, m_maxHp, m_attackDamage
	};
	DB_Event dbEv{ DB_EVENT_TYPE::EV_SAVE_PLAYER_INFO, m_id, &dbData };
	g_DB.Insert_DBEvent(dbEv);
}

S_STATE PlayerObject::GetPlayerState()
{
	lock_guard<mutex> stateLock{m_stateLock};
	return m_state;
}

void PlayerObject::ConsumeExp(short cExp)
{
	m_exp += cExp;
}

void PlayerObject::RegistGameObject(int id, SOCKET& sock)
{
	{
		lock_guard<mutex> stateLock{m_stateLock};
		m_state = ST_PLAYER_ALLOC;
	}
	SetId(id);
	RegistSocket(sock);
	DoRecv();
}

