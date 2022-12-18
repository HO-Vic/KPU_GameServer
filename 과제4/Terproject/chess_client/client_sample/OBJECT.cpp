#include "OBJECT.h"

OBJECT::OBJECT(sf::Texture& t, int x, int y, int x2, int y2)
{
	m_showing = false;
	m_sprite.setTexture(t);
	m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
}

OBJECT::OBJECT() {
	m_showing = false;
}
void OBJECT::show()
{
	m_showing = true;
}
void OBJECT::hide()
{
	m_showing = false;
}

void OBJECT::a_move(int x, int y) {
	m_sprite.setPosition((float)x, (float)y);
}

void OBJECT::a_draw() {
	if(m_showing)
		g_window->draw(m_sprite);
}
