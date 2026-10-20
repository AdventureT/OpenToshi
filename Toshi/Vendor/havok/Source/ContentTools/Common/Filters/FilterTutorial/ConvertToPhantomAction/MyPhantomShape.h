/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_MYPHANTOM_SHAPE_H
#define INC_MYPHANTOM_SHAPE_H

#include <Physics/Collide/Shape/Misc/PhantomCallback/hkpPhantomCallbackShape.h>

/// meta information
extern const class hkClass MyPhantomShapeClass;

// Our phantom call back shape will be notified when an object enters or exits the shape
// We use those event to dynamically add / remove an action.
class MyPhantomShape : public hkpPhantomCallbackShape
{
	//+vtable(1)
	public:
		
		HK_DECLARE_REFLECTION();
		
		MyPhantomShape();
		MyPhantomShape( hkFinishLoadedObjectFlag flag ) : hkpPhantomCallbackShape(flag) {}
		
		// hkpPhantom interface implementation
		/*virtual*/ void phantomEnterEvent( const hkpCollidable* phantomColl, const hkpCollidable* otherColl, const hkpCollisionInput& env );
		/*virtual*/ void phantomLeaveEvent( const hkpCollidable* phantomColl, const hkpCollidable* otherColl );
		
	public:
		
		enum ActionType
		{
			ACTION_WIND,
			ACTION_ATTRACT,
			ACTION_DEFLECT
		};
		
		hkEnum<ActionType, hkInt8>	m_actionType;
		hkVector4						m_direction;
		hkReal							m_strength;
};


#endif //INC_MYPHANTOM_SHAPE_H

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
