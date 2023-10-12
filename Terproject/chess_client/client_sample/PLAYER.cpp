#include "PLAYER.h"


void PLAYER::move(int x, int y)
{
	m_x = x;
	m_y = y;
}

void PLAYER::draw() {
	//g_left_x = myPlayer.m_x * TILE_WIDTH - TILE_WIDTH * 10;

	if (false == m_showing) return;
	;
	float rx = m_x * TILE_WIDTH - g_left_x;
	float ry = m_y * TILE_WIDTH - g_top_y;
	nameText.setPosition(rx, ry - 20);
	m_sprite.setPosition(rx, ry - 5);
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
	m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
}

void PLAYER::set_chat(const wchar_t str[])
{
	m_chat.setFont(font);
	m_chat.setString(str);
	m_chat.setFillColor(sf::Color(255, 255, 255));
	m_chat.setStyle(sf::Text::Bold);
	m_mess_end_time = chrono::system_clock::now() + chrono::seconds(3);
}

void PLAYER::StartEffect(chrono::system_clock::time_point t)
{
	skillEffectTime = t;
	showSkill = true;
}

chrono::system_clock::time_point PLAYER::GetSkillEffectTime()
{
	return skillEffectTime;
}

void PLAYER::SetPlayerStat(int hp, int maxHp, int exp, int level)
{
	this -> hp = hp;
	this -> maxHp = maxHp;
	this -> exp = exp;
	this -> level = level;
}
