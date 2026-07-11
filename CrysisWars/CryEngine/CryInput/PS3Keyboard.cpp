/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2006.
-------------------------------------------------------------------------
$Id$
$DateTime$
-------------------------------------------------------------------------
History:
- Dec 16,2005:	Created by Marco Koegler

*************************************************************************/
#include "StdAfx.h"
#include "PS3Keyboard.h"

#ifdef USE_PS3INPUT
#include "PS3Input.h"
#include <cell/keyboard.h>    // USB Keyboard Library
//#include <stdio.h> 

//global var due to functionality of GetAsyncKeyState
static unsigned char g_keyState[256];			// current state of the keys on the keyboard
static unsigned char g_LastKeyState[256];	// state of the keys when GetAsyncKeyState was called last


CPS3Keyboard::CPS3Keyboard(CPS3Input& input) : CPS3InputDevice(input, "keyboard"),
m_connected(false)
{
	memset(g_keyState, 0, sizeof(g_keyState));
	memset(g_LastKeyState, 0, sizeof(g_LastKeyState));
}

CPS3Keyboard::~CPS3Keyboard()
{
	cellKbEnd();
}

bool CPS3Keyboard::Init()
{
	//pSystem->GetILog()->Log("Init Cell keyboard library\n");
	const int cCellInitRes = cellKbInit(2);
	if (cCellInitRes != CELL_KB_OK && cCellInitRes != CELL_KB_ERROR_ALREADY_INITIALIZED)
	{
		gEnv->pLog->Log("Error: cannot init Cell keyboard library: Unknown");
		return false;
	}

/*	// we don't want any key repeat, so disable it
	if (cellKbSetRepeat(0, 0, 0) != CELL_KB_OK)
	{
		gEnv->pLog->Log("Error: cannot set key repeat\n");
		return false;
	}

	// set a 'classic' 101 keyboard layout
	if (cellKbSetArrangement(0, CELL_KB_ARRANGEMENT_101) != CELL_KB_OK)
	{
		gEnv->pLog->Log("Error: cannot set key arrangement\n");
		return false;
	}
*/
	if (cellKbSetReadMode(0, CELL_KB_RMODE_PACKET) != CELL_KB_OK)
	{
		gEnv->pLog->Log("Error: cannot set read mode\n");
		return false;
	}
	if (cellKbSetCodeType(0, CELL_KB_CODETYPE_RAW) != CELL_KB_OK)
	{
		gEnv->pLog->Log("Error: cannot set code type\n");
		return false;
	}

	SetupKeyNames();

	return true;
}

void CPS3Keyboard::Update()
{
	CellKbInfo info;
	CellKbData data;   // keyboard data buffer

	if (cellKbGetInfo(&info) == CELL_KB_OK)
	{
		if (!UpdateConnectionState(info.status[0] == 1)) return;

		if (cellKbRead(0, &data) != CELL_KB_OK) return;

		m_modifiers = data.mkey;
		// we also pack the led state into the modifiers
		// make sure the amount we shift the leds matches the amount of
		// bits the numlock key is shifted in EModifierMask
		m_modifiers = m_modifiers | (data.led << 8);

		bool rollover = false;

		// we have some key data, so let's start reading it
		for (int i = 0; i < data.len; ++i)
		{
			uint16_t key = data.keycode[i];
			//printf("%04X Event Received\n", key);
			if (key == CELL_KEYC_E_ROLLOVER)
			{
				rollover = true;
				continue;				
			}

			if (key == CELL_KEYC_NO_EVENT)
			{
				continue;
			}

			unsigned char code = key & 0x00ff;
			if (g_keyState[code] == 0)
			{
				if (rollover)
				{
					printf("Warning ... logic will fail ;)\n");
				}
				g_keyState[code] = 2;
//				printf("%04X Pressed\n", key);
				ProcessKey(code, true);
			}
			else if (g_keyState[code] == 1)
			{
				//refresh
				g_keyState[code] = 2;
			}
		}

		if (!rollover && data.len>0)
		{
			for (int i = 0; i < 256; ++i)
			{
				if (g_keyState[i] == 2)
				{
					g_keyState[i] = 1;
				}
				else if (g_keyState[i] == 1)
				{
					g_keyState[i] = 0;
					ProcessKey(i, false);
				}
			}		
		}
	}
}

const char* CPS3Keyboard::GetKeyName(const SInputEvent& event, bool bGUI)
{
	if (bGUI)
	{
		static char szKeyName[2];
		szKeyName[0] = Event2ASCII(event);
		szKeyName[1] = 0;
		return szKeyName;
	}
	else
	{
		return event.keyName.c_str();
	}
}

unsigned char CPS3Keyboard::Event2ASCII(const SInputEvent& event)
{
	uint32 id = NameToId(event.keyName);

	// unpack modifiers and led
	unsigned int mkey = event.modifiers & eMM_Modifiers;
	unsigned int led = (event.modifiers & eMM_LockKeys) >> 8;
	unsigned short ascii = cellKbCnvRawCode(CELL_KB_ARRANGEMENT_101, mkey, led, id);
	if (ascii < 0xff)
	{
		return (ascii & 0xff);
	}
	return 0;
}

void CPS3Keyboard::ProcessKey(uint32 key, bool pressed)
{
	SInputSymbol* pSymbol = DevSpecIdToSymbol(key);

	if (!pSymbol) return;

	if (pressed)
	{
		g_LastKeyState[key] = 1;
		pSymbol->state = eIS_Pressed;
		pSymbol->value = 1.0f;
	}
	else
	{
		pSymbol->state = eIS_Released;
		pSymbol->value = 0.0f;
	}

	// Post input events
	SInputEvent event;
	event.timestamp = 0;
	event.modifiers = GetEventModifiers();

	event.state = pSymbol->state;
	event.value = pSymbol->value;
	event.keyName = pSymbol->name;
	event.keyId = pSymbol->keyId;
	event.pSymbol = pSymbol;
  event.deviceId = eDI_Keyboard;
	GetPS3Input().PostInputEvent(event);
}

int CPS3Keyboard::GetEventModifiers()
{
	int modifiers = 0;
	int leds = m_modifiers >> 8;

	if (m_modifiers & CELL_KB_MKEY_L_CTRL)
		modifiers |= eMM_LCtrl;
	if (m_modifiers & CELL_KB_MKEY_L_SHIFT)
		modifiers |= eMM_LShift;
	if (m_modifiers & CELL_KB_MKEY_L_ALT)
		modifiers |= eMM_LAlt;
	if (m_modifiers & CELL_KB_MKEY_L_WIN)
		modifiers |= eMM_LWin;
	if (m_modifiers & CELL_KB_MKEY_R_CTRL)
		modifiers |= eMM_RCtrl;
	if (m_modifiers & CELL_KB_MKEY_R_SHIFT)
		modifiers |= eMM_RShift;
	if (m_modifiers & CELL_KB_MKEY_R_ALT)
		modifiers |= eMM_RAlt;
	if (m_modifiers & CELL_KB_MKEY_R_WIN)
		modifiers |= eMM_RWin;
	if (leds & CELL_KB_LED_NUM_LOCK)
		modifiers |= eMM_NumLock;
	if (leds & CELL_KB_LED_CAPS_LOCK)
		modifiers |= eMM_CapsLock;
	if (leds & CELL_KB_LED_SCROLL_LOCK)
		modifiers |= eMM_ScrollLock;
	return modifiers;
}

bool CPS3Keyboard::UpdateConnectionState(bool current)
{
	if (current != m_connected)
	{
		SInputEvent event;
		if (current)
		{
			//connect
//			cellKbSetLEDStatus (0, CELL_KB_LED_MODE_AUTO1);
			event.keyName = "connect";
		}
		else
		{
			// disconnect
			//printf("Keyboard0 disconnected\n");
			event.keyName = "disconnect";
		}
		m_connected = current;
		GetPS3Input().PostInputEvent(event);
	}

	return m_connected;
}

void	CPS3Keyboard::SetupKeyNames()
{
	MapSymbol(CELL_KEYC_ESC, eKI_Escape, "escape");
	MapSymbol(CELL_KEYC_1, eKI_1, "1");
	MapSymbol(CELL_KEYC_2, eKI_2, "2");
	MapSymbol(CELL_KEYC_3, eKI_3, "3");
	MapSymbol(CELL_KEYC_4, eKI_4, "4");
	MapSymbol(CELL_KEYC_5, eKI_5, "5");
	MapSymbol(CELL_KEYC_6, eKI_6, "6");
	MapSymbol(CELL_KEYC_7, eKI_7, "7");
	MapSymbol(CELL_KEYC_8, eKI_8, "8");
	MapSymbol(CELL_KEYC_9, eKI_9, "9");
	MapSymbol(CELL_KEYC_0, eKI_0, "0");
	MapSymbol(CELL_KEYC_MINUS, eKI_Minus, "minus");
	MapSymbol(CELL_KEYC_EQUAL_101, eKI_Equals, "equals");
	MapSymbol(CELL_KEYC_BS, eKI_Backspace, "backspace");
	MapSymbol(CELL_KEYC_TAB, eKI_Tab, "tab");
	MapSymbol(CELL_KEYC_Q, eKI_Q, "q");
	MapSymbol(CELL_KEYC_W, eKI_W, "w");
	MapSymbol(CELL_KEYC_E, eKI_E, "e");
	MapSymbol(CELL_KEYC_R, eKI_R, "r");
	MapSymbol(CELL_KEYC_T, eKI_T, "t");
	MapSymbol(CELL_KEYC_Y, eKI_Y, "y");
	MapSymbol(CELL_KEYC_U, eKI_U, "u");
	MapSymbol(CELL_KEYC_I, eKI_I, "i");
	MapSymbol(CELL_KEYC_O, eKI_O, "o");
	MapSymbol(CELL_KEYC_P, eKI_P, "p");
	MapSymbol(CELL_KEYC_LEFT_BRACKET_101, eKI_LBracket, "lbracket");
	MapSymbol(CELL_KEYC_RIGHT_BRACKET_101, eKI_RBracket, "rbracket");
	MapSymbol(CELL_KEYC_ENTER, eKI_Enter, "enter");
	//MapSymbol(CELL_KEYC_LCONTROL, "lctrl", SInputSymbol::Button, eMM_LCtrl);
	MapSymbol(CELL_KEYC_A, eKI_A, "a");
	MapSymbol(CELL_KEYC_S, eKI_S, "s");
	MapSymbol(CELL_KEYC_D, eKI_D, "d");
	MapSymbol(CELL_KEYC_F, eKI_F, "f");
	MapSymbol(CELL_KEYC_G, eKI_G, "g");
	MapSymbol(CELL_KEYC_H, eKI_H, "h");
	MapSymbol(CELL_KEYC_J, eKI_J, "j");
	MapSymbol(CELL_KEYC_K, eKI_K, "k");
	MapSymbol(CELL_KEYC_L, eKI_L, "l");
	MapSymbol(CELL_KEYC_SEMICOLON, eKI_Semicolon, "semicolon");
	//MapSymbol(CELL_KEYC_APOSTROPHE, "apostrophe");
	MapSymbol(0x35, eKI_Tilde, "tilde");
	//MapSymbol(CELL_KEYC_LSHIFT, "lshift", SInputSymbol::Button, eMM_LShift);
	MapSymbol(CELL_KEYC_BACKSLASH_101, eKI_Backslash, "backslash");
	MapSymbol(CELL_KEYC_Z, eKI_Z, "z");
	MapSymbol(CELL_KEYC_X, eKI_X, "x");
	MapSymbol(CELL_KEYC_C, eKI_C, "c");
	MapSymbol(CELL_KEYC_V, eKI_V, "v");
	MapSymbol(CELL_KEYC_B, eKI_B, "b");
	MapSymbol(CELL_KEYC_N, eKI_N, "n");
	MapSymbol(CELL_KEYC_M, eKI_M, "m");
	MapSymbol(CELL_KEYC_COMMA, eKI_Comma, "comma");
	MapSymbol(CELL_KEYC_PERIOD, eKI_Period, "period");
	MapSymbol(CELL_KEYC_SLASH, eKI_Slash, "slash");
	//MapSymbol(CELL_KEYC_RSHIFT, "rshift", SInputSymbol::Button, eMM_RShift);
	MapSymbol(CELL_KEYC_KPAD_ASTERISK, eKI_NP_Multiply, "np_multiply");
	//MapSymbol(CELL_KEYC_LALT, "lalt", SInputSymbol::Button, eMM_LAlt);
	MapSymbol(CELL_KEYC_SPACE, eKI_NP_Multiply, "space");
	MapSymbol(CELL_KEYC_CAPS_LOCK, eKI_CapsLock, "capslock", SInputSymbol::Toggle, eMM_CapsLock);
	MapSymbol(CELL_KEYC_F1, eKI_F1, "f1");
	MapSymbol(CELL_KEYC_F2, eKI_F2, "f2");
	MapSymbol(CELL_KEYC_F3, eKI_F3, "f3");
	MapSymbol(CELL_KEYC_F4, eKI_F4, "f4");
	MapSymbol(CELL_KEYC_F5, eKI_F5, "f5");
	MapSymbol(CELL_KEYC_F6, eKI_F6, "f6");
	MapSymbol(CELL_KEYC_F7, eKI_F7, "f7");
	MapSymbol(CELL_KEYC_F8, eKI_F8, "f8");
	MapSymbol(CELL_KEYC_F9, eKI_F9, "f9");
	MapSymbol(CELL_KEYC_F10, eKI_F10, "f10");
	MapSymbol(CELL_KEYC_NUM_LOCK, eKI_NumLock, "numlock", SInputSymbol::Toggle, eMM_NumLock);
	MapSymbol(CELL_KEYC_SCROLL_LOCK, eKI_ScrollLock, "scrolllock", SInputSymbol::Toggle, eMM_ScrollLock);
	MapSymbol(CELL_KEYC_KPAD_7, eKI_NP_7, "np_7");
	MapSymbol(CELL_KEYC_KPAD_8, eKI_NP_8, "np_8");
	MapSymbol(CELL_KEYC_KPAD_9, eKI_NP_9, "np_9");
	MapSymbol(CELL_KEYC_KPAD_MINUS, eKI_NP_Substract, "np_subtract");
	MapSymbol(CELL_KEYC_KPAD_4, eKI_NP_4, "np_4");
	MapSymbol(CELL_KEYC_KPAD_5, eKI_NP_5, "np_5");
	MapSymbol(CELL_KEYC_KPAD_6, eKI_NP_6, "np_6");
	MapSymbol(CELL_KEYC_KPAD_PLUS, eKI_NP_Add, "np_add");
	MapSymbol(CELL_KEYC_KPAD_1, eKI_NP_1, "np_1");
	MapSymbol(CELL_KEYC_KPAD_2, eKI_NP_2, "np_2");
	MapSymbol(CELL_KEYC_KPAD_3, eKI_NP_3, "np_3");
	MapSymbol(CELL_KEYC_KPAD_0, eKI_NP_0, "np_0");
	MapSymbol(CELL_KEYC_F11, eKI_F11, "f11");
	MapSymbol(CELL_KEYC_F12, eKI_F12, "f12");
	//MapSymbol(CELL_KEYC_F13, "f13");
	//MapSymbol(CELL_KEYC_F14, "f14");
	//MapSymbol(CELL_KEYC_F15, "f15");
	//MapSymbol(CELL_KEYC_KPAD_EQUALS, "np_equals");
	//MapSymbol(CELL_KEYC_ACCENT_CIRCONFLEX_106,  "circumflex");
	//MapSymbol(CELL_KEYC_ATMARK_106,  "at");
	MapSymbol(CELL_KEYC_COLON_106, eKI_Colon, "colon");
	//MapSymbol(CELL_KEYC_UNDERLINE,  "underline");
	MapSymbol(CELL_KEYC_KPAD_ENTER, eKI_NP_Enter, "np_enter");
	//MapSymbol(CELL_KEYC_RCONTROL,  "rctrl", SInputSymbol::Button, eMM_RCtrl);
	MapSymbol(CELL_KEYC_KPAD_PERIOD, eKI_NP_Period, "np_period");
	MapSymbol(CELL_KEYC_KPAD_SLASH, eKI_NP_Divide, "np_divide");
	MapSymbol(CELL_KEYC_PRINTSCREEN, eKI_Print, "print");
	//MapSymbol(CELL_KEYC_RALT,  "ralt", SInputSymbol::Button, eMM_RAlt);
	MapSymbol(CELL_KEYC_PAUSE, eKI_Pause, "pause");
	MapSymbol(CELL_KEYC_HOME, eKI_Home, "home");
	MapSymbol(CELL_KEYC_UP_ARROW, eKI_Up, "up");
	MapSymbol(CELL_KEYC_PAGE_UP, eKI_PgUp, "pgup");
	MapSymbol(CELL_KEYC_LEFT_ARROW, eKI_Left, "left");
	MapSymbol(CELL_KEYC_RIGHT_ARROW, eKI_Right, "right");
	MapSymbol(CELL_KEYC_END, eKI_End, "end");
	MapSymbol(CELL_KEYC_DOWN_ARROW, eKI_Down, "down");
	MapSymbol(CELL_KEYC_PAGE_DOWN, eKI_PgDn, "pgdn");
	MapSymbol(CELL_KEYC_INSERT, eKI_Insert, "insert");
	MapSymbol(CELL_KEYC_DELETE, eKI_Delete, "delete");
	//MapSymbol(CELL_KEYC_LWIN,  "lwin");
	//MapSymbol(CELL_KEYC_RWIN,  "rwin");
	//MapSymbol(CELL_KEYC_APPS,  "apps");
	//MapSymbol(CELL_KEYC_OEM_102, "oem_102");
}

short GetAsyncKeyState(int vKey)
{
	//set least significant bit if it was pressed since last call
	#define PRESSED_SINCE_LAST 1
	//set most significant bit if it is currently pressed down
	#define KEY_DOWN 0x8000

	uint16 retVal = g_LastKeyState[vKey]?PRESSED_SINCE_LAST:0;
	if(g_keyState[vKey] == 1)
		retVal |= KEY_DOWN;
	g_LastKeyState[vKey] = 0;
	return retVal;
}

uint16 GetKeyState(int vKey)
{
	#define TOGGLED 1
	//set most significant bit if it is currently pressed down
	#define KEY_DOWN 0x8000
	//special impl for scroll key, no other is used so far
	if(vKey == VK_SCROLL)
	{
		static uint32 sScrollToggled = 0;
		const uint32 cDown = (g_keyState[vKey] == 1);
		if(cDown)
			sScrollToggled = (sScrollToggled + 1) & 1;
		uint16 retVal = sScrollToggled?TOGGLED:0;
		retVal |= cDown?KEY_DOWN:0;
		return retVal;
	}
	else
	if(vKey == VK_PAUSE)
	{
		static uint32 sPauseToggled = 0;
		const uint32 cDown = (g_keyState[vKey] == 1);
		if(cDown)
			sPauseToggled = (sPauseToggled + 1) & 1;
		uint16 retVal = sPauseToggled?TOGGLED:0;
		retVal |= cDown?KEY_DOWN:0;
		return retVal;
	}
	else
		return (g_keyState[vKey] == 1)?(TOGGLED | KEY_DOWN) : 0;
}

#endif //USE_PS3INPUT
