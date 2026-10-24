#include "ToshiPCH.h"
#include "TPhysicsStandardMeshShape.h"
#include "TPhysicsMeshContainer.h"
#include "TPhysicsObjectContainer.h"

#include <Physics/Dynamics/Entity/hkpRigidBody.h>
#include <Physics/Collide/Shape/Compound/Collection/Mesh/hkpMeshMaterial.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0072d870
TPhysicsStandardMeshShape::TPhysicsStandardMeshShape( TPhysicsMeshContainer* a_pContainer )
    : hkpMeshShape( 0.05f, 12 )
    , m_pContainer( a_pContainer )
{
	// NOTE: Havok 5.5's hkpMeshShape ctor clears its bounding box
	m_vAabbHalfExtents.setZero4();
	m_vAabbCenter.setZero4();
}

void TPhysicsStandardMeshShape::GetContainerAabb( const TPhysicsMeshContainer* a_pContainer, const hkTransform& a_rLocalToWorld, hkReal a_fTolerance, hkAabb& a_rOut )
{
	const TVector4& rcExtents = a_pContainer->GetAABBExtents();
	const TVector4& rcCentre  = a_pContainer->GetAABBCentre();

	TMatrix44 mTransform;
	a_rLocalToWorld.get4x4ColumnMajor( &mTransform.m_f11 );

	TVector4 vX, vY, vZ;
	vX.Multiply( TVector4( rcExtents.x, rcExtents.x, rcExtents.x, 1.0f ), mTransform.AsBasisVector4( 0 ) );
	vX.Abs4();
	vY.Multiply( TVector4( rcExtents.y, rcExtents.y, rcExtents.y, 1.0f ), mTransform.AsBasisVector4( 1 ) );
	vY.Abs4();
	vZ.Multiply( TVector4( rcExtents.z, rcExtents.z, rcExtents.z, 1.0f ), mTransform.AsBasisVector4( 2 ) );
	vZ.Abs4();

	vZ.x += a_fTolerance;
	vZ.y += a_fTolerance;
	vZ.z += a_fTolerance;

	TFLOAT fHalfX = vZ.x + vY.x + vX.x;
	TFLOAT fHalfY = vZ.y + vY.y + vX.y;
	TFLOAT fHalfZ = vZ.z + vY.z + vX.z;

	TVector4 vCentre;
	TMatrix44::TransformVector( vCentre, mTransform, rcCentre );

	a_rOut.m_max.set( vCentre.x + fHalfX, vCentre.y + fHalfY, vCentre.z + fHalfZ, 0.0f );
	a_rOut.m_min.set( vCentre.x - fHalfX, vCentre.y - fHalfY, vCentre.z - fHalfZ, 0.0f );

	TASSERT( a_rOut.m_max( 0 ) >= a_rOut.m_min( 0 ) );
	TASSERT( a_rOut.m_max( 1 ) >= a_rOut.m_min( 1 ) );
	TASSERT( a_rOut.m_max( 2 ) >= a_rOut.m_min( 2 ) );
}

// $deBlob: FUNCTION 0072b460
void TPhysicsStandardMeshShape::getAabbImpl( const hkTransform& a_rLocalToWorld, hkReal a_fTolerance, hkAabb& a_rOut ) const
{
	GetContainerAabb( m_pContainer, a_rLocalToWorld, a_fTolerance, a_rOut );
}

void TPhysicsStandardMeshShape::AddContainerAabb( const TPhysicsMeshContainer* a_pContainer, const hkVector4& a_rScaling, hkReal a_fRadius, hkVector4& a_rHalfExtents, hkVector4& a_rCenter )
{
	TFLOAT fMinX = a_rCenter( 0 ) - a_rHalfExtents( 0 );
	TFLOAT fMinY = a_rCenter( 1 ) - a_rHalfExtents( 1 );
	TFLOAT fMinZ = a_rCenter( 2 ) - a_rHalfExtents( 2 );
	TFLOAT fMinW = a_rCenter( 3 ) - a_rHalfExtents( 3 );
	TFLOAT fMaxX = a_rCenter( 0 ) + a_rHalfExtents( 0 );
	TFLOAT fMaxY = a_rCenter( 1 ) + a_rHalfExtents( 1 );
	TFLOAT fMaxZ = a_rCenter( 2 ) + a_rHalfExtents( 2 );
	TFLOAT fMaxW = a_rCenter( 3 ) + a_rHalfExtents( 3 );

	const TVector4& rcExtents = a_pContainer->GetAABBExtents();
	const TVector4& rcCentre  = a_pContainer->GetAABBCentre();

	TVector4 vScaling( a_rScaling( 0 ), a_rScaling( 1 ), a_rScaling( 2 ), 1.0f );

	TVector4 vMin( rcCentre.x - rcExtents.x, rcCentre.y - rcExtents.y, rcCentre.z - rcExtents.z, rcCentre.w );
	vMin.Multiply( vScaling );

	TVector4 vMax( rcExtents.x + rcCentre.x, rcExtents.y + rcCentre.y, rcCentre.z + rcExtents.z, rcCentre.w );
	vMax.Multiply( vScaling );

	if ( vMin.x - a_fRadius <= fMinX ) fMinX = vMin.x - a_fRadius;
	if ( vMin.y - a_fRadius <= fMinY ) fMinY = vMin.y - a_fRadius;
	if ( vMin.z - a_fRadius <= fMinZ ) fMinZ = vMin.z - a_fRadius;
	if ( 0.0f - a_fRadius <= fMinW ) fMinW = 0.0f - a_fRadius;
	if ( fMaxX <= a_fRadius + vMax.x ) fMaxX = a_fRadius + vMax.x;
	if ( fMaxY <= a_fRadius + vMax.y ) fMaxY = a_fRadius + vMax.y;
	if ( fMaxZ <= a_fRadius + vMax.z ) fMaxZ = a_fRadius + vMax.z;
	if ( fMaxW <= a_fRadius + 0.0f ) fMaxW = a_fRadius + 0.0f;

	a_rHalfExtents.set( ( fMaxX - fMinX ) * 0.5f, ( fMaxY - fMinY ) * 0.5f, ( fMaxZ - fMinZ ) * 0.5f, ( fMaxW - fMinW ) * 0.5f );
	a_rCenter.set( ( fMaxX + fMinX ) * 0.5f, ( fMaxY + fMinY ) * 0.5f, ( fMaxZ + fMinZ ) * 0.5f, ( fMaxW + fMinW ) * 0.5f );
}

// $deBlob: FUNCTION 0072b780
void TPhysicsStandardMeshShape::addSubpart( const Subpart& a_rPart )
{
	hkpMeshShape::addSubpart( a_rPart );
	AddContainerAabb( m_pContainer, m_scaling, m_radius, m_vAabbHalfExtents, m_vAabbCenter );
}

hkUint32 TPhysicsStandardMeshShape::GetFilterInfo( const hkpMeshShape* a_pShape, hkpShapeKey a_uiKey )
{
	const hkpMeshMaterial* pMaterial = a_pShape->getMeshMaterial( a_uiKey );

	if ( pMaterial )
		return pMaterial->m_filterInfo;

	TPhysicsObjectContainer* pBody = reinterpret_cast<TPhysicsObjectContainer*>( a_pShape->getUserData() );

	if ( pBody && pBody->m_body )
		return pBody->m_body->getCollidable()->getCollisionFilterInfo();

	return 0;
}

// $deBlob: FUNCTION 0072bbf0
hkUint32 TPhysicsStandardMeshShape::getCollisionFilterInfo( hkpShapeKey a_uiKey ) const
{
	return GetFilterInfo( this, a_uiKey );
}

TOSHI_NAMESPACE_END
