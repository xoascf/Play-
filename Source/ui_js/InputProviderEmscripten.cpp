#include "InputProviderEmscripten.h"
#include "string_format.h"
#include <cstring>
#include <cctype>

constexpr uint32 PROVIDER_ID = 'EmSc';

enum
{
	INPUT_ARROW_UP = 0xFF80,
	INPUT_ARROW_DOWN,
	INPUT_ARROW_LEFT,
	INPUT_ARROW_RIGHT,
	INPUT_ENTER,
	INPUT_BACKSPACE,
	INPUT_TAB,
	INPUT_SPACE,
	INPUT_ESCAPE,
	INPUT_SHIFT_LEFT,
	INPUT_SHIFT_RIGHT,
	INPUT_CONTROL_LEFT,
	INPUT_CONTROL_RIGHT,
	INPUT_ALT_LEFT,
	INPUT_ALT_RIGHT,

	INPUT_KEY_A,
	INPUT_KEY_B,
	INPUT_KEY_C,
	INPUT_KEY_D,
	INPUT_KEY_E,
	INPUT_KEY_F,
	INPUT_KEY_G,
	INPUT_KEY_H,
	INPUT_KEY_I,
	INPUT_KEY_J,
	INPUT_KEY_K,
	INPUT_KEY_L,
	INPUT_KEY_M,
	INPUT_KEY_N,
	INPUT_KEY_O,
	INPUT_KEY_P,
	INPUT_KEY_Q,
	INPUT_KEY_R,
	INPUT_KEY_S,
	INPUT_KEY_T,
	INPUT_KEY_U,
	INPUT_KEY_V,
	INPUT_KEY_W,
	INPUT_KEY_X,
	INPUT_KEY_Y,
	INPUT_KEY_Z,

	INPUT_KEY_0,
	INPUT_KEY_1,
	INPUT_KEY_2,
	INPUT_KEY_3,
	INPUT_KEY_4,
	INPUT_KEY_5,
	INPUT_KEY_6,
	INPUT_KEY_7,
	INPUT_KEY_8,
	INPUT_KEY_9,
};

uint32 CInputProviderEmscripten::GetId() const
{
	return PROVIDER_ID;
}

std::string CInputProviderEmscripten::GetTargetDescription(const BINDINGTARGET& target) const
{
	if(target.providerId != PROVIDER_ID)
	{
		return std::string();
	}
	if(target.keyId >= INPUT_KEY_A && target.keyId <= INPUT_KEY_Z)
	{
		char keyChar = 'A' + (target.keyId - INPUT_KEY_A);
		return string_format("Keyboard: %c", keyChar);
	}
	if(target.keyId >= INPUT_KEY_0 && target.keyId <= INPUT_KEY_9)
	{
		char digitChar = '0' + (target.keyId - INPUT_KEY_0);
		return string_format("Keyboard: %c", digitChar);
	}
	switch(target.keyId)
	{
	case INPUT_ARROW_UP:
		return "Keyboard: Up";
	case INPUT_ARROW_DOWN:
		return "Keyboard: Down";
	case INPUT_ARROW_LEFT:
		return "Keyboard: Left";
	case INPUT_ARROW_RIGHT:
		return "Keyboard: Right";
	case INPUT_ENTER:
		return "Keyboard: Return";
	case INPUT_BACKSPACE:
		return "Keyboard: Backspace";
	case INPUT_SPACE:
		return "Keyboard: Space";
	case INPUT_TAB:
		return "Keyboard: Tab";
	case INPUT_ESCAPE:
		return "Keyboard: Escape";
	case INPUT_SHIFT_LEFT:
		return "Keyboard: Left Shift";
	case INPUT_SHIFT_RIGHT:
		return "Keyboard: Right Shift";
	case INPUT_CONTROL_LEFT:
		return "Keyboard: Left Control";
	case INPUT_CONTROL_RIGHT:
		return "Keyboard: Right Control";
	case INPUT_ALT_LEFT:
		return "Keyboard: Left Alt";
	case INPUT_ALT_RIGHT:
		return "Keyboard: Right Alt";
	default:
		return string_format("Keyboard: 0x%X", target.keyId);
	}
}

BINDINGTARGET CInputProviderEmscripten::MakeBindingTarget(const EM_UTF8* code)
{
	if(!code || !code[0])
	{
		return BINDINGTARGET(PROVIDER_ID, DeviceIdType{{0}}, 0, BINDINGTARGET::KEYTYPE::BUTTON);
	}

	uint32 keyCode = 0;

	// Arrows
	if(!strcmp(code, "ArrowUp") || !strcmp(code, "Up"))
		keyCode = INPUT_ARROW_UP;
	else if(!strcmp(code, "ArrowDown") || !strcmp(code, "Down"))
		keyCode = INPUT_ARROW_DOWN;
	else if(!strcmp(code, "ArrowLeft") || !strcmp(code, "Left"))
		keyCode = INPUT_ARROW_LEFT;
	else if(!strcmp(code, "ArrowRight") || !strcmp(code, "Right"))
		keyCode = INPUT_ARROW_RIGHT;
	// Enter / Return
	else if(!strcmp(code, "Enter") || !strcmp(code, "NumpadEnter") || !strcmp(code, "Return"))
		keyCode = INPUT_ENTER;
	// Backspace
	else if(!strcmp(code, "Backspace"))
		keyCode = INPUT_BACKSPACE;
	// Space / Tab / Escape
	else if(!strcmp(code, "Space") || !strcmp(code, " "))
		keyCode = INPUT_SPACE;
	else if(!strcmp(code, "Tab"))
		keyCode = INPUT_TAB;
	else if(!strcmp(code, "Escape") || !strcmp(code, "Esc"))
		keyCode = INPUT_ESCAPE;
	// Modifiers
	else if(!strcmp(code, "ShiftLeft"))
		keyCode = INPUT_SHIFT_LEFT;
	else if(!strcmp(code, "ShiftRight") || !strcmp(code, "Shift"))
		keyCode = INPUT_SHIFT_RIGHT;
	else if(!strcmp(code, "ControlLeft"))
		keyCode = INPUT_CONTROL_LEFT;
	else if(!strcmp(code, "ControlRight") || !strcmp(code, "Control"))
		keyCode = INPUT_CONTROL_RIGHT;
	else if(!strcmp(code, "AltLeft"))
		keyCode = INPUT_ALT_LEFT;
	else if(!strcmp(code, "AltRight") || !strcmp(code, "Alt"))
		keyCode = INPUT_ALT_RIGHT;
	// Alphabet keys: "KeyA" .. "KeyZ" or "a"/"A" .. "z"/"Z"
	else if(!strncmp(code, "Key", 3) && code[3] >= 'A' && code[3] <= 'Z' && code[4] == '\0')
		keyCode = INPUT_KEY_A + (code[3] - 'A');
	else if(!strncmp(code, "Key", 3) && code[3] >= 'a' && code[3] <= 'z' && code[4] == '\0')
		keyCode = INPUT_KEY_A + (code[3] - 'a');
	else if(code[0] >= 'A' && code[0] <= 'Z' && code[1] == '\0')
		keyCode = INPUT_KEY_A + (code[0] - 'A');
	else if(code[0] >= 'a' && code[0] <= 'z' && code[1] == '\0')
		keyCode = INPUT_KEY_A + (code[0] - 'a');
	// Numeric keys: "Digit0".."Digit9", "Numpad0".."Numpad9", "Key0".."Key9", "0".."9"
	else if(!strncmp(code, "Digit", 5) && code[5] >= '0' && code[5] <= '9' && code[6] == '\0')
		keyCode = INPUT_KEY_0 + (code[5] - '0');
	else if(!strncmp(code, "Numpad", 6) && code[6] >= '0' && code[6] <= '9' && code[7] == '\0')
		keyCode = INPUT_KEY_0 + (code[6] - '0');
	else if(!strncmp(code, "Key", 3) && code[3] >= '0' && code[3] <= '9' && code[4] == '\0')
		keyCode = INPUT_KEY_0 + (code[3] - '0');
	else if(code[0] >= '0' && code[0] <= '9' && code[1] == '\0')
		keyCode = INPUT_KEY_0 + (code[0] - '0');
	else
		keyCode = code[0];

	return BINDINGTARGET(PROVIDER_ID, DeviceIdType{{0}}, keyCode, BINDINGTARGET::KEYTYPE::BUTTON);
}

void CInputProviderEmscripten::OnKeyDown(const EM_UTF8* code, const EM_UTF8* key)
{
	auto target = MakeBindingTarget(code);
	if(target.keyId == 0 && key && key[0])
	{
		target = MakeBindingTarget(key);
	}
	OnInput(target, 1);
}

void CInputProviderEmscripten::OnKeyUp(const EM_UTF8* code, const EM_UTF8* key)
{
	auto target = MakeBindingTarget(code);
	if(target.keyId == 0 && key && key[0])
	{
		target = MakeBindingTarget(key);
	}
	OnInput(target, 0);
}

