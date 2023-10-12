#pragma once
#include "../PCH/stdafx.h"
using namespace std;
using namespace chrono;

class GameObject {
public:
	GameObject();
	GameObject(int id);
	virtual ~GameObject();
protected://Ingame Data
	//player - data base load, NPC Initialize data load
	int					m_id = 0;
	short				m_hp = 0;
	short				m_maxHp = 0;
	short				m_attackDamage = 0;
	pair<short, short>	m_position;
	wstring				m_inGameName;
	short				m_exp = 0;//player는 경험치, npc는 줄 경험치
	short				m_maxExp = 0;
	short				m_level = 0;
protected://Ingame Data
	unordered_set <int>	m_viewList;
	mutex				m_viewListLock;
protected:
	S_STATE				m_state;
protected:
	system_clock::time_point m_lastAttackTime = chrono::system_clock::now();
public:
	void InitSetting(int id, short hp, short mHp, short damage, short exp);
	void InitSetting(int id, short hp, short mHp, short damage, short exp, short x, short y);
	void InitSetting(int id, short hp, short mHp, short damage, short exp, pair<short, short>& pos);
public:
	void SetId(int id);

	void SetHp(short hp);
	void SetMaxHp(short mHp);
	void SetAttackDamage(short damage);

	void SetPosition(pair<short, short>& pos);
	void SetPosition(short x, short y);

	void SetName(const char* name);
	void SetName(char* name);
	void SetName(wchar_t* name);
	void SetName(const wchar_t* name);
	void SetName(wstring& name);

	void SetExp(short exp);
	void SetMaxExp(short mExp);

	void SetLevel(short level);
public:
	pair<short, short> GetPosition();
	wstring GetName();
	short GetHp();
	short GetMaxHp();
	short GetAttackDamage();
	short GetExp();
	short GetMaxExp();
	short GetLevel();
	virtual S_STATE GetPlayerState() = 0;
	bool IsAbleLevelUp();
	void LevelUp();
public:
	virtual void RemoveViewListPlayer(int removePlayerId) = 0;
	virtual void MovePlayer(int movePlayerId) = 0;
	virtual void AddViewListPlayer(int addPlayerId) = 0;
public:
	unordered_set<int> GetViewList();
	bool IsExistViewList(int playerId);
	void ClearViewList();
public:
	virtual short AttackedDamage(short damage);
	void ResetLastAttack();
	virtual bool IsAbleAttack() = 0;
};
