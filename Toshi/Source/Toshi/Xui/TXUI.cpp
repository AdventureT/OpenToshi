#include "ToshiPCH.h"
#include "TXUI.h"
#include "Toshi/Render/TAssetInit.h"

TOSHI_NAMESPACE_USING

TMemoryHeap* TXUI::ms_pXUIMemoryBlock = TNULL;
TMemoryHeap* TXUI::ms_pXUITRBMemoryBlock = TNULL;

TTRB::t_MemoryFuncAlloc TXUI::AssetTRBAllocator = [](TTRB::AllocType alloctype, size_t size, short unk1, size_t unk2, void* userData)
{
	return ms_pXUITRBMemoryBlock->Memalign(unk1, size);
};

TTRB::t_MemoryFuncDealloc TXUI::AssetTRBDeallocator = [](TTRB::AllocType alloctype, void* ptr, short unk1, size_t unk2, void* userData)
{
	return ms_pXUITRBMemoryBlock->Free(ptr);
};

TXUI::TXUI()
{
	m_pHeadTRBResource = TNULL;
	m_pDefaultFont = TNULL;
	m_sDefaultFont = TNULL;
	m_pRenderer = TNULL;
	m_pCanvas = TNULL;
	m_pContext = TNULL;
	m_pAudio = TNULL;
	m_pSkin1Resource = TNULL;
	m_pSkin2Resource = TNULL;

	m_FontTRB.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
	m_TRB2.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);
	m_TRB3.SetMemoryFunctions(AssetTRBAllocator, AssetTRBDeallocator, TNULL);

	auto pRender = TRender::GetSingleton();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();

	float fDisplayWidth = TSTATICCAST(float, pDisplayParams->Width);
	float fDisplayHeight = TSTATICCAST(float, pDisplayParams->Height);

	m_pContext = new (MemoryBlock()) T2GUIContext();
	m_pContext->GetRootElement()->SetDimensions(fDisplayWidth, fDisplayHeight);

	m_pCanvas = new (MemoryBlock()) TXUICanvas();
	m_pCanvas->SetDimensions(fDisplayWidth, fDisplayHeight);

	m_pContext->GetRootElement()->AddChildTail(m_pCanvas);

	m_pRenderer = new (MemoryBlock()) TXUIRenderer();
	m_Str[0] = '\0';

	TTODO("TXUIShapeCache, TGenericListener");
}

TXUIScene* TXUI::CreateScene(TXUIResource* a_pResource, uint32_t a_uiIndex)
{
	TXUIScene* pScene = a_pResource->CreateScene(a_uiIndex);
	if (!pScene) return TNULL;
	return pScene;
}

TXUIResource* TXUI::FindResource(const char* a_sName)
{
	TASSERT(T2String8::IsLowerCase(a_sName));

	auto pResource = m_pHeadTRBResource;

	while (pResource)
	{
		if (TStringManager::String8Compare(pResource->m_pFileName, a_sName) == 0)
		{
			return pResource->m_pResource;
		}

		pResource = pResource->m_pNext;
	}

	return TNULL;
}

void TXUI::AddResource(TXUIResourceTRB* a_pResourceTrb)
{
	a_pResourceTrb->m_pNext = m_pHeadTRBResource;
	a_pResourceTrb->m_pPrev = TNULL;

	if (m_pHeadTRBResource)
		m_pHeadTRBResource->m_pPrev = a_pResourceTrb;

	m_pHeadTRBResource = a_pResourceTrb;
}

void TXUI::RemoveResource(TXUIResourceTRB* a_pResourceTrb)
{
	if (a_pResourceTrb->m_pNext)
		a_pResourceTrb->m_pNext->m_pPrev = a_pResourceTrb->m_pPrev;

	if (a_pResourceTrb->m_pPrev)
		a_pResourceTrb->m_pPrev->m_pNext = a_pResourceTrb->m_pNext;

	if (a_pResourceTrb == m_pHeadTRBResource)
		m_pHeadTRBResource = m_pHeadTRBResource->m_pNext;
}

void TXUI::SetDefaultFont(const char* a_pData)
{
	m_FontTRB.Load(a_pData);
	TAssetInit::InitAssets(m_FontTRB, TTRUE, TFALSE);
}

void TXUI::SetSkin1(const char* a_szTRBFileName, const char* a_szXURFileName)
{
	if (m_pSkin1Resource)
	{
		TAssetInit::DeinitAssets(m_TRB2);
		m_TRB2.Close();
	}
	TTRB::ERROR error = m_TRB2.Load(a_szTRBFileName);
	TASSERT(error == TTRB::ERROR_OK);
	TAssetInit::InitAssets(m_TRB2, TTRUE, TFALSE);
	m_pSkin1Resource = FindResource(a_szXURFileName);
}

void TXUI::SetSkin2(const char* a_szTRBFileName, const char* a_szXURFileName)
{
	if (m_pSkin2Resource)
	{
		TAssetInit::DeinitAssets(m_TRB3);
		m_TRB3.Close();
	}
	TTRB::ERROR error = m_TRB3.Load(a_szTRBFileName);
	TASSERT(error == TTRB::ERROR_OK);
	TAssetInit::InitAssets(m_TRB3, TTRUE, TFALSE);
	m_pSkin2Resource = FindResource(a_szXURFileName);
}

void TXUIResourceTRB::Init()
{
	if (m_pResource == TNULL)
	{
		m_pResource = new (TXUI::MemoryBlock()) TXUIResource();
		TFIXME("Allocate memory chunk for the resource");
		TBOOL bRes = m_pResource->Load(m_pXURBuffer);
		TASSERT(TTRUE == bRes, "Could not load XUR");
		TXUI::GetSingletonSafe()->AddResource(this);
	}
}

void TXUIResourceTRB::Deinit()
{
	if (m_pResource)
		delete m_pResource;

	TXUI::GetSingletonSafe()->RemoveResource(this);
}