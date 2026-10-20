#pragma once
#include "ABuildingInstance.h"
#include "Entities/ABaseEntity.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>

namespace Toshi
{
class TParticleEffect;
} // namespace Toshi

class ACivSpawner;
class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: a paintable block of the level (the "PaintableBlocks" symbol of PaintableBlockData.trb): the cell instances
// of its buildings (converted to their ABuildingInstance by ABuildingManager::SetupPaintableBlocks), the entities
// linked to it (their ACivSpawner::m_pBlock is set to it) and its type in the top byte of m_uiFlags.
//-----------------------------------------------------------------------------
struct PaintableBlock
{
	// m_uiFlags
	enum FlagMask : TUINT32
	{
		FlagMask_EntityID   = 0x0000FFFF, // the entity triggered when the block completes (TINT16, -1: none)
		FlagMask_NumColours = 0x00FF0000, // the number of colours the multicolour bonus was given for
		FlagMask_Type       = 0x0F000000,
		FlagMask_Completed  = 0x10000000, // every building of the block is painted
		FlagMask_Dispersed  = 0x20000000, // its civilians were shared out to its spawners (DisperseCivs)
	};

	// The number of block types (the assert of OnBlockBuildingPainted: the type indexes the locale strings 0x6C...);
	// type 7 is a jelly block (its buildings take the colour of the painted one)
	static TCONSTEXPR_VAR TINT NUMOF = 8;

	TUINT GetType() const { return ( m_uiFlags & FlagMask_Type ) >> 24; }
	TINT  GetEntityID() const { return TINT16( m_uiFlags & FlagMask_EntityID ); }
	TINT  GetNumColours() const { return ( m_uiFlags & FlagMask_NumColours ) >> 16; }
	void  SetNumColours( TINT a_iNumColours ) { m_uiFlags = ( m_uiFlags & ~FlagMask_NumColours ) | ( TUINT32( a_iNumColours & 0xFF ) << 16 ); }

	Toshi::TVector4 m_vPosition;      // 0x00 the centre of its instances (divided by their count)
	TINT*           m_paiInstances;   // 0x10 m_uiNumInstances cell instances (or their ABuildingInstance, when converted)
	TUINT16*        m_pauiEntities;   // 0x14 m_uiNumEntities entity IDs (ABaseEntity::FindIDEntity)
	TUINT16         m_uiNumInstances; // 0x18
	TUINT16         m_uiNumEntities;  // 0x1A
	TUINT32         m_uiFlags;        // 0x1C FlagMask: the entity ID, the number of colours, the type (byte 0x1F & 0xF)
	TINT            m_iSSCamera;      // 0x20 the SS camera (ASSCameraFunction ID) played when the block completes (-1: none)
	TUINT8          m_aUnk24[ 0x0C ]; // 0x24
};
TSTATICASSERT( sizeof( PaintableBlock ) == 0x30 );

// The blocks of the level: the pointer to the first block and their count (the symbol's data, 0x08 bytes)
struct PaintableBlockList
{
	PaintableBlock* m_pBlocks;    // 0x00
	TINT            m_iNumBlocks; // 0x04
};

//-----------------------------------------------------------------------------
// Purpose: the buildings and landmarks of the level (created by AAppInitState): the pools of
// ABuildingInstance/ALandmarkInstance pointers (sized by ATerrain2Interface's StartInstanceCreation), the active
// (updated) instances, the free update and jelly instances, the paintable blocks.
//-----------------------------------------------------------------------------
class ABuildingManager
    : public Toshi::TSingleton<ABuildingManager>
{
public:
	// 0x1C bytes: a delayed colour change of a building of a jelly block (block type 7), added by
	// OnBlockBuildingPainted and run by Update once m_fDelay is over
	struct JellyInstance
	    : public Toshi::T2DList<JellyInstance>::Node
	{
		TINT               m_iPlayer;   // 0x08
		TUINT32            m_uiColour;  // 0x0C
		ABuildingInstance* m_pBuilding; // 0x10
		TBOOL              m_bDone;     // 0x14
		TFLOAT             m_fDelay;    // 0x18
	};

	// 0x10 bytes; also the ticked cell instances (AddTickInstance: the tree transforms)
	struct UpdateInstance
	    : public Toshi::T2DList<UpdateInstance>::Node
	{
		TFLOAT m_fTime;     // 0x08 since it was added
		TINT16 m_iInstance; // 0x0C the cell instance (ATerrain2)
	};

	static TCONSTEXPR_VAR TINT NUM_JELLY_INSTANCES  = 32;
	static TCONSTEXPR_VAR TINT NUM_UPDATE_INSTANCES = 64;

public:
	ABuildingManager();
	~ABuildingManager();

	// Allocates the pools for the instances of the level
	void Init( TINT a_iMaxBuildings, TINT a_iMaxLandmarks );

	// Frees the pools of the level, the update instances, the tree emerge effects and the block particle effects (the
	// level unload, AInGameState::OnRemoval); m_pUpdateInstances isn't cleared, as in retail.
	void Deinit();

	void AddBuilding( ABuildingInstance* a_pBuildingInst );
	void RemoveBuilding( ABuildingInstance* a_pBuildingInst );

	// Resets the buildings, landmarks and paintable blocks and frees the jelly and update instances (the level restart,
	// AInGameState)
	void Reset();

	PaintableBlock* GetPaintableBlock( TINT a_iIndex );
	void            ResetPaintableBlocks();

	// Updates the active instances (AInGameState::OnUpdate)
	void Update( TFLOAT a_fDeltaTime );

	// Adds the instance to the active (updated) instances if it isn't already
	void AddActiveInstance( ABuildingInstance* a_pBuildingInst );

	// The position of the paintable block with civilians nearest to the current camera that isn't complete and isn't a
	// jelly block (ACheatManager::Cheat_ShowCivBlockPosition), TFALSE if none
	TBOOL GetNearestCivBlockPosition( Toshi::TVector4& a_rvOutPosition );

	// Ticks the cell instance with the tick callback of its type until it returns TRUE (the tree transforms)
	void AddTickInstance( TerrainTypes::CellInstance* a_pInstance );

	// Runs the paint callback of the instance's type on it with a_pBlob (TNULL, or not an ABlobEntity: player 0's blob),
	// the building transformation; $deBlob 00578cc0. NOTE: the name comes from the assert ("Tried to trigger a building
	// transformation")
	static void TriggerTransformation( TerrainTypes::CellInstance* a_pInstance, ABaseEntity* a_pBlob );

	// Sends a_eMessage (data 0) to the entities of the instance's Links (ALandmarkInstance::Links,
	// m_pLinks) and runs TriggerTransformation on its linked instances, with the owner's blob if the instance is a
	// building with m_iMultiplayerOwner >= 0, else the current player's blob; $deBlob 00578d40. NOTE: name invented
	static void TriggerLinks( TerrainTypes::CellInstance* a_pInstance, AEntityIOPacket::MESSAGE a_eMessage );

	// The building (ABuildingInstance) of the terrain instance a_iInstance
	ABuildingInstance* GetBuildingInstance( TINT a_iInstance );

	// The landmark of the terrain instance a_iInstance (ALandmarkChallenge::Create)
	static ABuildingInstance* __stdcall GetLandmark( TINT a_iInstance );

	// Runs the touch callback of the instance's type with a_pBlob touching it at a_pPosition; $deBlob 00751e20.
	static void OnInstanceTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition );

	// Uncounts the civilians of the entities of every landmark (ALandmarkInstance::UncountCivs)
	static void UncountLandmarkCivs();

	// Paints every paintable building (IsAPaintableBuilding) in a_uiColour (a_uiARGB its colour) with no owner and
	// checks their blocks (the multiplayer)
	void PaintAllBuildings( TUINT8 a_uiColour, TUINT32 a_uiARGB );
	// Toggles the pulse of every paintable building (each starts after a random delay of up to 2 seconds) / scales them
	// back
	void TogglePulseAllBuildings();

	// PaintAllBuildings in a_iColour and its ABlobColourComponent::ms_auiColours
	void PaintAllBuildings( TINT a_iColour );

	// Game type 2: a random paintable building in a_uiColour with the best (lowest) rank (Get_MP_BuildingRank) that
	// isn't one of the next paintable ones, one within a_fRadius of a_rPosition if there are such
	ABuildingInstance* GetRandomPaintableBuilding( TUINT32 a_uiColour, const Toshi::TVector4& a_rPosition, TFLOAT a_fRadius );

	// The team mode (game type 3): paints the buildings of the teams 1-4 in the colour of the players a_iPlayer1-4 (-1:
	// the team isn't playing) with their pattern; returns the number of buildings painted
	TINT PaintTeamBuildings( TINT a_iPlayer1, TINT a_iPlayer2, TINT a_iPlayer3, TINT a_iPlayer4 );

	// a_pBuilding of a_rBlock was painted by a_iPlayer (-1: none). A jelly block (type 7) queues its other buildings to
	// take the colour (m_oJellyInstances). Otherwise, once all its buildings are painted the block completes (the
	// score, the banner, its entity, the sound, the block graphics, the civilians of its nearest spawners and its
	// checkpoint); a completed block repainted in more colours gives the multicolour bonus.
	void OnBlockBuildingPainted( ABuildingInstance* a_pBuilding, TINT a_iPlayer, PaintableBlock& a_rBlock );

	// Drops the queued colour change of a_pBuilding (a building of a jelly block) back to the free list
	void RemoveJellyInstance( ABuildingInstance* a_pBuilding );

	// Shares the civilians of a completed block out to its spawners once all of them (but a_pSpawner) have the low LOD
	// and none is spawning; the block is then dispersed.
	void DisperseCivs( PaintableBlock& a_rBlock, ACivSpawner* a_pSpawner );

	// Reads the paintable blocks of the level ("PaintableBlocks" of PaintableBlockData.trb), converts their instances to
	// their buildings, links the entities of the blocks and makes the particle effects of the block graphics; $deBlob
	void SetupPaintableBlocks();

	TINT GetNumBuildings() const { return m_iNumBuildings; }

	// Shows or hides the HUD scenes over the triggers of every landmark
	void SetLandmarkTargetScenesVisible( TBOOL a_bVisible );
	// $deBlob: FUNCTION 0076a8d0
	TINT GetNumLandmarks() const { return m_iNumLandmarks; }
	// The landmark a_iIndex of the pool (ABlobZJumpComponent's landmark search)
	// $deBlob: FUNCTION 0076a870
	ABuildingInstance* GetLandmarkAt( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumLandmarks );
		return m_apLandmarks[ a_iIndex ];
	}

	// $deBlob: FUNCTION 007bfe80
	ABuildingInstance* GetBuilding( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumBuildings );
		return m_aInstancePool[ a_iIndex ];
	}
	TBOOL IsBlobOnLandmark() const { return m_bBlobOnLandmark; } // read by the blob camera: its fixed cameras with flag 2
	// $deBlob: FUNCTION 007522c0
	void SetBlobOnLandmark( TBOOL a_bUnk34 ) { m_bBlobOnLandmark = a_bUnk34; } // set while Blob is on a landmark (ABlobEntity::EnterLandmark)

public:
	Toshi::T2DList<ABuildingInstance::Node> m_oActiveInstances;                       // 0x00
	TINT                                    m_iMaxBuildings;                          // 0x08
	TINT                                    m_iMaxLandmarks;                          // 0x0C
	TINT                                    m_iNumBuildings;                          // 0x10
	TINT                                    m_iNumLandmarks;                          // 0x14
	ABuildingInstance**                     m_aInstancePool;                          // 0x18
	ABuildingInstance**                     m_apLandmarks;                            // 0x1C
	UpdateInstance*                         m_pUpdateInstances;                       // 0x20
	Toshi::T2DList<UpdateInstance>          m_FreeUpdateInstanceList;                 // 0x24
	Toshi::T2DList<UpdateInstance>          m_oUsedUpdateInstances;                   // 0x2C
	TBOOL                                   m_bBlobOnLandmark;                        // 0x34
	PaintableBlockList*                     m_apPaintableBlocks;                      // 0x38
	Toshi::TParticleEffect**                m_pBlockParticleEffects;                  // 0x3C one per block graphic
	Toshi::TParticleEffect*                 m_pBlockGraphicEffect;                    // 0x40 "Blob_Block_Graphic"
	TINT                                    m_iNumBlockParticleEffects;               // 0x44
	JellyInstance                           m_aJellyInstances[ NUM_JELLY_INSTANCES ]; // 0x48
	Toshi::T2DList<JellyInstance>           m_oJellyInstances;                        // 0x3C8
	Toshi::T2DList<JellyInstance>           m_oFreeJellyInstances;                    // 0x3D0
	TWCHAR                                  m_wszBlockMessage[ 0x80 ];                // 0x3D8 the text of the block banner (OnBlockBuildingPainted)
};                                                                                    // 0x4D8
