#include <cstdio>
#include <emscripten/bind.h>
#include "Ps2VmJs.h"
#include "GSH_OpenGLJs.h"
#include "sound/SH_OpenAL/SH_OpenALProxy.h"
#include "input/PH_GenericInput.h"
#include "InputProviderEmscripten.h"
#include <emscripten/key_codes.h>
#include "ui_shared/StatsManager.h"
#include "DefaultAppConfig.h"

CPs2VmJs* g_virtualMachine = nullptr;
CGSHandler::NewFrameEvent::Connection g_gsNewFrameConnection;
EMSCRIPTEN_WEBGL_CONTEXT_HANDLE g_context = 0;
std::shared_ptr<CInputProviderEmscripten> g_inputProvider;
CSH_OpenAL* g_soundHandler = nullptr;

int main(int argc, const char** argv)
{
	printf("Play! - Version %s\r\n", PLAY_VERSION);
	return 0;
}

EM_BOOL keyboardCallback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData)
{
	if(keyEvent->repeat)
	{
		return true;
	}
	if(!g_inputProvider)
	{
		return false;
	}
	switch(eventType)
	{
	case EMSCRIPTEN_EVENT_KEYDOWN:
		g_inputProvider->OnKeyDown(keyEvent->code);
		break;
	case EMSCRIPTEN_EVENT_KEYUP:
		g_inputProvider->OnKeyUp(keyEvent->code);
		break;
	}
	return true;
}

extern "C" void initVm()
{
	EmscriptenWebGLContextAttributes attr;
	emscripten_webgl_init_context_attributes(&attr);
	attr.majorVersion = 2;
	attr.minorVersion = 0;
	attr.alpha = false;
	g_context = emscripten_webgl_create_context("#outputCanvas", &attr);
	assert(g_context >= 0);

	g_virtualMachine = new CPs2VmJs();
	g_virtualMachine->Initialize();
	g_virtualMachine->CreateGSHandler(CGSH_OpenGLJs::GetFactoryFunction(g_context));

	{
		//Size here needs to match the size of the canvas in HTML file.

		CGSHandler::PRESENTATION_PARAMS presentationParams;
		presentationParams.mode = CGSHandler::PRESENTATION_MODE_FIT;
		presentationParams.windowWidth = 640;
		presentationParams.windowHeight = 480;

		g_virtualMachine->m_ee->m_gs->SetPresentationParams(presentationParams);
	}

	{
		g_virtualMachine->CreatePadHandler(CPH_GenericInput::GetFactoryFunction());
		auto padHandler = static_cast<CPH_GenericInput*>(g_virtualMachine->GetPadHandler());
		auto& bindingManager = padHandler->GetBindingManager();

		g_inputProvider = std::make_shared<CInputProviderEmscripten>();
		bindingManager.RegisterInputProvider(g_inputProvider);

		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::START, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_ENTER));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::SELECT, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_BACKSPACE));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::DPAD_LEFT, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_ARROW_LEFT));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::DPAD_RIGHT, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_ARROW_RIGHT));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::DPAD_UP, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_ARROW_UP));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::DPAD_DOWN, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_ARROW_DOWN));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::SQUARE, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_A));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::CROSS, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_Z));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::TRIANGLE, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_S));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::CIRCLE, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_X));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::L1, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_1));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::L2, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_2));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::L3, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_3));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::R1, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_8));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::R2, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_9));
		bindingManager.SetSimpleBinding(0, PS2::CControllerInfo::R3, CInputProviderEmscripten::MakeBindingTarget(DOM_PK_0));

		bindingManager.SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_LEFT_X,
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_F),
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_H));
		bindingManager.SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_LEFT_Y,
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_T),
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_G));

		bindingManager.SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_RIGHT_X,
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_J),
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_L));
		bindingManager.SetSimulatedAxisBinding(0, PS2::CControllerInfo::ANALOG_RIGHT_Y,
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_I),
		                                       CInputProviderEmscripten::MakeBindingTarget(DOM_PK_K));
	}

	{
		g_soundHandler = new CSH_OpenAL();
		g_virtualMachine->CreateSoundHandler(CSH_OpenALProxy::GetFactoryFunction(g_soundHandler));
	}

	g_gsNewFrameConnection = g_virtualMachine->GetGSHandler()->OnNewFrame.Connect(std::bind(&CStatsManager::OnGsNewFrame, &CStatsManager::GetInstance(), std::placeholders::_1));

	EMSCRIPTEN_RESULT result = EMSCRIPTEN_RESULT_SUCCESS;

	result = emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, &keyboardCallback);
	assert(result == EMSCRIPTEN_RESULT_SUCCESS);

	result = emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, true, &keyboardCallback);
	assert(result == EMSCRIPTEN_RESULT_SUCCESS);
}

void bootElf(std::string path)
{
	g_virtualMachine->BootElf(path);
}

void bootDiscImage(std::string path)
{
	g_virtualMachine->BootDiscImage(path);
}

int getFrames()
{
	return CStatsManager::GetInstance().GetFrames();
}

void clearStats()
{
	CStatsManager::GetInstance().ClearStats();
}

EMSCRIPTEN_BINDINGS(Play)
{
	using namespace emscripten;

	function("bootElf", &bootElf);
	function("bootDiscImage", &bootDiscImage);
	function("getFrames", &getFrames);
	function("clearStats", &clearStats);
}
