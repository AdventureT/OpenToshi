/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE_DISPLAY_HANDLER
#define HK_VISUALIZE_DISPLAY_HANDLER

class hkDisplayGeometry;
#include <Common/Base/hkBase.h>

/// This class defines the graphical interface to the visual debugger.
/// It is also used to display worlds with the Havok demos.
///
/// IDs: IDs are used to manage persistent objects which are added/updated
/// and removed.  It is the responsibility of the user to supply a unique
/// id for every persistent object added and to use this id consistently when
/// updating and removing objects.
/// The current convention when supplying ids is to supply the
/// address of the hkpCollidable where the geometries came from.
/// Geometries are built using the hkpShapeDisplayBuilder class in
/// hkvisualize/hkpShapeDisplayBuilder.h/cpp.
///
/// TAGs: Unique tags can be obtained form the hkViewerFactory::getTag().
/// These can are used for separating display information into different
/// categories so that they can be configured and filtered independently.
/// For example, each Viewer has one or more unique tags.
///


class hkDebugDisplayHandler
{
	public:
		
		enum Command
		{
			HK_STEP = 0x00,
			HK_ADD_GEOMETRY,
			HK_UPDATE_GEOMETRY,
			HK_SET_COLOR_GEOMETRY,
			HK_REMOVE_GEOMETRY,
			HK_DISPLAY_GEOMETRY_WITH_TRANSFORM,
			HK_DISPLAY_GEOMETRY,
			HK_DISPLAY_POINT,
			HK_DISPLAY_LINE,
			HK_DISPLAY_TEXT,	// These may move into a separate interface later

			HK_SEND_STATISTICS_DUMP_DEPRICATED, // moved in to stat handler itself

			HK_HOLD_IMMEDIATE,

			HK_ADD_CAMERA,
			HK_UPDATE_CAMERA,
			HK_REMOVE_CAMERA,

			HK_SEND_MEMSTATS_DUMP,  // Added for 3.0.0
			HK_ADD_GEOMETRY_INSTANCE, // Added for 4.5.0
			HK_DISPLAY_TEXT_3D,	// Added for 4.6.0

			HK_UPDATE_BEHAVIOR,	// Added in 6.5
			HK_LIST_BEHAVIORS,	// Added in 6.5
		};

			/// virtual Destructor
		virtual ~hkDebugDisplayHandler() {} 


		//
		// Geometry functions
		//
			/// Adds a geometry to the display world managed by this display handler.
			/// Please refer to the class description for the meaning of id and tag.
		virtual hkResult addGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint) = 0;

			/// Adds an instanced of a geometry that has already been added to the display world managed by this display handler.
			/// Please refer to the class description for the meaning of id and tag.
		virtual hkResult addGeometryInstance(hkUlong origianalGeomId, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint) = 0;

			/// Sets the color of a geometry previously added to the display world.
			/// Please refer to the class description for the meaning of id and tag.
		virtual hkResult setGeometryColor(int color, hkUlong id, int tag) = 0;

			/// Updates the transform of a body in the display world.
			/// Please refer to the class description for the meaning of id and tag.
		virtual hkResult updateGeometry(const hkTransform& transform, hkUlong id, int tag) = 0;

			/// Removes a geometry from the display world managed by this display handler.
			/// Please refer to the class description for the meaning of id and tag.
		virtual hkResult removeGeometry(hkUlong id, int tag, hkUlong shapeIdHint) = 0;

		
		//
		// Camera Functionality
		//
		virtual hkResult updateCamera(const hkVector4& from, const hkVector4& to, const hkVector4& up, hkReal nearPlane, hkReal farPlane, hkReal fov, const char* name) = 0;

		//
		// Behavior Temp callbacks
		//
		virtual hkResult updateBehavior(hkArray<int>& wordVarIdx, hkArray<int>& wordStack, hkArray<int>& quadVarIdx, hkArray<hkVector4>& quadStack,
			hkArray<char*>& activeNodes, hkArray<int>& activeStateIds, hkArray<int>& activeTransitions, hkArray<hkQsTransform>& transforms) = 0;

		//
		// Immediate Mode Functions
		//

			/// Puts a display point into the display buffer for display in the next frame.
		virtual hkResult displayPoint(const hkVector4& position, int colour, int tag) = 0;

			/// Puts a display line into the display buffer for display in the next frame.
		virtual hkResult displayLine(const hkVector4& start, const hkVector4& end, int color, int tag) = 0;

			/// Puts a display triangle into the display buffer for display in the next frame.
		virtual hkResult displayTriangle(const hkVector4& a, const hkVector4& b, const hkVector4& c, int colour, int tag) = 0;

			/// Outputs user text to the display.  (The manner in which the text
			/// is displayed depends on the implementation of the display handler.)
		virtual hkResult displayText(const char* text, int color, int tag) = 0;

			// Outputs 3D text
		virtual hkResult display3dText(const char* text, const hkVector4& pos, int color, int tag) = 0;

			/// Displays the geometries	
		virtual hkResult displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, int color, int tag) = 0;

			/// Displays the geometries	without transform
		virtual hkResult displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, int color, int tag) = 0;


		// Utility functions (just call displayLine etc above)
		void displayFrame( const hkQsTransform& worldFromLocal, hkReal size, int tag );
		void displayFrame( const hkTransform& worldFromLocal, hkReal size, int tag );
		void displayArrow( const hkVector4& from, const hkVector4& dir, int color, int tag );

		//
		// Statistics functions (ideally these would be in a separate interface to the display handler)
		//
		virtual hkResult sendMemStatsDump(const char* data, int length) = 0;


			/// Ensures that the current immediate mode display information
			/// will be preserved and merged with all new immediate mode data
			/// for the next step/frame.
		virtual hkResult holdImmediate() = 0;


			/// Batch update speed increases
		virtual void lockForUpdate() { }
		virtual void unlockForUpdate() { }

};


#endif // HK_VISUALIZE_DISPLAY_HANDLER

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
