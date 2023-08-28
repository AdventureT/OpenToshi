#include "pch.h"
#include "ARootTask.h"
#include "Platform/Windows/DX11/TRender_DX11.h"

TOSHI_NAMESPACE_USING

ARootTask::ARootTask()
{

}

bool ARootTask::OnUpdate(float deltaTime)
{
	return true;
}


bool ARootTask::OnCreate()
{
	TIMPLEMENT_D("APrim::InitialiseAllPrimStatics()");
	TIMPLEMENT_D("AOptionLogic::AOptionLogic()");
	TIMPLEMENT_D("AOptionLogic::EnumerateModes()");
	m_oOptions.LoadOptions();
	TIMPLEMENT_D("DeserialiseOptions()");
	TIMPLEMENT_D("SerialiseOptions()");
	TIMPLEMENT_D("InitialiseConstants()");
	if (!CreateRenderInterface())
	{
		TIMPLEMENT_D("~AOptionLogic()");
		return TFALSE;
	}
	CreateARenderer();

	return TTask::OnCreate();
}

bool ARootTask::CreateRenderInterface()
{
	int screenWidth, screenHeight, screenDepth, screenBestZ, screenWindowed;
	m_oOptions.GetOption("ScreenWidth", screenWidth);
	m_oOptions.GetOption("ScreenHeight", screenHeight);
	m_oOptions.GetOption("ScreenDepth", screenDepth);
	m_oOptions.GetOption("ScreenBestZ", screenBestZ);
	m_oOptions.GetOption("ScreenWindowed", screenWindowed);

    auto renderer = new TRenderDX11();
    renderer->Create();

    TRender::DisplayParams displayParams;

    renderer->BuildAdapterDatabase();
    auto adapterList = renderer->GetAdapterList();

    auto adapter = adapterList->Head()->As<TD3DAdapter>();
    auto mode = adapter->GetMode();

    int width = 1280;
    int height = 720;


	displayParams.Unk3 = 32;
	displayParams.Unk4 = 3;
	displayParams.Unk5 = TTRUE;
	displayParams.IsFullscreen = TFALSE;
	displayParams.MultisampleQualityLevel = 1;
	displayParams.Width = width;
	displayParams.Height = height;
	renderer->CreateDisplay(&displayParams);

	return false;
}
