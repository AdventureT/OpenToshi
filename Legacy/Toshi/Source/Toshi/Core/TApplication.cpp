#include "ToshiPCH.h"
#include "TApplication.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Strings/TString8.h"
#include "Toshi/Core/TSystem.h"
#include "Toshi/File/TFile.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006ef060
TApplication::TApplication()
{
	m_Flags = 0;
}

// $deBlob: FUNCTION 006ef560
TApplication::~TApplication()
{
	Destroy();
}

// $deBlob: FUNCTION 006ef1a0
TBOOL TApplication::Create(const TCHAR* appName, TINT argc, TCHAR** argv)
{
	m_oExitEvent.Connect(this, OnApplicationExitEvent);
	m_Name = appName;

	m_pDebugConsole = new TDebugConsole;

#ifdef TOSHI_DEBUG
	if (IsConsoleEnabled()) m_pDebugConsole->Show(TTRUE);
#else
	m_pDebugConsole->Show(TFALSE);
#endif

	return OnCreate(argc, argv);
}

// $deBlob: FUNCTION 006ef210
void TApplication::Destroy()
{
	m_Flags |= TApplicationFlag_Destroyed;
}

// $deBlob: FUNCTION 006ef100
TBOOL TApplication::Execute()
{
	TASSERT(TApplication::IsCreated() == TTRUE);
	TSystemManager* pSystemManager = TSystemManager::GetSingletonSafe();

	TBOOL updateResult = TTRUE;
	while (updateResult && !IsDestroyed())
	{
		pSystemManager->Update();
		updateResult = OnUpdate(pSystemManager->GetTimer()->GetDelta());
	}

	return OnDestroy();
}

// $deBlob: FUNCTION 006ef230
TBOOL TApplication::OnCreate(TINT argc, TCHAR** argv)
{
	m_Flags |= TApplicationFlag_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 006ef240
TBOOL TApplication::OnUpdate(TFLOAT deltaTime)
{
	return (m_Flags & TApplicationFlag_Destroyed) == 0;
}

// $deBlob: FUNCTION 006ef250
TBOOL TApplication::OnDestroy()
{
	m_oExitEvent.Disconnect();
	return TTRUE;
}

TOSHI_NAMESPACE_END
