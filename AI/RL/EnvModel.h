#pragma once
#include "Action.h"

class Prey;


/*  zhangqi 2017/02/24
	功能：定义完整的用于学习的环境模型基类，作为学习算法使用的公共接口；
	其只应包括宿主实体、状态类指针，动作类指针，以及奖励函数接口
*/

class EnvModelLearning
{
public:
	EnvModelLearning(Prey* pA)
	{
		m_pOwner = pA;
		pState = NULL;
		preState = NULL;
		pAction = NULL;
		preAction = NULL;
		rewardFeedback = 0;
	}
	~EnvModelLearning(void){}
protected:
	Prey* m_pOwner;

public:
	//state
	CState* pState;
	CState* preState;
	//action
	CAction* pAction;
	CAction* preAction;
	//action data
	CActionData localActionData;
	CActionData preActionData;
	//reward
	float rewardFeedback;

public:
	virtual float CalculateRewardsEachStep(){return 0;}
	virtual CState* UpdateCurState(){return NULL;}
	virtual void SetCurAction(CAction* pA){pAction = pA;}
	virtual CState* MemyCopyState(CState* temp){return NULL;}
	virtual void bindActionData(CAction* pA, CActionData* pAD){pA->setActionData(pAD);}
};
