/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_MOPP_CODE_H
#define HK_COLLIDE2_MOPP_CODE_H

typedef hkUint32 hkpPrimitiveProperty;
extern const hkClass hkpMoppCodeClass;
class hkStatisticsCollector;

/// Defines the size of each mopp chunk (512 bytes)
#define HK_MOPP_CHUNK_SIZE 512
#define HK_MOPP_CHUNK_MASK (HK_MOPP_CHUNK_SIZE - 1)
#define HK_MOPP_LOG_CHUNK_SIZE 9


class hkpMoppChunk
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MOPP, hkpMoppChunk );

		unsigned char data[HK_MOPP_CHUNK_SIZE];
};


/// Struct mapping old shape keys to new terminal ids
struct hkpMoppCodeReindexedTerminal
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MOPP, hkpMoppCodeReindexedTerminal );
	HK_DECLARE_REFLECTION();
	hkUint32 m_origShapeKey;
	hkUint32 m_reindexedShapeKey;
};

/// The MOPP code simulates a hierarchical bounding volume tree. Note that hkMoppCodes are reference counted.
class hkpMoppCode : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MOPP );
		HK_DECLARE_REFLECTION();

			// range of property values
		enum
		{
			MIN_PROPERTY_VALUE = 0,
			MAX_PROPERTY_VALUE = 0xffffffff
		};

			// number of property values
		enum
		{
			MAX_PRIMITIVE_PROPERTIES = 1
		};

		enum BuildType
		{
			BUILT_WITH_CHUNK_SUBDIVISION,
			BUILT_WITHOUT_CHUNK_SUBDIVISION,
		};

		struct CodeInfo
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_MOPP, hkpMoppCode::CodeInfo );
			HK_DECLARE_REFLECTION();
			
			inline hkReal getScale() const;

			inline void setScale(hkReal inVal);

			hkVector4 m_offset;
		};

	public:


			/// Create empty mopp code.
		hkpMoppCode( ) { m_info.m_offset.setZero4(); }

			// Used for serialization.
		hkpMoppCode( hkFinishLoadedObjectFlag f ) : hkReferencedObject(f), m_data(f) { }

			/// Create from existing data. "moppData" is used inplace.
		inline hkpMoppCode( const CodeInfo& info, const hkUint8* moppData, int moppBytes );

		virtual ~hkpMoppCode() {} // so that the serialization knows it is virtual.

		inline hkInt32 getCodeSize() const;

			/// Create from existing data. "moppData" is used inplace.
		void initialize( const CodeInfo& info, const hkUint8* moppData, int moppBytes );

	public:

		struct CodeInfo m_info;

		hkArray<hkUint8> m_data;

		hkEnum<BuildType, hkInt8> m_buildType;
};

#include <Physics/Internal/Collide/Mopp/Code/hkpMoppCode.inl>

#endif // HK_COLLIDE2_MOPP_CODE_H

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
