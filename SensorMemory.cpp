#include "SensorMemory.h"
#include "GameWorld.h"
#include "Agent.h"
#include "Zone.h"
#include "GameMap.h"


SensorMemory::SensorMemory(Agent* owner)
{
	m_pOwner = owner;
	pNearestPredator = NULL;
	pNearestPrey =NULL;
	pNearestFood = NULL;
	pNearestHaven = NULL;
}


SensorMemory::~SensorMemory(void)
{
}

void SensorMemory::UpdateVision()
{
	//first of all clear all the sensor memory of last time, it is simple now not to consider long memory
	m_AllBotsInRange.clear();
	m_MemoryMapPredators.clear();
	m_MemoryMapPreys.clear();
	pNearestPredator = NULL;
	pNearestPrey = NULL;
	pNearestHaven = NULL;
	pNearestFood = NULL;

	//store all the bots in sensor range, store predators and preys in FOV and sensor range individually,record the nearest predator and prey
	double ClosestSoFar = MaxDouble;
	std::vector<Predator*>::const_iterator curPredator = m_pOwner->GetWorld()->GetPredatorBots().begin();
	for (curPredator; curPredator != m_pOwner->GetWorld()->GetPredatorBots().end(); ++curPredator)
	{
		//make sure time is not wasted checking against the same bot or against a
		// bot that is dead or re-spawning
		if (*curPredator == m_pOwner ||  !(*curPredator)->isAlive()) continue;

		double dist = Vec2DDistanceSq(m_pOwner->Pos(), (*curPredator)->Pos());
		
		//first of all in bot's sensor range
		if (dist < m_pOwner->SensorRange()*m_pOwner->SensorRange())
		{
			m_AllBotsInRange.push_back(*curPredator);

			//second test to see if this bot is within the FOV
			if (isSecondInFOVOfFirst(m_pOwner->Pos(),
				m_pOwner->Facing(),
				(*curPredator)->Pos(),
				m_pOwner->FieldOfView()))
			{
				//thirdly record the nearest predator and record all the preys in range as order from near to far
				if (dist < ClosestSoFar)
				{
					ClosestSoFar = dist;
					pNearestPredator = *curPredator;
					m_MemoryMapPredators.insert(m_MemoryMapPredators.begin(),*curPredator);
				}
				else
				{
					m_MemoryMapPredators.push_back(*curPredator);
				}
			}
		}
	}

	ClosestSoFar = MaxDouble;
	std::vector<Prey*>::const_iterator curPrey = m_pOwner->GetWorld()->GetPreyBots().begin();
	for (curPrey; curPrey != m_pOwner->GetWorld()->GetPreyBots().end(); ++curPrey)
	{
		//make sure time is not wasted checking against the same bot or against a
		// bot that is dead or re-spawning
		if (*curPrey == m_pOwner ||  !(*curPrey)->isAlive()) continue;

		double dist = Vec2DDistanceSq(m_pOwner->Pos(), (*curPrey)->Pos());

		if (dist < m_pOwner->SensorRange()*m_pOwner->SensorRange())
		{
			m_AllBotsInRange.push_back(*curPrey);
			//second test to see if this bot is within the FOV
			if (isSecondInFOVOfFirst(m_pOwner->Pos(),
				m_pOwner->Facing(),
				(*curPrey)->Pos(),
				m_pOwner->FieldOfView()))
			{
				//thirdly record the nearest prey and record all the preys in range as order from near to far
				if (dist < ClosestSoFar)
				{
					ClosestSoFar = dist;
					pNearestPrey = *curPrey;
					m_MemoryMapPreys.insert(m_MemoryMapPreys.begin(),*curPrey);
				}
				else
				{
					m_MemoryMapPreys.push_back(*curPrey);
				}
			}
		}
	}

	std::vector<BaseGameEntity*>::const_iterator curHaven = m_pOwner->GetWorld()->GetGameMap()->GetHavenZones().begin();
	ClosestSoFar = MaxDouble;
	for (curHaven; curHaven != m_pOwner->GetWorld()->GetGameMap()->GetHavenZones().end(); ++curHaven)
	{
		double dist =  Vec2DDistanceSq(m_pOwner->Pos(), (*curHaven)->Pos()); 
		if (dist<m_pOwner->SensorRange()*m_pOwner->SensorRange() && dist<ClosestSoFar)
		{
			ClosestSoFar = dist;
			pNearestHaven = *curHaven;
		}
	}
	
	std::vector<BaseGameEntity*>::const_iterator curFood = m_pOwner->GetWorld()->GetGameMap()->GetFoodZones().begin();
	ClosestSoFar = MaxDouble;
	for (curFood; curFood != m_pOwner->GetWorld()->GetGameMap()->GetFoodZones().end(); ++curFood)
	{
		double dist =  Vec2DDistanceSq(m_pOwner->Pos(), (*curFood)->Pos()); 
		if (dist<m_pOwner->SensorRange()*m_pOwner->SensorRange() && dist<ClosestSoFar)
		{
			ClosestSoFar = dist;
			pNearestFood = *curFood;
		}
	} 
}
