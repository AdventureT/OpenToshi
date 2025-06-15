#pragma once
#include "ToshiPCH.h"
#include "TModel_SDL.h"
#include "TRender_SDL.h"

TOSHI_NAMESPACE_START

void* TModel::ResourceCallback(void* pData, TTRB* pTRB, TBOOL bCreated)
{
	TModel* pModel = TSTATICCAST(TModel, pData);

	if (bCreated)
	{
		auto pRender = TRenderSDL::Interface();
		pRender->WaitForEndOfRender();

		pModel->m_pTRB = pTRB;
		pModel->m_Flags.Unset(Flags::TrbLoaded);
		pModel->m_pCollisionData = TNULL;
		pModel->m_pSkeleton      = TNULL;
		pModel->m_bFreeOnUnload  = TTRUE;
		pModel->LoadTRB();

		return pModel;
	}

	return TNULL;
}

TBOOL TModelHAL::Create(TTMDWin::TTRBWinHeader* pTMDHeader)
{
	CreateResource(pTMDHeader->m_pTXSModelName);

	m_pTMDHeader = pTMDHeader;
	m_pName      = pTMDHeader->m_pTXSModelName;
	CreateSkeleton(pTMDHeader->m_pSkeletonHeader, pTMDHeader->m_pSkeleton, TTRUE);
	CreateCollision(pTMDHeader->m_pModelCollision);

	m_Flags.Set(Flags::Created | Flags::TrbLoaded);

	if (sm_pTRBLoadCallback2)
	{
		sm_pTRBLoadCallback2(this, pTMDHeader);
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
