// XDebugKeyboard.cpp
//
//////////////////////////////////////////////////////////////////////


#include "StdAfx.h"
#include <ILog.h>
#include <stdio.h>

#ifdef DEBUG_KEYBOARD

#include "XDebugKeyboard.h"

#define		TOGGLE_CAPSLOCK			(1 << 0)
#define		TOGGLE_NUMLOCK			(1 << 1)
#define		TOGGLE_SCROLLLOCK		(1 << 2)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CXDebugKeyboard::CXDebugKeyboard(CXenonInput& input) : CXenonInputDevice(input, "keyboard")
{
  memset(m_cKeysState, 0, sizeof(m_cKeysState));
	memset(m_cOldKeysState, 0, sizeof(m_cOldKeysState));
  memset(m_bKeys, 0, sizeof(m_bKeys));
  m_hDebugKeyboard = 0;
	m_iToggleState = 0;
	m_modifiers = 0;
}

CXDebugKeyboard::~CXDebugKeyboard()
{

}

bool CXDebugKeyboard::Init()
{
	//Acquire();	

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::SetExclusive(bool value,void *pUser)
{ 

}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::Update()
{
  //BOOL bReturn;
  DWORD dwInsertions = 0, dwRemovals = 0;
  DWORD dwReturn;

#ifdef _XBOX

	XINPUT_KEYSTROKE keyStroke;

	memcpy(m_cOldKeysState,m_cKeysState,sizeof(m_cOldKeysState));

//	do {
    // Query the keyboard state
		dwReturn = XInputGetKeystroke( XUSER_INDEX_ANY,XINPUT_FLAG_KEYBOARD,&keyStroke );

		if (dwReturn == ERROR_SUCCESS)
		{
			//////////////////////////////////////////////////////////////////////////
			// Update modifiers.
			//////////////////////////////////////////////////////////////////////////
			if (keyStroke.Flags & XINPUT_KEYSTROKE_ALT)
				m_modifiers |= eMM_LAlt|eMM_RAlt;
			else
				m_modifiers &= ~(eMM_LAlt|eMM_RAlt);

			if (keyStroke.Flags & XINPUT_KEYSTROKE_CTRL)
				m_modifiers |= eMM_LCtrl|eMM_RCtrl;
			else
				m_modifiers &= ~(eMM_LCtrl|eMM_RCtrl);

			if (keyStroke.Flags & XINPUT_KEYSTROKE_SHIFT)
				m_modifiers |= eMM_LShift|eMM_RShift;
			else
				m_modifiers &= ~(eMM_LShift|eMM_RShift);
			//////////////////////////////////////////////////////////////////////////

			// We got a key up or key down event ?
      int dik =-1;

      switch(keyStroke.VirtualKey)
      {
        case VK_TAB         :dik = XKEY_TAB         ;break;
        case VK_BACK        :dik = XKEY_BACKSPACE   ;break;
        case VK_RETURN      :dik = XKEY_RETURN      ;break;
        case VK_SHIFT       :dik = XKEY_SHIFT       ;break;
        case VK_CONTROL     :dik = XKEY_CONTROL     ;break;
        case VK_PAUSE       :dik = XKEY_PAUSE       ;break;


        case VK_ESCAPE      :dik = XKEY_ESCAPE      ;break;

        case VK_SPACE       :dik = XKEY_SPACE       ;break;


        case VK_END         :dik = XKEY_END         ;break;
        case VK_HOME        :dik = XKEY_HOME        ;break;
        case VK_LEFT        :dik = XKEY_LEFT        ;break;
        case VK_UP          :dik = XKEY_UP          ;break;
        case VK_RIGHT       :dik = XKEY_RIGHT       ;break;
        case VK_DOWN        :dik = XKEY_DOWN        ;break;

        case VK_PRINT       :dik = XKEY_PRINT       ;break;


        case VK_INSERT      :dik = XKEY_INSERT      ;break;
        case VK_DELETE      :dik = XKEY_DELETE      ;break;
        case VK_HELP        :dik = XKEY_HELP        ;break;

        //case  VK_LALT       :dik = XKEY_LALT        ;break;
        //case  VK_RALT       :dik = XKEY_RALT        ;break;

        //case  VK_LSHIFT     :dik = XKEY_LSHIFT      ;break;
        //case  VK_SHIFT     :dik = XKEY_RSHIFT      ;break;
        //case  VK_LCONTROL   :dik = XKEY_LCONTROL    ;break;
        //case  VK_CONTROL   :dik = XKEY_RCONTROL    ;break;


        case VK_NUMPAD0    :  dik = XKEY_NUMPAD0    ;break;
        case VK_NUMPAD1    :  dik = XKEY_NUMPAD1    ;break;
        case VK_NUMPAD2    :  dik = XKEY_NUMPAD2    ;break;
        case VK_NUMPAD3    :  dik = XKEY_NUMPAD3    ;break;
        case VK_NUMPAD4    :  dik = XKEY_NUMPAD4    ;break;
        case VK_NUMPAD5    :  dik = XKEY_NUMPAD5    ;break;
        case VK_NUMPAD6    :  dik = XKEY_NUMPAD6    ;break;
        case VK_NUMPAD7    :  dik = XKEY_NUMPAD7    ;break;
        case VK_NUMPAD8    :  dik = XKEY_NUMPAD8    ;break;
        case VK_NUMPAD9    :  dik = XKEY_NUMPAD9    ;break;
        case VK_MULTIPLY   :  dik = XKEY_MULTIPLY   ;break;
        case VK_ADD        :  dik = XKEY_ADD        ;break;
        case VK_SEPARATOR  :  dik = XKEY_SEPARATOR  ;break;
        case VK_SUBTRACT   :  dik = XKEY_SUBTRACT   ;break;
        case VK_DECIMAL    :  dik = XKEY_DECIMAL    ;break;
        case VK_DIVIDE     :  dik = XKEY_DIVIDE     ;break;
        case VK_F1         :  dik = XKEY_F1         ;break;
        case VK_F2         :  dik = XKEY_F2         ;break;
        case VK_F3         :  dik = XKEY_F3         ;break;
        case VK_F4         :  dik = XKEY_F4         ;break;
        case VK_F5         :  dik = XKEY_F5         ;break;
        case VK_F6         :  dik = XKEY_F6         ;break;
        case VK_F7         :  dik = XKEY_F7         ;break;
        case VK_F8         :  dik = XKEY_F8         ;break;
        case VK_F9         :  dik = XKEY_F9         ;break;
        case VK_F10        :  dik = XKEY_F10        ;break;
        case VK_F11        :  dik = XKEY_F11        ;break;
        case VK_F12        :  dik = XKEY_F12        ;break;
        case VK_F13        :  dik = XKEY_F13        ;break;
        case VK_F14        :  dik = XKEY_F14        ;break;
        case VK_F15        :  dik = XKEY_F15        ;break;
        case VK_F16        :  dik = XKEY_F16        ;break;
        case VK_F17        :  dik = XKEY_F17        ;break;
        case VK_F18        :  dik = XKEY_F18        ;break;
        case VK_F19        :  dik = XKEY_F19        ;break;
        case VK_F20        :  dik = XKEY_F20        ;break;
        case VK_F21        :  dik = XKEY_F21        ;break;
        case VK_F22        :  dik = XKEY_F22        ;break;
        case VK_F23        :  dik = XKEY_F23        ;break;
        case VK_F24        :  dik = XKEY_F24        ;break;
      }
      
			if (dik < 0)
			{
				if('a' <= keyStroke.Unicode && keyStroke.Unicode <='z')
					dik = XKEY_A + keyStroke.Unicode - 'a';

				if('A' <= keyStroke.Unicode && keyStroke.Unicode <='Z')
					dik = XKEY_A + keyStroke.Unicode - 'A';

				if('0' <= keyStroke.Unicode && keyStroke.Unicode <='9')
					dik = XKEY_0 + keyStroke.Unicode - '0';
			}
			
			if (dik < 0)
			{
				switch(keyStroke.Unicode)
				{
				case '`' :dik = XKEY_TILDE       ;break;
				case '=' :dik = XKEY_EQUALS      ;break;
				case '[' :dik = XKEY_LBRACKET    ;break;
				case ']' :dik = XKEY_RBRACKET    ;break;
				case '\\' :dik = XKEY_BACKSLASH  ;break;
				case ';' :dik = XKEY_SEMICOLON   ;break;
				case '\'' :dik = XKEY_APOSTROPHE ;break;
				case ',' :dik = XKEY_COMMA       ;break;
				case '-' :dik = XKEY_MINUS       ;break;
				case '.' :dik = XKEY_DECIMAL     ;break;
				case '/' :dik = XKEY_SLASH       ;break;

				case '¬' :dik = XKEY_TILDE       ;break;
				case '+' :dik = XKEY_EQUALS      ;break;
				case '{' :dik = XKEY_LBRACKET    ;break;
				case '}' :dik = XKEY_RBRACKET    ;break;
				case '|' :dik = XKEY_BACKSLASH  ;break;
				case ':' :dik = XKEY_SEMICOLON   ;break;
				case '"' :dik = XKEY_APOSTROPHE ;break;
				case '<' :dik = XKEY_COMMA       ;break;
				case '_' :dik = XKEY_MINUS       ;break;
				case '>' :dik = XKEY_DECIMAL     ;break;
				case '?' :dik = XKEY_SLASH       ;break;

				case ')' :dik = XKEY_0       ;break;
				case '!' :dik = XKEY_1       ;break;
				case '@' :dik = XKEY_2       ;break;
				case '#' :dik = XKEY_3       ;break;
				case '$' :dik = XKEY_4       ;break;
				case '%' :dik = XKEY_5       ;break;
				case '^' :dik = XKEY_6       ;break;
				case '&' :dik = XKEY_7       ;break;
				case '*' :dik = XKEY_8       ;break;
				case '(' :dik = XKEY_9       ;break;
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// Toggle states.
			//////////////////////////////////////////////////////////////////////////
			if (keyStroke.Flags & (XINPUT_KEYSTROKE_KEYDOWN|XINPUT_KEYSTROKE_REPEAT))
			{
				if (keyStroke.VirtualKey == VK_CAPITAL)
				{
					if (m_iToggleState & TOGGLE_CAPSLOCK)
						m_iToggleState &= ~TOGGLE_CAPSLOCK;
					else
						m_iToggleState |= TOGGLE_CAPSLOCK;
				}
				if (keyStroke.VirtualKey == VK_NUMLOCK)
				{
					if (m_iToggleState & TOGGLE_NUMLOCK)
						m_iToggleState &= ~TOGGLE_NUMLOCK;
					else
						m_iToggleState |= TOGGLE_NUMLOCK;
				}
				if (keyStroke.VirtualKey == VK_SCROLL)
				{
					if (m_iToggleState & TOGGLE_SCROLLLOCK)
						m_iToggleState &= ~TOGGLE_SCROLLLOCK;
					else
						m_iToggleState |= TOGGLE_SCROLLLOCK;
				}
			}
			//////////////////////////////////////////////////////////////////////////
			
      if (dik!=0)
			{
				if (keyStroke.Flags & XINPUT_KEYSTROKE_KEYDOWN)
					m_cKeysState[dik] = 1;
				else if (keyStroke.Flags & XINPUT_KEYSTROKE_KEYUP)
					m_cKeysState[dik] = 0;

				if (keyStroke.Flags & (XINPUT_KEYSTROKE_KEYDOWN|XINPUT_KEYSTROKE_KEYUP))
				{
					// Generate keyboard event.
					SInputEvent event;
					event.keyId = dik;
					if (keyStroke.Flags & XINPUT_KEYSTROKE_KEYDOWN)
					{
						event.type = KEY_PRESS;
						event.value = 1.0f;
					}
					else
					{
						event.type = KEY_RELEASE;
						event.value = 0;
					}
					event.timestamp = 0;
					event.modifiers = m_modifiers;
					event.keyName = GetKeyName( event );
					m_pInput->PostInputEvent( event );
				}				
			}
		}
		//} while (dwReturn == ERROR_SUCCESS);


		XINPUT_STATE m_xbStatus[8];
		static XINPUT_STATE m_xbLastStatus[8];
		for (int i=0; i<1; i++)
		{
			// Save the old state
			m_xbLastStatus[i] = m_xbStatus[i];
			memset( m_xbStatus,0,sizeof(m_xbStatus) );

			// Get the new one
			XInputGetState( i, &m_xbStatus[i]);

			//////////////////////////////////////////////////////////////////////////
			// Use.
			//////////////////////////////////////////////////////////////////////////
			if (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_Y)
				PressKey( XKEY_SPACE );
			else
				ReleaseKey( XKEY_SPACE );

			if (m_xbStatus[i].Gamepad.wButtons & XINPUT_GAMEPAD_X)
				PressKey( XKEY_1 );
			else
				ReleaseKey( XKEY_1 );

			// Use on Right Trigger.
			if (m_xbStatus[i].Gamepad.bRightTrigger > 128)
				PressKey( XKEY_F );
			else
				ReleaseKey( XKEY_F );

			// Map on Left Trigger.
			if (m_xbStatus[i].Gamepad.bLeftTrigger > 128)
				PressKey( XKEY_F1 );
			else
				ReleaseKey( XKEY_F1 );

			//////////////////////////////////////////////////////////////////////////
			// Handle Left Stick.
			//////////////////////////////////////////////////////////////////////////
			int lx = m_xbStatus[i].Gamepad.sThumbLX >> 8;
			int ly = m_xbStatus[i].Gamepad.sThumbLY >> 8;
			//CryLogAlways( "lx=%d,   ly=%d",lx,ly );
			if (m_xbStatus[i].Gamepad.sThumbLX != m_xbLastStatus[i].Gamepad.sThumbLX)
			{
				if (abs(lx) > 60)
				{
					// Post Input events.
					SInputEvent event;
					if (lx > 0)
						PressKey( XKEY_D,-lx/100.0f );
					else
						PressKey( XKEY_A,-lx/100.0f );
				}
				else
				{
					ReleaseKey( XKEY_A );
					ReleaseKey( XKEY_D );
				}
			}
			if (m_xbStatus[i].Gamepad.sThumbLY != m_xbLastStatus[i].Gamepad.sThumbLY)
			{
				if (abs(ly) > 60)
				{
					// Post Input events.
					SInputEvent event;
					if (ly > 0)
						PressKey( XKEY_W,-ly/100.0f );
					else
						PressKey( XKEY_S,-ly/100.0f );
				}
				else
				{
					ReleaseKey( XKEY_W );
					ReleaseKey( XKEY_S );
				}
			}
		}

#endif // #ifdef _XBOX
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::PressKey( int key,float val )
{
	if (m_cKeysState[key] == 0)
	{
		m_cKeysState[key] = 1;
		SInputEvent event;
		event.type = KEY_PRESS;
		event.keyId = key;
		event.value = val;
		event.keyName = GetKeyName( event );
		m_pInput->PostInputEvent( event );

		CryLogAlways( "KeyPress  %s, value=%f",event.keyname,event.value );
	}
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::ReleaseKey( int key )
{
	if (m_cKeysState[key])
	{
		m_cKeysState[key] = 0;
		SInputEvent event;
		event.type = KEY_RELEASE;
		event.key = key;
		event.keyname = m_pInput->GetKeyName( event );
		m_pInput->PostInputEvent( event );
	}
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::ShutDown()
{
}

//////////////////////////////////////////////////////////////////////////
const char *CXDebugKeyboard::EnumerateKey(int nKeyCode)
{
	SInputEvent event;
	event.key = nKeyCode;

	return GetKeyName(event);
}

//////////////////////////////////////////////////////////////////////////
void CXDebugKeyboard::ClearKeyState()
{
	memset(m_cKeysState,0,sizeof(m_cKeysState));
}

//////////////////////////////////////////////////////////////////////////
unsigned char CXDebugKeyboard::XKEY2ASCII(unsigned short nCode,int modifiers)
{
	if (nCode >= XKEY_0 && nCode <= XKEY_9)
		return '0' + (nCode-XKEY_0);

	if (nCode >= XKEY_A && nCode <= XKEY_Z)
	{
		bool bCaps = m_iToggleState & TOGGLE_CAPSLOCK;
		if (modifiers & (XKEY_MOD_LSHIFT|XKEY_MOD_RSHIFT))
			bCaps = !bCaps;

		if (bCaps)
			return 'A' + (nCode-XKEY_A);
		else
			return 'a' + (nCode-XKEY_A);
	}
	return 0;
}

//////////////////////////////////////////////////////////////////////////
bool CXDebugKeyboard::GetOSKeyName(int nKey, wchar_t *szwKeyName, int iBufSize)
{
	SInputEvent event;
	event.key = nKey;
	event.modifiers = m_modifiers;

	static wchar_t sName[1];
	const char *s = m_pInput->GetKeyName(event);
	sName[0] = s[0];
  szwKeyName = sName;
	return true;
}


#endif // USE_DEBUGKEYBOARD