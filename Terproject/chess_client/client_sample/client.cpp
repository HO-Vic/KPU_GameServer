#include <iostream>
#include "PLAYER.h"

using namespace std;

sf::TcpSocket socket;
wstring* g_loginId;
sf::Font font;

constexpr auto SCREEN_WIDTH = 20;
constexpr auto SCREEN_HEIGHT = 20;

constexpr auto WINDOW_WIDTH = SCREEN_WIDTH * TILE_WIDTH;   // size of window
constexpr auto WINDOW_HEIGHT = SCREEN_WIDTH * TILE_WIDTH;

//constexpr auto WINDOW_WIDTH = 800;   // size of window
//constexpr auto WINDOW_HEIGHT = 600;

int g_left_x;
int g_top_y;
int g_myid;

sf::RenderWindow* g_window;

PLAYER myPlayer;
PLAYER players[MAX_USER + MAX_NPC];

OBJECT gameHouseMap;
OBJECT gameGeneralMap;
OBJECT playerAttackEffect;

OBJECT hpBar;
OBJECT BG_hpBar;
OBJECT EXPBar;
OBJECT BG_EXPBar;

OBJECT monsterHpBar;
OBJECT BG_monsterHpBar;

sf::Texture* textureHouseMap;
sf::Texture* textureGeneralMap;
sf::Texture** textureCharacter;
sf::Texture* texturePlayerAttck;

sf::Texture* textureBoss;
sf::Texture* textureGhost;
sf::Texture* textureDog;

sf::Texture* textureHPBar;
sf::Texture* textureBG_HPBar;
sf::Texture* textureEXPBar;
sf::Texture* textureBG_EXPBar;

sf::Texture* textureMonsterHPBar;
sf::Texture* textureMonsterBG_HPBar;

sf::String TextString = "(0, 0)";

sf::Text HPText;
sf::Text EXPText;


constexpr int IDLE_LEFT = 0;
constexpr int IDLE_RIGHT = 1;
constexpr int RUN_LEFT_F = 2;
constexpr int RUN_LEFT_L = 9;
constexpr int RUN_RIGHT_F = 10;
constexpr int RUN_RIGHT_L = 17;

bool g_isEnterPressed = false;
wstring g_chatBuffer;
wstring g_chatCompleteBuffer;


void client_initialize()
{
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
		textureCharacter[10]->loadFromFile("images/player/run/right/Player_Run_0.png");
		textureCharacter[11]->loadFromFile("images/player/run/right/Player_Run_1.png");
		textureCharacter[12]->loadFromFile("images/player/run/right/Player_Run_2.png");
		textureCharacter[13]->loadFromFile("images/player/run/right/Player_Run_3.png");
		textureCharacter[14]->loadFromFile("images/player/run/right/Player_Run_4.png");
		textureCharacter[15]->loadFromFile("images/player/run/right/Player_Run_5.png");
		textureCharacter[16]->loadFromFile("images/player/run/right/Player_Run_6.png");
		textureCharacter[17]->loadFromFile("images/player/run/right/Player_Run_7.png");
	}

	//initialize map
	textureHouseMap = new sf::Texture;
	textureHouseMap->loadFromFile("images/houseMap.png");
	gameHouseMap = OBJECT{ *textureHouseMap, 0, 0, 1000, 1000 };
	gameHouseMap.show();
	textureGeneralMap = new sf::Texture;
	textureGeneralMap->loadFromFile("images/generalMap.png");
	gameGeneralMap = OBJECT{ *textureGeneralMap, 0, 0, 1000, 1000 };
	gameGeneralMap.show();

	textureHPBar = new sf::Texture;
	textureHPBar->loadFromFile("red.png");
	textureBG_HPBar = new sf::Texture;
	textureBG_HPBar->loadFromFile("gray.png");
	textureEXPBar = new sf::Texture;
	textureEXPBar->loadFromFile("yellow.png");
	textureBG_EXPBar = new sf::Texture;
	textureBG_EXPBar->loadFromFile("gray.png");

	textureMonsterHPBar = new sf::Texture;
	textureMonsterHPBar->loadFromFile("red.png");
	textureMonsterBG_HPBar = new sf::Texture;
	textureMonsterBG_HPBar->loadFromFile("gray.png");

	monsterHpBar = OBJECT{ *textureMonsterHPBar, 0, 0, 1, 7 };
	monsterHpBar.SetScale(50, 1);
	monsterHpBar.show();
	BG_monsterHpBar = OBJECT{ *textureMonsterBG_HPBar, 0, 0, 1, 7 };
	BG_monsterHpBar.SetScale(50, 1);
	BG_monsterHpBar.show();

	hpBar = OBJECT{ *textureHPBar, 0, 0, 1, 30 };
	hpBar.SetScale(WINDOW_WIDTH - 500, 1);
	hpBar.a_move(WINDOW_WIDTH - 750, 900);
	hpBar.show();

	BG_hpBar = OBJECT{ *textureBG_HPBar, 0, 0, 1, 30 };
	BG_hpBar.SetScale(WINDOW_WIDTH - 500, 1);
	BG_hpBar.a_move(WINDOW_WIDTH - 750, 900);
	BG_hpBar.show();

	EXPBar = OBJECT{ *textureEXPBar, 0, 0, 1, 7 };
	EXPBar.SetScale(WINDOW_WIDTH - 500, 1);
	EXPBar.a_move(WINDOW_WIDTH - 750, 900 + 30);
	EXPBar.show();

	BG_EXPBar = OBJECT{ *textureBG_EXPBar, 0, 0, 1, 7 };
	BG_EXPBar.SetScale(WINDOW_WIDTH - 500, 1);
	BG_EXPBar.a_move(WINDOW_WIDTH - 750, 900 + 30);
	BG_EXPBar.show();

	HPText.setString("HP 100/100");
	HPText.setPosition(WINDOW_WIDTH - 750, 900);
	HPText.setFont(font);
	HPText.setCharacterSize(20);
	HPText.setFillColor(sf::Color::Black);
	HPText.setOutlineColor(sf::Color::Black);
	HPText.setOutlineThickness(1.f);

	EXPText.setString("EXP 0/100");
	EXPText.setPosition(WINDOW_WIDTH - 750, 900 + 30);
	EXPText.setFont(font);
	EXPText.setCharacterSize(10);
	EXPText.setFillColor(sf::Color::Black);
	EXPText.setOutlineColor(sf::Color::Black);
	EXPText.setOutlineThickness(1.f);

	textureBoss = new sf::Texture;
	textureBoss->loadFromFile("images/boss.png");

	textureGhost = new sf::Texture;
	textureGhost->loadFromFile("images/ghost.png");

	textureDog = new sf::Texture;
	textureDog->loadFromFile("images/dog.png");

	texturePlayerAttck = new sf::Texture;
	texturePlayerAttck->loadFromFile("images/player/Attack.png");

	for (int i = MAX_USER; i < MAX_NPC + MAX_USER; i++) {
		if (i < MAX_USER + 3)
			players[i] = PLAYER{ *textureBoss, 0, 0, TILE_WIDTH, TILE_WIDTH,*textureBG_EXPBar };
		else if (i < MAX_USER + MAX_NPC / 2)
			players[i] = PLAYER{ *textureDog, 0, 0, TILE_WIDTH, TILE_WIDTH ,*textureBG_EXPBar };
		else
			players[i] = PLAYER{ *textureGhost, 0, 0, TILE_WIDTH, TILE_WIDTH ,*textureBG_EXPBar };
	}
	playerAttackEffect = OBJECT{ *texturePlayerAttck, 0, 0, TILE_WIDTH * 3, TILE_WIDTH * 3 };
	playerAttackEffect.show();

}

void client_finish()
{
	delete textureHouseMap;

}

void ProcessPacket(char* ptr)
{
	static bool first_time = true;
	switch (ptr[1])
	{
	case SC_LOGIN_FAIL:
	{
		//g_window->close();
		*g_loginId = L"Login Error";
	}
	break;
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* packet = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(ptr);
		myPlayer = PLAYER{ *textureCharacter[IDLE_LEFT], 0, 0, 50, 50,*textureBG_EXPBar };
		g_myid = packet->id;
		myPlayer.m_x = packet->x;
		myPlayer.m_y = packet->y;
		myPlayer.name = packet->name;

		myPlayer.SetNameText(myPlayer.name);
		myPlayer.SetPlayerStat(packet->hp, packet->max_hp, packet->exp, packet->level);
		hpBar.SetScale((float)(WINDOW_WIDTH - 500) * ((float)packet->hp / (float)packet->max_hp), 1);
		std::string hpStr = "HP ";
		hpStr.append(std::to_string(packet->hp) + " / " + std::to_string(packet->max_hp));
		HPText.setString(hpStr);

		EXPBar.SetScale((float)(WINDOW_WIDTH - 500) * ((float)packet->exp / (float)packet->max_exp), 1);
		EXPText.setString("EXP");

		char xPos[7];
		char yPos[7];

		_itoa(myPlayer.m_x, xPos, 10);
		_itoa(myPlayer.m_y, yPos, 10);
		TextString.clear();
		TextString += "(";
		TextString += xPos;
		TextString += ", ";
		TextString += yPos;
		TextString += ")";

		g_window->close();

		g_left_x = myPlayer.m_x * TILE_WIDTH - TILE_WIDTH * 10;
		g_top_y = myPlayer.m_y * TILE_WIDTH - TILE_WIDTH * 10;
		myPlayer.show();
		break;
	}

	case SC_ADD_OBJECT:
	{
		SC_ADD_OBJECT_PACKET* my_packet = reinterpret_cast<SC_ADD_OBJECT_PACKET*>(ptr);
		int id = my_packet->id;

		if (id == g_myid) {
			players[id].id = -1;
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
		//else if (id < MAX_USER) {
		else {
			if (my_packet->hp == 0) {
				players[my_packet->id].m_showing = false;
			}
			if (id < MAX_USER)
				players[id] = PLAYER{ *textureCharacter[IDLE_LEFT], 0, 0, 50, 50 ,*textureBG_EXPBar };
			else if (id < MAX_USER + 3)
				players[id] = PLAYER{ *textureBoss, 0, 0, 50, 50,*textureBG_EXPBar };
			else if (id < MAX_USER + 3 + (MAX_NPC / 2))
				players[id] = PLAYER{ *textureDog, 0, 0, 50, 50 ,*textureBG_EXPBar };
			else
				players[id] = PLAYER{ *textureGhost, 0, 0, 50, 50 ,*textureBG_EXPBar };


			cout << "add player" << endl;
			players[id].move(my_packet->x, my_packet->y);
			players[id].name = my_packet->name;
			players[id].SetNameText(players[id].name);
			players[id].hp = my_packet->hp;
			players[id].maxHp = my_packet->max_hp;
			players[id].id = id;
			players[id].show();
		}
		//}
		break;
	}
	case SC_MOVE_OBJECT:
	{
		SC_MOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_MOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			if (my_packet->x == 0 && my_packet->y == 0) {
				cout << "fasfasd" << endl;
			}
			myPlayer.move(my_packet->x, my_packet->y);
			g_left_x = my_packet->x * TILE_WIDTH - 10 * TILE_WIDTH;
			g_top_y = my_packet->y * TILE_WIDTH - 10 * TILE_WIDTH;

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
		else {
			players[my_packet->id].move(my_packet->x, my_packet->y);
		}
		break;
	}

	case SC_REMOVE_OBJECT:
	{
		cout << "remove player" << endl;
		SC_REMOVE_OBJECT_PACKET* my_packet = reinterpret_cast<SC_REMOVE_OBJECT_PACKET*>(ptr);
		int other_id = my_packet->id;
		if (other_id == g_myid) {
			myPlayer.hide();
		}
		else {
			players[other_id].hide();
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
	case SC_STAT_CHANGE:
	{
		SC_STAT_CHANGEL_PACKET* packet = reinterpret_cast<SC_STAT_CHANGEL_PACKET*>(ptr);
		if (packet->id == g_myid) {
			myPlayer.SetPlayerStat(packet->hp, packet->max_hp, packet->exp, packet->level);
			hpBar.SetScale((float)(WINDOW_WIDTH - 500) * ((float)packet->hp / (float)packet->max_hp), 1);
			std::string hpStr = "HP ";
			hpStr.append(std::to_string(packet->hp) + " / " + std::to_string(packet->max_hp));
			HPText.setString(hpStr);
			EXPBar.SetScale((float)(WINDOW_WIDTH - 500) * ((float)packet->exp / (float)packet->max_exp), 1);
			EXPText.setString("EXP");
		}
		else {
			cout << "NPC Stat: " << packet->hp << " " << packet->max_hp << endl;
			players[packet->id].SetPlayerStat(packet->hp, packet->max_hp, packet->exp, packet->level);
			if (packet->hp == 0) {
				players[packet->id].m_showing = false;
			}
		}
	}
	break;
	case SC_ATTACK:
	{
		SC_ATTACK_PACKET* packet = reinterpret_cast<SC_ATTACK_PACKET*>(ptr);
		if (packet->id == g_myid)
			myPlayer.StartEffect(packet->skillExecuteTime);
		else players[packet->id].StartEffect(packet->skillExecuteTime);
	}
	break;
	break;
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
		if (0 == in_packet_size) in_packet_size = (unsigned char)ptr[0];
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

	//map rendering
	if (g_left_x < 0) { // 홈 끝 점
		if (g_top_y <= 0) {
			gameHouseMap.a_move(abs(g_left_x), abs(g_top_y));
			gameHouseMap.a_draw();
		}
		else if (g_top_y < 1000) {
			gameHouseMap.a_move(abs(g_left_x), -abs(g_top_y));
			gameHouseMap.a_draw();
			gameGeneralMap.a_move(abs(g_left_x), WINDOW_HEIGHT - abs(g_top_y));
			gameGeneralMap.a_draw();
		}
		else if (g_top_y >= TILE_WIDTH * 20 * 100 - TILE_WIDTH * 20) {
			gameGeneralMap.a_move(abs(g_left_x), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
		else {
			gameGeneralMap.a_move(abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(abs(g_left_x % WINDOW_WIDTH), WINDOW_HEIGHT - abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
	}
	else if (g_left_x <= 1000) {// 홈 과 겹침
		if (g_top_y <= 0) {
			gameHouseMap.a_move(-abs(g_left_x), abs(g_top_y));
			gameHouseMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x), abs(g_top_y));
			gameGeneralMap.a_draw();
		}
		else if (g_top_y < 1000) {
			gameHouseMap.a_move(-abs(g_left_x), -abs(g_top_y));
			gameHouseMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x), -abs(g_top_y));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(-abs(g_left_x), WINDOW_HEIGHT - abs(g_top_y));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x), WINDOW_HEIGHT - abs(g_top_y));
			gameGeneralMap.a_draw();
		}
		else if (g_top_y >= TILE_WIDTH * 20 * 100 - TILE_WIDTH * 20) {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
		else {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x % WINDOW_WIDTH), WINDOW_HEIGHT - abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), WINDOW_HEIGHT - abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
	}
	else if (g_left_x > TILE_WIDTH * 20 * 100 - TILE_WIDTH * 20) { //끝점
		if (g_top_y <= 0) {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), abs(g_top_y));
			gameGeneralMap.a_draw();
		}
		else if (g_top_y < 1000) {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), WINDOW_HEIGHT - abs(g_top_y));
			gameGeneralMap.a_draw();
		}
		else if (g_top_y >= TILE_WIDTH * 20 * 100 - TILE_WIDTH * 20) {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
		else {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), WINDOW_HEIGHT - abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
	}
	else { // 나머지
		if (g_top_y <= 0) {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x % WINDOW_WIDTH), abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
		else if (g_top_y >= TILE_WIDTH * 20 * 100 - TILE_WIDTH * 20) {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
		else {
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x % WINDOW_WIDTH), -abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(WINDOW_WIDTH - abs(g_left_x % WINDOW_WIDTH), WINDOW_HEIGHT - abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
			gameGeneralMap.a_move(-abs(g_left_x % WINDOW_WIDTH), WINDOW_HEIGHT - abs(g_top_y % WINDOW_HEIGHT));
			gameGeneralMap.a_draw();
		}
	}

	myPlayer.draw();
	myPlayer.CheckHideSkillEffect();
	if (myPlayer.GetshowSkill()) {
		playerAttackEffect.a_move(myPlayer.m_x * TILE_WIDTH - g_left_x - 50, myPlayer.m_y * TILE_WIDTH - g_top_y - 50 - 5);
		playerAttackEffect.a_draw();
	}
	for (auto& pl : players) {
		if (pl.id != g_myid) {
			pl.draw();
			pl.CheckHideSkillEffect();
			if (pl.GetshowSkill()) {
				playerAttackEffect.a_move(pl.m_x * TILE_WIDTH - g_left_x - 50, pl.m_y * TILE_WIDTH - g_top_y - 50 - 5);
				playerAttackEffect.a_draw();
			}
			if (pl.m_showing) {
				BG_monsterHpBar.a_move(pl.m_x * TILE_WIDTH - g_left_x, pl.m_y * TILE_WIDTH - g_top_y - 17);
				monsterHpBar.a_move(pl.m_x * TILE_WIDTH - g_left_x, pl.m_y * TILE_WIDTH - g_top_y - 17);
				monsterHpBar.SetScale(50.0f * ((float)pl.hp / (float)pl.maxHp), 1);
				BG_monsterHpBar.a_draw();
				monsterHpBar.a_draw();
			}
		}
		else {
			cout << "sadf" << endl;
		}
	}
	BG_hpBar.a_draw();
	BG_EXPBar.a_draw();
	hpBar.a_draw();
	EXPBar.a_draw();
	g_window->draw(HPText);
	g_window->draw(EXPText);
	
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
	whiteBoard.show();
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
					memset(p.loginId, 0, NAME_SIZE);
					string str;
					str.assign(loginId.begin(), loginId.end());
					memcpy(p.loginId, str.c_str(), str.size());
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
			else if (event.type == sf::Event::TextEntered) {
				if (g_isEnterPressed) {
					if (std::isprint(event.text.unicode)) {
						g_chatCompleteBuffer += event.text.unicode;
						g_chatBuffer = g_chatCompleteBuffer;
						myPlayer.set_chat(g_chatBuffer.c_str());
					}
				}
			}
			if (event.type == sf::Event::KeyPressed) {
				if (g_isEnterPressed) {
					if (event.key.code == sf::Keyboard::BackSpace) {
						if (!g_chatBuffer.empty()) {
							if (g_chatBuffer.back() == g_chatCompleteBuffer.back()) {
								g_chatBuffer.pop_back();
								g_chatCompleteBuffer.pop_back();
							}
							else g_chatBuffer.pop_back();
						}
						myPlayer.set_chat(g_chatBuffer.c_str());
					}
					if (event.key.code == sf::Keyboard::Return) {
						CS_CHAT_PACKET p;
						p.type = CS_CHAT;
						int sendSize = min(99, (int)g_chatCompleteBuffer.size());
						wcsncpy_s(p.mess, g_chatCompleteBuffer.c_str(), sendSize);
						p.mess[sendSize] = L'\0';
						p.size = sizeof(CS_CHAT_PACKET);
						send_packet(&p);
						myPlayer.set_chat(g_chatCompleteBuffer.c_str());
						g_chatCompleteBuffer.clear();
						g_chatBuffer.clear();
						g_isEnterPressed = false;
					}
					else {
						g_chatBuffer += event.text.unicode;
						myPlayer.set_chat(g_chatBuffer.c_str());
					}
				}
				else {
					int direction = -1;
					switch (event.key.code) {
					case sf::Keyboard::Enter:
					{
						g_isEnterPressed = true;
						myPlayer.ClearChatBuffer();
					}
					break;
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
						CS_LOGOUT_PACKET logoutPakcet;
						logoutPakcet.size = sizeof(CS_LOGOUT_PACKET);
						logoutPakcet.type = CS_LOGOUT;
						send_packet(&logoutPakcet);
						window.close();
					}
					break;
					case sf::Keyboard::A:
					{
						if (myPlayer.IsAbleSkill()) {
							CS_ATTACK_PACKET attackPacket;
							attackPacket.size = sizeof(CS_ATTACK_PACKET);
							attackPacket.type = CS_ATTACK;
							send_packet(&attackPacket);
						}
					}
					break;

					}
					if (-1 != direction) {
						CS_MOVE_PACKET p;
						p.size = sizeof(p);
						p.type = CS_MOVE;
						p.direction = direction;
						p.move_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
						send_packet(&p);
					}
				}
			}
		}

		window.clear();
		client_main();
		/*myPosText.setString(TextString);
		window.draw(myPosText);*/
		window.display();
	}

	client_finish();

	return 0;
}
