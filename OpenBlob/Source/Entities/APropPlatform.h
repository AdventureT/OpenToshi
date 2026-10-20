#pragma once
#include "Entities/AZJumpData.h"
#include "Entities/AProp.h"
#include "Entities/AWaypointData.h"
#include "Entities/PlatformSpring.h"
#include "Entities/ASpring.h"
#include "Entities/APlatformManager.h"
#include "Entities/AEntityHashFilter.h"
#include <Math/TQuaternion.h>
#include <Render/TMaterial.h>
#include <Platform/DX11/TMaterial_DX11.h>

TOSHI_NAMESPACE_START
class TParticleEffect;
class TAnimation;
class TModelInstance;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a prop that moves along a waypoint spline: sounds, splash damage, debris, delays at the nodes.
//-----------------------------------------------------------------------------
class APropPlatform
    : public AProp
{
public:
	TDECLARE_CLASS( APropPlatform, AProp );

public:
	// m_uiPlatformFlags (names from the entity params where they're read)
	enum PLATFORMFLAG : TUINT32
	{
		PLATFORMFLAG_Linear               = 0x1,        // "Linear"
		PLATFORMFLAG_Bounce               = 0x2,        // "Bounce"
		PLATFORMFLAG_Continuous           = 0x4,        // "Continuous"
		PLATFORMFLAG_Reset                = 0x8,        // "Reset"
		PLATFORMFLAG_Reverse              = 0x10,       // "Reverse"
		PLATFORMFLAG_Backwards            = 0x20,       // moving from m_iNextPoint back to m_iCurPoint
		PLATFORMFLAG_RotateYOnly          = 0x40,       // "RotateYOnly"
		PLATFORMFLAG_LockAllAxis          = 0x80,       // "LockAllAxis"
		PLATFORMFLAG_Toggled              = 0x100,      // follows the toggled links (AWaypointData::Node::FLAG_Toggled)
		PLATFORMFLAG_LinearSpeed          = 0x200,      // "LinearSpeed"
		PLATFORMFLAG_Stopped              = 0x400,      // reached the end of the path
		PLATFORMFLAG_DamageOnTouch        = 0x800,      // "DamageOnTouch"
		PLATFORMFLAG_Invulnerable         = 0x1000,     // "HitPoints" == -1
		PLATFORMFLAG_Oscillation          = 0x2000,     // "Oscillation"
		PLATFORMFLAG_BobOnEnd             = 0x4000,     // "BobOnEnd"
		PLATFORMFLAG_UnbalancePlayers     = 0x8000,     // "UnbalancePlayers"
		PLATFORMFLAG_Distance             = 0x20000,    // "Distance" > 0 (m_pDistanceSpring)
		PLATFORMFLAG_ResetToRoot          = 0x40000,    // "ResetToRoot"
		PLATFORMFLAG_Targetable           = 0x80000,    // "Targetable"
		PLATFORMFLAG_TargetableFromBelow  = 0x100000,   // "TargetableFromBelow"
		PLATFORMFLAG_Delayed              = 0x200000,   // waits m_fDelay
		PLATFORMFLAG_TurnAround           = 0x800000,   // turning around at the end of a ping-pong path
		PLATFORMFLAG_IdleAnimAlways       = 0x1000000,  // the idle animation also plays below the high LOD
		PLATFORMFLAG_Conveyor             = 0x2000000,  // "EnableConveyor"
		PLATFORMFLAG_MovingSound          = 0x4000000,  // the looping moving sound plays
		PLATFORMFLAG_IdleSound            = 0x8000000,  // the looping idle sound plays
		PLATFORMFLAG_ForceAlwaysTick      = 0x10000000, // "ForceAlwaysTick"
		PLATFORMFLAG_StopWhenTriggered    = 0x20000000, // "StopWhenTriggered"
		PLATFORMFLAG_ReverseWhenTriggered = 0x40000000, // "ReverseWhenTriggered"
		PLATFORMFLAG_StartOff             = 0x80000000, // "StartOff"
	};

	// A sound of the platform ("<Name>SoundID": group and event)
	struct SoundID
	{
		TINT16 iGroup; // 0x0
		TINT16 iEvent; // 0x2
	};

	// The Hermite segment the platform moves along (APropPlatform::CacheSplineVectors)
	struct Spline
	{
		Toshi::TVector4 vStart;        // 0x00
		Toshi::TVector4 vStartTangent; // 0x10
		Toshi::TVector4 vEnd;          // 0x20
		Toshi::TVector4 vEndTangent;   // 0x30
	};

	// The particle effect started before the delay at a node ends ("Effect Name", 0xC bytes)
	struct EffectData
	{
		Toshi::TParticleEffect* pEffect;      // 0x0
		TFLOAT                  fWarningTime; // 0x4 "Warning Time"
		SoundID                 oSound;       // 0x8
	};

	// A pass of the conveyor's material whose V offset scrolls (the original is restored after rendering)
	struct ConveyorPass
	{
		Toshi::T2Material::Pass* pPass;       // 0x0
		TFLOAT                   fOrigOffset; // 0x4 Pass::m_fUnk24
	};

	// "EnableConveyor" (0x1C bytes)
	struct Conveyor
	{
		Toshi::TMaterial** ppMaterials;    // 0x00 a copy of the material of each mesh of LOD 0 ("conveyor")
		TFLOAT             fSpeed;         // 0x04 "ConveyorSpeed"
		TFLOAT             fCurrentSpeed;  // 0x08 0 while it's off
		TFLOAT             fScrollSpeed;   // 0x0C "TextureScrollSpeedV"
		TFLOAT             fCurrentScroll; // 0x10
		ConveyorPass*      pPasses;        // 0x14 at most 8
		TINT               iNumPasses;     // 0x18
	};

	// "Bounce" (0x30 bytes)
	struct Bounce
	{
		Bounce()
		    : oSpring( 0.0f, 20.0f, 9.0f )
		{}

		ASpring oSpring;            // 0x00
		TFLOAT  fMinCompression;    // 0x1C "MinCompression"
		TFLOAT  fMaxCompression;    // 0x20 "MaxCompression"
		TFLOAT  fMinWeightTransfer; // 0x24 "MinWeightTransfer"
		TFLOAT  fMaxWeightTransfer; // 0x28 "MaxWeightTransfer"
		TFLOAT  fMaxSpringTravel;   // 0x2C "MaxSpringTravel"
	};

	// Where a respawned platform continues: only the members read here
	struct RespawnData
	{
		Toshi::TVector4     vCurPointPos; // 0x00 m_vCurPointPos
		TFLOAT              fSpeed;       // 0x10 m_fSpeed
		TFLOAT              fSpeedScale;  // 0x14 m_fSpeedScale
		TFLOAT              fPathLength;  // 0x18 m_fPathLength
		AWaypointData::Set* pWaypointSet; // 0x1C
		TFLOAT              fSplineT;     // 0x20
		TINT16              iCurPoint;    // 0x24
		TINT16              iNextPoint;   // 0x26
		TINT16              iStartPoint;  // 0x28
		TUINT8              uiFlags;      // 0x2A
	};


	// Accepts the entities that can be hurt (flags 0x10 and 0x20) but the platform itself
	class DamageableFilter
	    : public AEntityHashFilter
	{
	public:
		// $deBlob: FUNCTION 0079a290
		DamageableFilter( APropPlatform* a_pPlatform )
		    : m_pPlatform( a_pPlatform ) {}

		virtual TBOOL operator()( ABaseEntity* a_pEntity ) override; // +0x00

	private:
		APropPlatform* m_pPlatform; // 0x04
	};

public:
	APropPlatform();
	virtual ~APropPlatform() override; // +0x08

	virtual void            Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void            OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void            SyncBodyTransform() override;                                                        // +0x14
	virtual void            PostCreate() override;                                                               // +0x20
	virtual void            Destroy() override;                                                                  // +0x24
	virtual void            ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void            SetPosition( const Toshi::TVector4& a_rPosition ) override;                          // +0x40
	virtual void            HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual Toshi::TVector4 GetReactionForce( void* a_pVelocity, void* a_pEntity ) override;                     // +0x60
	// $deBlob: FUNCTION 0079a590
	virtual TBOOL IsTargetable() override { return ( m_uiPlatformFlags >> 19 ) & 1; } // +0xA0
	// $deBlob: FUNCTION 0076dc80
	virtual TBOOL CanBeTargetedFromBelow() override { return ( m_uiPlatformFlags >> 20 ) & 1; } // +0xA4
	// $deBlob: FUNCTION 0076dcb0
	virtual TFLOAT GetAttackSlamHeight() override { return m_fAttackSlamHeight; } // +0xB0
	virtual void   Reset( ABaseEntity* a_pEntity ) override;                      // +0xC8
	virtual void   OnPoolRelease() override;                                      // +0xD0
	virtual void   Despawn( TBOOL a_bPlayEffect ) override;                       // +0xE4
	// $deBlob: FUNCTION 0076dcd0
	virtual void* GetDespawnEffect() override { return m_pDespawnEffect; } // +0xE8
	// $deBlob: FUNCTION 0076dc90
	virtual TBOOL TicksAtLowLOD() { return ( TUINT8( m_uiPlatformFlags >> 28 ) & 1 ) || ( TUINT8( m_uiPlatformFlags >> 21 ) & 1 ); }                            // +0xF4
	virtual void  SetWaypointSet( TINT a_iWaypointSetID, TINT a_iStartPoint, const Toshi::TVector4* a_pPosition, RespawnData* a_pRespawnData ); // +0xF8
	virtual void  StartFromCurrentNode();                                                                                                       // +0xFC
	// $deBlob: FUNCTION 0076df30
	virtual void OnStartMoving() {} // +0x100
	virtual void StopMoving();      // +0x104
	// $deBlob: FUNCTION 0076de40
	virtual void SetTurretActive( TBOOL a_bActive ) {} // +0x108
	virtual void StartNodeDelay( TINT16 a_iNode );     // +0x10C
	// $deBlob: FUNCTION 00779850
	virtual TBOOL                QueuesBehindOthers() { return TFALSE; }                                              // +0x110
	virtual AWaypointData::Node* GetNextNode( TINT16 a_iNode, TBOOL a_bReverse, TBOOL a_bToggled, TINT* a_pOutNode ); // +0x114
	// $deBlob: FUNCTION 0076de30
	virtual void AdjustNodePosition( Toshi::TVector4* a_pOut, const Toshi::TVector4* a_pFrom, const Toshi::TVector4* a_pNodePos, TBOOL a_bNoRandomness ) {} // +0x118

	// Writes where the platform is on its route to a_pRespawnData (the civilians' respawn data)
	void SaveRespawnData( RespawnData* a_pRespawnData );
	// Moves a respawned platform along its route by a_fDeltaTime and writes the position to a_pPosition (the civilians'
	// walk). FALSE when the route is missing. NOTE: the retail call passes a 0 and the facing, both unused
	static TBOOL UpdateRespawnRoute( TFLOAT a_fDeltaTime, RespawnData* a_pRespawnData, Toshi::TVector4* a_pPosition );

public:
	TBOOL IsConveyor() const { return ( m_uiPlatformFlags & PLATFORMFLAG_Conveyor ) != 0; }
	TBOOL IsStopped() const { return ( m_uiPlatformFlags & PLATFORMFLAG_Stopped ) != 0; }

	// Are all the buildings among the instance targets painted in a_uiColour? (ABossTrigger)
	TBOOL                           AreInstancesColour( TUINT8 a_uiColour );
	APlatformManager::PlatformNode& GetPlatformNode() { return m_oPlatformNode; }
	// The Z-jump node of the platform (ABlobZJumpComponent)
	AZJumpData::Node* GetZJumpNode() const { return TSTATICCAST( AZJumpData::Node, m_pZJumpNode ); }

	// Whether this platform is ahead of the platform a_iPlatformID on the link a_iFrom-a_iTo within a_fDistance
	TBOOL IsBlocking( TINT a_iPlatformID, TINT a_iFrom, TINT a_iTo, TFLOAT a_fDistance );

	void               RemoveFromWaypointSet();
	Toshi::TMaterial** GetConveyorMaterials() const;
	TFLOAT             GetConveyorScroll() const;
	void               RestoreConveyorPasses();
	void               SetConveyorPassOffset( TFLOAT a_fOffset );

protected:
	// The looping/one-shot sounds (m_oMovingSound, m_oIdleSound, m_oStartSound, m_oStopSound)
	void StopMovingSound();
	void PlayMovingSound();
	void PlayIdleSound();
	void StopIdleSound();
	void PlayStartSound();
	void PlayStopSound();
	void PlayOnActiveSound();
	// Switches the sounds to a_iState (m_iSoundState: 0 stopped, 1 moving, 5 none; 2, 3, 4 play the start, stop and
	// on-active sounds without changing the state)
	void SetSoundState( TINT a_iState );
	// The moving sound as a positional loop (the conveyors)
	void StartMovingSoundLoop();
	void StopMovingSoundLoop();

	// The conveyor's materials and scrolling passes (FinishParams), its render method
	void CollectConveyorPasses();
	void AddConveyorPasses( Toshi::TMaterial* a_pMaterial );
	void CreateConveyorMaterials();
	static void __stdcall RenderConveyor( Toshi::TModelInstance* a_pModelInstance, void* a_pUserData );

	// Hurts the entities near the destroyed platform / the entity that touched it (MESSAGE_PROPATTACK)
	void SplashDamage();
	void DamageCaller( ABaseEntity* a_pCaller );

	void SetMovingSound( TINT16 a_iGroup, TINT16 a_iEvent );
	void SetIdleSound( TINT16 a_iGroup, TINT16 a_iEvent );
	// Sets the moving sound if the params have none
	void SetDefaultMovingSound( TINT16 a_iGroup, TINT16 a_iEvent );

	// The end of ProcessParams: the distance spring, the speed, the hit points and the conveyor
	void SetPlatformFlag( TUINT32 a_uiFlag, TBOOL a_bSet )
	{
		if ( a_bSet )
			m_uiPlatformFlags |= a_uiFlag;
		else
			m_uiPlatformFlags &= ~a_uiFlag;
	}

	void   FinishParams();
	void   UpdateConveyor();
	void   UpdateOrientation( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo );
	void   RestartFromStart();
	void   RestartFromRoot();
	void   ComputeSpeedScale( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo );
	void   SetSplineForward( const Toshi::TVector4& a_rPrev, const Toshi::TVector4& a_rCur, const Toshi::TVector4& a_rNext, TINT16 a_iNextNode, Spline* a_pSpline, TBOOL a_bToggle );
	void   SetSplineReverse( const Toshi::TVector4& a_rAfter, const Toshi::TVector4& a_rCur, const Toshi::TVector4& a_rNext, TINT16 a_iCurNode, Spline* a_pSpline, TBOOL a_bToggle );
	void   SetupRespawn( TINT a_iStartPoint, const Toshi::TVector4* a_pPosition, RespawnData* a_pRespawnData );
	void   ToggleNavSets( TINT16 a_iNode );
	TFLOAT ComputeMaximumNodeSeperation();

	static void                 CacheSplineVectors( const Toshi::TVector4& a_rPrev, const Toshi::TVector4& a_rCur, const Toshi::TVector4& a_rNext, const Toshi::TVector4& a_rAfterNext, Spline* a_pSpline );
	static AWaypointData::Node* FindNextNode( AWaypointData::Set* a_pSet, TINT16 a_iNode, TBOOL a_bReverse, TBOOL a_bToggled, TINT* a_pOutNode );



protected:
	Toshi::TVector4                m_vInitialPosition;  // 0xB4 (PostCreate)
	Toshi::TQuaternion             m_qInitialRotation;  // 0xC4 (PostCreate)
	Spline                         m_oSpline;           // 0xD4
	Toshi::TVector4                m_vCurPointPos;      // 0x114
	Toshi::TVector4                m_vNextPointPos;     // 0x124
	Toshi::TVector4                m_vVelocity;         // 0x134
	SoundID                        m_oMovingSound;      // 0x144 "MovingSoundID"
	SoundID                        m_oIdleSound;        // 0x148 "IdleSoundID"
	SoundID                        m_oStartSound;       // 0x14C "StartSoundID"
	SoundID                        m_oStopSound;        // 0x150 "StopSoundID"
	SoundID                        m_oOnActiveSound;    // 0x154 "OnActiveSoundID"
	TUINT32                        m_uiPlatformFlags;   // 0x158 PLATFORMFLAG
	TINT16                         m_iIdleAnim;         // 0x15C "PlayIdleAnimation" (anim_idle)
	TINT16                         m_iTouchAnim;        // 0x15E "PlayTouchAnimation" (anim_touch)
	Toshi::TAnimation*             m_pIdleAnim;         // 0x160
	TFLOAT                         m_fSpeed;            // 0x164 "Speed", divided by m_fPathLength: spline T per second
	TFLOAT                         m_fPathLength;       // 0x168 the longest link of the set
	AWaypointData::Set*            m_pWaypointSet;      // 0x16C m_pWaypointSet
	TINT                           m_iCurPoint;         // 0x170 m_iCurPoint
	TINT                           m_iNextPoint;        // 0x174 m_iNextPoint
	Conveyor*                      m_pConveyor;         // 0x178 "EnableConveyor" (0x1C bytes)
	TFLOAT                         m_fSplineT;          // 0x17C
	TFLOAT                         m_fBaseSpeed;        // 0x180 "Speed" as read
	TINT                           m_iStartPoint;       // 0x184 m_iStartPoint
	TFLOAT                         m_fNodeDelay;        // 0x188 the delay at the current node
	TFLOAT                         m_fSpeedScale;       // 0x18C "LinearSpeed": the path length over the segment length
	TFLOAT                         m_fTouchCooldown;    // 0x190
	PlatformSpring*                m_pDistanceSpring;   // 0x194 "Distance" > 0
	TFLOAT                         m_fDistance;         // 0x198 "Distance"
	TFLOAT                         m_fAccelDown;        // 0x19C "AccelDown" 20.0f
	TFLOAT                         m_fAccelUp;          // 0x1A0 "AccelUp" 1.0f
	TFLOAT                         m_fDampeningSpeed;   // 0x1A4 "DampeningSpeed" 0.2f
	PlatformSpring*                m_pOscillation;      // 0x1A8 "Oscillation"
	TINT                           m_iHitPoints;        // 0x1AC "HitPoints" -1, the hit points left
	TINT                           m_iMaxHitPoints;     // 0x1B0 "HitPoints" as loaded
	TFLOAT                         m_fDamageRadius;     // 0x1B4 "DamageRadius"
	TINT                           m_iDamage;           // 0x1B8 "Damage"
	TINT16                         m_iRecoil;           // 0x1BC "Recoil" 0
	TINT16                         m_iSoundState;       // 0x1BE 0 stopped, 1 moving, 5 none yet
	TFLOAT                         m_fAttackSlamHeight; // 0x1C0 "AttackSlamHeight"
	void*                          m_pTargetOffsets;    // 0x1C4 m_TargetOffsets: the attached instances' offsets (matrices)
	TFLOAT                         m_fDelayOnTrigger;   // 0x1C8 "DelayOnTrigger"
	TFLOAT                         m_fDelay;            // 0x1CC (PLATFORMFLAG_Delayed)
	APlatformManager::PlatformNode m_oPlatformNode;     // 0x1D0 m_oPlatformNode (APlatformManager)
	Bounce*                        m_pBounce;           // 0x1DC "Bounce"
	Toshi::TParticleEffect*        m_pDespawnEffect;    // 0x1E0 m_despawnEffect
	void*                          m_pZJumpNode;        // 0x1E4 m_pZJumpNode ("ZJumpSetID", "ZJumpNodeID")
	EffectData*                    m_pEffectData;       // 0x1E8 "Effect Name"
	TUINT8                         m_eDelayState;       // 0x1EC 0 moving, 1 waiting at a node
	TUINT8                         m_uiExtraFlags;      // 0x1ED
};
