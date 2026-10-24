#pragma once
#include "Entities/AVisibleEntity.h"

struct PaintableBlock;

TOSHI_NAMESPACE_START
class TModelInstance;
class TMaterial;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: spawns pooled civilians (ACiv) on its linked splines.
//-----------------------------------------------------------------------------
class ACiv;

class ACivSpawner
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( ACivSpawner, AVisibleEntity );

	static TCONSTEXPR_VAR TINT CIVSPAWNER_MAXCIVCOUNT = 8;

	// The doorway textures the spawner's model can swap to (m_uiSpawnerFlags & 3, 0: its own)
	static TCONSTEXPR_VAR TINT CIVSPAWNER_NOOFSWAPTEXTURES = 3;

	// The custom render method of a TModelInstance
	typedef void( __stdcall* t_RenderCallback )( Toshi::TModelInstance* a_pModelInstance, void* a_pUserData );

	// The waypoint of a slot on the linked splines (8 bytes): the set and the node of AWaypointData
	struct SplineEntry
	{
		TINT   m_iSetIndex;  // 0x00
		TINT16 m_iNodeIndex; // 0x04
		TINT16 m_iUnk06;     // 0x06
	};

	struct LinkedSplines
	{
		TINT         m_iNumLinkedSplines; // 0x00
		SplineEntry* m_pSplines;          // 0x04
	};

public:
	ACivSpawner();
	virtual ~ACivSpawner() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void Create() override;                                                                   // +0x1C
	virtual void Destroy() override;                                                                  // +0x24
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 00761460
	virtual ACollisionObject* GetCollisionObject() override { return m_pCollisionObject; } // +0x70
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                    // +0xC8
	virtual void              Despawn( TBOOL a_bPlayEffect ) override;                     // +0xE4
	virtual const TCHAR*      GetModelName() override;                                     // +0xEC

	// The cell instance of the spawner's building (m_iBuildingInstance, ATerrain2::GetCellInstance)
	TINT16 GetBuildingInstance() const { return m_iBuildingInstance; }

	// Spawns the civilian of a slot: the wrapper goes at the slot's place (or its spline waypoint) and the slot is
	// flagged (0x20); TRUE when a wrapper was free. a_bUseStateTable starts the ACiv in its slot's state
	TBOOL SpawnCiv( TINT a_iPos, TBOOL a_bUseStateTable );

	// TRUE when the slot has a civilian (the flag 0x20 of its byte)
	TBOOL IsSpawned( TINT a_iPos ) const { return ( m_auiSlotFlags[ a_iPos ] & 0x20 ) != 0; }

	// The paintable block of the spawner (ABuildingManager::SetupPaintableBlocks)
	void SetBlock( PaintableBlock* a_pBlock ) { m_pBlock = a_pBlock; }

	// TRUE when the spawner has linked splines (flag 0x8 of m_uiSpawnerFlags, set by ProcessParams)
	TBOOL HasLinkedSplines() const { return ( m_uiSpawnerFlags & 0x8 ) != 0; }

	// The number of linked splines
	// $deBlob: FUNCTION 0057b5e0
	TINT GetNumLinkedSplines() const
	{
		TASSERT( m_pLinkedSplines->m_iNumLinkedSplines > 0 );
		return m_pLinkedSplines->m_iNumLinkedSplines;
	}

	// Read by ABuildingManager::DisperseCivs
	TINT8          GetActiveCount() const { return m_iActiveCount; }
	TUINT8         GetSpawnerFlags() const { return m_uiSpawnerFlags; }
	LinkedSplines* GetLinkedSplines() const { return m_pLinkedSplines; }

	// Releases the civilians of the slots, then gives the spawner a_iNumCivs civilians (or one per linked spline when
	// a_bUseSplines and the spawner has them), each new slot with a random colour
	void SetNumCivs( TINT a_iNumCivs, TBOOL a_bUseSplines );

	// Starts spawning the civilians (Tick state 1) unless the spawner has FLAG_Despawned: it must be idle (state 0)
	// with fewer than CIVSPAWNER_TOTALSPAWN civilians (ABuildingManager::OnBlockBuildingPainted)
	void StartSpawning();

	// One more of the spawner's civilians was transformed (ACiv::Transform of a pooled one). At the 8th the spawner is
	// done (0x4 of m_uiSpawnerFlags): a pickup appears in front of it (not on sprint levels) and its civilians walk to their
	// linked splines
	void OnCivTransformed( ACiv* a_pCiv );

	// The spawner gives up the transformed civilian a_pCiv (it exploded): its slot is released and the slots behind it
	// move up
	void ReleaseTransformed( ACiv* a_pCiv );

	// Picks a random doorway swap texture (1 to 3, m_uiSpawnerFlags & 3) if it has none
	void PickSwapTexture();

	// Paints the doorway (the model's local ambient colour) in a_uiColour and the building of m_iBuildingInstance with it, or
	// unpaints them for the ink (0x10). a_bOnlyIfUnpainted: only a doorway that isn't painted. TRUE when painted
	TBOOL PaintDoorway( TUINT32 a_uiColour, TBOOL a_bOnlyIfUnpainted );

protected:
	void CreateCollisionObject();

	// The doorway swap texture number (m_uiSpawnerFlags & 3, SpawnerFlag_TexNoMask)
	// $deBlob: FUNCTION 007616e0
	void SetTexNo( TUINT8 a_iTexNo )
	{
		TASSERT( 0 == ( a_iTexNo & ~3 ) );
		m_uiSpawnerFlags = ( m_uiSpawnerFlags & ~3 ) | a_iTexNo;
	}

	// Adds a colour to the flags of a slot (FlagMask_Colour 0x1F)
	// $deBlob: FUNCTION 00761680
	void AddSlotColour( TINT a_iPos, TUINT8 a_uiColour )
	{
		TASSERT( 0 == ( a_uiColour & 0xE0 ) );
		m_auiSlotFlags[ a_iPos ] |= a_uiColour;
	}

	// Gives the civilian of a slot back to the pool (its colour goes to the slot's flags, the spawned flag is cleared)
	void ReleaseSlot( TINT a_iPos );

	// The custom render method of the model: renders it with its doorway swap texture (m_uiSpawnerFlags & 3), else with the
	// render method it had
	static void __stdcall RenderDoorway( Toshi::TModelInstance* a_pModelInstance, void* a_pUserData );

	// The materials of the swap textures, made from the model's material by the first ProcessParams
	static Toshi::TMaterial* ms_apSwapMaterials[ CIVSPAWNER_NOOFSWAPTEXTURES ];

protected:
	TINT8             m_iActiveCount;                           // 0x9C
	TINT8             m_iNumTransformed;                        // 0x9D
	TUINT8            m_uiSpawnerFlags;                         // 0x9E
	TFLOAT            m_fSpawnDelay;                            // 0xA0 random in [0.2, 0.6]
	ACollisionObject* m_pCollisionObject;                       // 0xA4
	LinkedSplines*    m_pLinkedSplines;                         // 0xA8
	PaintableBlock*   m_pBlock;                                 // 0xAC
	TINT16            m_iBuildingInstance;                      // 0xB0 -1, the first linked cell instance (the door surround)
	TINT16            m_iSecondBuildingInstance;                // 0xB2 -1, the second linked cell instance
	void*             m_apCivs[ CIVSPAWNER_MAXCIVCOUNT ];       // 0xB4
	TUINT8            m_auiSlotFlags[ CIVSPAWNER_MAXCIVCOUNT ]; // 0xD4
	t_RenderCallback  m_fnRender;                               // 0xDC the model's custom render method before ProcessParams
	void*             m_pRenderUserData;                        // 0xE0
};
