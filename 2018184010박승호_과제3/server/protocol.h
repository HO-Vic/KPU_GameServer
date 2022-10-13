#pragma once
#include<wtypes.h>
#include"include/glm/glm.hpp"
#include"include/glm/ext.hpp"
#include"include/glm/gtc/matrix_transform.hpp"

#define s2cMovePacket 0x00
#define s2cDiffClientInfoPacket 0x01

#define c2sDirectionPacket 0x10
#define s2cDisconnectCleintInfoPacket 0x11

#pragma pack(push, 1)

struct DirectionPacket {
	int size;
	char type;
	WORD direction;
};

struct ChessPiecePosPacket {
	int size;
	char type;
	int id;
	glm::vec3 pos;
};

struct ClientDisConnectPacket {
	int size;
	char type;
	int id;
};

#pragma pack(pop)