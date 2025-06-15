#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE  640 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

#define TOSHI_APP                                                \
	Toshi::TApplication& app = AApplication::g_oTheApp;          \
	app.Create("NickToons 06 - (c) Blue Tongue Software", 0, 0); \
	app.Execute();


// Including the entrypoint
#include <Toshi.h>

#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/TRender_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/TRender_SDL.h"
#endif // TOSHI_RENDERER_OPENGL

AApplication AApplication::g_oTheApp;

bool AApplication::OnCreate(int argc, char** argv)
{
	ParseCommandLineParams(argc, argv);
	InitialiseFreeList();
	AMemory::CreatePool(AMemory::POOL_FrequentAllocations);
	AMemory::CreatePool(AMemory::POOL_XUI);

	Toshi::T2String8::sm_pDefaultAllocatorCB  = AppStringAllocator();
	Toshi::T2String16::sm_pDefaultAllocatorCB = AppStringAllocator();

	m_Renderer             = ARenderer::GetSingletonSafe();
	TBOOL interfaceCreated = m_Renderer->CreateInterface();

	TApplication::OnCreate(argc, argv);
	return true;
}

bool AApplication::OnUpdate(float deltaTime)
{
	return true;
}

void AApplication::ParseCommandLineParams(int argc, char** argv)
{
	TIMPLEMENT();
}

void AApplication::InitialiseFreeList()
{
}
