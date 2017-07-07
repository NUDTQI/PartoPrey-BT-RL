#pragma once
#include "../../Common/misc/utils.h"
#include "QFunction.h"
#include "QLearningDataIO.h"

class EnvModelLearning;
class Agent;
class CState;
class CAction;
class CActionData;

class QLearning
{
public:
	QLearning(float gamma_in, float beta_in, float exploration_in);
	~QLearning(void);

public:
	float gamma;//折扣因子
	float beta;//学习率
	float exploration;//搜索率

public:
	EnvModelLearning* pEnvModel;
	std::map <CAction*, QFunction*> QFunctions;
	QLearningDataIO* pioQtable;
	string LearnerName;

	//for debug output
	std::map<CState*,int> bestActionID;
	int maxIter;
	int curIter;

protected:
	std::vector<CAction*> ActionList;
	std::vector<CState*> StateList;

	int ActionNum;

	float getQValue(CState* state, CAction* action);
	void  setQValue(CState* state,CAction* action, float v);
	int   getActionNum(){return ActionNum;}
	int   getActionIndex(CAction* action);

	int getRandomRLAction(int ActionNum){return RandFloat()*ActionNum;}
	CAction* chooseAction(Agent* owner,CState* state);
	CAction* getBestQValueOnState(CState* state);
	CState* findStateinList(CState* state);
	
	void updateValue(CState* pState, CAction* pAction, float reward, CState* pNextState, CAction* pNextAction); 
public:

	void saveQTableToFile(char*);
	void loadQTableFromFile(char*);

	void setParameters(float, float, float);
	//set different action set for each q-learner
	void setActionSet(std::vector<CAction*>&);
	void setStateSet( std::vector<CState*>&);

	//主要接口
	int selectActionByRL(Agent* owner);
	void KnowledgeLearn(Agent* owner);
	void AccuRewardsOption(Agent* owner,float& rdsignal);
	CState* updateEnvModelForRL(Agent* owner);
	void RecordMemory(Agent* owner);
};
