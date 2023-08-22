#include "pch.h"

// Predefining Toshi initialization settings
#define TOSHI_TMEMORY_SIZE 640 * 1024 * 1024
#define TOSHI_TMEMORY_FLAGS Toshi::TMemory::Flags_Standard

#define TOSHI_APP \
Toshi::TApplication& app = AApplication::g_oTheApp; \
app.Create("Jurassic Park: Operation Genesis - (c) Blue Tongue Software", 0, 0); \
app.Execute();


// Including the entrypoint
#include <Toshi.h>

#include <Toshi2/T2GUI/T2GUI.h>
#include <Toshi/Sound/TSound.h>
#include <Toshi/Render/TRender.h>
#include <Platform/Windows/TSound_Win.h>
#include "Toshi/Core/TSystem.h"
#include <Toshi/Core/TFreeList.h>
#include "Toshi/Plugins/PPropertyParser/PProperties.h"
#include "ARootTask.h"


#include TOSHI_MULTIRENDER(TRender)

TOSHI_NAMESPACE_USING

AApplication AApplication::g_oTheApp;

bool AApplication::OnCreate(int argc, char** argv)
{
	TSystem::CreateCStringPoolExplicit("data\\strpool.dat", 1024, 12000);
	TTODO("SetCapacities");
	Toshi::TSystem::GetLocale()->LoadInfo("Data/GUI/Locales/Locales.txt");
	Toshi::TScheduler* scheduler = new Toshi::TScheduler();
	m_pRootTask = scheduler->CreateTask(TGetClass(ARootTask));
	m_pRootTask->Activate(TTRUE);

	TApplication::OnCreate(argc, argv);
	return true;
}

bool AApplication::OnUpdate(float deltaTime)
{
	return true;
}