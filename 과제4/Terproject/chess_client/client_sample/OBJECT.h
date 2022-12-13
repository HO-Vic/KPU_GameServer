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
private:
	bool m_showing;
	sf::Sprite m_sprite;

	sf::Text m_name;
	sf::Text m_chat;
	chrono::system_clock::time_point m_mess_end_time;

public:
	int m_x, m_y;
	//wchar_t name[NAME_SIZE] = {0};
	sf::Text nameText;
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2);
	OBJECT();
	void show();
	void hide();
	void a_move(int x, int y);
	void a_draw();
	void move(int x, int y);
	void draw();
	void set_chat(const char str[]);
};

