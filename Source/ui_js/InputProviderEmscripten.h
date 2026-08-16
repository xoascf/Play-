#pragma once

#include "input/InputProvider.h"
#include <emscripten/html5.h>
#include <emscripten/key_codes.h>

class CInputProviderEmscripten : public CInputProvider
{
public:
	uint32 GetId() const override;
	std::string GetTargetDescription(const BINDINGTARGET&) const override;

	static BINDINGTARGET MakeBindingTarget(uint32 domPkCode);

	void OnKeyDown(const EM_UTF8* code);
	void OnKeyUp(const EM_UTF8* code);
};
