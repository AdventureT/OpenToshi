#include "pch.h"
#include "AAIMoveProbe.h"
#include "APathfindingComponent.h"
#include "Entities/ABaseEntity.h"
#include "Collision/ACollisionManager2.h"
#include "Collision/ACollisionObject.h"
#include "AAIJumpTrajectory.h"

#include <Math/TCollision.h>
#include <Math/TVector2.h>
#include <Toshi/TString8.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/TBoxData.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The steepest walkable ground (cos 50)
static TCONSTEXPR_VAR TFLOAT s_fGroundDot = 0.64278764f;

static APathfindingComponent* GetPathfinding( ABaseEntity* a_pEntity )
{
	return TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() );
}

// A unit box (ACharEntity::CanOccupyPosition's)
static const TVector4 s_vUnitHalfExtents( 1.0f, 1.0f, 1.0f, 1.0f );

// NOTE: a read-only copy of TVector4::VEC_ZERO made at start up (also AIOCollisionComponent's)
static const TVector4 s_vZero = TVector4::VEC_ZERO;

//-----------------------------------------------------------------------------
// The collision functors of the probes (StandablePosCollide)
//-----------------------------------------------------------------------------

// TestGroundStep's box test: walls only (the ground isn't an obstacle), remembers the group hit
class StepFilter
    : public ACollisionFunctor
{
public:
	// $deBlob: FUNCTION 007d7bd0
	StepFilter() {}

	// $deBlob: FUNCTION 007d7c00
	// $deBlob: FUNCTION 007d7cd0
	virtual ~StepFilter() override {} // +0x20

	// $deBlob: FUNCTION 007d7e70
	virtual TBOOL PreFilterSweptAABBPoly( const TVector4& a_rNormal, const TVector4* a_pTriangle ) override // +0x00
	{
		return s_fGroundDot < -a_rNormal.y;
	}

	// NOTE: the base's HitNonSolidObject returns TFALSE
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10

	// $deBlob: FUNCTION 007d7d90
	virtual void HitGroup( const TCHAR* a_szGroupName, TUINT32 a_eGroupFlag ) override // +0x14
	{
		m_strGroupName = a_szGroupName;
	}

private:
	TString8 m_strGroupName; // 0x04
};

// IsStandablePosition's sweep: only ground triangles, the last one is kept
class StandablePosCollideFunctor2
    : public ACollisionFunctor
{
public:
	// $deBlob: FUNCTION 007d7bc0
	StandablePosCollideFunctor2() { m_bHasTriangle = TFALSE; }

	// $deBlob: FUNCTION 007d7bf0
	// $deBlob: FUNCTION 007d7ca0
	virtual ~StandablePosCollideFunctor2() override {} // +0x20

	// $deBlob: FUNCTION 007d7e20
	virtual TBOOL PreFilterSweptAABBPoly( const TVector4& a_rNormal, const TVector4* a_pTriangle ) override // +0x00
	{
		if ( -a_rNormal.y < s_fGroundDot )
			return TTRUE;

		if ( a_pTriangle != TNULL )
		{
			m_bHasTriangle    = TTRUE;
			m_avTriangle[ 0 ] = a_pTriangle[ 0 ];
			m_avTriangle[ 1 ] = a_pTriangle[ 1 ];
			m_avTriangle[ 2 ] = a_pTriangle[ 2 ];
		}

		return TFALSE;
	}

	// NOTE: the base's HitNonSolidObject returns TFALSE
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10

public:
	TBOOL    m_bHasTriangle;    // 0x04
	TVector4 m_avTriangle[ 3 ]; // 0x08
};

// ProbeGround's sweep: only ground polygons
class GroundProbeCollideFunctor
    : public ACollisionFunctor
{
public:
	// $deBlob: FUNCTION 007d7bb0
	GroundProbeCollideFunctor() {}

	// $deBlob: FUNCTION 007d7df0
	virtual TBOOL PreFilterSweptAABBPoly( const TVector4& a_rNormal, const TVector4* a_pTriangle ) override // +0x00
	{
		return -s_fGroundDot < a_rNormal.y;
	}

	// NOTE: the base's HitNonSolidObject returns TFALSE
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10
};

// NOTE: these three are free functions; retail's copies pop their arguments (RET n) and don't use ECX
static TBOOL     ProjectSegmentOntoTriangle( const TVector4& a_rvStart, const TVector4& a_rvEnd, const TVector4* a_pTriangle, const TVector4& a_rvNormal, TINT& a_riEdge, TFLOAT& a_rfDistance );
static TBOOL     ProbeClosestPointOnLine2D( const TVector2& a_rvStart, const TVector2& a_rvEnd, const TVector2& a_rvPoint, TVector2& a_rvClosest );
static TVector4& ClosetPointOnSegment( const TVector4& a_rvPoint, const TVector4& a_rvStart, const TVector4& a_rvEnd, TFLOAT& a_rfT, TVector4& a_rvClosest );

// $deBlob: FUNCTION 007d5f20
AAIMoveProbe::AAIMoveProbe( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_pMotor   = TNULL;
	m_pPhantom = TNULL;
}

// $deBlob: FUNCTION 007d5fe0
TINT AAIMoveProbe::CheckGroundMove( const TVector4& a_rvStart, TVector4& a_rvGoal, TUINT32 a_uiFlags )
{
	GetPathfinding( m_pEntity )->GetObstruction().Reset();

	TVector4 vMove( a_rvStart.x - a_rvGoal.x, a_rvStart.y - a_rvGoal.y, a_rvStart.z - a_rvGoal.z, a_rvStart.w );

	if ( vMove.MagnitudeSqXZ() <= 0.0f )
		return PROBERESULT_Clear;

	TFLOAT fStepHeight = m_pEntity->GetStepUpHeight();

	return SteppedGroundTest( a_rvStart, a_rvGoal, m_pEntity->GetBBox().z * 2.0f, fStepHeight, fStepHeight, a_uiFlags );
}

// $deBlob: FUNCTION 007d63a0
void AAIMoveProbe::ResetComponent()
{
	m_pMotor = GetPathfinding( m_pEntity )->GetMotor();
}

// $deBlob: FUNCTION 007d6830
void AAIMoveProbe::Init()
{
}

// $deBlob: FUNCTION 007d6840
void AAIMoveProbe::Destroy()
{
}

// $deBlob: FUNCTION 007d6850
TINT AAIMoveProbe::SteppedGroundTest( const TVector4& a_rvStart, TVector4& a_rvGoal, TFLOAT a_fUnk, TFLOAT a_fStepHeight, TFLOAT a_fUnk2, TUINT32 a_uiFlags )
{
	GetPathfinding( m_pEntity )->GetObstruction().Reset();

	ACollisionManager2* pCollisionManager = ACollisionManager2::GetSingleton();
	const TVector4&     rcBBox            = m_pEntity->GetBBox();

	// The box is a little bigger for the edge tests
	TVector4 vExtents( rcBBox.x + 0.05f, rcBBox.y + 0.05f, rcBBox.z + 0.05f, rcBBox.w );

	TFLOAT fHalfHeight = rcBBox.y;
	TFLOAT fStartY     = a_rvStart.y - fHalfHeight;

	TVector4 vDirection( a_rvGoal.x - a_rvStart.x, ( a_rvGoal.y - fHalfHeight ) - fStartY, a_rvGoal.z - a_rvStart.z, a_rvGoal.w );
	TFLOAT   fRemaining = vDirection.NormaliseXZ();

	if ( fRemaining <= TMath::TFLOAT_EPSILON )
		return PROBERESULT_Clear;

	TVector4 vSide;
	vSide.CrossProduct( vDirection, TVector4::VEC_NEGY );

	TUINT32 uiLayer = 0x87;

	if ( a_uiFlags & 8 )
		uiLayer = 6;

	if ( a_uiFlags & 2 )
		uiLayer &= ~4;

	if ( a_uiFlags & 0x20 )
		uiLayer &= ~4;

	if ( a_uiFlags & 4 )
		uiLayer &= ~2;

	if ( m_pPhantom != TNULL )
		TPhysicsManager::GetSingleton()->SetPhantomCollisionFilterInfo( m_pPhantom, uiLayer * 0x1000 );

	GroundPolyFunctor oFunctor;
	TVector4          vPosition( a_rvStart.x, fStartY, a_rvStart.z, a_rvStart.w );
	TVector4          vNormal;
	TFLOAT            fStep;
	TBOOL             bMore      = TFALSE;
	TINT              iStepsLeft = 10;

	while ( TTRUE )
	{
		if ( --iStepsLeft <= 0 )
			return PROBERESULT_NoGround;

		// The ground under the box
		TFLOAT fDistance = a_fStepHeight;
		oFunctor.Reset();

		if ( pCollisionManager->CollideSweptAABB( vPosition, rcBBox, TVector4::VEC_POSY, fDistance, vNormal, 0.0f, fStep, m_pEntity->GetCollisionObject(), &oFunctor, uiLayer, m_pPhantom ) == TNULL || vNormal.y > s_fGroundDot || !oFunctor.m_bHasTriangle )
		{
			if ( !oFunctor.m_bNonMesh || vNormal.y >= s_fGroundDot )
				return PROBERESULT_NoGround;

			return PROBERESULT_Clear;
		}

		vPosition.y = ( vPosition.y + fDistance ) - 0.1f;

		// Along the slope of the ground
		TVector4 vSlope;
		vSlope.CrossProduct( vNormal, vSide );
		vSlope.Normalise();

		// Where the box leaves the triangle: the box comes back from 50 ahead against the triangle's edges
		TVector4 vAhead;
		vAhead.Multiply( vSlope, 50.0f );

		TVector4 vFar( vAhead.x + vPosition.x, vAhead.y + vPosition.y + fHalfHeight, vAhead.z + vPosition.z, vPosition.w );
		TVector4 vBack( -vSlope.x, -vSlope.y, -vSlope.z, vSlope.w );

		TINT   iNumEdges    = 0;
		TFLOAT fMaxBackDist = 0.0f;

		for ( TINT i = 0; i < 3; i++ )
		{
			const TVector4& rcVertex = oFunctor.m_avTriangle[ i ];
			const TVector4& rcNext   = oFunctor.m_avTriangle[ ( i + 1 ) % 3 ];

			TVector4 vEdge( rcNext.x - rcVertex.x, rcNext.y - rcVertex.y, rcNext.z - rcVertex.z, rcNext.w );
			TVector4 vEdgeNormal;
			vEdgeNormal.CrossProduct( vEdge, vNormal );
			vEdgeNormal.Normalise();

			TPlane oPlane( vEdgeNormal.x, vEdgeNormal.y, vEdgeNormal.z, rcVertex.x * vEdgeNormal.x + rcVertex.y * vEdgeNormal.y + rcVertex.z * vEdgeNormal.z );
			TFLOAT fBackDist = 50.0f;

			if ( TCollision::AABBSweptvsPlane( vFar, vExtents, vBack, oPlane, fBackDist ) == TCollision::AABBSWEPT_HIT )
			{
				iNumEdges++;

				if ( fBackDist > fMaxBackDist )
					fMaxBackDist = fBackDist;
			}
		}

		TVector4 vBackMove;
		vBackMove.Multiply( vBack, fMaxBackDist );

		TVector4 vExit( vBackMove.x + vFar.x, vBackMove.y + vFar.y, vBackMove.z + vFar.z, vFar.w );
		TFLOAT   fStepDist = TVector4::DistanceXZ( vExit, vPosition );

		if ( iNumEdges > 1 )
		{
			// The vertex of the triangle closest to the exit, if the exit is past all of them
			TBOOL           bBehind  = TFALSE;
			TFLOAT          fClosest = 100.0f;
			const TVector4* pClosest = TNULL;

			for ( TINT i = 0; i < 3; i++ )
			{
				const TVector4& rcVertex = oFunctor.m_avTriangle[ i ];

				if ( ( vExit.y - rcVertex.y ) * vSlope.y + ( vExit.x - rcVertex.x ) * vSlope.x + ( vExit.z - rcVertex.z ) * vSlope.z < 0.0f )
				{
					bBehind = TTRUE;
					break;
				}

				TFLOAT fVertexDist = TVector4::Distance( rcVertex, vExit );

				if ( fVertexDist < fClosest )
				{
					fClosest = fVertexDist;
					pClosest = &rcVertex;
				}
			}

			TFLOAT fRayDist = 50.0f;

			if ( !bBehind && TMath::RayVsAABB( *pClosest, vSlope, vExit, rcBBox, fRayDist ) )
				fStepDist = ( fStepDist - fRayDist ) + 0.05f;
			else
				fStepDist = fStepDist + 0.05f;
		}

		if ( fStepDist <= fRemaining )
		{
			bMore = TTRUE;
		}
		else
		{
			fStepDist = fRemaining;
			bMore     = TFALSE;
		}

		fStepDist = TMath::Max( fStepDist, 0.01f );

		// The step along the slope
		ACollisionObject* pHit = pCollisionManager->CollideSweptAABB( vPosition, rcBBox, vSlope, fStepDist, vNormal, a_fStepHeight, fStep, m_pEntity->GetCollisionObject(), TNULL, uiLayer, m_pPhantom );

		fRemaining -= fStepDist;

		TVector4 vMove;
		vMove.Multiply( vSlope, fStepDist );
		vPosition.x += vMove.x;
		vPosition.y += vMove.y;
		vPosition.z += vMove.z;

		if ( pHit != TNULL )
		{
			if ( fStep < 0.0f )
			{
				if ( vNormal.y > -s_fGroundDot )
				{
					// A wall (or an object) that can't be stepped onto
					APathfindingComponent::Obstruction& rObstruction = GetPathfinding( m_pEntity )->GetObstruction();
					rObstruction.pObject                             = pHit;
					rObstruction.vNormal                             = vNormal;

					TUINT8 uiObjectFlags = TUINT8( pHit->GetFlags() );

					if ( uiObjectFlags & 0x20 )
						return PROBERESULT_PushableObject;

					return 3 - ( ( uiObjectFlags & 0x30 ) != 0 );
				}

				if ( fStepDist < TMath::TFLOAT_EPSILON )
					vPosition.y -= 0.05f;
			}
			else
			{
				// Up the step
				vPosition.y -= fStep;

				TVector4 vNudge;
				vNudge.Multiply( vSlope, 0.05f );
				vPosition.x += vNudge.x;
				vPosition.y += vNudge.y;
				vPosition.z += vNudge.z;
				fRemaining -= 0.05f;
			}
		}

		if ( fRemaining <= TMath::TFLOAT_EPSILON )
			break;
	}

	if ( !bMore )
		return PROBERESULT_Clear;

	// The ground at the end
	TFLOAT fDistance = a_fStepHeight;
	oFunctor.Reset();

	if ( pCollisionManager->CollideSweptAABB( vPosition, rcBBox, TVector4::VEC_POSY, fDistance, vNormal, 0.0f, fStep, m_pEntity->GetCollisionObject(), &oFunctor, uiLayer, m_pPhantom ) != TNULL && vNormal.y <= s_fGroundDot && oFunctor.m_bHasTriangle )
		return PROBERESULT_Clear;

	(void)a_fUnk;
	(void)a_fUnk2;
	return PROBERESULT_NoGround;
}

// $deBlob: FUNCTION 007d5f60
// $deBlob: FUNCTION 007d7c10
AAIMoveProbe::~AAIMoveProbe()
{
	if ( m_pPhantom != TNULL )
	{
		TPhysicsManager::GetSingleton()->DestroyPhantom( m_pPhantom );
		m_pPhantom = TNULL;
	}
}

// $deBlob: FUNCTION 007d7d00
void AAIMoveProbe::GroundPolyFunctor::AABBSweptHitPoly( const TVector4* a_pTriangle, TFLOAT a_fDistance )
{
	if ( m_fDistance <= a_fDistance )
		return;

	m_fDistance = a_fDistance;

	if ( a_pTriangle != TNULL )
	{
		m_avTriangle[ 0 ] = a_pTriangle[ 0 ];
		m_avTriangle[ 1 ] = a_pTriangle[ 1 ];
		m_avTriangle[ 2 ] = a_pTriangle[ 2 ];
		m_bHasTriangle    = TTRUE;
		m_bNonMesh        = TFALSE;
	}
	else
	{
		m_avTriangle[ 0 ] = TVector4::VEC_ZERO;
		m_avTriangle[ 1 ] = TVector4::VEC_ZERO;
		m_avTriangle[ 2 ] = TVector4::VEC_ZERO;
		m_bHasTriangle    = TFALSE;
		m_bNonMesh        = TTRUE;
	}
}

// $deBlob: FUNCTION 007d7dc0
TBOOL AAIMoveProbe::GroundPolyFunctor::PreFilterSweptAABBPoly( const TVector4& a_rNormal, const TVector4* a_pTriangle )
{
	if ( a_rNormal.y > -s_fGroundDot )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007d66d0
void AAIMoveProbe::SetPhantomActive( TBOOL a_bActive )
{
	if ( m_pPhantom == TNULL )
	{
		if ( !a_bActive )
			return;

		// The phantom has the owner's box
		TBoxData oBox;
		oBox.SetHalfExtents( m_pEntity->GetBBox() );
		m_pEntity->GetBasePosition();

		m_pPhantom = TPhysicsManager::GetSingleton()->CreateShapePhantom( &oBox );
	}

	if ( a_bActive )
	{
		if ( m_pPhantom != TNULL )
			TPhysicsManager::GetSingleton()->AddPhantomToSimulation( m_pPhantom, m_pEntity->GetBasePosition(), TNULL );

		return;
	}

	if ( m_pPhantom != TNULL )
		TPhysicsManager::GetSingleton()->RemovePhantomFromSimulation( m_pPhantom, TNULL );
}

// $deBlob: FUNCTION 007d6320
TINT AAIMoveProbe::CheckClimbMove( const TVector4& a_rvStart, const TVector4& a_rvGoal, TUINT32 a_uiFlags )
{
	GetPathfinding( m_pEntity )->GetObstruction().Reset();

	// NOTE: the obstruction was just reset, so this is always PROBERESULT_NoGround (climbing never works)
	APathfindingComponent::Obstruction& rObstruction = GetPathfinding( m_pEntity )->GetObstruction();

	if ( rObstruction.pObject == TNULL )
		return PROBERESULT_NoGround;

	TUINT8 uiObjectFlags = TSTATICCAST( ACollisionObject, rObstruction.pObject )->GetFlags();

	if ( uiObjectFlags & 0x30 )
		return PROBERESULT_PushableObject;

	TASSERT( uiObjectFlags & 0x08 );
	return PROBERESULT_Wall;
}

// $deBlob: FUNCTION 007d60b0
TINT AAIMoveProbe::CheckJumpMove( const TVector4& a_rvStart, const TVector4& a_rvGoal, TUINT32 a_uiFlags )
{
	GetPathfinding( m_pEntity )->GetObstruction().Reset();

	AAIJumpTrajectory oTrajectory;

	APathfindingComponent::Obstruction& rObstruction = GetPathfinding( m_pEntity )->GetObstruction();

	TBOOL bValid = oTrajectory.CheckValidJump( m_pEntity, a_rvStart, a_rvGoal, a_uiFlags, &rObstruction, AAIJumpTrajectory::JumpType_Auto, TTRUE, TFALSE );

	TINT eResult;

	if ( bValid )
	{
		eResult = PROBERESULT_Clear;
	}
	else if ( rObstruction.pObject == TNULL )
	{
		eResult = PROBERESULT_NoGround;
	}
	else
	{
		TUINT8 uiObjectFlags = TSTATICCAST( ACollisionObject, rObstruction.pObject )->GetFlags();

		if ( ( uiObjectFlags & 0x30 ) == 0 )
		{
			TASSERT( uiObjectFlags & 0x08 );
			eResult = PROBERESULT_Wall;
		}
		else
		{
			eResult = PROBERESULT_PushableObject;
		}
	}

	return eResult;
}

// $deBlob: FUNCTION 007d61d0
TINT AAIMoveProbe::CheckFlyMove( const TVector4& a_rvStart, const TVector4& a_rvGoal, TUINT32 a_uiFlags )
{
	GetPathfinding( m_pEntity )->GetObstruction().Reset();

	TVector4 vDirection( a_rvGoal.x - a_rvStart.x, a_rvGoal.y - a_rvStart.y, a_rvGoal.z - a_rvStart.z, a_rvGoal.w );
	TFLOAT   fDistance = vDirection.Magnitude();
	vDirection.Divide( fDistance );

	TUINT32  uiLayer = ( ~( a_uiFlags * 2 ) & 4 ) | 0x83;
	TVector4 vNormal;
	TFLOAT   fStep;

	// NOTE: retail sweeps from the owner's position (its matrix), not from a_rvStart
	ACollisionObject* pHit = ACollisionManager2::GetSingleton()->CollideSweptAABB( m_pEntity->GetTranslation(), m_pEntity->GetBBox(), vDirection, fDistance, vNormal, 0.0f, fStep, m_pEntity->GetCollisionObject(), TNULL, uiLayer, TNULL );

	if ( pHit == TNULL )
		return PROBERESULT_Clear;

	APathfindingComponent::Obstruction& rObstruction = GetPathfinding( m_pEntity )->GetObstruction();
	rObstruction.pObject                             = pHit;
	rObstruction.vNormal                             = vNormal;

	TUINT8 uiObjectFlags = pHit->GetFlags();

	if ( uiObjectFlags & 0x08 )
		return 0;

	if ( uiObjectFlags & 0x20 )
		return PROBERESULT_PushableObject;

	TASSERT( uiObjectFlags & 0x30 );
	return PROBERESULT_Object;
}

// $deBlob: FUNCTION 007d63c0
TBOOL AAIMoveProbe::IsStandablePosition( const TVector4& a_rvPosition, TFLOAT a_fProbeDist, TINT a_iUnk, TFLOAT a_fUnk )
{
	TASSERT( a_fProbeDist > 0 );

	TFLOAT                      fDistance = a_fProbeDist;
	StandablePosCollideFunctor2 oFunctor;
	TVector4                    vNormal = TVector4::VEC_ZERO;

	// NOTE: retail gets the collision object twice without using it
	m_pEntity->GetCollisionObject();
	TVector4 vHalfExtents = s_vUnitHalfExtents;
	m_pEntity->GetCollisionObject();

	// NOTE: the sweep starts from the zero vector, not from a_rvPosition
	TVector4 vStart = s_vZero;
	vStart.y        = ( s_vZero.y + vHalfExtents.y ) - 0.2f;
	vHalfExtents.y  = 0.2f;

	TFLOAT            fStep;
	ACollisionObject* pHit = ACollisionManager2::GetSingleton()->CollideSweptAABB( vStart, vHalfExtents, TVector4::VEC_POSY, fDistance, vNormal, 0.0f, fStep, m_pEntity->GetCollisionObject(), &oFunctor, 135, TNULL );

	if ( pHit == TNULL )
		return TFALSE;

	TASSERT( vNormal.y <= -s_fGroundDot );

	if ( !oFunctor.m_bHasTriangle )
		return TFALSE;

	// The owner has to be inside the triangle, 0.2 or more from its edges (XZ)
	const TVector4& rcPosition = m_pEntity->GetTranslation();

	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4& rcA = oFunctor.m_avTriangle[ i ];
		const TVector4& rcB = oFunctor.m_avTriangle[ ( i + 1 ) % 3 ];

		TVector2 vClosest;
		if ( !ProbeClosestPointOnLine2D( TVector2( rcA.x, rcA.z ), TVector2( rcB.x, rcB.z ), TVector2( rcPosition.x, rcPosition.z ), vClosest ) )
			return TFALSE;

		TFLOAT fX = rcPosition.x - vClosest.x;
		TFLOAT fZ = rcPosition.z - vClosest.y;

		if ( fZ * fZ + fX * fX < 0.2f * 0.2f )
			return TFALSE;
	}

	(void)a_rvPosition;
	(void)a_iUnk;
	(void)a_fUnk;
	return TTRUE;
}

// $deBlob: FUNCTION 007d71d0
TINT AAIMoveProbe::TestGroundStep( const TVector4& a_rvStart, const TVector4& a_rvDirection, TVector4& a_rvPosition, TFLOAT a_fUnk, TFLOAT a_fStepHeight, TUINT32& a_ruiFlags, void* a_pCollisionCache )
{
	// Onto the ground first: at a_rvPosition or halfway there
	if ( ( a_ruiFlags & 2 ) == 0 )
	{
		TVector4 vProbe     = a_rvPosition;
		TFLOAT   fProbeDist = a_fStepHeight + 0.05f;

		if ( !ProbeGround( fProbeDist, vProbe, m_pEntity->GetBBox(), a_pCollisionCache, TFALSE ) )
		{
			TVector4 vHalf( a_rvPosition.x - a_rvStart.x, a_rvPosition.y - a_rvStart.y, a_rvPosition.z - a_rvStart.z, a_rvPosition.w );
			vHalf.Multiply( 0.5f );
			vProbe.Set( vHalf.x + a_rvStart.x, a_rvStart.y + vHalf.y, a_rvStart.z + vHalf.z, a_rvStart.w );

			fProbeDist = a_fStepHeight + 0.05f;

			if ( !ProbeGround( fProbeDist, vProbe, m_pEntity->GetBBox(), a_pCollisionCache, TFALSE ) )
				return -1;
		}

		vProbe.y     = ( fProbeDist - 0.05f ) + vProbe.y;
		a_rvPosition = vProbe;
	}

	StepFilter      oFilter;
	const TVector4& rcHalfExtents = m_pEntity->GetBBox();
	TUINT32         uiLayer       = ( a_ruiFlags & 1 ) ? 131 : 135;

	TVector4          vNormal;
	ACollisionObject* pHit = ACollisionManager2::CollideAABB( a_rvPosition, rcHalfExtents, vNormal, TNULL, &oFilter, uiLayer );

	if ( a_ruiFlags & 8 )
	{
		if ( a_ruiFlags & 0x10 )
		{
			a_ruiFlags |= 0x20;
			pHit = ACollisionManager2::CollideAABB( a_rvPosition, rcHalfExtents, vNormal, TNULL, &oFilter, uiLayer );
		}
	}
	else
	{
		a_ruiFlags &= ~0x20;
	}

	a_ruiFlags &= ~0x10;

	if ( pHit != TNULL )
	{
		TFLOAT fStepUp = a_fStepHeight;

		if ( -s_fGroundDot < vNormal.y )
		{
			a_rvPosition.y = a_rvPosition.y - a_fStepHeight;
		}
		else
		{
			// Up the slope
			TVector4 vSide;
			vSide.CrossProduct( a_rvDirection, TVector4::VEC_NEGY );

			TVector4 vSlope;
			vSlope.CrossProduct( vNormal, vSide );
			vSlope.Normalise();

			TASSERT( a_rvDirection.y * vSlope.y + a_rvDirection.x * vSlope.x + a_rvDirection.z * vSlope.z >= 0 );

			TFLOAT fOldY     = a_rvPosition.y;
			TFLOAT fDistance = TVector4::Distance( a_rvStart, a_rvPosition );

			TVector4 vMove;
			vMove.Multiply( vSlope, fDistance );
			a_rvPosition.Set( vMove.x + a_rvStart.x, a_rvStart.y + vMove.y, a_rvStart.z + vMove.z, a_rvStart.w );

			TFLOAT fRise = ( fOldY - a_rvPosition.y ) + 0.05f;
			fStepUp      = TMath::Max( fRise, 0.0f );
		}

		TVector4 vBlockNormal;
		pHit = ACollisionManager2::CollideAABB( a_rvPosition, rcHalfExtents, vBlockNormal, TNULL, &oFilter, uiLayer );

		if ( pHit != TNULL )
		{
			APathfindingComponent::Obstruction& rObstruction = GetPathfinding( m_pEntity )->GetObstruction();
			rObstruction.pObject                             = pHit;
			rObstruction.vNormal                             = vBlockNormal;
			return -3;
		}

		TASSERT( fStepUp >= 0 );

		TFLOAT fProbeDist = fStepUp * 2.0f + 0.05f;

		if ( ProbeGround( fProbeDist, a_rvPosition, m_pEntity->GetBBox(), a_pCollisionCache, TFALSE ) )
		{
			a_ruiFlags &= ~2;
			a_rvPosition.y = fProbeDist + a_rvPosition.y;
			a_rvPosition.y = a_rvPosition.y - 0.05f;
		}
		else if ( ( a_ruiFlags & 2 ) == 0 )
		{
			return -2;
		}
	}

	(void)a_fUnk;
	return 1;
}

// $deBlob: FUNCTION 007d7630
TINT AAIMoveProbe::ProbeResult( ACollisionObject* a_pObject )
{
	TUINT8 uiFlags = a_pObject->GetFlags();

	if ( uiFlags & 0x08 )
		return 0;

	if ( uiFlags & 0x20 )
		return PROBERESULT_PushableObject;

	return ( uiFlags & 0x30 ) ? PROBERESULT_Object : 0;
}

// $deBlob: FUNCTION 007d7660
TBOOL AAIMoveProbe::ProbeGround( TFLOAT& a_rfProbeDst, const TVector4& a_rvPosition, const TVector4& a_rvHalfExtents, void* a_pCollisionCache, TBOOL a_bUnk )
{
	TASSERT( a_rfProbeDst > 0 );

	GroundProbeCollideFunctor oFunctor;

	if ( a_pCollisionCache == TNULL )
		return TFALSE;

	TVector4 vNormal;
	TFLOAT   fStep;

	(void)a_bUnk;
	return ACollisionManager2::GetSingleton()->CollideSweptAABB( a_rvPosition, a_rvHalfExtents, TVector4::VEC_POSY, a_rfProbeDst, vNormal, 0.0f, fStep, TNULL, &oFunctor, 135, TNULL ) != TNULL;
}

// $deBlob: FUNCTION 007d7720
static TBOOL ProjectSegmentOntoTriangle( const TVector4& a_rvStart, const TVector4& a_rvEnd, const TVector4* a_pTriangle, const TVector4& a_rvNormal, TINT& a_riEdge, TFLOAT& a_rfDistance )
{
	TVector4 vEdgeNormal = TVector4::VEC_ZERO;
	TVector4 vSegment( a_rvEnd.x - a_rvStart.x, a_rvEnd.y - a_rvStart.y, a_rvEnd.z - a_rvStart.z, a_rvEnd.w );

	a_rfDistance = TMath::FLOATEPSILON;
	TFLOAT fBest = 2.0f;

	for ( TINT i = 0; i < 3; i++ )
	{
		const TVector4& rcA = a_pTriangle[ i ];
		const TVector4& rcB = a_pTriangle[ ( i + 1 ) % 3 ];

		TVector4 vEdge( rcB.x - rcA.x, rcB.y - rcA.y, rcB.z - rcA.z, rcB.w );
		vEdgeNormal.CrossProduct( vEdge, a_rvNormal );

		// The start has to be inside every edge
		if ( vEdgeNormal.y * ( a_rvStart.y - rcA.y ) + vEdgeNormal.x * ( a_rvStart.x - rcA.x ) + vEdgeNormal.z * ( a_rvStart.z - rcA.z ) > 0.0f )
			return TFALSE;

		if ( vEdgeNormal.y * vSegment.y + vEdgeNormal.x * vSegment.x + vEdgeNormal.z * vSegment.z > 0.0f )
		{
			vEdgeNormal.Normalise();

			TFLOAT fT = ( ( vEdgeNormal.x * rcA.x + vEdgeNormal.y * rcA.y + rcA.z * vEdgeNormal.z ) - ( a_rvStart.y * vEdgeNormal.y + a_rvStart.x * vEdgeNormal.x + a_rvStart.z * vEdgeNormal.z ) ) /
			    ( vEdgeNormal.y * vSegment.y + vEdgeNormal.x * vSegment.x + vEdgeNormal.z * vSegment.z );

			if ( fT >= 0.0f && fT <= 1.0f && fT < fBest )
			{
				a_riEdge = i;
				fBest    = fT;
			}
		}
	}

	if ( fBest > 1.0f )
		a_rfDistance = vSegment.Magnitude();
	else
		a_rfDistance = vSegment.Magnitude() * fBest;

	return TTRUE;
}

// $deBlob: FUNCTION 007d7970
static TBOOL ProbeClosestPointOnLine2D( const TVector2& a_rvStart, const TVector2& a_rvEnd, const TVector2& a_rvPoint, TVector2& a_rvClosest )
{
	TFLOAT fX = a_rvEnd.x - a_rvStart.x;
	TFLOAT fY = a_rvEnd.y - a_rvStart.y;

	if ( fY * fY + fX * fX <= TMath::FLOATEPSILON )
		return TFALSE;

	TFLOAT fDot = fX * ( a_rvPoint.x - a_rvStart.x ) + fY * ( a_rvPoint.y - a_rvStart.y );

	if ( fDot < 0.0f )
		return TFALSE;

	TFLOAT fLength        = TMath::Sqrt( fY * fY + fX * fX );
	TFLOAT fOneOverLength = 1.0f / fLength;
	TFLOAT fT             = fOneOverLength * fDot;

	if ( fT < 0.0f || fLength < fT )
		return TFALSE;

	a_rvClosest.x = fX * fOneOverLength * fT + a_rvStart.x;
	a_rvClosest.y = fY * fOneOverLength * fT + a_rvStart.y;
	return TTRUE;
}

// $deBlob: FUNCTION 007d7a80
static TVector4& ClosetPointOnSegment( const TVector4& a_rvPoint, const TVector4& a_rvStart, const TVector4& a_rvEnd, TFLOAT& a_rfT, TVector4& a_rvClosest )
{
	TVector4 vSegment( a_rvEnd.x - a_rvStart.x, a_rvEnd.y - a_rvStart.y, a_rvEnd.z - a_rvStart.z, a_rvEnd.w );

	a_rfT = ( ( a_rvPoint.y - a_rvStart.y ) * vSegment.y + ( a_rvPoint.x - a_rvStart.x ) * vSegment.x + ( a_rvPoint.z - a_rvStart.z ) * vSegment.z ) /
	    ( vSegment.y * vSegment.y + vSegment.x * vSegment.x + vSegment.z * vSegment.z );

	if ( a_rfT >= 0.0f )
	{
		if ( a_rfT > 1.0f )
			a_rfT = 1.0f;
	}
	else
	{
		a_rfT = 0.0f;
	}

	TVector4 vOffset;
	vOffset.Multiply( vSegment, a_rfT );

	a_rvClosest.Set( a_rvStart.x + vOffset.x, a_rvStart.y + vOffset.y, a_rvStart.z + vOffset.z, a_rvStart.w );
	return a_rvClosest;
}
