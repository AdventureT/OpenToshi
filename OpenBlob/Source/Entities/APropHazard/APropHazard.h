#pragma once
#include "Entities/AProp.h"
#include "Collision/ACollisionCommon.h"

#include <Render/TRenderPacket.h>

TOSHI_NAMESPACE_START
class TParticleEffect;
class TParticleEffectInstance;
class TModelInstance;
class TTexture;
class TMaterial;
class TRenderContext;
class TWorldMesh;
class TWorldMaterialHAL;
class TPhysicsObjectContainer;
struct TPhysicsMeshMaterial;
TOSHI_NAMESPACE_END

class AModelInstance2;
class AContainerHazard;
class ASpawnPointPool;

//-----------------------------------------------------------------------------
// Purpose: a prop that hurts the blob (APropHazard\APropHazard.cpp, plus ATriggerUtil's 6 at 0x9C): a hot plate (the
// "hot"/"cold" materials crossfade, a heat haze pass), electric tiles (one or more "haz_ET_1x1" models with sparks),
// spikes (an activate/deactivate animation) or an ink wall. Turned on and off by trigger messages (1 toggles, 0xE on,
// 0xF off): its collision surfaces carry the hazard's surface flag (ACollisionCommon::SURFACE_Hot, _Electric, _Spikes,
// _InkWall) only while it's on, which ABlobEntity::UpdateHazards reacts to. The paint the blob loses flies to one of
// the points of its "HazardPointList" (or of its AContainerHazard's).
//-----------------------------------------------------------------------------
class APropHazard
    : public AProp
{
public:
	TDECLARE_CLASS( APropHazard, AProp );

	// "Type" (bits 6-7 of m_uiHazardFlags)
	enum TYPE : TUINT8
	{
		TYPE_HOTPLATE = 0, // ACollisionCommon::SURFACE_Hot
		TYPE_ELECTRIC = 1, // ACollisionCommon::SURFACE_Electric
		TYPE_SPIKES   = 2, // ACollisionCommon::SURFACE_Spikes
		TYPE_INKWALL  = 3, // ACollisionCommon::SURFACE_InkWall
	};

	// Bits 4-5 of m_uiHazardFlags
	enum STATE : TUINT8
	{
		STATE_ON         = 0,
		STATE_TURNINGON  = 1,
		STATE_TURNINGOFF = 2,
		STATE_OFF        = 3,
	};

	enum HAZARDFLAG : TUINT8
	{
		HAZARDFLAG_Sound     = 0x1, // the looping sound is registered (StartSound)
		HAZARDFLAG_StartOn   = 0x4, // "StartOn"
		HAZARDFLAG_StateMask = 0x30,
		HAZARDFLAG_TypeMask  = 0xC0,
	};

	// The surface flags of a collision body or of one mesh material, as created (SetupCollision)
	struct CollisionEntry
	{
		Toshi::TPhysicsObjectContainer* m_pBody;     // 0x00 the body (m_uiSurface), or TNULL for a mesh material
		Toshi::TPhysicsMeshMaterial*    m_pMaterial; // 0x04 the mesh material (m_uiSurface)
		TUINT32                         m_uiFlags;   // 0x08 the original surface flags
	};

	// TYPE_HOTPLATE's data (0x14 bytes)
	struct HotPlateData
	{
		// $deBlob: FUNCTION 007b5a40
		HotPlateData()
		    : m_pHotMaterial( TNULL ), m_pColdMaterial( TNULL ), m_pMaterial( TNULL ), m_pHotTexture( TNULL ), m_pColdTexture( TNULL )
		{}

		Toshi::TWorldMaterialHAL* m_pHotMaterial;  // 0x00 "hot", the on look, its noise (bump) pass scrolls
		Toshi::TWorldMaterialHAL* m_pColdMaterial; // 0x04 "cold", the off look
		Toshi::TMaterial*         m_pMaterial;     // 0x08 the model's "hotplate" material both are copied from
		Toshi::TTexture*          m_pHotTexture;   // 0x0C "prop_rooftophotplate.tga"
		Toshi::TTexture*          m_pColdTexture;  // 0x10 "prop_rooftophotplateoff.tga"
	};

	// One tile of TYPE_ELECTRIC (0x1C bytes)
	struct Tile
	{
		enum TILEFLAG : TUINT32
		{
			TILEFLAG_TurningOn = 0x1, // flickering on (UpdateTurningOn)
			TILEFLAG_Lit       = 0x2,
		};

		Tile()
		    : m_pModel( TNULL ), m_pEffect( TNULL ), m_fEffectDelay( 0.0f ), m_fFlickerTime( 0.0f ), m_fTime( 0.0f ), m_fDuration( 0.0f ), m_uiFlags( 0 )
		{}

		void   UpdateColour( TFLOAT a_fDeltaTime );
		TBOOL  UpdateTurningOn( TFLOAT a_fDeltaTime ); // TTRUE once done
		TFLOAT StartTurningOn();                       // returns the duration
		void   TurnOn();
		void   TurnOff();

		AModelInstance2*                m_pModel;       // 0x00 "model/prop/haz_ET_1x1.trb" (or "PROP_rooftopelectric.trb")
		Toshi::TParticleEffectInstance* m_pEffect;      // 0x04 sm_pElectricEffect, while on
		TFLOAT                          m_fEffectDelay; // 0x08 until the effect starts
		TFLOAT                          m_fFlickerTime; // 0x0C
		TFLOAT                          m_fTime;        // 0x10
		TFLOAT                          m_fDuration;    // 0x14
		TUINT32                         m_uiFlags;      // 0x18 TILEFLAG
	};

	// TYPE_ELECTRIC's data (0xC bytes)
	struct ElectricData
	{
		Tile* m_pTiles;         // 0x00
		TINT  m_iNumTiles;      // 0x04
		TINT  m_iConfiguration; // 0x08 "HazardConfiguration" 0..8 (the tile layout)
	};

	// TYPE_SPIKES's data (4 bytes)
	struct SpikesData
	{
		TINT16 m_iActivateAnim;   // 0x00 "anim_activate"
		TINT16 m_iDeactivateAnim; // 0x02 "anim_deactivate"
	};

public:
	APropHazard();
	virtual ~APropHazard() override; // +0x08

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void  OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void  PostCreate() override;                                                               // +0x20
	virtual void  ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void  SetVisible( TBOOL a_bVisible ) override;                                             // +0x38
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void  AddToPhysics() override;                                                             // +0x88
	virtual void  RemoveFromPhysics() override;                                                        // +0x8C
	virtual TBOOL CanBlobStick() override;                                                             // +0xBC
	virtual void  Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual void  Despawn( TBOOL a_bPlayEffect ) override;                                             // +0xE4
	virtual void* GetDespawnEffect() override;                                                         // +0xE8

	TYPE GetHazardType() const { return TYPE( m_uiHazardFlags >> 6 ); }
	// $deBlob: FUNCTION 007b5b60
	STATE GetHazardState() const { return STATE( ( m_uiHazardFlags >> 4 ) & 3 ); }

	// The part of the transition done
	// $deBlob: FUNCTION 007b5b90
	TFLOAT GetTransitionProgress() const { return m_fTime / m_fDuration; }

	void SetHazardType( TYPE a_eType );

	// Takes a point of the "HazardPointList" (or of the container's) for the paint the blob loses
	void* GetSpawnPoint(); // ASpawnPointPool::SpawnPoint*

	void Toggle();
	void SetOn( TBOOL a_bOn );
	void SetState( STATE a_eState );

	// $deBlob: FUNCTION 007b60b0
	void SetContainer( AContainerHazard* a_pContainer ) { m_pContainer = a_pContainer; }

private:
	void UpdateTransition( TFLOAT a_fDeltaTime );
	void OnTurnedOn();
	void OnTurnedOff();
	void StartSound();
	void StopSound();
	void SetupCollision();

	// The hot plate mesh: the hot and cold materials blended by the transition
	void RenderHotPlateMesh( Toshi::TWorldMesh* a_pMesh, Toshi::TRenderContext* a_pContext );
	void RenderHeatMesh( Toshi::TWorldMesh* a_pMesh, Toshi::TRenderContext* a_pContext );
	void RenderInkWallMesh( Toshi::TWorldMesh* a_pMesh, Toshi::TRenderContext* a_pContext );

	// The custom render method of the model (TModelInstance::SetCustomRenderMethod, the user data is the hazard)
	static void __stdcall RenderModelCallback( Toshi::TModelInstance* a_pModelInstance, void* a_pHazard );
	// The heat haze pass of the hot plate (AEntityFactory::AddCallback): only the "heat" meshes
	static void RenderHeatCallback( void* a_pHazard );
	// The render packet callback of the "heat" and "Ink_Pool" meshes
	static void __stdcall RenderPacketCallback( Toshi::TRenderPacket* a_pPacket, void* a_pHazard, TBOOL a_bBefore );

private:
	static Toshi::TParticleEffect* sm_despawnEffect;
	static Toshi::TParticleEffect* sm_pElectricEffect; // "Blob_ElectricTrap"

private:
	AContainerHazard* m_pContainer;           // 0xB4 set by the container's PostCreate
	ASpawnPointPool*  m_pSpawnPoints;         // 0xB8 "HazardPointList"
	CollisionEntry*   m_pCollisionEntries;    // 0xBC
	TINT              m_iNumCollisionEntries; // 0xC0
	void*             m_pTypeData;            // 0xC4 HotPlateData, ElectricData or SpikesData
	TFLOAT            m_fTime;                // 0xC8 of the transition
	TFLOAT            m_fDuration;            // 0xCC of the transition
	TUINT8            m_uiHazardFlags;        // 0xD0 HAZARDFLAG, STATE << 4, TYPE << 6
};
