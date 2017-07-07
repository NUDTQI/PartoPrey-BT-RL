#pragma once

#include <vector>
#include <list>
#include "2D/vector2d.h"

class Agent;
class Predator;
class Prey;
class BaseGameEntity;
class FoodZone;

class SensorMemory
{
public:
	SensorMemory(Agent* owner);
	~SensorMemory(void);

private:

	//the owner of this instance
	Agent* m_pOwner;

public:
	//this method iterates through all the opponents in the game world and 
	//updates the records of those that are in the owner's FOV
	void     UpdateVision();

	//get Ally Bots in Range
	//std::vector<Agent*>& GetAllyBotsInRange()
	//{
	//	if (typeid(*m_pOwner) == typeid(Predator))
	//	{
	//		return m_MemoryMapPredators;
	//	}
	//	else if (typeid(*m_pOwner) == typeid(Prey))
	//	{
	//		return m_MemoryMapPreys;
	//	}
	//	else
	//	{
	//		return m_AllBotsInRange;
	//	}
	//}

	//container to store predators and preys sensed
	std::vector<Agent*>  m_MemoryMapPredators;
	std::vector<Agent*>  m_MemoryMapPreys;
	std::vector<Agent*>  m_AllBotsInRange;
	Predator*   pNearestPredator;
	Prey*   pNearestPrey;
	BaseGameEntity*   pNearestFood;
	BaseGameEntity*   pNearestHaven;
};

