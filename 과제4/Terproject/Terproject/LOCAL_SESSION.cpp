#include <array>
#include "LOCAL_SESSION.h"
#include "Game_OBJECT.h"
#include "SESSION.h"
LOCAL_SESSION::LOCAL_SESSION()
{
	obstacleCount = 31;
	obstacle = new  Game_OBJECT[obstacleCount];
	obstacle[0] = Game_OBJECT(make_pair(5, 1));
	obstacle[1] = Game_OBJECT(make_pair(6, 1));
	obstacle[2] = Game_OBJECT(make_pair(7, 1));
	obstacle[3] = Game_OBJECT(make_pair(2, 3));
	obstacle[4] = Game_OBJECT(make_pair(5, 3));
	obstacle[5] = Game_OBJECT(make_pair(14, 3));
	obstacle[6] = Game_OBJECT(make_pair(17, 4));
	obstacle[7] = Game_OBJECT(make_pair(2, 6));
	obstacle[8] = Game_OBJECT(make_pair(10, 6));
	obstacle[9] = Game_OBJECT(make_pair(6, 8));
	obstacle[10] = Game_OBJECT(make_pair(7, 8));
	obstacle[11] = Game_OBJECT(make_pair(8, 8));
	obstacle[12] = Game_OBJECT(make_pair(9, 8));
	obstacle[13] = Game_OBJECT(make_pair(10, 8));
	obstacle[14] = Game_OBJECT(make_pair(13, 8));
	obstacle[15] = Game_OBJECT(make_pair(6, 9));
	obstacle[16] = Game_OBJECT(make_pair(7, 9));
	obstacle[17] = Game_OBJECT(make_pair(8, 9));
	obstacle[18] = Game_OBJECT(make_pair(9, 9));
	obstacle[19] = Game_OBJECT(make_pair(10, 9));
	obstacle[20] = Game_OBJECT(make_pair(17, 10));
	obstacle[21] = Game_OBJECT(make_pair(2, 11));
	obstacle[22] = Game_OBJECT(make_pair(15, 13));
	obstacle[23] = Game_OBJECT(make_pair(13, 14));
	obstacle[24] = Game_OBJECT(make_pair(2, 15));
	obstacle[25] = Game_OBJECT(make_pair(6, 15));
	obstacle[26] = Game_OBJECT(make_pair(16, 15));
	obstacle[27] = Game_OBJECT(make_pair(16, 16));
	obstacle[28] = Game_OBJECT(make_pair(7, 17));
	obstacle[29] = Game_OBJECT(make_pair(8, 17));
	obstacle[30] = Game_OBJECT(make_pair(16, 17));


}

LOCAL_SESSION::LOCAL_SESSION(int posX, int posY) : x(posX), y(posY)
{
	if (posX < 3 || posY < 3) {
		obstacle = nullptr;
		return;
	}
}

LOCAL_SESSION::LOCAL_SESSION(LOCAL_SESSION& rhs)
{
	obstacle = rhs.obstacle;
	rhs.obstacle = nullptr;
	x = rhs.x;
	y = rhs.y;
	obstacleCount = rhs.obstacleCount;
}

LOCAL_SESSION::~LOCAL_SESSION()
{
	if (obstacle != nullptr)
		delete[] obstacle;

}

void LOCAL_SESSION::InsertPlayers(SESSION& player)
{
	{
		std::lock_guard<std::mutex> pl{ playersLock };
		players.emplace(player._id);
	}
}

void LOCAL_SESSION::UpdatePlayers(SESSION& player, std::array< std::array<LOCAL_SESSION, 100>, 100>& maps)
{
	if (20 * x > player.x || 20 * x + 19 < player.x) {
		{
			std::lock_guard<std::mutex> pl{ playersLock };
			if (players.count(player._id))
				players.erase(player._id);
		}
		if (player.x < 20 * x) {
			maps[x - 1][y].InsertPlayers(player);
			player.myLocalSectionIndex = std::make_pair(x - 1, y);
			return;
		}
		maps[x + 1][y].InsertPlayers(player);
		player.myLocalSectionIndex = std::make_pair(x + 1, y);
	}
	else if (20 * y > player.y || 20 * y + 19 < player.y) {
		{
			std::lock_guard<std::mutex> pl{ playersLock };
			if (players.count(player._id))
				players.erase(player._id);
		}
		if (player.y < 20 * y) {
			maps[x][y - 1].InsertPlayers(player);
			player.myLocalSectionIndex = std::make_pair(x, y - 1);
			return;
		}
		else maps[x][y + 1].InsertPlayers(player);
		player.myLocalSectionIndex = std::make_pair(x, y + 1);
	}
}

void LOCAL_SESSION::DeletePlayers(SESSION& player)
{
	std::lock_guard<std::mutex> pl{ playersLock };
	if (players.count(player._id))
		players.erase(player._id);
}

const std::unordered_set<int>& LOCAL_SESSION::GetPlayer()
{
	return players;
}

bool LOCAL_SESSION::CollisionObject(int x, int y)
{
	for (int i = 0; i < obstacleCount; i++) {
		if (obstacle[i].Collide(x % 20, y % 20))
			return true;
	}
	return false;
}

void LOCAL_SESSION::SetPos(int x, int y)
{
	this->x = x;
	this->y = y;
}
