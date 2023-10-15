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

void PlayerObject::ClearPlayerObject()
{
	m_hp = 0;
	m_maxHp = 0;
	m_attackDamage = 0;
	m_exp = 0;
	m_inGameName.clear();
	m_loginID.clear();
	m_level = 1;
	m_position = make_pair(0, 0);
	closesocket(m_socket);
	m_recvOver->Clear();
	m_state = ST_PLAYER_FREE;
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
	m_state = ST_INGAME;
}

void PlayerObject::SendLoginFailPacket()
{
	PacketManager::SendLoginFailPacket(m_socket);
}

void PlayerObject::Disconnect()
{
	m_state = ST_PLAYER_ALLOC;
	SaveData();
	auto viewList = GetViewList();
	ClearViewList();
	PacketManager::RemoveDisconnectClient(m_id, viewList);
	ClearPlayerObject();
	//m_hp = 0;
	//m_maxHp = 0;
	//m_attackDamage = 0;
	//m_exp = 0;
	//m_inGameName.clear();
	//m_loginID.clear();
	//m_level = 1;
	//m_position = make_pair(0, 0);
	//closesocket(m_socket);
	//m_recvOver->Clear();
	//m_state = ST_PLAYER_FREE;

	//lock_guard<mutex> stateLock{ m_stateLock };
	//m_state = ST_PLAYER_FREE;
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

void PlayerObject::SendMess(int sendId, wchar_t* mess)
{
	PacketManager::SendMessPacket(m_socket, sendId, mess);
}

short PlayerObject::AttackedDamage(short damage)
{
	m_hp -= damage;
	if (m_hp <= 0) {
		//10, 10À¸·Î ±ÍÈ¯ ½ÃÄÑ¾ßµÊ.

		m_exp = 0;
		return 0;
	}
	return 0;
}

bool PlayerObject::IsAbleAttack()
{
	return m_lastAttackTime + 1s < std::chrono::system_clock::now();
}

void PlayerObject::SendPacket(char* data)
{
	PacketManager::SendPacket(m_socket, data);
}

void PlayerObject::SendSkillExecutePacket(unordered_set<int>& viewList)
{
	PacketManager::SendSkillExecuteTImePacket(m_socket, m_id, viewList, m_lastAttackTime);
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
	return m_state;
}

void PlayerObject::ConsumeExp(short cExp)
{
	m_exp += cExp;
}

void PlayerObject::RegistGameObject(int id, SOCKET& sock)
{
	m_state = ST_PLAYER_ALLOC;
	SetId(id);
	RegistSocket(sock);
	DoRecv();
}

