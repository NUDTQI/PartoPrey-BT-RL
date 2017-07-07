#include "../fsm/PredatorOwnedStates.h"
#include "../../Predator.h"
#include "../../SensorMemory.h"
#include "../../SteeringBehaviors.h"
#include "../../GameWorld.h"
#include "../../Judger.h"

//--------------------------------------methods for PatrolState of predator
PatrolState::PatrolState()
{
	m_StateName = L"Patrol_AgentState";
}


PatrolState::~PatrolState( void )
{

}

bool PatrolState::Enter()
{
	//if (m_pOwner->GetSensorMem()->m_MemoryMapPreys.empty())
	{
		m_pOwner->GetSteering()->WanderOn();
		return true;
	}
	return false;
}

void PatrolState::Execute()
{
	if (!m_pOwner->GetSensorMem()->m_MemoryMapPreys.empty())
	{
		m_bIsDone = true;
	}
}

bool PatrolState::Exit()
{
	m_pOwner->GetSteering()->WanderOff();
	return m_bIsDone;
}


bool PatrolState::IsDone( void )
{
	return m_bIsDone;
}

//  [1/3/2017 zq]---methods for AttackState of predator

AttackState::AttackState(Prey* pAttacked)
{
	if (NULL != pAttacked)
	{
		m_pAttacked = pAttacked;
	}
	m_ExecuteTime = 0;

	m_StateName = L"Attack_AgentState";
}

bool AttackState::Enter()
{
	if (NULL!=m_pAttacked && m_pAttacked->isAlive() && !m_pOwner->GetSteering()->isPursuitOn())
	{
		m_pOwner->GetSteering()->PursuitOn(m_pAttacked);
		return true;
	}
	return false;
}

void AttackState::Execute()
{
	m_ExecuteTime++;

	if (NULL!=m_pAttacked && m_pAttacked->isAlive())
	{
		m_pOwner->GetWorld()->GetJudger()->PtoPreyInterTable.insert(std::make_pair(m_pOwner,m_pAttacked));
		//m_pOwner->GetWorld()->GetJudger()->JudgeAttackPToPrey(m_pOwner,m_pAttacked);

		if (m_pAttacked->InHavenZone())
		{
			m_pOwner->GetSteering()->PursuitOff();
		}
		else
		{
			m_pOwner->GetSteering()->PursuitOn(m_pAttacked);
		}
	}

	//the attacked prey is dead or lost in predator's sensor range, change to patrol state
	if (!m_pAttacked || m_pAttacked->isDead() || m_pAttacked->InHavenZone() || Vec2DDistanceSq(m_pOwner->Pos(),m_pAttacked->Pos())>m_pOwner->SensorRange()*m_pOwner->SensorRange())
	{
		m_bIsDone = true;
	}
}

bool AttackState::Exit()
{
	m_pOwner->GetSteering()->PursuitOff();
	m_pOwner->GetSteering()->SetTargetAgent1(NULL);
	m_pAttacked = NULL;
	return true;
}

AttackState::~AttackState( void )
{

}

bool AttackState::IsDone( void )
{
	return m_bIsDone;
}
