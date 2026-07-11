/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
-------------------------------------------------------------------------
History:
- Dec 18,2005:	Created by Marco Koegler

*************************************************************************/
#include "StdAfx.h"
#ifdef USE_PS3INPUT
#include "PS3Pad.h"
#include "PS3Input.h"

const char* deviceNames[] =
{
	"pad0",
	"pad1",
	"pad2",
	"pad3"
};

// libusbpad supported pad mapping
static const int sPadMappings[40] =
{
	6, 7, 4, 5, 3, 7, 3, 6, 3, 5, 3, 4, 3, 2, 3, 3, 3, 0, 3, 1, 2, 0, 2, 3, 2, 1, 2, 2, 2, 4, 2, 5, 2, 6, 2, 7, 20, 21, 22, 23,
};

// define the device id's (they are the offsets into the button array
#define PS3_BUTTON_SELECT 	8
#define PS3_BUTTON_L3				10
#define PS3_BUTTON_R3				11
#define PS3_BUTTON_START		9
#define PS3_BUTTON_UP				12
#define PS3_BUTTON_RIGHT		13
#define PS3_BUTTON_DOWN			14
#define PS3_BUTTON_LEFT			15
#define PS3_BUTTON_L2				6
#define PS3_BUTTON_R2				7
#define PS3_BUTTON_L1				4
#define PS3_BUTTON_R1				5
#define PS3_BUTTON_TRIANGLE	3
#define PS3_BUTTON_CIRCLE		2
#define PS3_BUTTON_CROSS		1
#define PS3_BUTTON_SQUARE		0
#define PS3_STICK_LEFT_X		16
#define PS3_STICK_RIGHT_X		17
#define PS3_STICK_LEFT_Y		18
#define PS3_STICK_RIGHT_Y		19
#define PS3_ROT_X						20
#define PS3_ROT_Y						21
#define PS3_ROT_Z						22
#define PS3_ROTX_KEY_L			23
#define PS3_ROTX_KEY_R			24
#define PS3_ROTZ_KEY_D			25
#define PS3_ROTZ_KEY_U			26


CPS3Pad::CPS3Pad(CPS3Input& input, int deviceNo) : CPS3InputDevice(input, deviceNames[deviceNo]),
	m_deviceNo(deviceNo), m_connected(false)
{
	m_deviceId = eDI_XI;
	memset(m_buttons, 0, sizeof(m_buttons));
	memset(m_buttonsLast, 0, sizeof(m_buttonsLast));
	m_xAxis[0] = m_xAxis[1] = m_xAxis[2] = m_yAxis[0] = m_yAxis[1] = m_yAxis[2] = m_zAxis = m_gyro  = 0;
}

#if defined(USE_LEAN_HACK)
	//when this threshold is crossed, a not in real existing key is toggled 
	//	this way jump/leaning can be implemented by the controller rotation feature
	#define LEAN_THRESHOLD 0.5f
	#define LEAN_THRESHOLD_RESET 0.3f
#endif

CPS3Pad::~CPS3Pad(){}

bool CPS3Pad::Init()
{
#if defined(USE_LEAN_HACK)
	m_rotXLActive = m_rotXRActive = m_rotZDActive = m_rotZUActive = false;
#endif

	MapSymbol(PS3_BUTTON_SELECT, eKI_PS3_Select, "pad_select");
	MapSymbol(PS3_BUTTON_L3, eKI_PS3_L3, "pad_l3");
	MapSymbol(PS3_BUTTON_R3, eKI_PS3_R3, "pad_r3");
	MapSymbol(PS3_BUTTON_START, eKI_PS3_Start, "pad_start");
	MapSymbol(PS3_BUTTON_UP, eKI_PS3_Up, "pad_up");
	MapSymbol(PS3_BUTTON_RIGHT, eKI_PS3_Right, "pad_right");
	MapSymbol(PS3_BUTTON_DOWN, eKI_PS3_Down, "pad_down");
	MapSymbol(PS3_BUTTON_LEFT, eKI_PS3_Left, "pad_left");
	MapSymbol(PS3_BUTTON_L2, eKI_PS3_L2, "pad_l2");
	MapSymbol(PS3_BUTTON_R2, eKI_PS3_R2, "pad_r2");
	MapSymbol(PS3_BUTTON_L1, eKI_PS3_L1, "pad_l1");
	MapSymbol(PS3_BUTTON_R1, eKI_PS3_R1, "pad_r1");
	MapSymbol(PS3_BUTTON_TRIANGLE, eKI_PS3_Triangle,"pad_triangle");
	MapSymbol(PS3_BUTTON_CIRCLE, eKI_PS3_Circle, "pad_circle");
	MapSymbol(PS3_BUTTON_CROSS, eKI_PS3_Cross, "pad_cross");
	MapSymbol(PS3_BUTTON_SQUARE, eKI_PS3_Square, "pad_square");
	MapSymbol(PS3_STICK_LEFT_X, eKI_PS3_StickLX, "pad_sticklx",	SInputSymbol::Axis);
	MapSymbol(PS3_STICK_LEFT_Y, eKI_PS3_StickLY, "pad_stickly",	SInputSymbol::Axis);
	MapSymbol(PS3_STICK_RIGHT_X, eKI_PS3_StickRX, "pad_stickrx",	SInputSymbol::Axis);
	MapSymbol(PS3_STICK_RIGHT_Y, eKI_PS3_StickRY, "pad_stickry",	SInputSymbol::Axis);
#if defined(USE_LEAN_HACK)
	MapSymbol(PS3_ROTX_KEY_L, eKI_PS3_ROTX_KeyL, "pad_rotx_keyl");
	MapSymbol(PS3_ROTX_KEY_R, eKI_PS3_ROTX_KeyR, "pad_rotx_keyr");
	MapSymbol(PS3_ROTZ_KEY_D, eKI_PS3_ROTZ_KeyD, "pad_rotz_keyd");
	MapSymbol(PS3_ROTZ_KEY_U, eKI_PS3_ROTZ_KeyU, "pad_rotz_keyu");
#endif
	MapSymbol(PS3_ROT_X, eKI_PS3_RotX, "pad_rotx",	SInputSymbol::Axis);
	MapSymbol(PS3_ROT_Y, eKI_PS3_RotY, "pad_roty",	SInputSymbol::Axis);
	MapSymbol(PS3_ROT_Z, eKI_PS3_RotZ, "pad_rotz",	SInputSymbol::Axis);

	return true;
}

void CPS3Pad::MapDigDirControl(const CellPadData& crData)
{
	// map pads which use discrete values for digital direction control rather than a bit per button
	switch(crData.button[2] & 0xF)
	{
		// no direction
	case 0x0:
	default:
		m_buttons[PS3_BUTTON_UP]		= 0;
		m_buttons[PS3_BUTTON_RIGHT] = 0;
		m_buttons[PS3_BUTTON_DOWN]	= 0;
		m_buttons[PS3_BUTTON_LEFT]	= 0;
		break;
		// up
	case 0x1:
		m_buttons[PS3_BUTTON_UP]		= 1;
		m_buttons[PS3_BUTTON_RIGHT] = 0;
		m_buttons[PS3_BUTTON_DOWN]	= 0;
		m_buttons[PS3_BUTTON_LEFT]	= 0;
		break;
		// right-up
	case 0x2:
		m_buttons[PS3_BUTTON_UP]		= 1;
		m_buttons[PS3_BUTTON_RIGHT] = 1;
		m_buttons[PS3_BUTTON_DOWN]	= 0;
		m_buttons[PS3_BUTTON_LEFT]	= 0;
		break;
		// right
	case 0x3:
		m_buttons[PS3_BUTTON_UP]		= 0;
		m_buttons[PS3_BUTTON_RIGHT] = 1;
		m_buttons[PS3_BUTTON_DOWN]	= 0;
		m_buttons[PS3_BUTTON_LEFT]	= 0;
		break;
		// right-down
	case 0x4:
		m_buttons[PS3_BUTTON_UP]		= 0;
		m_buttons[PS3_BUTTON_RIGHT] = 1;
		m_buttons[PS3_BUTTON_DOWN]	= 1;
		m_buttons[PS3_BUTTON_LEFT]	= 0;
		break;
		// down
	case 0x5:
		m_buttons[PS3_BUTTON_UP]		= 0;
		m_buttons[PS3_BUTTON_RIGHT] = 0;
		m_buttons[PS3_BUTTON_DOWN]	= 1;
		m_buttons[PS3_BUTTON_LEFT]	= 0;
		break;
		// left-down
	case 0x6:
		m_buttons[PS3_BUTTON_UP]		= 0;
		m_buttons[PS3_BUTTON_RIGHT] = 0;
		m_buttons[PS3_BUTTON_DOWN]	= 1;
		m_buttons[PS3_BUTTON_LEFT]	= 1;
		break;
		// left
	case 0x7:
		m_buttons[PS3_BUTTON_UP]		= 0;
		m_buttons[PS3_BUTTON_RIGHT] = 0;
		m_buttons[PS3_BUTTON_DOWN]	= 0;
		m_buttons[PS3_BUTTON_LEFT]	= 1;
		break;
		// left-up
	case 0x8:
		m_buttons[PS3_BUTTON_UP]		= 1;
		m_buttons[PS3_BUTTON_RIGHT] = 0;
		m_buttons[PS3_BUTTON_DOWN]	= 0;
		m_buttons[PS3_BUTTON_LEFT]	= 1;
		break;
	}
}

namespace
{
	ILINE const float MapPadValue256(const int cInput)
	{
		return (float)cInput / 127.5f - 1.f;
	}

	ILINE const float MapPadValue1024(const int cInput)
	{
		return (float)(cInput - 512) / 127.5f;
	}
}

void CPS3Pad::ReadAxis(const CellPadData& crData)
{
	int	xAxis[3];	//x-axis values
	int	yAxis[3];	//y-axis values
	int zAxis;		//z-axis values
	int	gyro;			// Gyro values

	//read the analog axes
	xAxis[0] = crData.button[sPadMappings[0]]; //range: 256
	yAxis[0] = crData.button[sPadMappings[1]]; //range: 256
	xAxis[1] = crData.button[sPadMappings[2]]; //range: 256
	yAxis[1] = crData.button[sPadMappings[3]]; //range: 256
	xAxis[2] = crData.button[sPadMappings[36]];//range: 1024
	yAxis[2] = crData.button[sPadMappings[37]];//range: 1024
	zAxis		 = crData.button[sPadMappings[38]];//range: 1024
	gyro = crData.button[sPadMappings[39]];		 //range: 1024
	
	SInputEvent event;
	SInputSymbol*	pSymbol = 0;

	for(int i=0; i<2; ++i)
	{
		//handle PS3_STICK_LEFT_X and (PS3_STICK_LEFT_X+1 => PS3_STICK_RIGHT_X)
		if(xAxis[i] != m_xAxis[i])
		{
			const float cVal = MapPadValue256(xAxis[i]);
			pSymbol = DevSpecIdToSymbol(PS3_STICK_LEFT_X+i);			assert(pSymbol);
			pSymbol->ChangeEvent(cVal);
			pSymbol->AssignTo(event);
			GetPS3Input().PostInputEvent(event);
		}
	}
	if(yAxis[0] != m_yAxis[0])
	{
		const float cVal = MapPadValue256(yAxis[0]);
		pSymbol = DevSpecIdToSymbol(PS3_STICK_LEFT_Y);			assert(pSymbol);
		pSymbol->ChangeEvent(-cVal);
		pSymbol->AssignTo(event);
		GetPS3Input().PostInputEvent(event);
	}
	if(yAxis[1] != m_yAxis[1])
	{
		const float cVal = MapPadValue256(yAxis[1]);
		pSymbol = DevSpecIdToSymbol(PS3_STICK_RIGHT_Y);			assert(pSymbol);
		pSymbol->ChangeEvent(-cVal);
		pSymbol->AssignTo(event);
		GetPS3Input().PostInputEvent(event);
	}
#if defined(USE_LEAN_HACK)
	if(xAxis[2] != m_xAxis[2])
	{
		const float cVal = MapPadValue1024(xAxis[2]);
		if(cVal > LEAN_THRESHOLD)
		{
			if(!m_rotXRActive)
			{
				m_rotXRActive = true;
				pSymbol = DevSpecIdToSymbol(PS3_ROTX_KEY_R);			assert(pSymbol);
				pSymbol->PressEvent(true);
				pSymbol->AssignTo(event);
				GetPS3Input().PostInputEvent(event);
			}
		}
		else
		if(m_rotXRActive && cVal < LEAN_THRESHOLD_RESET)
		{
			m_rotXRActive = false;
			pSymbol = DevSpecIdToSymbol(PS3_ROTX_KEY_R);			assert(pSymbol);
			pSymbol->PressEvent(false);
			pSymbol->AssignTo(event);
			GetPS3Input().PostInputEvent(event);
		}
		if(cVal < -LEAN_THRESHOLD)
		{
			if(!m_rotXLActive)
			{
				m_rotXLActive = true;
				pSymbol = DevSpecIdToSymbol(PS3_ROTX_KEY_L);			assert(pSymbol);
				pSymbol->PressEvent(true);
				pSymbol->AssignTo(event);
				GetPS3Input().PostInputEvent(event);
			}
		}
		else
		if(m_rotXLActive && cVal > -LEAN_THRESHOLD_RESET)
		{
			m_rotXLActive = false;
			pSymbol = DevSpecIdToSymbol(PS3_ROTX_KEY_L);			assert(pSymbol);
			pSymbol->PressEvent(false);
			pSymbol->AssignTo(event);
			GetPS3Input().PostInputEvent(event);
		}
	}

	if(zAxis != m_zAxis)
	{
		const float cVal = MapPadValue1024(zAxis);
		if(cVal > LEAN_THRESHOLD)
		{
			if(!m_rotZUActive)
			{
				m_rotZUActive = true;
				pSymbol = DevSpecIdToSymbol(PS3_ROTZ_KEY_U);			assert(pSymbol);
				pSymbol->PressEvent(true);
				pSymbol->AssignTo(event);
				GetPS3Input().PostInputEvent(event);
			}
		}
		else
		if(m_rotZUActive && cVal < LEAN_THRESHOLD_RESET)
		{
			m_rotZUActive = false;
			pSymbol = DevSpecIdToSymbol(PS3_ROTZ_KEY_U);			assert(pSymbol);
			pSymbol->PressEvent(false);
			pSymbol->AssignTo(event);
			GetPS3Input().PostInputEvent(event);
		}
		if(cVal < -LEAN_THRESHOLD)
		{
			if(!m_rotZDActive)
			{
				m_rotZDActive = true;
				pSymbol = DevSpecIdToSymbol(PS3_ROTZ_KEY_D);			assert(pSymbol);
				pSymbol->PressEvent(true);
				pSymbol->AssignTo(event);
				GetPS3Input().PostInputEvent(event);
			}
		}
		else
		if(m_rotZDActive && cVal > -LEAN_THRESHOLD_RESET)
		{
			m_rotZDActive = false;
			pSymbol = DevSpecIdToSymbol(PS3_ROTZ_KEY_D);			assert(pSymbol);
			pSymbol->PressEvent(false);
			pSymbol->AssignTo(event);
			GetPS3Input().PostInputEvent(event);
		}
	}
#endif
	//save values
	m_gyro	= gyro;
	m_xAxis[0] = xAxis[0];	m_xAxis[1] = xAxis[1];	m_xAxis[2] = xAxis[2];
	m_yAxis[0] = yAxis[0];	m_yAxis[1] = yAxis[1];	m_yAxis[2] = yAxis[2];
	m_zAxis		 = zAxis;
}

void CPS3Pad::Update()
{
	CellPadInfo info;	// connection status
	CellPadData state;	// gamepad data

	// get current status
	if (cellPadGetInfo (&info) == CELL_PAD_OK)
	{
		if (!UpdateConnectionState(info.status[m_deviceNo] == 1)) return;

		if (cellPadGetData(m_deviceNo, &state) == CELL_PAD_ERROR_UNSUPPORTED_GAMEPAD)
			if (cellPadGetRawData(m_deviceNo, &state) != CELL_PAD_OK) return;

		if (state.len == 0)
			return;

		SInputEvent event;
		SInputSymbol*	pSymbol = 0;

		for(int i = 0; i < 16; ++i)
		{
			if(sPadMappings[i * 2 + 4] >= 0)
				m_buttons[i] = (state.button[sPadMappings[i * 2 + 4]] & (1 << sPadMappings[i * 2 + 5])) ? 1 : 0;
			else
				m_buttons[i] = 0;
		}
//		MapDigDirControl(state);
		for(int i = 0; i < 16; ++i)
		{
			if(m_buttons[i] ^ m_buttonsLast[i])
			{
				if((pSymbol = DevSpecIdToSymbol(i)) != NULL)
				{
					pSymbol->PressEvent(m_buttons[i] != 0);
					pSymbol->AssignTo(event);
					GetPS3Input().PostInputEvent(event);
				}
			}
			m_buttonsLast[i] = m_buttons[i];
		}
		ReadAxis(state);
	}
}

bool CPS3Pad::UpdateConnectionState(bool current)
{
	if (current != m_connected)
	{
		SInputEvent event;
		if (current)
		{
			//connect
			event.keyName = "connect";
		}
		else
		{
			// disconnect
			event.keyName = "disconnect";
		}
		m_connected = current;
		if(m_connected)
		{
			if (cellPadInfoSensorMode(m_deviceNo) == CELL_PAD_INFO_SUPPORTED_SENSOR_MODE)
				cellPadSetSensorMode(m_deviceNo, CELL_PAD_SENSOR_MODE_ON);
		}
		GetPS3Input().PostInputEvent(event);
	}

	return m_connected;
}

#endif //USE_PS3INPUT
