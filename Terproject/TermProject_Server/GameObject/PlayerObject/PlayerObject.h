#pragma once
#include "../GameObject.h"
#include <atomic>
class RecvExpOverBuffer;

class PlayerObject : public GameObject{
private:
	SOCKET				m_socket;
	RecvExpOverBuffer * m_recvOver;
private://player Info
	wstring				m_loginID;
	std::atomic_bool m_isDisconn;
public:
	PlayerObject();
	PlayerObject(int id);
	virtual ~PlayerObject();
private:
	void ClearPlayerObject();
private:
	void DoRecv();
public:
	void RegistGameObject(int id, SOCKET & sock);
	void RegistSocket(SOCKET & sock);

	virtual S_STATE GetPlayerState() override;

	void SetLoginId(char * loginId);
	void SetLoginId(wchar_t * loginId);
	wstring GetLoginId();

	void ConsumeExp(short cExp);

	virtual bool IsAbleAttack() override;
	virtual short AttackedDamage(int attackId, short damage) override;
public:
	void RecvPacket(int ioByte);
public:
	//login
	void SendLoginInfoPacket();
	void SendLoginFailPacket();
	void Disconnect();
	//InGame
	virtual void RemoveViewListPlayer(int removePlayerId) override;
	virtual void MovePlayer(int movePlayerId) override;
	virtual void AddViewListPlayer(int addPlayerId) override;
	void SendMess(int sendId, wchar_t * mess);
	void SendPacket(char * data);
	void SendSkillExecutePacket(unordered_set<int> & viewList);
	//Delay
	void SendDelayPacket();
public:
	//DB
	void SaveData();
};

