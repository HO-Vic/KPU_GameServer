#pragma once
#include "../GameObject.h"

class RecvExpOverBuffer;

class PlayerObject : public GameObject
{
private:
	SOCKET				m_socket;
	RecvExpOverBuffer* m_recvOver;
private:
	mutex				m_stateLock;	
private://player Info
	wstring				m_loginID;
public:
	PlayerObject();
	PlayerObject(int id);
	virtual ~PlayerObject();
private:
	void DoRecv();
public:
	void RegistGameObject(int id, SOCKET& sock);
	void RegistSocket(SOCKET& sock);
	void SetLoginId(char* loginId);
	void SetLoginId(wchar_t* loginId);
	wstring GetLoginId();
	virtual S_STATE GetPlayerState() override;
	void ConsumeExp(short cExp);
public:	
	void RecvPacket(int ioByte);
public:
	void SendLoginInfoPacket();
	virtual void RemoveViewListPlayer(int removePlayerId) override;
	virtual void MovePlayer(int movePlayerId) override;
	virtual void AddViewListPlayer(int addPlayerId) override;
public:
	virtual short AttackedDamage(short damage) override;
	virtual bool IsAbleAttack() override;
public:
	void SendStatPacket(char* data);
	void SaveData();
};

