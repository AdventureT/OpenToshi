#include "ToshiPCH.h"
#include "T2Material_DX11.h"
#include "TTexture_DX11.h"
#include "TRender_DX11.h"
#include "Toshi/Core/TError.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006b1d00
void T2Material::Init()
{
	InitData();
	CreateResource(m_pMaterialName, this, ResourceCallback, this);
	TASSERT(!HasFlag(Flags_InMaterialManager));

	auto pMatManager = TMaterialManager::GetSingleton();
	auto pMetaMat    = pMatManager->FindMaterial(m_pMaterialName);

	if (pMetaMat == TNULL) pMatManager->AddMaterial(this);

	if (sm_pTRBLoadCallback) sm_pTRBLoadCallback(this, sm_pTRBLoadCallbackData);
}

// $deBlob: FUNCTION 006b1db0
void T2Material::Deinit()
{
	if (sm_pTRBUnloadCallback) sm_pTRBUnloadCallback(this, sm_pTRBUnloadCallbackData);

	auto pMatManager = TMaterialManager::GetSingleton();

	if (m_uiFlags & Flags_InMaterialManager)
	{
		m_uiFlags &= ~Flags_InMaterialManager;
		pMatManager->RemoveMaterial(this);
	}

	for (TINT i = 0; i < m_iNumTex; i++) m_aTextures[i].m_pTexture = TNULL;

	for (TINT i = 0; i < m_iNumPasses; i++) GetPassInfo(i)->m_pTexture->m_pTexture = TNULL;

	DestroyResource();
}

// $deBlob: FUNCTION 006b21a0
void T2Material::InitData()
{
	auto pTexManager = TTextureManager::GetSingleton();

	for (TINT i = 0; i < m_iNumTex; i++) m_aTextures[i].m_pTexture = pTexManager->FindTexture(m_aTextures[i].m_pName);

	for (TINT i = 0; i < m_iNumPasses; i++)
	{
		auto pPass    = GetPassInfo(i);
		auto pTexture = pTexManager->FindTexture(pPass->m_pTexture->m_pName);

		pPass->m_pTexture->m_pTexture = pTexture;

		if (pTexture == pTexManager->GetInvalidTexture()) TError::GetSingleton()->AddError("Material \'%s\' Couldn\'t find texture \'%s\'", m_pMaterialName, pPass->m_pTexture->m_pName);
	}
}

// $deBlob: FUNCTION 006b22d0
void* T2Material::ResourceCallback(void* pCustomData, TTRB* trb, TBOOL bCreated)
{
	if (!bCreated) return TNULL;

	TRenderDX11::Interface()->WaitForEndOfRender();

	auto pMat = TSTATICCAST(T2Material, trb->GetSymbolAddress("tmat"));

	if (pMat)
	{
		pMat->InitData();
		return pMat;
	}

	return pCustomData;
}

void TMaterialManager::AddMaterial(T2Material* a_pMat)
{
	if (a_pMat)
	{
		a_pMat->m_uiFlags |= T2Material::Flags_InMaterialManager;
		a_pMat->m_pNext = m_pHeadMaterial;
		a_pMat->m_pPrev = TNULL;

		if (m_pHeadMaterial) m_pHeadMaterial->m_pPrev = a_pMat;

		m_pHeadMaterial = a_pMat;
	}
}

// $deBlob: FUNCTION 006b2480
void TMaterialManager::RemoveMaterial(T2Material* a_pMat)
{
	TASSERT(TNULL != a_pMat);

	if (a_pMat->m_pNext) a_pMat->m_pNext->m_pPrev = a_pMat->m_pPrev;

	if (a_pMat->m_pPrev) a_pMat->m_pPrev->m_pNext = a_pMat->m_pNext;

	if (a_pMat == m_pHeadMaterial) m_pHeadMaterial = m_pHeadMaterial->m_pNext;

	a_pMat->m_pPrev = TNULL;
	a_pMat->m_pNext = TNULL;
}

// $deBlob: FUNCTION 006b23c0
T2Material* TMaterialManager::FindMaterial(const TCHAR* a_pName) const
{
	TASSERT(T2String8::IsLowerCase(a_pName));

	auto pMetaMat = m_pHeadMaterial;

	while (pMetaMat != TNULL)
	{
		if (T2String8::CompareNoCase(pMetaMat->m_pMaterialName, a_pName) == 0) return pMetaMat;

		pMetaMat = pMetaMat->m_pNext;
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
