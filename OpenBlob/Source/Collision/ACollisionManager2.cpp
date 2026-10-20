#include "pch.h"
#include "ACollisionManager2.h"
#include "ACollisionFunctor.h"
#include "ACollisionInstance2.h"
#include "Entities/ABaseEntity.h"

#include <Physics/TBoxData.h>
#include <Physics/TSphereData.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/Havok/TPhysicsPhantomContainer.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005a8360
ACollisionObject* ACollisionManager2::CollideSweptAABB( const TVector4& a_rcPosition, const TVector4& a_rcHalfExtents, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rvOutNormal, TFLOAT a_fStepHeight, TFLOAT& a_rfOutStep, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer, void* a_pPhantom )
{
	ACollisionObject* pHitObject = TNULL;
	a_rfOutStep                  = -2.0f;

	// The phantom is only used while it's in the world
	TPhysicsPhantomContainer* pPhantom = TSTATICCAST( TPhysicsPhantomContainer, a_pPhantom );

	if ( pPhantom != TNULL && !pPhantom->IsInSimulation() )
		pPhantom = TNULL;

	TBoxData oBox;
	oBox.SetHalfExtents( a_rcHalfExtents );

	TMatrix44 mStart = TMatrix44::IDENTITY;
	TASSERT( a_rcPosition.w == 1.0f );
	mStart.SetTranslation( a_rcPosition );

	TVector4 vMove;
	vMove.Multiply( a_rcDirection, a_rfDistance );

	TVector4 vEnd( a_rcPosition.x + vMove.x, a_rcPosition.y + vMove.y, a_rcPosition.z + vMove.z, a_rcPosition.w );

	TPhysicsShapeContact aContacts[ MAX_SWEEP_CONTACTS ];
	TINT                 iNumContacts = TPhysicsManager::GetSingleton()->SweepShape( &oBox, mStart, vEnd, a_uiCollisionLayer, aContacts, MAX_SWEEP_CONTACTS, pPhantom );

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact&    rContact = aContacts[ i ];
		TPhysicsObjectContainer* pBody    = rContact.m_pBody;

		// The collision object of the body's owner
		// NOTE: retail reads the owner info of a TNULL body
		ACollisionObject*               pObject    = TNULL;
		ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pBody != TNULL ? pBody->m_pUserData : TNULL );

		switch ( pOwnerInfo->m_eType )
		{
			case 1:
			case 2:
				pObject = TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->GetCollisionObject();
				break;
			case 3:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0x1C );
				break;
			case 4:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0xC );
				break;
			default:
				break;
		}

		TBOOL bIgnorable = TFALSE;

		if ( a_pFunctor != TNULL )
			bIgnorable = a_pFunctor->HitIgnorableObject( pObject );

		if ( pObject == a_pIgnore || bIgnorable )
			continue;

		if ( a_pFunctor != TNULL )
		{
			TINT16 eShapeType = pBody != TNULL ? pBody->m_eShapeType : 0;

			if ( eShapeType == 6 )
			{
				// A mesh: the triangle that was hit
				TVector4 avTriangle[ 3 ] = { rContact.m_avTriangle[ 0 ], rContact.m_avTriangle[ 1 ], rContact.m_avTriangle[ 2 ] };

				if ( a_pFunctor->PreFilterSweptAABBPoly( rContact.m_vSurfaceNormal, avTriangle ) )
					continue;

				a_pFunctor->AABBSweptHitPoly( avTriangle, rContact.m_fDistance * a_rfDistance );
			}
			else
			{
				if ( a_pFunctor->PreFilterSweptAABBPoly( rContact.m_vSurfaceNormal, TNULL ) )
					continue;

				a_pFunctor->AABBSweptHitPoly( TNULL, rContact.m_fDistance * a_rfDistance );
			}
		}

		// The first contact that is accepted
		a_rvOutNormal = rContact.m_vSurfaceNormal;
		a_rfDistance  = rContact.m_fDistance * a_rfDistance;

		if ( a_rfDistance <= 0.0f )
			a_rfDistance = 0.0f;

		GetStepHeight( rContact, a_rcPosition, a_rcHalfExtents, a_fStepHeight, &a_rfOutStep );
		pHitObject = pObject;

		TASSERT( pObject == TNULL || a_rfOutStep >= -1.0f || a_fStepHeight <= 0.0f );
		break;
	}

	return pHitObject;
}

// $deBlob: FUNCTION 005a8780
ACollisionObject* ACollisionManager2::CollideAABB( const TVector4& a_rcCenter, const TVector4& a_rcHalfExtents, TVector4& a_rvOutNormal, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer )
{
	TBoxData oBox;
	oBox.SetHalfExtents( a_rcHalfExtents );

	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	oMatrix.SetTranslation( a_rcCenter );

	TPhysicsShapeContact aContacts[ MAX_SWEEP_CONTACTS ];
	TINT                 iNumContacts = TPhysicsManager::GetSingleton()->CollideShape( &oBox, oMatrix, a_uiCollisionLayer, aContacts, MAX_SWEEP_CONTACTS, TNULL );

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact&    rContact = aContacts[ i ];
		TPhysicsObjectContainer* pBody    = rContact.m_pBody;

		// The collision object of the body's owner
		// NOTE: retail reads the owner info of a TNULL body
		ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pBody != TNULL ? pBody->m_pUserData : TNULL );
		ACollisionObject*               pObject    = TNULL;

		switch ( pOwnerInfo->m_eType )
		{
			case 1:
			case 2:
				pObject = TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->GetCollisionObject();
				break;
			case 3:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0x1C );
				break;
			case 4:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0xC );
				break;
			default:
				break;
		}

		TBOOL bIgnorable = TFALSE;
		TBOOL bFiltered  = TFALSE;

		if ( a_pFunctor != TNULL )
		{
			bIgnorable = a_pFunctor->HitIgnorableObject( pObject );
			bFiltered  = a_pFunctor->PreFilterAABBPoly( rContact.m_vSurfaceNormal );
		}

		if ( pObject != a_pIgnore && !bIgnorable && !bFiltered )
		{
			a_rvOutNormal = rContact.m_vSurfaceNormal;
			return pObject;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005a8c30
ACollisionObject* ACollisionManager2::CollideRaySolidFast( const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer, TUINT32* a_pOutMaterial )
{
	if ( a_rfDistance <= 0.0f )
		return TNULL;

	TVector4 vMove;
	vMove.Multiply( a_rcDirection, a_rfDistance );

	TPhysicsRay oRay;
	oRay.m_vFrom            = a_rcStart;
	oRay.m_vTo              = TVector4( a_rcStart.x + vMove.x, a_rcStart.y + vMove.y, a_rcStart.z + vMove.z, a_rcStart.w );
	oRay.m_uiCollisionLayer = a_uiCollisionLayer;

	TPhysicsShapeContact aContacts[ MAX_SWEEP_CONTACTS ];
	TINT                 iNumContacts = TPhysicsManager::GetSingleton()->CollideRayAll( oRay, aContacts, MAX_SWEEP_CONTACTS, TNULL );

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact&    rContact = aContacts[ i ];
		TPhysicsObjectContainer* pBody    = rContact.m_pBody;

		// The collision object of the body's owner
		// NOTE: retail reads the owner info of a TNULL body
		ACollisionObject*               pObject    = TNULL;
		ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pBody != TNULL ? pBody->m_pUserData : TNULL );

		switch ( pOwnerInfo->m_eType )
		{
			case 1:
			case 2:
				pObject = TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->GetCollisionObject();
				break;
			case 3:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0x1C );
				break;
			case 4:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0xC );
				break;
			default:
				break;
		}

		TBOOL bIgnorable = TFALSE;
		TBOOL bFiltered  = TFALSE;

		if ( a_pFunctor != TNULL )
		{
			bIgnorable = a_pFunctor->HitIgnorableObject( pObject );
			bFiltered  = a_pFunctor->PreFilterAABBPoly( rContact.m_vSurfaceNormal );
		}

		if ( pObject != a_pIgnore && !bIgnorable && !bFiltered )
		{
			// The first contact that is accepted
			a_rfDistance = rContact.m_fDistance * a_rfDistance;

			if ( a_pOutMaterial != TNULL )
				*a_pOutMaterial = rContact.m_uiMaterial;

			return pObject;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005a89a0
ACollisionObject* ACollisionManager2::CollideRay( const TVector4& a_rcStart, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rvOutNormal, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer, void* a_pPhantom )
{
	if ( a_rfDistance <= 0.0f )
		return TNULL;

	TVector4 vMove;
	vMove.Multiply( a_rcDirection, a_rfDistance );

	TPhysicsRay oRay;
	oRay.m_vFrom            = a_rcStart;
	oRay.m_vTo              = TVector4( a_rcStart.x + vMove.x, a_rcStart.y + vMove.y, a_rcStart.z + vMove.z, a_rcStart.w );
	oRay.m_uiCollisionLayer = a_uiCollisionLayer;

	// The phantom is only used while it's in the world
	TPhysicsPhantomContainer* pPhantom = TSTATICCAST( TPhysicsPhantomContainer, a_pPhantom );

	if ( pPhantom != TNULL && !pPhantom->IsInSimulation() )
		pPhantom = TNULL;

	TPhysicsShapeContact aContacts[ MAX_SWEEP_CONTACTS ];
	TINT                 iNumContacts = TPhysicsManager::GetSingleton()->CollideRayAll( oRay, aContacts, MAX_SWEEP_CONTACTS, pPhantom );

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact&    rContact = aContacts[ i ];
		TPhysicsObjectContainer* pBody    = rContact.m_pBody;

		// The collision object of the body's owner
		// NOTE: retail reads the owner info of a TNULL body
		ACollisionObject*               pObject    = TNULL;
		ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pBody != TNULL ? pBody->m_pUserData : TNULL );

		switch ( pOwnerInfo->m_eType )
		{
			case 1:
			case 2:
				pObject = TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->GetCollisionObject();
				break;
			case 3:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0x1C );
				break;
			case 4:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0xC );
				break;
			default:
				break;
		}

		if ( pObject == a_pIgnore )
			continue;

		TINT iShapeType = ( pBody != TNULL ) ? pBody->m_eShapeType : 0;

		if ( a_pFunctor != TNULL )
		{
			if ( a_pFunctor->HitIgnorableObject( pObject ) )
				continue;

			if ( iShapeType == 6 )
			{
				// A mesh: the triangle hit
				TVector4 avTriangle[ 3 ] = { rContact.m_avTriangle[ 0 ], rContact.m_avTriangle[ 1 ], rContact.m_avTriangle[ 2 ] };

				if ( a_pFunctor->PreFilterSweptAABBPoly( rContact.m_vSurfaceNormal, avTriangle ) )
					continue;

				a_pFunctor->AABBSweptHitPoly( avTriangle, rContact.m_fDistance * a_rfDistance );
			}
			else
			{
				if ( a_pFunctor->PreFilterSweptAABBPoly( rContact.m_vSurfaceNormal, TNULL ) )
					continue;

				a_pFunctor->AABBSweptHitPoly( TNULL, rContact.m_fDistance * a_rfDistance );
			}
		}

		// The first contact that is accepted
		a_rfDistance  = rContact.m_fDistance * a_rfDistance;
		a_rvOutNormal = rContact.m_vNormal;
		return pObject;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005a8fe0
ACollisionObject* ACollisionManager2::CollideSweptSphere( const TVector4& a_rcPosition, TFLOAT a_fRadius, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rvOutNormal, TFLOAT a_fStepHeight, TFLOAT& a_rfOutStep, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer )
{
	TSphereData oSphere;
	oSphere.SetRadius( a_fRadius );

	TMatrix44 mStart = TMatrix44::IDENTITY;
	TASSERT( a_rcPosition.w == 1.0f );
	mStart.SetTranslation( a_rcPosition );

	TVector4 vMove;
	vMove.Multiply( a_rcDirection, a_rfDistance );

	TVector4 vEnd( a_rcPosition.x + vMove.x, a_rcPosition.y + vMove.y, a_rcPosition.z + vMove.z, a_rcPosition.w );

	TPhysicsShapeContact aContacts[ MAX_SWEEP_CONTACTS ];
	TINT                 iNumContacts = TPhysicsManager::GetSingleton()->SweepShape( &oSphere, mStart, vEnd, a_uiCollisionLayer, aContacts, MAX_SWEEP_CONTACTS, TNULL );

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact&    rContact = aContacts[ i ];
		TPhysicsObjectContainer* pBody    = rContact.m_pBody;

		// The collision object of the body's owner
		// NOTE: retail reads the owner info of a TNULL body
		ACollisionObject*               pObject    = TNULL;
		ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pBody != TNULL ? pBody->m_pUserData : TNULL );

		switch ( pOwnerInfo->m_eType )
		{
			case 1:
			case 2:
				pObject = TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->GetCollisionObject();
				break;
			case 3:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0x1C );
				break;
			case 4:
				pObject = *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0xC );
				break;
			default:
				break;
		}

		TBOOL bIgnorable = TFALSE;
		TBOOL bFiltered  = TFALSE;

		if ( a_pFunctor != TNULL )
		{
			bIgnorable = a_pFunctor->HitIgnorableObject( pObject );
			bFiltered  = a_pFunctor->PreFilterSweptSpherePoly( rContact.m_vSurfaceNormal );
		}

		if ( pObject == a_pIgnore || bIgnorable || bFiltered )
			continue;

		// The first contact that is accepted
		a_rfDistance  = rContact.m_fDistance * a_rfDistance;
		a_rvOutNormal = rContact.m_vNormal;

		if ( pBody != TNULL && pBody->m_eShapeType == 6 && a_fStepHeight > 0.0f )
		{
			// A mesh: the height from the lowest corner of the triangle to the top of the sphere
			const TFLOAT* pLowest = &rContact.m_avTriangle[ 0 ].y;

			if ( *pLowest > rContact.m_avTriangle[ 1 ].y )
				pLowest = &rContact.m_avTriangle[ 1 ].y;

			if ( *pLowest > rContact.m_avTriangle[ 2 ].y )
				pLowest = &rContact.m_avTriangle[ 2 ].y;

			TFLOAT fStep = ( a_rcPosition.y + a_fRadius ) - *pLowest;
			a_rfOutStep  = ( a_fStepHeight < fStep ) ? -1.0f : fStep;
		}

		if ( a_rfDistance <= 0.0f )
			a_rfDistance = 0.0f;

		return pObject;
	}

	return TNULL;
}

// $deBlob: FUNCTION 005a96b0
TFLOAT* ACollisionManager2::GetStepHeight( const TPhysicsShapeContact& a_rcContact, const TVector4& a_rcPosition, const TVector4& a_rcHalfExtents, TFLOAT a_fStepHeight, TFLOAT* a_pOutStep )
{
	TPhysicsObjectContainer* pBody = a_rcContact.m_pBody;

	if ( pBody != TNULL )
	{
		if ( pBody->m_eShapeType == 6 )
		{
			// A mesh: from the bottom of the box (y is down) to the highest vertex of the triangle
			TFLOAT fBottom = a_rcHalfExtents.y + a_rcPosition.y;

			if ( a_fStepHeight > 0.0f )
			{
				const TFLOAT* pTop = &a_rcContact.m_avTriangle[ 0 ].y;

				if ( a_rcContact.m_avTriangle[ 1 ].y <= *pTop )
					pTop = &a_rcContact.m_avTriangle[ 1 ].y;

				if ( *pTop > a_rcContact.m_avTriangle[ 2 ].y )
					pTop = &a_rcContact.m_avTriangle[ 2 ].y;

				TFLOAT fStep = fBottom - *pTop;

				if ( fStep <= 0.0f && fStep > -a_fStepHeight )
				{
					*a_pOutStep = 0.0f;
					return a_pOutStep;
				}

				if ( fStep > 0.0f && a_fStepHeight > fStep )
				{
					*a_pOutStep = fStep;
					return a_pOutStep;
				}
			}
		}
		else if ( pBody->m_eShapeType == TShapeBaseClass::Type_Box )
		{
			// NOTE: retail returns the body (not a_pOutStep) and leaves *a_pOutStep alone here
			if ( a_fStepHeight <= 0.0f )
				return TREINTERPRETCAST( TFLOAT*, pBody );

			// The top of the box: its half extent along the world axis closest to its up axis
			const TMatrix44& rcMatrix = *pBody->m_pMatrix;
			TFLOAT           fXX      = TMath::Abs( rcMatrix.m_f11 );
			TFLOAT           fYY      = TMath::Abs( rcMatrix.m_f22 );
			TFLOAT           fZZ      = TMath::Abs( rcMatrix.m_f33 );

			TVector4 vHalfExtents = TSTATICCAST( TBoxData, pBody->m_pShapeData )->GetHalfExtents();
			TMatrix44::TransformVector( vHalfExtents, rcMatrix, vHalfExtents );

			TFLOAT fHalfHeight;

			if ( fXX <= fYY )
				fHalfHeight = ( fYY > fZZ ) ? vHalfExtents.y : vHalfExtents.z;
			else
				fHalfHeight = ( fXX <= fZZ ) ? vHalfExtents.z : vHalfExtents.x;

			TFLOAT fStep = TMath::Abs( ( a_rcPosition.y + fHalfHeight ) - ( rcMatrix.m_f42 - TMath::Abs( fHalfHeight ) ) );

			if ( a_fStepHeight < fStep )
				fStep = -1.0f;

			*a_pOutStep = fStep;
			return a_pOutStep;
		}
	}

	*a_pOutStep = -1.0f;
	return a_pOutStep;
}

// $deBlob: FUNCTION 005a8e30
void ACollisionManager2::CollideRayUp( const TFLOAT* a_pfLength, ACollisionObject* a_pIgnore, const TVector4* a_pcPosition, TVector4* a_pOutPosition, TVector4* a_pOutNormal )
{
	if ( *a_pfLength <= 0.0f )
		return;

	TPhysicsRay oRay;
	oRay.m_vFrom = *a_pcPosition;

	TVector4 vUp;
	vUp.Multiply( TVector4( 0.0f, 1.0f, 0.0f, 1.0f ), *a_pfLength );
	oRay.m_vTo              = TVector4( a_pcPosition->x + vUp.x, a_pcPosition->y + vUp.y, a_pcPosition->z + vUp.z, a_pcPosition->w );
	oRay.m_uiCollisionLayer = 0x87;

	TPhysicsShapeContact aContacts[ MAX_SWEEP_CONTACTS ];
	TINT                 iNumContacts = TPhysicsManager::GetSingleton()->CollideRayAll( oRay, aContacts, MAX_SWEEP_CONTACTS, TNULL );

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		if ( GetCollisionObject( aContacts[ i ].m_pBody ) != a_pIgnore )
		{
			*a_pOutPosition = aContacts[ i ].m_vPosition;
			*a_pOutNormal   = aContacts[ i ].m_vNormal;
			return;
		}
	}
}

// $deBlob: FUNCTION 005a9300
void ACollisionManager2::CollideSphere( const TVector4& a_rcPosition, TFLOAT* a_pOutDistance, TVector4* a_pOutNormal, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer )
{
	// NOTE: the sphere keeps TSphereData's default radius
	TSphereData oSphere;

	TMatrix44 mMatrix = TMatrix44::IDENTITY;
	mMatrix.SetTranslation( TVector4( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f ) );

	TPhysicsShapeContact aContacts[ MAX_SWEEP_CONTACTS ];
	TINT                 iNumContacts = TPhysicsManager::GetSingleton()->CollideShape( &oSphere, mMatrix, a_uiCollisionLayer, aContacts, MAX_SWEEP_CONTACTS, TNULL );

	for ( TINT i = 0; i < iNumContacts; i++ )
	{
		TPhysicsShapeContact& rContact = aContacts[ i ];
		ACollisionObject*     pObject  = GetCollisionObject( rContact.m_pBody );

		TBOOL bIgnorable = TFALSE;
		TBOOL bFiltered  = TFALSE;

		if ( a_pFunctor != TNULL )
		{
			bIgnorable = a_pFunctor->HitIgnorableObject( pObject );
			bFiltered  = a_pFunctor->PreFilterSpherePoly( rContact.m_vSurfaceNormal );
		}

		if ( pObject != a_pIgnore && !bIgnorable && !bFiltered )
		{
			TVector4 vOffset( a_rcPosition.x - rContact.m_vPosition.x, a_rcPosition.y - rContact.m_vPosition.y, a_rcPosition.z - rContact.m_vPosition.z, 1.0f );
			*a_pOutDistance = vOffset.Magnitude();
			*a_pOutNormal   = rContact.m_vNormal;
			break;
		}
	}
}

// $deBlob: FUNCTION 005a95e0
ACollisionObject* ACollisionManager2::GetCollisionObject( TPhysicsObjectContainer* a_pBody )
{
	// NOTE: retail reads the owner info of a TNULL body
	ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pBody != TNULL ? a_pBody->m_pUserData : TNULL );

	switch ( pOwnerInfo->m_eType )
	{
		case 1:
		case 2:
			return TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner )->GetCollisionObject();
		case 3:
			return *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0x1C );
		case 4:
			return *TREINTERPRETCAST( ACollisionObject**, TREINTERPRETCAST( TUINTPTR, pOwnerInfo->m_pOwner ) + 0xC );
		default:
			return TNULL;
	}
}

// $deBlob: FUNCTION 005a9640
void ACollisionManager2::GetSphereStepHeight( const TPhysicsShapeContact& a_rcContact, const TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fStepHeight, TFLOAT* a_pOutStep )
{
	if ( a_rcContact.m_pBody != TNULL && a_rcContact.m_pBody->m_eShapeType == 6 && a_fStepHeight > 0.0f )
	{
		// A mesh: the height from the lowest corner of the triangle to the top of the sphere
		const TFLOAT* pLowest = &a_rcContact.m_avTriangle[ 0 ].y;

		if ( *pLowest > a_rcContact.m_avTriangle[ 1 ].y )
			pLowest = &a_rcContact.m_avTriangle[ 1 ].y;

		if ( *pLowest > a_rcContact.m_avTriangle[ 2 ].y )
			pLowest = &a_rcContact.m_avTriangle[ 2 ].y;

		TFLOAT fStep = ( a_rcPosition.y + a_fRadius ) - *pLowest;

		if ( fStep <= a_fStepHeight )
		{
			*a_pOutStep = fStep;
			return;
		}

		*a_pOutStep = -1.0f;
	}
}

// $deBlob: FUNCTION 005a9880
void ACollisionManager2::GetTriangleStepHeight( const TVector4* const* a_ppTriangle, const TVector4& a_rcPosition, TFLOAT a_fStepHeight, TFLOAT* a_pOutStep )
{
	// The lowest corner of the triangle
	const TFLOAT* pLowest = &a_ppTriangle[ 0 ]->y;

	if ( a_ppTriangle[ 1 ]->y <= *pLowest )
		pLowest = &a_ppTriangle[ 1 ]->y;

	if ( *pLowest > a_ppTriangle[ 2 ]->y )
		pLowest = &a_ppTriangle[ 2 ]->y;

	TFLOAT fStep = a_rcPosition.y - *pLowest;

	if ( fStep <= 0.0f && -a_fStepHeight < fStep )
	{
		*a_pOutStep = 0.0f;
		return;
	}

	if ( 0.0f < fStep && fStep < a_fStepHeight )
	{
		*a_pOutStep = fStep;
		return;
	}

	*a_pOutStep = -1.0f;
}

// $deBlob: FUNCTION 005a9910
TBOOL ACollisionManager2::GetClosestPointOnSegment( const TVector4& a_rcPoint, const TVector4& a_rcFrom, const TVector4& a_rcTo, TVector4* a_pOutPoint )
{
	TVector4 vSegment( a_rcTo.x - a_rcFrom.x, a_rcTo.y - a_rcFrom.y, a_rcTo.z - a_rcFrom.z, a_rcTo.w );
	TFLOAT   fDot = vSegment.x * ( a_rcPoint.x - a_rcFrom.x ) + vSegment.y * ( a_rcPoint.y - a_rcFrom.y ) + vSegment.z * ( a_rcPoint.z - a_rcFrom.z );

	if ( 0.0f <= fDot )
	{
		TFLOAT fLength = vSegment.Magnitude();
		fDot           = fDot / fLength;

		if ( fDot <= fLength )
		{
			vSegment.Divide( fLength );

			TVector4 vOffset;
			vOffset.Multiply( vSegment, fDot );

			*a_pOutPoint = TVector4( a_rcFrom.x + vOffset.x, a_rcFrom.y + vOffset.y, a_rcFrom.z + vOffset.z, a_rcFrom.w );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005a9a40
TFLOAT ACollisionManager2::GetSlope( const TVector4& a_rcFrom, const TVector4& a_rcTo )
{
	TFLOAT fHeight   = a_rcFrom.y - a_rcTo.y;
	TFLOAT fDistance = TVector4::DistanceXZ( a_rcFrom, a_rcTo );

	if ( TMath::Abs( fHeight ) <= fDistance )
	{
		TFLOAT ret = fHeight / fDistance;
		TASSERT( ret > -100.0f && ret < 100.0f );
		return ret;
	}

	return 0.0f;
}
