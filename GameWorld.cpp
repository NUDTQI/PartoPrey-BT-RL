#include "GameWorld.h"
#include "Predator.h"
#include "Prey.h"
#include "constants.h"
#include "Obstacle.h"
#include "2d/Geometry.h"
#include "2d/Wall2D.h"
#include "2d/Transformations.h"
#include "SteeringBehaviors.h"
#include "time/PrecisionTimer.h"
#include "ParamLoader.h"
#include "misc/WindowUtils.h"
#include "misc/Stream_Utility_Functions.h"
#include "GameMap.h"
#include "Judger.h"
#include "AI/RL/QLearning.h"
#include "AI/RL/CPrimitiveAction.h"
#include "AI/RL/CAbstractionAction.h"


#include "resource.h"
#include <list>

//------------------------------- ctor -----------------------------------
//------------------------------------------------------------------------
GameWorld::GameWorld(int cx, int cy):

            m_cxClient(cx),
            m_cyClient(cy),
            m_bPaused(false),
            m_vCrosshair(Vector2D(cxClient()/2.0, cxClient()/2.0)),
            m_bShowWalls(false),
            m_bShowObstacles(false),
            m_bShowPath(false),
            m_bShowWanderCircle(false),
            m_bShowSteeringForce(false),
            m_bShowFeelers(false),
            m_bShowDetectionBox(false),
            m_bShowFPS(true),
            m_dAvFrameTime(0),
            m_pPath(NULL),
            m_bRenderNeighbors(false),
            m_bViewKeys(false),
            m_bShowCellSpaceInfo(false)
{

	//load in the default map
	m_pMap = new GameMap();
	m_pMap->LoadMap("PredatorPrey.map");

	//set judger for game——health increase of health bag, damage of predator vs prey,damage of prey vs predator, max tick num in each iterator
	m_pJudger = new Judger(Prm.DamagePreyToPredator,Prm.DamagePredatorToPrey,Prm.HealthGainInFoodZ,Prm.TimeOutGame,Prm.MaxRunTimes,this);

	ConstructQLearners();

	InitPredatorsList();
	InitPreysList();
}


//-------------------------------- dtor ----------------------------------
//------------------------------------------------------------------------
GameWorld::~GameWorld()
{
	ClearAgentsList();
  
	delete m_pPath;
	m_pPath = NULL;
	delete m_pMap;
	m_pMap = NULL;
	delete m_pJudger;
	m_pJudger = NULL;

	delete m_pRootQLearner;
	m_pRootQLearner = NULL;
}


//----------------------------- Update -----------------------------------
//------------------------------------------------------------------------
void GameWorld::Update(double time_elapsed)
{ 
//  if (m_bPaused) return;

  //create a smoother to smooth the framerate
  const int SampleRate = 10;
  static Smoother<double> FrameRateSmoother(SampleRate, 0.0);

  //m_dAvFrameTime = FrameRateSmoother.Update(time_elapsed);
  
  //update the predators
  for (unsigned int a=0; a<m_Predators.size(); ++a)
  {
	  if (m_Predators[a]->isAlive())
	  {
		  m_Predators[a]->Update(time_elapsed);
	  }
  }

  //update the preys
  for (unsigned int a=0; a<m_Preys.size(); ++a)
  {
	  if (m_pJudger->GetCurTicks()==0)
	  {
		  m_Preys[a]->UpdateSensorSys();
		  m_Preys[a]->UpdateAgentState();
	  }
	  //dead and inhaven are two absorbed states
	  if (m_Preys[a]->isAlive() && !m_Preys[a]->InHavenZone())
	  {
		  m_Preys[a]->SelectAction();
		  m_Preys[a]->UpdateMovement(time_elapsed);
		  m_Preys[a]->m_bLearnNeeded = true;
	  }
	  else
	  {
		  m_Preys[a]->m_bLearnNeeded = false;
	  }
  }

  //calculate damage accordance
  m_pJudger->CalculateCurDamage();

  //preys learning
  for (unsigned int a=0; a<m_Preys.size(); ++a)
  {
	  m_Preys[a]->UpdateSensorSys();
	  m_Preys[a]->UpdateAgentState();

	  if (m_Preys[a]->m_bLearnNeeded )
	  {
		  m_Preys[a]->ExecuteLearning();
	  }
	  m_Preys[a]->SetDamage(false);
  }

  //update episodes of game
  m_pJudger->AdvanceGameInEachRun();

  if(JudgeResult())
  {
	  //game over
	  if (m_pJudger->UptoMaxGameRuns())
	  {
		  //m_pGeneralQLearner->saveQTableToFile((char*)QTableFileAll.c_str());
		  m_pRootQLearner->saveQTableToFile((char*)QTableFileRoot.c_str());
	
		  m_pJudger->SaveFinalStastics("result.txt");
		  //MessageBox(NULL, "GameOver!", "", MB_OK);
		  // kill the application, this sends a WM_QUIT message  
		  PostQuitMessage (0);
	  }
	  else//next run
	  {
		  ResetGame();
		  m_pJudger->AdvanceGameRuns();	
	  }
  }
}
  

//------------------------- Set Crosshair ------------------------------------
//
//  The user can set the position of the crosshair by right clicking the
//  mouse. This method makes sure the click is not inside any enabled
//  Obstacles and sets the position appropriately
//------------------------------------------------------------------------
void GameWorld::SetCrosshair(POINTS p)
{
  Vector2D ProposedPosition((double)p.x, (double)p.y);

  //make sure it's not inside an obstacle
  for (std::vector<BaseGameEntity*>::const_iterator curOb = GetGameMap()->GetObstacles().begin(); curOb != GetGameMap()->GetObstacles().end(); ++curOb)
  {
    if (PointInCircle((*curOb)->Pos(), (*curOb)->BRadius(), ProposedPosition))
    {
      return;
    }

  }
  m_vCrosshair.x = (double)p.x;
  m_vCrosshair.y = (double)p.y;
}


//------------------------- HandleKeyPresses -----------------------------
void GameWorld::HandleKeyPresses(WPARAM wParam)
{

  //switch(wParam)
  //{
  //case 'U':
  //  {
  //    delete m_pPath;
  //    double border = 60;
  //    m_pPath = new Path(RandInt(3, 7), border, border, cxClient()-border, cyClient()-border, true); 
  //    m_bShowPath = true; 
  //    for (unsigned int i=0; i<m_Agents.size(); ++i)
  //    {
  //      m_Agents[i]->Steering()->SetPath(m_pPath->GetPath());
  //    }
  //  }
  //  break;

  //  case 'P':
  //    
  //    TogglePause(); break;

  //  case 'O':

  //    ToggleRenderNeighbors(); break;

  //  case 'I':

  //    {
  //      for (unsigned int i=0; i<m_Agents.size(); ++i)
  //      {
  //        m_Agents[i]->ToggleSmoothing();
  //      }

  //    }

  //    break;

  //  case 'Y':

  //     m_bShowObstacles = !m_bShowObstacles;

  //      if (!m_bShowObstacles)
  //      {
  //        m_Obstacles.clear();

  //        for (unsigned int i=0; i<m_Agents.size(); ++i)
  //        {
  //          m_Agents[i]->Steering()->ObstacleAvoidanceOff();
  //        }
  //      }
  //      else
  //      {
  //        CreateObstacles();

  //        for (unsigned int i=0; i<m_Agents.size(); ++i)
  //        {
  //          m_Agents[i]->Steering()->ObstacleAvoidanceOn();
  //        }
  //      }
  //      break;

  //}//end switch
}



//-------------------------- HandleMenuItems -----------------------------
void GameWorld::HandleMenuItems(WPARAM wParam, HWND hwnd)
{
  switch(wParam)
  {
    case ID_OB_OBSTACLES:

        //m_bShowObstacles = !m_bShowObstacles;

        //if (!m_bShowObstacles)
        //{
        //  m_Obstacles.clear();

        //  for (unsigned int i=0; i<m_Agents.size(); ++i)
        //  {
        //    m_Agents[i]->Steering()->ObstacleAvoidanceOff();
        //  }

        //  //uncheck the menu
        // ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_UNCHECKED);
        //}
        //else
        //{
        //  CreateObstacles();

        //  for (unsigned int i=0; i<m_Agents.size(); ++i)
        //  {
        //    m_Agents[i]->Steering()->ObstacleAvoidanceOn();
        //  }

        //  //check the menu
        //  ChangeMenuState(hwnd, ID_OB_OBSTACLES, MFS_CHECKED);
        //}

       break;

    case ID_OB_WALLS:

     { //m_bShowWalls = !m_bShowWalls;

      //if (m_bShowWalls)
      //{
      //  CreateWalls();

      //  for (unsigned int i=0; i<m_Agents.size(); ++i)
      //  {
      //    m_Agents[i]->Steering()->WallAvoidanceOn();
      //  }

      //  //check the menu
      //   ChangeMenuState(hwnd, ID_OB_WALLS, MFS_CHECKED);
      //}

      //else
      //{
      //  m_Walls.clear();

      //  for (unsigned int i=0; i<m_Agents.size(); ++i)
      //  {
      //    m_Agents[i]->Steering()->WallAvoidanceOff();
      //  }

      //  //uncheck the menu
      //   ChangeMenuState(hwnd, ID_OB_WALLS, MFS_UNCHECKED);
      }

      break;


    case IDR_PARTITIONING:
      {
        //for (unsigned int i=0; i<m_Agents.size(); ++i)
        //{
        //  m_Agents[i]->Steering()->ToggleSpacePartitioningOnOff();
        //}

        ////if toggled on, empty the cell space and then re-add all the 
        ////Agents
        //if (m_Agents[0]->Steering()->isSpacePartitioningOn())
        //{
        //  m_pCellSpace->EmptyCells();
       
        //  for (unsigned int i=0; i<m_Agents.size(); ++i)
        //  {
        //    m_pCellSpace->AddEntity(m_Agents[i]);
        //  }

        //  ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_CHECKED);
        //}
        //else
        //{
        //  ChangeMenuState(hwnd, IDR_PARTITIONING, MFS_UNCHECKED);
        //  ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
        //  m_bShowCellSpaceInfo = false;

        //}
      }

      break;

    case IDM_PARTITION_VIEW_NEIGHBORS:
      {
        //m_bShowCellSpaceInfo = !m_bShowCellSpaceInfo;
        //
        //if (m_bShowCellSpaceInfo)
        //{
        //  ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_CHECKED);

        //  if (!m_Agents[0]->Steering()->isSpacePartitioningOn())
        //  {
        //    SendMessage(hwnd, WM_COMMAND, IDR_PARTITIONING, NULL);
        //  }
        //}
        //else
        //{
        //  ChangeMenuState(hwnd, IDM_PARTITION_VIEW_NEIGHBORS, MFS_UNCHECKED);
        //}
      }
      break;
        

    case IDR_WEIGHTED_SUM:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

		for (unsigned int i=0; i<m_Predators.size(); ++i)
		{
			m_Predators[i]->GetSteering()->SetSummingMethod(SteeringBehavior::weighted_average);
		}	
		for (unsigned int i=0; i<m_Preys.size(); ++i)
		{
			m_Preys[i]->GetSteering()->SetSummingMethod(SteeringBehavior::weighted_average);
		}
      }

      break;

    case IDR_PRIORITIZED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_CHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_UNCHECKED);

        for (unsigned int i=0; i<m_Predators.size(); ++i)
        {
          m_Predators[i]->GetSteering()->SetSummingMethod(SteeringBehavior::prioritized);
        }
		for (unsigned int i=0; i<m_Preys.size(); ++i)
		{
			m_Preys[i]->GetSteering()->SetSummingMethod(SteeringBehavior::prioritized);
		}
      }

      break;

    case IDR_DITHERED:
      {
        ChangeMenuState(hwnd, IDR_WEIGHTED_SUM, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_PRIORITIZED, MFS_UNCHECKED);
        ChangeMenuState(hwnd, IDR_DITHERED, MFS_CHECKED);

        for (unsigned int i=0; i<m_Predators.size(); ++i)
        {
          m_Predators[i]->GetSteering()->SetSummingMethod(SteeringBehavior::dithered);
        }
		for (unsigned int i=0; i<m_Preys.size(); ++i)
		{
			m_Preys[i]->GetSteering()->SetSummingMethod(SteeringBehavior::dithered);
		}
      }

      break;


      case ID_VIEW_KEYS:
      //{
      //  ToggleViewKeys();

      //  CheckMenuItemAppropriately(hwnd, ID_VIEW_KEYS, m_bViewKeys);
      //}

      break;

      case ID_VIEW_FPS:
      {
        //ToggleShowFPS();

        //CheckMenuItemAppropriately(hwnd, ID_VIEW_FPS, RenderFPS());
      }

      break;

      case ID_MENU_SMOOTHING:
      {
		  //update the predators
		  for (unsigned int a=0; a<m_Predators.size(); ++a)
		  {
			  m_Predators[a]->ToggleSmoothing();
		  }
		  CheckMenuItemAppropriately(hwnd, ID_MENU_SMOOTHING, m_Predators[0]->isSmoothingOn());

		  //update the preys
		  for (unsigned int a=0; a<m_Preys.size(); ++a)
		  {
			  m_Preys[a]->ToggleSmoothing();
		  }

        CheckMenuItemAppropriately(hwnd, ID_MENU_SMOOTHING, m_Preys[0]->isSmoothingOn());
      }

      break;
      
  }//end switch
}


//------------------------------ Render ----------------------------------
//------------------------------------------------------------------------
void GameWorld::Render()
{
  gdi->TransparentText();

  //render map
  m_pMap->Render();

  //render the predators
  for (unsigned int a=0; a<m_Predators.size(); ++a)
  {
	  m_Predators[a]->Render();  
  }  

  //render the preys
  for (unsigned int a=0; a<m_Preys.size(); ++a)
  {
	  m_Preys[a]->Render();  
  } 
}

//////////////////////////////Judge Game////////////////////////////////////////////
bool GameWorld::JudgeResult()
{
	GameStatus curStatus = m_pJudger->JudgeCurrentRunResult();

	if (curStatus > 0/* && curStatus != 3*/)
	{
		if (AllPreysInHaven == curStatus)
		{
			//MessageBox(NULL, "Prey Win: All preys are in safe haven!", "GameOver!", MB_OK);
		}
		else if (AllPreysDead == curStatus)
		{
			//MessageBox(NULL, "Predator Win: All preys are dead!", "GameOver!", MB_OK);
		}
		else if (AllPredatorsDead == curStatus)
		{
			//MessageBox(NULL, "Prey Win: All predators are dead!", "GameOver!", MB_OK);
			//////////////////////debug////////////////////////////////////////////////////
			//string outputAction = "Prey Win: All predators are dead!";

			//OutputDebugString(outputAction.c_str());
			///////////////////////////////////////////////////////////////////////////
		}
		else if (TimeOut == curStatus)
		{
			//MessageBox(NULL, "Prey Win: Time out!", "GameOver!", MB_OK);
		}
		return true;
	}

	return false;
}

void GameWorld::ResetGame()
{
	m_pJudger->ResetTicksInEachRun();
	ClearAgentsList();
	UpdateGLIEPolicy();

	InitPredatorsList();
	InitPreysList();
}

void GameWorld::ClearAgentsList()
{
	for (unsigned int a=0; a<m_Predators.size(); ++a)
	{
		delete m_Predators[a];
		m_Predators[a] = NULL;
	}
	m_Predators.clear();

	for (unsigned int a=0; a<m_Preys.size(); ++a)
	{
		delete m_Preys[a];
		m_Preys[a] = NULL;
	}
	m_Preys.clear();
}

void GameWorld::InitPredatorsList()
{
	//initialize predators
	for (int a=0; a<Prm.NumPredatorAgents; ++a)
	{
		//determine a  starting position
		Vector2D SpawnPos = Vector2D(RandFloat()*GetGameMap()->GetSizeX(),RandFloat()*GetGameMap()->GetSizeY());

		Predator* pPredator = new Predator(this,
			SpawnPos,                 //initial position
			RandFloat()*TwoPi,        //start rotation
			Vector2D(0,0),            //velocity
			Prm.AgentMass,          //mass
			Prm.MaxSteeringForce,     //max force
			Prm.MaxSpeed,             //max velocity
			Prm.MaxTurnRatePerSecond, //max turn rate
			Prm.AgentScale,			  //scale
			Prm.ViewDistance,
			Prm.MaxAgentHealth);        //viewdis

		//pPredator->GetSteering()->FlockingOn();
		pPredator->m_iAgentID = a;
		m_Predators.push_back(pPredator);
	}
}

void GameWorld::InitPreysList()
{
	//initialize preys
	for (int a=0; a<Prm.NumPreyAgents; ++a)
	{
		//determine a starting position
		Vector2D SpawnPos = Vector2D(RandFloat()*GetGameMap()->GetSizeX(),RandFloat()*GetGameMap()->GetSizeY());

		Prey* pPrey = new Prey(this,
			SpawnPos,                 //initial position
			RandFloat()*TwoPi,        //start rotation
			Vector2D(0,0),            //velocity
			Prm.AgentMass,          //mass
			Prm.MaxSteeringForce,     //max force
			Prm.MaxSpeed,             //max velocity
			Prm.MaxTurnRatePerSecond, //max turn rate
			Prm.AgentScale,			  //scale
			Prm.ViewDistance,
			Prm.MaxAgentHealth);        //viewdis

		//pPrey->GetSteering()->WanderOn();
		pPrey->m_iAgentID = a;
		pPrey->m_bLearnNeeded = true;
		m_Preys.push_back(pPrey);
	}
}

void GameWorld::ConstructQLearners()
{
	///////////////////////////定义多个学习器///////////////////////////////////////////////
	std::vector<CAction*> tempActionList;

	//set q-learner for preys——learning rate，discount factor，ε-greedy policy
	m_pRootQLearner = new QLearning(Prm.gamma,Prm.beta,Prm.exploration);
	m_pRootQLearner->LearnerName = "RootLearner";
	//分别为每个学习器构建响应的动作空间、状态空间动态生成
	for (int i=0;i<8;i++)
	{
		PrimitiveAction* act = new PrimitiveAction(PRIMITIVEACTION(i));
		tempActionList.push_back(act);
	}
	m_pRootQLearner->setActionSet(tempActionList);
	tempActionList.clear();

	QTableFileRoot = "QTableRoot.txt";
	//m_pRootQLearner->loadQTableFromFile((char*)QTableFileRoot.c_str());
}

void GameWorld::UpdateGLIEPolicy()
{
	//learning rate decrease to 0 in limited maxrun;
	float newbeta = m_pRootQLearner->beta*0.9995;
	float newexploration = m_pRootQLearner->exploration*0.9995;

	m_pRootQLearner->setParameters(Prm.gamma,newbeta,newexploration);

	if(m_iMaxRunTimes-m_pJudger->m_iEpisode<100)
	{
		m_pRootQLearner->setParameters(Prm.gamma,0,0);
	}
}