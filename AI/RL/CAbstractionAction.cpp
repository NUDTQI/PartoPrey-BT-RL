#include "CAbstractionAction.h"
#include "State.h"


CHierarchicalStack::CHierarchicalStack()
{

}

CHierarchicalStack::~CHierarchicalStack()
{

}

void CHierarchicalStack::clearAndDelete()
{
	for (CHierarchicalStack::iterator it = begin(); it != end(); it ++)
	{
		delete (*it);
	}
	clear();
}



bool CMultiStepAction::isFinished( CState *oldState, CState *newState )
{
	return getActionData()->isFinished();
}

void CMultiStepAction::executeAction()
{

}

void CExtendedAction::getHierarchicalStack( CHierarchicalStack *actionStack )
{
	actionStack->push_back(this);

	if (nextHierarchyLevel != NULL)
	{
		if (nextHierarchyLevel->isType((int)Extended))
		{
			CExtendedAction *exAction = dynamic_cast<CExtendedAction *>(nextHierarchyLevel);
			exAction->getHierarchicalStack(actionStack);
		}
		else
		{
			actionStack->push_back(nextHierarchyLevel);
		}
	}
}

CAction* CExtendedAction::getNextHierarchyLevel( CState *state )
{
	return NULL;
}

bool CExtendedAction::isFinished( CState *oldState, CState *newState )
{
	return getActionData()->isFinished();
}

void CExtendedAction::executeAction()
{

}

