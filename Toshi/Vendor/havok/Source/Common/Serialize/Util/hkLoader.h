/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef LOADER_H
#define LOADER_H

#include <Common/Serialize/Packfile/hkPackfileData.h>

class hkTypeInfoRegistry;
class hkClassNameRegistry;
class hkVersionRegistry;

/// Utility class to load and manage packfiles.
class hkLoader : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE);

			/// Releases the references to all loaded memory
		~hkLoader();

			/// Loads from a file, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// Implicitly uses the built-in registry
		class hkRootLevelContainer* load( const char* filename );

			/// Loads from a stream, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// Implicitly uses the built-in registry
		class hkRootLevelContainer* load( class hkStreamReader* reader );

			/// Loads from a file, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// The hkVersionRegistry may be null which disables versioning.
			/// Uses the registries passed explicitly.
		class hkRootLevelContainer* load( const char* filename, hkTypeInfoRegistry* finish, hkVersionRegistry* version );

			/// Loads from a stream, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// The hkVersionRegistry may be null which disables versioning.
			/// Uses the registries passed explicitly.
		class hkRootLevelContainer* load( class hkStreamReader* reader, hkTypeInfoRegistry* finish, hkVersionRegistry* version );

			/// Loads from a file, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// Does not assume that the hkRoootLevelContainer is the top level class.
			/// Implicitly uses the built-in registry
		void* load( const char* filename, const hkClass& expectedTopLevelClass );

			/// Loads from a stream, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// Does not assume that the hkRoootLevelContainer is the top level class.
			/// Implicitly uses the built-in registry
		void* load( class hkStreamReader* reader, const hkClass& expectedTopLevelClass );

			/// Loads from a file, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// Does not assume that the hkRoootLevelContainer is the top level class.
			/// The hkVersionRegistry may be null which disables versioning.
			/// Uses the registries passed explicitly.
		void* load( const char* filename, const hkClass& expectedTopLevelClass, hkTypeInfoRegistry* finish, hkVersionRegistry* version );

			/// Loads from a stream, Explicitly allocates and keeps a handle to the memory allocated 
			/// This method is usually called multiple times to load several different assets.
			/// Does not assume that the hkRoootLevelContainer is the top level class.
			/// The hkVersionRegistry may be null which disables versioning.
			/// Uses the registries passed explicitly.
		void* load( class hkStreamReader* reader, const hkClass& expectedTopLevelClass, hkTypeInfoRegistry* finish, hkVersionRegistry* version );


		virtual void calcContentStatistics( hkStatisticsCollector* collector, const hkClass* cls ) const;

	public:

		hkArray<hkPackfileData*> m_loadedData;
};

#endif // LOADER_H

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
