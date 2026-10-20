/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_STRINGMAPBASE_H
#define HKBASE_STRINGMAPBASE_H

#include <Common/Base/Container/StringMap/hkCachedHashMap.h>
struct hkStringMapOperations
{
	HK_FORCE_INLINE hkBool32 equal(hkUlong k1, hkUlong k2) const
	{
		return hkString::strCmp( (const char*)k1, (const char*)k2 ) == 0;
	}
	HK_FORCE_INLINE hkUlong hash(hkUlong key) const
	{
		const char* p = (const char*)key;
		hkUlong h = 0;
		for( int i = 0; p[i] != HK_NULL; ++i )
		{
			h = 31 * h + p[i];
		}
		return h & (hkUlong(-1)>>1); // reserve -1 for empty
	}
};

// A class to map strings to pointers/pointer size integers.
class hkStringMapBase : public hkCachedHashMap<hkStringMapOperations>
{
};

#endif // HKBASE_STRINGMAPBASE_H

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
