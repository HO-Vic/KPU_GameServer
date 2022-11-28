﻿#define SFML_STATIC 1
#include <SFML/Graphics.hpp>
#include <SFML/Network.hpp>
#include <iostream>
using namespace std;

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

#include "protocol.h"

sf::TcpSocket socket;
sf::Font font;
wstring* g_loginId;
constexpr auto SCREEN_WIDTH = W_WIDTH;
constexpr auto SCREEN_HEIGHT = W_HEIGHT;

constexpr auto TILE_WIDTH = 65;
constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_WIDTH * TILE_WIDTH;
constexpr auto MAX_USER = 1000;

int g_left_x;
int g_top_y;
int g_myid;

sf::RenderWindow* g_window;

class OBJECT {
private:
	bool m_showing;
	sf::Sprite m_sprite;
public:
	int m_x, m_y;
	//wchar_t name[NAME_SIZE] = {0};
	sf::Text nameText;
	OBJECT(sf::Texture& t, int x, int y, int x2, int y2) {
		m_showing = false;
		m_sprite.setTexture(t);
		m_sprite.setTextureRect(sf::IntRect(x, y, x2, y2));
		nameText.setString("");
		nameText.setFont(font);
		nameText.setCharacterSize(20);
		nameText.setPosition(0, 0);
		nameText.setFillColor(sf::Color::Magenta);
		nameText.setOutlineColor(sf::Color::Magenta);
		nameText.setOutlineThickness(1.f);
	}
	OBJECT() {
		m_showing = false;
	}
	void show()
	{
		m_showing = true;
	}
	void hide()
	{
		m_showing = false;
	}

	void a_move(int x, int y) {
		m_sprite.setPosition((float)x, (float)y);
	}

	void a_draw() {
		g_window->draw(m_sprite);
	}

	void move(int x, int y) {
		m_x = x;
		m_y = y;
	}
	void draw() {
		if (false == m_showing) return;
		float rx = (m_x - g_left_x) * 65.0f + 8;
		float ry = (m_y - g_top_y) * 65.0f + 8;
		nameText.setPosition(rx, ry - 20);
		m_sprite.setPosition(rx, ry);
		g_window->draw(nameText);
		g_window->draw(m_sprite);

	}
};

OBJECT avatar;
OBJECT players[MAX_USER];

OBJECT white_tile;
OBJECT black_tile;

sf::Texture* board;
sf::Texture* pieces;

sf::String TextString = "(0, 0)";


void client_initialize()
{
	board = new sf::Texture;
	pieces = new sf::Texture;

	board->loadFromFile("chessmap.bmp");
	pieces->loadFromFile("chess2.png");
	white_tile = OBJECT{ *board, 5, 5, TILE_WIDTH, TILE_WIDTH };
	black_tile = OBJECT{ *board, 69, 5, TILE_WIDTH, TILE_WIDTH };
	avatar = OBJECT{ *pieces, 128, 0, 64, 64 };
	avatar.move(4, 4);
	for (auto& pl : players) {
		pl = OBJECT{ *pieces, 64, 0, 64, 64 };
	}
}

void client_finish()
{
	delete board;
	delete pieces;
}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_FAIL_INFO:
	{
		//g_window->close();
		*g_loginId = L"Login Error";
	}
	break;
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		g_myid = packet->id;
		avatar.m_x = packet->x;
		avatar.m_y = packet->y;
		string str{ packet->name };

		avatar.nameText.setColor(sf::Color::Green);
		avatar.nameText.setOutlineColor(sf::Color::Green);
		avatar.nameText.setString(packet->name);
		char xPos[5];
		char yPos[5];

		_itoa(avatar.m_x, xPos, 10);
		_itoa(avatar.m_y, yPos, 10);
		TextString.clear();
		TextString += "(";
		TextString += xPos;
		TextString += ", ";
		TextString += yPos;
		TextString += ")";

		g_window->close();
		avatar.show();
		break;
	}

	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* my_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - 8;
			g_top_y = my_packet->y - 8;
			//memcpy(avatar.name, my_packet->name, strlen(my_packet->name));
			char xPos[5];
			char yPos[5];

			_itoa(avatar.m_x, xPos, 10);
			_itoa(avatar.m_y, yPos, 10);
			TextString.clear();
			TextString += "(";
			TextString += xPos;
			TextString += ", ";
			TextString += yPos;
			TextString += ")";

			avatar.show();
		}
		else if (id < MAX_USER) {
			players[id].move(my_packet->x, my_packet->y);
			//memcpy(avatar.name, my_packet->name, strlen(my_packet->name));
			players[id].nameText.setString(my_packet->name);
			players[id].show();
		}
		else {
			//npc[id - NPC_START].x = my_packet->x;
			//npc[id - NPC_START].y = my_packet->y;
			//npc[id - NPC_START].attr |= BOB_ATTR_VISIBLE;
		}
		break;
	}
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - 8;
			g_top_y = my_packet->y - 8;

			char xPos[5];
			char yPos[5];

			_itoa(avatar.m_x, xPos, 10);
			_itoa(avatar.m_y, yPos, 10);
			TextString.clear();
			TextString += "(";
			TextString += xPos;
			TextString += ", ";
			TextString += yPos;
			TextString += ")";


		}
		else if (other_id < MAX_USER) {
			players[other_id].move(my_packet->x, my_packet->y);
		}
		else {
			//npc[other_id - NPC_START].x = my_packet->x;
			//npc[other_id - NPC_START].y = my_packet->y;
		}
		break;
	}

	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* my_packet = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			avatar.hide();
		}
		else if (other_id < MAX_USER) {
			players[other_id].hide();
		}
		else {
			//      npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
		}
		break;
	}
	default:
		printf("Unknown PACKET type [%d]\n", ptr[1]);
	}
}

void process_data(char* net_buf, size_t io_byte)
{
	char* ptr = net_buf;
	static size_t in_packet_size = 0;
	static size_t saved_packet_size = 0;
	static char packet_buffer[BUF_SIZE];

	while (0 != io_byte) {
		if (0 == in_packet_size) in_packet_size = ptr[0];
		if (io_byte + saved_packet_size >= in_packet_size) {
			memcpy(packet_buffer + saved_packet_size, ptr, in_packet_size - saved_packet_size);
			ProcessPacket(packet_buffer);
			ptr += in_packet_size - saved_packet_size;
			io_byte -= in_packet_size - saved_packet_size;
			in_packet_size = 0;
			saved_packet_size = 0;
		}
		else {
			memcpy(packet_buffer + saved_packet_size, ptr, io_byte);
			saved_packet_size += io_byte;
			io_byte = 0;
		}
	}
}

void client_main()
{
	char net_buf[BUF_SIZE];
	size_t   received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);

	for (int i = 0; i < SCREEN_WIDTH; ++i)
		for (int j = 0; j < SCREEN_HEIGHT; ++j)
		{
			int tile_x = i + g_left_x;
			int tile_y = j + g_top_y;
			if ((tile_x < 0) || (tile_y < 0)) continue;
			if ((tile_x > 399) || (tile_y > 399)) continue;
			if (((tile_x + tile_y) % 8) < 4) {
				white_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				white_tile.a_draw();
			}
			else
			{
				black_tile.a_move(TILE_WIDTH * i + 7, TILE_WIDTH * j + 7);
				black_tile.a_draw();
			}
		}

	avatar.draw();
	for (auto& pl : players) pl.draw();
}

void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	socket.send(packet, p[0], sent);
}

int main()
{
	wcout.imbue(locale("korean"));
	sf::Socket::Status status = socket.connect("127.0.0.1", PORT_NUM);
	socket.setBlocking(false);

	if (status != sf::Socket::Done) {
		wcout << L"서버와 연결할 수 없습니다.\n";
		while (true);
	}

	client_initialize();

	sf::Text myPosText;

	font.loadFromFile("cour.ttf");
	myPosText.setFont(font);
	myPosText.setCharacterSize(30);
	myPosText.setPosition(0, 0);
	myPosText.setFillColor(sf::Color::Red);
	myPosText.setOutlineColor(sf::Color::Red);
	myPosText.setOutlineThickness(1.f);

	myPosText.setString(TextString);

	sf::RenderWindow loginWindow(sf::VideoMode(300, 100), "Login");
	sf::Texture* whiletBardTexture = new sf::Texture;
	whiletBardTexture->loadFromFile("white.png");
	OBJECT whiteBoard;
	whiteBoard = OBJECT{ *whiletBardTexture, 0, 0, 300, 100 };
	g_window = &loginWindow;

	wstring loginId;
	g_loginId = &loginId;
	sf::Text loginText;
	loginText.setString("ID: ");
	loginText.setFont(font);
	loginText.setCharacterSize(20);
	loginText.setPosition(0, 0);
	loginText.setFillColor(sf::Color::Black);
	loginText.setOutlineColor(sf::Color::Black);
	loginText.setOutlineThickness(1.f);

	while (loginWindow.isOpen()) {
		loginWindow.clear();

		sf::Event event;
		while (loginWindow.pollEvent(event))
		{
			if (event.type == sf::Event::Closed) {
				loginWindow.close();
				return -1;
			}
			else if (event.type == sf::Event::TextEntered) {
				if (std::isprint(event.text.unicode))
					loginId += event.text.unicode;
			}
			else if (event.type == sf::Event::KeyPressed) {
				if (event.key.code == sf::Keyboard::BackSpace) {
					if (!loginId.empty())
						loginId.pop_back();
				}
				if (event.key.code == sf::Keyboard::Return) {
					if (loginId == L"Login Error")
						continue;
					CS_LOGIN_PACKET p;
					p.size = sizeof(p);
					p.type = CS_LOGIN;
					memset(p.id, 0, NAME_SIZE);
					string str;
					str.assign(loginId.begin(), loginId.end());
					memcpy(p.id, str.c_str(), str.size());
					send_packet(&p);
				}
			}
		}
		char net_buf[BUF_SIZE];
		size_t   received;
		auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
		if (recv_result == sf::Socket::Error)
		{
			wcout << L"Recv 에러!";
			while (true);
		}
		if (recv_result != sf::Socket::NotReady)
			if (received > 0) process_data(net_buf, received);

		loginText.setString(L"ID: " + loginId);
		whiteBoard.a_draw();
		loginWindow.draw(loginText);
		loginWindow.display();
	}


	sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "2D CLIENT");
	g_window = &window;


	while (window.isOpen())
	{
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();
			if (event.type == sf::Event::KeyPressed) {
				int direction = -1;
				switch (event.key.code) {
				case sf::Keyboard::Left:
					direction = 3;
					break;
				case sf::Keyboard::Right:
					direction = 4;
					break;
				case sf::Keyboard::Up:
					direction = 1;
					break;
				case sf::Keyboard::Down:
					direction = 2;
					break;
				case sf::Keyboard::Escape:
				{
					CS_LOG_OUT_PACKET logoutPakcet;					
					logoutPakcet.size = sizeof(CS_LOG_OUT_PACKET);
					logoutPakcet.type = CS_LOG_OUT;
					send_packet(&logoutPakcet);
					window.close();
				}
				break;
				}
				if (-1 != direction) {
					CS_MOVE_PACKET p;
					p.size = sizeof(p);
					p.type = CS_MOVE;
					p.direction = direction;
					send_packet(&p);
				}

			}
		}

		window.clear();
		client_main();
		myPosText.setString(TextString);
		window.draw(myPosText);
		window.display();
	}
	client_finish();

	return 0;
}