#include "stdafx.h"
#include "MapSession.h"
#include "../GameObject/GameObject.h"
#include "../Logic/Logic.h"

extern std::array<std::pair<short, short>, 31> g_mapObstacle;
extern std::array<std::pair<short, short>, 29> g_vilageObstacle;

MapSession::MapSession()
{
	m_pos.first = 0;
	m_pos.second = 0;
}

MapSession::MapSession(int posX, int posY)
{
	m_pos.first = posX;
	m_pos.second = posY;
	if (m_pos.first < 1 && m_pos.second < 1) {
		m_collision[0] = make_pair(9, 8);
		m_collision[1] = make_pair(9, 9);
		m_collision[2] = make_pair(10, 8);
		m_collision[3] = make_pair(10, 9);
	}
}

MapSession::MapSession(std::pair<int, int> pos)
{
	m_pos = pos;
	if (m_pos.first < 1 && m_pos.second < 1) {
		m_collision[0] = make_pair(9, 8);
		m_collision[1] = make_pair(9, 9);
		m_collision[2] = make_pair(10, 8);
		m_collision[3] = make_pair(10, 9);
	}
}

MapSession::MapSession(MapSession& rhs)
{
	m_pos = rhs.m_pos;
	if (m_pos.first < 1 && m_pos.second < 1) {
		m_collision[0] = make_pair(9, 8);
		m_collision[1] = make_pair(9, 9);
		m_collision[2] = make_pair(10, 8);
		m_collision[3] = make_pair(10, 9);
	}
}

MapSession::~MapSession()
{
}

void MapSession::InsertPlayer(int playerId)
{
	std::lock_guard<std::mutex> lg{ m_playerSetLock };
	m_playerSet.insert(playerId);
}

void MapSession::DeletePlayer(int playerId)
{
	std::lock_guard<std::mutex> lg{ m_playerSetLock };
	m_playerSet.erase(playerId);
}

std::unordered_set<int> MapSession::GetPlayer()
{
	std::lock_guard<std::mutex> lg{ m_playerSetLock };
	return m_playerSet;
}

bool MapSession::CollisionObject(std::pair<short, short>& position)
{
	std::pair<short, short> pos = make_pair(position.first % 20, position.second % 20);
	if (m_pos.first < 1 && m_pos.second < 1) {
		auto findIter = find(m_collision.begin(), m_collision.end(), pos);
		if (findIter != m_collision.end())return true;

		auto findCollisionIter = find(g_vilageObstacle.begin(), g_vilageObstacle.end(), pos);
		if (findCollisionIter == g_vilageObstacle.end())
			return false;
		return true;
	}
	auto findIter = find(g_mapObstacle.begin(), g_mapObstacle.end(), pos);
	if (findIter == g_mapObstacle.end())
		return false;
	return true;
}

bool MapSession::CollisionObject(short x, short y)
{
	//if (m_pos.first < 3 || m_pos.second < 3) return false;
	std::pair<short, short> pos = make_pair(x % 20, y % 20);
	if (m_pos.first < 1 && m_pos.second < 1) {
		auto findIter = find(m_collision.begin(), m_collision.end(), pos);
		if (findIter != m_collision.end())return true;

		auto findCollisionIter = find(g_vilageObstacle.begin(), g_vilageObstacle.end(), pos);
		if (findCollisionIter == g_vilageObstacle.end())
			return false;
		return true;
	}
	auto findIter = find(g_mapObstacle.begin(), g_mapObstacle.end(), pos);
	if (findIter == g_mapObstacle.end())
		return false;
	return true;
}

void MapSession::SetPos(int x, int y)
{
	m_pos.first = x;
	m_pos.second = y;
	if (m_pos.first < 1 && m_pos.second < 1) {
		m_collision[0] = make_pair(9, 8);
		m_collision[1] = make_pair(9, 9);
		m_collision[2] = make_pair(10, 8);
		m_collision[3] = make_pair(10, 9);
	}
}

void MapSession::SetPos(std::pair<int, int> pos)
{
	m_pos = pos;
	if (m_pos.first < 1 && m_pos.second < 1) {
		m_collision[0] = make_pair(9, 8);
		m_collision[1] = make_pair(9, 9);
		m_collision[2] = make_pair(10, 8);
		m_collision[3] = make_pair(10, 9);
	}
}
