/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_SHAPE_DISPLAY_BUILDER_H
#define HK_COLLIDE2_SHAPE_DISPLAY_BUILDER_H

#include <Common/Base/hkBase.h>

#include <Physics/Collide/Shape/hkpShapeType.h>

class hkpCollidable;
class hkDisplayGeometry;
class hkpShape;

	/// A utility class that creates hkDisplayGeometrys from hkCollidables and hkShapes.
	/// This class is used by the hkpShapeDisplayViewer.
class hkpShapeDisplayBuilder: public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);

		struct hkpShapeDisplayBuilderEnvironment
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_VDB, hkpShapeDisplayBuilder::hkpShapeDisplayBuilderEnvironment );

			hkpShapeDisplayBuilderEnvironment();
			int m_spherePhiRes;
			int m_sphereThetaRes;
		};

		hkpShapeDisplayBuilder(const hkpShapeDisplayBuilderEnvironment& env);

			/// Creates an array of display geometries from a given shape.
		void buildDisplayGeometries(		const hkpShape* shape, 
											hkArray<hkDisplayGeometry*>& displayGeometries);

			/// Creates an array of display geometries from a given shape and a transform.
		void buildShapeDisplay(				const hkpShape* shape, 
											const hkTransform& transform, 
											hkArray<hkDisplayGeometry*>& displayGeometries);

			/// Clears the current raw geometry.  A temporary geometry is used to
			/// create display geometries triangle by triangle from a MOPP for example.
		void resetCurrentRawGeometry();

			/// Convert a geometry and send it to the hkDebugDisplay
		static void HK_CALL addObjectToDebugDisplay( const hkpShape* shape, hkTransform& t, hkUlong id );
	protected:

		hkDisplayGeometry* getCurrentRawGeometry(hkArray<hkDisplayGeometry*>& displayGeometries);

	protected:

		hkpShapeDisplayBuilderEnvironment m_environment;
		hkDisplayGeometry* m_currentGeometry;


};

class hkpUserShapeDisplayBuilder : public hkReferencedObject, public hkSingleton< hkpUserShapeDisplayBuilder >
{
	//+vtable(true)
	public:

		hkpUserShapeDisplayBuilder() {}

			/// A function to build display geometries for user shapes.  The function is expected to add hkDisplayGeometry objects
			/// to the displayGeometries list.  It may call back the hkpShapeDisplayBuilder::buildDisplayGeometries() method on the
			/// builder object passed in to achieve this.
		typedef void (HK_CALL *ShapeBuilderFunction)(	const hkpShape* shape,
														const hkTransform& transform,
														hkArray<hkDisplayGeometry*>& displayGeometries,
														hkpShapeDisplayBuilder* builder );

			/// You can register functions to build display for your own user types with the shape display builder using this method
		void registerUserShapeDisplayBuilder( ShapeBuilderFunction f, hkpShapeType type );

	public:

		struct UserShapeBuilder
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_VDB, hkpUserShapeDisplayBuilder::UserShapeBuilder );

			ShapeBuilderFunction f;
			hkpShapeType type;
		};
		hkArray< UserShapeBuilder > m_userShapeBuilders;
};


#endif // HK_COLLIDE2_SHAPE_DISPLAY_BUILDER_H

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
