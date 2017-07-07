#pragma once
#include "State.h"
#include <map>

using namespace std;

class QFunction
{
public:
	QFunction(void);
	~QFunction(void);

	std::map<CState *, double> qf;

	float getQValue(CState* state);
	void  setQValue(CState* state, float v);
};

