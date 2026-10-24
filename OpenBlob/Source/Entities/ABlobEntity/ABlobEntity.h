#pragma once
#include "Entities/ABaseEntity.h"
#include "ABlobColourComponent.h"
#include "ABlobZJumpComponent.h"
#include "Entities/ASpring.h"
#include "Entities/ASpring3D.h"

#include <Toshi/T2DList.h>
#include <Render/TRenderPacket.h>
#include <Physics/TPhysicsResponder.h>
#include <Physics/TPhysicsMotionBase.h>
#include <Physics/TPhysicsPluginBase.h>
#include <Physics/Havok/TPhysicsManager.h>

namespace Toshi
{
class TPhysicsObjectContainer;
class TSysMesh;
class TMaterial;
class TTexture;
class TParticleEffect;
} // namespace Toshi

class ABlobAnimationComponent;
class ABlobRibbonTrailComponent;
class ABlobNavRingComponent;
class ACamera;
class ABlobZJumpComponent;
class ABlobSoundComponent;
class ABlobSizeHud;
class ABlobAmmoHud;
class AFXBreakpointComponent;
class APropHazard;
class APropTriggerHelp;
class APropTriggerHudMessage;
class APropWindCannon;
class ABoss;
class ALandmarkInstance;

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: the motion data at 0x80 of ABlobEntity; only its tail from 0x1B0 is set by its constructor.
//-----------------------------------------------------------------------------
struct ABlobMotionData
{
	ABlobMotionData();
	void Reset();

	// Clears the havok contact flags (0x10, 0x20) and their bodies, positions and materials
	void ResetContacts();

	// The flags of the contact a_iIndex (0x6c bytes each from 0x00, 4 of them)
	TUINT32 GetContactFlags( TINT a_iIndex ) const { return m_aContacts[ a_iIndex ].m_uiMaterial; }

	// The wall contact (0x2AC bit 4) has one of a_uiFlags
	// $deBlob: FUNCTION 00751d40
	TBOOL HasWallContactFlag( TUINT32 a_uiFlags ) const { return ( m_uiContactFlags & 4 ) != 0 && ( GetContactFlags( m_uiWallContact ) & a_uiFlags ) != 0; }

	// The ground contact (0x2AC bit 8) has one of a_uiFlags
	// $deBlob: FUNCTION 00751d10
	TBOOL HasGroundContactFlag( TUINT32 a_uiFlags ) const { return ( m_uiContactFlags & 8 ) != 0 && ( GetContactFlags( TUINT8( m_uiContactCountAndGround >> 8 ) ) & a_uiFlags ) != 0; }

	Toshi::TPhysicsShapeContact m_aContacts[ 4 ];          // 0x000 the contacts of the last physics step (m_uiContactCountAndGround & 0xFF of them)
	Toshi::TMatrix44            m_oUnk1B0;                 // 0x1B0
	Toshi::TVector4             m_vLiquidHit;              // 0x1F0
	Toshi::TVector4             m_vLiquidNormal;           // 0x200
	Toshi::TVector4             m_vContactNormal;          // 0x210
	Toshi::TVector4             m_vUnk220;                 // 0x220
	Toshi::TVector4             m_vUnk230;                 // 0x230
	Toshi::TVector4             m_vUnk240;                 // 0x240
	Toshi::TVector4             m_vUnk250;                 // 0x250
	Toshi::TVector4             m_vWallNormal;             // 0x260
	Toshi::TVector4             m_vUnk270;                 // 0x270
	TUINT32                     m_auiUnk280[ 11 ];         // 0x280
	TUINT16                     m_uiContactFlags;          // 0x2AC the contact flags (4: a wall contact, 8: a ground contact)
	TUINT16                     m_uiPrevContactFlags;      // 0x2AE last physics step's m_uiContactFlags
	TUINT16                     m_uiContactCountAndGround; // 0x2B0
	TUINT8                      m_uiWallContact;           // 0x2B2
	TUINT8                      m_uiUnk2B3;                // 0x2B3
};

TSTATICASSERT( sizeof( ABlobMotionData ) == 0x2B4 );

//-----------------------------------------------------------------------------
// Purpose: Blob, the player (TClass parent ABaseEntity, no virtuals of its own: 55 slots). Created by AEntityFactory
// from the level's entity data.
// Owns its components (animation/model, FX breakpoints, collision object, z-jump targets...), its physics body (a
// sphere sized by BlobEntityProperties[ m_iSize ]) and a mesh pool for the paint threads.
//-----------------------------------------------------------------------------
class ABlobEntity
    : public ABaseEntity
{
public:
	// A node of m_oList438: APickupLife's life node (APickup::ListNode)
	struct LifeNode : public Toshi::T2GenericDList::Node
	{
	};

	TDECLARE_CLASS( ABlobEntity, ABaseEntity );

	static TCONSTEXPR_VAR TINT    BLOB_SIZE_ABS_MAX = 10;
	static TCONSTEXPR_VAR TINT    ZPRIORITY_COUNT   = 38;
	static TCONSTEXPR_VAR TUINT32 POINTS_MAX        = 100;

	enum STATE
	{
		STATE_ROLL         = 0,
		STATE_BALLISTIC    = 1,
		STATE_STUCKWALL    = 2,
		STATE_HOTPLATE     = 3,
		STATE_LMARK        = 4,
		STATE_BOSS         = 5,
		STATE_FLOAT        = 6,
		STATE_CANNON       = 7,
		STATE_GRASSBOMB    = 8,
		STATE_RESPAWN      = 9,
		STATE_LEVEL_EXIT   = 10,
		STATE_DEAD         = 11,
		STATE_FAIL         = 12,
		STATE_VICTORY      = 13,
		STATE_LOSTLASTLIFE = 14,
	};

	// m_iZPlayerAttackStage
	enum Z_ATTACKSTAGE
	{
		Z_ATTACKSTAGE_NONE,
		Z_ATTACKSTAGE_OVER,    // up over the target (halfway between the jump's points, 8 above)
		Z_ATTACKSTAGE_TOWARDS, // at the target
		Z_ATTACKSTAGE_ONTO,    // still at the target
		Z_ATTACKSTAGE_THROUGH, // pushed through it at 30
		Z_ATTACKSTAGE_COUNT,
	};

	// One size of Blob (BlobProperties.trb "BlobEntityProperties", 0x4C bytes each, "NumBlobEntitySizes" of them)
	struct Properties
	{
		TFLOAT m_fUnk00;              // 0x00
		TFLOAT m_fTrailScale;         // 0x04
		TFLOAT m_fJellyStiffness;     // 0x08 scaled by the mass on load
		TFLOAT m_fJellyDamping;       // 0x0C
		TFLOAT m_fJellyOffset;        // 0x10
		TFLOAT m_fRadius;             // 0x14 the physics sphere and the bounding sphere
		TFLOAT m_fMass;               // 0x18
		TFLOAT m_fDensity;            // 0x1C computed on load: mass / sphere volume
		TFLOAT m_fTorque;             // 0x20 scaled by radius * mass on load
		TFLOAT m_fForce;              // 0x24 scaled by the mass on load
		TFLOAT m_fFriction;           // 0x28 the body's friction
		TFLOAT m_fRestitution;        // 0x2C the body's restitution
		TFLOAT m_fMaxSpeed;           // 0x30
		TFLOAT m_fUnk34;              // 0x34
		TFLOAT m_fMaxAngularVelocity; // 0x38 computed on load: m_fMaxSpeed / radius; the body's max angular velocity
		TFLOAT m_fStretchTime;        // 0x3C
		TFLOAT m_fUnk40;              // 0x40
		TFLOAT m_fUnk44;              // 0x44
		TFLOAT m_fSlamRadius;         // 0x48 scaled by the radius on load
	};

	TSTATICASSERT( sizeof( Properties ) == 0x4C );

	// BlobProperties.trb "BlobEntityCommonProperties" (mapped up to 0x1B0, the end of the challenge camera settings; the rest is unknown).
	// Read through ms_pBlobCommonProperties / GetCommonProperties().
	struct CommonProperties
	{
		TFLOAT  m_fStickDeadZone;       // 0x00 the stick response (GetStickResponse): below it, a ramp to m_fStickMinResponse
		TINT    m_eStickCurveLow;       // 0x04 1: linear, 2: exponential, else none
		TFLOAT  m_fStickMinResponse;    // 0x08
		TFLOAT  m_fStickFullZone;       // 0x0C from it, full response
		TINT    m_eStickCurve;          // 0x10 1: linear, 2: exponential, else m_fStickMinResponse
		TFLOAT  m_afUnk14[ 8 ];         // 0x14
		TFLOAT  m_fJumpSpeed;           // 0x34
		TFLOAT  m_afUnk38[ 9 ];         // 0x38
		TFLOAT  m_fUnk5C;               // 0x5C
		TFLOAT  m_fJumpVelocity;        // 0x60 computed on load
		TFLOAT  m_fUnk64;               // 0x64
		TFLOAT  m_fWallJumpReach;       // 0x68
		TFLOAT  m_fUnk6C;               // 0x6C computed on load
		TFLOAT  m_fWallJumpAngle;       // 0x70 an angle, degrees in the TRB (at most 80), radians after the load
		TFLOAT  m_fRollDamping;         // 0x74
		TFLOAT  m_fUnk78;               // 0x78
		TFLOAT  m_fAngularDamping;      // 0x7C the body's angular damping
		TFLOAT  m_fAirStickStrength;    // 0x80
		TFLOAT  m_fLiquidStickStrength; // 0x84
		TFLOAT  m_fCameraHoldTime;      // 0x88 the time the camera direction keeps its old value when it turns more than 70 degrees
		TFLOAT  m_fUnk8C;               // 0x8C
		TFLOAT  m_fInkedPenalty;        // 0x90 at least 0.1
		TFLOAT  m_fInkedJumpVelocity;   // 0x94 computed on load
		TFLOAT  m_fUnk98;               // 0x98 computed on load
		TFLOAT  m_fLookTurnRate;        // 0x9C
		TFLOAT  m_afUnkA0[ 6 ];         // 0xA0
		TFLOAT  m_fUnkB8;               // 0xB8
		TFLOAT  m_afUnkBC[ 7 ];         // 0xBC
		TFLOAT  m_fBlobKillRespawnTime; // 0xD8 the assert "s_cfFadeTime < ms_pBlobCommonProperties->m_fBlobKillRespawnTime"
		TFLOAT  m_fInkTime;             // 0xDC ABlobEntity: divided by 10 into its 0x588 timer
		TFLOAT  m_afUnkE0[ 2 ];         // 0xE0
		TFLOAT  m_afBlinkTime[ 2 ];     // 0xE8 min/max time the eyes stay closed
		TFLOAT  m_afEyesOpenTime[ 2 ];  // 0xF0 min/max time between blinks
		TFLOAT  m_afLookTime[ 2 ];      // 0xF8 min/max time before the next look (ABlobAnimationComponent m_iUnk1C)
		TFLOAT  m_afUnk100[ 2 ];        // 0x100
		TFLOAT  m_fFirstDrainDelay;     // 0x108
		TFLOAT  m_fMinDrainInterval;    // 0x10C the water drains a paint point at most this often
		TFLOAT  m_fDrainIntervalStep;   // 0x110 the interval shrinks by it with each point drained
		TUINT32 m_uiWaterPoints;        // 0x114 the water drains or fills the paint points towards it
		TFLOAT  m_fHotPlateTime;        // 0x118 ABlobEntity's 0x56C timer: compared with it
		TFLOAT  m_fPickupLifeTime;      // 0x11C
		TFLOAT  m_fUnk120;              // 0x120
		TFLOAT  m_fHurtTime;            // 0x124 the value ABlobEntity's 0x56C timer is reset to
		TFLOAT  m_afUnk128[ 6 ];        // 0x128

		// One challenge camera setting (0x10 bytes), by the challenge's type: 0 is the default, 1-6 the rest
		// (ABlobFollowCamera::UpdateChallengeCam).
		struct ChallengeCamSettings
		{
			TFLOAT m_fFOV;      // 0x00 degrees: the FOV the camera is given
			TFLOAT m_fFOVSpeed; // 0x04 the FOV speed the camera is given
			TFLOAT m_fMinDist;  // 0x08 the minimum of the camera distance, the distance blend's floor
			TFLOAT m_fAngle;    // 0x0C degrees: the angle the camera is rotated around the challenge
		};

		ChallengeCamSettings m_aChallengeCams[ 7 ]; // 0x140
	};

	TSTATICASSERT( sizeof( CommonProperties::ChallengeCamSettings ) == 0x10 );

	// Moves Blob's body in every physics step: a custom mover of the physics manager (0x10 bytes at 0x3F4).
	class ABlobPhysicsMover
	    : public Toshi::TPhysicsMotionBase
	{
	public:
		ABlobPhysicsMover( ABlobEntity* a_pOwner )
		    : m_iUnk8( -1 ), m_pOwner( a_pOwner )
		{}

		// $deBlob: FUNCTION 007513c0
		// $deBlob: FUNCTION 007512d0
		virtual ~ABlobPhysicsMover() override = default;

		// $deBlob: FUNCTION 00751610
		virtual void OnRemoved() override {}
		virtual void Update( TFLOAT a_fTimeStep, Toshi::TPhysicsManager* a_pManager ) override;
		// $deBlob: FUNCTION 00751df0
		virtual void Unk0C( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751de0
		virtual void Unk10( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751eb0
		virtual void Unk14( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751ea0
		virtual void Unk18( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751dd0
		virtual void Unk1C( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751dc0
		virtual void Unk20( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751e90
		virtual void Unk24( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751e80
		virtual void Unk28( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751c20
		virtual void Unk2C( void* a_pUnk1, void* a_pUnk2 ) override {}
		// $deBlob: FUNCTION 00751620
		virtual void Unk30() override {}
		virtual void GetLinearVelocity( Toshi::TPhysicsManager* a_pManager, Toshi::TVector4& a_rvOutVelocity ) override;

		TINT         m_iUnk8;  // 0x08
		ABlobEntity* m_pOwner; // 0x0C
	};

	// Step listener at 0x404.
	class ABlobPhysicsPlugin
	    : public Toshi::TPhysicsPluginBase
	{
	public:
		ABlobPhysicsPlugin( ABlobEntity* a_pOwner )
		    : m_pOwner( a_pOwner )
		    , m_pPreviousGroundBody( TNULL )
		{}

		// $deBlob: FUNCTION 007513f0
		// $deBlob: FUNCTION 007512e0
		virtual ~ABlobPhysicsPlugin() override = default;
		virtual void OnStartStep( TFLOAT a_fTimeStep, Toshi::TPhysicsManager* a_pManager ) override;
		virtual void OnFinishStep( Toshi::TPhysicsManager* a_pManager ) override;

		ABlobEntity*                    m_pOwner;              // 0x08
		Toshi::TPhysicsObjectContainer* m_pPreviousGroundBody; // 0x0C
	};

	// Receives the collisions of Blob's body (vtable 0x3EC)
	class ABlobCollisionEventHandler
	    : public Toshi::TPhysicsResponder
	{
	public:
		ABlobCollisionEventHandler( ABlobEntity* a_pOwner )
		    : m_pOwner( a_pOwner )
		{}

		virtual ~ABlobCollisionEventHandler() override;

		virtual void OnCollisionConfirmed( Toshi::TPhysicsCollisionInfo* a_pInfo ) override; // +0x08
		virtual void OnCollisionProcess( Toshi::TPhysicsCollisionInfo* a_pInfo ) override;   // +0x0c

		ABlobEntity* m_pOwner; // 0x04
	};

	// A paint thread between the ball and a surface it rolls off (0x44 bytes, 16 of them in a pool): a quad from the
	// point it was left on to its end on the ball, until it's longer than 3 or the ball is on the other side of it
	struct Thread
	{
		// $deBlob: FUNCTION 00751260
		Thread()
		{
			m_vEnd           = Toshi::TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
			m_vAnchor        = Toshi::TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
			m_uiAnchorColour = 0;
			m_uiEndColour    = 0;
			m_pMesh          = TNULL;
			m_pPrev          = TNULL;
			m_pNext          = TNULL;
		}

		Toshi::TVector4  m_vDirection;     // 0x00 the end's direction from the ball's centre, in the body's frame
		Toshi::TVector4  m_vEnd;           // 0x10 the end on the ball
		Toshi::TVector4  m_vAnchor;        // 0x20 the end left on the surface
		TUINT32          m_uiAnchorColour; // 0x30 ARGB
		TUINT32          m_uiEndColour;    // 0x34 ARGB, 0 while the thread is free
		Toshi::TSysMesh* m_pMesh;          // 0x38 the quad
		Thread*          m_pPrev;          // 0x3C
		Thread*          m_pNext;          // 0x40
	};

	TSTATICASSERT( sizeof( Thread ) == 0x44 );

	// A list of threads (0xC bytes): the free ones and the active ones
	struct ThreadList
	{
		// $deBlob: FUNCTION 0073cc50
		TBOOL IsInList( Thread* a_pThread ) const
		{
			for ( Thread* pThread = m_pHead; pThread != TNULL; pThread = pThread->m_pNext )
			{
				if ( pThread == a_pThread )
					return TTRUE;
			}

			return TFALSE;
		}

		void    Remove( Thread* a_pThread );
		Thread* PopBack();
		void    PushFront( Thread* a_pThread );

		Thread* m_pHead;  // 0x00
		Thread* m_pTail;  // 0x04
		TINT    m_iCount; // 0x08
	};

public:
	ABlobEntity();
	virtual ~ABlobEntity() override; // +0x08

	// Draws the trail and the models (APlayerManager::Render)
	void Render();

#ifdef DEBLOB_COMMUNITY_PATCH
	// m_oPhysicsMatrix at the time being rendered (UpdateModelTransforms): between the body's last two fixed steps
	void GetRenderPhysicsMatrix( Toshi::TMatrix44& a_rOutMatrix );

	// The position Blob is drawn at this frame (the cameras follow it instead of the body's last step); view only
	Toshi::TVector4 GetRenderPosition()
	{
		Toshi::TMatrix44 mRender;
		GetRenderPhysicsMatrix( mRender );
		return mRender.GetTranslation();
	}

	// The matrix of the last GetRenderPhysicsMatrix (the one the models were placed with this frame)
	const Toshi::TMatrix44& GetLastRenderMatrix() const { return m_mRenderCurrent; }
#endif

	virtual void Tick( TFLOAT a_fDeltaTime ) override;    // +0x0C
	virtual void OnOutOfWorld() override;                 // +0x18
	virtual void Create() override;                       // +0x1C
	virtual void Destroy() override;                      // +0x24
	virtual void SetVisible( TBOOL a_bVisible ) override; // +0x38
	// $deBlob: FUNCTION 007520e0
	virtual void SetFacing( const Toshi::TVector4& a_rFacing ) override { m_vFacing = a_rFacing; }    // +0x44
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 00751770
	virtual ACollisionObject* GetCollisionObject() override { return m_pCollisionObject; } // +0x70
	// $deBlob: FUNCTION 007516f0
	virtual Toshi::TVector4 GetBasePosition() const override { return m_oMatrix.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION ); } // +0x80
	// $deBlob: FUNCTION 00747340
	virtual TBOOL IsTargetable() override { return m_eState != STATE_CANNON; } // +0xA0
	virtual void  Reset( ABaseEntity* a_pEntity ) override;                    // +0xC8

	// Reads BlobProperties.trb (the level's DataBlock_BlobProperties); a_bReload: loads the file again first
	static void LoadProperties( TBOOL a_bReload );

	// The particle effects of the Blobs (data/Blob_FX/FXParticles), found when a level is loaded.
	static void LoadEffects();
	static void ClearEffects();

	// Clears the shared property pointers that point into the level's data blocks (freed with them)
	static void UnloadProperties();

	// $deBlob: FUNCTION 00578650
	TINT GetBlobState() const { return m_eState; }

	// The splat material of the decals (0x648)
	Toshi::TMaterial* GetSplatMaterial() const { return m_pMaterialTestSplat; }

	static CommonProperties* GetCommonProperties() { return ms_pBlobCommonProperties; }
	static Properties*       GetProperties( TINT a_iSize ) { return &ms_pBlobProperties[ a_iSize ]; }

	// Places the blob at a_rPosition and respawns it there (a_bSetPosition: a_rPosition becomes m_vRespawnPosition and the
	// respawn rotation is reset)
	void SetStartPosition( const Toshi::TVector4& a_rPosition, TBOOL a_bSetPosition, TBOOL a_bFindGround );

	// Detaches the blob from the physics during the level start, or puts it back
	void SetLevelStartAttached( TBOOL a_bAttached );

	// Respawns the blob at a_pPosition (TNULL: m_vRespawnPosition); a_bFindGround: moves the point onto the ground first
	void Respawn( TBOOL a_bUnk, TBOOL a_bFindGround, const Toshi::TVector4* a_pPosition );
	// The end of Respawn: restores the points/colour kept at 0xB6C/0xB70, wakes the body up and goes to STATE_BALLISTIC
	void FinishRespawn();

	// Keeps the respawn point (m_vRespawnPosition) and its data at 0xB56-0xB60 (APropTriggerRespawn::Trigger); a_vRotation is
	// used only with a_bHasRotation
	void SetRespawnPoint( const Toshi::TVector4& a_rcPosition, Toshi::TVector4 a_vRotation, TBOOL a_bHasRotation, TBOOL a_bPlaySeq, TBOOL a_bTransSeq, TFLOAT a_fSeqStartRatio, TFLOAT a_fSeqEndRatio );

	// The respawn point of the blob: the current position, the multiplayer start point, or (single player) m_vRespawnPosition
	// with the data kept at 0xB56-0xB60
	void GetRespawnData( Toshi::TVector4& a_rOutPosition, Toshi::TVector4& a_rOutRotation, TBOOL& a_rbOutHasRotation, TBOOL& a_rbOutPlaySeq, TBOOL& a_rbOutTransSeq, TUINT32& a_ruiOutSeqStartRatio, TFLOAT& a_rfOutSeqEndRatio );
	// Casts a ray from 1 above to 3 below a_pPosition (TNULL: m_vRespawnPosition) for the ground; a_pOutPosition gets the hit
	// (or a_pPosition); returns TTRUE if the ground is water (material 0x10)
	TBOOL FindRespawnGround( const Toshi::TVector4* a_pPosition, Toshi::TVector4* a_pOutPosition );
	// a_rPosition moved onto the ground (outside the multiplayer modes) and lifted by a radius
	void GetRespawnGroundPosition( const Toshi::TVector4& a_rPosition, Toshi::TVector4& a_rOutPosition );
	// Moves the dead blob to its respawn point (a_pPosition, or GetRespawnData's) before the life lost screen fades
	// out; a_pPoints/a_pColour are kept for FinishRespawn (TNULL: -1)
	void PrepareRespawn( const Toshi::TVector4* a_pPosition, const TINT* a_pPoints, const TINT* a_pColour );

	// The camera the blob is seen through: the current camera if it's a free camera, else the blob's camera
	ACamera* GetViewCamera();

	// The ABlobLifeLostHud is shown (0xB55)
	void SetLifeLostShown( TBOOL a_bShown ) { m_bLifeLostShown = a_bShown; }
	// $deBlob: FUNCTION 00657930
	TUINT32 GetUnkB68() const { return m_uiUnkB68; }
	void    SetUnkB68( TUINT32 a_uiValue ) { m_uiUnkB68 = a_uiValue; }

	// Moves the physics body (and m_vPhysicsPosition) to a_rPosition
	void SetPhysicsPosition( const Toshi::TVector4& a_rPosition );

	// Leaves a paint thread from a_rPosition (a contact with the normal a_rNormal) to the ball (a free thread, else the
	// oldest one)
	void AddThread( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rNormal );

	// Blob's Z-jump hit the stunned boss (ABoss::HandleEntityIO): STATE_BOSS, the shake meter of its life
	void EnterBoss( ABoss* a_pBoss );
	// Blob leaves the boss in front of its eye and is pushed off (a_bKilled: the round's life is over, ABoss::OnHit);
	void ExitBoss( TBOOL a_bKilled );

	// Blob bounced on the trigger a_iTrigger of a_pLandmark (ALandmarkInstance::Trigger::OnBlobContact): STATE_LMARK
	// in front of the trigger, the shake prompt ("SwingRemote") and the landmark meter
	void EnterLandmark( ALandmarkInstance* a_pLandmark, TINT a_iTrigger );
	// Blob leaves the landmark: jumps off the trigger (or is put in front of it), STATE_BALLISTIC; a_bCompleted: the
	// landmark's last trigger was painted (the celebration animation)
	void ExitLandmark( TBOOL a_bCompleted );
	// Hides the shake prompt, the landmark meter and the splats (the completed landmark's letterbox)
	void HideLandmarkPrompt();
	// Blob pops (STATE_DEAD): ink particles all around, the death pop spray, the sound; hides the blob
	void PopDeath();
	// Clears m_uiButtons and m_uiPrevButtons
	void ClearButtons()
	{
		m_uiButtons     = 0;
		m_uiPrevButtons = 0;
	}

	// The jelly's squash against a contact (0x624, 1: none)
	// $deBlob: FUNCTION 0075e4d0
	TFLOAT GetContactSquash() const { return m_fSquash; }

	// Starts the shockwave model (0xB74, the Z-jump slam's ring, the grass bomb's arrow) at 0.6 below a_rPosition for
	// a_fTime, unless it's still playing; a_bOrient: faces it along a_rNormal
	void StartShockwave( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rNormal, TFLOAT a_fTime, TFLOAT a_fAlpha, TBOOL a_bOrient );
	// Moves the playing shockwave model to a_rPosition, facing along a_rNormal
	void UpdateShockwave( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rNormal );
	// The AEntityFactory callback (added by Create) that draws the shockwave
	static void ShockwaveRenderCallback( void* a_pBlob );
	// Draws the playing shockwave model, faded out over its last part
	void RenderShockwave();
	// The shockwave mesh's packet callback: blend mode 10 uses tex coord gen 12, blend mode 9 the noise texture
	static void __stdcall ShockwavePacketCallback( Toshi::TRenderPacket* a_pPacket, void* a_pBlob, TBOOL a_bBefore );

	// Starts the rainbow paint (BFLAG 0x200000, the paint tanker's last hit, ACar): paint colour 2 if Blob had none or
	// ink, 100 points (the old ones kept at 0x5C0), size 10, the rainbow sound/effect/decals and animation 0x1F with
	// the ball held in place
	void StartRainbow();

	// Ends the rainbow paint (BFLAG 0x200000): the "rainbow" music snapshot, the points kept at 0x5C0 and the decals
	void EndRainbow();

private:
	// Resets the motion vectors to the base position
	void ResetMotion();

	// Creates the physics body and its triggers for the current size (a_fLinearDamping: the body's linear damping)
	void CreatePhysics( TFLOAT a_fLinearDamping );

	// The physics step of the blob (ABlobPhysicsMover): the contacts, the input and the state machine
	void UpdatePhysics( TFLOAT a_fTimeStep );

	// Leaves m_eState and enters a_eState (effects, flags, the body in the simulation)
	void SetState( TINT a_eState );

	// The state changes of the physics step (landing, falling, sticking to walls, water/ink)
	void UpdateStateTransitions( TFLOAT a_fTimeStep );

	// Enters water (BFLAG_INWATER 0x80) or ink (BFLAG_ININK 0x100): returns whether it just did
	TBOOL CheckLiquid();

	// Looks for water or ink at a_rcPosition (CollisionCache::FLAG_HIT_WATER 1 / FLAG_HIT_INK 2 of m_uiContactFlags): a ray
	// from 40 above down to the bottom of the ball through the liquid surfaces, then the soft collision query
	// around the ball; keeps the hit point (m_vLiquidHit) and the surface normal (m_vLiquidNormal)
	void UpdateLiquidCollision( const Toshi::TVector4& a_rcPosition );

	// Jumps out of the water or ink with the jump button
	TBOOL TryLiquidJump( TFLOAT a_fTimeStep );

	// The buoyancy of the water or ink
	void UpdateLiquidBuoyancy();

	// Steers the ball in the water or ink
	void UpdateLiquidMovement( TFLOAT a_fTimeStep );

	// Stops the rolling sound and pauses the roll effects (0xAF8, 0xAFC)
	void StopRollEffects();

	// Starts or resumes the roll effect: the paint drip (0xAF8, the colour's effect 0xADC) or, without paint, the
	// clear one (0xAFC); none (StopRollEffects) when it's invisible or unpainted in the multiplayer game
	void UpdateRollEffects();

	// The effect of the surface (material 0x2000000) the ball touches: 0xB04, from the effect 0xAE4
	void UpdateSurfaceEffect();

	// The hurting surfaces (material 0x100000: a paint point), the electric (0x8000) and hot (0x10000) hazards of the
	// contact
	void UpdateHazards();

	// The start of the electrocution by the electric hazard a_pHazard (BFLAG 0x2000000, its time at 0x584): the sparks,
	// the lost paint (or the death on the last life), the effect, the sound, the HUD and the bounce off
	void StartElectrocution( APropHazard* a_pHazard );

	// The wind cannon (STATE_CANNON): put into the barrel at a_rMatrix, shown at size a_uiSize
	void EnterCannon( const Toshi::TMatrix44& a_rMatrix, TUINT a_uiSize );
	// Moves the blob with the barrel while it's shot
	void SetCannonTransform( const Toshi::TMatrix44& a_rMatrix );
	// Shot out of the cannon from a_rPosition: the body back, the paint ring, the trail, STATE_BALLISTIC
	void LaunchFromCannon( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rVelocity );

public:
	// The end of the electrocution (BFLAG 0x2000000, its time at 0x584): the effect, the sound, the shake, the HUD
	void EndElectrocution();

private:
	// Pushes the ball 0.2 out along the contact normal (m_oMotion.m_vContactNormal) and bounces it off into STATE_BALLISTIC
	void BounceOffContact();

	// Kills the blob when it's stuck: in the air with jump held, not moving and touching more than one thing for 3 s
	void UpdateStuck( TFLOAT a_fTimeStep );

	// The jelly (m_oJelly): the model's wobble around the body, its squash against what it touches (m_fSquash,
	// m_fSquashCompensation, m_vUnk614) with paint splashes, and the stretch after a jump (m_fStretch, m_fStretchCompensation)
	void UpdateJelly( TFLOAT a_fTimeStep );

	// The soft surfaces (layer 0x100) within s_fPhantom424Radius: limits the speed into them and pushes the ball away
	// (BFLAG 0x200 while pushed); a step that moved far is put back in front of the one it went through
	void UpdateSoftCollision();

	// STATE_ROLL: brakes (BUTTON_BRAKE) when it may; returns whether it braked
	TBOOL UpdateBrake( TFLOAT a_fTimeStep );

	// STATE_ROLL: rolls the ball by the movement stick, relative to the camera
	void UpdateRollMovement( TFLOAT a_fTimeStep );

	// Damps the velocity across a_rDirection (and along it above a_fMaxSpeed) on slippery or no ground
	void ApplyRollDamping( TFLOAT a_fTimeStep, TFLOAT a_fDamping, const Toshi::TVector4& a_rDirection, TFLOAT a_fMaxSpeed );

	// STATE_ROLL: jumps when the jump button is pressed on the ground
	void UpdateJump( TFLOAT a_fTimeStep );

	// Jumps off the wall (jump pressed; the stick picks the direction along it): TRUE if it jumped
	TBOOL TryWallJump( TFLOAT a_fTimeStep );

public:
	// The launch velocity to reach a_rvTarget from a_rvPos at a_fTrajectory (radians above the horizontal) under
	// a_fGravity; returns the time of the flight
	static TFLOAT CalculateJumpVelocity( TFLOAT a_fTrajectory, const Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvTarget, TFLOAT a_fGravity, Toshi::TVector4& a_rOutVelocity );
	// The launch velocity to reach a_rvTarget from a_rvPos moving at a_fSpeed under a_fGravity; returns the time of the
	// flight
	static TFLOAT CalculateJumpVelocityAtSpeed( const Toshi::TVector4& a_rvPos, const Toshi::TVector4& a_rvTarget, TFLOAT a_fSpeed, TFLOAT a_fGravity, Toshi::TVector4& a_rOutVelocity );

private:
	// The wall's across/up axes (from m_oMotion.m_vWallNormal) and the stick mapped onto them, relative to the camera
	void GetWallAxes( const Toshi::TVector2& a_rStick, Toshi::TVector4& a_rOutAcross, Toshi::TVector4& a_rOutUp, TFLOAT& a_rfOutAcross, TFLOAT& a_rfOutUp );

	// STATE_BALLISTIC: steers the ball by the movement stick, relative to the camera
	void UpdateAirMovement( TFLOAT a_fTimeStep );

	// STATE_STUCKWALL: rolls along/up the wall by the movement stick
	void UpdateWallMovement( TFLOAT a_fTimeStep );

	// Keeps m_vCameraDirection, the camera-to-blob direction the movement is relative to
	void UpdateCameraDirection( TFLOAT a_fDeltaTime );
	// Sets the camera direction (0x5D0) to the XZ direction from the camera to the ball at once
	void ResetCameraDirection();

	// The camera's forward (XZ) and right directions the movement stick is relative to
	void GetMoveAxes( Toshi::TVector4& a_rOutForward, Toshi::TVector4& a_rOutRight );

	// The stick's response curve (CommonProperties m_fStickDeadZone...)
	static TFLOAT GetStickResponse( TFLOAT a_fValue );

	// The name of a STATE ("Unknown State" out of range), also as a wide string
	const TCHAR*  GetStateName( TINT a_eState ) const;
	const TWCHAR* GetStateNameW( TINT a_eState ) const;
	// The turrets targeting this blob (ATurretHelper): the first one plays event 38
	void IncTargetCount();
	// $deBlob: FUNCTION 007c98e0
	void DecTargetCount()
	{
		TASSERT( m_uiNumTurretsTargettingMe > 0 );
		m_uiNumTurretsTargettingMe--;
	}
	// BFLAG 0x400000 (the multiplayer shield)
	TBOOL IsShielded() const;
	// The radius of the current size (BlobEntityProperties)
	TFLOAT GetSizeRadius() const;
	// GetWallAxes' axes for any normal (this unused)
	void GetNormalAxes( const Toshi::TVector4& a_rNormal, Toshi::TVector4& a_rOutAcross, Toshi::TVector4& a_rOutUp );
	// Removes 10 paint points from the current player's blob (this unused)
	void RemoveCurrentBlobPaint( TBOOL a_bOnlyControllable );
	// AVibrationManager::VibrateOnce( this player, 4 )
	void Vibrate();

	// The flags of the contact a_iIndex of m_oMotion (0x6c bytes each from 0x00, 4 of them)
	TUINT32 GetContactFlags( TINT a_iIndex ) const { return m_oMotion.GetContactFlags( a_iIndex ); }

	TBOOL HasWallContactFlag( TUINT32 a_uiFlags ) const { return m_oMotion.HasWallContactFlag( a_uiFlags ); }
	TBOOL HasGroundContactFlag( TUINT32 a_uiFlags ) const { return m_oMotion.HasGroundContactFlag( a_uiFlags ); }

	// The blob can't be controlled (BFLAG 0x4080000 or 0x200000)
	// $deBlob: FUNCTION 00751ca0
	TBOOL IsLocked() const { return ms_bControlsLocked || ( m_uiFlags & 0x4080000 ) != 0 || ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) != 0; }

	// The blob can be controlled (BFLAG 0x4080000 and 0x200000)
	TBOOL IsControllable() const { return !ms_bControlsLocked && ( m_uiFlags & 0x4080000 ) == 0 && ( TUINT8( m_uiFlags >> 0x15 ) & 1 ) == 0; }

public:
	// The music's bar (AMusicManager's bar feedback): the jelly squash (0x4A8) jumps to m_afUnkBC[ 1 ] with the spring
	// of m_afUnkBC[ 2 ]/[ 3 ], BFLAG 0x80000000
	// $deBlob: FUNCTION 0060c370
	void OnMusicBar()
	{
		m_uiFlags |= 0x80000000;
		m_oJellySquash.m_fStiffness = ms_pBlobCommonProperties->m_afUnkBC[ 2 ];
		m_oJellySquash.SetDampingRatio( ms_pBlobCommonProperties->m_afUnkBC[ 3 ] );
		m_oJellySquash.m_fPosition = ms_pBlobCommonProperties->m_afUnkBC[ 1 ];
		m_oJellySquash.m_fVelocity = 0.0f;
	}

	// The music's beat (AMusicManager's beat feedback): as OnMusicBar with m_afUnkBC[ 4 ] to [ 6 ], unless a bar or
	// beat is already squashing (BFLAG 0x80000000)
	// $deBlob: FUNCTION 0060c3d0
	void OnMusicBeat()
	{
		if ( ( m_uiFlags & 0x80000000 ) == 0 )
		{
			m_uiFlags |= 0x80000000;
			m_oJellySquash.m_fStiffness = ms_pBlobCommonProperties->m_afUnkBC[ 5 ];
			m_oJellySquash.SetDampingRatio( ms_pBlobCommonProperties->m_afUnkBC[ 6 ] );
			m_oJellySquash.m_fPosition = ms_pBlobCommonProperties->m_afUnkBC[ 4 ];
			m_oJellySquash.m_fVelocity = 0.0f;
		}
	}

	// Changes the paint colour (ABlobColourComponent::COLOUR_*, 0x10: ink): sounds, effects and the counters
	void ChangeColour( TUINT32 a_uiColour );

	// The paint points (m_iPaintPoints) and the size they give
	void SetPaintPoints( TUINT32 a_uiPoints );
	// $deBlob: FUNCTION 00580490
	TINT    GetPaintPoints() const { return m_iPaintPoints; }
	TUINT32 RemovePaintPoints( TUINT32 a_uiPoints, TBOOL a_bOnlyControllable );
	void    AddPaintPoints( TUINT32 a_uiPoints );
	// Out of paint points while inky: Blob dies (STATE_DEAD, or 12 with a_bFail, 14 when the last life is gone in single
	// player)
	void OnOutOfPoints( TBOOL a_bFail );

	// In the water: washes the paint or ink off (white)
	void WashOff();

	// Multiplayer game type 1: another blob became "it", this one is washed and becomes an ink blob (30 paint points,
	// size 3, or 10 with BFLAG 0x200000)
	void BecomeInkBlob();

	// Destroys and creates the body again (keeping its velocities and damping)
	void RecreatePhysics();
	// m_iSize changed (from m_iOldSize): plays the HUD animation, keeps Blob on the ground, rebuilds the body, vibrates and
	// plays the size up/down sound
	void OnSizeChanged();
	void ClearZPlayerAttack();

	// In state 1, sets the camera button (0x20) of this physics step (the Z-jump ended)
	// $deBlob: FUNCTION 007453a0
	void PressCameraButton()
	{
		if ( m_eState == 1 )
			m_uiButtons |= 0x20;
	}

	// The water exit while a Z-jump is going on
	TBOOL TryLiquidZJump( TFLOAT a_fTimeStep );
	// Launches the Z-jump at the Z-jump component's target from the body moved by 0.2 * a_rcOffset: sets the body's
	// launch velocity; TFALSE if the Z-jump can't go on
	TBOOL LaunchZJump( TFLOAT a_fTimeStep, const Toshi::TVector4& a_rcOffset );
	// On the ground (not on a 0x2004000 surface, no jump cool-down, no LeechBot attached): the jump button launches the
	// Z-jump off the ground's normal and goes to STATE_BALLISTIC
	TBOOL TryZJumpFromGround( TFLOAT a_fTimeStep );
	// A Z-jump off the wall (m_oMotion.m_vWallNormal) when the jump button was just pressed
	TBOOL TryZJumpFromWall( TFLOAT a_fTimeStep );
	// The launch speed that reaches a_fHeight against a_fGravity ( t = sqrt( 2h / g): h / t + g t / 2)
	static TFLOAT GetLaunchSpeed( TFLOAT a_fHeight, TFLOAT a_fGravity );
	// In the air (no LeechBot attached): the jump button launches the Z-jump and goes to STATE_BALLISTIC
	TBOOL TryZJumpFromAir( TFLOAT a_fTimeStep );
	// Can the Blob vision scene open (rolling, the HUD shown alone, no Z-jump, one player...)?
	TBOOL CanEnterBlobVision();
	// The Z-jump attack flight towards the locked target: moves the ball along the arc; TRUE once it's there (or has
	// nothing to fly to)
	TBOOL UpdateZJumpFlight( TFLOAT a_fTimeStep );
	// The Z-jump at another player's blob: up over it, onto it, then pushed through (Z_ATTACKSTAGE 1-4); TFALSE while
	// flying
	TBOOL UpdateZPlayerAttack( TFLOAT a_fTimeStep );

	// Bounces off an ink surface (STATE 2)
	void InkBounce();
	// Debug: 10 more paint points (at most 100) for the current player's blob
	static void Debug_AddPaintPoints();
	// Adds a_iSeconds to the landmark time (0x3C4), only while the blob is controllable if a_bOnlyControllable
	void AddLandmarkTime( TINT a_iSeconds, TBOOL a_bOnlyControllable );
	// Switches the shield (BFLAG 0x400000) on/off with the animation states 0x17/0x18 (and a squeeze sound when it comes
	// on)
	void SetShield( TBOOL a_bShield );
	TINT AddLeechbot();
	void RemoveLeechbot();

	// Multiplayer: whether this blob can ink a_pOther by the game type's colour rules (iMpType assert)
	TBOOL CanInkBlob( ABlobEntity* a_pOther );
	// Multiplayer: this blob is inked by bumping into a_pOther, TFALSE with BFLAG 0x400000 (the shield) or while this
	// blob Z-jumps at a player and a_pOther is smaller, or as big and higher
	TBOOL LosesBlobBump( ABlobEntity* a_pOther );
	// Gets inked (colour 0x10, a paint point lost, hit state): by an inked blob (a_pAttacker, unused)
	void GetInked( ABlobEntity* a_pAttacker );
	// Multiplayer game type 1: this blob is uncoloured while another blob is painted
	TBOOL IsEvilBlob();
	// Holds the body at its position and pushes it down (velocity -80 Y)
	void PushDown();
	// Sets the paint points to m_iPaintPoints + 10 (at most 100)
	void RefillPaint();
	// The Z-jump (state 9) is locked on a player's entity
	TBOOL IsZJumpingAtPlayer();

private:
public:
	ABlobAnimationComponent* GetAnimationComponent() const { return m_pAnimationComponent; }

	// The end of a multiplayer match: the winner cheers (state 13), the others lose (state 12)
	// $deBlob: FUNCTION 00748a30
	void SetStateMPWinner() { SetState( 13 ); }
	// $deBlob: FUNCTION 00748a40
	void SetStateMPLoser() { SetState( 12 ); }

	// Touching water or ink (CollisionCache::FLAG_HIT_WATER | FLAG_HIT_INK of m_oCollData.m_uiFlags)
	TBOOL IsInLiquid() const { return ( m_oMotion.m_uiContactFlags & 3 ) != 0; }

	// The point where the blob touches the water or ink
	// $deBlob: FUNCTION 0058caa0
	const Toshi::TVector4& GetLiquidHitPosition() const
	{
		// NOTE: retail asserts ( m_oCollData.m_uiFlags & CollisionCache::FLAG_HIT_WATER ) || ( m_oCollData.m_uiFlags & CollisionCache::FLAG_HIT_INK )
		TASSERT( IsInLiquid() );
		return m_oMotion.m_vLiquidHit;
	}

	// Updates the blob after the physics step: the controller input and the model transforms
	void PostPhysicsUpdate( TFLOAT a_fDeltaTime, TINT a_iController );

	// The state of m_pZJump (+0x04)
	TINT GetZJumpState() const { return m_pZJump->GetState(); }
	// $deBlob: FUNCTION 00762730
	APropTriggerHelp* GetHelpTrigger() const { return m_pHelpTrigger; }
	// $deBlob: FUNCTION 00762770
	void             SetHelpTrigger( APropTriggerHelp* a_pTrigger ) { m_pHelpTrigger = a_pTrigger; }
	APropWindCannon* GetWindCannon() const { return m_pWindCannon; }
	void             SetWindCannon( APropWindCannon* a_pCannon ) { m_pWindCannon = a_pCannon; }
	// $deBlob: FUNCTION 00580570
	ABlobZJumpComponent* GetZJumpComponent() const { return m_pZJump; }
	// The current paint colour (ABlobColourComponent::COLOUR_*, 0x10: ink)
	TUINT32               GetPaintColour() const { return m_pColourComponent->GetCurrentColour(); }
	ABlobColourComponent* GetColourComponent() const { return m_pColourComponent; }

	// The paint meter (ABlobSizeHud, ABlobSizeHud::SetPlayer) and the colour indicator (ABlobAmmoHud) of the player
	// $deBlob: FUNCTION 005804d0
	ABlobSizeHud* GetSizeHud() const { return m_pSizeHud; }
	// $deBlob: FUNCTION 0075d420
	ABlobAmmoHud* GetAmmoHud() const { return m_pAmmoHud; }

	// $deBlob: FUNCTION 0065adb0
	void SetSizeHud( ABlobSizeHud* a_pSizeHud ) { m_pSizeHud = a_pSizeHud; }

	// Sets the ammo HUD and shows the current colour on it
	void SetAmmoHud( ABlobAmmoHud* a_pAmmoHud );

	// The shake progress (0-100) on a hotplate, a landmark or a boss (STATE_HOTPLATE-STATE_BOSS), 0 otherwise
	TINT GetShakeProgress() const;
	TINT GetNumLeechbotsAttached() const { return m_iLeechbotsAttached; }
	// $deBlob: FUNCTION 007b88f0
	TINT GetMaxLeechbotsAttached() const { return m_iMaxLeechbotsAttached; }

	TFLOAT                  GetUntargetableTime() const { return m_fUntargetableTime; }
	void                    SetUntargetableTime( TFLOAT a_fValue ) { m_fUntargetableTime = a_fValue; }
	TUINT16                 GetNumTurretsTargettingMe() const { return TUINT16( m_uiTargettingAndShakes ); }
	const Toshi::TMatrix44& GetPhysicsMatrix() const { return m_oPhysicsMatrix; }

	// Read by ABuildingInstance
	// $deBlob: FUNCTION 00578630
	TINT    GetSize() const { return m_iSize; }
	TUINT32 GetBlobFlags() const { return m_uiFlags; }
	void    AddBlobFlags( TUINT32 a_uiFlags ) { m_uiFlags |= a_uiFlags; }

	// Starts the level exit (state 10) towards a_rFacing for the level complete trigger a_pTrigger
	void StartLevelExit( ABaseEntity* a_pTrigger, const Toshi::TVector4& a_rFacing );
	// $deBlob: FUNCTION 00578640
	ABlobSoundComponent* GetSoundComponent() const { return m_pSoundComponent; }

	// The body's velocity, relative to the ground's when standing on a moving object
	void GetBlobVelocity( Toshi::TVector4& a_rOutVelocity ) const;

private:
	// Places the ball and character models at the body's transform
	void UpdateModelTransforms( TFLOAT a_fDeltaTime );

public:
	// The body's position (m_oPhysicsMatrix's translation, 0x364)
	const Toshi::TVector4& GetBodyPosition() const { return m_oPhysicsMatrix.GetTranslation(); }
	// $deBlob: FUNCTION 005804a0
	Toshi::TPhysicsObjectContainer* GetBody() const { return m_pBody; } // 0x3E8

public:
	friend class AAISteer;
	friend class APropWindCannon;
	friend class ABlobSplatTrail;
	friend class AChallenge;
	friend class AChallengeManager;
	friend class ABlobChallengeEndHud;
	friend class ABlobChallengeScene;
	friend class ABlobFollowCamera;
	friend class ABlobAnimationComponent;
	friend class AStencilShadow;
	friend class ABlobVisionHud;
	friend class ABlobPauseMenu;
	friend class APropTriggerHudMessage;
	friend class APropWaterHydrant;
	friend class ABlobColourComponent;
	friend class ABlobRibbonTrailComponent;
	friend class ABlobNavRingComponent;
	friend class ABlobZJumpComponent;
	friend class ABlobSoundComponent;
	friend class ALandmarkInstance;
	friend class ATurretHelper;
	friend class APropGrassBomb;
	friend class ACar;
	friend class ABlobInteractionHud;

public:
	static CommonProperties* ms_pBlobCommonProperties;

	// The particle effects (LoadEffects).
	static Toshi::TParticleEffect*  ms_pPaintSplashEffect;           // Blob_PaintSplash
	static Toshi::TParticleEffect*  ms_pElectrocutionEffect;         // Blob_Electrocution
	static Toshi::TParticleEffect*  ms_pHotPlateEffect;              // Blob_HotPlate
	static Toshi::TParticleEffect*  ms_pInkedStinkEffect;            // Blob_InkedStink
	static Toshi::TParticleEffect*  ms_pInkSplashEffect;             // Blob_InkSplash_med
	static Toshi::TParticleEffect*  ms_pWaterSplashEffect;           // Blob_WaterSplash_med
	static Toshi::TParticleEffect*  ms_pWaterSplashSlamEffect;       // Blob_WaterSplash_Slam
	static Toshi::TParticleEffect*  ms_pWaterRippleEffect;           // Blob_WaterRipple
	static Toshi::TParticleEffect*  ms_pLandmarkEffect;              // Blob_Landmark
	static Toshi::TParticleEffect*  ms_pUnk9B59C0;                   // (only cleared by ClearEffects)
	static Toshi::TParticleEffect*  ms_pUnk9B59C4;                   // (only cleared by ClearEffects)
	static Toshi::TParticleEffect*  ms_pDeathPopSplatEffect;         // Blob_Death_Pop_Splat
	static Toshi::TParticleEffect*  ms_pDeathPopSprayEffect;         // Blob_Death_Pop_Spray
	static Toshi::TParticleEffect*  ms_pWindLauncherTrailEffect;     // Blob_WindLauncher_Trail
	static Toshi::TParticleEffect*  ms_pRainbowEffect;               // Blob_Rainbow
	static Toshi::TParticleEffect*  ms_pZJumpFailEffect;             // Blob_ZJump_Fail
	static Toshi::TParticleEffect*  ms_pSpikeHaemorrhageClearEffect; // Blob_SpikeHaemorrhageClear
	static Toshi::TParticleEffect*  ms_pMPShieldLoopEffect;          // Blob_MP_Shield_Loop
	static Toshi::TParticleEffect*  ms_pMPShieldHitEffect;           // Blob_MP_Shield_Hit
	static Toshi::TParticleEffect*  ms_pElectricTrapSparksEffect;    // Blob_ElectricTrapSparks
	static Toshi::TParticleEffect*  ms_pLandmarkPaintSplatEffect;    // Blob_Landmark_PaintSplat
	static Toshi::TParticleEffect** ms_apInkCloudEffects;            // Blob_InkCloudUnderwater per colour (17; [ 16 ] is the original)
	static Toshi::TParticleEffect** ms_apInkCloudSmallEffects;       // copies of ms_apInkCloudEffects with 2 particles
	static Toshi::TParticleEffect*  ms_pUnpaintableEffect;           // Blob_Unpaintable
	static Properties*              ms_pBlobProperties;
	static void*                    ms_pBlobRibbonProperties;
	static void*                    ms_pZTargetPriorities;
	static TBOOL                    ms_bControlsLocked;
	static TFLOAT                   ms_fShockwaveFadeStart; // the share of the shockwave's time before it fades out

private:
	ABlobMotionData                  m_oMotion;                   // 0x080
	Toshi::TMatrix44                 m_oPhysicsMatrix;            // 0x334 the body's transform
	Toshi::TMatrix44                 m_oJellyMatrix;              // 0x374
	TINT                             m_eState;                    // 0x3B4 STATE
	TINT                             m_iPaintPoints;              // 0x3B8 ( size * 5 + 5 ) * 2
	TINT                             m_iSize;                     // 0x3BC index of BlobEntityProperties
	TINT                             m_iOldSize;                  // 0x3C0 the size too
	TUINT32                          m_uiLandmarkTime;            // 0x3C4
	ABlobColourComponent*            m_pColourComponent;          // 0x3C8
	ABlobSplatTrail*                 m_pSplatTrail;               // 0x3CC
	ABlobRibbonTrailComponent*       m_pRibbonTrail;              // 0x3D0 the paint trail
	ABlobSoundComponent*             m_pSoundComponent;           // 0x3D4
	ABlobZJumpComponent*             m_pZJump;                    // 0x3D8
	ABlobAnimationComponent*         m_pAnimationComponent;       // 0x3DC the models
	AFXBreakpointComponent*          m_pFXBreakpointComponent;    // 0x3E0
	ABlobNavRingComponent*           m_pNavRing;                  // 0x3E4
	Toshi::TPhysicsObjectContainer*  m_pBody;                     // 0x3E8
	ABlobCollisionEventHandler       m_oCollisionEventHandler;    // 0x3EC
	ABlobPhysicsMover                m_oPhysicsMover;             // 0x3F4
	ABlobPhysicsPlugin               m_oPhysicsPlugin;            // 0x404
	TINT                             m_iUserDataType;             // 0x414 1: the body's user data is { 1, this }
	ABlobEntity*                     m_pUserDataOwner;            // 0x418
	ACollisionObject*                m_pCollisionObject;          // 0x41C (0x14 bytes)
	Toshi::TPhysicsPhantomContainer* m_pLiquidRayCastHelper;      // 0x420 the phantoms (CreatePhysics)
	Toshi::TPhysicsPhantomContainer* m_pUnk424;                   // 0x424
	Toshi::TPhysicsPhantomContainer* m_pJellyQueryHelper;         // 0x428
	Toshi::TPhysicsPhantomContainer* m_pSoftCollisionQueryHelper; // 0x42C
	Toshi::TPhysicsPhantomContainer* m_pPaintParticlePhantomA;    // 0x430
	Toshi::TPhysicsPhantomContainer* m_pPaintParticlePhantomB;    // 0x434
	Toshi::T2DList<LifeNode>         m_oList438;                  // 0x438 the life nodes (APickupLife) of the lives lost to hazards
	ASpring3D                        m_oJelly;                    // 0x440 the jelly: the model's wobble around the body (UpdateJelly)
	ASpring                          m_oJellySquash;              // 0x4A8 (UpdateJelly)
	Toshi::TVector4                  m_vBallOffset;               // 0x4C4
	Toshi::TVector4                  m_vBallPosition;             // 0x4D4
	Toshi::TVector4                  m_vJellyPosition;            // 0x4E4
	Toshi::TVector4                  m_vFacing;                   // 0x4F4 SetFacing
	Toshi::TVector4                  m_vLookRotation;             // 0x504
	Toshi::TVector4                  m_vRespawnPosition;          // 0x514
	Toshi::TVector4                  m_vRespawnRotation;          // 0x524
	Toshi::TVector4                  m_vHoldPosition;             // 0x534 the position held while braking/stopped (BFLAG 0x40000)
	Toshi::TVector4                  m_vPhysicsPosition;          // 0x544 the physics position (SetPhysicsPosition)
	TUINT32                          m_uiFlags;                   // 0x554 BFLAG_*
	TUINT32                          m_uiPrevFlags;               // 0x558 last Tick's m_uiFlags
	TUINT32                          m_uiButtons;                 // 0x55C ABlobInputMap::BUTTON_* of the controller
	TUINT32                          m_uiPrevButtons;             // 0x560 last physics step's m_uiButtons
	TUINT32                          m_auiUnk564[ 6 ];            // 0x564
	TFLOAT                           m_fPrevSquashCompensation;   // 0x57C
	TUINT32                          m_auiUnk580[ 5 ];            // 0x580
	TFLOAT                           m_fAwakeTimer;               // 0x594 set to m_fAwakeDuration while the player gives input
	TFLOAT                           m_fAwakeDuration;            // 0x598
	TFLOAT                           m_fUnk59C;                   // 0x59C
	TFLOAT                           m_fLookTurnSpeed;            // 0x5A0
	TFLOAT                           m_fUntargetableTime;         // 0x5A4
	TUINT32                          m_auiUnk5A8[ 6 ];            // 0x5A8
	TUINT32                          m_uiUnk5C0;                  // 0x5C0
	TUINT32                          m_auiUnk5C4[ 3 ];            // 0x5C4
	Toshi::TVector4                  m_vCameraDirection;          // 0x5D0
	Toshi::TVector4                  m_vJumpDirection;            // 0x5E0
	Toshi::TVector4                  m_vJumpStart;                // 0x5F0
	Toshi::TVector4                  m_vClimbStart;               // 0x600
	TINT8                            m_iController;               // 0x610
	Toshi::TVector4                  m_vUnk614;                   // 0x614 (unaligned in retail)
	TFLOAT                           m_fSquash;                   // 0x624
	TFLOAT                           m_fSquashCompensation;       // 0x628
	Toshi::TVector4                  m_vStretchDirection;         // 0x62C
	TFLOAT                           m_fStretch;                  // 0x63C
	TFLOAT                           m_fStretchCompensation;      // 0x640
	TFLOAT                           m_fStretchTimeLeft;          // 0x644 the stretch time left (UpdateJelly), from Properties::m_fStretchTime
	Toshi::TMaterial*                m_pMaterialTestSplat;        // 0x648
	Toshi::TMaterial*                m_pMaterialUnk64C;           // 0x64C destroyed with the others (never created)
	Toshi::TMaterial*                m_pMaterialThread;           // 0x650
	Thread                           m_aThreads[ 16 ];            // 0x654
	ThreadList                       m_oFreeThreads;              // 0xA94
	ThreadList                       m_oActiveThreads;            // 0xAA0
	TUINT32                          m_auiUnkAAC[ 2 ];            // 0xAAC
	TFLOAT                           m_fThreadDistanceSq;         // 0xAB4 the squared distance from m_vThreadPosition after which threads are left
	Toshi::TVector4                  m_vThreadPosition;           // 0xAB8 the ball's position when threads were last left
	TINT                             m_iLeechbotsAttached;        // 0xAC8 > 0: the body is a list of spheres (one per LeechBot)
	TINT                             m_iMaxLeechbotsAttached;     // 0xACC the most attached at once since the last time none were
	union
	{
		TUINT32 m_uiTargettingAndShakes; // 0xAD0
		struct
		{
			TUINT16 m_uiNumTurretsTargettingMe; // 0xAD0 the turrets targeting this blob (ATurretHelper)
			TINT16  m_iShakesDone;              // 0xAD2 the shakes done
		};
	};
	TUINT16       m_uiShakesNeeded;      // 0xAD4
	TUINT32       m_auiSounds[ 17 ];     // 0xAD8 the effects: [0] the electrocution TParticleEffect, [4..16] TParticleEffectInstance*s
	ABlobSizeHud* m_pSizeHud;            // 0xB1C
	ABlobAmmoHud* m_pAmmoHud;            // 0xB20
	TINT          m_iZPlayerAttackStage; // 0xB24 Z_ATTACKSTAGE (the Z-jump at another player's blob)
	TFLOAT        m_fZPlayerAttackTime;  // 0xB28 the time in the stage
	TINT          m_iCamShakeHotPlateID; // 0xB2C
	TINT          m_iCamShakeID;         // 0xB30
	TFLOAT        m_fUnkB34;             // 0xB34
	union
	{
		ABoss*             m_pBoss;     // 0xB38 the boss Blob shakes (STATE_BOSS)
		ALandmarkInstance* m_pLandmark; // 0xB38 the landmark Blob shakes (STATE_LMARK)
	};
	TINT                    m_iCurrentLandmarkTrigger; // 0xB3C -1: none
	APropTriggerHelp*       m_pHelpTrigger;            // 0xB40 the help trigger the blob is in (its read button)
	APropTriggerHudMessage* m_pHudMessageTrigger;      // 0xB44 the last HUD message trigger
	APropHazard*            m_pHotPlate;               // 0xB48 the hot plate the blob bounces off (STATE_HOTPLATE)
	APropWindCannon*        m_pWindCannon;             // 0xB4C the wind cannon the blob is near or in (STATE_CANNON)
	ABaseEntity*            m_pLevelExitTrigger;       // 0xB50 StartLevelExit
	TUINT8                  m_bInBlobVision;           // 0xB54
	TUINT8                  m_bLifeLostShown;          // 0xB55 the ABlobLifeLostHud is shown (cleared by its EndTransBackFrom)
	TUINT8                  m_bHasRespawnRotation;     // 0xB56 m_vRespawnRotation is used as the respawn rotation
	TUINT8                  m_bRespawnPlaySeq;         // 0xB57 (cleared with 0xB58 as a TUINT16)
	TUINT32                 m_uiRespawnTransSeq;       // 0xB58
	TUINT32                 m_uiRespawnSeqStartRatio;  // 0xB5C
	TFLOAT                  m_fRespawnSeqEndRatio;     // 0xB60
	TBOOL                   m_bUnkB64;                 // 0xB64
	TUINT32                 m_uiUnkB68;                // 0xB68
	TINT                    m_iStartPaintPoints;       // 0xB6C
	TINT                    m_iStartColour;            // 0xB70
	void*                   m_pShockwaveModel;         // 0xB74 TModelInstance
	TFLOAT                  m_fShockwaveTime;          // 0xB78
	TFLOAT                  m_fShockwaveDuration;      // 0xB7C
	TFLOAT                  m_fShockwaveAlpha;         // 0xB80
	Toshi::TVector4         m_vShockwavePosition;      // 0xB84
	Toshi::TVector4         m_vShockwaveNormal;        // 0xB94
	Toshi::TTexture*        m_pShockwaveNoise;         // 0xBA4

#ifdef DEBLOB_COMMUNITY_PATCH
	// The render interpolation (UpdateModelTransforms): the models are drawn between the body's last two fixed steps
	// instead of at the last one, so they don't twitch when the frame rate is above the 60 Hz physics rate
	Toshi::TMatrix44 m_mRenderPrev;  // m_oPhysicsMatrix before the steps of m_iRenderSteps
	Toshi::TMatrix44 m_mRenderLast;  // m_oPhysicsMatrix at the last step seen
	TUINT32          m_uiRenderStep; // TPhysicsManager::GetTotalSteps() when m_mRenderLast was taken
	TINT             m_iRenderSteps; // the steps between m_mRenderPrev and m_mRenderLast (0: no interpolation)
	Toshi::TMatrix44 m_mRenderCurrent; // the result of the last GetRenderPhysicsMatrix
#endif
};
