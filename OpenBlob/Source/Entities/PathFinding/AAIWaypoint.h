#pragma once
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: a point of an AI path (PathFinding\AAIWaypoint.cpp): where to go, how (FLAG_*: the motor's move type) and
// how close is close enough. They come from a global free list that grows by 64 at a time.
//-----------------------------------------------------------------------------
class AAINode;

class AAIWaypoint
    : public Toshi::T2DList<AAIWaypoint>::Node
{
public:
	// m_uiFlags
	enum FLAG : TUINT8
	{
		FLAG_NoShortcut = 0x1,
		FLAG_Jump       = 0x2, // move type 1
		FLAG_Fly        = 0x4, // move type 2
		FLAG_Climb      = 0x8, // move type 3
		FLAG_Steer      = 0x10,
	};

public:
	AAIWaypoint()
	{
		m_fRadius   = 0.0f;
		m_pNode     = TNULL;
		m_vPosition = Toshi::TVector4::VEC_ZERO;
		m_uiFlags   = 0;
	}

	virtual ~AAIWaypoint(); // +0x00

	// A waypoint of the free list (filled when it's empty)
	static AAIWaypoint* GetFreeWaypoint();
	static void         FillWaypointFreeList();

	static Toshi::T2DList<AAIWaypoint> sm_FreeList;

public:
	TFLOAT          m_fRadius;   // 0x0C
	AAINode*        m_pNode;     // 0x10 the AI network node it was made from
	Toshi::TVector4 m_vPosition; // 0x14
	TUINT8          m_uiFlags;   // 0x24 FLAG
};
