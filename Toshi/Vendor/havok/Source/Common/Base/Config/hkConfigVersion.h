/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
// Build configuration settings
// These are compile time settings for the libraries.
// Changing these values with compiled libs will not work,

//
// Build number and version number
// The version number must match the version of the libraries
// but the build number is for informative reasons only as multiple
// builds will work with each other but they must have the same
// version number, which is of the form XX.YY.ZZ stored as XXYYZZ
// and the build number is the date of release in the form
// YEAR-MONTH-DAY stored YEARMNDY
//

#ifndef HK_BASE_CONFIGVERSION_H
#define HK_BASE_CONFIGVERSION_H

#define HAVOK_SDK_VERSION_MAJOR 6
#define HAVOK_SDK_VERSION_MINOR 5
#define HAVOK_SDK_VERSION_POINT 0
// release id must be less than 3 chars to fit in binary packfile header.
// Use the following ids:
// alpha -> "aN", beta -> "bN", release -> "rN" and internal -> "$$"
#define HAVOK_SDK_VERSION_RELEASE "r1"

#define HAVOK_SDK_VERSION_NUMBER ((10000*HAVOK_SDK_VERSION_MAJOR) + (100*HAVOK_SDK_VERSION_MINOR) + (HAVOK_SDK_VERSION_POINT))

// HAVOK version string
// temporary definitions
#define HK_STRING0 "0"
#define HK_STRING1 "1"
#define HK_STRING2 "2"
#define HK_STRING3 "3"
#define HK_STRING4 "4"
#define HK_STRING5 "5"
#define HK_STRING6 "6"
#define HK_STRING7 "7"
#define HK_STRING8 "8"
#define HK_STRING9 "9"
#define HK_NUM_TO_STR(NUM) HK_STRING##NUM
#define HAVOK_SDK_VERSION_STRING_SPEC(MAJOR,MINOR,POINT,RELEASE) "Havok-" HK_NUM_TO_STR(MAJOR) "." HK_NUM_TO_STR(MINOR) "." HK_NUM_TO_STR(POINT) "-" RELEASE
// define HAVOK version string
#define HAVOK_SDK_VERSION_STRING HAVOK_SDK_VERSION_STRING_SPEC(HAVOK_SDK_VERSION_MAJOR,HAVOK_SDK_VERSION_MINOR,HAVOK_SDK_VERSION_POINT,HAVOK_SDK_VERSION_RELEASE)

#define HAVOK_BUILD_NUMBER 20090216

#endif // HK_BASE_CONFIGVERSION_H

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
