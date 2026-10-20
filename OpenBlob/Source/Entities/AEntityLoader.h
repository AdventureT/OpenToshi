#pragma once
#include "ABaseEntity.h"
#include "AEntityParams.h"

#include <File/TTRB.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Purpose: creates the level's entities (AEntityLoader.cpp) from the "EntitiesMain" symbol of the level's entity data
// block, a few per frame (at most 5 ms of work per Update), then runs their post-create step.
//-----------------------------------------------------------------------------
class AEntityLoader
{
public:
	enum STATE
	{
		STATE_Loading,            // 0 reads "EntitiesMain"
		STATE_Creating,           // 1 creates the entities
		STATE_PostCreate,         // 2
		STATE_LoadChallenges,     // 3
		STATE_PostCreateEntities, // 4 walks the entity list
		STATE_Done,               // 5 done (Update returns 0)
	};

	// One entity of "EntitiesMain" (0x28 bytes): its type and properties, matrix, bounding box and ID
	struct EntityData
	    : public AEntityParams
	{
		Toshi::TMatrix44* m_pMatrix;                 // 0x0C
		Toshi::TVector4*  m_pBBox;                   // 0x10
		TINT              m_iID;                     // 0x14 ABaseEntity::SetID
		TINT              m_iTargetCount;            // 0x18 GetTargetCount
		TINT*             m_pTargets;                // 0x1C entity IDs
		TINT              m_iWaypointSetTargetCount; // 0x20 GetWaypointSetTargetCount
		TUINT32*          m_pWaypointSetTargets;     // 0x24 two words per set

		// $deBlob: FUNCTION 00761590
		TINT    GetWaypointSetTargetCount() const { return m_iWaypointSetTargetCount; }
		TUINT32 GetWaypointSetTarget( TINT a_iSetIndex, TINT a_iWord ) const
		{
			TASSERT( a_iSetIndex >= 0 && a_iSetIndex < m_iWaypointSetTargetCount );
			return m_pWaypointSetTargets[ a_iSetIndex * 2 + a_iWord ];
		}

		// The two words of a waypoint set target
		// $deBlob: FUNCTION 007615a0
		TUINT32 GetWaypointSetTargetFirst( TINT a_iSetIndex ) const
		{
			TASSERT( a_iSetIndex >= 0 && a_iSetIndex < m_iWaypointSetTargetCount );
			return m_pWaypointSetTargets[ a_iSetIndex * 2 ];
		}

		// $deBlob: FUNCTION 00761530
		TUINT32 GetWaypointSetTargetSecond( TINT a_iSetIndex ) const
		{
			TASSERT( a_iSetIndex >= 0 && a_iSetIndex < m_iWaypointSetTargetCount );
			return m_pWaypointSetTargets[ a_iSetIndex * 2 + 1 ];
		}

		// $deBlob: FUNCTION 00772ee0
		TINT GetTargetCount() const { return m_iTargetCount; }

		// $deBlob: FUNCTION 00772ef0
		TINT GetTarget( TINT a_iIndex ) const
		{
			TASSERT( a_iIndex >= 0 && a_iIndex < m_iTargetCount );
			return m_pTargets[ a_iIndex ];
		}

		const TCHAR* GetTypeName() const { return m_szTypeName; }
	};

	struct ParamList
	{
		EntityData* m_pEntities;    // 0x00
		TINT        m_iEntityCount; // 0x04
	};

public:
	AEntityLoader();

	// $deBlob: FUNCTION 00793490
	~AEntityLoader() { m_pTRB = TNULL; }

	// Returns FALSE once every entity is created
	TBOOL Update( TFLOAT a_fDeltaTime );

private:
	// Whether the entity isn't used in the current level type ("SkipInLevelType" flags)
	static TBOOL SkipInLevelType( const EntityData& a_rEntityData );

private:
	Toshi::TTRB*                          m_pTRB;       // 0x00 DataBlock_Entities
	TINT                                  m_eState;     // 0x04 STATE
	ParamList*                            m_pParamList; // 0x08
	TINT                                  m_iCurEnt;    // 0x0C
	Toshi::T2DList<ABaseEntity>::Iterator m_itEntity;   // 0x10 over the created entities (STATE_PostCreateEntities)
};
