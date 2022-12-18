#pragma once
#include "OBJECT.h"
#include "protocol_2022.h"

class PLAYER : public OBJECT
{
private:	
	sf::Text m_chat;
	sf::Text nameText;
	chrono::system_clock::time_point m_mess_end_time;

	chrono::system_clock::time_point skillEffectTime;
	bool showSkill = false;

public:
	PLAYER() :OBJECT() {}
	PLAYER(sf::Texture& t, int x, int y, int x2, int y2) :OBJECT{ t, x, y, x2 ,y2 } {}

	int m_x, m_y;

	int hp = 0;
	int maxHp = 0;
	int exp = 0;
	int level = 0;

	char name[NAME_SIZE] = { 0 };

	bool ableSkill = true;


	void move(int x, int y);
	void draw();
	void SetNameText(char* name);
	void set_chat(const char str[]);
	void StartEffect(chrono::system_clock::time_point t);
	chrono::system_clock::time_point GetSkillEffectTime();
	void HidSkill() { showSkill = false; }
	bool GetshowSkill() { return showSkill; };
	void SetPlayerStat(int hp, int maxHp, int exp, int level);
};
