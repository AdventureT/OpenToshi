/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

HK_FORCE_INLINE const char* hkStringPtr::val() const
{
	return reinterpret_cast<char*>(hkUlong(m_string) & ~FLAGS_MASK);
}

HK_FORCE_INLINE hkStringPtr& hkStringPtr::operator=(const char* string)
{
	HK_ASSERT(0x6752a072, (hkUlong(string) & HEAP_FLAG) == 0);
	if( string == val() )
	{
		return *this;
	}
	if( (hkUlong(m_string) & HEAP_FLAG) == HEAP_FLAG )
	{
		hkDeallocate<char>(reinterpret_cast<char*>(hkUlong(m_string) & ~FLAGS_MASK));
	}
	if( string )
	{
		m_string = reinterpret_cast<char*>(hkUlong(hkString::strDup(string)) | HEAP_FLAG);
	}
	else
	{
		m_string = HK_NULL;
	}
	return *this;
}

HK_FORCE_INLINE const char& hkStringPtr::operator[](int idx) const
{
	const char* strAddr = val();
	HK_ASSERT(0x2216fadd, strAddr);
	HK_ASSERT(0x73c986fd, 0 <= idx && idx <= hkString::strLen(strAddr));
	return strAddr[idx];
}

HK_FORCE_INLINE hkStringPtr::operator const char*() const
{
	return val();
}

HK_FORCE_INLINE hkStringPtr::hkStringPtr() : m_string(HK_NULL)
{
}

HK_FORCE_INLINE hkStringPtr::hkStringPtr(const char* string)
{
	HK_ASSERT(0x4a778cb2, (hkUlong(string) & HEAP_FLAG) == 0);
	if( string )
	{
		m_string = reinterpret_cast<char*>(hkUlong(hkString::strDup(string)) | HEAP_FLAG);
	}
	else
	{
		m_string = HK_NULL;
	}
}

HK_FORCE_INLINE hkStringPtr::hkStringPtr(const hkStringPtr& strRef)
{
	const char* string = strRef;
	HK_ASSERT(0x63bd6f3a, (hkUlong(string) & HEAP_FLAG) == 0);
	if( string )
	{
		m_string = reinterpret_cast<char*>(hkUlong(hkString::strDup(string)) | HEAP_FLAG);
	}
	else
	{
		m_string = HK_NULL;
	}
}

HK_FORCE_INLINE hkStringPtr::hkStringPtr(hkFinishLoadedObjectFlag f)
{
}


/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
