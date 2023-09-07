#pragma once
#include<unordered_set>
#include<mutex>
#include <array>
class PlayerSession;
class Game_OBJECT;
class LOCAL_SESSION
{
private:
	Game_OBJECT* obstacle;
	int x, y;
	int obstacleCount = 0;

public:
	std::unordered_set<int> players;
	std::mutex playersLock;
	//std::lock_guard<std::mutex> lg;
	LOCAL_SESSION();
	LOCAL_SESSION(int posX, int posY);
	LOCAL_SESSION(LOCAL_SESSION& rhs);
	~LOCAL_SESSION();

public:
	void InsertPlayers(PlayerSession& player);
	void UpdatePlayers(PlayerSession& player, std::array< std::array<LOCAL_SESSION, 100>, 100>& maps);
	void DeletePlayers(PlayerSession& player);
	const std::unordered_set<int>& GetPlayer();
	bool CollisionObject(int x, int y);
public:
	void SetPos(int x, int y);
};

