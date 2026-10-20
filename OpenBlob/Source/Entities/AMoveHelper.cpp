#include "pch.h"
#include "AMoveHelper.h"
#include "Console/AStaticCVars.h"
#include "ABaseEntity.h"
#include "AGndMotorComponent.h"
#include "PathFinding/APathfindingComponent.h"
#include "Console/AConsoleVar.h"
#include "Collision/ACollisionManager2.h"

#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The collision layer of the sweeps
static TCONSTEXPR_VAR TUINT32 MOVE_COLLISION_LAYER = 0x87;

// $deBlob: FUNCTION 007e9790
TVector4 AMoveHelper::PerformClampedMove( ABaseEntity* a_pEntity, const TVector4& a_rcMove, TVector4& a_rvOutPosition, TouchList* a_pTouches, TBOOL a_bProject, const TVector4& a_rcProjectNormal, ACollisionObject* a_pIgnore, TBOOL a_bNoCollision, void* a_pPhantom )
{
	AGndMotorComponent* pMotor         = TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() )->GetMotor();
	TINT                iNumTouchesOld = ( a_pTouches != TNULL ) ? a_pTouches->Size() : 0;

	TVector4 vStart = a_pEntity->GetBasePosition();
	TVector4 vEnd( vStart.x + a_rcMove.x, vStart.y + a_rcMove.y, vStart.z + a_rcMove.z, vStart.w );
	TVector4 vDirection( vEnd.x - vStart.x, vEnd.y - vStart.y, vEnd.z - vStart.z, vStart.w );
	TFLOAT   fDistance = vDirection.NormaliseMag();

	if ( fDistance <= 0.0f )
	{
		a_rvOutPosition = vStart;
		a_rvOutPosition.y += a_pEntity->GetBBox().y;
		return a_rcMove;
	}

	TFLOAT fStepUpHeight = a_pEntity->GetStepUpHeight();
	TFLOAT fStepHeight   = pMotor->IsOnGround( 0.03f ) ? fStepUpHeight : 0.0f;
	TFLOAT fStep         = 0.0f;

	if ( !a_bNoCollision )
	{
		TUINT32 uiLayer = MOVE_COLLISION_LAYER;

		if ( a_pEntity->GetPathfinding() != TNULL )
		{
			TINT iPathFlags = TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() )->GetCollisionFlags();
			uiLayer         = MOVE_COLLISION_LAYER;

			if ( iPathFlags & 2 )
				uiLayer = 0x83;

			if ( TUINT8( iPathFlags >> 5 ) & 1 )
				uiLayer &= ~4;

			if ( iPathFlags & 8 )
				uiLayer &= ~0x81;
		}

		MoveHelperFunctor oFunctor( a_pTouches, a_pIgnore );

		if ( a_pPhantom != TNULL )
			TPhysicsManager::SetCollisionTestFlags( TSTATICCAST( TPhysicsPhantomContainer, a_pPhantom ), uiLayer );

		TVector4          vNormal;
		ACollisionObject* pHit = ACollisionManager2::GetSingleton()->CollideSweptAABB( vStart, a_pEntity->GetBBox(), vDirection, fDistance, vNormal, fStepHeight, fStep, a_pEntity->GetCollisionObject(), &oFunctor, uiLayer, a_pPhantom );

		if ( pHit != TNULL )
		{
			TVector4 vMove = a_rcMove;

			// A low obstacle in front: step up onto it
			if ( fStep >= 0.0f && -vNormal.y < 0.64278764f && -0.8f > vNormal.x * vDirection.x + vNormal.y * vDirection.y + vNormal.z * vDirection.z )
			{
				TFLOAT fOldStep = fStep;
				fStep           = fStep + 0.095f;
				vStart.y        = vStart.y - ( fOldStep + 0.095f );
			}

			if ( SlideMove( a_pEntity, vStart, vMove, vEnd, a_pTouches, a_bProject, a_rcProjectNormal, a_pIgnore, a_pPhantom ) )
			{
				a_rvOutPosition = vEnd;
				a_rvOutPosition.y += a_pEntity->GetBBox().y;
				return vMove;
			}

			// Couldn't move: stays where it is and forgets the touches of this move
			a_rvOutPosition = a_pEntity->GetBasePosition();
			a_rvOutPosition.y += a_pEntity->GetBBox().y;

			if ( a_pTouches != TNULL )
			{
				while ( a_pTouches->Size() > iNumTouchesOld )
					a_pTouches->PopBack();
			}

			return TVector4::VEC_ZERO;
		}
	}

	a_rvOutPosition = vEnd;
	a_rvOutPosition.y += a_pEntity->GetBBox().y;
	return a_rcMove;
}

// $deBlob: FUNCTION 007e9b30
void AMoveHelper::AddTouchedObject( TouchList* a_pTouches, ACollisionObject* a_pObject, const TVector4& a_rcNormal )
{
	if ( a_pTouches->Size() >= MAX_TOUCHES )
		return;

	for ( TINT i = 0; i < a_pTouches->Size(); i++ )
	{
		if ( a_pTouches->At( i ).pObject == a_pObject )
			return;
	}

	ObjectTouched oTouched;
	oTouched.pObject = a_pObject;
	oTouched.vNormal = a_rcNormal;
	a_pTouches->PushBack( oTouched );
}

// $deBlob: FUNCTION 007e9b80
TBOOL AMoveHelper::SlideMove( ABaseEntity* a_pEntity, const TVector4& a_rcStart, TVector4& a_rvMove, TVector4& a_rvOutPosition, TouchList* a_pTouches, TBOOL a_bProject, const TVector4& a_rcProjectNormal, ACollisionObject* a_pIgnore, void* a_pPhantom )
{
	SlideCollisionFunctor oFunctor( a_pTouches, a_pIgnore );
	TBOOL                 bVertical = TFALSE;

	a_rvOutPosition     = a_rcStart;
	TVector4 vRemaining = a_rvMove;

	for ( TINT i = 0; i < 4; i++ )
	{
		if ( a_bProject == TTRUE )
		{
			TVector4 vProjected;
			vProjected.Multiply( a_rcProjectNormal, a_rcProjectNormal.y * vRemaining.y + a_rcProjectNormal.x * vRemaining.x + a_rcProjectNormal.z * vRemaining.z );
			vRemaining.x -= vProjected.x;
			vRemaining.y -= vProjected.y;
			vRemaining.z -= vProjected.z;
		}

		TFLOAT fLength = vRemaining.Magnitude();
		oFunctor.SetBottom( a_rvOutPosition.y + a_pEntity->GetBBox().y );

		if ( fLength <= TMath::TFLOAT_EPSILON )
			break;

		TVector4 vDirection = vRemaining;
		TFLOAT   fDistance  = fLength;
		vDirection.Divide( fLength );

		// The phantom's collision layer
		if ( a_pPhantom != TNULL )
			TPhysicsManager::GetSingleton()->SetPhantomCollisionFilterInfo( TSTATICCAST( TPhysicsPhantomContainer, a_pPhantom ), MOVE_COLLISION_LAYER * 0x1000 );

		TVector4          vNormal;
		TFLOAT            fStep;
		ACollisionObject* pHit = ACollisionManager2::GetSingleton()->CollideSweptAABB( a_rvOutPosition, a_pEntity->GetBBox(), vDirection, fDistance, vNormal, 0.0f, fStep, a_pEntity->GetCollisionObject(), &oFunctor, MOVE_COLLISION_LAYER, a_pPhantom );

		TVector4 vMoved;
		vMoved.Multiply( vDirection, fDistance );
		a_rvOutPosition.x += vMoved.x;
		a_rvOutPosition.y += vMoved.y;
		a_rvOutPosition.z += vMoved.z;

		if ( pHit == TNULL )
			break;

		if ( a_pTouches != TNULL )
			AddTouchedObject( a_pTouches, pHit, vNormal );

		TVector4 vMask( 1.0f, 1.0f, 1.0f, 1.0f );

		if ( vNormal.y <= -0.64278764f || 0.64278764f <= vNormal.y )
		{
			// A floor or a ceiling
			bVertical = TTRUE;
		}
		else
		{
			// A wall: pushed away horizontally
			if ( a_rvMove.y <= 0.0f || vRemaining.y <= 0.0f )
			{
				vNormal.y = 0.0f;
				vNormal.Normalise();
			}

			vMask.y = 0.0f;
		}

		// Moves off the surface a bit
		TVector4 vOffset;
		vOffset.Multiply( vNormal, vMask );
		vOffset.Multiply( AStaticCVars::cvCollisionOffset.GetFloat() );
		a_rvOutPosition.x += vOffset.x;
		a_rvOutPosition.y += vOffset.y;
		a_rvOutPosition.z += vOffset.z;

		// The rest of the move, reflected off the surface: its tangent part slides (friction), its normal part bounces
		TVector4 vReflected;
		vReflected.Multiply( vNormal, ( vNormal.x * vDirection.x + vDirection.y * vNormal.y + vNormal.z * vDirection.z ) * -2.0f );
		vReflected.x += vDirection.x;
		vReflected.y += vDirection.y;
		vReflected.z += vDirection.z;
		vReflected.w = vDirection.w;
		vReflected.Normalise();

		TFLOAT fRest = fLength - fDistance;

		TVector4 vNormalPart;
		vNormalPart.Multiply( vNormal, ( vReflected.x * vNormal.x + vReflected.y * vNormal.y + vReflected.z * vNormal.z ) * fRest );

		TVector4 vReflectedRest;
		vReflectedRest.Multiply( vReflected, fRest );

		TVector4 vTangentPart( vReflectedRest.x - vNormalPart.x, vReflectedRest.y - vNormalPart.y, vReflectedRest.z - vNormalPart.z, vReflectedRest.w );
		vTangentPart.Multiply( AStaticCVars::cvCollisionFriction.GetFloat() );
		vNormalPart.Multiply( AStaticCVars::cvCollisionBump.GetFloat() );

		vRemaining.x = vTangentPart.x + vNormalPart.x;
		vRemaining.y = vTangentPart.y + vNormalPart.y;
		vRemaining.z = vTangentPart.z + vNormalPart.z;
		vRemaining.w = vNormalPart.w;
	}

	a_rvMove.x = a_rvOutPosition.x - a_rcStart.x;
	a_rvMove.z = a_rvOutPosition.z - a_rcStart.z;

	if ( bVertical == TTRUE )
		a_rvMove.y = a_rvOutPosition.y - a_rcStart.y;

	return TTRUE;
}

// $deBlob: FUNCTION 007ea250
TBOOL AMoveHelper::MoveHelperFunctor::HitNonSolidObject( ACollisionObject* a_pObject )
{
	if ( m_pTouches != TNULL )
		AddTouchedObject( m_pTouches, a_pObject, TVector4::VEC_ZERO );

	return TFALSE;
}

// $deBlob: FUNCTION 007ea210
TBOOL AMoveHelper::MoveHelperFunctor::HitIgnorableObject( ACollisionObject* a_pObject )
{
	return a_pObject == m_pIgnore;
}

// $deBlob: FUNCTION 007ea2f0
TBOOL AMoveHelper::SlideCollisionFunctor::PreFilterSweptAABBPoly( const TVector4& a_rNormal, const TVector4* a_pTriangle )
{
	if ( -0.64278764f < a_rNormal.y || a_pTriangle == TNULL )
		return TFALSE;

	// The lowest vertex (y is down): a floor below the bottom of the box is ignored
	const TFLOAT* pLowest = &a_pTriangle[ 0 ].y;

	if ( a_pTriangle[ 0 ].y <= a_pTriangle[ 1 ].y )
		pLowest = &a_pTriangle[ 1 ].y;

	if ( *pLowest < a_pTriangle[ 2 ].y )
		pLowest = &a_pTriangle[ 2 ].y;

	if ( m_fBottom > *pLowest )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007ea270
TBOOL AMoveHelper::SlideCollisionFunctor::HitNonSolidObject( ACollisionObject* a_pObject )
{
	if ( m_pTouches != TNULL )
		AddTouchedObject( m_pTouches, a_pObject, TVector4::VEC_ZERO );

	return TFALSE;
}

// $deBlob: FUNCTION 007ea230
TBOOL AMoveHelper::SlideCollisionFunctor::HitIgnorableObject( ACollisionObject* a_pObject )
{
	return a_pObject == m_pIgnore;
}
