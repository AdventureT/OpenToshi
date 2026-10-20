#pragma once
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>
#include <GUI/T2GUIRectangle.h>
#include "Entities/AZJumpData.h"

class ABlobEntity;

namespace Toshi
{
class TXUIControl;
class TXUIElement;
class TPhysicsObjectContainer;
class TParticleEffect;
class TParticleEffectInstance;
} // namespace Toshi

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: Blob's Z-jump (lock-on jump) component (no vtable, created by ABlobEntity::Create at 0x3D8): the Z-jump
// state read by the animation and colour components, the priority targets with their off-screen markers
// ("zjumpoffscreen.tga", 15 T2GUIRectangles) and the "Blob_Env_ZJump_Screen" effect.
// NOTE: class, file and the names m_eState, m_pPoint, m_pCurrentPriorityLock, PriorityTarget
//-----------------------------------------------------------------------------
class ABlobZJumpComponent
{
public:
	friend class ABlobEntity; // the Z-jump flight (UpdateZJumpFlight) reads the flight times
	friend class ABlobInteractionHud;

	enum STATE : TINT
	{
		STATE_NONE         = 0,
		STATE_POINT        = 1, // at a Z-jump point in reach
		STATE_NODEFLIGHT   = 2, // jumping to the next node of the point's set
		STATE_AIM          = 3, // holding the aim
		STATE_AIMNODE      = 4, // the aim found a node
		STATE_ATTACKSLAM   = 5,
		STATE_SETEND       = 6, // the last node of the set was reached
		STATE_TIMEOUT      = 7, // the hold time ran out
		STATE_LOCK         = 8, // locked on an entity
		STATE_ATTACKFLIGHT = 9,
		STATE_LOCKLMARK    = 10, // locked on a landmark
		STATE_LMARKFLIGHT  = 11,
		STATE_SWITCHSLAM   = 12, // slamming a pressure switch or a trigger
	};

	// 0x28 bytes
	struct PriorityTarget : public Toshi::T2GenericDList::Node
	{
		// m_uiFlags (bits 0-3: the colour of a landmark trigger)
		enum FLAGS : TUINT32
		{
			FLAGS_PAINTABLE  = 0x10, // a target Blob paints (bits 0-3: the colour it needs, 0: any)
			FLAGS_UNLOCKABLE = 0x20, // shown, but not a target Blob can Z-jump at (UpdateTargetFlags)
			FLAGS_UNSORTED   = 0x40, // still in m_lTargets
			FLAGS_LOCKED     = 0x80, // the current lock is kept (no cycling)
			FLAGS_UNK100     = 0x100,
		};

		// m_eType (the assert's PriorityTarget::TARGET_LMARK)
		enum TARGET : TINT
		{
			TARGET_NONE     = -1,
			TARGET_ENTITY   = 0,
			TARGET_LMARK    = 1,
			TARGET_INSTANCE = 2,
		};

		TBOOL CanPaint( ABlobEntity* a_pBlob ) const;

		// Writes the position of the target itself (an entity's matrix, a landmark trigger, an instance's matrix) to
		// a_rOutPosition, X0Y0Z0W1 and TFALSE without a target
		TBOOL GetBasePosition( Toshi::TVector4& a_rOutPosition ) const;

		TINT                   m_eType;         // 0x08 -1: free, 0: TARGET_ENTITY, 1: TARGET_LMARK, 2: TARGET_INSTANCE
		TFLOAT                 m_fDistance;     // 0x0C the sort key (the squared distance, or a weight)
		TINT                   m_ePriorityType; // 0x10 ZPRIORITY (the index of the group and of ms_pZTargetPriorities)
		TINT                   m_iPriority;     // 0x14 from ms_pZTargetPriorities (-1: not a target)
		TUINT32                m_uiFlags;       // 0x18 FLAGS
		Toshi::T2GUIRectangle* m_pRectangle;    // 0x1C off-screen marker
		void*                  m_pTarget;       // 0x20
		TINT                   m_iIndex;        // 0x24 the trigger of a landmark, the pooled ID of an entity

		// Writes the target's position to a_rOutPosition: type 1 is the landmark trigger m_iIndex of the landmark
		// at m_pTarget, type 0 the entity at m_pTarget (GetTargetOffset + GetBasePosition), type 2 the position behind the
		// target's +0x10 pointer. Without a position, X0Y0Z0W1 is written and TFALSE returned.
		TBOOL GetPosition( Toshi::TVector4& a_rOutPosition ) const;

		// Writes how far above the target the Z-jump lands to a_pfOutHeight (0 for a landmark trigger, the target's
		// height over the entity, or m_afUnk38[ 6 ] + a_fRadius for an instance); a_fRadius is the owner's bounding
		// sphere radius. Returns TFALSE when there is no such target.
		TBOOL GetHeight( TFLOAT* a_pfOutHeight, TFLOAT a_fRadius ) const;
	};

	// 0x10 bytes
	struct PriorityGroup : public Toshi::T2GenericDList::Node
	{
		// $deBlob: FUNCTION 00769ea0
		PriorityGroup() = default;

		// $deBlob: FUNCTION 00769ff0
		// $deBlob: FUNCTION 0076a4a0
		~PriorityGroup() = default;

		Toshi::T2DList<PriorityTarget> m_lLockList; // 0x08
	};

	static TCONSTEXPR_VAR TINT NUM_TARGETS = 15;
	static TCONSTEXPR_VAR TINT NUM_GROUPS  = 38; // ZPRIORITY_COUNT

	// The priority types (the index of the groups; 0x12/0x13: a landmark trigger Blob can't/can paint, 0x24: another
	// Blob, 0x25: the next node of a Z-jump set); NOTE: the names of the bounds are the asserts'
	static TCONSTEXPR_VAR TINT ZPRIORITY_INVALID = -1;
	static TCONSTEXPR_VAR TINT ZPRIORITY_COUNT   = 38;

	// The priorities of a ZPRIORITY in ms_pZTargetPriorities (4 bytes each)
	struct ZTargetPriority
	{
		TINT8 m_iSmall; // 0x00 Blob's m_iPaintPoints is 0
		TINT8 m_iBig;   // 0x01
		TINT8 m_iInk;   // 0x02 Blob is inked
		TINT8 m_iUnk03; // 0x03
	};

public:
	ABlobZJumpComponent( ABlobEntity* a_pOwner );
	~ABlobZJumpComponent();

	void Tick( TFLOAT a_fDeltaTime );
	void Reset();
	// Back to state 0 (ABlobEntity::EnterBoss): Blob's 0xB24 cleared and the lock marker hidden
	void Deactivate();
	// Keeps the contacted body's user data
	void SetContactBody( Toshi::TPhysicsObjectContainer* a_pBody );
	void SetState( TINT a_eState );

	// Drops the priority lock on a_pEntity (an enemy given back to its pool)
	void OnEntityRemoved( ABaseEntity* a_pEntity );

	// Called when Blob's paint colour changes
	void OnColourChanged();

	// Called when Blob jumps (a_fFlightTime, the time of the flight, is kept at 0x158 as a float): moves the Z-jump on
	// from its current state; returns TFALSE when a jump at a target can't go on
	TBOOL OnBlobJump( TFLOAT a_fFlightTime );

	// $deBlob: FUNCTION 0076add0
	TINT GetState() const { return m_eState; }

	// Updates the lock element in states 1, 8 and 10 (the blob's size changed)
	void OnSizeChanged();

	// Cancels the current Z-jump lock/aim (states 1, 4, 8, 10) and hides the target markers
	void Cancel();

	// Blob bounces off ink while aiming (state 3/4): back to state 0, the landing sound, the hold timeline and the lock
	// element are hidden
	void OnInkBounce();
	// Cancel, then frees the targets and groups and goes back to state 0
	void End();
	// The blob recoiled off an enemy (MESSAGE_RECOIL): states 5 and 9 go back to state 0
	void OnRecoil();

	// $deBlob: FUNCTION 00765790
	TBOOL IsUnk120() const { return m_bUnk120; }
	// $deBlob: FUNCTION 00580400
	AZJumpData::Point* GetLastPoint() const { return m_pLastPoint; }
	// $deBlob: FUNCTION 00580410
	PriorityTarget* GetCurrentPriorityLock() const { return m_pCurrentPriorityLock; }

	// The current priority lock is a target the Z-jump can reach (state 10, a landmark)
	TBOOL CanJumpToLandmark() const;

	// The point the Z-jump aims at: the target at 0x24, else the current priority lock, else Blob's body (written to
	// a_rOutPosition, which is returned)
	Toshi::TVector4& GetTargetPosition( Toshi::TVector4& a_rOutPosition ) const;
	// How far above GetTargetPosition the Z-jump lands (0 without a target)
	TFLOAT GetTargetHeight() const;
	// The height above a_pTarget the Z-jump lands at (a_fRadius: the owner's bounding sphere radius)
	static TFLOAT GetEntityHeight( ABaseEntity* a_pTarget, TFLOAT a_fRadius );

	// The current priority lock is a valid entity/landmark target (state 8)
	TBOOL HasValidPriorityLock() const;

	// The ZLock button was pressed: starts the Z-jump at a point in reach (state 1), else the lock on the best target
	// (state 8, or 10 at a landmark), or the aim of state 3 (state 4)
	void StartLock();

	// Blob landed: moves the Z-jump on (the next node in state 2, the end of states 5-7, 9, 11, 12)
	void OnLanded();

	// The HUD's lock element (control 0xE0 of the HUD, ABlobHud::OnCreated) and its children
	void SetLockElement( Toshi::TXUIControl* a_pElement );

	// The X offset of the lock element (the right half of the split screen HUDs not in 16:9: the canvas's X)
	// $deBlob: FUNCTION 006546d0
	void SetLockElementOffsetX( TFLOAT a_fOffsetX ) { m_fLockElementOffsetX = a_fOffsetX; }

	// The Z-jump is locking on (states 8, 9) the entity a_pEntity (the multiplayer threat marker)
	// $deBlob: FUNCTION 00766960
	TBOOL IsLockingOn( ABaseEntity* a_pEntity ) const
	{
		return m_pCurrentPriorityLock != TNULL && ( m_eState == STATE_LOCK || m_eState == STATE_ATTACKFLIGHT ) && m_pCurrentPriorityLock->m_eType == PriorityTarget::TARGET_ENTITY && m_pCurrentPriorityLock->m_pTarget == a_pEntity;
	}

	// The current priority lock is an entity of exact class APropTrigger that needs paint points and leads to a level
	// complete trigger (APropTriggerHudMessage::HandleEntityIO)
	TBOOL IsLockedOnLevelCompleteTrigger() const;

	// The priority of a_ePriorityType for a_pBlob's colour and size
	static TINT GetPriority( TINT a_ePriorityType, ABlobEntity* a_pBlob );

	static ZTargetPriority& GetZTargetPriority( TINT a_ePriorityType );

	// The attack (state 9, 11) ended: a_bHit goes on to state 5 (also ABlobEntity::SetState)
	void EndAttack( TBOOL a_bHit );
	// Ends a charge (states 2 and 4): back to state 0, the hold timeline and the lock element are hidden
	void EndCharge();

private:
	void UpdateLockElement( TBOOL a_bNoTimeline );
	// Hides the lock element and stops its animation
	void HideLockElement();
	// The hold timer of state 3/4 on the HUD (element 0x154 of ABlobHud)
	void UpdateHoldTimeline();
	// Moves the lock element to a_rcTarget on screen and the off-screen markers to the other targets (a_fDeltaTime
	// is unused)
	void UpdateAim( TFLOAT a_fDeltaTime, const Toshi::TVector4& a_rcTarget );
	// Shows the lock element at a_rLock
	void ShowLock( PriorityTarget& a_rLock );
	// Picks the next target of the groups as m_pCurrentPriorityLock
	void SelectLock();

	// The Z-jump point in reach of Blob, or TNULL
	AZJumpData::Point* FindPoint() const;
	// The set of a_pPoint has no entity node
	static TBOOL IsPointWithoutEntities( AZJumpData::Point* a_pPoint );
	// How much the camera looks at a_pNode (2 for m_pStandNode, 1/0 against m_pUnk2C)
	TFLOAT GetNodeWeight( AZJumpData::Node* a_pNode ) const;
	// Is the target of the attack (STATE_ATTACKFLIGHT, STATE_SWITCHSLAM) still there?
	TBOOL IsStillAttackable( PriorityTarget& a_rTarget ) const;

	// A free target (m_lFreeTargets, else one of the last group, else the farthest of m_lTargets when a_fDistance
	// is nearer), unlinked, or TNULL
	PriorityTarget* AllocTarget( TFLOAT a_fDistance );
	// Sets FLAGS_UNLOCKABLE on a_rTarget when Blob can't Z-jump at it
	void UpdateTargetFlags( PriorityTarget& a_rTarget );
	// Is a_rTarget still a target (alive, in reach, in view)? (the assert's IsStillLockable)
	TBOOL IsStillLockable( PriorityTarget& a_rTarget );
	TBOOL IsTargetVisible( PriorityTarget& a_rTarget );

	// Adds the landmark triggers in reach and view to m_lTargets
	void FindLandmarkTargets();
	// Adds the entities in reach and view to m_lTargets (and moves a platform's old target to its new type)
	void FindEntityTargets();
	// Adds the instance nodes linked to the Z-jump node Blob stands on (m_pStandNode) to m_lTargets
	void FindNodeTargets();
	// Frees the grouped targets that aren't lockable any more (the lock goes on to the next)
	void PruneGroups();

	// Sorted inserts (by the priority, FLAGS_UNLOCKABLE and the distance)
	static void InsertTarget( Toshi::T2DList<PriorityTarget>& a_rList, PriorityTarget* a_pTarget );
	static void InsertGroupTarget( Toshi::T2DList<PriorityTarget>& a_rList, PriorityTarget* a_pTarget );
	static void InsertGroup( Toshi::T2DList<PriorityGroup>& a_rList, PriorityGroup* a_pGroup );
	static TINT CompareGroups( PriorityGroup* a_pGroup1, PriorityGroup* a_pGroup2 );
	// The comparisons of two targets
	static TINT __stdcall CompareTargets( PriorityTarget* a_pTarget1, PriorityTarget* a_pTarget2 );
	static TINT __stdcall CompareGroupTargets( PriorityTarget* a_pTarget1, PriorityTarget* a_pTarget2 );

	// Gives a_rTarget (unlinked) back to m_lFreeTargets
	void FreeTarget( PriorityTarget& a_rTarget );
	void FreeTargets();
	void FreeGroups();
	void HideGroupMarkers();

private:
	ABlobEntity*                    m_pOwner;                // 0x00
	TINT                            m_eState;                // 0x04
	Toshi::T2GUIMaterial*           m_pMaterial;             // 0x08 zjumpoffscreen.tga
	Toshi::T2GUIRectangle*          m_pRectangles;           // 0x0C [ NUM_TARGETS ]
	TUINT32                         m_uiUnk10;               // 0x10
	Toshi::TParticleEffect*         m_pScreenEffect;         // 0x14 Blob_Env_ZJump_Screen
	Toshi::TParticleEffectInstance* m_pScreenEffectInstance; // 0x18
	AZJumpData::Point*              m_pPoint;                // 0x1C the point the Z-jump started from
	AZJumpData::Node*               m_pCurrentNode;          // 0x20 the node Blob is at
	AZJumpData::Node*               m_pTargetNode;           // 0x24 the node Blob jumps to
	AZJumpData::Node*               m_pStandNode;            // 0x28
	AZJumpData::Node*               m_pUnk2C;                // 0x2C
	AZJumpData::Point*              m_pLastPoint;            // 0x30 the point just used (FindPoint skips it)
	TFLOAT                          m_fSearchTimer;          // 0x34 the time to the next target search
	TFLOAT                          m_fKeepTargetsTimer;     // 0x38 the targets are kept while it runs (Cancel sets 0.5)
	TFLOAT                          m_fHoldTime;             // 0x3C the hold time of state 3/4
	PriorityTarget*                 m_pTargets;              // 0x40 [ NUM_TARGETS ]
	PriorityGroup*                  m_pGroups;               // 0x44 [ NUM_GROUPS ]
	Toshi::T2DList<PriorityTarget>  m_lFreeTargets;          // 0x48
	Toshi::T2DList<PriorityTarget>  m_lTargets;              // 0x50
	Toshi::T2DList<PriorityGroup>   m_lGroups;               // 0x58
	PriorityTarget*                 m_pCurrentPriorityLock;  // 0x60
	TINT                            m_iUnk64;                // 0x64
	Toshi::TXUIControl*             m_pLockElement;          // 0x68 XUI control 0xE0 of the HUD (SetLockElement)
	TUINT32                         m_uiUnk6C;               // 0x6C
	Toshi::TXUIElement*             m_pLeftArrowColour;      // 0x70 "leftarrowcolour"
	Toshi::TXUIElement*             m_pRightArrowColour;     // 0x74 "rightarrowcolour"
	Toshi::TXUIElement*             m_pEnergyFill;           // 0x78 "EnergyFill" (a TXUIVisual)
	TFLOAT                          m_fEnergy1Point;         // 0x7C its "1Point" frame
	TFLOAT                          m_fEnergy100Point;       // 0x80 its "100Point" frame
	TFLOAT                          m_fEnergy200Point;       // 0x84 its "200Point" frame
	Toshi::TXUIElement*             m_pOutline;              // 0x88 "outline"
	Toshi::TXUIElement*             m_pCircleAbove;          // 0x8C "circleabove" of m_pEnergyFill
	Toshi::TXUIElement*             m_pGreyFill;             // 0x90 "GreyFill"
	Toshi::TXUIElement*             m_pCannotLock;           // 0x94 "cannotlock"
	Toshi::TXUIElement*             m_pCannotTarget;         // 0x98 "CannotTarget"
	Toshi::TXUIElement*             m_pPout;                 // 0x9C "pout"
	Toshi::TXUIElement*             m_pPassiveFill;          // 0xA0 "PassiveFill"
	Toshi::TXUIElement*             m_apPaintMeters[ 13 ];   // 0xA4 "PaintMeter1"-"PaintMeter13" of m_pEnergyFill
	Toshi::TXUIControl*             m_pScore;                // 0xD8 "Score" of m_pEnergyFill
	Toshi::TXUIControl*             m_pScoreWhite;           // 0xDC "ScoreWhite" of m_pEnergyFill
	TWCHAR                          m_wszScoreText[ 32 ];    // 0xE0 text of m_pScore/m_pScoreWhite
	TBOOL                           m_bUnk120;               // 0x120
	TBOOL                           m_bLockFound;            // 0x121 StartLock found a lock
	Toshi::TVector4                 m_vUnk124;               // 0x124
	Toshi::TVector4                 m_vUnk134;               // 0x134
	Toshi::TVector4                 m_vUnk144;               // 0x144
	TFLOAT                          m_fUnk154;               // 0x154
	TFLOAT                          m_fFlightTime;           // 0x158 OnBlobJump
	TFLOAT                          m_fLockElementOffsetX;   // 0x15C added to the lock element's X (single player)
	TUINT32                         m_aUnk160[ 2 ];          // 0x160
	TWCHAR                          m_wszLockText[ 8 ];      // 0x168 text of m_pLockElement
};

TSTATICASSERT( sizeof( ABlobZJumpComponent ) == 0x178 );
TSTATICASSERT( sizeof( ABlobZJumpComponent::PriorityTarget ) == 0x28 );
TSTATICASSERT( sizeof( ABlobZJumpComponent::PriorityGroup ) == 0x10 );
