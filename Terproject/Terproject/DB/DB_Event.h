#pragma once
#include "../PCH/stdafx.h"

using namespace std;
using namespace chrono;

namespace DB {
	struct DB_PlayerId
	{
		int m_ownerId;
		wchar_t m_playerId[NAME_SIZE];

		DB_PlayerId(int ownerId, char* const playerId) : m_ownerId(ownerId)
		{
			string id{ playerId };
			wstring wId;
			wId.assign(id.begin(), id.end());
			wcscpy_s(m_playerId, wId.c_str());
		}
		DB_PlayerId(int ownerId, wchar_t* const playerId) : m_ownerId(ownerId)
		{
			wcscpy_s(m_playerId, playerId);
		}
		DB_PlayerId(int ownerId, string& playerId) : m_ownerId(ownerId)
		{
			wstring wId;
			wId.assign(playerId.begin(), playerId.end());
			wcscpy_s(m_playerId, wId.c_str());
		}
		DB_PlayerId(int ownerId, wstring& playerId) : m_ownerId(ownerId)
		{
			wcscpy_s(m_playerId, playerId.c_str());
		}
	};

	struct DB_PlayerInfo
	{
		int m_id;
		wchar_t m_playerId[NAME_SIZE];
		wchar_t m_playerName[NAME_SIZE];//Ingame Name
		short m_posX;
		short m_posY;
		short m_level;
		short m_exp;
		short m_hp;
		short m_maxHp;
		short m_attackDamage;

		DB_PlayerInfo(int id, wchar_t* const playerId, wchar_t* const playerName, short pos_X, short pos_Y, short level, short exp, short hp, short maxHp, short attackDamage)
			:m_id(id), m_posX(pos_X), m_posY(pos_Y), m_level(level), m_exp(exp), m_hp(hp), m_maxHp(maxHp), m_attackDamage(attackDamage)
		{
			wcscpy_s(m_playerId, playerId);
			wcscpy_s(m_playerName, playerName);
		}

		DB_PlayerInfo(int id, wstring& playerId, wchar_t* const playerName, short pos_X, short pos_Y, short level, short exp, short hp, short maxHp, short attackDamage)
			:m_id(id), m_posX(pos_X), m_posY(pos_Y), m_level(level), m_exp(exp), m_hp(hp), m_maxHp(maxHp), m_attackDamage(attackDamage)
		{
			wcscpy_s(m_playerId, playerId.c_str());
			wcscpy_s(m_playerName, playerName);
		}

		DB_PlayerInfo(int id, wstring& playerId, wstring& playerName, short pos_X, short pos_Y, short level, short exp, short hp, short maxHp, short attackDamage)
			:m_id(id), m_posX(pos_X), m_posY(pos_Y), m_level(level), m_exp(exp), m_hp(hp), m_maxHp(maxHp), m_attackDamage(attackDamage)
		{
			wcscpy_s(m_playerId, playerId.c_str());
			wcscpy_s(m_playerName, playerName.c_str());
		}

	};
};

class DB_Event {
private:
	DB_EVENT_TYPE m_type;
	int m_ownerId;
	char m_buffer[512];
public:
	DB_Event();
	DB_Event(DB_EVENT_TYPE type);
	DB_Event(DB_EVENT_TYPE type, int ownerId);
	DB_Event(DB_EVENT_TYPE type, int ownerId, void* buffer);
public:
	DB_Event(const DB_Event& other);
	DB_Event(DB_Event&& other);
	DB_Event& operator=(DB_Event& other);
	DB_Event& operator=(DB_Event&& other);
public:
	const DB_EVENT_TYPE& GetEvent()
	{
		return m_type;
	}
	const int& GetOwnerId()
	{
		return m_ownerId;
	}
	char* GetBuffer()
	{
		return m_buffer;
	}
};