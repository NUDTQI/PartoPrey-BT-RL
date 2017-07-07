#include "SteeringBehaviors.h"
#include "Agent.h"
#include "2d/Wall2D.h"
#include "2d/Transformations.h"
#include "misc/utils.h"
#include "misc/Cgdi.h"
#include "GameWorld.h"
#include "2d/geometry.h"
#include "BaseGameEntity.h"
#include "misc/CellSpacePartition.h"
#include "misc/Stream_Utility_Functions.h"
#include "EntityFunctionTemplates.h"
#include "SensorMemory.h"
#include "GameMap.h"

#include <cassert>


using std::string;
using std::vector;


//------------------------- ctor -----------------------------------------
//
//------------------------------------------------------------------------
SteeringBehavior::SteeringBehavior(Agent* agent):
                                  
             
             m_pAgent(agent),
             m_iFlags(0),
             m_dDBoxLength(Prm.MinDetectionBoxLength),
             m_dWeightCohesion(Prm.CohesionWeight),
             m_dWeightAlignment(Prm.AlignmentWeight),
             m_dWeightSeparation(Prm.SeparationWeight),
             m_dWeightObstacleAvoidance(Prm.ObstacleAvoidanceWeight),
             m_dWeightWander(Prm.WanderWeight),
             m_dWeightWallAvoidance(Prm.WallAvoidanceWeight),
             m_dViewDistance(Prm.ViewDistance),
             m_dWallDetectionFeelerLength(Prm.WallDetectionFeelerLength),
             m_Feelers(3),
             m_Deceleration(normal),
             m_pTargetAgent1(NULL),
             m_pTargetAgent2(NULL),
             m_dWanderDistance(WanderDist),
             m_dWanderJitter(WanderJitterPerSec),
             m_dWanderRadius(WanderRad),
             m_dWaypointSeekDistSq(WaypointSeekDist*WaypointSeekDist),
             m_dWeightSeek(Prm.SeekWeight),
             m_dWeightFlee(Prm.FleeWeight),
             m_dWeightArrive(Prm.ArriveWeight),
             m_dWeightPursuit(Prm.PursuitWeight),
             m_dWeightOffsetPursuit(Prm.OffsetPursuitWeight),
             m_dWeightInterpose(Prm.InterposeWeight),
             m_dWeightHide(Prm.HideWeight),
             m_dWeightEvade(Prm.EvadeWeight),
             m_dWeightFollowPath(Prm.FollowPathWeight),
             m_bCellSpaceOn(false),
             m_SummingMethod(prioritized)


{
  //stuff for the wander behavior
  double theta = RandFloat() * TwoPi;

  //create a vector to a target position on the wander circle
  m_vWanderTarget = Vector2D(m_dWanderRadius * cos(theta),
                              m_dWanderRadius * sin(theta));

  //create a Path
  m_pPath = new Path();
  m_pPath->LoopOn();

}

//---------------------------------dtor ----------------------------------
SteeringBehavior::~SteeringBehavior(){delete m_pPath;}


/////////////////////////////////////////////////////////////////////////////// CALCULATE METHODS 


//----------------------- Calculate --------------------------------------
//
//  calculates the accumulated steering force according to the method set
//  in m_SummingMethod
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Calculate()
{ 
	//reset the steering force
	m_vSteeringForce.Zero();

	m_vSteeringForce = CalculatePrioritized(); 

	return m_vSteeringForce;
}

//------------------------- ForwardComponent -----------------------------
//
//  returns the forward oomponent of the steering force
//------------------------------------------------------------------------
double SteeringBehavior::ForwardComponent()
{
  return m_pAgent->Heading().Dot(m_vSteeringForce);
}

//--------------------------- SideComponent ------------------------------
//  returns the side component of the steering force
//------------------------------------------------------------------------
double SteeringBehavior::SideComponent()
{
  return m_pAgent->Side().Dot(m_vSteeringForce);
}


//--------------------- AccumulateForce ----------------------------------
//
//  This function calculates how much of its max steering force the 
//  Agent has left to apply and then applies that amount of the
//  force to add.
//------------------------------------------------------------------------
bool SteeringBehavior::AccumulateForce(Vector2D &RunningTot,
                                       Vector2D ForceToAdd)
{
  
  //calculate how much steering force the Agent has used so far
  double MagnitudeSoFar = RunningTot.Length();

  //calculate how much steering force remains to be used by this Agent
  double MagnitudeRemaining = m_pAgent->MaxForce() - MagnitudeSoFar;

  //return false if there is no more force left to use
  if (MagnitudeRemaining <= 0.0) return false;

  //calculate the magnitude of the force we want to add
  double MagnitudeToAdd = ForceToAdd.Length();
  
  //if the magnitude of the sum of ForceToAdd and the running total
  //does not exceed the maximum force available to this Agent, just
  //add together. Otherwise add as much of the ForceToAdd vector is
  //possible without going over the max.
  if (MagnitudeToAdd < MagnitudeRemaining)
  {
    RunningTot += ForceToAdd;
  }

  else
  {
    //add it to the steering force
    RunningTot += (Vec2DNormalize(ForceToAdd) * MagnitudeRemaining); 
  }

  return true;
}



//---------------------- CalculatePrioritized ----------------------------
//
//  this method calls each active steering behavior in order of priority
//  and acumulates their forces until the max steering force magnitude
//  is reached, at which time the function returns the steering force 
//  accumulated to that  point
//------------------------------------------------------------------------
Vector2D SteeringBehavior::CalculatePrioritized()
{       
  Vector2D force;
  
   if (On(wall_avoidance))
  {
    force = WallAvoidance(m_pAgent->GetWorld()->GetGameMap()->GetWalls()) *
            m_dWeightWallAvoidance;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

   if (On(haven_avoidance))
   {
	   force = ObstacleAvoidance(m_pAgent->GetWorld()->GetGameMap()->GetHavenZones()) *
		   m_dWeightWallAvoidance;

	   if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
   }
   
  if (On(obstacle_avoidance))
  {
    force = ObstacleAvoidance(m_pAgent->GetWorld()->GetGameMap()->GetObstacles()) * 
            m_dWeightObstacleAvoidance;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(evade))
  {
    assert(m_pTargetAgent1 && "Evade target not assigned");
    
    force = Evade(m_pTargetAgent1) * m_dWeightEvade;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  
  if (On(flee))
  {
    force = Flee(m_vTarget) * m_dWeightFlee;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(separation))
  {
	  force = Separation(m_pAgent->GetAllyBotsInRange()) * m_dWeightSeparation;

	  if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(allignment))
  {
	  force = Alignment(m_pAgent->GetAllyBotsInRange()) * m_dWeightAlignment;

	  if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(cohesion))
  {
	  force = Cohesion(m_pAgent->GetAllyBotsInRange()) * m_dWeightCohesion;

	  if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(seek))
  {
    force = Seek(m_vTarget) * m_dWeightSeek;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }


  if (On(arrive))
  {
    force = Arrive(m_vTarget, m_Deceleration) * m_dWeightArrive;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(wander))
  {
    force = Wander() * m_dWeightWander;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  if (On(pursuit))
  {
    assert(m_pTargetAgent1 && "pursuit target not assigned");

    force = Pursuit(m_pTargetAgent1) * m_dWeightPursuit;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }


  if (On(follow_path))
  {
    force = FollowPath() * m_dWeightFollowPath;

    if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;
  }

  return m_vSteeringForce;
}


//---------------------- CalculateWeightedSum ----------------------------
//
//  this simply sums up all the active behaviors X their weights and 
//  truncates the result to the max available steering force before 
//  returning
//------------------------------------------------------------------------
//Vector2D SteeringBehavior::CalculateWeightedSum()
//{        
//  if (On(wall_avoidance))
//  {
//    m_vSteeringForce += WallAvoidance(m_pAgent->GetWorld()->Walls()) *
//                         m_dWeightWallAvoidance;
//  }
//   
//  if (On(obstacle_avoidance))
//  {
//    m_vSteeringForce += ObstacleAvoidance(m_pAgent->GetWorld()->Obstacles()) * 
//            m_dWeightObstacleAvoidance;
//  }
//
//  if (On(evade))
//  {
//    assert(m_pTargetAgent1 && "Evade target not assigned");
//    
//    m_vSteeringForce += Evade(m_pTargetAgent1) * m_dWeightEvade;
//  }
//
//
//  //these next three can be combined for flocking behavior (wander is
//  //also a good behavior to add into this mix)
//  if (!isSpacePartitioningOn())
//  {
//    if (On(separation))
//    {
//      m_vSteeringForce += Separation(m_pAgent->GetWorld()->Agents()) * m_dWeightSeparation;
//    }
//
//    if (On(allignment))
//    {
//      m_vSteeringForce += Alignment(m_pAgent->GetWorld()->Agents()) * m_dWeightAlignment;
//    }
//
//    if (On(cohesion))
//    {
//      m_vSteeringForce += Cohesion(m_pAgent->GetWorld()->Agents()) * m_dWeightCohesion;
//    }
//  }
//  else
//  {
//    if (On(separation))
//    {
//      m_vSteeringForce += SeparationPlus(m_pAgent->GetWorld()->Agents()) * m_dWeightSeparation;
//    }
//
//    if (On(allignment))
//    {
//      m_vSteeringForce += AlignmentPlus(m_pAgent->GetWorld()->Agents()) * m_dWeightAlignment;
//    }
//
//    if (On(cohesion))
//    {
//      m_vSteeringForce += CohesionPlus(m_pAgent->GetWorld()->Agents()) * m_dWeightCohesion;
//    }
//  }
//
//
//  if (On(wander))
//  {
//    m_vSteeringForce += Wander() * m_dWeightWander;
//  }
//
//  if (On(seek))
//  {
//    m_vSteeringForce += Seek(m_pAgent->GetWorld()->Crosshair()) * m_dWeightSeek;
//  }
//
//  if (On(flee))
//  {
//    m_vSteeringForce += Flee(m_pAgent->GetWorld()->Crosshair()) * m_dWeightFlee;
//  }
//
//  if (On(arrive))
//  {
//    m_vSteeringForce += Arrive(m_pAgent->GetWorld()->Crosshair(), m_Deceleration) * m_dWeightArrive;
//  }
//
//  if (On(pursuit))
//  {
//    assert(m_pTargetAgent1 && "pursuit target not assigned");
//
//    m_vSteeringForce += Pursuit(m_pTargetAgent1) * m_dWeightPursuit;
//  }
//
//  if (On(offset_pursuit))
//  {
//    assert (m_pTargetAgent1 && "pursuit target not assigned");
//    assert (!m_vOffset.isZero() && "No offset assigned");
//
//    m_vSteeringForce += OffsetPursuit(m_pTargetAgent1, m_vOffset) * m_dWeightOffsetPursuit;
//  }
//
//  if (On(interpose))
//  {
//    assert (m_pTargetAgent1 && m_pTargetAgent2 && "Interpose agents not assigned");
//
//    m_vSteeringForce += Interpose(m_pTargetAgent1, m_pTargetAgent2) * m_dWeightInterpose;
//  }
//
//  if (On(hide))
//  {
//    assert(m_pTargetAgent1 && "Hide target not assigned");
//
//    m_vSteeringForce += Hide(m_pTargetAgent1, m_pAgent->GetWorld()->Obstacles()) * m_dWeightHide;
//  }
//
//  if (On(follow_path))
//  {
//    m_vSteeringForce += FollowPath() * m_dWeightFollowPath;
//  }
//
//  m_vSteeringForce.Truncate(m_pAgent->MaxForce());
// 
//  return m_vSteeringForce;
//}


//---------------------- CalculateDithered ----------------------------
//
//  this method sums up the active behaviors by assigning a probabilty
//  of being calculated to each behavior. It then tests the first priority
//  to see if it should be calcukated this simulation-step. If so, it
//  calculates the steering force resulting from this behavior. If it is
//  more than zero it returns the force. If zero, or if the behavior is
//  skipped it continues onto the next priority, and so on.
//
//  NOTE: Not all of the behaviors have been implemented in this method,
//        just a few, so you get the general idea
//------------------------------------------------------------------------
//Vector2D SteeringBehavior::CalculateDithered()
//{  
//  //reset the steering force
//   m_vSteeringForce.Zero();
//
//  if (On(wall_avoidance) && RandFloat() < Prm.prWallAvoidance)
//  {
//    m_vSteeringForce = WallAvoidance(m_pAgent->GetWorld()->Walls()) *
//                         m_dWeightWallAvoidance / Prm.prWallAvoidance;
//
//    if (!m_vSteeringForce.isZero())
//    {
//      m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//      return m_vSteeringForce;
//    }
//  }
//   
//  if (On(obstacle_avoidance) && RandFloat() < Prm.prObstacleAvoidance)
//  {
//    m_vSteeringForce += ObstacleAvoidance(m_pAgent->GetWorld()->Obstacles()) * 
//            m_dWeightObstacleAvoidance / Prm.prObstacleAvoidance;
//
//    if (!m_vSteeringForce.isZero())
//    {
//      m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//      return m_vSteeringForce;
//    }
//  }
//
//  if (!isSpacePartitioningOn())
//  {
//    if (On(separation) && RandFloat() < Prm.prSeparation)
//    {
//      m_vSteeringForce += Separation(m_pAgent->GetWorld()->Agents()) * 
//                          m_dWeightSeparation / Prm.prSeparation;
//
//      if (!m_vSteeringForce.isZero())
//      {
//        m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//        return m_vSteeringForce;
//      }
//    }
//  }
//
//  else
//  {
//    if (On(separation) && RandFloat() < Prm.prSeparation)
//    {
//      m_vSteeringForce += SeparationPlus(m_pAgent->GetWorld()->Agents()) * 
//                          m_dWeightSeparation / Prm.prSeparation;
//
//      if (!m_vSteeringForce.isZero())
//      {
//        m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//        return m_vSteeringForce;
//      }
//    }
//  }
//
//
//  if (On(flee) && RandFloat() < Prm.prFlee)
//  {
//    m_vSteeringForce += Flee(m_pAgent->GetWorld()->Crosshair()) * m_dWeightFlee / Prm.prFlee;
//
//    if (!m_vSteeringForce.isZero())
//    {
//      m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//      return m_vSteeringForce;
//    }
//  }
//
//  if (On(evade) && RandFloat() < Prm.prEvade)
//  {
//    assert(m_pTargetAgent1 && "Evade target not assigned");
//    
//    m_vSteeringForce += Evade(m_pTargetAgent1) * m_dWeightEvade / Prm.prEvade;
//
//    if (!m_vSteeringForce.isZero())
//    {
//      m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//      return m_vSteeringForce;
//    }
//  }
//
//
//  if (!isSpacePartitioningOn())
//  {
//    if (On(allignment) && RandFloat() < Prm.prAlignment)
//    {
//      m_vSteeringForce += Alignment(m_pAgent->GetWorld()->Agents()) *
//                          m_dWeightAlignment / Prm.prAlignment;
//
//      if (!m_vSteeringForce.isZero())
//      {
//        m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//        return m_vSteeringForce;
//      }
//    }
//
//    if (On(cohesion) && RandFloat() < Prm.prCohesion)
//    {
//      m_vSteeringForce += Cohesion(m_pAgent->GetWorld()->Agents()) * 
//                          m_dWeightCohesion / Prm.prCohesion;
//
//      if (!m_vSteeringForce.isZero())
//      {
//        m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//        return m_vSteeringForce;
//      }
//    }
//  }
//  else
//  {
//    if (On(allignment) && RandFloat() < Prm.prAlignment)
//    {
//      m_vSteeringForce += AlignmentPlus(m_pAgent->GetWorld()->Agents()) *
//                          m_dWeightAlignment / Prm.prAlignment;
//
//      if (!m_vSteeringForce.isZero())
//      {
//        m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//        return m_vSteeringForce;
//      }
//    }
//
//    if (On(cohesion) && RandFloat() < Prm.prCohesion)
//    {
//      m_vSteeringForce += CohesionPlus(m_pAgent->GetWorld()->Agents()) *
//                          m_dWeightCohesion / Prm.prCohesion;
//
//      if (!m_vSteeringForce.isZero())
//      {
//        m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//        return m_vSteeringForce;
//      }
//    }
//  }
//
//  if (On(wander) && RandFloat() < Prm.prWander)
//  {
//    m_vSteeringForce += Wander() * m_dWeightWander / Prm.prWander;
//
//    if (!m_vSteeringForce.isZero())
//    {
//      m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//      return m_vSteeringForce;
//    }
//  }
//
//  if (On(seek) && RandFloat() < Prm.prSeek)
//  {
//    m_vSteeringForce += Seek(m_pAgent->GetWorld()->Crosshair()) * m_dWeightSeek / Prm.prSeek;
//
//    if (!m_vSteeringForce.isZero())
//    {
//      m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//      return m_vSteeringForce;
//    }
//  }
//
//  if (On(arrive) && RandFloat() < Prm.prArrive)
//  {
//    m_vSteeringForce += Arrive(m_pAgent->GetWorld()->Crosshair(), m_Deceleration) * 
//                        m_dWeightArrive / Prm.prArrive;
//
//    if (!m_vSteeringForce.isZero())
//    {
//      m_vSteeringForce.Truncate(m_pAgent->MaxForce()); 
//      
//      return m_vSteeringForce;
//    }
//  }
// 
//  return m_vSteeringForce;
//}



/////////////////////////////////////////////////////////////////////////////// START OF BEHAVIORS

//------------------------------- Seek -----------------------------------
//
//  Given a target, this behavior returns a steering force which will
//  direct the agent towards the target
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Seek(Vector2D TargetPos)
{
  Vector2D DesiredVelocity = Vec2DNormalize(TargetPos - m_pAgent->Pos())
                            * m_pAgent->MaxSpeed();

  return (DesiredVelocity - m_pAgent->Velocity());
}

//----------------------------- Flee -------------------------------------
//
//  Does the opposite of Seek
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Flee(Vector2D TargetPos)
{
  //only flee if the target is within 'panic distance'. Work in distance
  //squared space.
 /* const double PanicDistanceSq = 100.0f * 100.0;
  if (Vec2DDistanceSq(m_pAgent->Pos(), target) > PanicDistanceSq)
  {
    return Vector2D(0,0);
  }
  */

  Vector2D DesiredVelocity = Vec2DNormalize(m_pAgent->Pos() - TargetPos) 
                            * m_pAgent->MaxSpeed();

  return (DesiredVelocity - m_pAgent->Velocity());
}

//--------------------------- Arrive -------------------------------------
//
//  This behavior is similar to seek but it attempts to arrive at the
//  target with a zero velocity
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Arrive(Vector2D     TargetPos,
                                  Deceleration deceleration)
{
  Vector2D ToTarget = TargetPos - m_pAgent->Pos();

  //calculate the distance to the target
  double dist = ToTarget.Length();

  if (dist > 0)
  {
    //because Deceleration is enumerated as an int, this value is required
    //to provide fine tweaking of the deceleration..
    const double DecelerationTweaker = 0.3;

    //calculate the speed required to reach the target given the desired
    //deceleration
    double speed =  dist / ((double)deceleration * DecelerationTweaker);     

    //make sure the velocity does not exceed the max
    speed = min(speed, m_pAgent->MaxSpeed());

    //from here proceed just like Seek except we don't need to normalize 
    //the ToTarget vector because we have already gone to the trouble
    //of calculating its length: dist. 
    Vector2D DesiredVelocity =  ToTarget * speed / dist;

    return (DesiredVelocity - m_pAgent->Velocity());
  }

  return Vector2D(0,0);
}

//------------------------------ Pursuit ---------------------------------
//
//  this behavior creates a force that steers the agent towards the 
//  evader
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Pursuit(const Agent* evader)
{
  //if the evader is ahead and facing the agent then we can just seek
  //for the evader's current position.
  Vector2D ToEvader = evader->Pos() - m_pAgent->Pos();

  double RelativeHeading = m_pAgent->Heading().Dot(evader->Heading());

  if ( (ToEvader.Dot(m_pAgent->Heading()) > 0) &&  
       (RelativeHeading < -0.95))  //acos(0.95)=18 degs
  {
    return Seek(evader->Pos());
  }

  //Not considered ahead so we predict where the evader will be.
 
  //the lookahead time is propotional to the distance between the evader
  //and the pursuer; and is inversely proportional to the sum of the
  //agent's velocities
  double LookAheadTime = ToEvader.Length() / 
                        (m_pAgent->MaxSpeed() + evader->Speed());
  
  //now seek to the predicted future position of the evader
  return Seek(evader->Pos() + evader->Velocity() * LookAheadTime);
}


//----------------------------- Evade ------------------------------------
//
//  similar to pursuit except the agent Flees from the estimated future
//  position of the pursuer
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Evade(const Agent* pursuer)
{
  /* Not necessary to include the check for facing direction this time */

  Vector2D ToPursuer = pursuer->Pos() - m_pAgent->Pos();

  //uncomment the following two lines to have Evade only consider pursuers 
  //within a 'threat range'
  const double ThreatRange = 100.0;
  if (ToPursuer.LengthSq() > ThreatRange * ThreatRange) return Vector2D();
 
  //the lookahead time is propotional to the distance between the pursuer
  //and the pursuer; and is inversely proportional to the sum of the
  //agents' velocities
  double LookAheadTime = ToPursuer.Length() / 
                         (m_pAgent->MaxSpeed() + pursuer->Speed());
  
  //now flee away from predicted future position of the pursuer
  return Flee(pursuer->Pos() + pursuer->Velocity() * LookAheadTime);
}


//--------------------------- Wander -------------------------------------
//
//  This behavior makes the agent wander about randomly
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Wander()
{ 
  //this behavior is dependent on the update rate, so this line must
  //be included when using time independent framerate.
  double JitterThisTimeSlice = m_dWanderJitter * m_pAgent->TimeElapsed();

  //first, add a small random vector to the target's position
  m_vWanderTarget += Vector2D(RandomClamped() * JitterThisTimeSlice,
                              RandomClamped() * JitterThisTimeSlice);

  //reproject this new vector back on to a unit circle
  m_vWanderTarget.Normalize();

  //increase the length of the vector to the same as the radius
  //of the wander circle
  m_vWanderTarget *= m_dWanderRadius;

  //move the target into a position WanderDist in front of the agent
  Vector2D target = m_vWanderTarget + Vector2D(m_dWanderDistance, 0);

  //project the target into world space
  Vector2D Target = PointToWorldSpace(target,
                                       m_pAgent->Heading(),
                                       m_pAgent->Side(), 
                                       m_pAgent->Pos());

  //and steer towards it
  return Target - m_pAgent->Pos(); 
}


//---------------------- ObstacleAvoidance -------------------------------
//
//  Given a vector of CObstacles, this method returns a steering force
//  that will prevent the agent colliding with the closest obstacle
//------------------------------------------------------------------------
Vector2D SteeringBehavior::ObstacleAvoidance(const std::vector<BaseGameEntity*>& obstacles)
{
  //the detection box length is proportional to the agent's velocity
  m_dDBoxLength = Prm.MinDetectionBoxLength + 
                  (m_pAgent->Speed()/m_pAgent->MaxSpeed()) *
                  Prm.MinDetectionBoxLength;

  //tag all obstacles within range of the box for processing
  m_pAgent->GetWorld()->GetGameMap()->TagHavensWithinViewRange(m_pAgent, m_dDBoxLength);

  //this will keep track of the closest intersecting obstacle (CIB)
  BaseGameEntity* ClosestIntersectingObstacle = NULL;
 
  //this will be used to track the distance to the CIB
  double DistToClosestIP = MaxDouble;

  //this will record the transformed local coordinates of the CIB
  Vector2D LocalPosOfClosestObstacle;

  std::vector<BaseGameEntity*>::const_iterator curOb = obstacles.begin();

  while(curOb != obstacles.end())
  {
    //if the obstacle has been tagged within range proceed
    if ((*curOb)->IsTagged())
    {
      //calculate this obstacle's position in local space
      Vector2D LocalPos = PointToLocalSpace((*curOb)->Pos(),
                                             m_pAgent->Heading(),
                                             m_pAgent->Side(),
                                             m_pAgent->Pos());

      //if the local position has a negative x value then it must lay
      //behind the agent. (in which case it can be ignored)
      if (LocalPos.x >= 0)
      {
        //if the distance from the x axis to the object's position is less
        //than its radius + half the width of the detection box then there
        //is a potential intersection.
        double ExpandedRadius = (*curOb)->BRadius() + m_pAgent->BRadius();

        if (fabs(LocalPos.y) < ExpandedRadius)
        {
          //now to do a line/circle intersection test. The center of the 
          //circle is represented by (cX, cY). The intersection points are 
          //given by the formula x = cX +/-sqrt(r^2-cY^2) for y=0. 
          //We only need to look at the smallest positive value of x because
          //that will be the closest point of intersection.
          double cX = LocalPos.x;
          double cY = LocalPos.y;
          
          //we only need to calculate the sqrt part of the above equation once
          double SqrtPart = sqrt(ExpandedRadius*ExpandedRadius - cY*cY);

          double ip = cX - SqrtPart;

          if (ip <= 0.0)
          {
            ip = cX + SqrtPart;
          }

          //test to see if this is the closest so far. If it is keep a
          //record of the obstacle and its local coordinates
          if (ip < DistToClosestIP)
          {
            DistToClosestIP = ip;

            ClosestIntersectingObstacle = *curOb;

            LocalPosOfClosestObstacle = LocalPos;
          }         
        }
      }
    }

    ++curOb;
  }

  //if we have found an intersecting obstacle, calculate a steering 
  //force away from it
  Vector2D SteeringForce;

  if (ClosestIntersectingObstacle)
  {
    //the closer the agent is to an object, the stronger the 
    //steering force should be
    double multiplier = 1.0 + (m_dDBoxLength - LocalPosOfClosestObstacle.x) /
                        m_dDBoxLength;

    //calculate the lateral force
    SteeringForce.y = (ClosestIntersectingObstacle->BRadius()-
                       LocalPosOfClosestObstacle.y)  * multiplier;   

    //apply a braking force proportional to the obstacles distance from
    //the Agent. 
    const double BrakingWeight = 0.2;

    SteeringForce.x = (ClosestIntersectingObstacle->BRadius() - 
                       LocalPosOfClosestObstacle.x) * 
                       BrakingWeight;
  }

  //finally, convert the steering vector from local to world space
  return VectorToWorldSpace(SteeringForce,
                            m_pAgent->Heading(),
                            m_pAgent->Side());
}


//--------------------------- WallAvoidance --------------------------------
//
//  This returns a steering force that will keep the agent away from any
//  walls it may encounter
//------------------------------------------------------------------------
Vector2D SteeringBehavior::WallAvoidance(const std::vector<Wall2D*>& walls)
{
  //the feelers are contained in a std::vector, m_Feelers
  CreateFeelers();
  
  double DistToThisIP    = 0.0;
  double DistToClosestIP = MaxDouble;

  //this will hold an index into the vector of walls
  int ClosestWall = -1;

  Vector2D SteeringForce,
            point,         //used for storing temporary info
            ClosestPoint;  //holds the closest intersection point

  //examine each feeler in turn
  for (unsigned int flr=0; flr<m_Feelers.size(); ++flr)
  {
    //run through each wall checking for any intersection points
    for (unsigned int w=0; w<walls.size(); ++w)
    {
      if (LineIntersection2D(m_pAgent->Pos(),
                             m_Feelers[flr],
                             walls[w]->From(),
                             walls[w]->To(),
                             DistToThisIP,
                             point))
      {
        //is this the closest found so far? If so keep a record
        if (DistToThisIP < DistToClosestIP)
        {
          DistToClosestIP = DistToThisIP;

          ClosestWall = w;

          ClosestPoint = point;
        }
      }
    }//next wall

  
    //if an intersection point has been detected, calculate a force  
    //that will direct the agent away
    if (ClosestWall >=0)
    {
      //calculate by what distance the projected position of the agent
      //will overshoot the wall
      Vector2D OverShoot = m_Feelers[flr] - ClosestPoint;

      //create a force in the direction of the wall normal, with a 
      //magnitude of the overshoot
      SteeringForce = walls[ClosestWall]->Normal() * OverShoot.Length();
    }

  }//next feeler

  return SteeringForce;
}


//------------------------------- CreateFeelers --------------------------
//
//  Creates the antenna utilized by WallAvoidance
//------------------------------------------------------------------------
void SteeringBehavior::CreateFeelers()
{
  //feeler pointing straight in front
  m_Feelers[0] = m_pAgent->Pos() + m_dWallDetectionFeelerLength * m_pAgent->Heading();

  //feeler to left
  Vector2D temp = m_pAgent->Heading();
  Vec2DRotateAroundOrigin(temp, HalfPi * 3.5f);
  m_Feelers[1] = m_pAgent->Pos() + m_dWallDetectionFeelerLength/2.0f * temp;

  //feeler to right
  temp = m_pAgent->Heading();
  Vec2DRotateAroundOrigin(temp, HalfPi * 0.5f);
  m_Feelers[2] = m_pAgent->Pos() + m_dWallDetectionFeelerLength/2.0f * temp;
}


//---------------------------- Separation --------------------------------
//
// this calculates a force repelling from the other neighbors
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Separation(const vector<Agent*> &neighbors)
{  
  Vector2D SteeringForce;

  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure this agent isn't included in the calculations and that
    //the agent being examined is close enough. ***also make sure it doesn't
    //include the evade target ***
    if(neighbors[a] != m_pTargetAgent1)
    {
      Vector2D ToAgent = m_pAgent->Pos() - neighbors[a]->Pos();

      //scale the force inversely proportional to the agents distance  
      //from its neighbor.
	  if (ToAgent.Length() > 0.000001)
	  {
		  SteeringForce += Vec2DNormalize(ToAgent)/ToAgent.Length();
	  }
    }
  }

  return SteeringForce;
}


//---------------------------- Alignment ---------------------------------
//
//  returns a force that attempts to align this agents heading with that
//  of its neighbors
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Alignment(const vector<Agent*>& neighbors)
{
  //used to record the average heading of the neighbors
  Vector2D AverageHeading;

  //used to count the number of Agents in the neighborhood
  int    NeighborCount = 0;

  //iterate through all the tagged Agents and sum their heading vectors  
  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined  is close enough ***also make sure it doesn't
    //include any evade target ***
    if(neighbors[a] != m_pTargetAgent1)
    {
      AverageHeading += neighbors[a]->Heading();

      ++NeighborCount;
    }
  }

  //if the neighborhood contained one or more Agents, average their
  //heading vectors.
  if (NeighborCount > 0)
  {
    AverageHeading /= (double)NeighborCount;

    AverageHeading -= m_pAgent->Heading();
  }
  
  return AverageHeading;
}

//-------------------------------- Cohesion ------------------------------
//
//  returns a steering force that attempts to move the agent towards the
//  center of mass of the agents in its immediate area
//------------------------------------------------------------------------
Vector2D SteeringBehavior::Cohesion(const vector<Agent*> &neighbors)
{
  //first find the center of mass of all the agents
  Vector2D CenterOfMass, SteeringForce;

  int NeighborCount = 0;

  //iterate through the neighbors and sum up all the position vectors
  for (unsigned int a=0; a<neighbors.size(); ++a)
  {
    //make sure *this* agent isn't included in the calculations and that
    //the agent being examined is close enough ***also make sure it doesn't
    //include the evade target ***
    if(neighbors[a] != m_pTargetAgent1)
    {
      CenterOfMass += neighbors[a]->Pos();

      ++NeighborCount;
    }
  }

  if (NeighborCount > 0)
  {
    //the center of mass is the average of the sum of positions
    CenterOfMass /= (double)NeighborCount;

    //now seek towards that position
    SteeringForce = Seek(CenterOfMass);
  }

  //the magnitude of cohesion is usually much larger than separation or
  //allignment so it usually helps to normalize it.
  return Vec2DNormalize(SteeringForce);
}


Vector2D SteeringBehavior::FollowPath()
{ 
  //move to next target if close enough to current target (working in
  //distance squared space)
  if(Vec2DDistanceSq(m_pPath->CurrentWaypoint(), m_pAgent->Pos()) <
     m_dWaypointSeekDistSq)
  {
    m_pPath->SetNextWaypoint();
  }

  if (!m_pPath->Finished())
  {
    return Seek(m_pPath->CurrentWaypoint());
  }

  else
  {
    return Arrive(m_pPath->CurrentWaypoint(), normal);
  }
}


//void SteeringBehavior::RenderAids( )
//{ 
//  
//  gdi->TransparentText();
//  gdi->TextColor(Cgdi::grey);
//
//  int NextSlot = 0; int SlotSize = 20;
//
//  if (KEYDOWN(VK_INSERT)){m_pAgent->SetMaxForce(m_pAgent->MaxForce() + 1000.0f*m_pAgent->TimeElapsed());} 
//  if (KEYDOWN(VK_DELETE)){if (m_pAgent->MaxForce() > 0.2f) m_pAgent->SetMaxForce(m_pAgent->MaxForce() - 1000.0f*m_pAgent->TimeElapsed());}
//  if (KEYDOWN(VK_HOME)){m_pAgent->SetMaxSpeed(m_pAgent->MaxSpeed() + 50.0f*m_pAgent->TimeElapsed());}
//  if (KEYDOWN(VK_END)){if (m_pAgent->MaxSpeed() > 0.2f) m_pAgent->SetMaxSpeed(m_pAgent->MaxSpeed() - 50.0f*m_pAgent->TimeElapsed());}
//
//  if (m_pAgent->MaxForce() < 0) m_pAgent->SetMaxForce(0.0f);
//  if (m_pAgent->MaxSpeed() < 0) m_pAgent->SetMaxSpeed(0.0f);
//  
//  if (m_pAgent->ID() == 0){ gdi->TextAtPos(5,NextSlot,"MaxForce(Ins/Del):"); gdi->TextAtPos(160,NextSlot,ttos(m_pAgent->MaxForce()/Prm.SteeringForceTweaker)); NextSlot+=SlotSize;}
//  if (m_pAgent->ID() == 0){ gdi->TextAtPos(5,NextSlot,"MaxSpeed(Home/End):"); gdi->TextAtPos(160,NextSlot,ttos(m_pAgent->MaxSpeed()));NextSlot+=SlotSize;}
//
//  //render the steering force
//  if (m_pAgent->GetWorld()->RenderSteeringForce())
//  {  
//    gdi->RedPen();
//    Vector2D F = (m_vSteeringForce / Prm.SteeringForceTweaker) * Prm.AgentScale ;
//    gdi->Line(m_pAgent->Pos(), m_pAgent->Pos() + F);
//  }
//
//  //render wander stuff if relevant
//  if (On(wander) && m_pAgent->GetWorld()->RenderWanderCircle())
//  {    
//    if (KEYDOWN('F')){m_dWanderJitter+=1.0f*m_pAgent->TimeElapsed(); Clamp(m_dWanderJitter, 0.0f, 100.0f);}
//    if (KEYDOWN('V')){m_dWanderJitter-=1.0f*m_pAgent->TimeElapsed(); Clamp(m_dWanderJitter, 0.0f, 100.0f );}
//    if (KEYDOWN('G')){m_dWanderDistance+=2.0f*m_pAgent->TimeElapsed(); Clamp(m_dWanderDistance, 0.0f, 50.0f);}
//    if (KEYDOWN('B')){m_dWanderDistance-=2.0f*m_pAgent->TimeElapsed(); Clamp(m_dWanderDistance, 0.0f, 50.0f);}
//    if (KEYDOWN('H')){m_dWanderRadius+=2.0f*m_pAgent->TimeElapsed(); Clamp(m_dWanderRadius, 0.0f, 100.0f);}
//    if (KEYDOWN('N')){m_dWanderRadius-=2.0f*m_pAgent->TimeElapsed(); Clamp(m_dWanderRadius, 0.0f, 100.0f);}
//
// 
//    if (m_pAgent->ID() == 0){ gdi->TextAtPos(5,NextSlot, "Jitter(F/V): "); gdi->TextAtPos(160, NextSlot, ttos(m_dWanderJitter));NextSlot+=SlotSize;}
//    if (m_pAgent->ID() == 0) {gdi->TextAtPos(5,NextSlot,"Distance(G/B): "); gdi->TextAtPos(160, NextSlot, ttos(m_dWanderDistance));NextSlot+=SlotSize;}
//    if (m_pAgent->ID() == 0) {gdi->TextAtPos(5,NextSlot,"Radius(H/N): ");gdi->TextAtPos(160, NextSlot,  ttos(m_dWanderRadius));NextSlot+=SlotSize;}
//
//    
//    //calculate the center of the wander circle
//    Vector2D m_vTCC = PointToWorldSpace(Vector2D(m_dWanderDistance*m_pAgent->BRadius(), 0),
//                                         m_pAgent->Heading(),
//                                         m_pAgent->Side(),
//                                         m_pAgent->Pos());
//    //draw the wander circle
//    gdi->GreenPen();
//    gdi->HollowBrush();
//    gdi->Circle(m_vTCC, m_dWanderRadius*m_pAgent->BRadius()); 
//
//    //draw the wander target
//    gdi->RedPen();
//    gdi->Circle(PointToWorldSpace((m_vWanderTarget + Vector2D(m_dWanderDistance,0))*m_pAgent->BRadius(),
//                                  m_pAgent->Heading(),
//                                  m_pAgent->Side(),
//                                  m_pAgent->Pos()), 3);                                  
//  }
//
//  //render the detection box if relevant
//  if (m_pAgent->GetWorld()->RenderDetectionBox())
//  {
//    gdi->GreyPen();
//
//    //a vertex buffer rqd for drawing the detection box
//    static std::vector<Vector2D> box(4);
//
//    double length = Prm.MinDetectionBoxLength + 
//                  (m_pAgent->Speed()/m_pAgent->MaxSpeed()) *
//                  Prm.MinDetectionBoxLength;
//
//    //verts for the detection box buffer
//    box[0] = Vector2D(0,m_pAgent->BRadius());
//    box[1] = Vector2D(length, m_pAgent->BRadius());
//    box[2] = Vector2D(length, -m_pAgent->BRadius());
//    box[3] = Vector2D(0, -m_pAgent->BRadius());
// 
//  
//    if (!m_pAgent->isSmoothingOn())
//    {
//      box = WorldTransform(box,m_pAgent->Pos(),m_pAgent->Heading(),m_pAgent->Side());
//      gdi->ClosedShape(box);
//    }
//    else
//    {
//      box = WorldTransform(box,m_pAgent->Pos(),m_pAgent->SmoothedHeading(),m_pAgent->SmoothedHeading().Perp());
//      gdi->ClosedShape(box);
//    } 
//
//
//    //////////////////////////////////////////////////////////////////////////
//    //the detection box length is proportional to the agent's velocity
//  m_dDBoxLength = Prm.MinDetectionBoxLength + 
//                  (m_pAgent->Speed()/m_pAgent->MaxSpeed()) *
//                  Prm.MinDetectionBoxLength;
//
//  //tag all obstacles within range of the box for processing
//  m_pAgent->GetWorld()->TagObstaclesWithinViewRange(m_pAgent, m_dDBoxLength);
//
//  //this will keep track of the closest intersecting obstacle (CIB)
//  BaseGameEntity* ClosestIntersectingObstacle = NULL;
// 
//  //this will be used to track the distance to the CIB
//  double DistToClosestIP = MaxDouble;
//
//  //this will record the transformed local coordinates of the CIB
//  Vector2D LocalPosOfClosestObstacle;
//
//  std::vector<BaseGameEntity*>::const_iterator curOb = m_pAgent->GetWorld()->GetGameMap()->GetObstacles().begin();
//
//  while(curOb != m_pAgent->GetWorld()->GetGameMap()->GetObstacles().end())
//  {
//    //if the obstacle has been tagged within range proceed
//    if ((*curOb)->IsTagged())
//    {
//      //calculate this obstacle's position in local space
//      Vector2D LocalPos = PointToLocalSpace((*curOb)->Pos(),
//                                             m_pAgent->Heading(),
//                                             m_pAgent->Side(),
//                                             m_pAgent->Pos());
//
//      //if the local position has a negative x value then it must lay
//      //behind the agent. (in which case it can be ignored)
//      if (LocalPos.x >= 0)
//      {
//        //if the distance from the x axis to the object's position is less
//        //than its radius + half the width of the detection box then there
//        //is a potential intersection.
//        if (fabs(LocalPos.y) < ((*curOb)->BRadius() + m_pAgent->BRadius()))
//        {
//          gdi->ThickRedPen();
//          gdi->ClosedShape(box);        
//        }
//      }
//    }
//
//    ++curOb;
//  }
//
//
///////////////////////////////////////////////////////
//  }
//
//  //render the wall avoidnace feelers
//  if (On(wall_avoidance) && m_pAgent->GetWorld()->RenderFeelers())
//  {
//    gdi->OrangePen();
//
//    for (unsigned int flr=0; flr<m_Feelers.size(); ++flr)
//    {
//
//      gdi->Line(m_pAgent->Pos(), m_Feelers[flr]);
//    }            
//  }  
//
//  //render path info
//  if (On(follow_path) && m_pAgent->GetWorld()->RenderPath())
//  {
//    m_pPath->Render();
//  }
//
//  
//  if (On(separation))
//  {
//    if (m_pAgent->ID() == 0){ gdi->TextAtPos(5, NextSlot, "Separation(S/X):");gdi->TextAtPos(160,NextSlot, ttos(m_dWeightSeparation/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}
//
//    if (KEYDOWN('S')){m_dWeightSeparation += 200*m_pAgent->TimeElapsed(); Clamp(m_dWeightSeparation, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
//    if (KEYDOWN('X')){m_dWeightSeparation -= 200*m_pAgent->TimeElapsed();Clamp(m_dWeightSeparation, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
//  }
//
//  if (On(allignment))
//  {
//    if (m_pAgent->ID() == 0) {gdi->TextAtPos(5, NextSlot, "Alignment(A/Z):"); gdi->TextAtPos(160, NextSlot, ttos(m_dWeightAlignment/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}
//
//    if (KEYDOWN('A')){m_dWeightAlignment += 200*m_pAgent->TimeElapsed();Clamp(m_dWeightAlignment, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
//    if (KEYDOWN('Z')){m_dWeightAlignment -= 200*m_pAgent->TimeElapsed();Clamp(m_dWeightAlignment, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
//  }
//
//  if (On(cohesion))
//  {
//    if (m_pAgent->ID() == 0) {gdi->TextAtPos(5, NextSlot, "Cohesion(D/C):"); gdi->TextAtPos(160, NextSlot, ttos(m_dWeightCohesion/Prm.SteeringForceTweaker));NextSlot+=SlotSize;}
//    if (KEYDOWN('D')){m_dWeightCohesion += 200*m_pAgent->TimeElapsed();Clamp(m_dWeightCohesion, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
//    if (KEYDOWN('C')){m_dWeightCohesion -= 200*m_pAgent->TimeElapsed();Clamp(m_dWeightCohesion, 0.0f, 50.0f * Prm.SteeringForceTweaker);}
//  }
//
//  if (On(follow_path))
//  { 
//    double sd = sqrt(m_dWaypointSeekDistSq);
//    if (m_pAgent->ID() == 0){ gdi->TextAtPos(5, NextSlot, "SeekDistance(D/C):");gdi->TextAtPos(160, NextSlot,ttos(sd));NextSlot+=SlotSize;}
//    
//    if (KEYDOWN('D')){m_dWaypointSeekDistSq += 1.0;}
//    if (KEYDOWN('C')){m_dWaypointSeekDistSq -= 1.0; Clamp(m_dWaypointSeekDistSq, 0.0f, 400.0f);}
//  }  
//
//}





