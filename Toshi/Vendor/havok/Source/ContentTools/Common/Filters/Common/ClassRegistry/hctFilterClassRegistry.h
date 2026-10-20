/* 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_REGISTRY__H
#define HAVOK_FILTER_REGISTRY__H

class hkTypeInfo;
class hkClass;
class hkVtableClassRegistry;
class hkTypeInfoRegistry;
class hkClassNameRegistry;

	/// An object of this class is passed to each filter DLL - each filter DLL registers the classes its filters create. 
	/// This is necessary since objects created in a particular DLL will have the VTable corresponding to that DLL - other DLLs need to be aware of the address
	/// of those VTables in order to recognize the class of objects.
	/// An hctFilterClassRegistry stores an hkVtableClassRegistry, an hkClassNameRegistry and an hkTypeInfoRegistry.
class hctFilterClassRegistry
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SERIALIZE, hctFilterClassRegistry);

			/// Constructor
		hctFilterClassRegistry();

			/// Destructor
		~hctFilterClassRegistry();

			/// Returns a const reference to the hkVtableClassRegistry object
		const hkVtableClassRegistry& getVtableRegistry() const;

			/// Returns a non-const (read/write) reference to the hkVtableClassRegistry object
		hkVtableClassRegistry& getVtableRegistry();

			/// Returns a const reference to the hkTypeInfoRegistry object
		const hkTypeInfoRegistry& getTypeInfoRegistry() const;

			/// Returns a non-const (read/write) reference to the hkTypeInfoRegistry object
		hkTypeInfoRegistry& getTypeInfoRegistry();

			/// Returns a const reference to the hkClassNameRegistry object
		const hkClassNameRegistry& getClassNameRegistry() const; 

			/// Returns a non-const (read/write) reference to the hkClassNameRegistry object
		hkClassNameRegistry& getClassNameRegistry(); 

			/// Given an instance of an object (NOTE: The object has to have a Vtable) it returns its class. 
		const hkClass* getClassFromVirtualInstance(void*) const;

			/// Registers a class
		void registerClass(const hkTypeInfo* ti, const hkClass* ci);

			/// Registers an array of classes
		void registerClasses(const hkTypeInfo* const * tis, const hkClass* const * cis);

			/// Merges another hctFilterClassRegistry object into this one
		void merge ( const hctFilterClassRegistry& copyFrom );

		struct Implementation;

	protected:

		Implementation* m_impl;
};

#endif // HAVOK_FILTER_REGISTRY__H

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
