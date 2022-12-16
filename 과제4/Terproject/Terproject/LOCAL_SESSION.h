#pragma once
#include<unordered_set>
#include<mutex>
#include <array>
class SESSION;
class Game_OBJECT;
class LOCAL_SESSION
{
private:
	Game_OBJECT* obstacle;
	int x, y;
	int obstacleCount = 0;	

	std::unordered_set<int> players;
	std::mutex playersLock;
public:
	LOCAL_SESSION();
	LOCAL_SESSION(int posX, int posY);
	LOCAL_SESSION(LOCAL_SESSION& rhs);
	~LOCAL_SESSION();
	
public:
	void InsertPlayers(SESSION& player);
	void UpdatePlayers(SESSION& player, std::array< std::array<LOCAL_SESSION, 100>, 100>& maps);
	const std::unordered_set<int>& GetPlayer();
	bool CollisionObject(int x, int y);
public:
	void SetPos(int x, int y);
};

