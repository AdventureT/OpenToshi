#pragma once

TOSHI_NAMESPACE_START

namespace TTMDBase
{

struct SkeletonHeader
{
	const TCHAR* m_pTKLName;
	TINT         m_iTKeyCount;
	TINT         m_iQKeyCount;
	TINT         m_iSKeyCount;
	TINT         m_iTBaseIndex;
	TINT         m_iQBaseIndex;
	TINT         m_iSBaseIndex;
};

} // namespace TTMDBase

TOSHI_NAMESPACE_END
