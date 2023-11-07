#include "pch.h"
#include "ADisplayMode_Win.h"
#include TOSHI_MULTIRENDER(TRender)

TOSHI_NAMESPACE_USING

void ADisplayMode_Win::Initialise()
{
	TRender *pRenderer = TRender::GetSingleton();
	auto pAdapterList = pRenderer->GetAdapterList();

	TUtil::Log("Enum Display Modes");
	TUtil::LogUp();

	for (auto pAdapter = pAdapterList->Begin(); pAdapter != pAdapterList->End(); pAdapter++)
	{
		TD3DAdapter* d3dadapter = pAdapter->As<TD3DAdapter>();
		TUtil::Log("Adapter %s", d3dadapter->GetDescription());
		TUtil::LogUp();

		auto pModeList = d3dadapter->GetModeList();
		bool firstMode = true;
		for (auto pMode = pModeList->Begin(); pMode != pModeList->End(); pMode++)
		{
			TD3DAdapter::Mode* mode = pMode->As<TD3DAdapter::Mode>();
			uint32_t uiWidth = pMode->GetWidth();
			uint32_t uiHeight = pMode->GetHeight();
			
			TIMPLEMENT();
			return;
		}
		TUtil::LogDown();
	}

	TUtil::LogDown();
}