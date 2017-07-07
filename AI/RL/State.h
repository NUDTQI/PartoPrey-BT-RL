#pragma once
#include <vector>

class Prey;

class Discreter
{
public:
	int CalculateVarDiscrete(int val, const int MaxVal, const int MidVal, const int LowVal)
	{
		if (val >= MaxVal){	return 3;}
		else if(val >= MidVal){	return 2;}
		else if(val >= LowVal){	return 1;}
		else{return 0;}
	}
};

class CState
{
public:
	CState(void){}
	~CState(void){}

	std::vector<int> output;
	Discreter PubDiscreter;

public:
	virtual bool IsSameState(CState *pS){return false;}
	virtual void UpdateDiscreteState(Prey* owner){}
	virtual void CopyState(CState* pS){}
	virtual std::vector<int>& PrintStateValues(){std::vector<int> temp; return temp;}
};

