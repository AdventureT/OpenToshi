/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE_DRAWUTIL_H
#define HK_VISUALIZE_DRAWUTIL_H

#include <Common/Base/Types/Color/hkColor.h>

/// This class facilitates drawing cones, arrows, planes representing ragdoll limits.
class hkDrawUtil: public hkReferencedObject
{
	public:
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TOOLS);

		hkDrawUtil(hkReal new_scale);

			/// Displays an oriented point.
		void displayOrientedPoint(const hkVector4& position,const hkRotation& rot,hkReal size,unsigned int color);

			/// Displays a point.
		void displayPoint(const hkVector4& position,hkReal size,hkUint32 color);

			/// Displays a line segment.
		void displaySegment(const hkVector4& p1, const hkVector4& p2, hkUint32 color);

			/// Displays a cone.
		void displayCone (hkReal cone_angle, const hkVector4& startPos, const hkVector4& coneAaxis, const hkVector4& perpVector, int numSegments, unsigned color);

			/// Displays a plane.
		void displayPlane (const hkVector4& startPos, const hkVector4& planeNormal, const hkVector4& vectorOnPlane, unsigned color, hkBool showNormal = true);

			/// Displays an arrow.
		void displayArrow (const hkVector4& startPos, const hkVector4& arrowDirection, const hkVector4& perpDirection, unsigned color);
        
			/// Draws a semi circle.
			/// \param center The position of the center in world space
			/// \param normal The axis about which the circle is drawn
			/// \param startPerp An orthogonal axis to the normal which defines the start of the sweep
		void drawSemiCircle(const hkVector4& center, hkVector4& normal, 
						    hkVector4& startPerp, hkReal thetaMin, hkReal thetaMax,
							hkReal radius,int numSegments, 
							unsigned int color);

			/// Gets the scale by which the visual representation of cones, arrows and planes will be scaled.
		hkReal getScale();

			/// Sets the scale by which the visual representation of cones, arrows and planes will be scaled.
		void setScale(hkReal new_scale);
	
	protected:
		hkReal m_scale;
};

#endif // HK_VISUALIZE_DRAWUTIL_H

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
