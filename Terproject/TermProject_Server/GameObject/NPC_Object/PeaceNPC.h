#pragma once
#include "NPC_Object.h"
class PeaceNPC : public NPC_Object
{
	atomic_bool m_isAggro = false;
private:
	bool AcitiveAggro();
	bool InacitiveAggro();
public:
	PeaceNPC() :NPC_Object() {}
	PeaceNPC(int id) :NPC_Object(id) {}
	virtual void RandMove()override;
	virtual void ChaseMove(int targetId)override;
	virtual short AttackedDamage(int attackId, short damage) override;
	void RespawnData();
};

