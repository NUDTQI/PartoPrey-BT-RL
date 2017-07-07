#include "Predator.h"
#include "misc/Cgdi.h"
#include <cassert>
#include "SensorMemory.h"
#include "SteeringBehaviors.h"
#include "Prey.h"



Predator::Predator(GameWorld* world,
					Vector2D position,
					double    rotation,
					Vector2D velocity,
					double    mass,
					double    max_force,
					double    max_speed,
					double    max_turn_rate,
					double    scale,			  
					double    ViewDistance,
					int       max_Health):		Agent(world,
					position,
													rotation,
													velocity,
													mass,
													max_force,
													max_speed,
													max_turn_rate,
													scale,			  
													ViewDistance,
													max_Health)
{
	m_pStateMachine = new CStateMachine<Predator>(this);
	
	GetSteering()->WallAvoidanceOn();
	GetSteering()->HavenAvoidanceOn();
}


Predator::~Predator(void)
{
	if (m_pStateMachine!=NULL)
	{
		delete m_pStateMachine;
		m_pStateMachine = NULL;
	}
}


void Predator::SelectAction()
{
	m_pStateMachine->Update();

	if (GetSensorMem()->m_MemoryMapPreys.empty())
	{
		Patrol();
	}
	else
	{
		int size = GetSensorMem()->m_MemoryMapPreys.size();
		int randIndex = RandFloat()*size;

		Prey* prey = dynamic_cast<Prey*>(GetSensorMem()->m_MemoryMapPreys[randIndex]);
		if (NULL != prey && prey->isAlive() && !prey->InHavenZone())
		{
			Attack(prey);
		}
		else
		{
			Patrol();
		}
	}
}

void Predator::Patrol()
{
	PatrolState* pNewState = new PatrolState();
	pNewState->m_pOwner = this;
	pNewState->m_priority = EStatePriority_Minimal;

	bool b = m_pStateMachine->ChangeState(pNewState);
	if (!b)
	{
		delete pNewState;
		pNewState = NULL;
	}
}

void Predator::Attack(Prey* preyAttacked)
{
	assert(preyAttacked);

	AttackState* pNewState = new AttackState(preyAttacked);
	pNewState->m_pOwner = this;
	pNewState->m_priority = EStatePriority_Minimal;

	bool b = m_pStateMachine->ChangeState(pNewState);
	if (!b)
	{
		delete pNewState;
		pNewState = NULL;
	}

}

void Predator::Render()
{
	//a vector to hold the transformed vertices
	static std::vector<Vector2D>  m_vecVehicleVBTrans;

	if (isAlive())
	{
		gdi->RedPen(); 
	}
	else if (isDead())
	{
		gdi->GreyPen();
	}
	gdi->HollowBrush();

	m_vecVehicleVBTrans = WorldTransform(m_vecAgentVB,
		Pos(),
		Heading(),
		Side(),
		Scale());

	gdi->ClosedShape(m_vecVehicleVBTrans);
}

std::vector<Agent*>& Predator::GetAllyBotsInRange()
{
	return GetSensorMem()->m_MemoryMapPredators;
}
