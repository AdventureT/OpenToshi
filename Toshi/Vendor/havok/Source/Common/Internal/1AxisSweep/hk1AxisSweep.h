/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_INTERNAL_1AXIS_SWEEP_H
#define HK_INTERNAL_1AXIS_SWEEP_H

#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

struct hkKeyPair
{
	hkUint32 m_keyA;
	hkUint32 m_keyB;
};

	/// Utility to quickly find overlapping pairs of AABBS
class hk1AxisSweep
{
	public:

		struct AabbInt : public hkAabbUint32
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hk1AxisSweep::AabbInt );

			hkUint32& getKey() { return m_min[3]; }
			const hkUint32& getKey() const { return m_min[3]; }

			HK_FORCE_INLINE bool operator<(const hk1AxisSweep::AabbInt& aabb1) const	{		return this->m_min[0] < aabb1.m_min[0];		}
			void operator=( const AabbInt& other )		{			hkString::memCpy16<sizeof(AabbInt)>( this, &other );			}

			void setEndMarker()
			{
				m_min[0] = hkUint32(-1);
			}

				/// set from a float aabb. The float values will be converted into ordered ints
			void set( const hkAabb& aabb, int key );

			static HK_FORCE_INLINE hkUint32 yzDisjoint( const hk1AxisSweep::AabbInt& aabb, const hk1AxisSweep::AabbInt& other )
			{
				hkUint32 yab = aabb.m_max[1] - other.m_min[1];
				hkUint32 yba = other.m_max[1] - aabb.m_min[1];
				hkUint32 zab = aabb.m_max[2] - other.m_min[2];
				hkUint32 zba = other.m_max[2] - aabb.m_min[2];
				hkUint32 combined = (yab | yba) | (zab | zba);
				return combined & 0x80000000;
			}
		};

			/// This returns all overlapping Aabb pairs, where one aabb is from the first list, and the other from the second list.
			/// Both lists must be appended with four Aabb elements, where aabb.m_min[0] == HK_REAL_MAX.
			/// numA/numB should be equal to the actual number of elements excluding the padding aabbs.
		static int HK_CALL collide( const AabbInt* pa, int numA, const AabbInt* pb, int numB, hkKeyPair* HK_RESTRICT pairsOut, int maxNumPairs, hkPadSpu<int>& numPairsSkipped);

			/// This returns all overlapping Aabb pairs when collide pa with itself
			/// The list must be appended with four Aabb elements, where aabb.m_min[0] == HK_REAL_MAX.
			/// numA should be equal to the actual number of elements excluding the padding aabbs.
			/// The number of pairs which didn't fit into the buffer is written into numPairsSkippedInOut.
		static int HK_CALL collide( const AabbInt* pa, int numA, hkKeyPair* HK_RESTRICT pairsOut, int maxNumPairs, hkPadSpu<int>& numPairsSkippedOut);

			/// Utility function to sort aabbs in place. This implementation is really fast, don't try to beat it
		static void HK_CALL sortAabbs(hk1AxisSweep::AabbInt* aabbs, int size);
};





#endif // HK_INTERNAL_1AXIS_SWEEP_H

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
