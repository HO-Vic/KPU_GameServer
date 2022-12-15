#pragma once
#include <utility>
class SESSION;
class Game_OBJECT
{
private:
	int x, y;
	bool isInteraction = false;
public:
	Game_OBJECT() {}
	Game_OBJECT(int x, int y, bool interaction = false) : x(x), y(y), isInteraction(interaction) {}
	Game_OBJECT(std::pair<int, int> pos, bool interaction = false) : x(pos.first), y(pos.second), isInteraction(interaction) {}
	
	void Interaction(SESSION& client);
	bool Collide(int x, int y);
};
