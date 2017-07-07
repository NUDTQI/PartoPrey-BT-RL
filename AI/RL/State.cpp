#include "State.h"

CLocalState::CLocalState(void)
{
	Health = HealthLevel::High;
	NumAllyNeigh = NumAllyLevel::N;
	DisNearestFood = DisLevel::Far;
	DisNearestHaven = DisLevel::Far;
	DisNearestEnemy = DisLevel::Far;
}

CLocalState::CLocalState(HealthLevel iHealth,NumAllyLevel iNumAllyNeigh,DisLevel iDisNearestFood,DisLevel iDisNearestHaven,DisLevel iDisNearestEnemy)
{
	Health = iHealth;
	NumAllyNeigh = iNumAllyNeigh;
	DisNearestFood = iDisNearestFood;
	DisNearestHaven = iDisNearestHaven;
	DisNearestEnemy = iDisNearestEnemy;
}

CLocalState::~CLocalState(void)
{
}

void CLocalState::updateDiscreteState(Prey* pPrey)
{
	
	setHealthlevel(pPrey->Health());

	setNumAllyNeighbor(pPrey->getNumAllyNeighbour());

	setdistoEnemy(pPrey->getDisToNearestPredator());

	setdistoFood(pPrey->getDisToNearestFood());

	setdistoHaven(pPrey->getDisToNearestHaven());

}


bool CLocalState::IsSameState(CLocalState *pS)
{
	if (Health==pS->Health && NumAllyNeigh==pS->NumAllyNeigh && DisNearestFood==pS->DisNearestFood
		&& DisNearestEnemy==pS->DisNearestEnemy && DisNearestHaven==pS->DisNearestHaven)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void CLocalState::copyState(CLocalState* pS)
{
	Health = pS->Health;
	NumAllyNeigh = pS->getNumAllyNeighbor(); 
	DisNearestEnemy = pS->getdistoEnemy();
	DisNearestHaven = pS->getdistoHaven();
	DisNearestFood = pS->getdistoFood();
}

void CLocalState::setHealthlevel(int hl)
{
	if (hl >= healthHigh)
	{
		Health = HealthLevel::High;
	}
	else if(hl >= healthMedium)
	{
		Health = HealthLevel::Medium;
	}
	else if(hl >= healthLow)
	{
		Health = HealthLevel::Low;
	}
	else
	{
		Health = HealthLevel::Non;
	}
}

void CLocalState::setNumAllyNeighbor(int num)
{
	if (num >= numAllyHigh)
	{
		NumAllyNeigh = NumAllyLevel::H;
	}
	else if(num >= numAllyMedium)
	{
		NumAllyNeigh = NumAllyLevel::Mid;
	}
	else if(num >= numAllyLow)
	{
		NumAllyNeigh = NumAllyLevel::L;
	}
	else
	{
		NumAllyNeigh = NumAllyLevel::N;
	}
}

DisLevel CLocalState::calculateDisDiscrete( double distance )
{
	if (distance >= distanceMax)
	{
		return DisLevel::Far;
	}
	else if (distance > distanceMedium)
	{
		return DisLevel::Middle;
	}
	else if(distance > distanceNear)
	{
		return DisLevel::Near;
	}
	else
	{
		return DisLevel::Inside;
	}
}
