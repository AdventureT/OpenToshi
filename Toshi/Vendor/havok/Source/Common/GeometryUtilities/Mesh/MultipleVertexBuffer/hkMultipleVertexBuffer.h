/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_MULTIPLE_VERTEX_BUFFER_H
#define HK_MULTIPLE_VERTEX_BUFFER_H

class hkMemoryMeshVertexBuffer;

#include <Common/Base/Container/Array/hkObjectArray.h>
#include <Common/GeometryUtilities/Mesh/hkMeshVertexBuffer.h>

extern const hkClass hkMultipleVertexBufferClass;


/// The hkMultipleVertexBuffer class allows multiple contained vertex buffers to appear as a single vertex buffer
///
/// It's often useful to be able to make multiple vertex buffers appear to an application as a single vertex buffer.
/// For example in an implementation of the hkMeshSystem if a vertex buffer has non shared elements, internally
/// it would be beneficial to have share vertex data in one buffer and non shared in another. If this shared and non
/// shared data is held in objects with the hkMeshVertexBuffer interface - they can be returned together as an
/// apparent single vertex buffer, by containing them in a hkMultipleVertexBuffer. When the multiple vertex buffer is cloned
/// it can clone the contained vertex buffers - and the fully shared one will just be referenced counted - saving memory
/// and perhaps improving performance.
///
/// A multiple vertex buffer can contain one or more contained vertex buffers. A vertex format is specified when the
/// multiple vertex buffer is constructed. Each of the elements of multiple buffers vertex buffer format needs to be
/// mapped to an element of a contained vertex buffer. This mapping is set up by calling 'addElement', for each element
/// of the multiple vertex buffers vertex format. Once all of the elements have been added a call to 'completeConstruction'
/// should be made.
///
/// The multiple vertex buffer has an extra trick - in that it provides a limited amount of emulation of different
/// vertex element type conversions. If the data type in the vertex format is different from the multiple vertex buffers
/// vertex format, when a lock takes place, a conversion vertex buffer (m_lockedBuffer) is created, and it will hold
/// the vertex elements in the data format the multiple vertex buffer is presenting. The conversion between the internal
/// and external element formats must be possible through the hkVertexBufferUtil::convert method.
///
/// For an example of usage, the hkgMeshSystem uses the hkMultipleVertexBuffer to implement skinning. The \p hkg graphics
/// library doesn't support hardware skinning on many platforms. To get around this problem, the hkgMeshSystem,
/// creates a hkMultipleVertexBuffer for a vertex buffer that contains skinning data. The skinning data is held in a
/// memory vertex buffer which is in a suitable format for software skinning. Conversions between the formats is handled
/// automatically by the hkMultipleVertexBuffer implementation.
///
/// \sa hkMeshVertexBuffer

class hkMultipleVertexBuffer: public hkMeshVertexBuffer
{
    public:
        HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SCENE_DATA);

            /// Ctor
        hkMultipleVertexBuffer(const hkVertexFormat& format, int numVertices);

			/// Copy Ctor
        hkMultipleVertexBuffer(const hkMultipleVertexBuffer& rhs);

            // hkMeshVertexBuffer implementation
        virtual hkMeshVertexBuffer* clone();

			// hkMeshVertexBuffer implementation
		virtual bool isSharable() { return m_isSharable; }

            // hkMeshVertexBuffer implementation
        virtual void getVertexFormat( hkVertexFormat& formatOut ) { formatOut = m_vertexFormat; }

            // hkMeshVertexBuffer implementation
        virtual int getNumVertices() { return m_numVertices; }

            // hkMeshVertexBuffer implementation
        virtual LockResult lock( const LockInput& input, LockedVertices& lockedVerticesOut );

            // hkMeshVertexBuffer implementation
        virtual LockResult partialLock( const LockInput& input, const PartialLockInput& partialInput, LockedVertices& lockedOut);

            // hkMeshVertexBuffer implementation
        virtual void getElementVectorArray(const LockedVertices& lockedVertices, int elementIndex, hkVector4* compData);

            // hkMeshVertexBuffer implementation
        virtual void setElementVectorArray(const LockedVertices& lockedVertices, int elementIndex, const hkVector4* compData);

            // hkMeshVertexBuffer implementation
        virtual void getElementIntArray(const LockedVertices& lockedVertices, int elementIndex, int* compData);

            // hkMeshVertexBuffer implementation
        virtual void setElementIntArray(const LockedVertices& lockedVertices, int elementIndex, const int* compData);

            // hkMeshVertexBuffer implementation
        virtual void unlock( const LockedVertices& lockedVertices );

            /// Add an element
        void addElement(int vertexBufferIndex, int elementIndex);

            /// Add a vertex buffer
        void addVertexBuffer(hkMeshVertexBuffer* vertexBuffer);

			/// Must be called before the buffer can be used
		void completeConstruction();

            /// Returns the current update count - a non zero number that changes every time there is a write lock
        HK_FORCE_INLINE hkUint32 getUpdateCount() const { return m_updateCount; }

			// Get a vertex buffer
		HK_FORCE_INLINE hkMeshVertexBuffer* getVertexBuffer(int index) const { return m_vertexBufferInfos[index].m_vertexBuffer; }

            // hkReferencedObject Implementation
        virtual const hkClass* getClassType() const { return &hkMultipleVertexBufferClass; }

            // hkReferencedObject Implementation
        virtual void calcContentStatistics( hkStatisticsCollector* collector,const hkClass* cls ) const;

    protected:
			/// ElementInfo
        struct ElementInfo
        {
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkMultipleVertexBuffer::ElementInfo );

            hkUint8 m_vertexBufferIndex;                            ///
            hkUint8 m_elementIndex;                                 ///
        };
			/// VertexBufferInfo
        struct VertexBufferInfo
        {
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkMultipleVertexBuffer::VertexBufferInfo );
            VertexBufferInfo() : m_isLocked(false) {}

            hkRefPtr<hkMeshVertexBuffer> m_vertexBuffer;                ///
            hkMeshVertexBuffer::LockedVertices m_lockedVertices;        ///
            hkBool m_isLocked;											///
        };

			/// LockedElement
        struct LockedElement
        {
            hkUint8 m_vertexBufferIndex;                            ///
            hkUint8 m_elementIndex;                                 ///

            hkUint8 m_lockedBufferIndex;                            ///
            hkUint8 m_vertexFormatIndex;                            ///

            hkUint8 m_lockFlags;                                    ///
            hkUint8 m_outputBufferIndex;                            ///

            hkInt8 m_emulatedIndex;                                 ///
        };
        static HK_FORCE_INLINE bool _less(const LockedElement& a, const LockedElement& b)
        {
            if (a.m_vertexBufferIndex != b.m_vertexBufferIndex)
            {
                return a.m_vertexBufferIndex < b.m_vertexBufferIndex;
            }

            // All of the element indices must be unique
            return a.m_elementIndex < b.m_elementIndex;
        }

		void _unlockVertexBuffers();

		hkMeshVertexBuffer::LockResult _lockElements( const hkMeshVertexBuffer::LockInput& input, hkMeshVertexBuffer::LockedVertices& lockedOut);


	protected:
		//
		//	Members
		//

        hkVertexFormat m_vertexFormat;                              ///

        hkArray<LockedElement> m_lockedElements;                    ///
        hkRefPtr<hkMemoryMeshVertexBuffer> m_lockedBuffer;          ///

        hkArray<ElementInfo> m_elementInfos;						///
        hkObjectArray<VertexBufferInfo> m_vertexBufferInfos;

		int m_numVertices;											///
        hkBool m_isLocked;

        hkUint32 m_updateCount;                                     ///
        hkBool m_writeLock;                                         ///

		hkBool m_isSharable;
		hkBool m_constructionComplete;
};


#endif // HK_MULTIPLE_VERTEX_BUFFER_H

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
