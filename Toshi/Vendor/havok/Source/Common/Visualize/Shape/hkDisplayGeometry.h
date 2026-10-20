/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE_SHAPE_GEOMETRY_H
#define HK_VISUALIZE_SHAPE_GEOMETRY_H

#include <Common/Base/hkBase.h>
#include <Common/Visualize/Shape/hkDisplayGeometryTypes.h>
struct hkGeometry;

class hkDisplayGeometry : public hkReferencedObject
{
	public:
		
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_DISPLAY);

		inline hkDisplayGeometry(hkDisplayGeometryType type);

		virtual ~hkDisplayGeometry();

		inline hkGeometry* getGeometry();

		// method returns array of points s,e,s,e,s,e - start, end ...
		virtual void getWireframeGeometry(hkArray<hkVector4>& lines) = 0;

		virtual void buildGeometry() = 0;

		inline const hkTransform& getTransform() const;

		inline hkTransform& getTransform();

		inline hkDisplayGeometryType getType() const;

		hkGeometry* m_geometry;
	
	protected:

		hkTransform m_transform;
		hkDisplayGeometryType m_type;
};

#include <Common/Visualize/Shape/hkDisplayGeometry.inl>

#endif // HK_GEOMETRY2_DISPLAY_GEOMETRY_H

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
