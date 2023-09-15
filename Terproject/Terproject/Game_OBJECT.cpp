#include "Game_OBJECT.h"
#include "SESSION.h"


void Game_OBJECT::Interaction(SESSION& client) // is interaction OBJ
{
	if (!isInteraction) {
		return;
	}

	//interaction Do
	return;
}

bool Game_OBJECT::Collide(int x, int y)
{
	if (x == this->x && y == this->y) return true;
	return false;
}
