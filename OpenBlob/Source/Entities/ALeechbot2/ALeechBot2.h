#pragma once
#include <TParticleSystem/TParticleEffect.h>
#include "Entities/Enemies/AEnemyGrappler.h"

//-----------------------------------------------------------------------------
// Purpose: the LeechBot (ALeechbot2\ALeechBot2.cpp, AEnemyManager's ENEMYTYPE_LeechBot2): grapples the blob and drains
// its paint into its tank, respawns after a delay.
//-----------------------------------------------------------------------------
class ALeechBot2
    : public AEnemyGrappler
{
public:
	TDECLARE_CLASS( ALeechBot2, AEnemyGrappler );

	// GetLeechbotType (the wrapper's custom data & 0x7F)
	enum LeechbotType : TUINT8
	{
		LeechBot_Invalid = 0,
		LeechBot_Empty   = 2,
		LeechBot_Colour1 = 3,
		LeechBot_Colour2 = 4,
		LeechBot_Colour4 = 5,
		LeechBot_Multi   = 6,
		LeechBot_Single  = 7,
	};

	// m_uiLeechFlags
	enum LFLAG : TUINT8
	{
		LFLAG_Full   = 0x1, // the big capsule (a full tank; SetFullShape / SetEmptyShape)
		LFLAG_Static = 0x2, // custom data bit 0x80: no motor, pathfinding nor schedule
	};

	// The type data ("<type>_data_EntityData")
	struct LeechBotData
	    : public AEnemyGrappler::GrapplerData
	{
		TFLOAT m_fRespawnDelay; // 0xF4
	};

public:
	ALeechBot2();
	virtual ~ALeechBot2() override; // +0x08

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void  OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void  Create() override;                                                                   // +0x1C
	virtual void  PostCreate() override;                                                               // +0x20
	virtual TBOOL IsDead() const override;                                                             // +0x3C
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void  PreRenderUpdate() override;                                                          // +0x6C
	virtual TBOOL IsTargetable() override;                                                             // +0xA0
	// $deBlob: FUNCTION 0077b460
	virtual TINT GetTargetColour() override { return m_uiColour & 0xF; } // +0xB4
	// $deBlob: FUNCTION 0077cf80
	virtual TINT   GetMaxLife() override { return m_eLeechbotType == LeechBot_Empty; } // +0xB8
	virtual void   Reset( ABaseEntity* a_pEntity ) override;                           // +0xC8
	virtual void   OnLeaveState( TINT a_iState ) override;                             // +0xD4
	virtual void   OnEnterState( TINT a_iState ) override;                             // +0xD8
	virtual TFLOAT GetAttackRangeOptimal() override;                                   // +0x118
	virtual TFLOAT GetAttackRangeMax() override;                                       // +0x11C
	virtual void   InitialiseComponents() override;                                    // +0x12C
	virtual TBOOL  UpdateCurrentStateTask( TFLOAT a_fDeltaTime ) override;             // +0x158
	virtual void   InitIdleState() override;                                           // +0x15C
	virtual void   InitMoveState() override;                                           // +0x16C
	virtual void   SetActive( TBOOL a_bActive, TBOOL a_bUnk ) override;                // +0x19C
	virtual void   SetCustomData( TINT a_iCustomData ) override;                       // +0x1A0
	virtual TBOOL  ShouldIgnoreBlob( void* a_pEntity ) override;                       // +0x1BC
	virtual TFLOAT GetFidgetFreq() override;                                           // +0x1C8
	virtual TINT   GetKillScore() override;                                            // +0x1D8
	virtual void   OnBlobDrained() override;                                           // +0x228
	virtual void   ReleaseGrapple() override;                                          // +0x234

	LeechbotType GetLeechbotType() const { return m_eLeechbotType; }
	// The paint it holds and how full its tank is (0..1)
	// $deBlob: FUNCTION 0076a860
	TUINT32 GetPaintColour() const { return m_uiColour; }
	// $deBlob: FUNCTION 0076aa30
	TFLOAT GetTankFill() const { return m_fTankFill; }

	// Types 3, 4 and 5 (the custom data & 0x7F)
	// $deBlob: FUNCTION 0077bbe0
	static TBOOL IsFixedColourType( TUINT8 a_uiCustomData )
	{
		TUINT8 uiType = a_uiCustomData & 0x7F;
		return uiType == LeechBot_Colour2 || uiType == LeechBot_Colour1 || uiType == LeechBot_Colour4;
	}

	// Spawned in the sky by its wrapper (AEnemyWrapper::SpawnEnemy): an empty tank, state 0x15, the motor, pathfinding
	// and schedule components off
	void StartDrop();

	LeechBotData* GetLeechBotData() { return TSTATICCAST( LeechBotData, GetEnemyData() ); }

	// Creates the paint-coloured copies of the destroy effect
	// Adds the capsule of the a_iIndex-th leech bot stuck on a_pPlayer to its list shape a_pBody
	// (ABlobEntity::CreatePhysics)
	static void AddAttachedShape( Toshi::TPhysicsObjectContainer* a_pBody, ABaseEntity* a_pPlayer, TINT a_iIndex );

	static void LoadEffects();
	static void ClearEffects();

private:
	// The high byte of m_iCollisionGroupAndType
	void SetPriorityType( TUINT8 a_uiValue ) { m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( a_uiValue << 8 ) ); }

	// Takes a_uiColour (a mixed colour: one of its primaries at random) and colours the model
	void SetColour( TUINT32 a_uiColour );
	// The type's colour and a full tank (type 2: an empty tank)
	void ResetColour();
	// Empties the tank: no colour, the small capsule
	void EmptyTank();
	// Swaps the physics capsule (the big one of a full tank / the small one of an empty tank)
	void SetFullShape();
	void SetEmptyShape();
	// The capsule shared by Create, SetFullShape and SetEmptyShape
	void CreateCapsule( TFLOAT a_fHeight, TFLOAT a_fRadius, AEntityPhysicsData* a_pPhysicsData );
	// Pops the LeechBot (the destroy effect of its colour) and waits to respawn unless a_bNoRespawn or type 2
	void Destroy( ABaseEntity* a_pKiller, TBOOL a_bNoRespawn );

	friend class ACar;         // the paint tanker pops the destroy effects of the blob's colour
	friend class ABigPaintBot; // pops with the destroy effects of its colour

	static TCONSTEXPR_VAR TINT NUM_PRIMARY_COLOURS   = 7;
	static TCONSTEXPR_VAR TINT NUM_SECONDARY_COLOURS = 8;

	static Toshi::TParticleEffect* ms_pExplosionEffect;                                   // "Blob_LeechBot_Explosion"
	static Toshi::TParticleEffect* ms_apSecondaryDestroyEffects[ NUM_SECONDARY_COLOURS ]; // colours 8..15
	static Toshi::TParticleEffect* ms_apPrimaryDestroyEffects[ NUM_PRIMARY_COLOURS ];     // colours 1..7
	static TBOOL                   sm_bSkinMaterialSetUp;                                 // "spheremap_leechbot.tga"

	LeechbotType m_eLeechbotType; // 0x11C
	TUINT8       m_uiLeechFlags;  // 0x11D LFLAG
	TUINT32      m_uiColour;      // 0x120 the paint it holds
	TFLOAT       m_fTankFill;     // 0x124 0..1
	TFLOAT       m_fGrabCooldown; // 0x128
	TFLOAT       m_fRespawnTimer; // 0x12C -1.0f: not waiting
};
