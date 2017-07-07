#pragma once
#include "Agent.h"
#include <string>

class BevNode;
class QLearning;
class CAction;
class CState;
class ModelSelAttackBevLearning;
class ModelSelRetreatBevLearning;
class ModelSelRootBevLearning;
class ModelSelGeneralBevLearning;

using namespace std;

class Prey :
	public Agent
{

public:
	Prey(GameWorld* world,
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
	~Prey(void);

private:
	//flag in special zones
	bool m_bInFoodZone;
	bool m_bInHavenZone;

	// Number of ally neighbours
	int m_iNumAlly;
	//Distance to nearest Food
	double m_dDisToFood;
	//Distance to nearest Haven
	double m_dDisToHaven;
	//Distance to nearest Predator
	double m_dDisToPredator;

public:
	//  [1/17/2017 zq]behavior tree variables - all the behavior nodes
	BevNode* m_pBevTreeRoot;
		BevNode* m_pRetreat;
			BevNode* m_pFlee;
			BevNode* m_pSeekSafety;
		BevNode* m_pIdle;
			BevNode* m_pGraze;
				BevNode* m_pForage;
				BevNode* m_pEat;
			BevNode* m_pExplore;
				BevNode* m_pFlock;
				BevNode* m_pWander;
		BevNode* m_pAttack;
			BevNode* m_pCharge;
				BevNode* m_pAssist;
		BevNode* m_pRandomWalk;

	//flag to determine is needing learn
	bool m_bLearnNeeded;
	Predator* m_CurEnemy;
	double m_dReward;
	// construct BT for agent
	virtual void CreateBTforAgent(const char* a_zXMLFile);
	//构建学习选择节点
	BevNode* GenerateLearningNode(BevNode* pParentNode,string NodeName);
	virtual void BindLearnerForNode(BevNode* pLearningNode);

	//define EnvModel for bev node needing learning
	ModelSelGeneralBevLearning* pRootEnvModel;
		
	std::string curPrimaryAct;
	///////////////////////////set accessor///////////////////////////////////////////////
	
	void setInFoodZone(bool flag){m_bInFoodZone = flag;}
	void setInHavenZone(bool flag){m_bInHavenZone = flag;}
	void setNumAllyNeighbour(int num){m_iNumAlly = num;}
	void setDisToNearestFood(double dis){m_dDisToFood = dis;}
	void setDisToNearestHaven(double dis){m_dDisToHaven = dis;}
	void setDisToNearestPredator(double dis){m_dDisToPredator = dis;}

	////////////////////get accessor//////////////////////////////////////////////////////
	
	bool InFoodZone(){return m_bInFoodZone;}
	bool InHavenZone(){return m_bInHavenZone;}
	int    getNumAllyNeighbour(){return m_iNumAlly;}
	double getDisToNearestFood(){return m_dDisToFood;}
	double getDisToNearestHaven(){return m_dDisToHaven;}
	double getDisToNearestPredator(){return m_dDisToPredator;}
	//get Ally Bots in Range
	virtual std::vector<Agent*>& GetAllyBotsInRange();

	//////////////////////////////////update logic////////////////////////////////////////
	// override update(sensor,action,movment,state)
	virtual void Update(double time_elapsed);
	// update some states of agent recorded
	void UpdateAgentState(void);
	// update if it is in Heaven zone
	void UpdateInHeaven(void);
	// update if it is in Food Zone
	void UpdateInFoodZone(void);

	//decision-select action
	void   SelectAction();

	void ExecuteLearning();

	///////////////////////////////////render///////////////////////////////////////
	void        Render();
};
