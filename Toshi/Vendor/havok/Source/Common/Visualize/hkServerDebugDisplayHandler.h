/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_VISUALIZE_SERVER_DEBUG_DISPLAY_HANDLER
#define HK_VISUALIZE_SERVER_DEBUG_DISPLAY_HANDLER

#include <Common/Visualize/hkDebugDisplayHandler.h>
class hkDisplaySerializeOStream;

class hkServerDebugDisplayHandler : public hkReferencedObject, public hkDebugDisplayHandler
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);

			/// Serializes hkDisplayHandler messages, the stream must be writable.
		hkServerDebugDisplayHandler(hkDisplaySerializeOStream* stream);

			/// Destructor.
		virtual ~hkServerDebugDisplayHandler();
		
		//
		// Display handler functions
		//
		virtual hkResult addGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint);
		virtual hkResult addGeometryInstance(hkUlong originalInstanceID, const hkTransform& transform, hkUlong id, int tag, hkUlong shapeIdHint);
		virtual hkResult setGeometryColor(int color, hkUlong id, int tag);
		virtual hkResult updateGeometry(const hkTransform& transform, hkUlong id, int tag);
		virtual hkResult removeGeometry(hkUlong id, int tag, hkUlong shapeIdHint);

		virtual hkResult updateCamera(const hkVector4& from, const hkVector4& to, const hkVector4& up, hkReal nearPlane, hkReal farPlane, hkReal fov, const char* name);
		virtual hkResult updateBehavior(hkArray<int>& wordVarIdx, hkArray<int>& wordStack, hkArray<int>& quadVarIdx, hkArray<hkVector4>& quadStack,
			 hkArray<char*>& activeNodes, hkArray<int>& activeStateIds, hkArray<int>& activeTransitions, hkArray<hkQsTransform>& transforms);

		virtual hkResult displayPoint(const hkVector4& position, int colour, int tag);
		virtual hkResult displayLine(const hkVector4& start, const hkVector4& end, int color, int tag);
		virtual hkResult displayTriangle(const hkVector4& a, const hkVector4& b, const hkVector4& c, int color, int tag);

		virtual hkResult displayText(const char* text, int length, int tag);
		virtual hkResult display3dText(const char* text, const hkVector4& pos, int length, int tag);

		virtual hkResult displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, const hkTransform& transform, int color, int tag);
		virtual hkResult displayGeometry(const hkArray<hkDisplayGeometry*>& geometries, int color, int tag);

		virtual hkResult sendMemStatsDump(const char* data, int length);

		virtual hkResult holdImmediate();
		
	protected:
	
		hkDisplaySerializeOStream* m_outStream; 

		/// A critical section to ensure that writing to m_outStream will be
		/// thread-safe.
		mutable class hkCriticalSection* m_outstreamLock;

		void sendGeometryData(const hkArray<hkDisplayGeometry*>& geometries);
};

#endif // HK_VISUALIZE_SERVER_DEBUG_DISPLAY_HANDLER


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
