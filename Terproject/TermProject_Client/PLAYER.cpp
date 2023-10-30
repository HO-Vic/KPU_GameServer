#include "PLAYER.h"
#include <iostream>
using namespace std;
using namespace chrono;

extern bool g_isEnterPressed;
extern int g_myid;
void PLAYER::move(int x, int y)
{
	m_x = x;
	m_y = y;
	float rx = m_x * TILE_WIDTH - g_left_x;
	float ry = m_y * TILE_WIDTH - g_top_y;
}

void PLAYER::draw() {
	//g_left_x = myPlayer.m_x * TILE_WIDTH - TILE_WIDTH * 10;

	if (false == m_showing) return;
	;
	float rx = m_x * TILE_WIDTH - g_left_x;
	float ry = m_y * TILE_WIDTH - g_top_y;
	nameText.setPosition(rx, ry - 20);
	m_sprite.setPosition(rx, ry - 5);
	m_chat.setPosition(rx, ry - 40);
	m_messTextSprite.setPosition(rx, ry - 40);

	bool owner = g_myid == id;
	owner = owner && g_isEnterPressed;

	if (owner || m_mess_end_time + 2s > chrono::system_clock::now()) {
		g_window->draw(m_messTextSprite);
		g_window->draw(m_chat);
	}
	g_window->draw(nameText);
	g_window->draw(m_sprite);
}

void PLAYER::SetNameText(char* name)
{
	nameText.setString(name);
	nameText.setFont(font);
	nameText.setCharacterSize(20);
	nameText.setPosition(0, 0);
	nameText.setFillColor(sf::Color::Magenta);
	nameText.setOutlineColor(sf::Color::Magenta);
	nameText.setOutlineThickness(1.f);
}

void PLAYER::SetNameText(wstring& name)
{
	nameText.setString(name);
	nameText.setFont(font);
	nameText.setCharacterSize(20);
	nameText.setPosition(0, 0);
	nameText.setFillColor(sf::Color::Magenta);
	nameText.setOutlineColor(sf::Color::Magenta);
	nameText.setOutlineThickness(1.f);
}

void PLAYER::set_chat(const char str[])
{
	m_chat.setFont(font);
	m_chat.setString(str);
	m_chat.setFillColor(sf::Color(255, 255, 255));
	m_chat.setStyle(sf::Text::Bold);
	m_mess_end_time = chrono::system_clock::now();
}

void PLAYER::set_chat(const wchar_t str[])
{
	m_chatBuffer = str;
	m_messTextSprite.setTextureRect(sf::IntRect(0, 20, 10 + m_chatBuffer.size() * 10, 20));
	m_chat.setFont(font);
	m_chat.setString(m_chatBuffer);
	m_chat.setFillColor(sf::Color(255, 255, 255));
	m_chat.setStyle(sf::Text::Bold);
	m_chat.setColor(sf::Color::Black);
	m_chat.setCharacterSize(15);

	m_mess_end_time = chrono::system_clock::now();
}

bool PLAYER::IsAbleSkill()
{
	return skillExecuteTime + 1s < chrono::system_clock::now();
}

void PLAYER::StartEffect()
{
	skillEffectTime = skillExecuteTime = chrono::system_clock::now();
	showSkill = true;
}

void PLAYER::StartEffect(chrono::system_clock::time_point& t)
{
	skillEffectTime = skillExecuteTime = t;
	showSkill = true;
}

void PLAYER::CheckHideSkillEffect()
{
	if (skillEffectTime + 600ms < chrono::system_clock::now()) {
		showSkill = false;
	}
}

void PLAYER::SetPlayerStat(int hp, int maxHp, int exp, int level)
{
	this->hp = hp;
	this->maxHp = maxHp;
	this->exp = exp;
	this->level = level;
}

void PLAYER::ClearChatBuffer()
{
	m_chat.setString("");
}
