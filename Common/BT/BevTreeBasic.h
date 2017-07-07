#pragma once
#include "Anydata.h"
#include <string>
#include <vector>

#define DLL_EXPORT _declspec(dllexport)
#define k_BLimited_MaxChildNodeCnt              16
#define k_BLimited_InvalidChildNodeIndex        k_BLimited_MaxChildNodeCnt
#define D_SafeDelete(p)		if(p!=NULL)		{ delete p; p = NULL; }

enum E_ParallelFinishCondition
{
	k_PFC_OR = 1,
	k_PFC_AND
};

enum BevRunningStatus
{
	k_BRS_Executing					= 0,
	k_BRS_Finish					= 1,
	k_BRS_ERROR_Transition			= -1,
};

enum E_TerminalNodeStaus
{
	k_TNS_Ready         = 1,
	k_TNS_Running       = 2,
	k_TNS_Finish        = 3,
};

typedef CAnydata BevNodeInputParam;
typedef CAnydata BevNodeOutputParam;

//-------------------------------------------------------------------------------------------------------------------------------------
class DLL_EXPORT BevNodePrecondition
{
public:
	BevNodePrecondition(){};
	~BevNodePrecondition(){};
	virtual bool ExternalCondition(const BevNodeInputParam& input) const = 0;
};

class DLL_EXPORT BevNodePreconditionTRUE : public BevNodePrecondition
{
public:
	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return true;
	}
};
class DLL_EXPORT BevNodePreconditionFALSE : public BevNodePrecondition
{
public:
	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return false;
	}
};

class DLL_EXPORT BevNodePreconditionNOT : public BevNodePrecondition
{
public:
	BevNodePreconditionNOT(BevNodePrecondition* lhs)
		:BevNodePrecondition(),
		m_lhs(lhs)
	{

	}

	~BevNodePreconditionNOT()
	{
		D_SafeDelete(m_lhs); 
	}

	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return !m_lhs->ExternalCondition(input);
	}

private:
	BevNodePrecondition* m_lhs;
};

class DLL_EXPORT BevNodePreconditionAND : public BevNodePrecondition
{
public:
	BevNodePreconditionAND(BevNodePrecondition* lhs, BevNodePrecondition* rhs)
		: m_lhs(lhs)
		, m_rhs(rhs)
	{
		//D_CHECK(m_lhs && m_rhs);
	}
	~BevNodePreconditionAND(){
		D_SafeDelete(m_lhs);
		D_SafeDelete(m_rhs);
	}
	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return m_lhs->ExternalCondition(input) && m_rhs->ExternalCondition(input);
	}
private:
	BevNodePrecondition* m_lhs;
	BevNodePrecondition* m_rhs;
};
class DLL_EXPORT BevNodePreconditionOR : public BevNodePrecondition
{
public:
	BevNodePreconditionOR(BevNodePrecondition* lhs, BevNodePrecondition* rhs)
		: m_lhs(lhs)
		, m_rhs(rhs)
	{
		//D_CHECK(m_lhs && m_rhs);
	}
	~BevNodePreconditionOR(){
		D_SafeDelete(m_lhs);
		D_SafeDelete(m_rhs);
	}
	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return m_lhs->ExternalCondition(input) || m_rhs->ExternalCondition(input);
	}
private:
	BevNodePrecondition* m_lhs;
	BevNodePrecondition* m_rhs;
};

class DLL_EXPORT BevNodePreconditionXOR : public BevNodePrecondition
{
public:
	BevNodePreconditionXOR(BevNodePrecondition* lhs, BevNodePrecondition* rhs)
		: m_lhs(lhs)
		, m_rhs(rhs)
	{
		//D_CHECK(m_lhs && m_rhs);
	}
	~BevNodePreconditionXOR(){
		D_SafeDelete(m_lhs);
		D_SafeDelete(m_rhs);
	}
	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return m_lhs->ExternalCondition(input) ^ m_rhs->ExternalCondition(input);
	}
private:
	BevNodePrecondition* m_lhs;
	BevNodePrecondition* m_rhs;
};


class DLL_EXPORT BevNodePreconditionBiggerThanInt : public BevNodePrecondition
{
public:
	BevNodePreconditionBiggerThanInt(int lhs,int rhs)
		:BevNodePrecondition(),
		m_lhs(lhs),
		m_rhs(rhs)
	{

	}

	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return m_lhs>m_rhs;
	}

private:
	int m_lhs,m_rhs;
}; 

class DLL_EXPORT BevNodePreconditionSmallerThanInt : public BevNodePrecondition
{
public:
	BevNodePreconditionSmallerThanInt(int lhs,int rhs)
		:BevNodePrecondition(),
		m_lhs(lhs),
		m_rhs(rhs)
	{

	}

	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return m_lhs<m_rhs;
	}

private:
	int m_lhs,m_rhs;
}; 

class DLL_EXPORT BevNodePreconditionEuqalInt : public BevNodePrecondition
{
public:
	BevNodePreconditionEuqalInt(int lhs,int rhs)
		:BevNodePrecondition(),
		m_lhs(lhs),
		m_rhs(rhs)
	{

	}

	virtual bool ExternalCondition(const BevNodeInputParam& input) const{
		return m_lhs==m_rhs;
	}

private:
	int m_lhs,m_rhs;
}; 

//-------------------------------------------------------------------------------------------------------------------------------------
class DLL_EXPORT BevNode
{
public:
	BevNode(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodeScript = NULL)
		: mul_ChildNodeCount(0)
		, mo_ActiveNode(NULL)
		, mo_LastActiveNode(NULL)
		, mo_NodePrecondition(NULL)
	{
		for(int i = 0; i < k_BLimited_MaxChildNodeCnt; ++i)
			mao_ChildNodeList[i] = NULL;

		_SetParentNode(_o_ParentNode);
		SetNodePrecondition(_o_NodeScript);
		mz_DebugName = new std::string("UNKNOWN");
	}
	~BevNode();
	bool Evaluate(const BevNodeInputParam& input);
	void Transition(const BevNodeInputParam& input);
	BevRunningStatus Tick(const BevNodeInputParam& input, BevNodeOutputParam& output);
	//---------------------------------------------------------------
	BevNode& AddChildNode(BevNode* _o_ChildNode);
	BevNode& SetNodePrecondition(BevNodePrecondition* _o_NodePrecondition);
	BevNode& SetDebugName(const char* _debugName);
	const BevNode* oGetLastActiveNode() const;
	void SetActiveNode(BevNode* _o_Node);
	const char* GetDebugName() const;
protected:
	//--------------------------------------------------------------
	// virtual function
	//--------------------------------------------------------------
	virtual bool _DoEvaluate(const BevNodeInputParam& input);
	virtual void _DoTransition(const BevNodeInputParam& input);
	virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);
	void _SetParentNode(BevNode* _o_ParentNode);
	bool _bCheckIndex(unsigned int _ui_Index) const;
protected:
	BevNode*                mao_ChildNodeList[k_BLimited_MaxChildNodeCnt];
	unsigned int						mul_ChildNodeCount;
	BevNode*                mo_ParentNode;
	BevNode*                mo_ActiveNode;
	BevNode*				mo_LastActiveNode;
	BevNodePrecondition*    mo_NodePrecondition;
	std::string*		    mz_DebugName;
};

class DLL_EXPORT BevNodePrioritySelector : public BevNode
{
public:
	BevNodePrioritySelector(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
		: BevNode(_o_ParentNode, _o_NodePrecondition)
		, mui_LastSelectIndex(k_BLimited_InvalidChildNodeIndex)
		, mui_CurrentSelectIndex(k_BLimited_InvalidChildNodeIndex)
	{}
	virtual bool _DoEvaluate(const BevNodeInputParam& input);
	virtual void _DoTransition(const BevNodeInputParam& input);
	virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

protected:
	unsigned int mui_CurrentSelectIndex;
	unsigned int mui_LastSelectIndex;
};

class DLL_EXPORT BevNodeNonePrioritySelector : public BevNodePrioritySelector
{
public:
	BevNodeNonePrioritySelector(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
		: BevNodePrioritySelector(_o_ParentNode, _o_NodePrecondition)
	{}
	virtual bool _DoEvaluate(const BevNodeInputParam& input);
};

class DLL_EXPORT BevWeightedSelector : public BevNodePrioritySelector
{
public:
	BevWeightedSelector(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
		: BevNodePrioritySelector(_o_ParentNode, _o_NodePrecondition)
	{}
	virtual bool _DoEvaluate(const BevNodeInputParam& input);

	void SetChildNodesWeight(std::vector<double>& weights);

protected:
	//存储各节点的权重值，可为外部修改用于RL
	std::vector<double> mao_ChildNodesWeight;
};

class DLL_EXPORT BevNodeSequence : public BevNode
{
public:
	BevNodeSequence(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
		: BevNode(_o_ParentNode, _o_NodePrecondition)
		, mui_CurrentNodeIndex(k_BLimited_InvalidChildNodeIndex)
	{}
	virtual bool _DoEvaluate(const BevNodeInputParam& input);
	virtual void _DoTransition(const BevNodeInputParam& input);
	virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

private:
	unsigned int mui_CurrentNodeIndex;
};

class DLL_EXPORT BevNodeTerminal : public BevNode
{
public:
	BevNodeTerminal(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
		: BevNode(_o_ParentNode, _o_NodePrecondition)
		, me_Status(k_TNS_Ready)
		, mb_NeedExit(false)
	{}
	virtual void _DoTransition(const BevNodeInputParam& input);
	virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

protected:
	virtual void _DoEnter(const BevNodeInputParam& input);
	virtual BevRunningStatus _DoExecute(const BevNodeInputParam& input, BevNodeOutputParam& output);
	virtual void _DoExit(const BevNodeInputParam& input, BevRunningStatus _ui_ExitID);

private:
	E_TerminalNodeStaus me_Status;
	bool                mb_NeedExit;
};

class DLL_EXPORT BevNodeParallel : public BevNode
{
public:
	BevNodeParallel(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL)
		: BevNode(_o_ParentNode, _o_NodePrecondition)
		, me_FinishCondition(k_PFC_OR)
	{
		for(unsigned int i = 0; i < k_BLimited_MaxChildNodeCnt; ++i)
			mab_ChildNodeStatus[i] = k_BRS_Executing;
	}
	virtual bool _DoEvaluate(const BevNodeInputParam& input);
	virtual void _DoTransition(const BevNodeInputParam& input);
	virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

	BevNodeParallel& SetFinishCondition(E_ParallelFinishCondition _e_Condition);

private:
	E_ParallelFinishCondition me_FinishCondition;
	BevRunningStatus		  mab_ChildNodeStatus[k_BLimited_MaxChildNodeCnt];
};

class DLL_EXPORT BevNodeLoop : public BevNode
{
public:
	static const int kInfiniteLoop = -1;

public:
	BevNodeLoop(BevNode* _o_ParentNode, BevNodePrecondition* _o_NodePrecondition = NULL, int _i_LoopCnt = kInfiniteLoop)
		: BevNode(_o_ParentNode, _o_NodePrecondition)
		, mi_LoopCount(_i_LoopCnt)
		, mi_CurrentCount(0)
	{}
	virtual bool _DoEvaluate(const BevNodeInputParam& input);
	virtual void _DoTransition(const BevNodeInputParam& input);
	virtual BevRunningStatus _DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output);

private:
	int mi_LoopCount;
	int mi_CurrentCount;
};