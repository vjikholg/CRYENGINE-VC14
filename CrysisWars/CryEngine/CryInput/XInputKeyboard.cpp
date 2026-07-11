#include "StdAfx.h"

#ifdef USE_XENONINPUT

#include "XInputKeyboard.h"
#if defined(USE_XENONINPUT)
#	include <xam.h>
#else
#define XINPUT_FLAG_KEYBOARD            0x00000002
#endif

CXInputKeyboard::CXInputKeyboard(IInput& input) : CInputDevice(input, "xinputkb")
{
	m_deviceId = eDI_Keyboard;
}

bool CXInputKeyboard::Init()
{
	MapSymbol(VK_ESCAPE, eKI_Escape, "escape");
	MapSymbol('1', eKI_1, "1");
	MapSymbol('2', eKI_2, "2");
	MapSymbol('3', eKI_3, "3");
	MapSymbol('4', eKI_4, "4");
	MapSymbol('5', eKI_5, "5");
	MapSymbol('6', eKI_6, "6");
	MapSymbol('7', eKI_7, "7");
	MapSymbol('8', eKI_8, "8");
	MapSymbol('9', eKI_9, "9");
	MapSymbol('0', eKI_0, "0");
	MapSymbol(VK_OEM_MINUS, eKI_Minus, "minus");
	MapSymbol(VK_OEM_NEC_EQUAL, eKI_Equals, "equals");
	MapSymbol(VK_BACK, eKI_Backspace, "backspace");
	MapSymbol(VK_TAB, eKI_Tab, "tab");
	MapSymbol('Q', eKI_Q, "q");
	MapSymbol('W', eKI_W, "w");
	MapSymbol('E', eKI_E, "e");
	MapSymbol('R', eKI_R, "r");
	MapSymbol('T', eKI_T, "t");
	MapSymbol('Y', eKI_Y, "y");
	MapSymbol('U', eKI_U, "u");
	MapSymbol('I', eKI_I, "i");
	MapSymbol('O', eKI_O, "o");
	MapSymbol('P', eKI_P, "p");
	MapSymbol(VK_OEM_4, eKI_LBracket, "lbracket");
	MapSymbol(VK_OEM_6, eKI_RBracket, "rbracket");
	MapSymbol(VK_RETURN, eKI_Enter, "enter");
	MapSymbol(VK_LCONTROL, eKI_LCtrl, "lctrl", SInputSymbol::Button, eMM_LCtrl);
	MapSymbol('A',eKI_A, "a");
	MapSymbol('S',eKI_S, "s");
	MapSymbol('D',eKI_D, "d");
	MapSymbol('F',eKI_F, "f");
	MapSymbol('G',eKI_G, "g");
	MapSymbol('H',eKI_H, "h");
	MapSymbol('J',eKI_J, "j");
	MapSymbol('K',eKI_K, "k");
	MapSymbol('L',eKI_L, "l");
	MapSymbol(VK_OEM_1, eKI_Semicolon, "semicolon");
	MapSymbol(VK_OEM_7, eKI_Apostrophe, "apostrophe");
	MapSymbol(VK_OEM_3, eKI_Tilde, "tilde");
	MapSymbol(VK_LSHIFT, eKI_LShift, "lshift", SInputSymbol::Button, eMM_LShift);
	MapSymbol(VK_OEM_5, eKI_Backslash, "backslash");
	MapSymbol('Z', eKI_Z, "z");
	MapSymbol('X', eKI_X, "x");
	MapSymbol('C', eKI_C, "c");
	MapSymbol('V', eKI_V, "v");
	MapSymbol('B', eKI_B, "b");
	MapSymbol('N', eKI_N, "n");
	MapSymbol('M', eKI_M, "m");
	MapSymbol(VK_OEM_COMMA, eKI_Comma, "comma");
	MapSymbol(VK_OEM_PERIOD, eKI_Period, "period");
	MapSymbol(VK_OEM_2, eKI_Slash, "slash");
	MapSymbol(VK_RSHIFT, eKI_RShift, "rshift", SInputSymbol::Button, eMM_RShift);
	MapSymbol(VK_MULTIPLY, eKI_NP_Multiply, "np_multiply");
	MapSymbol(VK_LMENU, eKI_LAlt, "lalt", SInputSymbol::Button, eMM_LAlt);
	MapSymbol(VK_SPACE, eKI_Space, "space");
	MapSymbol(VK_CAPITAL, eKI_CapsLock, "capslock", SInputSymbol::Toggle, eMM_CapsLock);
	MapSymbol(VK_F1, eKI_F1, "f1");
	MapSymbol(VK_F2, eKI_F2, "f2");
	MapSymbol(VK_F3, eKI_F3, "f3");
	MapSymbol(VK_F4, eKI_F4, "f4");
	MapSymbol(VK_F5, eKI_F5, "f5");
	MapSymbol(VK_F6, eKI_F6, "f6");
	MapSymbol(VK_F7, eKI_F7, "f7");
	MapSymbol(VK_F8, eKI_F8, "f8");
	MapSymbol(VK_F9, eKI_F9, "f9");
	MapSymbol(VK_F10, eKI_F10, "f10");
	MapSymbol(VK_NUMLOCK, eKI_NumLock, "numlock", SInputSymbol::Toggle, eMM_NumLock);
	MapSymbol(VK_SCROLL, eKI_ScrollLock, "scrolllock", SInputSymbol::Toggle, eMM_ScrollLock);
	MapSymbol(VK_NUMPAD7, eKI_NP_7, "np_7");
	MapSymbol(VK_NUMPAD8, eKI_NP_8, "np_8");
	MapSymbol(VK_NUMPAD9, eKI_NP_9, "np_9");
	MapSymbol(VK_SUBTRACT, eKI_NP_Substract, "np_subtract");
	MapSymbol(VK_NUMPAD4, eKI_NP_4, "np_4");
	MapSymbol(VK_NUMPAD5, eKI_NP_5, "np_5");
	MapSymbol(VK_NUMPAD6, eKI_NP_6, "np_6");
	MapSymbol(VK_ADD, eKI_NP_Add, "np_add");
	MapSymbol(VK_NUMPAD1, eKI_NP_1, "np_1");
	MapSymbol(VK_NUMPAD2, eKI_NP_2, "np_2");
	MapSymbol(VK_NUMPAD3, eKI_NP_3, "np_3");
	MapSymbol(VK_NUMPAD0, eKI_NP_0, "np_0");
	MapSymbol(VK_F11, eKI_F11, "f11");
	MapSymbol(VK_F12, eKI_F12, "f12");
	MapSymbol(VK_F13, eKI_F13, "f13");
	MapSymbol(VK_F14, eKI_F14, "f14");
	MapSymbol(VK_F15, eKI_F15, "f15");
	//MapSymbol(VK_NUMPADEQUALS, "np_equals");
	//MapSymbol(VK_CIRCUMFLEX,  "circumflex");
	//MapSymbol(VK_AT,  "at");
	//MapSymbol(VK_COLON, "colon");
	MapSymbol(VK_SEPARATOR,  eKI_Underline, "underline");
	MapSymbol(VK_EXECUTE,  eKI_NP_Enter, "np_enter");
	MapSymbol(VK_RCONTROL,  eKI_RCtrl, "rctrl", SInputSymbol::Button, eMM_RCtrl);
	MapSymbol(VK_DECIMAL,  eKI_NP_Period, "np_period");
	MapSymbol(VK_DIVIDE, eKI_NP_Divide, "np_divide");
	MapSymbol(VK_PRINT, eKI_Print, "print");
	MapSymbol(VK_RMENU, eKI_RAlt, "ralt", SInputSymbol::Button, eMM_RAlt);
	MapSymbol(VK_PAUSE, eKI_Pause, "pause");
	MapSymbol(VK_HOME, eKI_Home, "home");
	MapSymbol(VK_UP, eKI_Up, "up");
	MapSymbol(VK_PRIOR, eKI_PgUp, "pgup");
	MapSymbol(VK_LEFT, eKI_Left, "left");
	MapSymbol(VK_RIGHT, eKI_Right, "right");
	MapSymbol(VK_END, eKI_End, "end");
	MapSymbol(VK_DOWN, eKI_Down, "down");
	MapSymbol(VK_NEXT, eKI_PgDn, "pgdn");
	MapSymbol(VK_INSERT, eKI_Insert, "insert");
	MapSymbol(VK_DELETE, eKI_Delete, "delete");
	MapSymbol(VK_LWIN, eKI_LWin, "lwin");
	MapSymbol(VK_RWIN, eKI_RWin, "rwin");
	MapSymbol(VK_APPS, eKI_Apps, "apps");
	MapSymbol(VK_OEM_102, eKI_OEM_102, "oem_102");

	return true;
}

void CXInputKeyboard::Update()
{
	FUNCTION_PROFILER( GetISystem(),PROFILE_INPUT );

	XINPUT_KEYSTROKE keyStroke;
	int result = XInputGetKeystroke(XUSER_INDEX_ANY, XINPUT_FLAG_KEYBOARD, &keyStroke);

	if (result == ERROR_SUCCESS)
	{
		if (!(keyStroke.Flags & XINPUT_KEYSTROKE_REPEAT))
		{
			bool commit = true;
			bool pressed;

			if (keyStroke.Flags & XINPUT_KEYSTROKE_KEYDOWN)
				pressed = true;
			else if (keyStroke.Flags & XINPUT_KEYSTROKE_KEYUP)
				pressed = false;
			else
				commit = false;

			if (commit)
				ProcessKey(keyStroke.VirtualKey, pressed, keyStroke.Unicode);
		}
	}
}

void CXInputKeyboard::ProcessKey(uint32 key, bool pressed, WCHAR unicode)
{
	SInputSymbol* pSymbol = DevSpecIdToSymbol(key);

	if (!pSymbol) return;

	EInputState newState;

	if (pressed)
	{
		if (pSymbol->type == SInputSymbol::Toggle)
		{
			if (m_modifiers & pSymbol->user)
				m_modifiers &= ~pSymbol->user;
			else
				m_modifiers |= pSymbol->user;
		}
		else if (pSymbol->user)
		{
			// must be a regular modifier key
			m_modifiers |= pSymbol->user;
		}

		newState = eIS_Pressed;
		pSymbol->value = 1.0f;
	}
	else
	{
		if (pSymbol->user && pSymbol->type == SInputSymbol::Button)
		{
			// must be a regular modifier key
			m_modifiers &= ~pSymbol->user;
		}

		newState = eIS_Released;
		pSymbol->value = 0.0f;
	}

	// check if the state has really changed ... otherwise ignore it
	if (newState == pSymbol->state)
	{
		//gEnv->pLog->Log("Input: Identical key event discarded: '%s' - %s", pSymbol->name.c_str(), newState==eIS_Pressed?"pressed":"released");
		return;
	}
	pSymbol->state = newState;

	// Post input events
	SInputEvent event;
	//hacky!!! - abuse timestamp for carrying unicode value
  event.timestamp = unicode ? unicode : key;
	event.modifiers = m_modifiers;
  event.deviceId = eDI_Keyboard;

	event.state = pSymbol->state;
	event.value = pSymbol->value;
	event.keyName = pSymbol->name;
	event.keyId = pSymbol->keyId;
	GetIInput().PostInputEvent(event);
}

const char* CXInputKeyboard::GetKeyName(const SInputEvent& event, bool bGUI)
{
	if (event.deviceId != eDI_Keyboard)
		return 0;

	if (bGUI)
	{
		// get some hacky keyname
    switch (event.timestamp)
    {
      case VK_ADD:
        return "+";
        break;
      case VK_SUBTRACT:
        return "-";
        break;
      case VK_NUMPAD1:
        return "1";
        break;
      case VK_NUMPAD2:
        return "2";
        break;
      case VK_NUMPAD3:
        return "3";
        break;
      case VK_NUMPAD4:
        return "4";
        break;
      case VK_NUMPAD5:
        return "5";
        break;
      case VK_NUMPAD6:
        return "6";
        break;
      case VK_NUMPAD7:
        return "7";
        break;
      case VK_NUMPAD8:
        return "8";
        break;
      case VK_NUMPAD9:
        return "9";
        break;
      case VK_NUMPAD0:
        return "0";
        break;

      default:
        {
          static char szKeyName[4] = {0};
          if (wctomb(szKeyName, (WCHAR)event.timestamp) != -1)
          {
            szKeyName[1] = 0;
          }

          return szKeyName;
        }
    }
	}
	else
	{
		return event.keyName;
	}
}

#endif USE_XENONINPUT
