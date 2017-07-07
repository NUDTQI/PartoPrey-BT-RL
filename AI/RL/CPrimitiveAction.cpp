#include "CPrimitiveAction.h"
#include "..\..\Prey.h"


PrimitiveAction::PrimitiveAction()
{
	m_type = ActionType::Primitive;
}

PrimitiveAction::PrimitiveAction( PRIMITIVEACTION act ) 
{
	m_type = (ActionType)0;
	m_eSelectedAction = (int)act;

	switch (act)
	{
		case PRIMITIVEACTION::Wander:
			m_sActionName = "Wander";
			break;
		case PRIMITIVEACTION::Assist:
			m_sActionName = "Assist";
			break;
		case PRIMITIVEACTION::Charge:
			m_sActionName = "Charge";
			break;
		case PRIMITIVEACTION::Eat:
			m_sActionName = "Eat";
			break;
		case PRIMITIVEACTION::Flee:
			m_sActionName = "Flee";
			break;
		case PRIMITIVEACTION::Flock:
			m_sActionName = "Flock";
			break;
		case PRIMITIVEACTION::Forage:
			m_sActionName = "Forage";
			break;
		case PRIMITIVEACTION::SeekSafety:
			m_sActionName = "SeekSafety";
			break;
	};
}

PrimitiveAction::~PrimitiveAction(void)
{
}

std::string PrimitiveAction::getActionName()
{
	string ActionName = "";
	switch (m_eSelectedAction)
	{
	case PRIMITIVEACTION::Wander:
		ActionName = "Wander";
		break;
	case PRIMITIVEACTION::Assist:
		ActionName = "Assist";
		break;
	case PRIMITIVEACTION::Charge:
		ActionName = "Charge";
		break;
	case PRIMITIVEACTION::Eat:
		ActionName = "Eat";
		break;
	case PRIMITIVEACTION::Flee:
		ActionName = "Flee";
		break;
	case PRIMITIVEACTION::Flock:
		ActionName = "Flock";
		break;
	case PRIMITIVEACTION::Forage:
		ActionName = "Forage";
		break;
	case PRIMITIVEACTION::SeekSafety:
		ActionName = "SeekSafety";
		break;
	default:
		ActionName = "Wander";
	}
	return ActionName;
}

void PrimitiveAction::executeAction(Prey* pPrey)
{
	switch (m_eSelectedAction)
	{
	}
}

bool PrimitiveAction::isSameAction( CAction *action )
{
	if (action->getActionType()==m_type && action->getAction()==m_eSelectedAction)
	{
		return true;
	}
	return false;
}

