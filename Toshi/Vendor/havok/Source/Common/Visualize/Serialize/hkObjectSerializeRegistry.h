/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_VISUALIZE_OBJECT_SERIALIZE_REG_H
#define HK_VISUALIZE_OBJECT_SERIALIZE_REG_H

#include <Common/Base/Container/PointerMap/hkPointerMap.h>

/// A utility class that can serialize in and out classes
/// from a stream, and store the created 
class hkObjectSerializeRegistry : public hkReferencedObject
{

	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);

		hkObjectSerializeRegistry() {}

			/// dtor will destruct all owned data (will call clear())
		virtual ~hkObjectSerializeRegistry();

			/// Add an object to the map or refresh
			/// the old object data with this new data
			/// (would dealloc old data and replace in that case)
			/// Takes ownership of data. Assumes it was from hkAllocated<char> 
		void addObject( hkUint64 id, void* data, hkUint32 dataSize );

			/// Deallocate the storage used by the data and 
			/// remove from map.
		void deleteObject( hkUint64 id);
		
			/// Get the data that has been read in for object id.
			/// May return null, in which case you should request the data from 
			/// the host and get readObject to interpret it.
		void* getObjectData( hkUint64 id );
		hkUint32 getObjectSize( hkUint64 id );

			/// deallocate and clear records of all owned data
		void clear();

			/// reverse lookup 
		hkUint64 findObjectID( const void* data );

	protected:
		
		/// Not actually 64 bits stored as hkPointerMap uses ulong internally. XX Fixme.
		hkPointerMap< hkUint64, void* > m_idToObjectMap;
		hkPointerMap< hkUint64, hkUint32 > m_idToObjectSizeMap;
};

#endif //  HK_VISUALIZE_OBJECT_SERIALIZE_REG_H

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
