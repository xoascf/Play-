#include "InputProviderEmscripten.h"
#include "string_format.h"

constexpr uint32 PROVIDER_ID = 'EmSc';

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

	const char* keyName = emscripten_dom_pk_code_to_string(target.keyId);
	if(keyName && keyName[0])
	{
		return string_format("Keyboard: %s", keyName);
	}
	
	return string_format("Keyboard: 0x%X", target.keyId);
}

BINDINGTARGET CInputProviderEmscripten::MakeBindingTarget(uint32 domPkCode)
{
	return BINDINGTARGET(PROVIDER_ID, DeviceIdType{{0}}, domPkCode, BINDINGTARGET::KEYTYPE::BUTTON);
}

void CInputProviderEmscripten::OnKeyDown(const EM_UTF8* code)
{
	if(code && code[0])
	{
		OnInput(MakeBindingTarget(emscripten_compute_dom_pk_code(code)), 1);
	}
}

void CInputProviderEmscripten::OnKeyUp(const EM_UTF8* code)
{
	if(code && code[0])
	{
		OnInput(MakeBindingTarget(emscripten_compute_dom_pk_code(code)), 0);
	}
}

