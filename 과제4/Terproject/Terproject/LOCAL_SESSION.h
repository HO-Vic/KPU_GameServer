#pragma once
#include<unordered_map>
#include<mutex>
class SESSION;
class Game_OBJECT;
class LOCAL_SESSION
{
private:
	Game_OBJECT* stone;
	Game_OBJECT* tree;
	int x, y;
	int treeCount = 0;
	int stoneCount = 0;

	std::unordered_map<int, int> players;
	std::mutex playersLock;
public:
	LOCAL_SESSION();
	LOCAL_SESSION(int posX, int posY);
	LOCAL_SESSION(LOCAL_SESSION& rhs);
	~LOCAL_SESSION();
	
public:
	void InsertPlayers(SESSION& player);
	void UpdatePlayers(SESSION& player, std::array< std::array<LOCAL_SESSION, 100>, 100>& maps);
public:
	void SetPos(int x, int y);
};

