#include "pch.h"
#include "ADisplayMode_Win.h"

#ifdef TOSHI_RENDERER_DX11
#  include "Platform/DX11/TRender_DX11.h"
#elif defined(TOSHI_RENDERER_OPENGL) // TOSHI_RENDERER_DX11
#  include "Platform/SDL/TRender_SDL.h"
#endif // TOSHI_RENDERER_OPENGL

TOSHI_NAMESPACE_USING

void ADisplayMode_Win::Initialise()
{
	TRender* pRenderer    = TRender::GetSingleton();
	auto     pAdapterList = pRenderer->GetAdapterList();

	TUtil::Log("Enum Display Modes");
	TUtil::LogUp();

	for (auto pAdapter = pAdapterList->Begin(); pAdapter != pAdapterList->End(); pAdapter++)
	{
		TD3DAdapter* d3dadapter = pAdapter->As<TD3DAdapter>();
		TUtil::Log("Adapter %s", d3dadapter->GetDescription());
		TUtil::LogUp();

		auto  pModeList = d3dadapter->GetModeList();
		TBOOL firstMode = true;
		for (auto pMode = pModeList->Begin(); pMode != pModeList->End(); pMode++)
		{
			TD3DAdapter::Mode* mode     = pMode->As<TD3DAdapter::Mode>();
			TUINT32            uiWidth  = pMode->GetWidth();
			TUINT32            uiHeight = pMode->GetHeight();

			TIMPLEMENT();
			return;
		}
		TUtil::LogDown();
	}

	TUtil::LogDown();
}
