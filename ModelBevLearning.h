#pragma once
#include "Prey.h"
#include "AI/rl/EnvModel.h"
#include "AI/rl/Action.h"
#include "AI/rl/CPrimitiveAction.h"
#include "AI/rl/State.h"
#include "StateBevLearning.h"

//该文件需要用户为所有需要学习的行为树节点定义其奖励函数模型，对应所设计的状态类
//由于可以统一的将可选动作定义为子节点的序号，因此动作模型无需实例化


//动作1表示retreat节点；动作2表示attack；动作3表示idle
class ModelSelRootBevLearning
	: public EnvModelLearning
{
public:

	ModelSelRootBevLearning(Prey* pA)
		: EnvModelLearning(pA){}

	~ModelSelRootBevLearning(void){}

	StateRootBevSel temp;

public:

	virtual CState* UpdateCurState()
	{
		temp.UpdateDiscreteState(m_pOwner);
		return &temp;
	}

	virtual void SetCurAction(CAction* pA)
	{
		pAction->setAction(pA->getAction());
	}

	virtual CState* MemyCopyState(CState* temp)
	{
		StateRootBevSel* ps = new StateRootBevSel();
		ps->CopyState(temp);
		return ps;
	}


	//本函数主要计算第一部分自身定义的状态动作空间的伪奖励值
	virtual float CalculateRewardsEachStep()
	{
		StateRootBevSel* pS = dynamic_cast<StateRootBevSel*>(pState);
		StateRootBevSel* preS = dynamic_cast<StateRootBevSel*>(preState);

		////////////////以下为Root节点空间内定义的单步伪奖励值函数//////////////////////////////////////////////////////////
		float pseudoReward = 0;
		if (NonHl==pS->GetHealthlevel())
		{
			pseudoReward = pseudoReward - 10;
		}
		if (Inside==pS->GetDisToHaven())
		{
			pseudoReward = pseudoReward + 15;
		}

		//没有敌人，没有发现haven选择撤退惩罚
		if (None==pS->GetDisToHaven() && None==pS->GetDisToEnemy() && preAction->getAction()==0)
		{
			pseudoReward = pseudoReward - 0.5;
		}

		//没有敌人，选择进攻惩罚
		if (None==pS->GetDisToEnemy() && preAction->getAction()==2)
		{
			pseudoReward = pseudoReward - 0.5;
		}

		//离敌人不远，选择漫游惩罚
		if (Middle>pS->GetDisToEnemy() && preAction->getAction()==1)
		{
			pseudoReward = pseudoReward - 0.5;
		}

		//血量增加，奖励
		if (pS->GetHealthlevel() > preS->GetHealthlevel())
		{
			pseudoReward = pseudoReward + 2;
		}

		if (LowHl == pS->GetHealthlevel())
		{
			if (Near==pS->GetDisToEnemy() && preAction->getAction()==0)
			{
				pseudoReward = pseudoReward + 0.5;
			}
		}

		//队友多血量高鼓励进攻
		if(HighHl==pS->GetHealthlevel() && LowAlly<pS->GetNumAllyNeighbor() && None>pS->GetDisToEnemy() && preAction->getAction()==2)
		{
			pseudoReward = pseudoReward + 0.5;
		}		
		return pseudoReward;
	}
};

class ModelSelRetreatBevLearning
	: public EnvModelLearning
{
public:

	ModelSelRetreatBevLearning(Prey* pA)
		: EnvModelLearning(pA){}

	~ModelSelRetreatBevLearning(void){}

	StateRetreatBevSel temp;

public:

	virtual CState* UpdateCurState()
	{
		temp.UpdateDiscreteState(m_pOwner);
		return &temp;
	}

	virtual void SetCurAction(CAction* pA)
	{
		pAction->setAction(pA->getAction());
	}

	virtual CState* MemyCopyState(CState* temp)
	{
		StateRetreatBevSel* ps = new StateRetreatBevSel();
		ps->CopyState(temp);
		return ps;
	}

	virtual float CalculateRewardsEachStep()
	{
		StateRetreatBevSel* pS = dynamic_cast<StateRetreatBevSel*>(pState);

		////////////////以下为retreat节点空间内定义的单步伪奖励值函数//////////////////////////////////////////////////////////
		float pseudoReward = 0;
		//对于撤退行为，奖励考虑自身的生命情况和到达安全区域的情况
		if (NonHl == pS->GetHealth())
		{
			pseudoReward = pseudoReward - 10;
		}

		if (Inside == pS->GetDisToHaven())
		{
			pseudoReward = pseudoReward + 15;
		}

		//没有Haven，选择到达Haven惩罚
		if (None==pS->GetDisToHaven() && preAction->getAction()==1)
		{
			pseudoReward = pseudoReward - 0.5;
		}
		return pseudoReward;
	}
};

class ModelSelAttackBevLearning
	: public EnvModelLearning
{
public:

	ModelSelAttackBevLearning(Prey* pA)
		: EnvModelLearning(pA){}

	~ModelSelAttackBevLearning(void){}

	StateAttackBevSel temp;

public:

	virtual CState* UpdateCurState()
	{
		temp.UpdateDiscreteState(m_pOwner);
		return &temp;
	}

	virtual void SetCurAction(CAction* pA)
	{
		pAction->setAction(pA->getAction());
	}

	virtual CState* MemyCopyState(CState* temp)
	{
		StateAttackBevSel* ps = new StateAttackBevSel();
		ps->CopyState(temp);
		return ps;
	}

	virtual float CalculateRewardsEachStep()
	{
		float pseudoReward = 0; 
		StateAttackBevSel* pS = dynamic_cast<StateAttackBevSel*>(pState);

		////////////////以下为attack节点空间内定义的单步伪奖励值函数//////////////////////////////////////////////////////////
		//float pseudoReward = 0;
		//对于进攻行为，奖励考虑自身的生命情况和到达安全区域的情况

		if (NonHl == pS->GetHealthlevel())
		{
			pseudoReward = pseudoReward - 10;
		}

		if (m_pOwner->GetNearestPredator() && m_pOwner->GetNearestPredator()->isDead())
		{
			pseudoReward = pseudoReward + 2;
		}

		return pseudoReward;
	}
};


//general learning 
class ModelSelGeneralBevLearning
	: public EnvModelLearning
{
public:

	ModelSelGeneralBevLearning(Prey* pA)
		: EnvModelLearning(pA){}

	~ModelSelGeneralBevLearning(void){}

	StateGeneralBevSel temp;

public:

	virtual CState* UpdateCurState()
	{
		temp.UpdateDiscreteState(m_pOwner);
		return &temp;
	}

	virtual void SetCurAction(CAction* pA)
	{
		pAction->setAction(pA->getAction());
	}

	virtual CState* MemyCopyState(CState* temp)
	{
		StateGeneralBevSel* ps = new StateGeneralBevSel();
		ps->CopyState(temp);
		return ps;
	}

	virtual float CalculateRewardsEachStep()
	{
		StateGeneralBevSel* pS = dynamic_cast<StateGeneralBevSel*>(pState);
		StateGeneralBevSel* preS = dynamic_cast<StateGeneralBevSel*>(preState);

		////////////////以下为Root节点空间内定义的单步伪奖励值函数//////////////////////////////////////////////////////////
		float reward = 0.0f;
		if (HealthLevel::NonHl == pS->GetHealthlevel())
		{
			reward = reward - 200;
		}

		if (DisLevel::Inside == pS->GetDisToHaven())
		{
			reward = reward + 200;
		}

		if (m_pOwner->m_CurEnemy && m_pOwner->m_CurEnemy->isDead())
		{
			reward = reward + 10;
		}
		
		//if (preS->GetHealthlevel() < pS->GetHealthlevel())
		//{
		//	reward = reward + 5;
		//}

		//if (preS->GetHealthlevel() > pS->GetHealthlevel())
		//{
		//	reward = reward - 5;
		//}

		//没有Haven，选择到达Haven惩罚
		//if (DisLevel::None==pS->GetDisToHaven() && m_pOwner->curPrimaryAct=="SeekSafety")
		//{
		//	reward = reward - 1;
		//}

		////没有敌人，选择进攻惩罚
		//if (DisLevel::None==pS->GetDisToEnemy() && (m_pOwner->curPrimaryAct=="Charge"||m_pOwner->curPrimaryAct=="Assist"))
		//{
		//	reward = reward - 1;
		//}

		////没有Food，选择Forge惩罚
		//if (DisLevel::None==pS->GetDisToFood() && m_pOwner->curPrimaryAct=="Forage")
		//{
		//	reward = reward - 1;
		//}
		//
		//if (HealthLevel::LowHl >= pS->GetHealthlevel() && DisLevel::Inside==pS->GetDisToFood() && m_pOwner->curPrimaryAct=="Eat")
		//{
		//	reward = reward + 1;
		//}

		/*if (HealthLevel::LowHl == pS->GetHealthlevel())
		{
			if (DisLevel::Near==pS->GetDisToEnemy() && 
				DisLevel::Near==pS->GetDisToHaven() && m_pOwner->curPrimaryAct=="SeekSafety")
			{
				reward = reward + 15;
			}

			if (DisLevel::Inside==pS->GetDisToFood() && m_pOwner->curPrimaryAct=="Eat")
			{
				reward = reward + 10;
			}

			if (DisLevel::Inside<pS->GetDisToFood() && m_pOwner->curPrimaryAct=="Forage")
			{
				reward = reward + 5;
			}

			if (DisLevel::Near==pS->GetDisToEnemy() && DisLevel::None==pS->GetDisToHaven() && m_pOwner->curPrimaryAct=="Flee")
			{
				reward = reward + 1;
			}
		}

		if (HealthLevel::HighHl==pS->GetHealthlevel() && NumAllyLevel::HighAlly==pS->GetNumAllyNeighbor() && m_pOwner->curPrimaryAct=="Charge" )
		{
			reward = reward + 0.6;
		}

		if (HealthLevel::LowHl<pS->GetHealthlevel() && NumAllyLevel::MidAlly==pS->GetNumAllyNeighbor() && m_pOwner->curPrimaryAct=="Assist")
		{
			reward = reward + 0.5;
		}*/
		m_pOwner->m_dReward = m_pOwner->m_dReward + reward;
		return reward;
	}
};