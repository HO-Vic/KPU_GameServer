#include <iostream>
#include "OBJECT.h"

using namespace std;
#include "protocol.h"

sf::TcpSocket socket;
wstring* g_loginId;
sf::Font font;
constexpr auto SCREEN_WIDTH = W_WIDTH;
constexpr auto SCREEN_HEIGHT = W_HEIGHT;

constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_WIDTH * TILE_WIDTH;
constexpr auto MAX_USER = 10000;

int g_left_x;
int g_top_y;
int g_myid;

sf::RenderWindow* g_window;

OBJECT myPlayer;
OBJECT players[MAX_USER];

OBJECT white_tile;
OBJECT gameMap;

sf::Texture* textureMap;
sf::Texture** textureCharacter;

sf::String TextString = "(0, 0)";

constexpr int IDLE_LEFT = 0;
constexpr int IDLE_RIGHT = 1;
constexpr int RUN_LEFT_F = 2;
constexpr int RUN_LEFT_L = 9;
constexpr int RUN_RIGHT_F = 10;
constexpr int RUN_RIGHT_L = 17;


void client_initialize()
{
	textureMap = new sf::Texture;
	textureMap->loadFromFile("images/map.png");
	textureCharacter = new sf::Texture * [18];
	for (int i = 0; i < 18; i++)
		textureCharacter[i] = new sf::Texture;
	//player Texture Init
	{
		textureCharacter[0]->loadFromFile("images/player/Idle/Player_Idle_Right.png");
		textureCharacter[1]->loadFromFile("images/player/Idle/Player_Idle_Left.png");
		textureCharacter[2]->loadFromFile("images/player/run/left/Player_Run_0.png");
		textureCharacter[3]->loadFromFile("images/player/run/left/Player_Run_1.png");
		textureCharacter[4]->loadFromFile("images/player/run/left/Player_Run_2.png");
		textureCharacter[5]->loadFromFile("images/player/run/left/Player_Run_3.png");
		textureCharacter[6]->loadFromFile("images/player/run/left/Player_Run_4.png");
		textureCharacter[7]->loadFromFile("images/player/run/left/Player_Run_5.png");
		textureCharacter[8]->loadFromFile("images/player/run/left/Player_Run_6.png");
		textureCharacter[9]->loadFromFile("images/player/run/left/Player_Run_7.png");
		textureCharacter[10]->loadFromFile("images/player/run/right/Player_Run_10.png");
		textureCharacter[11]->loadFromFile("images/player/run/right/Player_Run_11.png");
		textureCharacter[12]->loadFromFile("images/player/run/right/Player_Run_12.png");
		textureCharacter[13]->loadFromFile("images/player/run/right/Player_Run_13.png");
		textureCharacter[14]->loadFromFile("images/player/run/right/Player_Run_14.png");
		textureCharacter[15]->loadFromFile("images/player/run/right/Player_Run_15.png");
		textureCharacter[16]->loadFromFile("images/player/run/right/Player_Run_16.png");
		textureCharacter[17]->loadFromFile("images/player/run/right/Player_Run_17.png");
	}

	//initialize map

	gameMap = OBJECT{ *textureMap, 0, 0, 1000, 1000 };

	myPlayer = OBJECT{ *textureCharacter[IDLE_LEFT], 0, 0, 50, 50 };
	myPlayer.move(4, 4);
	g_left_x = 4 * TILE_WIDTH - TILE_WIDTH * 10;
	g_top_y = 4 * TILE_WIDTH - TILE_WIDTH * 10;
	myPlayer.show();

	/*for (auto& pl : players) {
		pl = OBJECT{ *textureCharacter[0], 0, 0, 32, 32 };
	}*/
}

void client_finish()
{
	delete textureMap;

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
		myPlayer.m_x = packet->x;
		myPlayer.m_y = packet->y;
		string str{ packet->name };

		myPlayer.nameText.setColor(sf::Color::Green);
		myPlayer.nameText.setOutlineColor(sf::Color::Green);
		myPlayer.nameText.setString(packet->name);
		char xPos[5];
		char yPos[5];

		_itoa(myPlayer.m_x, xPos, 10);
		_itoa(myPlayer.m_y, yPos, 10);
		TextString.clear();
		TextString += "(";
		TextString += xPos;
		TextString += ", ";
		TextString += yPos;
		TextString += ")";

		g_window->close();
		myPlayer.show();
		break;
	}

	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* my_packet = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {
			myPlayer.move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - 10 * 50;
			g_top_y = my_packet->y - 10 * 50;
			//memcpy(avatar.name, my_packet->name, strlen(my_packet->name));
			char xPos[5];
			char yPos[5];

			_itoa(myPlayer.m_x, xPos, 10);
			_itoa(myPlayer.m_y, yPos, 10);
			TextString.clear();
			TextString += "(";
			TextString += xPos;
			TextString += ", ";
			TextString += yPos;
			TextString += ")";

			myPlayer.show();
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
			myPlayer.move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x - 10 * 50;
			g_top_y = my_packet->y - 10 * 50;

			char xPos[5];
			char yPos[5];

			_itoa(myPlayer.m_x, xPos, 10);
			_itoa(myPlayer.m_y, yPos, 10);
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
			myPlayer.hide();
		}
		else if (other_id < MAX_USER) {
			players[other_id].hide();
		}
		else {
			//      npc[other_id - NPC_START].attr &= ~BOB_ATTR_VISIBLE;
		}
		break;
	}
	case SC_CHAT:
	{
		SC_CHAT_PACKET* my_packet = reinterpret_cast<SC_CHAT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			myPlayer.set_chat(my_packet->mess);
		}
		else {
			players[other_id].set_chat(my_packet->mess);
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
	/*char net_buf[BUF_SIZE];
	size_t   received;

	auto recv_result = socket.receive(net_buf, BUF_SIZE, received);
	if (recv_result == sf::Socket::Error)
	{
		wcout << L"Recv 에러!";
		while (true);
	}
	if (recv_result != sf::Socket::NotReady)
		if (received > 0) process_data(net_buf, received);*/

		//for (int i = 0; i < SCREEN_WIDTH; ++i)
		//	for (int j = 0; j < SCREEN_HEIGHT; ++j)
		//	{
		//		int tile_x = i + g_left_x;
		//		int tile_y = j + g_top_y;
		//		if ((tile_x < 0) || (tile_y < 0)) continue;
		//		if ((tile_x > 2000) || (tile_y > 2000)) continue;
		//		else
		//		{
		//			gameMap[tile_x / 20][tile_y / 20].a_move(tile_x % 20, tile_y % 20);
		//			gameMap[tile_x / 20][tile_y / 20].a_draw();
		//		}
		//	}

	if (g_left_x < 0) {
		gameMap.m_x = abs(g_left_x);
		gameMap.a_move(abs(g_left_x), gameMap.m_y);
	}
	else if (g_left_x > 2000) {
		gameMap.m_x = -abs(g_left_x - 2000);
		gameMap.a_move(-abs(g_left_x - 2000), gameMap.m_y);
	}
	else {
		gameMap.m_x = 0;
		gameMap.a_move(0, gameMap.m_y);
	}
	if (g_top_y < 0) {
		gameMap.a_move(gameMap.m_x, abs(g_top_y));
	}
	else if (g_top_y > 2000)
		gameMap.a_move(gameMap.m_x, -abs(g_top_y - 2000));
	else gameMap.a_move(gameMap.m_x, 0);

	gameMap.a_draw();
	myPlayer.draw();
	//for (auto& pl : players) pl.draw();
}

void send_packet(void* packet)
{
	unsigned char* p = reinterpret_cast<unsigned char*>(packet);
	size_t sent = 0;
	socket.send(packet, p[0], sent);
}

int main()
{
	//wcout.imbue(locale("korean"));
	//sf::Socket::Status status = socket.connect("127.0.0.1", PORT_NUM);
	//socket.setBlocking(false);

	//if (status != sf::Socket::Done) {
	//	wcout << L"서버와 연결할 수 없습니다.\n";
	//	while (true);
	//}

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

	/*sf::RenderWindow loginWindow(sf::VideoMode(300, 100), "Login");
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
	loginText.setOutlineThickness(1.f);*/

	/*while (loginWindow.isOpen()) {
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
	}*/


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
					myPlayer.move(myPlayer.m_x - 1, myPlayer.m_y);
					g_left_x = myPlayer.m_x * TILE_WIDTH - 10 * TILE_WIDTH;
					direction = 3;
					break;
				case sf::Keyboard::Right:
					myPlayer.move(myPlayer.m_x + 1, myPlayer.m_y);
					g_left_x = myPlayer.m_x * TILE_WIDTH - 10 * TILE_WIDTH;
					direction = 4;
					break;
				case sf::Keyboard::Up:
					myPlayer.move(myPlayer.m_x, myPlayer.m_y - 1);
					g_top_y = myPlayer.m_y * TILE_WIDTH - 10 * TILE_WIDTH;
					direction = 1;
					break;
				case sf::Keyboard::Down:
					myPlayer.move(myPlayer.m_x, myPlayer.m_y + 1);
					g_top_y = myPlayer.m_y * TILE_WIDTH - 10 * TILE_WIDTH;
					direction = 2;
					break;
					/*case sf::Keyboard::Escape:
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
					}*/

				}
			}

			window.clear();
			client_main();
			/*myPosText.setString(TextString);
			window.draw(myPosText);*/
			window.display();
		}
	}
	client_finish();

	return 0;
}
