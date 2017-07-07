#include <windows.h>
#include "BevTreeBasic.h"

#include <algorithm>

using namespace std;
//-------------------------------------------------------------------------------------
// BevNode
//-------------------------------------------------------------------------------------

BevNode::~BevNode()
{
	D_SafeDelete(mz_DebugName);
	for(unsigned int i = 0; i < mul_ChildNodeCount; ++i)
	{
		D_SafeDelete(mao_ChildNodeList[i]);
	}
	if(mo_NodePrecondition==NULL) return;
	if(typeid(*mo_NodePrecondition)==typeid(BevNodePreconditionNOT))
	{
		BevNodePreconditionNOT* pNot = (BevNodePreconditionNOT*)mo_NodePrecondition;
		D_SafeDelete(pNot);
		return;
	}
	if(typeid(*mo_NodePrecondition)==typeid(BevNodePreconditionAND))
	{
		BevNodePreconditionAND* pAnd = (BevNodePreconditionAND*)mo_NodePrecondition;
		D_SafeDelete(pAnd);
		return;
	}
	if(typeid(*mo_NodePrecondition)==typeid(BevNodePreconditionOR))
	{
		BevNodePreconditionOR* pOr = (BevNodePreconditionOR*)mo_NodePrecondition;
		D_SafeDelete(pOr);
		return;
	}
	if(typeid(*mo_NodePrecondition)==typeid(BevNodePreconditionXOR))
	{
		BevNodePreconditionXOR* pXor = (BevNodePreconditionXOR*)mo_NodePrecondition;
		D_SafeDelete(pXor);
		return;
	}
	D_SafeDelete(mo_NodePrecondition);
}

BevNode& BevNode::AddChildNode(BevNode* _o_ChildNode)
{
	if(mul_ChildNodeCount == k_BLimited_MaxChildNodeCnt)
	{
		//D_Output("The number of child nodes is up to 16");
		//D_CHECK(0);
		return (*this);
	}
	mao_ChildNodeList[mul_ChildNodeCount] = _o_ChildNode;
	++mul_ChildNodeCount;
	return (*this);
}
BevNode& BevNode::SetNodePrecondition(BevNodePrecondition* _o_NodePrecondition)
{
	if(mo_NodePrecondition != _o_NodePrecondition)
	{
		if(mo_NodePrecondition)
			delete mo_NodePrecondition;

		mo_NodePrecondition = _o_NodePrecondition;
	}
	return (*this);
}
BevNode& BevNode::SetDebugName(const char* _debugName)
{
	*mz_DebugName = _debugName;
	return (*this);
}
const BevNode* BevNode::oGetLastActiveNode() const
{
	return mo_LastActiveNode;
}

void BevNode::SetActiveNode(BevNode* _o_Node)
{
	mo_LastActiveNode = mo_ActiveNode;
	mo_ActiveNode = _o_Node;
	if(mo_ParentNode != NULL)
		mo_ParentNode->SetActiveNode(_o_Node);
}
const char* BevNode::GetDebugName() const
{
	return mz_DebugName->c_str();
}

bool BevNode::_DoEvaluate(const BevNodeInputParam& input)
{
	return true;
}
void BevNode::_DoTransition(const BevNodeInputParam& input)
{
}
BevRunningStatus BevNode::_DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output)
{
	return k_BRS_Finish;
}

bool BevNode::Evaluate(const BevNodeInputParam& input)
{
	return (mo_NodePrecondition == NULL || mo_NodePrecondition->ExternalCondition(input)) && _DoEvaluate(input);
}
void BevNode::Transition(const BevNodeInputParam& input)
{
	_DoTransition(input);
}
BevRunningStatus BevNode::Tick(const BevNodeInputParam& input, BevNodeOutputParam& output)
{
	return _DoTick(input, output);
}

void BevNode::_SetParentNode(BevNode* _o_ParentNode)
{
	mo_ParentNode = _o_ParentNode;
}
bool BevNode::_bCheckIndex(unsigned int _ui_Index) const
{
	return _ui_Index >= 0 && _ui_Index < mul_ChildNodeCount;
}

//-------------------------------------------------------------------------------------
// BevNodePrioritySelector
//-------------------------------------------------------------------------------------
bool BevNodePrioritySelector::_DoEvaluate(const BevNodeInputParam& input)
{
	mui_CurrentSelectIndex = k_BLimited_InvalidChildNodeIndex;
	for(unsigned int i = 0; i < mul_ChildNodeCount; ++i)
	{
		BevNode* oBN = mao_ChildNodeList[i];
		if(oBN->Evaluate(input))
		{
			mui_CurrentSelectIndex = i;
			return true;
		}
	}
	return false;
}
void BevNodePrioritySelector::_DoTransition(const BevNodeInputParam& input)
{
	if(_bCheckIndex(mui_LastSelectIndex))
	{
		BevNode* oBN = mao_ChildNodeList[mui_LastSelectIndex];
		oBN->Transition(input);
	}
	mui_LastSelectIndex = k_BLimited_InvalidChildNodeIndex;
}
BevRunningStatus BevNodePrioritySelector::_DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output)
{
	BevRunningStatus bIsFinish = k_BRS_Finish;
	if(_bCheckIndex(mui_CurrentSelectIndex))
	{
		if(mui_LastSelectIndex != mui_CurrentSelectIndex)  //new select result
		{
			if(_bCheckIndex(mui_LastSelectIndex))
			{
				BevNode* oBN = mao_ChildNodeList[mui_LastSelectIndex];
				oBN->Transition(input);   //we need transition
			}
			mui_LastSelectIndex = mui_CurrentSelectIndex;
		}
	}
	if(_bCheckIndex(mui_LastSelectIndex))
	{
		//Running node
		BevNode* oBN = mao_ChildNodeList[mui_LastSelectIndex];
		bIsFinish = oBN->Tick(input, output);
		//clear variable if finish
		if(bIsFinish)
			mui_LastSelectIndex = k_BLimited_InvalidChildNodeIndex;
	}
	return bIsFinish;
}

//-------------------------------------------------------------------------------------
// BevNodeWeightSelector
//-------------------------------------------------------------------------------------
bool BevWeightedSelector::_DoEvaluate(const BevNodeInputParam& input)
{
	int MinWeight = -10000000;
	std::vector<double>::iterator maxIter;

	while (1)
	{
		maxIter = _Max_element(mao_ChildNodesWeight.begin(),mao_ChildNodesWeight.end());

		if (MinWeight == *maxIter) return false;

		int i = maxIter - mao_ChildNodesWeight.begin();

		BevNode* oBN = mao_ChildNodeList[i];
		if(oBN->Evaluate(input))
		{
			mui_CurrentSelectIndex = i;
			return true;
		}
		else
		{
			*maxIter = MinWeight;
		}
	}
}

void BevWeightedSelector::SetChildNodesWeight( std::vector<double>& weights )
{
	mao_ChildNodesWeight.clear();

	std::vector<double>::iterator myiter = weights.begin();
	for (;myiter!=weights.end();myiter++)
	{
		mao_ChildNodesWeight.push_back(*myiter);
	}
}

//-------------------------------------------------------------------------------------
// BevNodeNonePrioritySelector
//-------------------------------------------------------------------------------------
bool BevNodeNonePrioritySelector::_DoEvaluate(const BevNodeInputParam& input)
{
	if(_bCheckIndex(mui_CurrentSelectIndex))
	{
		BevNode* oBN = mao_ChildNodeList[mui_CurrentSelectIndex];
		if(oBN->Evaluate(input))
		{
			return true;
		}
	}
	return BevNodePrioritySelector::_DoEvaluate(input);
}
//-------------------------------------------------------------------------------------
// BevNodeSequence
//-------------------------------------------------------------------------------------
bool BevNodeSequence::_DoEvaluate(const BevNodeInputParam& input)
{
	unsigned int testNode;
	if(mui_CurrentNodeIndex == k_BLimited_InvalidChildNodeIndex)
		testNode = 0;
	else
		testNode = mui_CurrentNodeIndex;

	if(_bCheckIndex(testNode))
	{
		BevNode* oBN = mao_ChildNodeList[testNode];
		if(oBN->Evaluate(input))
			return true;
	}
	return false;
}
void BevNodeSequence::_DoTransition(const BevNodeInputParam& input)
{
	if(_bCheckIndex(mui_CurrentNodeIndex))
	{
		BevNode* oBN = mao_ChildNodeList[mui_CurrentNodeIndex];
		oBN->Transition(input);
	}
	mui_CurrentNodeIndex = k_BLimited_InvalidChildNodeIndex;
}
BevRunningStatus BevNodeSequence::_DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output)
{
	BevRunningStatus bIsFinish = k_BRS_Finish;

	//First Time
	if(mui_CurrentNodeIndex == k_BLimited_InvalidChildNodeIndex)
		mui_CurrentNodeIndex = 0;

	BevNode* oBN = mao_ChildNodeList[mui_CurrentNodeIndex];
	bIsFinish = oBN->Tick(input, output);
	if(bIsFinish == k_BRS_Finish)
	{
		++mui_CurrentNodeIndex;
		//sequence is over
		if(mui_CurrentNodeIndex == mul_ChildNodeCount)
		{
			mui_CurrentNodeIndex = k_BLimited_InvalidChildNodeIndex;
		}
		else
		{
			bIsFinish = k_BRS_Executing;
		}
	}
	if(bIsFinish < 0)
	{
		mui_CurrentNodeIndex = k_BLimited_InvalidChildNodeIndex;
	}
	return bIsFinish;
}

//-------------------------------------------------------------------------------------
// BevNodeTerminal
//-------------------------------------------------------------------------------------
void BevNodeTerminal::_DoTransition(const BevNodeInputParam& input)
{
	if(mb_NeedExit)     //call Exit if we have called Enter
		_DoExit(input, k_BRS_ERROR_Transition);

	SetActiveNode(NULL);
	me_Status = k_TNS_Ready;
	mb_NeedExit = false;
}

BevRunningStatus BevNodeTerminal::_DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output)
{
	BevRunningStatus bIsFinish = k_BRS_Finish;

	if(me_Status == k_TNS_Ready)
	{
		_DoEnter(input);
		mb_NeedExit = TRUE;
		me_Status = k_TNS_Running;
		SetActiveNode(this);
	}
	if(me_Status == k_TNS_Running)
	{
		bIsFinish = _DoExecute(input, output);
		SetActiveNode(this);
		if(bIsFinish == k_BRS_Finish || bIsFinish < 0)
			me_Status = k_TNS_Finish;
	}
	if(me_Status == k_TNS_Finish)
	{
		if(mb_NeedExit)     //call Exit if we have called Enter
			_DoExit(input, bIsFinish);

		me_Status = k_TNS_Ready;
		mb_NeedExit = FALSE;
		SetActiveNode(NULL);

		return bIsFinish;
	}
	return bIsFinish;
}

void BevNodeTerminal::_DoEnter(const BevNodeInputParam& input)
{

}
BevRunningStatus BevNodeTerminal::_DoExecute(const BevNodeInputParam& input, BevNodeOutputParam& output)
{
	return k_BRS_Finish;
}
void BevNodeTerminal::_DoExit(const BevNodeInputParam& input, BevRunningStatus _ui_ExitID)	
{

}

//-------------------------------------------------------------------------------------
// BevNodeParallel
//-------------------------------------------------------------------------------------
bool BevNodeParallel::_DoEvaluate(const BevNodeInputParam& input)
{
	for(unsigned int i = 0; i < mul_ChildNodeCount; ++i)
	{
		BevNode* oBN = mao_ChildNodeList[i];
		if(mab_ChildNodeStatus[i] == 0)
		{
			if(!oBN->Evaluate(input))
			{
				return false;
			}
		}
	}
	return true;
}
void BevNodeParallel::_DoTransition(const BevNodeInputParam& input)
{
	for(unsigned int i = 0; i < k_BLimited_MaxChildNodeCnt; ++i)
		mab_ChildNodeStatus[i] = k_BRS_Executing;

	for(unsigned int i = 0; i < mul_ChildNodeCount; ++i)
	{
		BevNode* oBN = mao_ChildNodeList[i];
		oBN->Transition(input);
	}
}
BevNodeParallel& BevNodeParallel::SetFinishCondition(E_ParallelFinishCondition _e_Condition)
{
	me_FinishCondition = _e_Condition;
	return (*this);
}
BevRunningStatus BevNodeParallel::_DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output)
{   
	unsigned int finishedChildCount = 0;  
	for(unsigned int i = 0; i < mul_ChildNodeCount; ++i)
	{
		BevNode* oBN = mao_ChildNodeList[i];
		if(me_FinishCondition == k_PFC_OR)
		{
			if(mab_ChildNodeStatus[i] == k_BRS_Executing)
			{
				mab_ChildNodeStatus[i] = oBN->Tick(input, output);
			}
			if(mab_ChildNodeStatus[i] != k_BRS_Executing)
			{
				for(unsigned int i = 0; i < k_BLimited_MaxChildNodeCnt; ++i)
					mab_ChildNodeStatus[i] = k_BRS_Executing;
				return k_BRS_Finish;
			}
		}
		else if(me_FinishCondition == k_PFC_AND)
		{
			if(mab_ChildNodeStatus[i] == k_BRS_Executing)
			{
				mab_ChildNodeStatus[i] = oBN->Tick(input, output);
			}
			if(mab_ChildNodeStatus[i] != k_BRS_Executing)
			{
				finishedChildCount++;
			}
		}
		else
		{
			//D_CHECK(0);
		}
	}
	if(finishedChildCount == mul_ChildNodeCount)
	{
		for(unsigned int i = 0; i < k_BLimited_MaxChildNodeCnt; ++i)
			mab_ChildNodeStatus[i] = k_BRS_Executing;
		return k_BRS_Finish;
	}
	return k_BRS_Executing;
}
//-------------------------------------------------------------------------------------
// BevNodeLoop
//-------------------------------------------------------------------------------------
bool BevNodeLoop::_DoEvaluate(const BevNodeInputParam& input)
{
	bool checkLoopCount = (mi_LoopCount == kInfiniteLoop) ||
		mi_CurrentCount < mi_LoopCount;

	if(!checkLoopCount)
		return false;

	if(_bCheckIndex(0))
	{
		BevNode* oBN = mao_ChildNodeList[0];
		if(oBN->Evaluate(input))
			return true;
	}
	return false;				 
}
void BevNodeLoop::_DoTransition(const BevNodeInputParam& input)
{
	if(_bCheckIndex(0))
	{
		BevNode* oBN = mao_ChildNodeList[0];
		oBN->Transition(input);
	}
	mi_CurrentCount = 0;
}
BevRunningStatus BevNodeLoop::_DoTick(const BevNodeInputParam& input, BevNodeOutputParam& output)
{
	BevRunningStatus bIsFinish = k_BRS_Finish;
	if(_bCheckIndex(0))
	{
		BevNode* oBN = mao_ChildNodeList[0];
		bIsFinish = oBN->Tick(input, output);

		if(bIsFinish == k_BRS_Finish)
		{
			if(mi_LoopCount != kInfiniteLoop)
			{
				mi_CurrentCount++;
				if(mi_CurrentCount < mi_LoopCount)
				{
					bIsFinish = k_BRS_Executing;
				}
			}
			else
			{
				bIsFinish = k_BRS_Executing;
			}
		}
	}
	if(bIsFinish)
	{
		mi_CurrentCount = 0;
	}
	return bIsFinish;
}	