#pragma once
#include <Toshi/T2SList.h>
#include "Entities/AWaypointData.h"

class APropPlatform;

//-----------------------------------------------------------------------------
// Purpose: the platforms grouped by waypoint set (4 bytes): a group ticks while one of its platforms is above the low
// LOD (or forced, APropPlatform::TicksAtLowLOD), and the platforms of a set queue behind each other (APropPlatform::QueuesBehindOthers).
// NOTE: not a TSingleton in retail
//-----------------------------------------------------------------------------
class APlatformManager
{
public:
	// The node of a platform (APropPlatform::m_oPlatformNode)
	struct PlatformNode
	    : public Toshi::T2GenericSList::Node
	{
		APropPlatform* m_pPlatform; // 0x4
		void*          m_Group;     // 0x8 Group
	};

	// The platforms of a waypoint set (0xC bytes)
	struct Group
	    : public Toshi::T2GenericSList::Node
	{
		AWaypointData::Set*   m_pSet;       // 0x4
		Toshi::T2GenericSList m_oPlatforms; // 0x8 PlatformNode
	};

public:
	APlatformManager();
	~APlatformManager();

	void AddPlatform( APropPlatform* a_pPlatform, AWaypointData::Set* a_pSet );
	void RemovePlatform( APropPlatform* a_pPlatform, AWaypointData::Set* a_pSet );

	// Sets the platforms of each group ticking while one of them is
	void Tick();

	// Whether another platform of a_pGroup is ahead of a_iPlatformID on the link a_iFrom-a_iTo within a_fDistance
	static TBOOL IsPathBlocked( Group* a_pGroup, TINT a_iPlatformID, TINT a_iFrom, TINT a_iTo, TFLOAT a_fDistance );

	static APlatformManager* GetSingleton() { return ms_Singleton; }

	// Unlinks the platforms of every group (AInGameState::OnRemoval, before the entities are destroyed)
	void ClearGroups();

private:
	friend class ACivSpawner; // ProcessParams makes the groups of its linked splines

	Group* GetGroup( AWaypointData::Set* a_pSet );

private:
	static APlatformManager* ms_Singleton;

private:
	Toshi::T2GenericSList m_oGroups; // 0x0 Group
};
