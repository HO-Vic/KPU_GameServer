#pragma once
#include "OBJECT.h"
#include "protocol_2022.h"

class PLAYER : public OBJECT
{
private:	
	sf::Text m_chat;
	sf::Text nameText;
	chrono::system_clock::time_point m_mess_end_time;
public:
	PLAYER() :OBJECT() {}
	PLAYER(sf::Texture& t, int x, int y, int x2, int y2) :OBJECT{ t, x, y, x2 ,y2 } {}

	int m_x, m_y;

	char name[NAME_SIZE] = { 0 };

	void move(int x, int y);
	void draw();
	void SetNameText();
	void set_chat(const char str[]);

};
