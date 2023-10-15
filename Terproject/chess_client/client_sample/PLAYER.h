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
	chrono::system_clock::time_point skillExecuteTime;
	bool showSkill = false;

public:
	PLAYER() :OBJECT() {}
	PLAYER(sf::Texture& t, int x, int y, int x2, int y2, sf::Texture& messTexture) :OBJECT{ t, x, y, x2 ,y2 } {
		m_messTextSprite.setTexture(messTexture);
		m_messTextSprite.setTextureRect(sf::IntRect(0, 20, 100, 20));
	}

	int m_x, m_y;

	int hp = 0;
	int maxHp = 1;
	int exp = 0;
	int level = 0;
	int id = -1;

	wstring name;
	wstring m_chatBuffer;
	sf::Sprite m_messTextSprite;
	
	//wchar_t name[NAME_SIZE] = { 0 };


	void move(int x, int y);
	void draw();
	void SetNameText(char* name);
	void SetNameText(wstring& name);
	void set_chat(const char str[]);
	void set_chat(const wchar_t str[]);
	bool IsAbleSkill();
	void StartEffect();
	void StartEffect(chrono::system_clock::time_point& t);
	void CheckHideSkillEffect();
	bool GetshowSkill() { return showSkill; };
	void SetPlayerStat(int hp, int maxHp, int exp, int level);
	
	void ClearChatBuffer();
};
