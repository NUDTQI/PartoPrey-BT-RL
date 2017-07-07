#pragma once
#include "cenvironmentmodel.h"
class CMyEnvModel:	public CEnvironmentModel
{
public:
	CMyEnvModel(void);
	~CMyEnvModel(void);



public:
	void getState(CState* state);
};

