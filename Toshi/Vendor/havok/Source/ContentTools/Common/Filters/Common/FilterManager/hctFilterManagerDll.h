/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_MANAGER_DLL_H
#define HAVOK_FILTER_MANAGER_DLL_H

#include <ContentTools/Common/Filters/Common/hctBaseDll.h>

class hctFilterManagerDll;
class hctFilterManagerInterface;
class hkTypeInfo;
class hkClass;

	/// Prototype for the DLL-exported method
typedef hctFilterManagerDll* (__cdecl *hkGetFilterManagerDllFunc) (HMODULE dllModule);

	/// The DLL implementing the filter manager should contain an instance of this class and expose it through the DLL via
	/// a getFilterManagerDLL() exported function (see hkGetFilterManagerDllFunc for the prototype).
	/// This class inherits from hctBaseDll, extending it with methods to create and destroy a filter manager.
class hctFilterManagerDll : public hctBaseDll
{
	public:

			/// Constructor. Requires a Handle to the DLL (returned by LoadLibrary() and passed to getFilterManagerDll());
		hctFilterManagerDll (HMODULE dllModule) : hctBaseDll (dllModule) {}

			/// Constructs and return an instance of hctFilterManagerInterface.
		virtual hctFilterManagerInterface* createFilterManager () = 0;

			/// Registers an array of classes.
		virtual void registerClasses(hctFilterManagerInterface* f, const hkTypeInfo* const* tis, const hkClass* const* cis) = 0;

			/// Destroys the given filter manager.
		virtual void deleteFilterManager (hctFilterManagerInterface* f) = 0;

};

#endif // HAVOK_FILTER_MANAGER_DLL_H

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
