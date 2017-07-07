#include "CMyEnvModel.h"


CMyEnvModel::CMyEnvModel(void):	CEnvironmentModel(2,5)
{
	properties->setMinValue(0,0);
	properties->setMaxValue(0,1);

	properties->setMinValue(1,0);
	properties->setMaxValue(1,1);

	properties->setDiscreteStateSize(0,2);
	properties->setDiscreteStateSize(1,2);
	properties->setDiscreteStateSize(2,2);
	properties->setDiscreteStateSize(3,4);
	properties->setDiscreteStateSize(4,3);

}

void CMyEnvModel::getState(CState* state)
{
	CEnvironmentModel::getState(state);

	//state->setContinuousState(0,Health);
	//state->setDiscreteState(0,);
	//state->setDiscreteState(1,);

}


CMyEnvModel::~CMyEnvModel(void)
{
}
