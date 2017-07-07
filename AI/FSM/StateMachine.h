#ifndef CSTATE_MACHINE_H
#define CSTATE_MACHINE_H

#pragma once
#include "AgentState.h"
template <class entity_type>
class CStateMachine
{
public:
	entity_type *m_pOwner;                    // 该状态机实例的宿主指针变量
	CAgentState<entity_type> *m_pCurrentState;// 当前状态指针引用
	CAgentState<entity_type> *m_pGlobalState; // 全局状态指针引用
	

	// 状态机构造函数
	CStateMachine(entity_type* owner):m_pOwner(owner),m_pCurrentState(0)
	{
		m_pCurrentState=NULL;
		m_pGlobalState=NULL;
	}

	// 析构函数
	~CStateMachine()
	{
		if (m_pCurrentState!=0)
		{
			//m_pCurrentState->Exit();
			delete m_pCurrentState;
			m_pCurrentState=NULL;
		}
		if (m_pGlobalState!=0)
		{
			//m_pGlobalState->Exit();
			delete m_pGlobalState;
			m_pGlobalState=NULL;
		}
	}

	// 状态转移
	bool  ChangeState(CAgentState<entity_type>* pNewState)
	{
		bool b = false;
		if(m_pCurrentState!=0)
		{
			// 若当前状态机是非空，那么要首先检查新状态机的优先级是否高于当前状态机，
			// 若是，还要执行当前状态的退出处理函数
			if(pNewState->m_priority>=m_pCurrentState->m_priority)
			{
				std::wstring NewStateName = pNewState->m_StateName,OldStateName = m_pCurrentState->m_StateName;
				if(NewStateName == OldStateName) 
				{
					b = false;
				}
				else 
				{
					b = m_pCurrentState->Exit();
				}
			}
		}  
		else b=true;      // 若当前状态机是空的，则直接可以进入新的状态机
		if(!b){return b;}                                     // 若当前状态不能退出，则状态切换失败,函数返回
		b = pNewState->Enter();                               // 执行新状态的入口操作函数，为状态的更新执行做前期准备
		if(!b){return b;}                                     // 若不满足新状态的入口条件，则状态切换失败，函数返回
		if (m_pCurrentState!=0){delete m_pCurrentState;}      // 若当前状态非空，删除当前状态
		m_pCurrentState = pNewState;                          // 若状态入口操作成功完成，则切换新的状态
		return b;
	}

	// 状态转移
	bool  ChangeGlobalState(CAgentState<entity_type>* pNewState)
	{
		bool b = true;
		if(m_pGlobalState!=0){b=m_pGlobalState->Exit();}      // 执行当前状态的退出处理函数
		if(!b){return b;}                                     // 若当前状态不能退出，则状态切换失败,函数返回
		b = pNewState->Enter();                               // 执行新状态的入口操作函数，为状态的更新执行做前期准备
		if(!b){return b;}                                     // 若不满足新状态的入口条件，则状态切换失败，函数返回
		if (m_pGlobalState!=0){delete m_pGlobalState;}        // 若当前状态非空，删除当前状态
		m_pGlobalState = pNewState;                           // 若状态入口操作成功完成，则切换新的状态
		return b;
	}


	// 当前状态更新，若当前状态机之行为完毕，删除之，返回true，否则返回false;
	bool  Update()//const
	{
		// 执行当前状态
		bool b = false;
		if (m_pCurrentState) 
		{
			m_pCurrentState->Execute();
		    if(m_pCurrentState->IsDone())
	        {
			   m_pCurrentState->Exit();
	           delete m_pCurrentState;
	           m_pCurrentState=NULL;
	        }
		}
		else
		{
			// 当前状态为空，无需执行
			b=true;
		}
		return b;
	}

	// 全局状态更新
	void UpdateGlobalState()
	{
		if (m_pGlobalState) 
		{
			m_pGlobalState->Execute();
		}
	}

    // 完全关闭状态机
    void ShutdownStateMachine()
   {
       if(m_pGlobalState) {delete m_pGlobalState;m_pGlobalState=NULL;}
       if(m_pCurrentState){delete m_pCurrentState;m_pCurrentState=NULL;}
   }
};



#endif


