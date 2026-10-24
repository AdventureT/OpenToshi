/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_HKBASESYSTEM_H
#define HKBASE_HKBASESYSTEM_H

/// An error report function to be passed to Havok when initializing it subsystems. In particular
typedef void (HK_CALL *hkErrorReportFunction)(const char* s, void* errorReportObject);

struct hkHardwareInfo
{
	int m_numThreads;
};


// Get information about the hardware configuration
void HK_CALL hkGetHardwareInfo( hkHardwareInfo& info );


/// This class is used to initialize the Havok subsystems (memory, singleton, etc).
namespace hkBaseSystem
{
		/// Initializes Havok's subsystems.<br>
		/// Havok provides a number of memory managers, please consult hkbase/memory/impl for more details.
		/// On error, the errorReportFunction is called with an error string and the errorReportObject pointer.
		/// init() sets in internal flag, so it may be called more than once but multiple init() and quit() calls
		/// do not nest.
		/// \param threadMemory: this is an optional smart memory manager which
		///                      tries to cache memory blocks so we can reduce locks
	hkResult HK_CALL init( hkMemory* memoryManager, 
									hkThreadMemory* threadMemory,
									hkErrorReportFunction errorReportFunction,										
									void* errorReportObject = HK_NULL );

		/// Call for each active thread
	hkResult HK_CALL initThread( hkThreadMemory* threadMemory );

		/// Quit thread local storage
	hkResult HK_CALL clearThreadResources(  );

		/// Quit the subsystems. It is safe to call multiple times.
		/// It cleans up in reverse order to init: default singletons, hkError,
		/// hkFileSystem and hkMemory
	hkResult HK_CALL quit();

		/// Checks if the system has already been initialized.
	hkBool HK_CALL isInitialized();

		/// Utility function to initialize all singletons, called from init().
		/// You do not need to call this method unless you have completely replaced
		/// the init method.
		/// All global non plain old data (POD) types are encapsulated in
		/// singletons. At program startup, static constructors put all these
		/// singletons and functions to create them into a linked list
		/// (see hkSingleton.h). At init time, we can then traverse the list,
		/// setting up singletons which have not been explicitly set up already.
	void HK_CALL initSingletons();

		/// Utility function to quit all singletons, called from quit().
		/// You do not need to call this method unless you have completely replaced
		/// the quit method.
	void HK_CALL quitSingletons();

}

#endif // HKBASE_HKBASESYSTEM_H

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
