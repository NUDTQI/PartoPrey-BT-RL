#pragma once
#include "misc/utils.h"
#include <string>

enum ActionType{Primitive,Multistep,Extended};


using namespace std;

class CActionData
{
private:
	//is action finished
	bool m_bFinish;

	// duration an action long
	int m_iDuration;

	//
	bool bIsChangeAble;

public:
	CActionData(){m_iDuration=0;bIsChangeAble=true;m_bFinish=false;}
	~CActionData(){}

	const CActionData& operator=(const CActionData &rhs)
	{
		m_bFinish = rhs.m_bFinish;
		m_iDuration = rhs.m_iDuration;
		bIsChangeAble = rhs.bIsChangeAble;

		return *this;
	}

	///returns the duration of the action, per default 1
	virtual int getDuration() {return m_iDuration;}
	void setDuration(int dur){m_iDuration = dur;}

	bool isChangeAble();
	void setIsChangeAble(bool changeAble);

	//return is the action finished
	virtual bool isFinished(){return m_bFinish;}
	void setIsFinished(bool isFinish){m_bFinish=isFinish;}
};

class CAction
{
public:
	CAction(){pActionData=NULL;}
	CAction(ActionType type,int act){m_type = type;m_eSelectedAction = act;pActionData=NULL;}
	~CAction(void){};

protected:
	//action type
	ActionType m_type;
	//action index
	int m_eSelectedAction;
	//action name
	string m_sActionName;

	/// Pointer to the action executed by the extended Action.
	CAction *nextHierarchyLevel;
	CActionData* pActionData;

public:
	////////////////////////////set get assessor//////////////////////////////////////////////

	CActionData* getActionData(){return pActionData;}
	void setActionData(CActionData* dt){pActionData = dt;}

	void setAction(int act){m_eSelectedAction = act;}
	int getAction(){return m_eSelectedAction;}

	void setActionType(ActionType type){m_type = type;}
	ActionType getActionType(){return m_type;}
	//for debug
	void setActionName(string name){m_sActionName = name;}
	string getActionName(){return m_sActionName;}

	virtual void setDuration(int dur) {pActionData->setDuration(dur);}
	virtual int getDuration() {return pActionData->getDuration();}
	virtual void setisFinished(bool isfinish){pActionData->setIsFinished(isfinish);}
	virtual bool isFinished(){return pActionData->isFinished();}
	/////////////////////////method for assist/////////////////////////////////////////////////
	/// Compares the action.
	virtual bool isSameAction(CAction *action){return this==action;}
	// judge action type
	bool isType(int type){return type==(int)m_type;}

	////////////////////////////////execute action//////////////////////////////////////////
	virtual void executeAction(){};
};

