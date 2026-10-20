/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkAabb::hkAabb(const hkVector4& min, const hkVector4& max)
	: m_min(min), m_max(max)
{
}

hkBool hkAabb::overlaps( const hkAabb& other ) const
{
	HK_ASSERT2(0x3f5578fc,  isValid(), "Invalid aabb in hkAabb::overlaps." );
	HK_ASSERT2(0x76dd800a,  other.isValid(), "Invalid aabb in hkAabb::overlaps.");
	hkVector4Comparison mincomp = m_min.compareLessThanEqual4(other.m_max);
	hkVector4Comparison maxcomp = other.m_min.compareLessThanEqual4(m_max);
	hkVector4Comparison both; both.setAnd( mincomp, maxcomp );
	return both.allAreSet(hkVector4Comparison::MASK_XYZ) != 0;
}

hkBool hkAabb::contains(const hkAabb& other) const
{
	hkVector4Comparison mincomp = m_min.compareLessThanEqual4(other.m_min);
	hkVector4Comparison maxcomp = other.m_max.compareLessThanEqual4(m_max);
	hkVector4Comparison both; both.setAnd( mincomp, maxcomp );
	return both.allAreSet(hkVector4Comparison::MASK_XYZ) != 0;
}

hkBool hkAabb::containsPoint(const hkVector4& other) const
{
	hkVector4Comparison mincomp = m_min.compareLessThanEqual4(other);
	hkVector4Comparison maxcomp = other.compareLessThanEqual4(m_max);
	hkVector4Comparison both; both.setAnd( mincomp, maxcomp );
	return both.allAreSet(hkVector4Comparison::MASK_XYZ) != 0;
}

hkBool hkAabb::isEmpty() const
{
	hkVector4Comparison comp = m_min.compareGreaterThanEqual4(m_max);
	return comp.anyIsSet(hkVector4Comparison::MASK_XYZ) != 0;
}

void hkAabb::includePoint (const hkVector4& point)
{
	m_min.setMin4(m_min, point);
	m_max.setMax4(m_max, point);
}

void hkAabb::includeAabb (const hkAabb& aabb)
{
	m_min.setMin4(m_min, aabb.m_min);
	m_max.setMax4(m_max, aabb.m_max);
}


void hkAabb::setEmpty()
{
	m_min.setAll(HK_REAL_MAX);
	m_max.setAll(-HK_REAL_MAX);
}

inline void hkAabbUint32::setInvalid()
{
	const int ma = 0x7fffffff;

	// SNC warning workaround
	hkUint32* minP = m_min;
	hkUint32* maxP = m_max;

	minP[0] = ma;
	minP[1] = ma;
	minP[2] = ma;
	minP[3] = 0;
	maxP[0] = 0;
	maxP[1] = 0;
	maxP[2] = 0;
	maxP[3] = 0;
}

inline void hkAabbUint32::setInvalidY()
{
	const int ma = 0x7fff0000;
	hkUint32* minP = m_min;
	hkUint32* maxP = m_max;

	minP[1] = ma;
	minP[2] = ma;
	minP[3] = 0;
	maxP[1] = 0;
	maxP[2] = 0;
}

void hkAabbUint32::operator=( const hkAabbUint32& other )
{
	hkString::memCpy16<sizeof(hkAabbUint32)>( this, &other );
}

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
