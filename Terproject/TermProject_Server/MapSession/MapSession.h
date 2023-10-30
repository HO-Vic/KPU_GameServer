#pragma once
#include "../PCH/stdafx.h"

class MapSession
{
private:
	std::pair<int, int> m_pos;
	std::unordered_set<int> m_playerSet;
	std::mutex m_playerSetLock;
	std::array<std::pair<short, short>, 4> m_collision;
public:
	MapSession();
	MapSession(int posX, int posY);
	MapSession(std::pair<int, int> pos);
	MapSession(MapSession& rhs);
	~MapSession();
public:
	void InsertPlayer(int playerId);
	void DeletePlayer(int playerId);

	std::unordered_set<int> GetPlayer();
	bool CollisionObject(std::pair<short, short>& position);
	bool CollisionObject(short x, short y);
public:
	void SetPos(int x, int y);
	void SetPos(std::pair<int, int> pos);
};

