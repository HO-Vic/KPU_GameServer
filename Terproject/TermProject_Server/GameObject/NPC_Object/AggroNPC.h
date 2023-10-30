#pragma once
#include "NPC_Object.h"
class AggroNPC : public NPC_Object
{
public:
	AggroNPC() :NPC_Object() {}
	AggroNPC(int id) :NPC_Object(id) {}
	virtual void RandMove() override;
	virtual void ChaseMove(int targetId) override;
};

