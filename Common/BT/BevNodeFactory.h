#pragma once
#include "BevTreeBasic.h"

class BevNodeFactory
{
public:
	//-------------------------------------------------------------------------------------
	// BevNodeFactory
	//-------------------------------------------------------------------------------------

	static BevNode& oCreateParallelNode(BevNode* _o_Parent, E_ParallelFinishCondition _e_Condition, const char* _debugName)
	{
		BevNodeParallel* pReturn = new BevNodeParallel(_o_Parent);
		pReturn->SetFinishCondition(_e_Condition);
		oCreateNodeCommon(pReturn, _o_Parent, _debugName);
		return (*pReturn);
	}
	static BevNode& oCreatePrioritySelectorNode(BevNode* _o_Parent, const char* _debugName)
	{
		BevNodePrioritySelector* pReturn = new BevNodePrioritySelector(_o_Parent);
		oCreateNodeCommon(pReturn, _o_Parent, _debugName);
		return (*pReturn);
	}
	static BevNode& oCreateNonePrioritySelectorNode(BevNode* _o_Parent, const char* _debugName)
	{
		BevNodeNonePrioritySelector* pReturn = new BevNodeNonePrioritySelector(_o_Parent);
		oCreateNodeCommon(pReturn, _o_Parent, _debugName);
		return (*pReturn);
	}
	static BevNode& oCreateWeightedSelectorNode(BevNode* _o_Parent, const char* _debugName)
	{
		BevWeightedSelector* pReturn = new BevWeightedSelector(_o_Parent);
		oCreateNodeCommon(pReturn, _o_Parent, _debugName);
		return (*pReturn);
	}
	static BevNode& oCreateSequenceNode(BevNode* _o_Parent, const char* _debugName)
	{
		BevNodeSequence* pReturn = new BevNodeSequence(_o_Parent);
		oCreateNodeCommon(pReturn, _o_Parent, _debugName);
		return (*pReturn);
	}
	static BevNode& oCreateLoopNode(BevNode* _o_Parent, const char* _debugName, int _i_LoopCount)
	{
		BevNodeLoop* pReturn = new BevNodeLoop(_o_Parent, NULL, _i_LoopCount);
		oCreateNodeCommon(pReturn, _o_Parent, _debugName);
		return (*pReturn);
	}

	template<typename T> static BevNode& oCreateTeminalNode(BevNode* _o_Parent, const char* _debugName)
	{
		BevNodeTerminal* pReturn = new T(_o_Parent);
		oCreateNodeCommon(pReturn, _o_Parent, _debugName);
		return (*pReturn);
	}
private:
	static void oCreateNodeCommon(BevNode* _o_Me, BevNode* _o_Parent, const char* _debugName)
	{
		if(_o_Parent)
			_o_Parent->AddChildNode(_o_Me);
		_o_Me->SetDebugName(_debugName);
	}

};