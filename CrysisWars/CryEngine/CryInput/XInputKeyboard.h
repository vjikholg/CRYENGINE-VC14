#ifndef __XINPUTKEYBOARD_H__
#define __XINPUTKEYBOARD_H__
#pragma once

#ifdef USE_XENONINPUT

#include"InputDevice.h"

#if defined(USE_DXINPUT) || defined(USE_XENONINPUT)

#if defined(USE_DXINPUT)
#include <xinput.h>
#pragma comment(lib, "xinput.lib")
#elif defined(USE_XENONINPUT)
#include <xtl.h>
#endif

class CXInputKeyboard : public CInputDevice
{
public:
	CXInputKeyboard(IInput& pInput);

	// IInputDevice overrides
	virtual bool Init();
	virtual void Update();
	virtual const char *GetKeyName(const SInputEvent& event, bool bGUI);
	virtual bool IsOfDeviceType( EInputDeviceType type ) const { return false; }
	// ~IInputDevice overrides
private:
	void ProcessKey(uint32 key, bool pressed, WCHAR unicode);

	uint32	m_modifiers;
};

#endif //defined(USE_DXINPUT) || defined(USE_XENONINPUT)

#endif //USE_XENONINPUT

#endif //__XINPUTKEYBOARD_H__
