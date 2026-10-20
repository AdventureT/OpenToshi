/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_MODELESS_FILTER_DLL_H
#define HAVOK_MODELESS_FILTER_DLL_H

	/// Each filter DLL should contain and return (through a "getFilterDll" function) a single instance of this class. 
class hctModelessFilterDll : public hctFilterDll
{
	public:

			/// Constructor. Requires a Handle to the DLL (returned by LoadLibrary() and passed to getFilterDll()). 
		hctModelessFilterDll (HMODULE dllModule);

		/*
		** From hctFilterDll
		*/

			/// Override from hctFilterDll - it does nothing since DLLs with modeless filters cannot change the memory manager
			/// on the fly.
		/*virtual*/ void pushMemoryManager (hkMemory* newMemory);

			/// Override from hctFilterDll - it does nothing since DLLs with modeless filters cannot change the memory manager
			/// on the fly.
		/*virtual*/ void popMemoryManager ();

		/*
		** From hctBaseDll
		*/

			/// We have to handle errors a little different for modeless filters, hence the override
		/*virtual*/ void initDll (const BaseSystemInfo* baseSystemInfo );

	private:

		static void errorReportFunction(const char* buf, void *obj);

};

#endif // HAVOK_MODELESS_FILTER_DLL_H

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
