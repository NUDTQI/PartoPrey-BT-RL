#pragma once
#ifndef PREDATOR_OWNED_STATES_H
#define PREDATOR_OWNED_STATES_H
#include "AgentState.h"

class Predator;
class Prey;

class PatrolState : public CAgentState<Predator>
{

public:

	PatrolState();

	//copy ctor and assignment should be private
	PatrolState(const PatrolState&);
	PatrolState& operator=(const PatrolState&);

	~PatrolState(void);

	virtual bool Enter(void);

	virtual void Execute(void);

	virtual bool IsDone(void);

	virtual bool Exit(void);
};

class AttackState : public CAgentState<Predator>
{

public:

	AttackState(Prey* pAttacked);

	//copy ctor and assignment should be private
	AttackState(const AttackState&);
	AttackState& operator=(const AttackState&);

	~AttackState(void);

	virtual bool Enter(void);

	virtual void Execute(void);

	virtual bool IsDone(void);

	virtual bool Exit(void);

protected:

	Prey* m_pAttacked;				//Agent Attacked
};

#endif