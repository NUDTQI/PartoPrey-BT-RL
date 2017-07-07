#pragma once
#include "Prey.h"
#include "AI/rl/State.h"

//该文件需要用户为所有需要学习的行为树节点定义自身的状态类，继承字基类CState

//Health (None, Low, Medium, High)
enum HealthLevel{NonHl,LowHl,MediumHl,HighHl};

//Number of ally neighbours (None, Low, Medium, High);
enum NumAllyLevel{NoneAlly,LowAlly,MidAlly,HighAlly};

//Distance to nearest Food (Inside, Near, Medium, Far);
//Distance to nearest Haven (Inside, Near, Medium, Far);
//Distance to nearest Predator (Inside, Near, Medium, Far).
enum DisLevel{Inside,Near,Middle,None};

//不建立公共的离散类，是因为对于不同层次的学习节点，即便是同一变量，如Health，也可能对其离散化精细
//程度不同，因此此处还是各自离散化获得自己的类。虽然可能会有重复.

class StateRootBevSel 
	: public CState
{
private:
	//state,生命状况、队友数量、对最近的敌人距离、距离安全区域的距离
	HealthLevel m_eHl;
	NumAllyLevel m_eNumAlly;
	DisLevel m_eDisToEnemy;
	DisLevel m_eDisToHaven;

public:
	StateRootBevSel(HealthLevel hl,NumAllyLevel NumAlly,DisLevel DisToEnemy,DisLevel DisToHaven){m_eHl = hl;m_eNumAlly = NumAlly;m_eDisToEnemy=DisToEnemy;m_eDisToHaven=DisToHaven;}
	StateRootBevSel(){m_eHl = HighHl;m_eNumAlly = NoneAlly;m_eDisToEnemy = None;m_eDisToHaven = None;}
	~StateRootBevSel(){}

	virtual bool IsSameState(CState *pS)
	{
		StateRootBevSel* plocal = dynamic_cast<StateRootBevSel*>(pS);

		if(plocal->GetHealthlevel()==m_eHl && plocal->GetNumAllyNeighbor()==m_eNumAlly && plocal->GetDisToEnemy()==m_eDisToEnemy && plocal->GetDisToHaven()==m_eDisToHaven)
			return true;

		return false;
	}

	virtual void CopyState(CState* pS)
	{
		StateRootBevSel* plocal = dynamic_cast<StateRootBevSel*>(pS);

		m_eHl = plocal->GetHealthlevel();
		m_eNumAlly = plocal->GetNumAllyNeighbor();
		m_eDisToEnemy = plocal->GetDisToEnemy();
		m_eDisToHaven = plocal->GetDisToHaven();
	}

	virtual void UpdateDiscreteState(Prey* owner)
	{
		m_eHl = (HealthLevel)PubDiscreter.CalculateVarDiscrete(owner->Health(),80,30,1);
		m_eNumAlly = (NumAllyLevel)PubDiscreter.CalculateVarDiscrete(owner->getNumAllyNeighbour(),4,2,1);
		m_eDisToEnemy = (DisLevel)PubDiscreter.CalculateVarDiscrete(owner->getDisToNearestPredator(),owner->SensorRange(),15,5);
		m_eDisToHaven = (DisLevel)PubDiscreter.CalculateVarDiscrete(owner->getDisToNearestHaven(),owner->SensorRange(),30,20-owner->Scale().x);
	
		//deal with conputation difference on the edge
		if(owner->InHavenZone())
		{
			m_eDisToHaven = DisLevel::Inside;
		}
		if(!owner->InHavenZone() && m_eDisToHaven==DisLevel::Inside)
		{
			m_eDisToHaven = DisLevel::Near;
		}
	}

	virtual std::vector<int>& PrintStateValues()
	{
		std::vector<int> out;
		out.push_back(int(m_eHl));
		out.push_back(int(m_eNumAlly));
		out.push_back(int(m_eDisToEnemy));
		out.push_back(int(m_eDisToHaven));
		return out;
	}

	HealthLevel GetHealthlevel(){return m_eHl;}
	DisLevel GetDisToEnemy(){return m_eDisToEnemy;}
	DisLevel GetDisToHaven(){return m_eDisToHaven;}
	NumAllyLevel GetNumAllyNeighbor(){return m_eNumAlly;}
};

class StateRetreatBevSel 
	: public CState
{
private:
	//state,自身的健康状况、距离安全区域的距离、与敌方的距离
	HealthLevel m_eHealth;
	DisLevel m_eDisNearestHaven;
	DisLevel m_eDisToEnemy;

public:
	StateRetreatBevSel(DisLevel DisToHaven,HealthLevel hl,DisLevel DisToEnemy){m_eDisNearestHaven = DisToHaven;m_eHealth=hl;m_eDisToEnemy=DisToEnemy;}
	StateRetreatBevSel(){m_eDisNearestHaven = None;m_eHealth = HighHl;m_eDisToEnemy = None;}
	~StateRetreatBevSel(){}

	virtual bool IsSameState(CState *pS)
	{
		StateRetreatBevSel* plocal = dynamic_cast<StateRetreatBevSel*>(pS);

		if(plocal->GetDisToHaven()==m_eDisNearestHaven  && plocal->GetHealth()==m_eHealth && plocal->GetDisToEnemy()==m_eDisToEnemy)
			return true;

		return false;
	}

	virtual void CopyState(CState* pS)
	{
		StateRetreatBevSel* plocal = dynamic_cast<StateRetreatBevSel*>(pS);

		m_eDisNearestHaven = plocal->GetDisToHaven();
		m_eHealth = plocal->GetHealth();
		m_eDisToEnemy = plocal->GetDisToEnemy();
	}

	virtual void UpdateDiscreteState(Prey* owner)
	{
		m_eHealth = (HealthLevel)PubDiscreter.CalculateVarDiscrete(owner->Health(),80,30,1);
		m_eDisToEnemy = (DisLevel)PubDiscreter.CalculateVarDiscrete(owner->getDisToNearestPredator(),owner->SensorRange(),15,5);
		m_eDisNearestHaven = (DisLevel)PubDiscreter.CalculateVarDiscrete(owner->getDisToNearestHaven(),owner->SensorRange(),30,20-owner->Scale().x);
	
		//deal with conputation difference on the edge
		if(owner->InHavenZone())
		{
			m_eDisNearestHaven = DisLevel::Inside;
		}
		if(!owner->InHavenZone() && m_eDisNearestHaven==DisLevel::Inside)
		{
			m_eDisNearestHaven = DisLevel::Near;
		}
	}

	virtual std::vector<int>& PrintStateValues()
	{
		std::vector<int> out;
		out.push_back(int(m_eHealth));
		out.push_back(int(m_eDisNearestHaven));
		out.push_back(int(m_eDisToEnemy));
		return out;
	}

	HealthLevel GetHealth(){return m_eHealth;}
	DisLevel GetDisToHaven(){return m_eDisNearestHaven;}
	DisLevel GetDisToEnemy(){return m_eDisToEnemy;}
};

class StateAttackBevSel 
	: public CState
{
private:
	//state,生命状况、队友数量
	HealthLevel m_eHl;
	NumAllyLevel m_eNumAlly;

public:
	StateAttackBevSel(HealthLevel hl,NumAllyLevel NumAlly){m_eHl = hl;m_eNumAlly = NumAlly;}
	StateAttackBevSel(){m_eHl = HighHl;m_eNumAlly = NoneAlly;}
	~StateAttackBevSel(){}

	virtual bool IsSameState(CState *pS)
	{
		StateAttackBevSel* plocal = dynamic_cast<StateAttackBevSel*>(pS);

		if(plocal->GetHealthlevel()==m_eHl && plocal->GetNumAllyNeighbor()==m_eNumAlly)
			return true;

		return false;
	}

	virtual void CopyState(CState* pS)
	{
		StateAttackBevSel* plocal = dynamic_cast<StateAttackBevSel*>(pS);

		m_eHl = plocal->GetHealthlevel();
		m_eNumAlly = plocal->GetNumAllyNeighbor();
	}

	virtual void UpdateDiscreteState(Prey* owner)
	{
		m_eHl = (HealthLevel)PubDiscreter.CalculateVarDiscrete(owner->Health(),80,30,1);
		m_eNumAlly = (NumAllyLevel)PubDiscreter.CalculateVarDiscrete(owner->getNumAllyNeighbour(),4,2,1);
	}

	virtual std::vector<int>& PrintStateValues()
	{
		std::vector<int> out;
		out.push_back(int(m_eHl));
		out.push_back(int(m_eNumAlly));
		return out;
	}

	HealthLevel GetHealthlevel(){return m_eHl;}
	NumAllyLevel GetNumAllyNeighbor(){return m_eNumAlly;}
};

//General state for flat Q-learning
class StateGeneralBevSel 
	: public CState
{
private:
	//state,生命状况、队友数量、对最近的敌人距离、距离安全区域的距离、距离食物区域的距离
	HealthLevel m_eHl;
	NumAllyLevel m_eNumAlly;
	DisLevel m_eDisToEnemy;
	DisLevel m_eDisToHaven;
	DisLevel m_eDisToFood;
	
public:
	StateGeneralBevSel(HealthLevel hl,NumAllyLevel NumAlly,DisLevel DisToEnemy,DisLevel DisToHaven,DisLevel DisToFood){m_eHl = hl;m_eNumAlly = NumAlly;m_eDisToEnemy=DisToEnemy;m_eDisToHaven=DisToHaven;m_eDisToFood=DisToFood;}
	StateGeneralBevSel(){m_eHl = HighHl;m_eNumAlly = NoneAlly;m_eDisToEnemy = None;m_eDisToHaven = None;m_eDisToFood = None;}
	~StateGeneralBevSel(){}

	virtual bool IsSameState(CState *pS)
	{
		StateGeneralBevSel* plocal = dynamic_cast<StateGeneralBevSel*>(pS);

		if(plocal->GetHealthlevel()==m_eHl && plocal->GetNumAllyNeighbor()==m_eNumAlly && plocal->GetDisToEnemy()==m_eDisToEnemy && plocal->GetDisToHaven()==m_eDisToHaven && plocal->GetDisToFood()==m_eDisToFood)
			return true;

		return false;
	}

	virtual void CopyState(CState* pS)
	{
		StateGeneralBevSel* plocal = dynamic_cast<StateGeneralBevSel*>(pS);

		m_eHl = plocal->GetHealthlevel();
		m_eNumAlly = plocal->GetNumAllyNeighbor();
		m_eDisToEnemy = plocal->GetDisToEnemy();
		m_eDisToHaven = plocal->GetDisToHaven();
		m_eDisToFood = plocal->GetDisToFood();
	}

	virtual void UpdateDiscreteState(Prey* owner)
	{
		m_eHl = (HealthLevel)PubDiscreter.CalculateVarDiscrete(owner->Health(),80,30,1);
		m_eNumAlly = (NumAllyLevel)PubDiscreter.CalculateVarDiscrete(owner->getNumAllyNeighbour(),4,2,1);
		m_eDisToEnemy = (DisLevel)PubDiscreter.CalculateVarDiscrete(owner->getDisToNearestPredator(),owner->SensorRange(),15,5);
		m_eDisToHaven = (DisLevel)PubDiscreter.CalculateVarDiscrete(owner->getDisToNearestHaven(),owner->SensorRange(),30,20-owner->Scale().x);
		m_eDisToFood = (DisLevel)PubDiscreter.CalculateVarDiscrete(owner->getDisToNearestFood(),owner->SensorRange(),30,20-owner->Scale().x);

		//deal with conputation difference on the edge
		if(owner->InHavenZone())
		{
			m_eDisToHaven = DisLevel::Inside;
		}
		if(owner->InFoodZone())
		{
			m_eDisToFood = DisLevel::Inside;
		}
		if(!owner->InHavenZone() && m_eDisToHaven==DisLevel::Inside)
		{
			m_eDisToHaven = DisLevel::Near;
		}
		if(!owner->InFoodZone() && m_eDisToFood==DisLevel::Inside)
		{
			m_eDisToFood = DisLevel::Near;
		}
	}

	virtual std::vector<int>& PrintStateValues()
	{
		output.clear();
		output.push_back(int(m_eHl));
		output.push_back(int(m_eNumAlly));
		output.push_back(int(m_eDisToEnemy));
		output.push_back(int(m_eDisToHaven));
		output.push_back(int(m_eDisToFood));
		return output;
	}

	HealthLevel GetHealthlevel(){return m_eHl;}
	DisLevel GetDisToEnemy(){return m_eDisToEnemy;}
	DisLevel GetDisToHaven(){return m_eDisToHaven;}
	DisLevel GetDisToFood(){return m_eDisToFood;}
	NumAllyLevel GetNumAllyNeighbor(){return m_eNumAlly;}
};