#pragma once
#include "TTMDBase.h"
#include "TSkeleton.h"
#include "TModelCollision.h"

TOSHI_NAMESPACE_START

namespace TTMDWin
{
struct TTRBWinHeader
{
	TCHAR*                    m_pTXSModelName;
	TINT                      m_iLODCount;
	TFLOAT                    m_fUnknown;
	TTMDBase::SkeletonHeader* m_pSkeletonHeader;
	TSkeleton*                m_pSkeleton;
	TModelCollision*          m_pModelCollision;
};
}; // namespace TTMDWin

TOSHI_NAMESPACE_END
