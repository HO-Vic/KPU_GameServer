#pragma once
#define SFML_STATIC 1

#include<chrono>
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>

#ifdef _DEBUG
#pragma comment (lib, "lib/sfml-graphics-s-d.lib")
#pragma comment (lib, "lib/sfml-window-s-d.lib")
#pragma comment (lib, "lib/sfml-system-s-d.lib")
#pragma comment (lib, "lib/sfml-network-s-d.lib")
#else
#pragma comment (lib, "lib/sfml-graphics-s.lib")
#pragma comment (lib, "lib/sfml-window-s.lib")
#pragma comment (lib, "lib/sfml-system-s.lib")
#pragma comment (lib, "lib/sfml-network-s.lib")
#endif
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "winmm.lib")
#pragma comment (lib, "ws2_32.lib")

using namespace std;

extern sf::Font font;
extern sf::RenderWindow* g_window;
extern int g_left_x;
extern int g_top_y;

constexpr auto TILE_WIDTH = 50;

class OBJECT {
protected:
	sf::Sprite m_sprite;
public:
	bool m_showing;
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2);
	OBJECT();
	void show();
	void hide();
	void a_move(int x, int y);
	virtual void a_draw();

	void SetScale(float x, float y) //체력 바 퍼센트
	{
		//m_sprite.setTextureRect(sf::IntRect(0, 0, 1, 30));
		m_sprite.setScale(1, 1);
		m_sprite.scale(x, y);
	}
};

extern int g_left_x;
extern int g_top_y;