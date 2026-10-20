/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_BOUNDING_VOLUME_TREE_H
#define HK_BOUNDING_VOLUME_TREE_H

#include <Common/Base/hkBase.h>

/// The base class for bounding volume tree implementations: mainly a collection of interfaces.
class hkBoundingVolumeTree
{
	public:

		/// Used for BOUNDING_VOLUME::overlaps argument
		struct Ray
		{
			void setRay( const hkVector4& start, const hkVector4& end )
			{
				extern const hkQuadReal hkQuadRealMax;

				m_startMin = start;
				m_startMax = start;
				hkVector4 delta; delta.setSub4( end, start );
				hkVector4Comparison isZero = delta.compareEqual4( hkVector4::getZero() );
				hkVector4 maskedDelta; maskedDelta.select32(delta, (hkVector4&)hkQuadReal1111, isZero);

				hkVector4 invDelta; invDelta.setReciprocal3( maskedDelta );
				m_invDir.select32( invDelta, (const hkVector4&)hkQuadRealMax, isZero );
			}

			hkVector4 m_startMin;	///
			hkVector4 m_startMax;	///
			hkVector4 m_invDir;
		};

		/// A tree node (either leaf or internal node)
		/// An implementation has to append a bounding volume to each node.
		struct Node
		{
			/// Return true if this node is a leaf.
			hkUlong isLeaf( ) const
			{
				return m_leafData.m_zero == 0;
			}

			Node* m_parent;			///
			Node* m_skipNode;		///

			union 
			{
				struct ///
				{
					Node* m_children[2];	///
				} m_nodeData;

				struct ///
				{
					void* m_zero;		///
					void* m_userData;	///
				} m_leafData;
			};
		};

		/// A hint to allocator.
		enum AllocationType
		{
			ALLOCATION_TYPE_LEAF, ALLOCATION_TYPE_INTERNAL_NODE
		};

		/// An allocator which uses the ThreadMemory.
		class ThreadMemoryAllocator
		{
			public:

				ThreadMemoryAllocator( );

				/// Allocate an object of \a size.
				HK_FORCE_INLINE void* allocate( AllocationType objectType, int size ){ return hkThreadMemory::getInstance().allocateChunkConstSize(size, HK_MEMORY_CLASS_BASE);}

				/// Free an object.
				HK_FORCE_INLINE void deallocate( AllocationType objectType, void* data, int size ){ hkThreadMemory::getInstance().deallocateChunkConstSize( (data), size, HK_MEMORY_CLASS_BASE); }
		};


		/// Collector Interface
		class Collector
		{
			public:
				/// Virtual destructor
				virtual ~Collector() {}

				/// Called by the hkBoundingVolumeTree whenever a leaf node is hit.
				virtual void addLeaf( const Node* leaf ) = 0;

			public:

				hkVector4 m_earlyOutHitFraction;	///
		};


		/// Collector which stores all hits in an array.
		class AllHitsCollector: public Collector
		{
			public:

				inline void addLeaf( const Node* leaf ){ m_hits.pushBack( leaf->m_leafData.m_userData); }

			public:

				hkInplaceArray<void*,128> m_hits;	///
		};


		/// Collector which stores all hits in a user defined preallocated buffer.
		/// When the buffer overflows, it simply ignores any further hits.
		class AllHitsBufferCollector: public Collector
		{
			public:

				/// Constructor
				AllHitsBufferCollector( void** buffer, int bufferCapacity ){ m_hits = buffer; m_numHits = 0; m_hitsCapacity = bufferCapacity; }

				HK_FORCE_INLINE void addLeaf( const Node* leaf )
				{
					if ( m_numHits < m_hitsCapacity )
					{
						m_hits[ m_numHits ] = leaf->m_leafData.m_userData;
						m_numHits++;
					}
				}

			public:

				void** m_hits;			///
				int m_numHits;			///
				int m_hitsCapacity;		///
		};


		/// This class is used as an input structure for castRay().
		struct CastRayInput
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_COLLIDE, CastRayInput );

			hkVector4 m_from;			///
			hkVector4 m_to;				///
			hkUint32  m_filterInfo; 	///
		};


		/// Collect all nodes and leaves in the tree.
		inline void collectNodes( Node* node, Node**leavesOut, Node**internalNodesOut )
		{
			while(1)
			{
				if ( !node->isLeaf() )
				{
					*internalNodesOut = node;
					internalNodesOut++;
					node = node->m_nodeData.m_children[0];
					continue;
				}
				*leavesOut = node;
				leavesOut++;
				node = node->m_skipNode;
				if ( !node )
				{
					break;
				}
			}
		}
};


#endif // HK_BOUNDING_VOLUME_TREE_H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
