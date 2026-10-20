/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_PAIR_COLLISION_FILTER_H
#define HK_COLLIDE2_PAIR_COLLISION_FILTER_H


#include <Common/Base/Container/PointerMap/hkPointerMapBase.h>
#include <Physics/Collide/Filter/hkpCollisionFilter.h>


class hkpEntity;


/// A simple pairwise collision filter that allows to enable/disable collisions between two dedicated entities.
/// Each pair maintains a counter, i.e. you have to 'enable' as many times as 'disable' to actually re-enable
/// collision detection between a pair of entities.
class hkpPairCollisionFilter : public hkpCollisionFilter	
{
	public:

#if HK_POINTER_SIZE == 4
		typedef hkUint64 PairFilterKey;
#else
		struct PairFilterKey
		{
			hkUint64 m_a;
			hkUint64 m_b;
		};

		struct PairFilterPointerMapOperations
		{
			inline unsigned hash( PairFilterKey key, unsigned modulus )
			{
				// using Knuth's multiplicative golden hash
				// the lowest four bits on the address are ignored as most addresses will be 16-byte aligned
				// finally we truncate the hash value as hkPointerMap can only handle 32 bit 
				hkUint64 combinedKey = (key.m_a + (key.m_b << 28)) >> 4;
				return (unsigned)(((combinedKey * 0x9e3779b97f4a7c13LL) & modulus) & 0xffffffff);
			}
			inline void invalidate( PairFilterKey& key )
			{
				key.m_a = 0xffffffffffffffff;
				key.m_b = 0xffffffffffffffff;
			}
			inline hkBool32 isValid( PairFilterKey key )
			{
				return key.m_a != 0xffffffffffffffff && key.m_b != 0xffffffffffffffff;
			}
			inline hkBool32 equal( PairFilterKey key0, PairFilterKey key1 )
			{
				return key0.m_a == key1.m_a && key0.m_b == key1.m_b;
			}
		};
#endif


	public:

		//HK_DECLARE_REFLECTION();

		 hkpPairCollisionFilter(const hkpCollisionFilter* childFilter = HK_NULL);
		 hkpPairCollisionFilter( class hkFinishLoadedObjectFlag flag ) {}
		~hkpPairCollisionFilter();

			/// Enable collisions between entity A and entity B. Returns the number of times the pair is still 'disabled'.
			/// ###ACCESS_CHECKS###( [entityA->getWorld(),HK_ACCESS_RW] );
		int enableCollisionsBetween(const hkpEntity* entityA, const hkpEntity* entityB);

			/// Disable collisions between entity A and entity B. Returns the number of times the pair is now 'disabled'.
			/// ###ACCESS_CHECKS###( [entityA->getWorld(),HK_ACCESS_RW] );
		int disableCollisionsBetween(const hkpEntity* entityA, const hkpEntity* entityB);

			/// This implementation of isCollisionEnabled() checks if the two supplied entities are allowed to collide or not.
		hkBool isCollisionEnabled( const hkpEntity* entityA, const hkpEntity* entityB ) const;

			/// This implementation of isCollisionEnabled() simply forwards to the entity/entity variant.
		virtual hkBool isCollisionEnabled( const hkpCollidable& collidableA, const hkpCollidable& collidableB ) const;

			/// This implementation of isCollisionEnabled() simply forwards to the entity/entity variant.
		virtual	hkBool isCollisionEnabled( const hkpCollisionInput& input, const hkpCdBody& a, const hkpCdBody& b, const HK_SHAPE_CONTAINER& bContainer, hkpShapeKey bKey  ) const;

		// hkpShapeCollectionFilter interface forwarding
		virtual hkBool isCollisionEnabled( const hkpCollisionInput& input, const hkpCdBody& collectionBodyA, const hkpCdBody& collectionBodyB, const HK_SHAPE_CONTAINER& containerShapeA, const HK_SHAPE_CONTAINER& containerShapeB, hkpShapeKey keyA, hkpShapeKey keyB ) const;

			/// This implementation of isCollisionEnabled() will always return true.
		virtual hkBool isCollisionEnabled( const hkpShapeRayCastInput& aInput, const hkpShape& bShape, const HK_SHAPE_CONTAINER& bContainer, hkpShapeKey bKey ) const;

			/// This implementation of isCollisionEnabled() will always return true.
		virtual hkBool isCollisionEnabled( const hkpWorldRayCastInput& inputA, const hkpCollidable& collidableB ) const;

	private:

			// Constructs the 64bit hash key from the entities' pointers.
		HK_FORCE_INLINE void calcKey(const hkpEntity* entityA, const hkpEntity* entityB, PairFilterKey& keyOut) const;

	public:

#if HK_POINTER_SIZE == 4
		class hkPointerMapBase<PairFilterKey> m_disabledPairs; //+nosave
#else
		class hkPointerMapBase<PairFilterKey, hkUint64, PairFilterPointerMapOperations> m_disabledPairs; //+nosave
#endif

		const hkpCollisionFilter* m_childFilter;
};


#include <Physics/Utilities/Collide/Filter/pair/hkpPairCollisionFilter.inl>


#endif // HK_COLLIDE2_PAIR_COLLISION_FILTER_H

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
