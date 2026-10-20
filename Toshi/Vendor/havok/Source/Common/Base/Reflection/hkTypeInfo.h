/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_BASE_TYPE_INFO_H
#define HK_BASE_TYPE_INFO_H

class hkClass;

class hkTypeInfo
{
	public:

		typedef void (HK_CALL *FinishLoadedObjectFunction)(void*, int);
		typedef void (HK_CALL *CleanupLoadedObjectFunction)(void*);

		hkTypeInfo( const char* name,
				FinishLoadedObjectFunction finish,
				CleanupLoadedObjectFunction cleanup,
				const void* vtable )
			:   m_typeName(name),
				m_finishLoadedObjectFunction(finish),
				m_cleanupLoadedObjectFunction(cleanup),
				m_vtable(vtable)
		{
		}

		const char* getTypeName() const
		{
			return m_typeName;
		}

		const void* getVtable() const
		{
			return m_vtable;
		}

		FinishLoadedObjectFunction getFinishFunction() const
		{
			return m_finishLoadedObjectFunction;
		}

		CleanupLoadedObjectFunction getCleanupFunction() const
		{
			return m_cleanupLoadedObjectFunction;
		}

	private:

		const char* m_typeName;
		FinishLoadedObjectFunction m_finishLoadedObjectFunction;
		CleanupLoadedObjectFunction m_cleanupLoadedObjectFunction;
		const void* m_vtable;
};

//
// Internal definitions, used by public macros later in the file.
//

#define HK_REFLECTION_DEFINE_COMMON( TEXT_CLASS, SCOPED_CPP_CLASS, FINISH, CLEANUP, VTABLE ) \
	const hkClass& HK_CALL SCOPED_CPP_CLASS::staticClass() { return TEXT_CLASS##Class; } \
	extern const hkTypeInfo TEXT_CLASS##TypeInfo; \
	const hkTypeInfo TEXT_CLASS##TypeInfo( #TEXT_CLASS, FINISH, CLEANUP, VTABLE )

#define HK_REFLECTION_DEFINE_INTERNAL_FINISH( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	static void HK_CALL finishLoadedObject##TEXT_CLASS(void* p, int finishing = 1) \
	{ \
		hkFinishLoadedObjectFlag f; f.m_finishing = finishing; \
		new (p) SCOPED_CPP_CLASS(f); \
	}

#define HK_REFLECTION_DEFINE_INTERNAL_CLEANUP( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	static void HK_CALL cleanupLoadedObject##TEXT_CLASS(void* p) \
	{ \
		static_cast<SCOPED_CPP_CLASS*>(p)->~CPP_CLASS(); \
	}

#define HK_REFLECTION_DEFINE_INTERNAL_VTABLE( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	static const void* HK_CALL getVtable##TEXT_CLASS() \
	{ \
		union { HK_ALIGN16(void* ptr); char buf[sizeof(SCOPED_CPP_CLASS)]; } u; \
		hkFinishLoadedObjectFlag f; \
		new (u.buf) SCOPED_CPP_CLASS(f); \
		return u.ptr; \
	}

#define HK_REFLECTION_DEFINE_INTERNAL_VIRTUAL( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_INTERNAL_FINISH( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_INTERNAL_CLEANUP( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_INTERNAL_VTABLE( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_COMMON( TEXT_CLASS, SCOPED_CPP_CLASS, finishLoadedObject##TEXT_CLASS, cleanupLoadedObject##TEXT_CLASS, getVtable##TEXT_CLASS() )

#define HK_REFLECTION_DEFINE_INTERNAL_NONVIRTUAL( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_INTERNAL_FINISH( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_INTERNAL_CLEANUP( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_COMMON( TEXT_CLASS, SCOPED_CPP_CLASS, finishLoadedObject##TEXT_CLASS, cleanupLoadedObject##TEXT_CLASS, HK_NULL )

#define HK_REFLECTION_DEFINE_INTERNAL_SIMPLE( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_INTERNAL_CLEANUP( TEXT_CLASS, SCOPED_CPP_CLASS, CPP_CLASS ) \
	HK_REFLECTION_DEFINE_COMMON( TEXT_CLASS, SCOPED_CPP_CLASS, HK_NULL, cleanupLoadedObject##TEXT_CLASS, HK_NULL )

//
// Public macro definitions
//

/// Define an hkTypeInfo for a class which is virtual.
#define HK_REFLECTION_DEFINE_VIRTUAL(KLASS) \
	HK_REFLECTION_DEFINE_INTERNAL_VIRTUAL( KLASS, KLASS, KLASS )
/// Define an hkTypeInfo for a class which is virtual.
#define HK_REFLECTION_DEFINE_SCOPED_VIRTUAL(SCOPE, KLASS) \
	HK_REFLECTION_DEFINE_INTERNAL_VIRTUAL( SCOPE##KLASS, SCOPE::KLASS, KLASS )

/// Define an hkTypeInfo for a non-virtual class. The vtable member will be set to null.
#define HK_REFLECTION_DEFINE_NONVIRTUAL(KLASS) \
	HK_REFLECTION_DEFINE_INTERNAL_NONVIRTUAL( KLASS, KLASS, KLASS )
/// Define an hkTypeInfo for a non-virtual class. The vtable member will be set to null.
#define HK_REFLECTION_DEFINE_SCOPED_NONVIRTUAL(SCOPE, KLASS) \
	HK_REFLECTION_DEFINE_INTERNAL_NONVIRTUAL( SCOPE##KLASS, SCOPE::KLASS, KLASS )

/// Define an hkTypeInfo for a non-virtual class which has no constructor (for simple structures).
#define HK_REFLECTION_DEFINE_SIMPLE(KLASS) \
	HK_REFLECTION_DEFINE_INTERNAL_SIMPLE( KLASS, KLASS, KLASS )
/// Define an hkTypeInfo for a non-virtual class which has no constructor (for simple structures).
#define HK_REFLECTION_DEFINE_SCOPED_SIMPLE(SCOPE, KLASS) \
	HK_REFLECTION_DEFINE_INTERNAL_SIMPLE( SCOPE##KLASS, SCOPE::KLASS, KLASS )

/// Utility to define an empty stub hkClass instance.
#define HK_REFLECTION_DEFINE_STUB_VIRTUAL_BASE(KLASS) \
    const hkClass KLASS##Class(#KLASS, HK_NULL, sizeof(KLASS), HK_NULL, 0, HK_NULL, 0, HK_NULL, 0, HK_NULL, HK_NULL, 0);
/// Utility to define an empty stub hkClass instance.
#define HK_REFLECTION_DEFINE_STUB_VIRTUAL(KLASS, PARENT) \
    const hkClass KLASS##Class(#KLASS, &PARENT##Class, sizeof(KLASS), HK_NULL, 0, HK_NULL, 0, HK_NULL, 0, HK_NULL, HK_NULL, 0);

#endif // HK_BASE_TYPE_INFO_H

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
