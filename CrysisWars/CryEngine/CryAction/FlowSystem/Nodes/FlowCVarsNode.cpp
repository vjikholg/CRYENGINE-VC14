#include "StdAfx.h"
#include "FlowBaseNode.h"

class CFlowNode_CVar : public CFlowBaseNode
{
public:
	CFlowNode_CVar( SActivationInfo * pActInfo ) {}

	enum EInPorts
	{
		SET = 0,
		GET,
		NAME,
		VALUE,
	};
	enum EOutPorts
	{
		CURVALUE = 0,
		CURVALUE_INT,
		CURVALUE_FLOAT,
	};

	virtual void GetConfiguration( SFlowNodeConfig& config )
	{
		static const SInputPortConfig in_config[] = {
			InputPortConfig<SFlowSystemVoid>( "Set", _HELP("Trigger to Set CVar's value") ),
			InputPortConfig<SFlowSystemVoid>( "Get", _HELP("Trigger to Get CVar's value") ),
			InputPortConfig<string>( "CVar", _HELP("Name of the CVar to set/get [case-INsensitive]") ),
			InputPortConfig<string>( "Value", _HELP("Value of the CVar to set") ),
			{0}
		};
		static const SOutputPortConfig out_config[] = {
			OutputPortConfig<string>( "CurValue", _HELP("Current Value of the CVar [Triggered on Set/Get]") ),
			OutputPortConfig<int>( "CurValue Int", _HELP("Current Value of the CVar, if integer [Triggered on Set/Get]") ),
			OutputPortConfig<float>( "CurValue Float", _HELP("Current Value of the CVar, if float [Triggered on Set/Get]") ),
			{0}
		};
		config.sDescription = _HELP("Sets/Gets the value of a console variable (CVar).");
		config.pInputPorts = in_config;
		config.pOutputPorts = out_config;
		config.SetCategory(EFLN_DEBUG);
	}

	virtual void ProcessEvent( EFlowEvent event, SActivationInfo *pActInfo )
	{
		if (event == eFE_Activate)
		{
			const bool isSet = (IsPortActive(pActInfo, SET));
			const bool isGet = (IsPortActive(pActInfo, GET));
			if (!isGet && !isSet)
				return;

			const string& cvar = GetPortString(pActInfo, NAME);
			ICVar* pICVar = gEnv->pConsole->GetCVar(cvar.c_str());
			if (pICVar != 0)
			{
				if (isSet)
				{
					const string& val = GetPortString(pActInfo, VALUE);
					pICVar->Set(val.c_str());
				}
				const string curVal = pICVar->GetString();
				ActivateOutput(pActInfo, CURVALUE, curVal);

				if(pICVar->GetType() == CVAR_INT)
					ActivateOutput(pActInfo, CURVALUE_INT, pICVar->GetIVal());

				if(pICVar->GetType() == CVAR_FLOAT)
					ActivateOutput(pActInfo, CURVALUE_FLOAT, pICVar->GetFVal());
			}
			else
			{
				CryLogAlways("[flow] Cannot find console variable '%s'", cvar.c_str());
			}
		}
	}

	virtual void GetMemoryStatistics(ICrySizer * s)
	{
		s->Add(*this);
	}
};

REGISTER_FLOW_NODE_SINGLETON( "System:ConsoleVariable", CFlowNode_CVar );

