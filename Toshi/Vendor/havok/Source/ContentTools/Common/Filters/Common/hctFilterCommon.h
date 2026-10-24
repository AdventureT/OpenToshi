/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_FILTER_COMMON_H
#define INC_FILTER_COMMON_H

#ifndef _WIN32_WINNT		
#define _WIN32_WINNT 0x0500		// win2000
#endif						

#ifndef WINVER
#define WINVER 0x0500
#endif

#if (_MSC_VER >= 1400)
#	define _CRT_SECURE_NO_DEPRECATE 1
#	define _CRT_NONSTDC_NO_DEPRECATE 1
#endif

//#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <commctrl.h> 
#include <commdlg.h>
#include <locale.h>

#include <Common/Base/hkBase.h>
#include <Common/Base/Reflection/hkClass.h>

#include <Common/Base/hkBase.h>

#include <ContentTools/Common/Filters/Common/Memory/hctFilterMemoryTracker.h>
#include <ContentTools/Common/Filters/Common/Filter/hctFilterDll.h>
#include <ContentTools/Common/Filters/Common/Filter/hctFilterDescriptor.h>
#include <ContentTools/Common/Filters/Common/Filter/hctFilterInterface.h>
#include <ContentTools/Common/Filters/Common/Filter/Modeless/hctModelessFilterDll.h>
#include <ContentTools/Common/Filters/Common/Filter/Modeless/hctModelessFilterDescriptor.h>
#include <ContentTools/Common/Filters/Common/Filter/Modeless/hctModelessFilter.h>


#include <ContentTools/Common/Filters/Common/FilterManager/hctFilterManagerInterface.h>
#include <ContentTools/Common/Filters/Common/FilterManager/hctFilterManagerDll.h>

#include <ContentTools/Common/Filters/Common/ClassRegistry/hctFilterClassRegistry.h>

#include <ContentTools/Common/Filters/Common/Utils/hctFilterUtils.h>

#include <ContentTools/Common/Filters/Common/Utils/hctOptionsRegistry.h>
#include <ContentTools/Common/Filters/Common/Utils/hctTreeViewManager.h>


#include <Common/SceneData/Scene/hkxScene.h>

#include <Common/Serialize/Util/hkRootLevelContainer.h>

#include <ContentTools/Common/Filters/Common/version.h>

#include <ContentTools/Common/Filters/Common/Utils/hctFilterDialogUtils.h>

#endif //INC_FILTER_COMMON_H

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
