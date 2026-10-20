/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKUTILITIES_BINDINGDATA_H
#define HKUTILITIES_BINDINGDATA_H

#include <Common/Base/hkBase.h>
#include <Common/Base/Container/Array/hkArray.h>

extern const class hkClass hkpRigidBodyDisplayBindingClass;
extern const class hkClass hkpPhysicsSystemDisplayBindingClass;
extern const class hkClass hkpDisplayBindingDataClass;

// A single 'rigid body <-> display object' binding
struct hkpRigidBodyDisplayBinding 
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkpRigidBodyDisplayBinding );
	HK_DECLARE_REFLECTION();
		
	class hkpRigidBody* m_rigidBody;
	
	class hkxMesh* m_displayObject;
	
	hkMatrix4 m_rigidBodyFromDisplayObjectTransform;
};

// A physics system and its collection of bindings
struct hkpPhysicsSystemDisplayBinding
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkpPhysicsSystemDisplayBinding );
	HK_DECLARE_REFLECTION();
	
	hkArray<hkpRigidBodyDisplayBinding*> m_bindings;
	
	class hkpPhysicsSystem* m_system;
	
		/// For serialization we need a ctor that does not call the ctor of the array on load.
	hkpPhysicsSystemDisplayBinding() { }
	hkpPhysicsSystemDisplayBinding(hkFinishLoadedObjectFlag f) : m_bindings(f) { }  
};

// A collection of display bindings
struct hkpDisplayBindingData
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkpDisplayBindingData );
	HK_DECLARE_REFLECTION();
	
	hkArray<hkpRigidBodyDisplayBinding*> m_rigidBodyBindings;
	hkArray<hkpPhysicsSystemDisplayBinding*> m_physicsSystemBindings;
	
		/// For serialization we need a ctor that does not call the ctor of the arrays on load.
	hkpDisplayBindingData() { }
	hkpDisplayBindingData(hkFinishLoadedObjectFlag f) : m_rigidBodyBindings(f), m_physicsSystemBindings(f) { }  
};

#endif // HKUTILITIES_BINDINGDATA_H

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
