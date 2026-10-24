#pragma once
#include "AEntityComponent2.h"
#include "AMoveHelper.h"
#include "Collision/ACollisionFunctor.h"
#include "PathFinding/AAIJumpTrajectory.h"

#include <Toshi/T2DList.h>
#include <Math/TVector4.h>
#include <Math/TMathInline.h>

namespace Toshi
{
struct TCollisionSurface;
class TPhysicsPhantomContainer;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: moves an AI character on the ground: velocity, facing, ground contact and the walk/jump/climb/fly goals of
// the pathfinding.
//-----------------------------------------------------------------------------
class AGndMotorComponent;
struct AIMoveGoal_t;

//-----------------------------------------------------------------------------
// Purpose: the functor of AGndMotorComponent::UpdateTouches: the touched non-solid objects are added to the touch list,
// the name of the last group hit is kept.
//-----------------------------------------------------------------------------
class EntityContactFunctor
    : public ACollisionFunctor
{
public:
	// $deBlob: FUNCTION 007dd350
	EntityContactFunctor( AMoveHelper::TouchList* a_pTouches )
	    : m_pTouches( a_pTouches )
	{
		m_szGroupName[ 0 ] = '\0';
	}

	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override;             // +0x10
	virtual void  HitGroup( const TCHAR* a_szGroupName, TUINT32 a_eGroupFlag ) override; // +0x14
	// $deBlob: FUNCTION 007dd460
	// $deBlob: FUNCTION 007dd3c0
	virtual ~EntityContactFunctor() override = default; // +0x20

public:
	AMoveHelper::TouchList* m_pTouches;          // 0x04
	TCHAR                   m_szGroupName[ 64 ]; // 0x08
};

// The node of AGndMotorManager's list (AGndMotorComponent's base already is an AEntityComponent2 list node)
class AGndMotorNode
    : public Toshi::T2DList<AGndMotorNode>::Node
{
public:
	AGndMotorComponent* GetMotor();
};

class AGndMotorComponent
    : public AEntityComponent2
{
public:
	friend class AGndMotorManager;

	// m_uiFlags
	enum FLAG : TUINT32
	{
		FLAG_CustomGravity    = 0x1,  // m_fGravity instead of sm_pData->Gravity
		FLAG_CustomTraction   = 0x2,  // m_vCustomTraction instead of sm_pData's
		FLAG_CustomFriction   = 0x4,  // m_vCustomFriction instead of sm_pData's
		FLAG_AtRest           = 0x20, // not moving: the tick skips the movement
		FLAG_ProbeGround      = 0x40, // AGndMotorManager::Update: probe the ground contact on the next tick
		FLAG_Unk100           = 0x100,
		FLAG_Unk400           = 0x400,
		FLAG_Unk80            = 0x80,
		FLAG_Held             = 0x200, // AEnemy::EndAtomState
		FLAG_Unk8             = 0x8,
		FLAG_OnWalkableGround = 0x10,   // UpdateGroundContact: on walkable ground, inside the triangle's edges
		FLAG_GroundCached     = 0x800,  // m_avGroundTriangle/m_oGroundContact hold the last ground
		FLAG_CanCacheGround   = 0x1000, // the ground may be cached
	};

	// The ground found by FindGroundContact (0x20 bytes)
	struct GroundContactInfo
	{
		// uiFlags
		enum FLAG : TUINT32
		{
			FLAG_HasTriangle = 0x1, // a mesh triangle was hit
			FLAG_InsideEdges = 0x2, // the entity is above the triangle, away from its edges
		};

		Toshi::TVector4   vNormal;    // 0x00
		TFLOAT            fDistance;  // 0x10
		ACollisionObject* pObject;    // 0x14
		TUINT32           eGroupFlag; // 0x18 TOSHICGROUPFLAG (the surface bits)
		TUINT32           uiFlags;    // 0x1C FLAG
	};

	// The functor of the ground probe (0x40 bytes)
	class GroundProbeCollideFunctor2
	    : public ACollisionFunctor
	{
	public:
		// $deBlob: FUNCTION 007dd380
		GroundProbeCollideFunctor2( AMoveHelper::TouchList* a_pTouches )
		    : m_pTouches( a_pTouches ), m_eGroupFlag( 0 ), m_bHasTriangle( TFALSE )
		{}

		virtual TBOOL PreFilterSweptAABBPoly( const Toshi::TVector4& a_rNormal, const Toshi::TVector4* a_pTriangle ) override; // +0x00
		virtual TBOOL PreFilterSweptSpherePoly( const Toshi::TVector4& a_rNormal ) override;                                   // +0x04
		virtual TBOOL PreFilterSpherePoly( const Toshi::TVector4& a_rNormal ) override;                                        // +0x08
		virtual TBOOL PreFilterAABBPoly( const Toshi::TVector4& a_rNormal ) override;                                          // +0x0C
		virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override;                                               // +0x10
		virtual void  HitGroup( const TCHAR* a_szGroupName, TUINT32 a_eGroupFlag ) override;                                   // +0x14
		// $deBlob: FUNCTION 007dd490
		// $deBlob: FUNCTION 007dd3d0
		virtual ~GroundProbeCollideFunctor2() override = default; // +0x20

	public:
		AMoveHelper::TouchList* m_pTouches;        // 0x04
		TUINT32                 m_eGroupFlag;      // 0x08
		TBOOL                   m_bHasTriangle;    // 0x0C
		Toshi::TVector4         m_avTriangle[ 3 ]; // 0x10
	};

	// GlobalData_AMovementComponentData_EntityData
	struct Data
	{
		TFLOAT           fUnk00;         // 0x00
		TFLOAT           Gravity;        // 0x04
		Toshi::TVector4* GroundTraction; // 0x08
		Toshi::TVector4* AirTraction;    // 0x0C
		Toshi::TVector4* GroundFriction; // 0x10
		Toshi::TVector4* AirFriction;    // 0x14
	};

public:
	AGndMotorComponent( ABaseEntity* a_pEntity );
	virtual ~AGndMotorComponent() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void ResetComponent() override;            // +0x08

	// Reads sm_pData from the entity data (AEntityFramework)
	static void InitialiseData();

	// Forgets sm_pData (AEntityFramework::Shutdown)
	// $deBlob: FUNCTION 007d9b70
	static void DeinitialiseData() { sm_pData = TNULL; }

	// $deBlob: FUNCTION 007da3f0
	TFLOAT GetGravity() const
	{
		if ( HASANYFLAG( m_uiFlags, FLAG_CustomGravity ) )
			return m_fGravity;

		return sm_pData->Gravity;
	}

	// $deBlob: FUNCTION 007da300
	TBOOL IsOnGround( TFLOAT a_fTime ) const { return ( m_iState == 2 || m_iState == 1 ) && a_fTime > m_fStateTime; }

	// $deBlob: FUNCTION 0078c6e0
	void SetFacing( const Toshi::TVector4& a_rvFacing )
	{
		TASSERT( a_rvFacing.IsNormalised() );
		m_vFacing = a_rvFacing;
	}

	// Sets m_fGravity and FLAG_CustomGravity
	void SetCustomGravity( TFLOAT a_fGravity )
	{
		m_uiFlags |= FLAG_CustomGravity;
		m_fGravity = a_fGravity;
	}

	// A launch: the velocity (and the facing with a_bFace) for a_fTime (0x244), a_fDrag (0x248)
	void SetVelocity( const Toshi::TVector4& a_rcVelocity, TFLOAT a_fTime, TFLOAT a_fDrag, TBOOL a_bFace );

	// Sets the velocity (w 1); a downward one ends the state (m_iState 0)
	void SetVelocity( const Toshi::TVector4& a_rcVelocity );
	// m_vAcceleration = a_rcValue
	void SetAcceleration( const Toshi::TVector4& a_rcValue );
	// Adds a_rcVelocity (x, y, z) to the velocity
	void AddVelocity( const Toshi::TVector4& a_rcVelocity );
	// UpdateCharacterMovement( a_fDeltaTime )
	void UpdateMovement( TFLOAT a_fDeltaTime );
	// Applies (a_iMode 1: takes back) the height offset to a_pPosition, or to the entity when it's TNULL, with the
	// collision flags of Tick; a_bApplyNext (not with mode 0) switches to m_fNextHeightOffset first
	void ApplyHeightOffset( TINT a_iMode, Toshi::TVector4* a_pPosition, TBOOL a_bApplyNext );
	// The point of the segment a_pStart-a_pEnd (x and y only) closest to a_pPoint in a_pOut; TFALSE if it's off the
	// segment or the segment is a point
	static TBOOL GetClosestPointOnSegment2D( const TFLOAT* a_pStart, const TFLOAT* a_pEnd, const TFLOAT* a_pPoint, TFLOAT* a_pOut );

	// Stops the AI move (the motor's velocity on the ground)
	void StopImmediate();

	// Adds the query phantoms (created with the entity's box) to the simulation or removes them
	void SetQueryHelpersActive( TBOOL a_bActive );

	// Forgets the cached ground if it is a_pObject (AGndMotorManager::OnCollisionObjectDestroyed)
	void OnCollisionObjectDestroyed( ACollisionObject* a_pObject );

	// Moves towards a_rcGoal with the move type (0: walk, 1: jump, 2: fly): 0 can't (no ground ahead), 3 arrived, else
	// m_iUnk270 (the result of the last movement update)
	TINT MoveToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime, TINT a_eMoveType );

	// The position with the height offset applied
	Toshi::TVector4 GetGroundedPosition() const;

	const Toshi::TVector4& GetGroundVelocity() const { return m_vGroundVelocity; }
	const Toshi::TVector4& GetFacing() const { return m_vFacing; }
	const Toshi::TVector4& GetDesiredFacing() const { return m_vDesiredFacing; }
	// $deBlob: FUNCTION 007e85b0
	TFLOAT                 GetCurrentSpeed() const { return m_fCurrentSpeed; }
	const Toshi::TVector4& GetVelocity() const { return m_vVelocity; }
	// $deBlob: FUNCTION 0059cff0
	TUINT32 GetFlags() const { return m_uiFlags; }
	void    AddFlags( TUINT32 a_uiFlags ) { m_uiFlags |= a_uiFlags; }
	void    ClearFlags( TUINT32 a_uiFlags ) { m_uiFlags &= ~a_uiFlags; }
	TINT    GetState() const { return m_iState; }
	// $deBlob: FUNCTION 0076c1e0
	const Toshi::TCollisionSurface* GetSurface() const { return m_pSurface; }

	// Sets the speed of the moves (0x21C) and raises the maximum (0x268) to it
	void SetSpeed( TFLOAT a_fSpeed )
	{
		m_fSpeed    = a_fSpeed;
		m_fMaxSpeed = Toshi::TMath::Max( m_fMaxSpeed, a_fSpeed );
	}

	// Is a_pObject something to stand on (terrain: a_rcNormal is zero or steep enough; an entity: visible and
	// not flagged 8/0x10/0x20)?
	static TBOOL IsStandableCollisionObject( ACollisionObject* a_pObject, const Toshi::TVector4& a_rcNormal );

	// Moves the entity by a_pEntity's reaction force for this frame
	void ReactToEntity( ABaseEntity* a_pEntity );

	// Puts the entity at a_rcPosition, facing the way it moved
	void MoveTo( const Toshi::TVector4& a_rcPosition );

private:
	TINT                   WalkToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime );
	TINT                   JumpToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime );
	TINT                   FlyToGoal( const AIMoveGoal_t& a_rcGoal, TFLOAT a_fDeltaTime );
	TBOOL                  FindGroundContact( TFLOAT a_fTolerance, GroundContactInfo* a_pContact );
	const Toshi::TVector4& GetTraction() const;
	const Toshi::TVector4& GetFriction() const;
	void                   UpdateTouches( TFLOAT a_fDeltaTime );
	void                   UpdateGroundContact( TFLOAT a_fDeltaTime );
	void                   UpdateCharacterMovement( TFLOAT a_fDeltaTime );
	void                   UpdateAIClimb( TFLOAT a_fDeltaTime );
	void                   UpdateAIFly( TFLOAT a_fDeltaTime );
	void                   ResolveAABBCollisions();
	void                   TryUnStick( TINT& a_riAttempt );

public:
	static Data* sm_pData;

	static TFLOAT sm_fMinGroundDot;
	// The steepest ground the probe accepts (cos 70)
	static const TFLOAT scm_fMaxSlidableGroundDot;

private:
	AGndMotorNode                    m_oMotorNode;            // 0x14 AGndMotorManager's list
	Toshi::TVector4                  m_vAcceleration;         // 0x1C
	Toshi::TVector4                  m_vVelocity;             // 0x2C
	Toshi::TVector4                  m_vGravityDir;           // 0x3C
	Toshi::TVector4                  m_vLastPosition;         // 0x4C
	Toshi::TVector4                  m_vFacing;               // 0x5C
	Toshi::TVector4                  m_vDesiredFacing;        // 0x6C
	Toshi::TVector4                  m_vCustomFriction;       // 0x7C FLAG_CustomFriction
	Toshi::TVector4                  m_vCustomTraction;       // 0x8C FLAG_CustomTraction
	Toshi::TVector4                  m_vGroundVelocity;       // 0x9C
	Toshi::TVector4                  m_vUnkAC;                // 0xAC
	Toshi::TVector4                  m_vUnkBC;                // 0xBC
	Toshi::TVector4                  m_avGroundTriangle[ 3 ]; // 0xCC the cached ground triangle (FLAG_GroundCached)
	GroundContactInfo                m_oGroundContact;        // 0xFC the cached ground contact (FLAG_GroundCached)
	AAIJumpTrajectory                m_JumpParabola;          // 0x11C
	AMoveHelper::TouchList           m_oTouchList;            // 0x1B4
	TFLOAT                           m_fSpeed;                // 0x21C
	TFLOAT                           m_fTerminalVelocity;     // 0x220 200.0f
	TFLOAT                           m_fAcceleration;         // 0x224 -1.0f
	TFLOAT                           m_fDeceleration;         // 0x228 -1.0f
	TINT                             m_iState;                // 0x22C
	TFLOAT                           m_fStateTime;            // 0x230
	TFLOAT                           m_fHeightOffset;         // 0x234
	TFLOAT                           m_fNextHeightOffset;     // 0x238 -1.0f: none
	TBOOL                            m_bHasReactionForce;     // 0x23C
	TFLOAT                           m_fCurrentSpeed;         // 0x240
	TFLOAT                           m_fLaunchTime;           // 0x244
	TFLOAT                           m_fLaunchDrag;           // 0x248
	TINT                             m_iMoveType;             // 0x24C 0: walk, 1, 2
	TFLOAT                           m_fMoveDeltaTime;        // 0x250 the delta time of the last MoveToGoal
	TINT                             m_iStuckCount;           // 0x254
	TFLOAT                           m_fGravity;              // 0x258 FLAG_CustomGravity
	TUINT32                          m_uiFlags;               // 0x25C FLAG
	TUINT32                          m_uiCollisionFlags;      // 0x260 the collision object's flags while m_fHeightOffset is applied
	TUINT32                          m_uiPathCollisionFlags;  // 0x264
	TFLOAT                           m_fMaxSpeed;             // 0x268
	const Toshi::TCollisionSurface*  m_pSurface;              // 0x26C the ground's surface
	TINT                             m_iUnk270;               // 0x270 3
	ABaseEntity*                     m_pReactionEntity;       // 0x274 an entity whose reaction force moves this one
	Toshi::TPhysicsPhantomContainer* m_apQueryHelpers[ 3 ];   // 0x278 the phantoms of the queries (SetQueryHelpersActive)
};

inline AGndMotorComponent* AGndMotorNode::GetMotor()
{
	return TREINTERPRETCAST( AGndMotorComponent*, TREINTERPRETCAST( TUINTPTR, this ) - 0x14 );
}
