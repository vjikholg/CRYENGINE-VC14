#include "StdAfx.h"
#include "FlowBaseNode.h"

class CFlowNode_ClientServer : public CFlowBaseNode
{
public:
	CFlowNode_ClientServer( SActivationInfo * pActInfo ) 
	{
	}

	virtual void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<bool>("isServer", _HELP("Is this instance a server?")),
			OutputPortConfig<bool>("isClient", _HELP("Is this instance a client?")),
			{0}
		};
		config.pInputPorts = 0;
		config.pOutputPorts = out_config;
		config.sDescription = _HELP("Provides output whether this instance is a client and/or a server.");
		config.SetCategory(EFLN_APPROVED); // POLICY CHANGE: Maybe an Enable/Disable Port
	}

	virtual void GetMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		switch (event)
		{
		case eFE_Initialize:
			pActInfo->pGraph->SetRegularlyUpdated( pActInfo->myID, true );
			break;
		case eFE_Update:
			{
				ActivateOutput(pActInfo, 0, gEnv->bServer);
				ActivateOutput(pActInfo, 1, gEnv->bClient);
			}
		}
	}
};

REGISTER_FLOW_NODE_SINGLETON( "Multiplayer:ClientServer", CFlowNode_ClientServer );