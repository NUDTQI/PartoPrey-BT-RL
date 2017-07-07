#ifndef AGENT_H
#define AGENT_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name:   Agent.h
//
//  Desc:   Definition of a simple Agent
//
//  Author: Qi Zhang 2016
//
//------------------------------------------------------------------------
#include "MovingEntity.h"
#include "2d/Vector2D.h"
#include "misc/Smoother.h"

#include <vector>
#include <list>
#include <string>

class GameWorld;
class SteeringBehavior;
class SensorMemory;
class Predator;
class Prey;
class BaseGameEntity;
class FoodZone;


class Agent : public MovingEntity
{

private:

	enum Status{alive, dead, spawning};

private:

	//alive, dead or spawning?
	Status                             m_Status;

  //a pointer to the world data. So a Agent can access any obstacle,
  //path, wall or agent data
  GameWorld*            m_pWorld;

  //the steering behavior class
  SteeringBehavior*     m_pSteering;

  //the sensor system class
  SensorMemory*         m_pSenseSys;

  //the bot's health. Every time the bot is shot this value is decreased. If
  //it reaches zero then the bot dies (and respawns)
  int                                m_iHealth;

  //the bot's maximum health value. It starts its life with health at this value
  int                                m_iMaxHealth;

  //each time this bot kills another this value is incremented
  int                                m_iScore;

  //the direction the bot is facing (and therefore the direction of aim). 
  //Note that this may not be the same as the bot's heading, which always
  //points in the direction of the bot's movement
  Vector2D                           m_vFacing;

  //a bot only perceives other bots within this field of view
  double                             m_dFieldOfView;

  //a bot only perceives other bots within this range
  double                             m_dSensorRange;

  bool								 m_bDamaged;


protected:

  //some steering behaviors give jerky looking movement. The
  //following members are used to smooth the Agent's heading
  Smoother<Vector2D>*  m_pHeadingSmoother;

  //this vector represents the average of the Agent's heading
  //vector smoothed over the last few frames
  Vector2D             m_vSmoothedHeading;

  //when true, smoothing is active
  bool                  m_bSmoothingOn;

  //keeps a track of the most recent update time. (some of the
  //steering behaviors make use of this - see Wander)
  double                m_dTimeElapsed;

  //buffer for the Agent shape
  std::vector<Vector2D> m_vecAgentVB;

  //fills the buffer with vertex data
  void InitializeBuffer();

  //disallow the copying of Agent types
  Agent(const Agent&);
  Agent& operator=(const Agent&);


public:

  Agent(GameWorld* world,
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

  ~Agent();

  int m_iAgentID;
                                                                          
  //-------------------------------------------accessor methods
  SteeringBehavior*const  GetSteering()const{return m_pSteering;}
  GameWorld*const         GetWorld()const{return m_pWorld;} 
  SensorMemory* const     GetSensorMem()const{return m_pSenseSys;}

  
  Vector2D    SmoothedHeading()const{return m_vSmoothedHeading;}

  bool        isSmoothingOn()const{return m_bSmoothingOn;}
  void        SmoothingOn(){m_bSmoothingOn = true;}
  void        SmoothingOff(){m_bSmoothingOn = false;}
  void        ToggleSmoothing(){m_bSmoothingOn = !m_bSmoothingOn;}
  
  double       TimeElapsed()const{return m_dTimeElapsed;}

  bool          isDead()const{return m_Status == dead;}
  bool          isAlive()const{return m_Status == alive;}
  bool          isSpawning()const{return m_Status == spawning;}
  bool          isDamaged()const{return m_bDamaged;}

  void          SetSpawning(){m_Status = spawning;}
  void          SetDead(){m_Status = dead;}
  void          SetAlive(){m_Status = alive;}
  void          SetDamage(bool damage){m_bDamaged = damage;}

  //this rotates the bot's heading until it is facing directly at the target
  //position. Returns false if not facing at the target.
  bool          RotateFacingTowardPosition(Vector2D target);

  //methods for accessing attribute data
  int           Health()const{return m_iHealth;}
  int           MaxHealth()const{return m_iMaxHealth;}
  void          ReduceHealth(unsigned int val);
  void          IncreaseHealth(unsigned int val);
  void          RestoreHealthToMaximum();

  int           Score()const{return m_iScore;}
  void          IncrementScore(){++m_iScore;}

  Vector2D      Facing()const{return m_vFacing;}
  double        FieldOfView()const{return m_dFieldOfView;}
  double        SensorRange()const{return m_dSensorRange;}

  //get Ally Bots in Range
  virtual std::vector<Agent*>& GetAllyBotsInRange();
  //
  Prey* GetNearestPrey();
  Predator* GetNearestPredator();
  BaseGameEntity* GetNearestHaven();
  BaseGameEntity* GetNearestFood();



  /////////////////////////behavior/////////////////////////////
  //updates the Agent's position and orientation
  virtual void        UpdateMovement(double time_elapsed);

  //updates the Agent's sensor system
  virtual void        UpdateSensorSys();

  //select action
  virtual void        SelectAction();

  //updates the Agent's sensor system
  virtual void        Update(double time_elapsed);

  virtual void        Render();
};



#endif