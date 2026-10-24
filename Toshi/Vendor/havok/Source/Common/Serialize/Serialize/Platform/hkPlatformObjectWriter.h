/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SERIALIZE_PLATFORM_OBJECT_WRITER_H
#define HK_SERIALIZE_PLATFORM_OBJECT_WRITER_H

#include <Common/Serialize/Util/hkStructureLayout.h>
#include <Common/Serialize/Serialize/hkObjectWriter.h>

class hkOArchive;
class hkObjectCopier;

/// Write an object in the native format of a given platform.
class hkPlatformObjectWriter : public hkObjectWriter
{
	public:

			/// Create an hkPlatformObjectWriter.
		hkPlatformObjectWriter(const hkStructureLayout& layout);

			/// Destroy an hkPlatformObjectWriter.
		virtual ~hkPlatformObjectWriter();

			/// Save object data, using class information klass.
		virtual hkResult writeObject(hkStreamWriter* writer, const void* data, const hkClass& klass, hkRelocationInfo& reloc );

			/// Write a raw binary chunk.
		virtual hkResult writeRaw( hkStreamWriter* writer, const void* data, int dataLen );

			/// Get the layout this writer was created with.
		const hkStructureLayout& getLayout() const;

		struct Cache;
		
	protected:


		hkObjectCopier* m_copier;
		Cache* m_cache;
};

#endif //HK_SERIALIZE_PLATFORM_OBJECT_WRITER_H


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
