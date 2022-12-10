#include <array>
#include "LOCAL_SESSION.h"
#include "Game_OBJECT.h"
#include "SESSION.h"
LOCAL_SESSION::LOCAL_SESSION()
{
	/*stone = nullptr;
	tree = nullptr;*/
	stoneCount = 3;
	treeCount = 3;
	stone = new  Game_OBJECT[stoneCount];
	tree = new  Game_OBJECT[treeCount];
	for (int i = 0; i < stoneCount; i++) {
		stone[i] = Game_OBJECT(5, 3);
		stone[i] = Game_OBJECT(12, 6);
		stone[i] = Game_OBJECT(8, 7);
	}

	for (int i = 0; i < treeCount; i++) {
		tree[i] = Game_OBJECT(5, 3);
		tree[i] = Game_OBJECT(12, 6);
		tree[i] = Game_OBJECT(8, 7);
	}
}

LOCAL_SESSION::LOCAL_SESSION(int posX, int posY) : x(posX), y(posY)
{
	if (posX < 3 || posY < 3) {
		stone = nullptr;
		tree = nullptr;
		return;
	}
}

LOCAL_SESSION::LOCAL_SESSION(LOCAL_SESSION& rhs)
{
	stone = rhs.stone;
	tree = rhs.tree;
	rhs.stone = nullptr;
	rhs.tree = nullptr;
	x = rhs.x;
	y = rhs.y;
	treeCount = rhs.treeCount;
	stoneCount = rhs.stoneCount;
}

LOCAL_SESSION::~LOCAL_SESSION()
{
	if (stone != nullptr)
		delete[] stone;
	if (tree != nullptr)
		delete[] tree;
}

void LOCAL_SESSION::InsertPlayers(SESSION& player)
{
	playersLock.lock();
	players.try_emplace(player._id, player._id);
	playersLock.unlock();
}

void LOCAL_SESSION::UpdatePlayers(SESSION& player, std::array< std::array<LOCAL_SESSION, 100>, 100>& maps)
{
	if (20 * x > player.x || 20 * x + 19 < player.x) {
		playersLock.lock();
		if(players.count(player._id))
			players.erase(player._id);
		playersLock.unlock();
		if (player.x < 20 * x) {
			maps[x - 1][y].InsertPlayers(player);
			player.myLocalSectionIndex = std::make_pair(x - 1, y);
			return;
		}
		maps[x + 1][y].InsertPlayers(player);
		player.myLocalSectionIndex = std::make_pair(x + 1, y);
	}
	else if (20 * y > player.y || 20 * y + 19 < player.y) {
		playersLock.lock();
		if (players.count(player._id))
			players.erase(player._id);
		playersLock.unlock();
		if (player.y < 20 * y) {
			maps[x][y - 1].InsertPlayers(player);
			player.myLocalSectionIndex = std::make_pair(x, y - 1);
			return;
		}
		else maps[x][y + 1].InsertPlayers(player);
		player.myLocalSectionIndex = std::make_pair(x, y + 1);
	}
}

void LOCAL_SESSION::SetPos(int x, int y)
{
	this->x = x;
	this->y = y;
}
