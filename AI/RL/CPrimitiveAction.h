#pragma once
#include "Action.h"
#include "../../Prey.h"
#include <string>

using namespace std;

// all the actions prey can perform
enum PRIMITIVEACTION{ SeekSafety, Flee, Eat,Forage,Flock, Wander, Assist,Charge};

class PrimitiveAction:
	public CAction
{
public:
	PrimitiveAction();
	PrimitiveAction(PRIMITIVEACTION act);
	~PrimitiveAction(void);

private:
	const static int PrimitiveActionNum = 8;

public:
	//for debug
	virtual string getActionName();

	/// Compares the action.
	virtual bool isSameAction(CAction *action);

	void executeAction(Prey* pPrey);
};
