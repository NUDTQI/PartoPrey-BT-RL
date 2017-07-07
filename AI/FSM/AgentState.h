#ifndef CAGENTSTATE_H
#define CAGENTSTATE_H
#include <string>
/************************************************************************/
/*                          2012-3-6     秦龙                               
智能体状态机对象基类，封装了该状态的执行函数
*/
/************************************************************************/
#pragma once

enum EStatePriority   // 状态机的优先级
{
	EStatePriority_Minimal = 0,
	EStatePriority_Minal = 1,
	EStatePriority_Midle = 2,
	EStatePriority_Upper = 3,
	EStatePriority_Most = 4
};

template <typename entity_type>
class CAgentState
{
public:
	entity_type *m_pOwner;
	virtual ~CAgentState(){};
	CAgentState()
	{
		m_pOwner = 0;
		m_ExecuteTime = 0;
		m_bIsDone = false;
		m_ParaInfo = L"none";
		m_CurPhase = 0;
		m_priority = EStatePriority_Midle;    // 默认状态机的优先级是中等
	};
	virtual bool Enter(void)=0;       // 第一次进入状态需要处理的例程
	virtual void Execute(void)=0;     // 该状态下每一个步长需要处理的例程
	virtual bool IsDone(void)=0;      // 检查本状态是否已完成，可退出？
	virtual bool Exit(void)=0;        // 退出本状态前需要处理的例程

	int m_CurPhase;                   // 当前执行阶段
	int m_ExecuteTime;                // 当前执行时间
	bool m_bIsDone;                   // 是否执行完当前状态
    std::wstring m_StateName;         // 有限状态机名称
	std::wstring m_ParaInfo;          // 状态机传出的参数信息，以#号隔开
	EStatePriority m_priority;        // 状态机优先级
};
#endif




