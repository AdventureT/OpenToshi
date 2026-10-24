#pragma once
#include "Animation/AModelInstance2.h"
#include "Terrain2/TerrainTypes.h"

class ABlobEntity;

#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: a paintable model instance of the terrain (cell instance type 1, TerrainTypes::InstanceType_Building):
// created by AModel2::CreateInstance, registered with ABuildingManager when its cell instance is created. Blob paints
// it in his colour (m_uiCurrentColour); ink (COLOUR_BLACK) turns it grey again.
// Also the base of ALandmarkInstance and AGrassInstance.
//-----------------------------------------------------------------------------
class ABuildingInstance
    : public AModelInstance2
{
public:
	static TCONSTEXPR_VAR TINT MAXBLOCKS = 2;

	using Node = Toshi::T2DListNodeWrapper<ABuildingInstance>;

	// m_uiFlags (Flags_Pulse, Flags_Locked, Flags_Inked, Flags_Painted)
	enum Flags : TUINT32
	{
		Flags_Painted      = BITFLAG( 0 ), // painted in a colour (counted by RenderLightmappedInstance)
		Flags_Inked        = BITFLAG( 2 ), // inked, fading back to grey (m_fInktTimer)
		Flags_Update       = BITFLAG( 3 ), // stays in ABuildingManager's active instances for another frame
		Flags_Pulse        = BITFLAG( 4 ), // the mesh scale pulses (m_fPulseTimer)
		Flags_ScaleBack    = BITFLAG( 5 ), // the mesh scale goes back to 1
		Flags_Reproject    = BITFLAG( 6 ), // a paint challenge projects it again once its jelly is done (APaintChallenge)
		Flags_Locked       = BITFLAG( 7 ),
		Flags_FadeDone     = BITFLAG( 8 ), // the paint fade (m_fPaintFadeTimer) is over
		Flags_BossPlatform = BITFLAG( 9 ),
		Flags_NotPaintable = BITFLAG( 10 ), // not paintable
		Flags_PaintedOnce  = BITFLAG( 12 ),
	};

	static const TFLOAT s_cfFadeTime;      // the ink time
	static const TFLOAT s_cfPaintFadeTime; // the fade from the paint colour to the dark colour

public:
	ABuildingInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints );
	~ABuildingInstance();

	virtual void UpdateImp( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x00

	// Starts the mesh scale pulse after a_fDelay seconds (the scale must be 1)
	void StartPulse( TFLOAT a_fDelay );
	// Stops the pulse: the mesh scale goes back to 1
	void         StartScaleBack();
	virtual void SetPaintColour( TUINT32 a_uiColour, TINT a_iPlayer, TBOOL a_bInstant, TINT a_iUnk ); // +0x10
	virtual TINT Reset();                                                                             // +0x14

	// The callbacks of TerrainTypes::InstanceType_Building in ATerrain2Interface's per type tables
	static void OnCreated( TerrainTypes::CellInstance* a_pInstance );
	// Blob touched the instance: the touch is kept in m_pPendingBlob/m_vPendingPosition and painted by UpdateImp
	static void OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition );
	static void OnTable97b958( TerrainTypes::CellInstance* a_pInstance );
	static void OnTable97b978( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob );

	// Paints the building in a_pBlob's colour (Blob touched it at a_pPosition)
	void Paint( ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition );

	// Changes the colour of the building (COLOUR_WHITE: grey, COLOUR_BLACK: inked)
	void SetColour( TUINT32 a_uiColour, TINT a_iPlayer, TBOOL a_bJelly, TBOOL a_bUnk2 );

	// A building (cell instance type 1) that isn't a fading one (flag 0x8000) in a colour
	TBOOL IsAPaintableBuilding() const;

	// Game type 2: the order the building gets painted in (ATerrain2's multiplayer data), 0: none
	TINT Get_MP_BuildingRank() const;

	// The multiplayer score of the building (ATerrain2's multiplayer data, MPInstanceData::m_iUnk0), -1 without a
	// terrain instance
	TINT Get_MP_Score() const;

	// The team of the building in the team mode (its terrain instance's m_uiTeam, 0: none)
	// $deBlob: FUNCTION 005774f0
	TINT GetTeam() const { return m_pTerrainInstance != TNULL ? TINT( TINT8( m_pTerrainInstance->m_uiTeam ) ) : 0; }

	// Can a_pBlob paint the building?
	TBOOL CanBePaintedBy( ABlobEntity* a_pBlob );

	void SetBlock( void* a_pBlock );

	// Locks the building (a paint challenge): unpaints it and gives it the locked colour
	void Lock();

	// Unlocks it again (a locked, unpainted, white building)
	void Unlock();

	// Takes the jelly and the paint challenge projection off the building's materials
	void RemoveJellyPass();
	void NotifyCivSpawners();

	// $deBlob: FUNCTION 00577ad0
	void SetTargetColour( TUINT8 a_uiValue ) { m_uiTargetColour = a_uiValue; }
	void SetCollisionLayers( TBOOL a_bRestore );

	// The per byte lerp of two colours (no clamp)
	static TUINT32 __stdcall LerpColour( TUINT32 a_uiFrom, TUINT32 a_uiTo, TFLOAT a_fT );

	// $deBlob: FUNCTION 005784e0
	TUINT32                     GetCurrentColour() const { return m_uiCurrentColour; }
	TUINT32                     GetBuildingFlags() const { return m_uiFlags; }
	TerrainTypes::CellInstance* GetTerrainInstance() const { return m_pTerrainInstance; }
	Node&                       GetNode() { return m_oNode; }

	// $deBlob: FUNCTION 0057b3b0
	void* GetBlock( TINT a_iBlock ) const
	{
		TASSERT( a_iBlock >= 0 && a_iBlock < MAXBLOCKS );
		return m_apBlocks[ a_iBlock ];
	}

public:
	TUINT32                     m_uiFlags;               // 0x74 Flags
	TINT8                       m_iMultiplayerOwner;     // 0x78 the player who painted it, -1: none
	TINT8                       m_iPendingPriority;      // 0x79 the size of the Blob of the pending touch, -1: none
	TUINT8                      m_uiTargetColour;        // 0x7A
	TUINT8                      m_uiCurrentColour;       // 0x7B ABlobColourComponent::COLOUR
	TUINT32                     m_uiCollisionLayerMask;  // 0x7C bit per physics object whose layer bit 8 was cleared
	Toshi::TVector4             m_vPendingPosition;      // 0x80 where m_pPendingBlob touched it
	ABlobEntity*                m_pPendingBlob;          // 0x90 painted by the next UpdateImp
	TFLOAT                      m_fPendingTimer;         // 0x94 the pending touch is dropped when it runs out
	void*                       m_apBlocks[ MAXBLOCKS ]; // 0x98 ABuildingManager's PaintableBlock
	Node                        m_oNode;                 // 0xA0 in ABuildingManager's active instances
	TerrainTypes::CellInstance* m_pTerrainInstance;      // 0xAC
	TFLOAT                      m_fInktTimer;            // 0xB0
	TFLOAT                      m_fPulseTimer;           // 0xB4
	TFLOAT                      m_fPaintFadeTimer;       // 0xB8
	TFLOAT                      m_fUnkBC;                // 0xBC
	TFLOAT                      m_fUnkC0;                // 0xC0
	TINT16                      m_iTexScaleIndex;        // 0xC4 AJellyShader_DX11's m_iCurTexScale
	TUINT8                      m_uiUnkC6;               // 0xC6
	TINT16                      m_iGlintIndex;           // 0xC8 the paint challenge projection (TWorldShaderHAL::ProjTexture), -1: none
};                                                       // 0xCC
