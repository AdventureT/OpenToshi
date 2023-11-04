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
		TD3DAdapter *adapter = TSTATICCAST(TD3DAdapter*, pAdapter.operator Toshi::TRenderAdapter *());
		TUtil::Log("Adapter %s", adapter->GetAdapterDesc()->Description);
	}
}