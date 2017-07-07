#ifndef GameWorld_H
#define GameWorld_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   GameWorld.h
//
//  Desc:   All the environment data and methods for the Steering
//          Behavior projects. This class is the root of the project's
//          update and render calls (excluding main of course)
//
//  Author: Mat Buckland 2002 (fup@ai-junkie.com)
//
//------------------------------------------------------------------------
#include <Windows.h>
#include <vector>
#include <string>
#include "2d/Vector2D.h"
#include "time/PrecisionTimer.h"
#include "BaseGameEntity.h"
#include "Predator.h"
#include "Prey.h"

class Obstacle;
class Wall2D;
class Path;
class GameMap;
class Judger;
class QLearning;


class GameWorld
{ 
private:
   //the current game map
	GameMap*                       m_pMap;

	//judger for game
	Judger*                        m_pJudger;

  //a container of all the moving entities
  std::vector<Predator*>        m_Predators;
  std::vector<Prey*>            m_Preys;

  //any path we may create for the Agents to follow
  Path*                         m_pPath;

  //set true to pause the motion
  bool                          m_bPaused;

  //local copy of client window dimensions
  int                           m_cxClient,
                                m_cyClient;
  //the position of the crosshair
  Vector2D                      m_vCrosshair;

  //keeps track of the average FPS
  double                         m_dAvFrameTime;

  //flags to turn aids and obstacles etc on/off
  bool  m_bShowWalls;
  bool  m_bShowObstacles;
  bool  m_bShowPath;
  bool  m_bShowDetectionBox;
  bool  m_bShowWanderCircle;
  bool  m_bShowFeelers;
  bool  m_bShowSteeringForce;
  bool  m_bShowFPS;
  bool  m_bRenderNeighbors;
  bool  m_bViewKeys;
  bool  m_bShowCellSpaceInfo;

  

public:
  
  GameWorld(int cx, int cy);

  ~GameWorld();

  void  Update(double time_elapsed);

  void  Render();

  bool  JudgeResult();

  void  ResetGame();

  void  ClearAgentsList();

  void InitPredatorsList();

  void InitPreysList();

  void  ConstructQLearners();

  const std::vector<Predator*>&          GetPredatorBots()const{return m_Predators;}
  const std::vector<Prey*>&              GetPreyBots()const{return m_Preys;}
  GameMap*								 GetGameMap() {return m_pMap;}
  Judger*								 GetJudger() {return m_pJudger;}

  //a common Qlearner for all preys
  QLearning* m_pRootQLearner;
  std::string QTableFileRoot;

  void UpdateGLIEPolicy();

  //handle WM_COMMAND messages
  void        HandleKeyPresses(WPARAM wParam);
  void        HandleMenuItems(WPARAM wParam, HWND hwnd);
  
  void        TogglePause(){m_bPaused = !m_bPaused;}
  bool        Paused()const{return m_bPaused;}

  Vector2D    Crosshair()const{return m_vCrosshair;}
  void        SetCrosshair(POINTS p);
  void        SetCrosshair(Vector2D v){m_vCrosshair=v;}

  int   cxClient()const{return m_cxClient;}
  int   cyClient()const{return m_cyClient;}
 
  bool  RenderWalls()const{return m_bShowWalls;}
  bool  RenderObstacles()const{return m_bShowObstacles;}
  bool  RenderPath()const{return m_bShowPath;}
  bool  RenderDetectionBox()const{return m_bShowDetectionBox;}
  bool  RenderWanderCircle()const{return m_bShowWanderCircle;}
  bool  RenderFeelers()const{return m_bShowFeelers;}
  bool  RenderSteeringForce()const{return m_bShowSteeringForce;}

  bool  RenderFPS()const{return m_bShowFPS;}
  void  ToggleShowFPS(){m_bShowFPS = !m_bShowFPS;}
  
  void  ToggleRenderNeighbors(){m_bRenderNeighbors = !m_bRenderNeighbors;}
  bool  RenderNeighbors()const{return m_bRenderNeighbors;}
  
  void  ToggleViewKeys(){m_bViewKeys = !m_bViewKeys;}
  bool  ViewKeys()const{return m_bViewKeys;}
  
};



#endif