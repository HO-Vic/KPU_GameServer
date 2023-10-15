#include "stdafx.h"
#include "GameObject.h"

GameObject::GameObject()
{
	m_id = -1;
	m_hp = 0;
	m_maxHp = 0;
	m_attackDamage = 0;
	m_position = make_pair(0, 0);
	m_lastAttackTime = chrono::system_clock::now();
	m_state = ST_PLAYER_FREE;
	ZeroMemory(prevPacketData, 512);
}

GameObject::GameObject(int id) : m_id(id)
{
	m_hp = 0;
	m_maxHp = 0;
	m_attackDamage = 0;
	m_position = make_pair(0, 0);
	m_lastAttackTime = chrono::system_clock::now();
	m_state = ST_PLAYER_FREE;
	ZeroMemory(prevPacketData, 512);
}

GameObject::~GameObject() {}

void GameObject::InitSetting(int id, short hp, short mHp, short damage, short exp, pair<short, short>& pos)
{
	SetId(id);
	SetHp(hp);
	SetMaxHp(mHp);
	SetAttackDamage(damage);
	SetPosition(pos);
	SetExp(exp);
	m_lastAttackTime = chrono::system_clock::now();
}

void GameObject::InitSetting(int id, short hp, short mHp, short damage, short exp)
{
	SetId(id);
	SetHp(hp);
	SetMaxHp(mHp);
	SetAttackDamage(damage);
	SetExp(exp);
	m_lastAttackTime = chrono::system_clock::now();
}

void GameObject::InitSetting(int id, short hp, short mHp, short damage, short exp, short x, short y)
{
	SetId(id);
	SetHp(hp);
	SetMaxHp(mHp);
	SetAttackDamage(damage);
	SetPosition(x, y);
	SetExp(exp);
	m_lastAttackTime = chrono::system_clock::now();
}

void GameObject::SetId(int id)
{
	m_id = id;
}

void GameObject::SetHp(short hp)
{
	m_hp = hp;
}

void GameObject::SetMaxHp(short mHp)
{
	m_maxHp = mHp;
}

void GameObject::SetAttackDamage(short damage)
{
	m_attackDamage = damage;
}

void GameObject::SetPosition(pair<short, short>& pos)
{
	m_position = pos;
}

void GameObject::SetPosition(short x, short y)
{
	m_position.first = x;
	m_position.second = y;
}

void GameObject::SetName(const char* name)
{
	string nameStr = name;
	wstring wName;
	wName.assign(nameStr.begin(), nameStr.end());
	m_inGameName = wName;
}

void GameObject::SetName(char* name)
{
	string nameStr = name;
	wstring wName;
	wName.assign(nameStr.begin(), nameStr.end());
	m_inGameName = wName;
}

void GameObject::SetName(wchar_t* name)
{
	m_inGameName.clear();
	m_inGameName = name;
}

void GameObject::SetName(const wchar_t* name)
{
	m_inGameName.clear();
	m_inGameName = name;
}

void GameObject::SetName(wstring& name)
{
	m_inGameName.clear();
	m_inGameName = name;
}

void GameObject::SetExp(short exp)
{
	m_exp = exp;
}

void GameObject::SetMaxExp(short mExp)
{
	m_maxExp = mExp;
}

void GameObject::SetLevel(short level)
{
	m_level = level;
}

pair<short, short> GameObject::GetPosition()
{
	return m_position;
}

wstring GameObject::GetName()
{
	return m_inGameName;
}

short GameObject::GetHp()
{
	return m_hp;
}

short GameObject::GetMaxHp()
{
	return m_maxHp;
}

short GameObject::GetAttackDamage()
{
	return m_attackDamage;
}

short GameObject::GetExp()
{
	return m_exp;
}

short GameObject::GetMaxExp()
{
	return m_maxExp;
}

short GameObject::GetLevel()
{
	return m_level;
}

bool GameObject::IsAbleLevelUp()
{
	return m_exp > m_maxExp;
}

void GameObject::LevelUp()
{
	m_level += 1;
}

unordered_set<int> GameObject::GetViewList()
{
	lock_guard<mutex> lg(m_viewListLock);
	return m_viewList;
}

bool GameObject::IsExistViewList(int playerId)
{
	lock_guard<mutex> lg(m_viewListLock);
	return m_viewList.count(playerId) != 0;
}

void GameObject::ClearViewList()
{
	lock_guard<mutex> lg(m_viewListLock);
	m_viewList.clear();
}

short GameObject::AttackedDamage(short damage)
{
	m_hp -= damage;
	if (m_hp <= 0) {
		return m_exp;
	}
	return 0;
}

void GameObject::ResetLastAttack()
{
	m_lastAttackTime = std::chrono::system_clock::now();
}
