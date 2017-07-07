#include "Action.h"
#include <list>

using namespace std;

class CState;

// all the actions prey can perform
enum EXTENDEDACTION{Retreat,Idle,Attack,Graze,Explore};

/// A list of Actions representing the actual hierarchical Stack
/** The actual hierarchical stack contains alls ExtendedActions which are aktiv at the moment and the 
PrimitiveAction which is returned by the last extended action. 
*/
class CHierarchicalStack: public std::list<CAction *> 
{
protected:
	
public:
	CHierarchicalStack();
	~CHierarchicalStack();
	
	void clearAndDelete();
};

// An action which can long for several steps.
/** An action with a duration different than one has to be treated with SemiMarkov-Learning rules in all
the learning algorithms, since otherwise the result can be far from optimal. Many Learning-Algorithm in the this package support
Semi-Markov Learning updates.
This action class maintains its own action data object, a CMultiStepActionData object. This data object stores the duration it has needed by now and the finished flag. The duration and the finished flag get normally set by an HierarchicalController, but it is also possible to set the duration for example in the environment model, if you have a primitive action which takes longer than other primitive actions. In that case your model-specific action has to be derivated from CPrimitiveAction and CMultiStepAction.
\par
The class also contains the method isFinished(CStateCollection *state). This method is used (normally by a hierarchical controller) to determine
wether the action has finished or not. The controller sets the finished flag according to isFinished, so other Listeners only have to
look at this flag. This method must be implemented by all (non-abstract) sub-classes.
*/

class CMultiStepAction :  public CAction
{
public:
	CMultiStepAction(){m_type = (ActionType)1;}
	CMultiStepAction(EXTENDEDACTION act):CAction((ActionType)1,int(act)){};

public:
	~CMultiStepAction(){};

	/**
	This method is normally used by a hierarchical controller to determine
	wether the action has finished or not. The finished method may depend only on the current state transition,
	so you get the old state and the new state as parameters. The controller sets the finished flag according to isFinished, 
	so other Listeners only have to look at this flag. This method must be implemented by all (non-abstract) sub-classes.
	*/
	virtual bool isFinished(CState *oldState, CState *newState);

	void setDuration(int dur){getActionData()->setDuration(dur);}
	/// returns the duration of the action (member: duration)
	virtual int getDuration() {return getActionData()->getDuration();};

	virtual void executeAction();
};


class CExtendedAction : public CMultiStepAction
{
public:
	CExtendedAction(){m_type = (ActionType)2;}
	CExtendedAction(EXTENDEDACTION act):CMultiStepAction(act){}

	~CExtendedAction(){}

public:
	/// Pointer to the action executed by the extended Action.
	CAction *nextHierarchyLevel;

	/// Constructs a hierarchical ActionStack, with the extended action itself as root.
	void getHierarchicalStack(CHierarchicalStack *actionStack);

	///determining the next action in the next hierarchie level.
	virtual CAction* getNextHierarchyLevel(CState *state);

	virtual bool isFinished(CState *oldState, CState *newState);

	virtual void executeAction();
};

