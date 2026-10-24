/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE_DEBUG_DISPLAY_H
#define HK_VISUALIZE_DEBUG_DISPLAY_H

#include <Common/Base/Types/Color/hkColor.h>
#include <Common/Visualize/hkDebugDisplayHandler.h>

class hkDisplayGeometry;

/// A helper class, which enable the display of debug points and lines
/// To use this class call the HK_DISPLAY_POINT(position, color) and 
/// HK_DISPLAY_LINE(start, end, color) macros.  A HK_SET_OBJECT_COLOR(id, color)
/// macro is also defined for convenience, where id is the address of the rigid 
/// body.
/// This is a thread safe singleton
class hkDebugDisplay : public hkReferencedObject, public hkSingleton<hkDebugDisplay>
{
	//+vtable(true)
	friend class hkSingleton<hkDebugDisplay>;

	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);

		hkDebugDisplay();
		~hkDebugDisplay();

			/// Add a display viewer to the debug display.  All
			/// debug points and lines will be send to this display
			/// handler until it is removed.
		void addDebugDisplayHandler(hkDebugDisplayHandler* debugDisplay);

			/// Remove a display viewer from the debug display.
			/// No more debug points and lines will be sent to the
			/// display handler.
		void removeDebugDisplayHandler(hkDebugDisplayHandler* debugDisplay);

			/// Removes all display viewer from the debug display.
			/// No debug points and lines will be displayed after
			/// this function is called.
		void clear();

	public:

			/// Adds a geometry to the display world managed by this display handler.  The current convention
			/// for supplying an id is to take the address of the hkpCollidable from which the geometries were
			/// created.  
			/// Please refer to the hkDisplayHandler class description for the meaning of id and tag.
		void addGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint);

			/// Adds an instanced of a geometry that has already been added to the display world managed by this display handler.
			/// Please refer to the class description for the meaning of id and tag.
		void addGeometryInstance(hkUlong origianalGeomId, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint);

			/// Sets the color of an object that has already been added to all display viewer (targeted by the debug display)
			/// Please refer to the hkDisplayHandler class description for the meaning of id and tag.
		void setGeometryColor(int color, hkUlong id, int tag);

			/// Updates the transform of a body in the display world.
			/// Please refer to the hkDisplayHandler class description for the meaning of id and tag.
		void updateGeometry(const hkTransform& transform, hkUlong id, int tag);

			/// Removes a geometry from the display world managed by this display handler.
			/// Please refer to the hkDisplayHandler class description for the meaning of id and tag.
		void removeGeometry(hkUlong id, int tag, hkUlong shapeIdHint);

			/// updated the user camera, the user can supply as many cameras as they like and these are identified by
			/// supplying a unique id for each camera.  (NOTE: currently only one user camera is supported and the id is
			/// ignored.)
		void updateCamera(const hkVector4& from, const hkVector4& to, const hkVector4& up, hkReal nearPlane, hkReal farPlane, hkReal fov, const char* name);

			/// Behavior updates
		void updateBehavior(hkArray<int>& wordVarIdx, hkArray<int>& wordStack, hkArray<int>& quadVarIdx, hkArray<hkVector4>& quadStack,
								hkArray<char*>& activeNodes, hkArray<int>& activeStateIds, hkArray<int>& activeTransitions, hkArray<hkQsTransform>& transforms);

			/// Displays a debug point. Please use the HK_DISPLAY_POINT macro instead of calling this function directly.
		void displayPoint(const hkVector4& position, int color, int tag);

			/// Display a debug line. Please use the HK_DISPLAY_LINE macro instead of calling this function directly.
		void displayLine(const hkVector4& start, const hkVector4& end, int color, int tag);

			/// Display a debug line. Please use the HK_DISPLAY_LINE macro instead of calling this function directly.
		void displayTriangle(const hkVector4& a, const hkVector4& b, const hkVector4& c, int color, int tag);

			/// Display a debug line using model coordinate endpoints.  
			/// Please use the HK_DISPLAY_LINE_MODEL macro instead of calling this function directly.
		void displayLineModelSpace(const hkQsTransform& worldFromModel, const hkVector4& start, const hkVector4& end, int color, int tag);

			/// Display a debug line using model coordinate endpoints.  
			/// Please use the HK_DISPLAY_LINE_MODEL macro instead of calling this function directly.
		void displayLineModelSpace(const hkTransform& worldFromModel, const hkVector4& start, const hkVector4& end, int color, int tag);

			/// Display a debug line specified as a start and direction (not necessarily normalized).
			/// Please use the HK_DISPLAY_RAY macro instead of calling this function directly.
		void displayRay(const hkVector4& start, const hkVector4& direction, int color, int tag);

			/// Display a debug ray in model coordinates.
			/// Please use the HK_DISPLAY_RAY_MODEL macro instead of calling this function directly.
		void displayRayModelSpace(const hkQsTransform& worldFromModel, const hkVector4& start, const hkVector4& direction, int color, int tag);

			/// Display a debug ray in model coordinates.
			/// Please use the HK_DISPLAY_RAY_MODEL macro instead of calling this function directly.
		void displayRayModelSpace(const hkTransform& worldFromModel, const hkVector4& start, const hkVector4& direction, int color, int tag);

			/// Display a debug arrow. Please use the HK_DISPLAY_ARROW macro
			/// instead of calling this function directly.
		void displayArrow(const hkVector4& start, const hkVector4& dir, int color, int tag);

			/// Display a debug star. Please use the HK_DISPLAY_STAR macro
			/// instead of calling this function directly.
		void displayStar(const hkVector4& position, hkReal scale, int color, int tag);

			/// Display a debug star in model coordinates.
			/// Please use the HK_DISPLAY_STAR_MODEL macro instead of calling this function directly.
		void displayStarModelSpace(const hkQsTransform& worldFromModel, const hkVector4& position, hkReal scale, int color, int tag);

			/// Display a debug star in model coordinates.
			/// Please use the HK_DISPLAY_STAR_MODEL macro instead of calling this function directly.
		void displayStarModelSpace(const hkTransform& worldFromModel, const hkVector4& position, hkReal scale, int color, int tag);
		
			/// Display a debug plane. Please use the HK_DISPLAY_PLANE macro
			/// instead of calling this function directly.
		void displayPlane(const hkVector4& plane, const hkVector4& offset, hkReal scale, int color, int tag);

			/// Outputs user text to the display.  (The manner in which the text is displayed depends on the 
			/// implementation of the display handler.)
		void displayText(const char* text, int color, int tag);

			/// Outputs user text to the display at the coordinates in world space
		void display3dText(const char* text, const hkVector4& pos, int color, int tag);

			/// Display a debug frame of reference, with the axes colored X=red, Y=green, Z=blue
			/// Please use the HK_DISPLAY_FRAME macro instead of calling this function directly.
		void displayFrame( const hkQsTransform& worldFromLocal, hkReal size, int tag );

			/// Display a debug frame of reference, with the axes colored X=red, Y=green, Z=blue
			/// Please use the HK_DISPLAY_FRAME macro instead of calling this function directly.
		void displayFrame( const hkTransform& worldFromLocal, hkReal size, int tag );

			/// Displays the geometries	for one frame in immediate mode.
		void displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, int color, int tag);

			/// Displays the geometries	without transform for one frame in immediate mode.
		void displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, int color, int tag);

			/// Display a wireframe AABB.
			/// Please use the HK_DISPLAY_BOUNDING_BOX macro instead of calling this function directly.
		void displayAabb(const class hkAabb& aabb, int color, int tag);

	protected:

		hkArray<hkDebugDisplayHandler*> m_debugDisplayHandlers;
		mutable class hkCriticalSection* m_arrayLock;
};


// Macros for displaying debug points and lines.
// These can be compiled out if HK_DISABLE_DEBUG_DISPLAY is defined.

#if defined(HK_DISABLE_DEBUG_DISPLAY)
// No display of points and lines
#	define HK_ADD_GEOMETRY(geometries, transform, id)	/* nothing */
#	define HK_SET_OBJECT_COLOR(id, color)
#	define HK_UPDATE_GEOMETRY(transform, id)
#	define HK_REMOVE_GEOMETRY(id)
#	define HK_UPDATE_CAMERA(from, to, up, nearPlane, farPlane, fov, name)
#	define HK_DISPLAY_POINT(position, color)
#	define HK_DISPLAY_LINE(start, end, color)
#	define HK_DISPLAY_TRIANGLE(a, b, c, color)
#	define HK_DISPLAY_RAY(start, dir, color)
#	define HK_DISPLAY_ARROW(start, end, color)
#	define HK_DISPLAY_STAR(pos, scale, color)
#	define HK_DISPLAY_PLANE(plane, offset, scale, color)
#	define HK_DISPLAY_TEXT(text, color)
#	define HK_DISPLAY_3D_TEXT(text, pos, color)
#   define HK_DISPLAY_FRAME(worldFromLocal, size)
#	define HK_DISPLAY_GEOMETRY(geometries, color)
#	define HK_DISPLAY_GEOMETRY_WITH_TRANSFORM(geometries, transform, color)
#	define HK_DISPLAY_BOUNDING_BOX(aabb, color)

#else // HK_DISABLE_DEBUG_DISPLAY
// The debug display (display points and lines)
// NOTE: Please refer to the hkDebugDisplay class definition above for
// information on these macros parameters.  These calls are essentially
// user versions of the calls in the hkDisplayHandler class interface,
// please read its class description.
#	define HK_ADD_GEOMETRY(geometries, transform, id) hkDebugDisplay::getInstance().addGeometry(geometries, transform, id, 0, 0)
#	define HK_SET_OBJECT_COLOR(id, color) hkDebugDisplay::getInstance().setGeometryColor(color, id, 0)
#	define HK_UPDATE_GEOMETRY(transform, id) hkDebugDisplay::getInstance().updateGeometry(transform, id, 0)
#	define HK_REMOVE_GEOMETRY(id) hkDebugDisplay::getInstance().removeGeometry(id, 0, 0)
#	define HK_UPDATE_CAMERA(from, to, up, nearPlane, farPlane, fov, name) hkDebugDisplay::getInstance().updateCamera(from, to, up, nearPlane, farPlane, fov, name)
#	define HK_DISPLAY_POINT(position, color) hkDebugDisplay::getInstance().displayPoint(position, color, 0)
#	define HK_DISPLAY_LINE(start, end, color) hkDebugDisplay::getInstance().displayLine(start, end, color, 0)
#	define HK_DISPLAY_TRIANGLE(a, b, c, color) hkDebugDisplay::getInstance().displayTriangle(a,b,c, color, 0)
#	define HK_DISPLAY_LINE_MODEL(worldFromModel, start, end, color ) hkDebugDisplay::getInstance().displayLineModelSpace(worldFromModel, start, end, color, 0)
#	define HK_DISPLAY_RAY(start, dir, color) hkDebugDisplay::getInstance().displayRay(start, dir, color, 0)
#	define HK_DISPLAY_RAY_MODEL(worldFromModel, start, dir, color) hkDebugDisplay::getInstance().displayRayModelSpace(worldFromModel, start, dir, color, 0)
#	define HK_DISPLAY_ARROW(start, direction, color) hkDebugDisplay::getInstance().displayArrow(start, direction, color, 0)
#	define HK_DISPLAY_STAR(pos, scale, color) hkDebugDisplay::getInstance().displayStar(pos, scale, color, 0)
#	define HK_DISPLAY_STAR_MODEL(worldFromModel, pos, scale, color) hkDebugDisplay::getInstance().displayStarModelSpace(worldFromModel, pos, scale, color, 0)
#	define HK_DISPLAY_PLANE(plane, offset, scale, color) hkDebugDisplay::getInstance().displayPlane(plane, offset, scale, color, 0)
#	define HK_DISPLAY_TEXT(text, color) hkDebugDisplay::getInstance().displayText(text, color, 0)
#	define HK_DISPLAY_3D_TEXT(text, pos, color) hkDebugDisplay::getInstance().display3dText(text, pos, color, 0)
#   define HK_DISPLAY_FRAME(worldFromLocal, size) hkDebugDisplay::getInstance().displayFrame( worldFromLocal, size, 0 )
#	define HK_DISPLAY_GEOMETRY(geometries, color) hkDebugDisplay::getInstance().displayGeometry(geometries, color, 0)
#	define HK_DISPLAY_GEOMETRY_WITH_TRANSFORM(geometeries, transform, color) hkDebugDisplay::getInstance().displayGeometry(geometeries, transform, color, 0)
#	define HK_DISPLAY_BOUNDING_BOX(aabb, color) hkDebugDisplay::getInstance().displayAabb(aabb, color, 0)

#endif // HK_DISABLE_DEBUG_DISPLAY

#endif // HK_VISUALIZE_DEBUG_DISPLAY_H




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
