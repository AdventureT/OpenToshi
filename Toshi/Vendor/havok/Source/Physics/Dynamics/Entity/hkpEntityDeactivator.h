/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#ifndef HK_DYNAMICS2_ENTITY_DEACTIVATOR_H
#define HK_DYNAMICS2_ENTITY_DEACTIVATOR_H

#include <Common/Base/hkBase.h>

class hkpEntity;
class hkEntityDeactivatorCinfo;

extern const hkClass hkpEntityDeactivatorClass;

/// An hkpEntityDeactivator is created for every entity. This tells the system when it is safe to deactivate the entity.
/// This is an internal class used by dynamics - you do not use this class directly.
/// This class is deprecated.
class hkpEntityDeactivator : public hkReferencedObject
{
	public:

		HK_DECLARE_REFLECTION();

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_DEACTIVATOR);

			//High frequency deactivation is used to quickly deactivate entities that have come to a definite halt. 
		virtual hkBool shouldDeactivateHighFrequency( const hkpEntity* entity ) = 0;

			//Low frequency deactivation checks less frequently if the object needs to be deactivated. This makes it less likely that the system will deactivate an object that might quickly need to move again.
		virtual hkBool shouldDeactivateLowFrequency( const hkpEntity* entity ) = 0;

	protected:

		hkpEntityDeactivator() {}

	public:

		hkpEntityDeactivator( class hkFinishLoadedObjectFlag flag ) : hkReferencedObject(flag) {}
};






#endif // HK_DYNAMICS2_ENTITY_DEACTIVATOR_H

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
