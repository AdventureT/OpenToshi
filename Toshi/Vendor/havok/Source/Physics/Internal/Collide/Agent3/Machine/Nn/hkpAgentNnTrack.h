/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_COLLISION_AGENT_TRACK_H
#define HK_COLLIDE2_COLLISION_AGENT_TRACK_H

#include <Common/Base/hkBase.h>
#include <Physics/Internal/Collide/Agent3/hkpAgent3.h>
typedef void hkpAgentData;
struct hkpAgentNnEntry;
struct hkpAgentNnSector; 
class hkpCollidable;

#if ( HK_POINTER_SIZE == 4 )
#	define HK_AGENT3_AGENT_SIZE 128
#	define HK_AGENT3_SECTOR_SIZE 512
#else //XXX
#	define HK_AGENT3_AGENT_SIZE 160
#	define HK_AGENT3_SECTOR_SIZE 960
#endif
#define HK_AGENT3_AGENTS_PER_SECTOR ((HK_AGENT3_SECTOR_SIZE-16)/HK_AGENT3_AGENT_SIZE)


	// Make sure if you use this entry that you pad to get a 16 byte alignment
	// Notes: 
	//
	//   HK_POINTER_SIZE == 4
	//   --------------------
	//   - sizeof(hkpAgentEntry) = 8
	//   - sizeof(hkpAgentNnEntry) = 8 + 20
	//
	//   HK_POINTER_SIZE == 8
	//   --------------------
	//   - sizeof(hkpAgentEntry) = 16
	//   - sizeof(hkpAgentNnEntry) = 16 + 32
struct hkpAgentNnEntry: hkpAgentEntry
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkpAgentNnEntry );

	hkObjectIndex		m_agentIndexOnCollidable[2]; // this value must not be written back from spu!
	hkpContactMgr*		m_contactMgr;

	hkUchar				m_collisionQualityIndex;
	hkUchar				m_forceCollideOntoPpu;	// only used if the platform has an spu
	hkUchar				m_padding[2];
	// Additional 4 bytes padding on 64-bit platforms
	hkpLinkedCollidable*	m_collidable[2];

	inline hkpCollidable*  getCollidableA(){ return reinterpret_cast<hkpCollidable*>(m_collidable[0]); }
	inline hkpCollidable*  getCollidableB(){ return reinterpret_cast<hkpCollidable*>(m_collidable[1]); }
};


struct hkpAgentNnSector
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_AGENT, hkpAgentNnSector);
	
	hkUint8 m_data[ HK_AGENT3_SECTOR_SIZE ];

	hkpAgentNnEntry* getBegin()
	{
		return reinterpret_cast<hkpAgentNnEntry*>( &m_data[0] );
	}

	hkpAgentNnEntry* getEnd()
	{
		return reinterpret_cast<hkpAgentNnEntry*>( &m_data[HK_AGENT3_SECTOR_SIZE] );
	}
};

struct hkpAgentNnTrack
{
	hkpAgentNnTrack( )
	{
		m_bytesUsedInLastSector = HK_AGENT3_SECTOR_SIZE;
		HK_ASSERT2(0xf0ff0088, HK_AGENT3_SECTOR_SIZE % HK_AGENT3_AGENT_SIZE == 0, "SectorSize must be a multiple of agentSize");
	}

	int getSectorSize( int sectorIndex ) const
	{
		if ( sectorIndex+1 == m_sectors.getSize())
		{
			return m_bytesUsedInLastSector;
		}
		return HK_AGENT3_SECTOR_SIZE;
	}


	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_CDINFO, hkpAgentNnTrack );

	hkUint32	m_bytesUsedInLastSector;

	hkInplaceArray<hkpAgentNnSector*,1> m_sectors; 
};





#endif // HK_COLLIDE2_COLLISION_AGENT_TRACK_H


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
