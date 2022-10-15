#include<iostream>
#include<string>
#include<unordered_map>
#include<WS2tcpip.h>
#include<chrono> 

#include"Dependencies/glew.h"
#include"Dependencies/freeglut.h"
#include"include/glm/glm.hpp"
#include"include/glm/ext.hpp"
#include"include/glm/gtc/matrix_transform.hpp"
#include"filetobuf.h"
#include"readTriangleObj.h"
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"
#include"protocol.h"
#pragma comment(lib,"ws2_32")

using namespace std;
using namespace chrono;

//shader func
void makeVertexShader();
void makeFragmentShader();
void makeShaderID();
void InitBuffer();
void initTexture();

//call_back
void timercall(int value);
void DrawSceneCall();
void ReshapeCall(int w, int h);
void keyboardCall(unsigned char key, int x, int y);
void specialkeycall(int key, int x, int y);

//func
void makeObj();
void drawChessBoard();
void drawChessPiece();
void drawChessPieceOtherClient(glm::vec3&);

//
void constructPacket(char* inputPacket, int inputSize);
void proccessPacket(char* completePacket);
void display_Err(int Errcode);

void CALLBACK recv_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK send_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);


void doRecv();
void doSend(char* sendPacket, int size);

struct EX_Over {
	WSAOVERLAPPED over;
	char Buf[BUF_SIZE] = { 0 };
	WSABUF WSABuf;
};

GLuint fragmentShader;
GLuint modelvertexShader;
GLuint ShaderID;

char* vertexSource;
char* fragmentSource;

int Wwidth = 800;
int Wheight = 600;

GLuint chessBoardVAO;
GLuint chessBoardVertexVBO;
GLuint chessBoardNormalVBO;
GLuint chessBoardTextureVBO;
vector<glm::vec4> chessBoardVertex;
vector<glm::vec4> chessBoardNormal;
vector<glm::vec2> chessBoardTexture;

GLuint chessPieceVAO;
GLuint chessPieceVertexVBO;
GLuint chessPieceNormalVBO;
GLuint chessPieceTextureVBO;
vector<glm::vec4> chessPieceVertex;
vector<glm::vec4> chessPieceNormal;
vector<glm::vec2> chessPieceTexture;

glm::vec3 chessPiecePos = glm::vec3(0.5f, 0.0f, 3.5f);

glm::vec3 cameraPos = glm::vec3(0, 7.0f, 7.0f);
float cameraRevolu = 0.0f;

glm::vec3 lightPos = glm::vec3(0, 3.0f, 2.5f);
glm::vec3 lightColor = glm::vec3(0.7f, 0.7f, 0.7f);
float lightRevoluAngle = 0.0f;

float Xangle = 0;
float Yangle = 0;

unsigned int textures[3] = { 0 };

int myId = -1;

/////////////////////
SOCKET mySocket;
unordered_map<int, glm::vec3>DiffClients;
char prevPacket[BUF_SIZE] = { 0 };
int prevPacketSize = 0;
int prevPacketLastLocal = 0;



int main(int argc, char** argv)
{
	wcout.imbue(std::locale("korean"));
	string serverIp = "127.0.0.1";
	//cout << "서버 IP입력: ";
	//cin >> serverIp;
	string name;
	cout << "이름 입력: ";
	cin >> name;
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition(2200, 200);
	glutInitWindowSize(Wwidth, Wheight);
	glutCreateWindow("chess Board");

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
		cerr << "fail Initialize" << endl;
	else cout << "Initialize" << endl;

	WSADATA WSAData;

	if (WSAStartup(MAKEWORD(2, 2), &WSAData) != 0) {
		cout << "wsaStartUp Fail" << endl;
	}

	mySocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, NULL, WSA_FLAG_OVERLAPPED);

	if (mySocket == INVALID_SOCKET) {
		cout << "socket create Fail" << endl;
	}

	SOCKADDR_IN serverAddr;
	ZeroMemory(&serverAddr, sizeof(SOCKADDR_IN));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr);

	if (connect(mySocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == INVALID_SOCKET) {
		cout << "connect Fail" << endl;
	}

	CS_LOGIN_PACKET loginPacket;
	loginPacket.type = CS_LOGIN;
	memcpy(loginPacket.name, name.c_str(), name.size());
	loginPacket.size = sizeof(loginPacket);
	doSend(reinterpret_cast<char*>(&loginPacket), loginPacket.size);
	//doRecv();
	makeShaderID();

	makeObj();
	InitBuffer();
	glUniform1i(glGetUniformLocation(ShaderID, "ambientLight"), 0.3f);
	initTexture();
	glutDisplayFunc(DrawSceneCall);
	glutReshapeFunc(ReshapeCall);
	glutKeyboardFunc(keyboardCall);
	glutSpecialFunc(specialkeycall);
	glutTimerFunc(1, timercall, 1);
	glutMainLoop();

	closesocket(mySocket);
	WSACleanup();
}

void timercall(int value)
{
	doRecv();
	glutPostRedisplay();
	glutTimerFunc(17, timercall, value);
}

void DrawSceneCall()
{
	glClearColor(0.5, 0.5, 0.5, 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	//카메라
	glm::mat4 cameraRevoluMatrix = glm::mat4(1.0f);
	//cameraRevoluMatrix = glm::rotate(cameraRevoluMatrix, glm::radians(cameraRevolu), glm::vec3(0, 1, 0));
	//glm::vec3 newCameraPos = glm::vec3(cameraRevoluMatrix * glm::vec4(cameraPos, 1));

	glm::vec3 ResChessPiecePos;
	ResChessPiecePos.y = 0;
	if (chessPiecePos.x < -191) {
		ResChessPiecePos.x = chessPiecePos.x + 192;
	}
	else if (chessPiecePos.x > 192) {
		ResChessPiecePos.x = chessPiecePos.x - 192;
	}
	else if ((int)(chessPiecePos.x + chessPiecePos.z) % 2 == 0) ResChessPiecePos.x = 0;
	else ResChessPiecePos.x = 1;

	if (chessPiecePos.z < -191) {
		ResChessPiecePos.z = chessPiecePos.z + 192;
	}
	else if (chessPiecePos.z > 192) {
		ResChessPiecePos.z = chessPiecePos.z - 192;
	}
	else ResChessPiecePos.z = 0;


	cameraPos = ResChessPiecePos + glm::vec3(0.0f, 8.0f * tanf(glm::radians(55.0f)), 00.0f);
	//glm::vec3 objCenter = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 cameraDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));//n
	glm::vec3 up = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 cameraRight = glm::cross(up, cameraDir);//v
	glm::vec3 cameraUp = glm::cross(cameraDir, cameraRight);
	glm::mat4 cameraView = glm::mat4(1.0f);
	cameraView = glm::lookAt(cameraPos, ResChessPiecePos, cameraUp);
	unsigned int cameraViewLocation = glGetUniformLocation(ShaderID, "viewTransform");
	glUniformMatrix4fv(cameraViewLocation, 1, GL_FALSE, glm::value_ptr(cameraView));
	unsigned int cameraPosLocation = glGetUniformLocation(ShaderID, "cameraPos");
	glUniform3fv(cameraPosLocation, 1, glm::value_ptr(cameraPos));

	//카메라#

	//투영
	//원근 투영
	glm::mat4 projection = glm::mat4(1.0f);

	projection = glm::perspective(glm::radians(45.0f), (float)Wheight / (float)Wwidth, 0.1f, 50.0f);
	projection = glm::translate(projection, glm::vec3(0, 0, -5.0f));
	unsigned int projectionLocation = glGetUniformLocation(ShaderID, "projectionTransform");
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

	//조명
	//light Color
	unsigned int lightColorLocation = glGetUniformLocation(ShaderID, "LightColor");
	glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));
	unsigned int lightPosLocation = glGetUniformLocation(ShaderID, "LightPos");
	glUniform3fv(lightPosLocation, 1, glm::value_ptr(lightPos));
	//light Trans
	glm::mat4 lightTransMatrix = glm::mat4(1.0f);
	lightTransMatrix = glm::rotate(lightTransMatrix, glm::radians(lightRevoluAngle), glm::vec3(0, 1, 0));
	unsigned int lightTransLocation = glGetUniformLocation(ShaderID, "LightTransform");
	glUniformMatrix4fv(lightTransLocation, 1, GL_FALSE, glm::value_ptr(lightTransMatrix));


	drawChessBoard();
	drawChessPiece();
	for (auto& client : DiffClients) {
		//cout << "client[" << client.first << "]" << endl;
		if (client.first != myId)
			drawChessPieceOtherClient(client.second);
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	glutSwapBuffers();
}

void ReshapeCall(int w, int h)
{
	glViewport(0, 0, w, h);
	Wwidth = w;
	Wheight = h;
}

void keyboardCall(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27:
		exit(0);
		break;
	case'z':
		break;
	default:
		break;
	}
	glutPostRedisplay();
}

void specialkeycall(int key, int x, int y)
{
	WSABUF wsaBuf;

	CS_MOVE_PACKET directionPacket;
	directionPacket.size = sizeof(CS_MOVE_PACKET);
	directionPacket.type = CS_MOVE;

	switch (key)
	{
	case GLUT_KEY_LEFT:
	{
		directionPacket.direction = DIRECTION_LEFT;
		directionPacket.move_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		doSend(reinterpret_cast<char*>(&directionPacket), directionPacket.size);
		cout << "DIRECTION_LEFT" << endl;
	}
	break;
	case GLUT_KEY_RIGHT:
		directionPacket.direction = DIRECTION_RIGHT;
		directionPacket.move_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		doSend(reinterpret_cast<char*>(&directionPacket), directionPacket.size);
		cout << "DIRECTION_RIGHT" << endl;
		break;
	case GLUT_KEY_UP:
		directionPacket.direction = DIRECTION_FRONT;
		directionPacket.move_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		doSend(reinterpret_cast<char*>(&directionPacket), directionPacket.size);
		cout << "DIRECTION_FRONT: " << endl;
		break;
	case GLUT_KEY_DOWN:
		directionPacket.direction = DIRECTION_BACK;
		directionPacket.move_time = duration_cast<milliseconds>(high_resolution_clock::now().time_since_epoch()).count();
		doSend(reinterpret_cast<char*>(&directionPacket), directionPacket.size);
		cout << "DIRECTION_BACK: " << endl;
		break;
	default:
		break;
	}

	glutPostRedisplay();
}


void makeVertexShader()
{
	vertexSource = filetobuf("modelVertexShader.glsl");
	modelvertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(modelvertexShader, 1, &vertexSource, NULL);
	glCompileShader(modelvertexShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(modelvertexShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(modelvertexShader, 512, NULL, errorLog);
		cerr << "VERTEXSHADER ERROR: " << errorLog << endl;
	}
}

void makeFragmentShader()
{
	fragmentSource = filetobuf("fragmentShader.glsl");
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		cerr << "FRAGMENT SHADER ERROR: " << errorLog << endl;
	}
}

void makeShaderID()
{
	makeVertexShader();
	makeFragmentShader();

	ShaderID = glCreateProgram();

	glAttachShader(ShaderID, modelvertexShader);
	glAttachShader(ShaderID, fragmentShader);

	glLinkProgram(ShaderID);
	GLint result;
	glGetProgramiv(ShaderID, GL_LINK_STATUS, &result);
	GLchar errorLog[512];
	if (!result) {
		glGetProgramInfoLog(ShaderID, 512, NULL, errorLog);
		cerr << "ShaderID0 Program ERROR: " << errorLog << endl;
	}

	glDeleteShader(modelvertexShader);
	glDeleteShader(fragmentShader);
	glUseProgram(ShaderID);
}

void InitBuffer()
{
	glGenVertexArrays(1, &chessBoardVAO);
	glBindVertexArray(chessBoardVAO);
	glGenBuffers(1, &chessBoardVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chessBoardVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * chessBoardVertex.size(), &chessBoardVertex[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &chessBoardNormalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chessBoardNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * chessBoardNormal.size(), &chessBoardNormal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &chessBoardTextureVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chessBoardTextureVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * chessBoardTexture.size(), &chessBoardTexture[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &chessPieceVAO);
	glBindVertexArray(chessPieceVAO);
	glGenBuffers(1, &chessPieceVertexVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chessPieceVertexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * chessPieceVertex.size(), &chessPieceVertex[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &chessPieceNormalVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chessPieceNormalVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * chessPieceNormal.size(), &chessPieceNormal[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &chessPieceTextureVBO);
	glBindBuffer(GL_ARRAY_BUFFER, chessPieceTextureVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * chessPieceTexture.size(), &chessPieceTexture[0], GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
	glEnableVertexAttribArray(2);
}

void makeObj()
{
	readTriangleObj("Model/chessBoard/Obj_ChessBoard.obj", chessBoardVertex, chessBoardTexture, chessBoardNormal);
	readTriangleObj("Model/chessPiece/Obj_ChessPiece.obj", chessPieceVertex, chessPieceTexture, chessPieceNormal);
}

void initTexture()
{
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int chessBoardwidthImage, chessBoardheightImage, chessBoardnumberOfChannel;
		unsigned char* chessBoardData = stbi_load("Model/chessBoard/Texture_ChessBoard.jpg", &chessBoardwidthImage, &chessBoardheightImage, &chessBoardnumberOfChannel, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, chessBoardwidthImage, chessBoardheightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, chessBoardData);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(chessBoardData);
	}

	{
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, GL_TEXTURE1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int chessPiecewidthImage, chessPieceheightImage, chessPiecenumberOfChannel;
		unsigned char* chessPieceData = stbi_load("Model/chessPiece/Texture_ChessPiece.bmp", &chessPiecewidthImage, &chessPieceheightImage, &chessPiecenumberOfChannel, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, chessPiecewidthImage, chessPieceheightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, chessPieceData);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(chessPieceData);
	}

	{
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, GL_TEXTURE2);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		int chessPiece_RWidthImage, chessPiece_RHeightImage, chessPiece_RNumberOfChannel;
		unsigned char* chessPiece_RData = stbi_load("Model/chessPiece/Texture_ChessPiece_R.bmp", &chessPiece_RWidthImage, &chessPiece_RHeightImage, &chessPiece_RNumberOfChannel, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, chessPiece_RWidthImage, chessPiece_RHeightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, chessPiece_RData);
		glGenerateMipmap(GL_TEXTURE_2D);
		stbi_image_free(chessPiece_RData);
	}
}

void drawChessBoard()
{
	glUseProgram(ShaderID);
	glBindVertexArray(chessBoardVAO);
	glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);
	glm::mat4 chessBoardTrans = glm::mat4(1.0f);
	unsigned int chessBoardNormalLocation = glGetUniformLocation(ShaderID, "normalTransform");
	glUniformMatrix4fv(chessBoardNormalLocation, 1, GL_FALSE, glm::value_ptr(chessBoardTrans));
	chessBoardTrans = glm::translate(chessBoardTrans, glm::vec3(0.5f, 0, 0.5f));
	chessBoardTrans = glm::scale(chessBoardTrans, glm::vec3(16, 16, 16));
	unsigned int chessBoardTransLocation = glGetUniformLocation(ShaderID, "modelTransform");
	glUniformMatrix4fv(chessBoardTransLocation, 1, GL_FALSE, glm::value_ptr(chessBoardTrans));
	glUniform1i(glGetUniformLocation(ShaderID, "textureC"), 0);
	glUniform1i(glGetUniformLocation(ShaderID, "isTexture"), 1);
	glDrawArrays(GL_TRIANGLES, 0, chessBoardVertex.size());
}

void drawChessPiece()
{
	glUseProgram(ShaderID);
	glBindVertexArray(chessPieceVAO);
	glBindTexture(GL_TEXTURE_2D, GL_TEXTURE1);
	glm::mat4 chessPieceTrans = glm::mat4(1.0f);
	unsigned int chessPieceNormalLocation = glGetUniformLocation(ShaderID, "normalTransform");
	glUniformMatrix4fv(chessPieceNormalLocation, 1, GL_FALSE, glm::value_ptr(chessPieceTrans));
	glm::vec3 ResChessPiecePos;
	ResChessPiecePos.y = 0;
	if (chessPiecePos.x < -191) {
		ResChessPiecePos.x = chessPiecePos.x + 192;
	}
	else if (chessPiecePos.x > 192) {
		ResChessPiecePos.x = chessPiecePos.x - 192;
	}
	else if ((int)(chessPiecePos.x + chessPiecePos.z) % 2 == 0) ResChessPiecePos.x = 0;
	else ResChessPiecePos.x = 1;

	if (chessPiecePos.z < -191) {
		ResChessPiecePos.z = chessPiecePos.z + 192;
	}
	else if (chessPiecePos.z > 192) {
		ResChessPiecePos.z = chessPiecePos.z - 192;
	}
	else ResChessPiecePos.z = 0;


	chessPieceTrans = glm::translate(chessPieceTrans, ResChessPiecePos);
	unsigned int chessPieceTransLocation = glGetUniformLocation(ShaderID, "modelTransform");
	glUniformMatrix4fv(chessPieceTransLocation, 1, GL_FALSE, glm::value_ptr(chessPieceTrans));
	glUniform1i(glGetUniformLocation(ShaderID, "textureC"), 1);
	glUniform1i(glGetUniformLocation(ShaderID, "isTexture"), 1);
	glDrawArrays(GL_TRIANGLES, 0, chessPieceVertex.size());
}

void drawChessPieceOtherClient(glm::vec3& pos)
{
	glUseProgram(ShaderID);
	glBindVertexArray(chessPieceVAO);
	glBindTexture(GL_TEXTURE_2D, GL_TEXTURE2);
	glm::mat4 chessPieceTrans = glm::mat4(1.0f);
	unsigned int chessPieceNormalLocation = glGetUniformLocation(ShaderID, "normalTransform");
	glUniformMatrix4fv(chessPieceNormalLocation, 1, GL_FALSE, glm::value_ptr(chessPieceTrans));

	glm::vec3 ResChessPiecePos;
	ResChessPiecePos.y = 0;
	if (chessPiecePos.x < -191) {
		ResChessPiecePos.x = chessPiecePos.x + 192;
	}
	else if (chessPiecePos.x > 192) {
		ResChessPiecePos.x = chessPiecePos.x - 192;
	}
	else if ((int)(chessPiecePos.x + chessPiecePos.z) % 2 == 0) ResChessPiecePos.x = 0;
	else ResChessPiecePos.x = 1;

	if (chessPiecePos.z < -191) {
		ResChessPiecePos.z = chessPiecePos.z + 192;
	}
	else if (chessPiecePos.z > 192) {
		ResChessPiecePos.z = chessPiecePos.z - 192;
	}
	else ResChessPiecePos.z = 0;


	glm::vec3 renderPos = { 0,-100,0 };
	if (abs(chessPiecePos.x - pos.x) < 16 && abs(chessPiecePos.z - pos.z) < 16) {
		renderPos.x = ResChessPiecePos.x + pos.x - chessPiecePos.x;
		renderPos.z = ResChessPiecePos.z + pos.z - chessPiecePos.z;
		renderPos.y = 0;
	}

	chessPieceTrans = glm::translate(chessPieceTrans, renderPos);
	unsigned int chessPieceTransLocation = glGetUniformLocation(ShaderID, "modelTransform");
	glUniformMatrix4fv(chessPieceTransLocation, 1, GL_FALSE, glm::value_ptr(chessPieceTrans));
	glUniform1i(glGetUniformLocation(ShaderID, "textureC"), 2);
	glUniform1i(glGetUniformLocation(ShaderID, "isTexture"), 1);
	glDrawArrays(GL_TRIANGLES, 0, chessPieceVertex.size());
}

void constructPacket(char* inputPacket, int inputSize)
{
	char* packetStart = inputPacket;
	short restSize = inputSize;
	short currentPacketLocal = 0;
	while (restSize != 0) {
		//cout << "restSize: " << restSize << endl;
		if (inputSize - currentPacketLocal < 1) {
			memcpy(prevPacket + prevPacketLastLocal, inputPacket + currentPacketLocal, inputSize - (int)currentPacketLocal);
			prevPacketLastLocal += inputSize - (int)currentPacketLocal;
			break;
		}
		if (prevPacketLastLocal < 1) {
			int memSize = 2 - prevPacketLastLocal;
			memcpy(prevPacket + prevPacketLastLocal, inputPacket + currentPacketLocal, memSize);
			currentPacketLocal += memSize;
			prevPacketLastLocal += memSize;
			restSize -= memSize;
		}
		unsigned char makePacketSize;
		memcpy(&makePacketSize, prevPacket, 1);

		if (restSize + prevPacketLastLocal < makePacketSize) {
			memcpy(prevPacket + prevPacketLastLocal, inputPacket + currentPacketLocal, restSize);
			currentPacketLocal += restSize;
			prevPacketLastLocal += restSize;
			restSize = 0;
		}
		else {
			memcpy(prevPacket + prevPacketLastLocal, inputPacket + currentPacketLocal, (int)makePacketSize - prevPacketLastLocal);
			restSize -= makePacketSize - prevPacketLastLocal;
			currentPacketLocal += makePacketSize - prevPacketLastLocal;
			prevPacketLastLocal += makePacketSize - prevPacketLastLocal;

			char* procPacket = new char[makePacketSize];
			ZeroMemory(procPacket, makePacketSize);
			memcpy(procPacket, prevPacket, makePacketSize);
			proccessPacket(procPacket);
			ZeroMemory(prevPacket, BUF_SIZE);
			prevPacketLastLocal = 0;
		}
	}
}

void proccessPacket(char* completePacket)
{
	switch (completePacket[1])
	{
	case SC_LOGIN_INFO:
	{
		SC_LOGIN_INFO_PACKET* recvpacket = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(completePacket);
		chessPiecePos = { recvpacket->x, 0, recvpacket->y };
		cout << "Login pos: " << chessPiecePos.x << " , " << chessPiecePos.z << endl;
		myId = recvpacket->id;
	}
	break;
	case SC_ADD_PLAYER:
	{
		SC_ADD_PLAYER_PACKET* recvpacket = reinterpret_cast<SC_ADD_PLAYER_PACKET*>(completePacket);
		DiffClients.try_emplace(recvpacket->id, glm::vec3{ recvpacket->x, 0 ,recvpacket->y });
		cout << "ADD Player: " << recvpacket->id << endl;
	}
	break;
	case SC_REMOVE_PLAYER:
	{
		SC_REMOVE_PLAYER_PACKET* recvpacket = reinterpret_cast<SC_REMOVE_PLAYER_PACKET*>(completePacket);
		DiffClients.erase(recvpacket->id);
		cout << "RMV Player: " << recvpacket->id << endl;
	}
	break;
	case SC_MOVE_PLAYER:
	{
		SC_MOVE_PLAYER_PACKET* recvpacket = reinterpret_cast<SC_MOVE_PLAYER_PACKET*>(completePacket);
		if (recvpacket->id == myId) {
			chessPiecePos.x = recvpacket->x;
			chessPiecePos.z = recvpacket->y;
			cout << "pos: " << chessPiecePos.x << " , " << chessPiecePos.z << endl;
		}
		else {
			DiffClients[recvpacket->id].x = recvpacket->x;
		}	DiffClients[recvpacket->id].z = recvpacket->y;
	}
	break;
	default:
		break;
	}
	delete[] completePacket;
}

void doRecv()
{
	DWORD recvByte = 0;
	DWORD flag = 0;

	EX_Over* newOver = new EX_Over();
	ZeroMemory(&newOver->over, sizeof(newOver->over));
	newOver->WSABuf.buf = newOver->Buf;
	newOver->WSABuf.len = BUF_SIZE;

	newOver->over.hEvent = reinterpret_cast<HANDLE>(newOver);

	if (WSARecv(mySocket, &newOver->WSABuf, 1, &recvByte, &flag, &newOver->over, recv_Callback) != 0 && WSAGetLastError() != WSA_IO_PENDING) {
		LPVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPWSTR)&lpMsgBuf, 0, NULL);
		wcout << "ErrorCode: " << WSAGetLastError() << " - " << (WCHAR*)lpMsgBuf << endl;
		LocalFree(lpMsgBuf);
		if (WSAGetLastError() == WSAECONNRESET || WSAGetLastError() == WSAENOTCONN) {
			exit(0);
		}
	}
	SleepEx(100, TRUE);
}

void doSend(char* sendPacket, int size)
{
	EX_Over* newOver = new EX_Over();
	ZeroMemory(&newOver->over, sizeof(newOver->over));

	memcpy(newOver->Buf, sendPacket, size);

	newOver->WSABuf.buf = newOver->Buf;
	newOver->WSABuf.len = size;

	newOver->over.hEvent = reinterpret_cast<HANDLE>(newOver);

	if (WSASend(mySocket, &newOver->WSABuf, 1, &newOver->WSABuf.len, 0, &newOver->over, send_Callback) != 0) {
		cout << "specialKeyCall() - send fail" << endl;
		display_Err(WSAGetLastError());
	}

	SleepEx(100, TRUE);
}

void CALLBACK recv_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	if (cbTransferred != 0) {
		constructPacket(reinterpret_cast<EX_Over*>(lpOverlapped)->Buf, cbTransferred);
	}
	delete reinterpret_cast<EX_Over*>(lpOverlapped);
	doRecv();
}

void CALLBACK send_Callback(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	cout << "send Byte: " << cbTransferred << endl;
	delete reinterpret_cast<EX_Over*>(lpOverlapped);
}

void display_Err(int Errcode)
{
	LPVOID lpMsgBuf;
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, Errcode, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)&lpMsgBuf, 0, NULL);
	wcout << "ErrorCode: " << Errcode << " - " << (WCHAR*)lpMsgBuf << endl;
	LocalFree(lpMsgBuf);
}
