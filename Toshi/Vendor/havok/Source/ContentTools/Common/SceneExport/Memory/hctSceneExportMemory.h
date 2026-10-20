/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SCENE_EXPORT_MEMORY_H
#define HK_SCENE_EXPORT_MEMORY_H

	/// Functions in this namespace are used to initialize thread memory as well as to retrieve information
	/// about singletons.
namespace hkSceneExportMemory
{
		///	Initialized Havok's base system using thread-safe memory
	void HK_CALL threadSafeBaseSystemInit(bool debugMem = false);

		/// Quits Havok's bases system
	void HK_CALL threadSafeBaseSystemQuit();

		/// Retrieves information about the singletons used by this module (DLL or EXE)
	void HK_CALL getBaseSystemInfo( hctBaseDll::BaseSystemInfo& baseSystemInfo );
}

#endif //HK_SCENE_EXPORT_MEMORY_H

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
