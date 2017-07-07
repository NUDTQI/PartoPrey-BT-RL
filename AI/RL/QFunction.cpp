#include "QFunction.h"
#include <map>
#include <assert.h>

QFunction::QFunction(void)
{
}


QFunction::~QFunction(void)
{
	for (std::map<CState *, double>::iterator it = qf.begin();it!=qf.end();it++)
	{
		CState* st = it->first;
		delete st;
		st = NULL;
	}
}

float QFunction::getQValue(CState* state)
{
	float value = 0.0;

	std::map<CState *, double>::iterator iter = qf.find(state);

	if(iter != qf.end())
	{
		value = iter->second;
	}
	else
	{
		qf.insert(std::make_pair(state,value));
	}

	return value;
}

void  QFunction::setQValue(CState* state, float v)
{
	//assert(qf[state]);

	qf[state] = v;
}