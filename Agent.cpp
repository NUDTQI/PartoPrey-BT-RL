#include "Agent.h"
#include "2d/C2DMatrix.h"
#include "2d/Geometry.h"
#include "SteeringBehaviors.h"
#include "2d/Transformations.h"
#include "GameWorld.h"
#include "misc/CellSpacePartition.h"
#include "misc/cgdi.h"
#include "SensorMemory.h"
#include "GameMap.h"

using std::vector;
using std::list;


//----------------------------- ctor -------------------------------------
//------------------------------------------------------------------------
Agent::Agent(GameWorld* world,
               Vector2D position,
               double    rotation,
               Vector2D velocity,
               double    mass,
               double    max_force,
               double    max_speed,
               double    max_turn_rate,
               double    scale,
			   double    ViewDistance,
			   int       max_Health):    MovingEntity(position,
                                                 scale,
                                                 velocity,
                                                 max_speed,
                                                 Vector2D(sin(rotation),-cos(rotation)),
                                                 mass,
                                                 Vector2D(scale,scale),
                                                 max_turn_rate,
                                                 max_force),

                                       m_pWorld(world),
                                       m_vSmoothedHeading(Vector2D(0,0)),
                                       m_bSmoothingOn(false),
                                       m_dTimeElapsed(0.0),
									   m_dSensorRange(ViewDistance),
									   m_iHealth(max_Health),
									   m_iMaxHealth(max_Health),
									   m_iScore(0)
{  
  InitializeBuffer();


  m_dFieldOfView = 2*pi;

  SetAlive();

  //set up the steering behavior class
  m_pSteering = new SteeringBehavior(this);    

  //set up the steering behavior class
  m_pSenseSys = new SensorMemory(this);

  //set up the smoother
  m_pHeadingSmoother = new Smoother<Vector2D>(Prm.NumSamplesForSmoothing, Vector2D(0.0, 0.0)); 

  m_bDamaged = false;
}


//---------------------------- dtor -------------------------------------
//-----------------------------------------------------------------------
Agent::~Agent()
{
  delete m_pSteering;
  m_pSteering = NULL;
  delete m_pSenseSys;
  m_pSenseSys = NULL;
  delete m_pHeadingSmoother;
  m_pHeadingSmoother = NULL;
}

//------------------------------ Update ----------------------------------
//
//  Updates the Agent's position from a series of steering behaviors
//------------------------------------------------------------------------
void Agent::UpdateMovement(double time_elapsed)
{
	//update the time elapsed
	m_dTimeElapsed = time_elapsed;

	//calculate the combined steering force
	Vector2D force = m_pSteering->Calculate();

	//if no steering force is produced decelerate the player by applying a
	//braking force
	if (m_pSteering->Force().isZero())
	{
		const double BrakingRate = 0.8; 
 
		m_vVelocity = m_vVelocity * BrakingRate;                                     
	}

	//calculate the acceleration
	Vector2D accel = force / m_dMass;

	//update the velocity
	m_vVelocity += accel* time_elapsed;

	//make sure Agent does not exceed maximum velocity
	m_vVelocity.Truncate(m_dMaxSpeed);

	//update the position
	m_vPos += m_vVelocity * time_elapsed;

	//if the Agent has a non zero velocity the heading and side vectors must 
	//be updated
	if (!m_vVelocity.isZero())
	{    
		m_vHeading = Vec2DNormalize(m_vVelocity);

		m_vSide = m_vHeading.Perp();
	}

	//treat the screen as a toroid
	WrapAround(m_vPos, m_pWorld->cxClient(), m_pWorld->cyClient());
}


//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void Agent::Render()
{ 
	gdi->BluePen(); 
	gdi->HollowBrush();
	gdi->Circle(Pos(), BRadius()+1);
}


//----------------------------- InitializeBuffer -----------------------------
//
//  fills the Agent's shape buffer with its vertices
//-----------------------------------------------------------------------------
void Agent::InitializeBuffer()
{
  const int NumAgentVerts = 3;

  Vector2D Agent[NumAgentVerts] = {Vector2D(-1.0f,0.6f),
                                        Vector2D(1.0f,0.0f),
                                        Vector2D(-1.0f,-0.6f)};

  //setup the vertex buffers and calculate the bounding radius
  for (int vtx=0; vtx<NumAgentVerts; ++vtx)
  {
    m_vecAgentVB.push_back(Agent[vtx]);
  }
}

void Agent::UpdateSensorSys()
{
	m_pSenseSys->UpdateVision();
}


void Agent::RestoreHealthToMaximum()
{
	m_iHealth = m_iMaxHealth;
}

//------------------ RotateFacingTowardPosition -------------------------------
//
//  given a target position, this method rotates the bot's facing vector
//  by an amount not greater than m_dMaxTurnRate until it
//  directly faces the target.
//
//  returns true when the heading is facing in the desired direction
bool Agent::RotateFacingTowardPosition( Vector2D target )
{
	Vector2D toTarget = Vec2DNormalize(target - Pos());

	double dot = m_vFacing.Dot(toTarget);

	//clamp to rectify any rounding errors
	Clamp(dot, -1, 1);

	//determine the angle between the heading vector and the target
	double angle = acos(dot);

	//return true if the bot's facing is within WeaponAimTolerance degs of
	//facing the target
	const double WeaponAimTolerance = 0.01; //2 degs approx

	if (angle < WeaponAimTolerance)
	{
		m_vFacing = toTarget;
		return true;
	}

	//clamp the amount to turn to the max turn rate
	if (angle > m_dMaxTurnRate) angle = m_dMaxTurnRate;

	//The next few lines use a rotation matrix to rotate the player's facing
	//vector accordingly
	C2DMatrix RotationMatrix;

	//notice how the direction of rotation has to be determined when creating
	//the rotation matrix
	RotationMatrix.Rotate(angle * m_vFacing.Sign(toTarget));	
	RotationMatrix.TransformVector2Ds(m_vFacing);

	return false;
}

void Agent::ReduceHealth( unsigned int val )
{
	m_iHealth -= val;

	if (m_iHealth <= 0)
	{
		SetDead();
	}
}

void Agent::IncreaseHealth( unsigned int val )
{
	m_iHealth+=val; 
	Clamp(m_iHealth, 0, m_iMaxHealth);
}

void Agent::Update(double time_elapsed)
{
	//update the sensory memory with any visual stimulus
	UpdateSensorSys();

	//think-decide to choose action
	SelectAction();

	//Calculate the steering force and update the bot's velocity and position
	UpdateMovement(time_elapsed);
}

void Agent::SelectAction()
{

}

std::vector<Agent*>& Agent::GetAllyBotsInRange()
{
	return GetSensorMem()->m_AllBotsInRange;
}

Prey* Agent::GetNearestPrey()
{
	return GetSensorMem()->pNearestPrey;
}

Predator* Agent::GetNearestPredator()
{
	return GetSensorMem()->pNearestPredator;
}

BaseGameEntity* Agent::GetNearestHaven()
{
	return GetSensorMem()->pNearestHaven;
}

BaseGameEntity* Agent::GetNearestFood()
{
	return GetSensorMem()->pNearestFood;
}

