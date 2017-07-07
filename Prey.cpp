#include "Prey.h"
#include "misc/cgdi.h"
#include "SensorMemory.h"
#include "SteeringBehaviors.h"
#include "2D/Vector2D.h"
#include "GameWorld.h"
#include "GameMap.h" 
#include "Zone.h"
#include "Judger.h"
#include "BaseGameEntity.h"

#include <iostream>
#include "BT/BevNodeFactory.h"
#include "AI/BT/BevTreeLocal.h"
#include "AI/RL/QLearning.h"
#include "AI/RL/CAbstractionAction.h"
#include "ModelBevLearning.h"
#include "AI/RL/EnvModel.h"


Prey::Prey(GameWorld* world,
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
	m_bInFoodZone = false;
	m_bInHavenZone = false;
	m_CurEnemy = NULL;
	GetSteering()->WallAvoidanceOn();
	m_dReward = 0;

	CreateBTforAgent(NULL);
	BindLearnerForNode(NULL);
}


Prey::~Prey(void)
{
	D_SafeDelete(m_pBevTreeRoot);

	delete pRootEnvModel;
	pRootEnvModel = NULL;
}


void Prey::SelectAction()
{
	/////////////////////////////////tick behavior tree/////////////////////////////////////////
	BevInputData m_BevTreeInputData;
	m_BevTreeInputData.m_pOwner = this;
	BevNodeInputParam input(&m_BevTreeInputData);
	BevOutputData m_BevTreeOutputData;
	BevNodeOutputParam output(&m_BevTreeOutputData);

	GetWorld()->m_pRootQLearner->pEnvModel = pRootEnvModel;

	if(m_pBevTreeRoot->Evaluate(input))
	{
		m_pBevTreeRoot->Tick(input, output);
	}
	curPrimaryAct = m_pBevTreeRoot->oGetLastActiveNode()->GetDebugName();
}

void Prey::Render()
{
	//a vector to hold the transformed vertices
	static std::vector<Vector2D>  m_vecVehicleVBTrans;

	if (isAlive())
	{
		gdi->BluePen(); 
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

std::vector<Agent*>& Prey::GetAllyBotsInRange()
{
	return GetSensorMem()->m_MemoryMapPreys;
}


// override update(sensor,action,movment,state)
void Prey::Update(double time_elapsed)
{
	SelectAction();

	UpdateMovement(time_elapsed);

	UpdateSensorSys();

	UpdateAgentState();
}


// update if it is in Heaven zone
void Prey::UpdateInHeaven(void)
{
	std::vector<BaseGameEntity*>::const_iterator curHaven = GetWorld()->GetGameMap()->GetHavenZones().begin();
	for (curHaven; curHaven != GetWorld()->GetGameMap()->GetHavenZones().end(); ++curHaven)
	{
		double dist = Vec2DDistanceSq(Pos(),(*curHaven)->Pos());
		if (dist <= (-Scale().x+(*curHaven)->BRadius())*(-Scale().x+(*curHaven)->BRadius()))
		{
			setInHavenZone(true);
			break;
		}
		else
		{
			setInHavenZone(false);
		}
	}
}


// update if it is in Food Zone
void Prey::UpdateInFoodZone(void)
{
	std::vector<BaseGameEntity*>::const_iterator curFoodZone = GetWorld()->GetGameMap()->GetFoodZones().begin();
	for (curFoodZone; curFoodZone != GetWorld()->GetGameMap()->GetFoodZones().end(); ++curFoodZone)
	{
		double dist = Vec2DDistanceSq(Pos(),(*curFoodZone)->Pos());
		if (dist <= (Scale().x+(*curFoodZone)->BRadius())*(Scale().x+(*curFoodZone)->BRadius()))
		{
			setInFoodZone(true);
			break;
		}
		else
		{
			setInFoodZone(false);
		}
	}
}


// construct BT for agent
void Prey::CreateBTforAgent(const char* a_zXMLFile)
{
	//cannot construct BT from XML file
	if (a_zXMLFile == NULL)
	{
		std::cout << "xml file empty." << std::endl;
	}

	//////////////////////////////////flat RL for BT////////*/////////////////////////////////

	m_pBevTreeRoot = GenerateLearningNode(NULL,"root");              // 创建行为树的根节点

	m_pSeekSafety = &BevNodeFactory::oCreateTeminalNode<NOD_SeekSafety>(m_pBevTreeRoot,"SeekSafety");
	m_pFlee = &BevNodeFactory::oCreateTeminalNode<NOD_Flee>(m_pBevTreeRoot,"Flee");
	m_pEat = &BevNodeFactory::oCreateTeminalNode<NOD_Eat>(m_pBevTreeRoot,"Eat");
	m_pForage = &BevNodeFactory::oCreateTeminalNode<NOD_Forage>(m_pBevTreeRoot,"Forage");
	m_pFlock = &BevNodeFactory::oCreateTeminalNode<NOD_Flock>(m_pBevTreeRoot,"Flock");
	m_pWander = &BevNodeFactory::oCreateTeminalNode<NOD_Wander>(m_pBevTreeRoot,"Wander");
	m_pAssist = &BevNodeFactory::oCreateTeminalNode<NOD_Assist>(m_pBevTreeRoot,"Assist");
	m_pCharge = &BevNodeFactory::oCreateTeminalNode<NOD_Charge>(m_pBevTreeRoot,"Charge");
}

//构建学习选择节点
BevNode* Prey::GenerateLearningNode(BevNode* pParentNode,string NodeName)
{
	BevNodeLearningSelector* pReturn = new BevNodeLearningSelector(pParentNode);
	if (pParentNode)
	{
		pParentNode->AddChildNode(pReturn);
	}
	pReturn->SetDebugName(NodeName.c_str());
	return pReturn;
}

// update some states of agent recorded
void Prey::UpdateAgentState(void)
{
	UpdateInHeaven();
	UpdateInFoodZone();

	setNumAllyNeighbour(GetAllyBotsInRange().size());

	BaseGameEntity* nearestHaven = GetNearestHaven();
	if (nearestHaven)
	{
		setDisToNearestHaven(Vec2DDistance(Pos(),nearestHaven->Pos()));
	}
	else
	{
		setDisToNearestHaven(MaxInt);
	}

	BaseGameEntity* nearestFood = GetNearestFood();
	if (nearestFood)
	{
		setDisToNearestFood(Vec2DDistance(Pos(),nearestFood->Pos()));
	}
	else
	{
		setDisToNearestFood(MaxInt);
	}

	Predator* pNearestEnemy = GetNearestPredator();
	if (pNearestEnemy)
	{
		setDisToNearestPredator(Vec2DDistance(Pos(),pNearestEnemy->Pos()));
	}
	else
	{
		setDisToNearestPredator(MaxInt);
	}
}

void Prey::BindLearnerForNode( BevNode* pLearningNode )
{
	pRootEnvModel = new ModelSelGeneralBevLearning(this);
	dynamic_cast<BevNodeLearningSelector*>(m_pBevTreeRoot)->SetLearner(GetWorld()->m_pRootQLearner);
}

void Prey::ExecuteLearning()
{
	BevInputData m_BevTreeInputData;
	m_BevTreeInputData.m_pOwner = this;
	BevNodeInputParam input(&m_BevTreeInputData);
	BevOutputData m_BevTreeOutputData;
	m_BevTreeOutputData.rewardSignal = 0;
	BevNodeOutputParam output(&m_BevTreeOutputData);

	BevNodeLearningSelector* pRoot = dynamic_cast<BevNodeLearningSelector*>(m_pBevTreeRoot);
	GetWorld()->m_pRootQLearner->pEnvModel = pRootEnvModel;
	pRoot->_DoNodeLearning(input,output);

}