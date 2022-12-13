#pragma once

class SESSION;
class Game_OBJECT
{
private:
	int x, y;
	bool isInteraction = false;
public:
	Game_OBJECT() {}
	Game_OBJECT(int x, int y, bool interaction = false) : x(x), y(y), isInteraction(interaction) {}

	void Interaction(SESSION& client);
};
