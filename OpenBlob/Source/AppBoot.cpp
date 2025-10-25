#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE  640 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

// Including the entrypoint
#include <Toshi.h>

// Including everything else
#include "AAssetStreaming.h"
#include "AExampleClass.h"
#include "ALevelInformation.h"
#include "GameInterface/AAppInitState.h"
#include "Movie/AMoviePlayer.h"
#include "Locale/ALocaleManager.h"
#include "Input/AInputManager2.h"
#include "Console/AConsoleVar.h"
#include "Cameras/ACameraManager.h"
#include "Sound/AXUIFMODExAudio.h"

#include <Toshi/Xui/TXUI.h>
#include <Toshi/Core/TArray.h>
#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi2/T2SimpleArray.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include <Platform/Windows/TSound_Win.h>
#include <Toshi/Input/TInputInterface.h>
#include <GameInterface/AXUIState.h>

#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/A2GUI/A2GUIRenderer_DX11.h"
#  include "Platform/DX11/TRender_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/A2GUI/A2GUIRenderer_SDL.h"
#  include "Platform/SDL/TRender_SDL.h"
#endif // TOSHI_RENDERER_OPENGL

AApplication AApplication::g_oTheApp;

CVAR_CREATE(testcvar, TFALSE)

TBOOL AApplication::OnCreate(TINT argc, TCHAR** argv)
{
	TOSHI_INFO("Starting Blob...");
	AMemory::CreatePools();

	ARenderer::CreateSingleton();
	AAssetStreaming::CreateSingleton();
	ALocaleManager::Create();

	m_Renderer             = ARenderer::GetSingletonSafe();
	TBOOL interfaceCreated = m_Renderer->CreateInterface();

	TOSHI_INFO("testcvar value: {0}", CVAR_GET_BOOL(testcvar));
	//TASSERT(TFALSE, "Assertion test");

	/*Toshi::T2SimpleArray<TINT> simpleArray;
	simpleArray.Create(4);

	simpleArray[0] = 1;
	simpleArray[1] = 2;
	simpleArray[2] = 3;
	simpleArray[3] = 4;*/

	Toshi::TArray<TINT>::Storage genArray;
	genArray.Push(5);
	genArray.Push(9);

	Toshi::TArray<TINT> arr(genArray);

	for (TINT i = 0; i < genArray.GetNumElements(); i++)
	{
		TOSHI_INFO("{0}", genArray[i]);
	}

	if (interfaceCreated)
	{
		auto pGUIRenderer = new A2GUIRenderer;
		auto pGUI         = Toshi::T2GUI::Open(AMemory::ms_apMemoryBlocks[AMemory::POOL_FrequentAllocations]);
		pGUI->SetRenderer(pGUIRenderer);

		AInputManager2::CreateSingleton();

		// Temp solution
		Toshi::TXUI::ms_pXUIMemoryBlock    = AMemory::GetPool(AMemory::POOL_XUI);
		Toshi::TXUI::ms_pXUITRBMemoryBlock = AMemory::GetPool(AMemory::POOL_XUI);
		auto txui                          = Toshi::TXUI::CreateSingleton();

		const size_t poolSize = 128 * 1024 * 1024;
		void*        mempool  = Toshi::TMemory::GetGlobalHeap()->Memalign(0x20, poolSize);

		TBOOL bResult = Toshi::TSound::CreateSingleton()->Create(mempool, poolSize, -1, -1, Toshi::TSound::SpeakerType_7POINT1);
		TASSERT(TTRUE == bResult);

		FMOD::System* system = Toshi::TSound::GetSingleton()->GetSystem();
		system->setFileSystem(NULL, NULL, NULL, NULL, NULL, NULL, 0);

		// Doens't Load

		Toshi::TMemoryHeap* oldHeap = AXUIState::SetFontMemBlock(AMemory::GetPool(AMemory::POOL_XUI));

		//AXUIState::SetSkin1("commonskin.trb", "commonskin.xur");
		AXUIState::SetSkin2("frontendskin.trb", "frontendskin.xur");
		TIMPLEMENT_D("Font according to steam language");
		AXUIState::InitFont("data/font/blob.font.trb");
		AXUIState::SetCommonSkin("common.trb");
		TIMPLEMENT_D("Implement rest of XUI Classes")
		//AXUIState::SetSkinWithFont("frontend.trb", TFALSE);

		txui->m_pAudio = new AXUIFMODExAudio();
		AXUIState::SetFontMemBlock(oldHeap);

		//Toshi::TInputInterface::CreateSingleton();

		m_Renderer->Create();
		AGameState::SetupLoadIcon();
		SetRenderWorld(TTRUE);

		m_pGameStateController = AGameStateController::CreateSingleton();
		ACameraManager::CreateSingleton();

		m_pGameStateController->Create();
		m_pGameStateController->PushState(new AAppInitState);

		TApplication::OnCreate(argc, argv);
		//ALevelInformation info = ALevelInformation();
		//info.Create("C:\\Program Files (x86)\\Steam\\steamapps\\common\\de Blob\\Data\\Levels.trb");

		AExampleClass* exampleClass = new AExampleClass();
		exampleClass->Delete();
	}

	return TTRUE;
}

TBOOL AApplication::OnUpdate(TFLOAT deltaTime)
{
	UpdateSound(deltaTime);

	Toshi::T2GUI*   pGUI          = Toshi::T2GUI::GetSingleton();
	AMoviePlayer*   pMoviePlayer  = AMoviePlayer::GetSingleton();
	AInputManager2* pInputManager = AInputManager2::GetSingleton();

	if (pGUI) pGUI->Tick(deltaTime);
	if (pInputManager) pInputManager->Update(deltaTime);
	if (pMoviePlayer) pMoviePlayer->OnUpdate(deltaTime);

	m_pGameStateController->Update(deltaTime);
	ARenderer::GetSingleton()->Update(deltaTime);
	return TTRUE;
}

TBOOL AApplication::UpdateSound(TFLOAT deltaTime)
{
	Toshi::TSound* sound = Toshi::TSound::GetSingletonSafe();
	sound->Update();
	return TFALSE;
}
