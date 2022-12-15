#include "PLAYER.h"


void PLAYER::move(int x, int y)
{
	m_x = x;
	m_y = y;
}

void PLAYER::draw() {
	if (false == m_showing) return;
	float rx = 10 * TILE_WIDTH;
	float ry = 10 * TILE_WIDTH;
	nameText.setPosition(rx, ry - 20);
	m_sprite.setPosition(rx, ry - 5);
	g_window->draw(nameText);
	g_window->draw(m_sprite);

}

void PLAYER::SetNameText()
{
	nameText.setString("");
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
