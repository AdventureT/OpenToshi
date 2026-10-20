#include "pch.h"
#include "AAIPath.h"
#include "AAINode.h"
#include "AAIMoveProbe.h"
#include "APathfindingComponent.h"
#include "Entities/ABaseEntity.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AAIPath::OptimiseParams AAIPath::ms_oOptimiseParams = { 12.0f, 40.0f, 4.0f, 8 };

// $deBlob: FUNCTION 007e5eb0
AAIPath::AAIPath( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
}

// $deBlob: FUNCTION 007e6a70
// $deBlob: FUNCTION 007e5f10
AAIPath::~AAIPath()
{
	ClearPath();
}

// $deBlob: FUNCTION 007e63d0
void AAIPath::ClearPath()
{
	while ( !m_Path.IsEmpty() )
		AAIWaypoint::sm_FreeList.PushBack( m_Path.PopBack() );
}

// $deBlob: FUNCTION 007dfe60
void AAIPath::AddWaypoint( AAIWaypoint* a_pWaypoint )
{
	m_Path.PushBack( a_pWaypoint );
}

// $deBlob: FUNCTION 007e6470
void AAIPath::RemoveWaypoints( AAIWaypoint* a_pStart, AAIWaypoint* a_pEnd )
{
	AAIWaypoint* pWaypoint = a_pStart;

	while ( pWaypoint != a_pEnd )
	{
		AAIWaypoint* pNext = TSTATICCAST( AAIWaypoint, pWaypoint->Next() );
		pWaypoint->Remove();
		AAIWaypoint::sm_FreeList.PushBack( pWaypoint );
		pWaypoint = pNext;
	}

	a_pEnd->Remove();
	AAIWaypoint::sm_FreeList.PushBack( a_pEnd );
}

// $deBlob: FUNCTION 007e6970
AAIWaypoint* AAIPath::ShortcutPath( AAIWaypoint* a_pStart, AAIWaypoint* a_pTarget, const TVector4& a_rvPosition )
{
	if ( a_pTarget->m_vPosition.IsEqual( a_rvPosition ) )
	{
		RemoveWaypoints( a_pStart, TSTATICCAST( AAIWaypoint, a_pTarget->Prev() ) );
		return a_pTarget;
	}

	AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
	pWaypoint->m_fRadius   = 0.01f;
	pWaypoint->m_vPosition = a_rvPosition;
	pWaypoint->m_pNode     = TNULL;
	pWaypoint->m_uiFlags   = 0;

	m_Path.InsertAfter( a_pTarget, pWaypoint );
	RemoveWaypoints( a_pStart, a_pTarget );

	return pWaypoint;
}

// $deBlob: FUNCTION 007e65f0
TBOOL AAIPath::LookAheadOptimise( const OptimiseParams& a_rcParams, AAIWaypoint* a_pWaypoint, AAIWaypoint*& a_pNewWaypoint )
{
	TINT   iSteps        = 0;
	TFLOAT fDistanceLeft = a_rcParams.fLookAheadDistance - TVector4::Distance( m_pEntity->GetTranslation(), a_pWaypoint->m_vPosition );

	if ( fDistanceLeft < 0.1f )
		return TFALSE;

	return LookAheadOptimiseRecursive( a_pWaypoint, a_pWaypoint->m_vPosition, a_rcParams, TFALSE, fDistanceLeft, iSteps, a_pWaypoint, a_pNewWaypoint );
}

// $deBlob: FUNCTION 007e6670
TBOOL AAIPath::LookAheadOptimiseRecursive( AAIWaypoint* a_pWaypoint, const TVector4& a_rvPosition, const OptimiseParams& a_rcParams, TBOOL a_bCanShortcut, TFLOAT& a_rfDistanceLeft, TINT& a_riSteps, AAIWaypoint* a_pStartWaypoint, AAIWaypoint*& a_pNewWaypoint )
{
	if ( a_rfDistanceLeft > 0.0f && a_riSteps++ < a_rcParams.iMaxSteps )
	{
		AAIWaypoint* pNext = TSTATICCAST( AAIWaypoint, a_pWaypoint->Next() );

		// Never across a jump
		if ( a_pWaypoint->m_pNode != TNULL && pNext->m_pNode != TNULL )
		{
			AAILink* pLink = a_pWaypoint->m_pNode->GetLinkTo( pNext->m_pNode );

			if ( pLink != TNULL && pLink->m_uiMoveType == AAILink::MOVETYPE_Jump )
				return TFALSE;
		}

		if ( ( a_pWaypoint->m_uiFlags & AAIWaypoint::FLAG_Jump ) || ( pNext->m_uiFlags & AAIWaypoint::FLAG_Jump ) )
			return TFALSE;

		TFLOAT       fDistance = TVector4::Distance( a_rvPosition, pNext->m_vPosition );
		TVector4     vPosition;
		AAIWaypoint* pWaypoint;

		if ( a_rcParams.fStep * 1.1f <= fDistance )
		{
			// One step towards the next waypoint
			TVector4 vDirection( pNext->m_vPosition.x - a_pWaypoint->m_vPosition.x, pNext->m_vPosition.y - a_pWaypoint->m_vPosition.y, pNext->m_vPosition.z - a_pWaypoint->m_vPosition.z, pNext->m_vPosition.w );
			vDirection.Normalise();

			TVector4 vStep;
			vStep.Multiply( vDirection, a_rcParams.fStep );

			vPosition = TVector4( vStep.x + a_rvPosition.x, a_rvPosition.y + vStep.y, a_rvPosition.z + vStep.z, a_rvPosition.w );
			a_rfDistanceLeft -= a_rcParams.fStep;
			pWaypoint = a_pWaypoint;
		}
		else
		{
			// On to the next waypoint
			vPosition = pNext->m_vPosition;
			a_rfDistanceLeft -= fDistance;
			pWaypoint = pNext;

			if ( pNext == m_Path.Back() )
			{
				if ( !a_bCanShortcut || ( pNext->m_uiFlags & AAIWaypoint::FLAG_NoShortcut ) || !CanShortcut( vPosition ) )
					return TFALSE;

				TASSERT( TNULL == a_pNewWaypoint );
				a_pNewWaypoint = ShortcutPath( a_pStartWaypoint, pNext, vPosition );
				return TTRUE;
			}
		}

		TFLOAT fDistanceToEntity = TVector4::Distance( m_pEntity->GetTranslation(), vPosition );

		if ( LookAheadOptimiseRecursive( pWaypoint, vPosition, a_rcParams, a_rcParams.fShortcutDistance >= fDistanceToEntity, a_rfDistanceLeft, a_riSteps, a_pStartWaypoint, a_pNewWaypoint ) )
			return TTRUE;
	}

	// Nothing further along: shortcut to here
	if ( !a_bCanShortcut || ( a_pWaypoint->m_uiFlags & AAIWaypoint::FLAG_NoShortcut ) || !CanShortcut( a_rvPosition ) )
		return TFALSE;

	TASSERT( TNULL == a_pNewWaypoint );
	a_pNewWaypoint = ShortcutPath( a_pStartWaypoint, a_pWaypoint, a_rvPosition );
	return TTRUE;
}

// $deBlob: FUNCTION 007e6a30
TBOOL AAIPath::CanShortcut( const TVector4& a_rvPosition )
{
	AAIMoveProbe* pMoveProbe = TSTATICCAST( APathfindingComponent, m_pEntity->GetPathfinding() )->GetMoveProbe();
	TVector4      vGoal      = a_rvPosition;

	return pMoveProbe->CheckGroundMove( m_pEntity->GetTranslation(), vGoal, 2 ) == AAIMoveProbe::PROBERESULT_Clear ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 007e5f80
AAIWaypoint* AAIPath::OptimisePath( AAIWaypoint* a_pStartwaypoint )
{
	if ( a_pStartwaypoint == m_Path.Back() )
		return a_pStartwaypoint;

	TASSERT( m_Path.IsInList( a_pStartwaypoint ) );

	AAIWaypoint* pShortcutWaypoint = TNULL;
	TINT         iSteps            = 0;
	TFLOAT       fDistanceLeft     = ms_oOptimiseParams.fLookAheadDistance - TVector4::Distance( m_pEntity->GetTranslation(), a_pStartwaypoint->m_vPosition );

	if ( fDistanceLeft >= 0.1f && LookAheadOptimiseRecursive( a_pStartwaypoint, a_pStartwaypoint->m_vPosition, ms_oOptimiseParams, TFALSE, fDistanceLeft, iSteps, a_pStartwaypoint, pShortcutWaypoint ) )
	{
		TASSERT( TNULL != pShortcutWaypoint );
		return pShortcutWaypoint;
	}

	// Straight to the next waypoint
	AAIWaypoint* pNext = TSTATICCAST( AAIWaypoint, a_pStartwaypoint->Next() );

	if ( !( a_pStartwaypoint->m_uiFlags & AAIWaypoint::FLAG_NoShortcut ) && CanShortcut( pNext->m_vPosition ) )
		return ShortcutPath( a_pStartwaypoint, pNext, pNext->m_vPosition );

	return a_pStartwaypoint;
}

// $deBlob: FUNCTION 007e6530
TFLOAT AAIPath::GetClosestReachablePointToEnd( TVector4& a_rvOutPoint, const TVector4& a_rvStart, const TVector4& a_rvFrom, const TVector4& a_rvTo, TFLOAT a_fStep )
{
	TFLOAT fStep = a_fStep / TVector4::Distance( a_rvFrom, a_rvTo );
	TFLOAT fT    = 0.0f;
	TFLOAT fBest = 0.0f;

	a_rvOutPoint = a_rvFrom;

	do
	{
		fT = ( fT + fStep <= 1.0f ) ? fT + fStep : 1.0f;

		TVector4 vPoint;
		vPoint.Lerp4( a_rvFrom, a_rvTo, fT );

		AAIMoveProbe* pMoveProbe = TSTATICCAST( APathfindingComponent, m_pEntity->GetPathfinding() )->GetMoveProbe();

		if ( pMoveProbe->CheckGroundMove( a_rvStart, vPoint, 0 ) == AAIMoveProbe::PROBERESULT_Clear )
		{
			fBest        = fT;
			a_rvOutPoint = vPoint;
		}
	} while ( fT < 1.0f );

	return fBest;
}

// $deBlob: FUNCTION 007e6110
void AAIPath::SmoothCorneringXZ( AAIWaypoint* a_pWaypoint, TFLOAT a_fRadius )
{
	if ( a_pWaypoint->m_uiFlags & ( AAIWaypoint::FLAG_Jump | AAIWaypoint::FLAG_Fly ) )
		return;

	// NOTE: the end of the path isn't checked: after the last waypoint comes the list's root (read as a waypoint)
	AAIWaypoint* pCorner = TSTATICCAST( AAIWaypoint, a_pWaypoint->Next() );

	if ( pCorner == a_pWaypoint || ( pCorner->m_uiFlags & ( AAIWaypoint::FLAG_Jump | AAIWaypoint::FLAG_Fly ) ) )
		return;

	// Only between walked links (NOTE: GetLinkTo isn't checked for TNULL)
	AAINode* pCornerNode = pCorner->m_pNode;

	if ( a_pWaypoint->m_pNode != TNULL && pCornerNode != TNULL && a_pWaypoint->m_pNode->GetLinkTo( pCornerNode )->m_uiMoveType != AAILink::MOVETYPE_Walk )
		return;

	AAIWaypoint* pNext = TSTATICCAST( AAIWaypoint, pCorner->Next() );

	if ( pNext == pCorner || ( pNext->m_uiFlags & ( AAIWaypoint::FLAG_Jump | AAIWaypoint::FLAG_Fly ) ) )
		return;

	if ( pNext->m_pNode != TNULL && pCornerNode != TNULL && pCornerNode->GetLinkTo( pNext->m_pNode )->m_uiMoveType != AAILink::MOVETYPE_Walk )
		return;

	pCorner->Remove();
	AAIWaypoint::sm_FreeList.PushBack( pCorner );

	TVector4 vIn( pCorner->m_vPosition.x - a_pWaypoint->m_vPosition.x, pCorner->m_vPosition.y - a_pWaypoint->m_vPosition.y, pCorner->m_vPosition.z - a_pWaypoint->m_vPosition.z, pCorner->m_vPosition.w );
	TVector4 vOut( pNext->m_vPosition.x - pCorner->m_vPosition.x, pNext->m_vPosition.y - pCorner->m_vPosition.y, pNext->m_vPosition.z - pCorner->m_vPosition.z, pNext->m_vPosition.w );

	TVector4 vInDirection  = vIn;
	TVector4 vOutDirection = vOut;
	TFLOAT   fInLength     = vInDirection.NormaliseMag();
	vOutDirection.Normalise();

	TFLOAT fAngle = TMath::AngleBetweenXZ( vInDirection, vOutDirection );

	if ( TMath::Abs( fAngle ) <= 0.5235988f )
		return;

	TVector4 vOffset;
	vOffset.Multiply( vInDirection, fInLength - a_fRadius );

	TVector4 vStart( a_pWaypoint->m_vPosition.x + vOffset.x, a_pWaypoint->m_vPosition.y + vOffset.y, a_pWaypoint->m_vPosition.z + vOffset.z, a_pWaypoint->m_vPosition.w );

	vOffset.Multiply( vOutDirection, a_fRadius );

	TVector4 vEnd( pCorner->m_vPosition.x + vOffset.x, pCorner->m_vPosition.y + vOffset.y, pCorner->m_vPosition.z + vOffset.z, pCorner->m_vPosition.w );

	AAIWaypoint* pPrevious = a_pWaypoint;
	TFLOAT       fT        = 1.0f / 6.0f;

	for ( TINT i = 0; i < 4; i++ )
	{
		TVector4 vPoint;
		TMath::HermitePosition( vPoint, vStart, vInDirection, vEnd, vOutDirection, fT );
		fT += 1.0f / 6.0f;

		AAIWaypoint* pWaypoint = AAIWaypoint::GetFreeWaypoint();
		pWaypoint->m_vPosition = vPoint;
		pWaypoint->m_fRadius   = 0.01f;
		pWaypoint->m_pNode     = TNULL;
		pWaypoint->m_uiFlags   = AAIWaypoint::FLAG_NoShortcut;

		m_Path.InsertAfter( pPrevious, pWaypoint );
		pPrevious = pWaypoint;
	}
}
