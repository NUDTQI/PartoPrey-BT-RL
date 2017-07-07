#pragma once
#include "Agent.h"
#include "AI/FSM/StateMachine.h"
#include "AI/FSM/PredatorOwnedStates.h"

class PatrolState;
class AttackState;

class Predator :
	public Agent
{
public:
	Predator(GameWorld* world,
		Vector2D position,
		double    rotation,
		Vector2D velocity,
		double    mass,
		double    max_force,
		double    max_speed,
		double    max_turn_rate,
		double    scale,			  
		double    ViewDistance,
		int       max_Health);
	~Predator(void);
public:
	CStateMachine<Predator>* m_pStateMachine; 

	//get Ally Bots in Range
	virtual std::vector<Agent*>& GetAllyBotsInRange();

public:
	//update bot's state in game
	//void Update(double time_elapsed);

	//select action
	void        SelectAction();

	//define behaviors of preadator
	void Patrol();
	void Attack(Prey* preyAttacked);

	//render
	void Render();
};

