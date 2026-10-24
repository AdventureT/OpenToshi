/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_VERSION_H
#define INC_VERSION_H

#define HCT_VERSION_MAJOR 6
#define HCT_VERSION_MINOR 5
#define HCT_VERSION_POINT 0
#define HCT_VERSION_BUILD 2
#define HCT_CURRENT_VERSION_STRING		"6.5.0.2 (6.5.0 RC1)"
#define HCT_CURRENT_VERSION_UNICODE		L"6.5.0.2 (6.5.0 RC1)"

#define HCT_MAKE_VERSION(major, minor ,point, build) ( ((major) << 24) | ((minor) << 16) | ((point)<<8) | (build) )
#define HCT_CURRENT_VERSION HCT_MAKE_VERSION( HCT_VERSION_MAJOR, HCT_VERSION_MINOR, HCT_VERSION_POINT, HCT_VERSION_BUILD )

#define HCT_IS_MAJOR_COMPATIBLE(thisVer, otherVer) ( (thisVer & ~0x0ffffff) == (otherVer & ~0x0ffffff) )
#define HCT_IS_MINOR_COMPATIBLE(thisVer, otherVer) ( (thisVer & ~0x0ffff) == (otherVer & ~0x0ffff) )
#define HCT_IS_POINT_COMPATIBLE(thisVer, otherVer) ( (thisVer & ~0x0ff) == (otherVer & ~0x0ff) )

#define ENVVAR_FILTER_ROOT "HAVOK_EXPORT_FILTERS_ROOT"

#endif // INC_VERSION_H


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
