/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_DLL_H
#define HAVOK_FILTER_DLL_H

#include <ContentTools/Common/Filters/Common/hctBaseDll.h>

class hctFilterDll;
class hctFilterDescriptor;

	/// Prototype for the DLL-exported method (implemented by each filter DLL)
typedef hctFilterDll* (__cdecl *hkGetFilterDLLFunc) (HMODULE dllModule);

	/// Each filter DLL should contain and return (through a "getFilterDll" function) a single instance of this class. 
	/// The class extends the functionality in hctBaseDll by providing information about how many filters are implemented in the DLL.
	/// It also provides functionality to switch memory managers during filter processing.
	/// In general, if you are implementing the most common case where filters are modal, you only need to override getNumberOfFilters()
	/// and getFilterDescriptor().
class hctFilterDll : public hctBaseDll
{
	public:

			/// Constructor. Requires a Handle to the DLL (returned by LoadLibrary() and passed to getFilterDll()). 
		hctFilterDll (HMODULE dllModule);

			/// Must return the number of filters described in this DLL.
		virtual int getNumberOfFilters() const = 0;

			/// Must return the descriptor for the i-th filter in this DLL.
		virtual hctFilterDescriptor* getFilterDescriptor (int index) const = 0;

			/// Should register any classes and types created by filters in this DLL
		virtual void registerClasses (class hctFilterClassRegistry& classReg) const = 0;

			/// Sets the singleton serialization registries for this DLL
		virtual void setSingletonRegistries (const class hctFilterClassRegistry& classReg) const;

			/// Temporarily replaces the DLL memory manager with the given one.
			/// It can be overriden to do nothing if replacing the memory manager is not possible; for example,
			/// if the filter is non-modal.
		virtual void pushMemoryManager (hkMemory* newMemory);

			/// Restores the original memory manager after a call to pushMemoryManager()
			/// It can be overriden to do nothing if replacing the memory manager is not possible; for example,
			/// if the filter is non-modal.
		virtual void popMemoryManager ();

	protected:

		hkMemory* m_previousMemoryManager;

};

#endif // HAVOK_FILTER_DLL_H

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
