#include "pch.h"
#include "ADecalManager.h"
#include "Cameras/ACamera.h"
#include "Collision/ACollisionInstance2.h"
#include "GameController/AGameStatus.h"
#include "Terrain2/ATerrain2.h"
#include "Terrain2/ATerrain2Interface.h"

#include <Math/TSphere.h>
#include <Render/TMaterial.h>
#include <Physics/TSphereData.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Platform/DX11/TSysShader_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ADecalManager::Decal ) == 0x38 );
TSTATICASSERT( sizeof( ADecalManager::DecalPoly ) == 0x80 );
TSTATICASSERT( sizeof( ADecalManager ) == 0x1E67BC );

typedef T2Vector<TVector4, 128> TriangleVector;

// The most clipped triangles of a decal triangle (MAXOUTPUTTRIS)
static TCONSTEXPR_VAR TINT MAXOUTPUTTRIS = 90;

// The shape type of the bodies decals go on (TPhysicsObjectContainer::m_eShapeType)
static TCONSTEXPR_VAR TINT16 DECAL_SHAPE_TYPE_MESH = 6;

// The collision layer of the decal sphere
static TCONSTEXPR_VAR TUINT32 DECAL_COLLISION_LAYER = 8;

// The rainbow colours of AddDecal
static TFLOAT s_fRainbowAngle      = 0.0f;
static TBOOL  s_bRainbowIncreasing = TTRUE;
static TFLOAT s_fRainbowStep       = 0.05f;

// The area error under which the triangles of a flat decal are replaced by a quad
static TFLOAT s_fQuadAreaEpsilon = 0.001f;

// The rotation of the quad of a flat decal, in units of PI
static TFLOAT s_fQuadRotation = 0.25f;

// The largest size of a stored decal
static TFLOAT s_fMaxDecalSize = 10.0f;

// The bounds of a terrain cell and its 16x16 sub cells, relative to the cell's matrix (their initialisers are marked on
// the constants)
// $deBlob: FUNCTION 0053caa0
static TCONSTEXPR_VAR TFLOAT CELL_XZ_BOUND = 150.0f;
// $deBlob: FUNCTION 0053cac0
static TCONSTEXPR_VAR TFLOAT CELL_Y_BOUND = CELL_XZ_BOUND * 6.0f;
// $deBlob: FUNCTION 0053cb00
static TCONSTEXPR_VAR TFLOAT SUBCELL_SIZE = CELL_XZ_BOUND / 15.99f;
// $deBlob: FUNCTION 0053cae0
static TCONSTEXPR_VAR TFLOAT SUBCELL_INV_SIZE   = 15.99f / CELL_XZ_BOUND;
static TCONSTEXPR_VAR TINT   sc_iSearchEdgeSize = 32;
static TCONSTEXPR_VAR TINT   RESULTS_SIZE       = sc_iSearchEdgeSize * sc_iSearchEdgeSize;

// $deBlob: FUNCTION 005f65f0
void __stdcall ADecalManager::RenderCallback( TOrderTable* a_pOrderTable, void* a_pDecalManager )
{
	ADecalManager* pDM = TSTATICCAST( ADecalManager, a_pDecalManager );
	TASSERT( pDM->m_iNumRenderCalls < 2 );

	for ( TINT i = 0; i < pDM->m_iNumRenderCalls; i++ )
		pDM->Render( &pDM->m_aRenderCalls[ i ] );

	pDM->m_iNumRenderCalls = 0;
}

// $deBlob: FUNCTION 005f4c40
TBOOL ADecalManager::WriteToMesh( TMaterial* a_pMaterial, const TVector4* a_pPos, TFLOAT a_fSize, TUINT32 a_uiColour, TINT a_iFrame, TFLOAT a_fAlpha )
{
	TASSERT( a_fAlpha >= 0.0f && a_fAlpha <= 1.0f );

	TINT iNumVerts = m_iCurNumOutVerts;

	if ( iNumVerts != 0 )
	{
		TUINT16 uiMaxNumVerts = TMath::Int32ToUInt16( iNumVerts );
		TASSERT( ( iNumVerts + uiMaxNumVerts - 1 ) / uiMaxNumVerts == 1 && "iNumMeshes == 1" );
		TASSERT( uiMaxNumVerts % 3 == 0 && "iNumVerts % 3 == 0" );

		ADecalShader::Mesh* pMesh = m_pMesh;

		if ( uiMaxNumVerts != 0 )
		{
			TINT iNumTris = ( uiMaxNumVerts - 1 ) / 3 + 1;

			for ( TINT i = 0; i < iNumTris; i++ )
			{
				ADecalShader::Vertex* pVertices = ADecalShader::Lock( pMesh );

				for ( TINT j = 0; j < 3; j++ )
				{
					pVertices[ j ].vPosition = m_avOutVerts[ i * 3 + j ].AsVector3();
					pVertices[ j ].vUV       = m_avOutUVs[ i * 3 + j ];
					pVertices[ j ].uiColour  = a_uiColour;
				}

				ADecalShader::Unlock( pMesh );

				pMesh->m_uiCurVertex += 3;

				if ( pMesh->m_uiCurVertex >= pMesh->m_uiVertexBufferSize )
					pMesh->m_uiCurVertex = 0;
			}
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005f4e20
void ADecalManager::CreateDecal( const TVector4* a_pPos, TFLOAT a_fSize, TFLOAT a_fRadius, TUINT32 a_uiColour, TMaterial* a_pMaterial, TINT a_eType )
{
	m_iCurNumOutVerts = 0;
	m_vPos            = *a_pPos;
	m_fSize           = a_fSize;
	ADecalShader::GetSingleton()->SetTexture( a_pMaterial->GetTexture( 0 ) );

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	TSphereData oSphere;
	oSphere.SetRadius( a_fRadius * 1.4142135f );

	TMatrix44 oTransform = TMatrix44::IDENTITY;
	TASSERT( m_vPos.w == 1.0f );
	oTransform.SetTranslation( m_vPos );

	TPhysicsShapeContact aContacts[ 21 ];

	// The decal world first, then the game play one
	TINT iOldWorld = pPhysicsManager->GetDefaultWorld();
	pPhysicsManager->SetDefaultWorld( APhysicsWorldType_Decal );
	TINT iNumContacts = pPhysicsManager->CollideShape( &oSphere, oTransform, DECAL_COLLISION_LAYER, aContacts, TARRAYSIZE( aContacts ), TNULL );
	pPhysicsManager->SetDefaultWorld( iOldWorld );

	if ( iNumContacts < 1 )
	{
		iOldWorld = pPhysicsManager->GetDefaultWorld();
		pPhysicsManager->SetDefaultWorld( APhysicsWorldType_GamePlay );
		iNumContacts = pPhysicsManager->CollideShape( &oSphere, oTransform, DECAL_COLLISION_LAYER, aContacts, TARRAYSIZE( aContacts ), TNULL );
		pPhysicsManager->SetDefaultWorld( iOldWorld );
	}


	if ( iNumContacts >= 1 )
		ProcessContacts( aContacts, iNumContacts, TTRUE, a_eType, a_pPos, a_fSize, a_uiColour, a_pMaterial );

	// NOTE: retail then calls an empty debug draw of the sphere
}

// $deBlob: FUNCTION 005f2a00
void ADecalManager::ProcessContacts( TPhysicsShapeContact* a_pContacts, TINT a_iNumContacts, TBOOL a_bSortByAxis, TINT a_eType, const TVector4* a_pPos, TFLOAT a_fSize, TUINT32 a_uiColour, TMaterial* a_pMaterial )
{
	static const TVector4 s_aAxes[ NUM_AXES ] = {
		TVector4::VEC_POSX,
		TVector4::VEC_POSY,
		TVector4::VEC_POSZ,
		TVector4::VEC_NEGX,
		TVector4::VEC_NEGY,
		TVector4::VEC_NEGZ,
	};

	// The triangles by the axis closest to their normal: 5 entries each (the decal's position, the normal, the
	// 3 vertices)
	TriangleVector  aTris[ NUM_AXES ];
	TriangleVector* apTris[ NUM_AXES ];

	for ( TINT i = 0; i < NUM_AXES; i++ )
		apTris[ i ] = &aTris[ i ];

	for ( TINT i = 0; i < a_iNumContacts; i++ )
	{
		TPhysicsShapeContact& rContact = a_pContacts[ i ];

		if ( rContact.m_pBody == TNULL || rContact.m_pBody->m_eShapeType != DECAL_SHAPE_TYPE_MESH )
			continue;

		TVector4 vNormal = rContact.m_vSurfaceNormal;

		// The triangle, moved a bit along its normal
		TVector4 vOffset;
		vOffset.Multiply( vNormal, 0.1f );

		TVector4 aTri[ 3 ];

		for ( TINT j = 0; j < 3; j++ )
		{
			aTri[ j ].x = rContact.m_avTriangle[ j ].x + vOffset.x;
			aTri[ j ].y = rContact.m_avTriangle[ j ].y + vOffset.y;
			aTri[ j ].z = rContact.m_avTriangle[ j ].z + vOffset.z;
			aTri[ j ].w = rContact.m_avTriangle[ j ].w;
		}

		// Its centre (the mid points of the edges averaged in turn)
		TVector4 vCentre;
		vCentre.x = ( aTri[ 1 ].x + aTri[ 0 ].x ) * 0.5f;
		vCentre.y = ( aTri[ 1 ].y + aTri[ 0 ].y ) * 0.5f;
		vCentre.z = ( aTri[ 1 ].z + aTri[ 0 ].z ) * 0.5f;

		vCentre.x = ( vCentre.x + ( aTri[ 2 ].x + aTri[ 0 ].x ) * 0.5f ) * 0.5f;
		vCentre.y = ( vCentre.y + ( aTri[ 2 ].y + aTri[ 0 ].y ) * 0.5f ) * 0.5f;
		vCentre.z = ( vCentre.z + ( aTri[ 2 ].z + aTri[ 0 ].z ) * 0.5f ) * 0.5f;

		vCentre.x = ( vCentre.x + ( aTri[ 2 ].x + aTri[ 1 ].x ) * 0.5f ) * 0.5f;
		vCentre.y = ( vCentre.y + ( aTri[ 2 ].y + aTri[ 1 ].y ) * 0.5f ) * 0.5f;
		vCentre.z = ( vCentre.z + ( aTri[ 2 ].z + aTri[ 1 ].z ) * 0.5f ) * 0.5f;

		m_vNormal.x = m_vPos.x - vCentre.x;
		m_vNormal.y = m_vPos.y - vCentre.y;
		m_vNormal.z = m_vPos.z - vCentre.z;
		m_vNormal.w = m_vPos.w;

		// Only the triangles facing the decal
		if ( m_vNormal.y * vNormal.y + vNormal.x * m_vNormal.x + m_vNormal.z * vNormal.z < 0.0f )
			continue;

		TINT iFirstVert = m_iCurNumOutVerts;

		if ( !a_bSortByAxis )
		{
			if ( m_fSize > 0.0f )
				ClipTriangle( aTri, vNormal );

			ComputeUVs( iFirstVert, m_iCurNumOutVerts, a_eType );

			if ( m_iCurNumOutVerts >= MAX_OUT_VERTS )
				return;
		}
		else
		{
			m_vNormal = vNormal;
			m_vNormal.Normalise();

			// NOTE: retail compares the contact index here, not the number of vertices
			if ( i + 3 > MAX_OUT_VERTS )
				break;

			// The axis closest to the normal
			TFLOAT fBest = TMath::Abs( m_vNormal.y * s_aAxes[ 0 ].y + m_vNormal.x * s_aAxes[ 0 ].x + m_vNormal.z * s_aAxes[ 0 ].z );
			if ( fBest <= 0.0f ) fBest = 0.0f;

			TINT iAxis = 0;

			for ( TINT j = 1; j < NUM_AXES; j++ )
			{
				TFLOAT fDot = TMath::Abs( m_vNormal.y * s_aAxes[ j ].y + s_aAxes[ j ].x * m_vNormal.x + m_vNormal.z * s_aAxes[ j ].z );

				if ( fBest < fDot )
				{
					iAxis = j;
					fBest = fDot;
				}
			}

			TriangleVector* pTris = apTris[ iAxis ];
			pTris->PushBack( m_vPos );
			pTris->PushBack( m_vNormal );
			pTris->PushBack( aTri[ 0 ] );
			pTris->PushBack( aTri[ 1 ] );
			pTris->PushBack( aTri[ 2 ] );
		}
	}

	if ( a_bSortByAxis )
		ProjectTriangles( apTris, a_pMaterial, a_pPos, a_fSize, a_uiColour, a_eType, 0 );
}

// $deBlob: FUNCTION 005f5050
void ADecalManager::SetupClipPlanes( const TVector4* a_pForward, TVector4* a_pUp, TVector4* a_pRight, TFLOAT a_fSize )
{
	TASSERT( a_pForward->IsNormalised() && "a_rvForward.IsNormalised()" );
	TASSERT( a_pUp->IsNormalised() && "a_rvUp.IsNormalised()" );
	TASSERT( a_pRight->IsNormalised() && "a_rvRight.IsNormalised()" );

	if ( m_uiFlags & Flags_Rotation )
	{
		TFLOAT fRotation = m_fRotation;

		TVector4 vRotated;
		TVector4::RotateArbitaryVector( vRotated, *a_pUp, *a_pForward, fRotation );
		*a_pUp = vRotated;
		TVector4::RotateArbitaryVector( vRotated, *a_pRight, *a_pForward, fRotation );
		*a_pRight = vRotated;

		TASSERT( 0.001f > TMath::Abs( a_pUp->y * a_pRight->y + a_pUp->x * a_pRight->x + a_pUp->z * a_pRight->z ) && "0.001f > TMath::Abs(a_rvUp.DotProduct3(a_rvRight))" );
	}

	TVector4 vNegRight( -a_pRight->x, -a_pRight->y, -a_pRight->z, a_pRight->w );
	TVector4 vNegUp( -a_pUp->x, -a_pUp->y, -a_pUp->z, a_pUp->w );
	TFLOAT   fNegSize = -a_fSize;

	// The 4 sides of the square around m_vPos, facing inwards
	TVector4 vPoint;
	vPoint.Multiply( *a_pRight, fNegSize );
	vPoint.x += m_vPos.x;
	vPoint.y += m_vPos.y;
	vPoint.z += m_vPos.z;
	m_aClipPlanes[ 0 ] = TPlane( a_pRight->x, a_pRight->y, a_pRight->z, a_pRight->x * vPoint.x + vPoint.y * a_pRight->y + a_pRight->z * vPoint.z );

	vPoint.Multiply( vNegRight, fNegSize );
	vPoint.x += m_vPos.x;
	vPoint.y += m_vPos.y;
	vPoint.z += m_vPos.z;
	m_aClipPlanes[ 1 ] = TPlane( vNegRight.x, vNegRight.y, vNegRight.z, vPoint.y * vNegRight.y + vPoint.x * vNegRight.x + vPoint.z * vNegRight.z );

	vPoint.Multiply( *a_pUp, fNegSize );
	vPoint.x += m_vPos.x;
	vPoint.y += m_vPos.y;
	vPoint.z += m_vPos.z;
	m_aClipPlanes[ 2 ] = TPlane( a_pUp->x, a_pUp->y, a_pUp->z, a_pUp->x * vPoint.x + a_pUp->y * vPoint.y + a_pUp->z * vPoint.z );

	vPoint.Multiply( vNegUp, fNegSize );
	m_aClipPlanes[ 3 ] = TPlane( vNegUp.x, vNegUp.y, vNegUp.z, ( m_vPos.y + vPoint.y ) * vNegUp.y + ( m_vPos.x + vPoint.x ) * vNegUp.x + ( m_vPos.z + vPoint.z ) * vNegUp.z );
}

// $deBlob: FUNCTION 005f3350
void ADecalManager::ProjectTriangles( TriangleVector** a_ppTris, TMaterial* a_pMaterial, const TVector4* a_pPos, TFLOAT a_fSize, TUINT32 a_uiColour, TINT a_eType, TINT a_iUnused )
{
	static const TVector4 s_aAxes[ NUM_AXES ] = {
		TVector4::VEC_POSX,
		TVector4::VEC_POSY,
		TVector4::VEC_POSZ,
		TVector4::VEC_NEGX,
		TVector4::VEC_NEGY,
		TVector4::VEC_NEGZ,
	};

	TINT iNumOutVerts = 0;

	for ( TINT iAxis = 0; iAxis < NUM_AXES; iAxis++ )
	{
		if ( a_ppTris[ iAxis ] == TNULL )
			continue;

		m_iCurNumOutVerts = 0;

		TriangleVector* pTris          = a_ppTris[ iAxis ];
		TINT            iSavedNumVerts = iNumOutVerts;
		TBOOL           bAborted       = TFALSE;

		TPlane oPlane;
		TBOOL  bCoplanar = IsCoplanar( pTris, &oPlane );

		for ( TINT i = 0; i != pTris->Size(); i += 5 )
		{
			TVector4 vUp = s_aAxes[ ( iAxis + 1 ) % 3 ];

			m_vPos    = ( *pTris )[ i ];
			m_vNormal = ( *pTris )[ i + 1 ];

			TVector4 vRight;
			vRight.CrossProduct( m_vNormal, vUp );
			vUp.CrossProduct( vRight, m_vNormal );
			vRight.Normalise();
			vUp.Normalise();
			SetupClipPlanes( &m_vNormal, &vUp, &vRight, m_fSize );

			TVector4 aTri[ 3 ];
			aTri[ 0 ] = ( *pTris )[ i + 2 ];
			aTri[ 1 ] = ( *pTris )[ i + 3 ];
			aTri[ 2 ] = ( *pTris )[ i + 4 ];

			TINT iFirstVert = m_iCurNumOutVerts;

			if ( m_fSize > 0.0f )
				ClipTriangle( aTri, m_vNormal );

			TINT iNumNewVerts = m_iCurNumOutVerts - iFirstVert;
			ComputeUVs( iFirstVert, iFirstVert + iNumNewVerts, a_eType );

			if ( m_iCurNumOutVerts > MAX_OUT_VERTS / 2 )
			{
				bAborted = TTRUE;
				break;
			}

			iNumOutVerts += iNumNewVerts;
		}

		if ( bAborted )
		{
			iNumOutVerts = iSavedNumVerts;
			continue;
		}

		if ( bCoplanar )
		{
			TASSERT( m_iCurNumOutVerts % 3 == 0 );

			// The area of the clipped triangles (twice)
			TFLOAT fArea = 0.0f;

			for ( TINT i = 0; i < m_iCurNumOutVerts; i += 3 )
			{
				TVector4 vEdge1( m_avOutVerts[ i + 1 ].x - m_avOutVerts[ i ].x, m_avOutVerts[ i + 1 ].y - m_avOutVerts[ i ].y, m_avOutVerts[ i + 1 ].z - m_avOutVerts[ i ].z, 0.0f );
				TVector4 vEdge2( m_avOutVerts[ i + 2 ].x - m_avOutVerts[ i ].x, m_avOutVerts[ i + 2 ].y - m_avOutVerts[ i ].y, m_avOutVerts[ i + 2 ].z - m_avOutVerts[ i ].z, 0.0f );

				TVector4 vCross;
				vCross.CrossProduct( vEdge1, vEdge2 );
				fArea += vCross.Magnitude();
			}

			// The whole square is on the plane: a quad instead
			TFLOAT fWidth = m_fSize * 2.0f;

			if ( TMath::Abs( fWidth * fWidth * 2.0f - fArea ) < s_fQuadAreaEpsilon )
			{
				iNumOutVerts      = iSavedNumVerts;
				m_iCurNumOutVerts = 0;

				TVector4 vNormal( oPlane.GetNormal().x, oPlane.GetNormal().y, oPlane.GetNormal().z, 1.0f );
				TVector4 vUp = s_aAxes[ ( iAxis + 1 ) % 3 ];

				TVector4 vRight;
				vRight.CrossProduct( vNormal, vUp );
				vUp.CrossProduct( vRight, vNormal );
				vRight.Normalise();
				vUp.Normalise();
				vNormal.w = 0.0f;

				m_vPos = *a_pPos;
				SetupClipPlanes( &vNormal, &vUp, &vRight, m_fSize );

				TINT iFirstVert = m_iCurNumOutVerts;

				TVector4 vRotated;
				TVector4::RotateArbitaryVector( vRotated, vRight, vNormal, s_fQuadRotation * TMath::PI );
				vRight = vRotated;

				if ( AddQuad( &oPlane, a_pPos, &vRight ) )
				{
					iNumOutVerts = iSavedNumVerts + 6;
					ComputeUVs( iFirstVert, iFirstVert + 6, a_eType );
				}
			}
		}

		if ( iNumOutVerts >= MAX_OUT_VERTS )
			break;

		if ( pTris->Size() > 0 )
			WriteToMesh( a_pMaterial, a_pPos, a_fSize, a_uiColour, m_iFrame, 1.0f );
	}

	m_iCurNumOutVerts = iNumOutVerts;
}

// $deBlob: FUNCTION 005f5470
TBOOL __stdcall ADecalManager::IsCoplanar( TriangleVector* a_pTris, TPlane* a_pOutPlane )
{
	// NOTE: retail leaves these uninitialised (only used when a_pTris is empty, then FALSE is returned anyway)
	TVector4 vNormal( 0.0f, 0.0f, 0.0f, 0.0f );
	TVector4 vPrevNormal( 0.0f, 0.0f, 0.0f, 0.0f );
	TVector4 vVertex( 0.0f, 0.0f, 0.0f, 0.0f );
	TFLOAT   fPrevD = 0.0f;
	TINT     iCount = 0;

	for ( TINT i = 0; i != a_pTris->Size(); i += 5 )
	{
		const TVector4& rPos = ( *a_pTris )[ i ];
		vNormal              = ( *a_pTris )[ i + 1 ];
		TASSERT( vNormal.IsNormalised() && "vCurNormal.IsNormalised()" );

		TFLOAT fD = rPos.y * vNormal.y + rPos.x * vNormal.x + rPos.z * vNormal.z;

		if ( iCount != 0 &&
		     ( 0.001f < TMath::Abs( fD - fPrevD ) || 0.001f < TMath::Abs( vNormal.x - vPrevNormal.x ) ||
		       0.001f < TMath::Abs( vNormal.y - vPrevNormal.y ) || 0.001f < TMath::Abs( vNormal.z - vPrevNormal.z ) ) )
			return TFALSE;

		fPrevD      = fD;
		vPrevNormal = vNormal;
		vVertex     = ( *a_pTris )[ i + 2 ];
		iCount += 5;
	}

	TASSERT( iCount == a_pTris->Size() );

	*a_pOutPlane = TPlane( vNormal.x, vNormal.y, vNormal.z, vVertex.y * vNormal.y + vVertex.x * vNormal.x + vVertex.z * vNormal.z );
	return a_pTris->Size() != 0;
}

// $deBlob: FUNCTION 005f3d00
void __stdcall ADecalManager::GetTriangleNormal( const TVector4** a_ppVerts, TVector4* a_pOutNormal )
{
	const TVector4* pV0 = a_ppVerts[ 0 ];
	const TVector4* pV1 = a_ppVerts[ 1 ];
	const TVector4* pV2 = a_ppVerts[ 2 ];

	TVector4 vEdge1( pV1->x - pV0->x, pV1->y - pV0->y, pV1->z - pV0->z, pV1->w );
	TVector4 vEdge2( pV2->x - pV0->x, pV2->y - pV0->y, pV2->z - pV0->z, pV2->w );

	a_pOutNormal->CrossProduct( vEdge2, vEdge1 );
}

// $deBlob: FUNCTION 005f5930
void ADecalManager::StoreDecal( Decal* a_pDecal, TriangleVector* a_pvTris, TUINT32 a_uiUnk60, TUINT8 a_uiUnk6C, TFLOAT a_fWidth )
{
	TASSERT( a_pvTris->Size() % 5 == 0 );

	if ( a_pvTris->Size() / 5 < 5 )
	{
		ATerrain2* pTerrain = ATerrain2::GetInstance();

		TriangleVector::Iterator rIter     = a_pvTris->Begin();
		TVector4                 vDecalPos = *rIter;

		TerrainTypes::Cell* pCell       = TNULL;
		TINT                uiCellIndex = pTerrain->GetCellIndex( ATerrain2Interface::GetInstance()->GetCurrentRegion(), &vDecalPos, &pCell );

		if ( uiCellIndex != TerrainTypes::CellVis::NONE )
		{
			DecalPoly* pStoredDecal = AllocDecalPoly();

			if ( a_pDecal->m_pStoredDecal != TNULL )
				a_pDecal->m_pStoredDecal->m_pDecal = TNULL;

			if ( pStoredDecal->m_pDecal != TNULL )
				pStoredDecal->m_pDecal->m_pStoredDecal = TNULL;

			a_pDecal->m_pStoredDecal = pStoredDecal;
			pStoredDecal->m_uiFlags &= ~0x40;
			pStoredDecal->m_pDecal    = a_pDecal;
			pStoredDecal->m_uiUnk60   = a_uiUnk60;
			pStoredDecal->m_uiUnk6C   = a_uiUnk6C;
			pStoredDecal->m_uiNumTris = 0;

			TMath::Clip( a_fWidth, 0.0f, s_fMaxDecalSize );
			pStoredDecal->m_uiSize = TUINT8( TINT( ( a_fWidth / s_fMaxDecalSize ) * 256.0f ) );
			TASSERT( 0.05 > TMath::Abs( a_fWidth - ( ( (TFLOAT)pStoredDecal->m_uiSize / 256.f ) * s_fMaxDecalSize ) ) );

			pStoredDecal->m_iFrame = m_iFrame;

			TASSERT( (TUINT32)( (TUINT8)uiCellIndex ) == uiCellIndex );
			pStoredDecal->m_uiCellIndex = TUINT8( uiCellIndex );

			TVector4 vCellPos = pCell->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
			TASSERT( ATerrain2::GetInstance()->IsPosInsideCell( pCell, &vDecalPos ) && "ATerrain2::GetInstance()->IsPosInsideCell(pCell, vDecalPos)" );
			TASSERT( ATerrain2::GetInstance()->GetCell( 0, pStoredDecal->m_uiCellIndex ) == pCell );

			CompressPos( &vCellPos, &vDecalPos, &pStoredDecal->m_uiX, &pStoredDecal->m_uiY, &pStoredDecal->m_uiZ, &pStoredDecal->m_uiSubCell );

			while ( rIter != a_pvTris->End() )
			{
				TASSERT( pStoredDecal->m_uiNumTris < DecalPoly::s_iMaxTrisPerDecal );
				StoredTri* pTri = &pStoredDecal->m_aTris[ pStoredDecal->m_uiNumTris ];

				// The normal
				++rIter;
				TVector4 vNormal = *rIter;
				TASSERT( rIter != a_pvTris->End() );
				CompressNormal( &vNormal, &pTri->m_aiNormal[ 0 ], &pTri->m_aiNormal[ 1 ], &pTri->m_aiNormal[ 2 ] );

				// The vertices
				TVector4 aVerts[ 3 ];
				++rIter;
				aVerts[ 0 ] = *rIter;
				++rIter;
				aVerts[ 1 ] = *rIter;
				++rIter;
				aVerts[ 2 ] = *rIter;
				TASSERT( rIter != a_pvTris->End() );

				TINT                uiTriVertCellIndex = TerrainTypes::CellVis::NONE;
				TerrainTypes::Cell* pTriVertCell       = TNULL;
				TBOOL               bStored            = TTRUE;

				for ( TINT i = 0; i < 3; i++ )
				{
					if ( pTriVertCell == TNULL || !pTerrain->IsPosInsideCell( pTriVertCell, &aVerts[ i ] ) )
						uiTriVertCellIndex = pTerrain->GetCellIndex( ATerrain2Interface::GetInstance()->GetCurrentRegion(), &aVerts[ i ], &pTriVertCell );

					if ( pTriVertCell == TNULL || uiTriVertCellIndex == TerrainTypes::CellVis::NONE ||
					     !pTerrain->IsPosInsideCell( pTerrain->GetCell( 0, uiTriVertCellIndex ), &aVerts[ i ] ) )
					{
						// The triangle isn't counted (the vertices before stay written)
						bStored = TFALSE;
						break;
					}

					TASSERT( pTriVertCell == ATerrain2::GetInstance()->GetCell( 0, uiTriVertCellIndex ) );
					TASSERT( ATerrain2::GetInstance()->IsPosInsideCell( ATerrain2::GetInstance()->GetCell( 0, uiTriVertCellIndex ), &aVerts[ i ] ) && "ATerrain2::GetInstance()->IsPosInsideCell(ATerrain2::GetInstance()->GetCell(0, uiTriVertCellIndex), aVerts[i])" );
					TASSERT( (TUINT32)( (TUINT8)uiTriVertCellIndex ) == uiTriVertCellIndex );

					pTri->m_auiCell[ i ] = TUINT8( uiTriVertCellIndex );

					TVector4 vTriVertCellPos = pTriVertCell->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
					CompressPos( &vTriVertCellPos, &aVerts[ i ], &pTri->m_auiX[ i ], &pTri->m_auiY[ i ], &pTri->m_auiZ[ i ], &pTri->m_auiSubCell[ i ] );
				}

				if ( bStored )
					pStoredDecal->m_uiNumTris++;

				++rIter;
			}

			m_oDecalsByPos.AddDecal( pStoredDecal );
			return;
		}
	}

	a_pDecal->m_pStoredDecal = TNULL;
}

// $deBlob: FUNCTION 005f60b0
void __stdcall ADecalManager::CompressNormal( const TVector4* a_pNormal, TINT8* a_pX, TINT8* a_pY, TINT8* a_pZ )
{
	TASSERT( a_pNormal->IsNormalised() && "a_vIn.IsNormalised()" );

	TVector4 vScaled;
	vScaled.Multiply( *a_pNormal, 127.99f );

	*a_pX = TINT8( TINT( vScaled.x ) );
	*a_pY = TINT8( TINT( vScaled.y ) );
	*a_pZ = TINT8( TINT( vScaled.z ) );
}

// $deBlob: FUNCTION 005f6130
void __stdcall ADecalManager::CompressPos( const TVector4* a_pCellPos, const TVector4* a_pPos, TUINT8* a_pX, TUINT16* a_pY, TUINT8* a_pZ, TUINT8* a_pSubCell )
{
	TVector3 vCellRelPos( a_pPos->x - a_pCellPos->x, a_pPos->y - a_pCellPos->y, a_pPos->z - a_pCellPos->z );

	TASSERT( vCellRelPos.x <= CELL_XZ_BOUND && vCellRelPos.x >= 0.f && "vCellRelPos.m_fX <= CELL_XZ_BOUND && vCellRelPos.m_fX >= 0.f" );
	TASSERT( vCellRelPos.z <= CELL_XZ_BOUND && vCellRelPos.z >= 0.f && "vCellRelPos.m_fZ <= CELL_XZ_BOUND && vCellRelPos.m_fZ >= 0.f" );
	TASSERT( vCellRelPos.y <= CELL_Y_BOUND && vCellRelPos.y >= -CELL_Y_BOUND && "vCellRelPos.m_fY <= CELL_Y_BOUND && vCellRelPos.m_fY >= -CELL_Y_BOUND" );

	TUINT8 uiSubCellZIndex = TUINT8( TINT( SUBCELL_INV_SIZE * vCellRelPos.z ) );
	TUINT8 uiSubCellXIndex = TUINT8( TINT( SUBCELL_INV_SIZE * vCellRelPos.x ) );
	TASSERT( uiSubCellXIndex < 16 && uiSubCellZIndex < 16 );

	*a_pSubCell = TUINT8( uiSubCellXIndex + uiSubCellZIndex * 16 );

	TFLOAT fSubCellXOffset = TFLOAT( fmod( vCellRelPos.x, SUBCELL_SIZE ) );
	TFLOAT fSubCellZOffset = TFLOAT( fmod( vCellRelPos.z, SUBCELL_SIZE ) );
	TASSERT( fSubCellXOffset >= 0.f && fSubCellXOffset < SUBCELL_SIZE );
	TASSERT( fSubCellZOffset >= 0.f && fSubCellZOffset < SUBCELL_SIZE );

	*a_pX = TUINT8( TINT( ( fSubCellXOffset / SUBCELL_SIZE ) * 256.0f ) );
	*a_pZ = TUINT8( TINT( ( fSubCellZOffset / SUBCELL_SIZE ) * 256.0f ) );
	*a_pY = TUINT16( TINT( ( ( CELL_Y_BOUND + vCellRelPos.y ) / ( CELL_Y_BOUND * 2.0f ) ) * 65535.9f ) );
}

// $deBlob: FUNCTION 005f6410
TBOOL __stdcall ADecalManager::IsSphereInsidePlanes( const TVector4* a_pSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes )
{
	for ( TINT i = 0; i < a_iNumPlanes; i++ )
	{
		// Not the near and far planes
		if ( i == 2 || i == 3 )
			continue;

		const TPlane& rPlane = a_pPlanes[ i ];

		if ( a_pSphere->w < ( rPlane.GetNormal().y * a_pSphere->y + rPlane.GetNormal().x * a_pSphere->x + a_pSphere->z * rPlane.GetNormal().z ) - rPlane.GetD() )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005f6480
void __stdcall ADecalManager::DecompressNormal( TINT8 a_iX, TINT8 a_iY, TINT8 a_iZ, TVector4* a_pOutNormal )
{
	a_pOutNormal->w = 0.0f;
	a_pOutNormal->x = TFLOAT( a_iX ) / 127.0f;
	a_pOutNormal->y = TFLOAT( a_iY ) / 127.0f;
	a_pOutNormal->z = TFLOAT( a_iZ ) / 127.0f;
}

// $deBlob: FUNCTION 005f64e0
TVector4* __stdcall ADecalManager::DecompressPos( const TVector4* a_pCellPos, TUINT8 a_uiSubCell, TUINT8 a_uiX, TUINT16 a_uiY, TUINT8 a_uiZ, TVector4* a_pOutPos )
{
	TFLOAT fX = ( TFLOAT( a_uiX ) * ( 1.0f / 256.0f ) ) * SUBCELL_SIZE;
	TFLOAT fZ = ( TFLOAT( a_uiZ ) * ( 1.0f / 256.0f ) ) * SUBCELL_SIZE;
	TFLOAT fY = ( ( ( TFLOAT( a_uiY ) * ( 1.0f / 65536.0f ) ) - 0.5f ) * CELL_Y_BOUND ) * 2.0f;

	TVector4 vPos;
	vPos.x = a_pCellPos->x + ( TFLOAT( TUINT8( TMath::FastMod( a_uiSubCell, 16 ) ) ) * SUBCELL_SIZE + fX );
	vPos.y = a_pCellPos->y + fY;
	vPos.z = a_pCellPos->z + ( TFLOAT( a_uiSubCell >> 4 ) * SUBCELL_SIZE + fZ );
	vPos.w = a_pCellPos->w;

	*a_pOutPos   = vPos;
	a_pOutPos->w = 1.0f;
	return a_pOutPos;
}

// $deBlob: FUNCTION 005f3da0
void ADecalManager::ClipTriangle( const TVector4* a_pTriangle, TVector4 a_vNormal )
{
	if ( m_iCurNumOutVerts + 3 > MAX_OUT_VERTS )
		return;

	TVector4 aIn[ 3 ] = { a_pTriangle[ 0 ], a_pTriangle[ 1 ], a_pTriangle[ 2 ] };

	// Trivially inside all 4 planes?
	TBOOL abInside[ 3 ] = { TTRUE, TTRUE, TTRUE };

	for ( TINT i = 0; i < NUM_CLIP_PLANES; i++ )
	{
		const TPlane& rPlane = m_aClipPlanes[ i ];
		TVector3      vPlanePoint( rPlane.GetNormal().x * rPlane.GetD(), rPlane.GetNormal().y * rPlane.GetD(), rPlane.GetNormal().z * rPlane.GetD() );

		TBOOL abVertInside[ 3 ];

		for ( TINT j = 0; j < 3; j++ )
			abVertInside[ j ] = TMath::IsPointInFrontOfPlane( rPlane.GetNormal(), vPlanePoint, aIn[ j ].AsVector3() );

		abInside[ 0 ] = abInside[ 0 ] && abVertInside[ 0 ];
		abInside[ 1 ] = abInside[ 1 ] && abVertInside[ 1 ];
		abInside[ 2 ] = abInside[ 2 ] && abVertInside[ 2 ];
	}

	if ( abInside[ 0 ] == abInside[ 1 ] && abInside[ 1 ] == abInside[ 2 ] && abInside[ 0 ] == TTRUE )
	{
		TASSERT( m_iCurNumOutVerts + 3 <= MAX_OUT_VERTS && "m_iCurNumOutVerts+3 <= s_iMaxNumOutVerts" );
		AddOutVert( a_pTriangle[ 0 ], a_vNormal );
		AddOutVert( a_pTriangle[ 1 ], a_vNormal );
		AddOutVert( a_pTriangle[ 2 ], a_vNormal );
		return;
	}

	// Clip against each plane, splitting the quads into two triangles
	TBOOL    abAlive[ MAXOUTPUTTRIS ];
	TINT8    aiFirstPlane[ MAXOUTPUTTRIS ];
	TVector4 aTris[ MAXOUTPUTTRIS ][ 3 ];

	for ( TINT i = 0; i < MAXOUTPUTTRIS; i++ )
		abAlive[ i ] = TFALSE;

	abAlive[ 0 ]      = TTRUE;
	aiFirstPlane[ 0 ] = 0;
	aTris[ 0 ][ 0 ]   = a_pTriangle[ 0 ];
	aTris[ 0 ][ 1 ]   = a_pTriangle[ 1 ];
	aTris[ 0 ][ 2 ]   = a_pTriangle[ 2 ];

	TINT iNumOutTris = 1;

	for ( TINT iTri = 0; iTri < MAXOUTPUTTRIS; iTri++ )
	{
		if ( iNumOutTris >= MAXOUTPUTTRIS || !abAlive[ iTri ] )
			break;

		for ( TINT iPlane = aiFirstPlane[ iTri ]; iPlane < NUM_CLIP_PLANES; iPlane++ )
		{
			const TPlane& rPlane = m_aClipPlanes[ iPlane ];
			TVector3      vPlanePoint( rPlane.GetNormal().x * rPlane.GetD(), rPlane.GetNormal().y * rPlane.GetD(), rPlane.GetNormal().z * rPlane.GetD() );

			TBOOL abVertInside[ 3 ];

			for ( TINT j = 0; j < 3; j++ )
				abVertInside[ j ] = TMath::IsPointInFrontOfPlane( rPlane.GetNormal(), vPlanePoint, aTris[ iTri ][ j ].AsVector3() );

			TVector4 aVerts[ 4 ];
			TINT     iNumNewVerts   = 0;
			TINT     iNumTotalVerts = 0;
			TBOOL    bFailed        = TFALSE;

			for ( TINT j = 1; j <= 3; j++ )
			{
				TBOOL bInside = abVertInside[ j - 1 ];

				if ( bInside )
					aVerts[ iNumTotalVerts++ ] = aTris[ iTri ][ j - 1 ];

				if ( bInside != abVertInside[ j % 3 ] )
				{
					TVector3 vIntersection;

					if ( !TMath::LineIntersectsPlane3D( rPlane, aTris[ iTri ][ j - 1 ].AsVector3(), aTris[ iTri ][ j % 3 ].AsVector3(), vIntersection ) )
					{
						bFailed = TTRUE;
						break;
					}

					TASSERT( iNumNewVerts < 2 );
					aVerts[ iNumTotalVerts++ ].Set( vIntersection.x, vIntersection.y, vIntersection.z, 1.0f );
					iNumNewVerts++;
				}
			}

			if ( bFailed || iNumNewVerts < 1 )
			{
				if ( iNumTotalVerts < 3 )
				{
					// Completely outside
					abAlive[ iTri ] = TFALSE;
					break;
				}
			}
			else
			{
				TASSERT( iNumTotalVerts >= 3 );

				if ( iNumTotalVerts == 3 )
				{
					aTris[ iTri ][ 0 ] = aVerts[ 0 ];
					abAlive[ iTri ]    = TTRUE;
					aTris[ iTri ][ 1 ] = aVerts[ 1 ];
					aTris[ iTri ][ 2 ] = aVerts[ 2 ];
				}
				else
				{
					TASSERT( iNumTotalVerts == 4 );
					TASSERT( iNumOutTris < MAXOUTPUTTRIS );

					aTris[ iNumOutTris ][ 1 ] = aVerts[ 1 ];
					abAlive[ iNumOutTris ]    = TTRUE;
					aTris[ iNumOutTris ][ 0 ] = aVerts[ 0 ];
					aTris[ iNumOutTris ][ 2 ] = aVerts[ 2 ];

					aTris[ iTri ][ 1 ] = aVerts[ 2 ];
					aTris[ iTri ][ 0 ] = aVerts[ 0 ];
					aTris[ iTri ][ 2 ] = aVerts[ 3 ];

					// The new triangle still goes through the remaining planes
					aiFirstPlane[ iNumOutTris ] = TMath::Int32ToInt8( iPlane + 1 );
					iNumOutTris++;

					if ( iNumOutTris >= MAXOUTPUTTRIS )
						break;
				}
			}
		}
	}

	for ( TINT i = 0; i < MAXOUTPUTTRIS; i++ )
	{
		if ( !abAlive[ i ] )
			continue;

		if ( m_iCurNumOutVerts + 3 > MAX_OUT_VERTS )
			break;

		AddOutVert( aTris[ i ][ 0 ], a_vNormal );
		AddOutVert( aTris[ i ][ 1 ], a_vNormal );
		AddOutVert( aTris[ i ][ 2 ], a_vNormal );
	}
}

// $deBlob: FUNCTION 005f4620
TBOOL ADecalManager::AddQuad( const TPlane* a_pPlane, const TVector4* a_pPos, const TVector4* a_pDir )
{
	if ( m_iCurNumOutVerts + 6 > MAX_OUT_VERTS )
		return TFALSE;

	const TVector3& rNormal = a_pPlane->GetNormal();

	// Where the line through a_pPos along the normal hits the plane
	TVector3 vStart( rNormal.x * 5.0f + a_pPos->x, rNormal.y * 5.0f + a_pPos->y, rNormal.z * 5.0f + a_pPos->z );
	TVector3 vEnd( rNormal.x * -10.0f + vStart.x, rNormal.y * -10.0f + vStart.y, rNormal.z * -10.0f + vStart.z );
	TVector3 vHit;

	if ( !TMath::LineIntersectsPlane3D( *a_pPlane, vStart, vEnd, vHit ) )
		return TFALSE;

	TVector3 vDir( a_pDir->x, a_pDir->y, a_pDir->z );
	vDir.Normalize();

	TFLOAT fHalfDiagonal = m_fSize * 1.4142135f;
	vDir.x *= fHalfDiagonal;
	vDir.y *= fHalfDiagonal;
	vDir.z *= fHalfDiagonal;

	TVector4 vAxis( vHit.x - vStart.x, vHit.y - vStart.y, vHit.z - vStart.z, 0.0f );
	vAxis.Normalise();

	TVector4 vNormal( rNormal.x, rNormal.y, rNormal.z, 0.0f );
	TVector4 vCorner( vDir.x, vDir.y, vDir.z, 0.0f );

	// The 4 corners: 0, 1, 2 and 2, 3, 0
	TVector4 vVert( vCorner.x + vHit.x, vCorner.y + vHit.y, vCorner.z + vHit.z, 1.0f );
	AddOutVert( vVert, vNormal );

	TVector4 vRotated;
	TVector4::RotateArbitaryVector( vRotated, vCorner, vAxis, TMath::HALF_PI );
	vVert.Set( vRotated.x + vHit.x, vRotated.y + vHit.y, vRotated.z + vHit.z, 1.0f );
	AddOutVert( vVert, vNormal );

	TVector4::RotateArbitaryVector( vRotated, vCorner, vAxis, TMath::PI );
	vVert.Set( vRotated.x + vHit.x, vRotated.y + vHit.y, vRotated.z + vHit.z, 1.0f );
	AddOutVert( vVert, vNormal );
	AddOutVert( vVert, vNormal );

	TVector4::RotateArbitaryVector( vRotated, vCorner, vAxis, 4.712389f );
	vVert.Set( vRotated.x + vHit.x, vRotated.y + vHit.y, vRotated.z + vHit.z, 1.0f );
	AddOutVert( vVert, vNormal );

	TVector4::RotateArbitaryVector( vRotated, vCorner, vAxis, TMath::TWO_PI );
	vVert.Set( vRotated.x + vHit.x, vRotated.y + vHit.y, vRotated.z + vHit.z, 1.0f );
	AddOutVert( vVert, vNormal );

	return TTRUE;
}

// $deBlob: FUNCTION 005f4aa0
void __stdcall ADecalManager::ClassifyTriangle( const TPlane* a_pPlane, TBOOL* a_pOutInFront, const TVector4* a_pVerts )
{
	a_pOutInFront[ 0 ] = TTRUE;
	a_pOutInFront[ 1 ] = TTRUE;
	a_pOutInFront[ 2 ] = TTRUE;

	TVector3 vPlanePoint( a_pPlane->GetNormal().x * a_pPlane->GetD(), a_pPlane->GetNormal().y * a_pPlane->GetD(), a_pPlane->GetNormal().z * a_pPlane->GetD() );

	for ( TINT i = 0; i < 3; i++ )
		a_pOutInFront[ i ] = TMath::IsPointInFrontOfPlane( a_pPlane->GetNormal(), vPlanePoint, a_pVerts[ i ].AsVector3() );
}

// $deBlob: FUNCTION 005f4b20
void ADecalManager::ComputeUVs( TINT a_iStart, TINT a_iEnd, TINT a_eType )
{
	// The quarter of the texture
	TFLOAT fUOffset = 0.0f;
	TFLOAT fVOffset = 0.0f;

	if ( a_eType == Type_TopRight )
	{
		fUOffset = 0.5f;
	}
	else if ( a_eType == Type_BottomLeft )
	{
		fVOffset = 0.5f;
	}
	else if ( a_eType == Type_BottomRight )
	{
		fUOffset = 0.5f;
		fVOffset = 0.5f;
	}

	if ( a_iStart < a_iEnd )
	{
		TFLOAT fScale = ( 1.0f / ( m_fSize * 2.0f ) ) * 0.5f;

		for ( TINT i = a_iStart; i < a_iEnd; i++ )
		{
			const TVector4& rVert   = m_avOutVerts[ i ];
			const TPlane&   rPlaneU = m_aClipPlanes[ 0 ];
			const TPlane&   rPlaneV = m_aClipPlanes[ 2 ];

			TFLOAT fU = ( rPlaneU.GetNormal().y * rVert.y + rPlaneU.GetNormal().x * rVert.x + rVert.z * rPlaneU.GetNormal().z ) - rPlaneU.GetD();
			TFLOAT fV = ( rPlaneV.GetNormal().x * rVert.x + rPlaneV.GetNormal().y * rVert.y + rPlaneV.GetNormal().z * rVert.z ) - rPlaneV.GetD();

			m_avOutUVs[ i ].x = fU * fScale + fUOffset;
			m_avOutUVs[ i ].y = fV * fScale + fVOffset;
		}
	}
}

// $deBlob: FUNCTION 005f2880
void ADecalManager::AddRenderCall( TFLOAT a_fDeltaTime, ACamera* a_pCamera )
{
	if ( m_bEnabled )
	{
		TASSERT( m_iNumRenderCalls < MAX_RENDER_CALLS );
		m_aRenderCalls[ m_iNumRenderCalls ].m_pCamera    = a_pCamera;
		m_aRenderCalls[ m_iNumRenderCalls ].m_fDeltaTime = a_fDeltaTime;
		m_iNumRenderCalls++;
	}
}

// $deBlob: FUNCTION 005f2900
void ADecalManager::Render( const RenderCall* a_pRenderCall )
{
	m_pCamera = a_pRenderCall->m_pCamera;

	if ( ATerrain2::GetInstance() != TNULL && ATerrain2Interface::GetInstance() != TNULL )
	{
		m_iFrame++;

		for ( TINT i = 0; i < m_iNumPendingDecals; i++ )
			CreateDecal( &m_avPendingPos[ i ], m_afPendingSize[ i ], m_afPendingRadius[ i ], m_auiPendingColour[ i ], m_apPendingMaterial[ i ], m_aePendingType[ i ] );

		m_iNumPendingDecals = 0;

		if ( ADecalShader::IsValid( m_pMesh ) )
		{
			ADecalShader* pShader = ADecalShader::GetSingleton();
			pShader->PreRender();
			pShader->Render( m_pMesh );
			pShader->PostRender();
		}
	}
}

// $deBlob: FUNCTION 005f29d0
void ADecalManager::Reset()
{
	ADecalShader* pShader = ADecalShader::GetSingleton();
	ADecalShader::ResetMesh( m_pMesh );
	pShader->SetTexture( TTextureManager::GetSingleton()->GetInvalidTexture() );
	m_iNumPendingDecals = 0;
}

// $deBlob: FUNCTION 005f26f0
TBOOL ADecalManager::AddDecal( const TVector4* a_pPos, TFLOAT a_fSize, TFLOAT a_fRadius, TUINT32 a_uiColour, TMaterial* a_pMaterial, TINT a_eType )
{
	if ( m_iNumPendingDecals >= MAX_PENDING_DECALS )
		return TFALSE;

	m_avPendingPos[ m_iNumPendingDecals ]      = *a_pPos;
	m_afPendingSize[ m_iNumPendingDecals ]     = a_fSize;
	m_afPendingRadius[ m_iNumPendingDecals ]   = a_fRadius;
	m_apPendingMaterial[ m_iNumPendingDecals ] = a_pMaterial;
	m_aePendingType[ m_iNumPendingDecals ]     = a_eType;

	if ( !( m_uiFlags & Flags_Rainbow ) )
	{
		m_auiPendingColour[ m_iNumPendingDecals ] = a_uiColour;
		m_iNumPendingDecals++;
		return TTRUE;
	}

	// Rainbow: the colour goes back and forth around the colour wheel
	if ( !s_bRainbowIncreasing )
	{
		s_fRainbowAngle -= s_fRainbowStep;

		if ( s_fRainbowAngle < 0.0f )
			s_bRainbowIncreasing = TTRUE;
	}
	else
	{
		s_fRainbowAngle += s_fRainbowStep;

		if ( TMath::TWO_PI < s_fRainbowAngle )
			s_bRainbowIncreasing = TFALSE;
	}

	TFLOAT fSin, fCos;
	TMath::SinCos( s_fRainbowAngle, fSin, fCos );

	TFLOAT fRed = ( fSin + 1.0f ) * 0.5f;

	m_auiPendingColour[ m_iNumPendingDecals ] =
	    ( ( ( TINT( fRed * 255.0f ) | 0xFF00U ) << 8 | TINT( ( fCos + 1.0f ) * 0.5f * 255.0f ) ) << 8 ) | TINT( ( 1.0f - fRed ) * 255.0f );
	m_iNumPendingDecals++;
	return TTRUE;
}

// $deBlob: FUNCTION 005f22e0
ADecalManager::ADecalManager()
{
	m_oDecalsByPos.m_iLastCell       = 0;
	m_oDecalsByPos.m_iLastSearchCell = 0;
	m_oDecalsByPos.Validate();

	m_iCurNumOutVerts   = 0;
	m_uiUnkB74          = 0;
	m_iNumPendingDecals = 0;
	m_fRotation         = 0.0f;
	m_fUnk1E6778        = 1.0f;
	m_uiUnkA6C          = 0;
	m_uiFlags           = 0;
	m_pCamera           = TNULL;
	m_iFrame            = 0;
	m_uiUnk182760       = 0;
	m_uiUnk182764       = 0;
	m_iNumRenderCalls   = 0;
	m_pMesh             = TNULL;
	m_iMaxDecals        = MAX_DECALS;
	m_bEnabled          = TFALSE;
}

// $deBlob: FUNCTION 005f1c40
T2DList<ADecalManager::DecalPoly>* ADecalManager::DecalsByPos::GetDecals( const TVector4* a_pPos )
{
	ATerrain2*          pTerrain = ATerrain2::GetInstance();
	TerrainTypes::Cell* pCell    = pTerrain->GetCell( 0, m_iLastCell );

	if ( !pTerrain->IsPosInsideCell( pCell, a_pPos ) )
		m_iLastCell = pTerrain->GetCellIndex( ATerrain2Interface::GetInstance()->GetCurrentRegion(), a_pPos, &pCell );

	if ( m_iLastCell == TerrainTypes::CellVis::NONE )
	{
		m_iLastCell = 0;
		return TNULL;
	}

	TASSERT( ATerrain2::GetInstance()->IsPosInsideCell( pCell, a_pPos ) && "ATerrain2::GetInstance()->IsPosInsideCell(pCell, a_vPos)" );

	const TVector4& rCellPos = pCell->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION );
	TVector3        vCellRelPos( a_pPos->x - rCellPos.x, a_pPos->y - rCellPos.y, a_pPos->z - rCellPos.z );

	TASSERT( vCellRelPos.x <= CELL_XZ_BOUND && vCellRelPos.x >= 0.f && "vCellRelPos.m_fX <= CELL_XZ_BOUND && vCellRelPos.m_fX >= 0.f" );
	TASSERT( vCellRelPos.z <= CELL_XZ_BOUND && vCellRelPos.z >= 0.f && "vCellRelPos.m_fZ <= CELL_XZ_BOUND && vCellRelPos.m_fZ >= 0.f" );
	TASSERT( vCellRelPos.y <= CELL_Y_BOUND && vCellRelPos.y >= -CELL_Y_BOUND && "vCellRelPos.m_fY <= CELL_Y_BOUND && vCellRelPos.m_fY >= -CELL_Y_BOUND" );

	TUINT8 uiSubCellZIndex = TUINT8( TINT( SUBCELL_INV_SIZE * vCellRelPos.z ) );
	TUINT8 uiSubCellXIndex = TUINT8( TINT( SUBCELL_INV_SIZE * vCellRelPos.x ) );
	TASSERT( uiSubCellXIndex < 16 && uiSubCellZIndex < 16 );

	TUINT32 iIndex = ( m_iLastCell * 16 + uiSubCellZIndex ) * 16 + uiSubCellXIndex;
	TASSERT( iIndex < TARRAYSIZE( m_aDecalsByPos ) && "iIndex < TNumOf(m_aDecalsByPos)" );

	return &m_aDecalsByPos[ iIndex ];
}

// $deBlob: FUNCTION 005f1ec0
TBOOL ADecalManager::DecalsByPos::FindDecals( ACamera* a_pCamera, T2DList<DecalPoly>** a_ppResults, TBOOL a_bCull )
{
	static const TFLOAT   sc_fSearchSize       = SUBCELL_SIZE * 16.0f;
	static const TFLOAT   sc_fSubCellDiagonal  = TMath::Sqrt( SUBCELL_SIZE * 2.0f * SUBCELL_SIZE );
	static const TVector4 sc_vSearchHalfExtent = TVector4( sc_fSearchSize, 0.0f, sc_fSearchSize, 1.0f );

	ATerrain2*          pTerrain = ATerrain2::GetInstance();
	TerrainTypes::Cell* pCell    = pTerrain->GetCell( 0, m_iLastSearchCell );
	TVector4            vCamPos  = a_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

	if ( !pTerrain->IsPosInsideCell( pCell, &vCamPos ) )
		m_iLastSearchCell = pTerrain->GetCellIndex( ATerrain2Interface::GetInstance()->GetCurrentRegion(), &vCamPos, &pCell );

	if ( m_iLastSearchCell == TerrainTypes::CellVis::NONE )
	{
		m_iLastSearchCell = 0;
		return TFALSE;
	}

	TFLOAT fStartX = vCamPos.x - sc_vSearchHalfExtent.x;
	TFLOAT fStartZ = vCamPos.z - sc_vSearchHalfExtent.z;
	TFLOAT fStartY = ( pCell->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION ).y - sc_vSearchHalfExtent.y ) + 0.0f;

	const TPlane* pViewPlanes = a_pCamera->GetViewPlanes();
	TBOOL         bFound      = TFALSE;

	for ( TINT i = 0; i < sc_iSearchEdgeSize; i++ )
	{
		for ( TINT j = 0; j < sc_iSearchEdgeSize; j++, a_ppResults++ )
		{
			TSphere vCurSearchPos( TFLOAT( i ) * SUBCELL_SIZE + fStartX, fStartY, TFLOAT( j ) * SUBCELL_SIZE + fStartZ, sc_fSubCellDiagonal );
			TASSERT( sc_fSubCellDiagonal == vCurSearchPos.GetRadius() && "sc_fSubCellDiagonal == vCurSearchPos.m_fRadius" );
			TASSERT( ( i * sc_iSearchEdgeSize + j ) < RESULTS_SIZE );

			if ( a_bCull )
			{
				TBOOL bCulled = TFALSE;

				for ( TINT k = 0; k < 6; k++ )
				{
					// Not the near and far planes
					if ( k == 2 || k == 3 )
						continue;

					const TPlane& rPlane = pViewPlanes[ k ];

					if ( sc_fSubCellDiagonal < ( rPlane.GetNormal().y * vCurSearchPos.GetOrigin().y + rPlane.GetNormal().x * vCurSearchPos.GetOrigin().x + rPlane.GetNormal().z * vCurSearchPos.GetOrigin().z ) - rPlane.GetD() )
					{
						bCulled = TTRUE;
						break;
					}
				}

				if ( bCulled )
				{
					*a_ppResults = TNULL;
					continue;
				}
			}

			*a_ppResults = GetDecals( &vCurSearchPos.AsVector4() );
			bFound       = TTRUE;
		}
	}

	return bFound;
}

// $deBlob: FUNCTION 005f2220
void ADecalManager::DecalsByPos::Validate()
{
	for ( TINT i = 0; i < NUM_DECALS_BY_POS; i++ )
	{
		for ( auto it = m_aDecalsByPos[ i ].Begin(); it != m_aDecalsByPos[ i ].End(); it++ )
		{
			TASSERT( TFALSE == m_aDecalsByPos[ i ].IsEmpty() );
		}
	}
}

// $deBlob: FUNCTION 005f2480
// $deBlob: FUNCTION 0056bf50
ADecalManager::~ADecalManager()
{
}

// $deBlob: FUNCTION 005f2560
void ADecalManager::Initialise()
{
	m_bEnabled   = TTRUE;
	m_iMaxDecals = ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE ) ? MAX_DECALS : 2000;

	ADecalShader* pShader = new ADecalShader( m_iMaxDecals * 9 );

	TSysShaderHAL* pSysShader = TSTATICCAST( TSysShaderHAL, TSysShader::GetSingleton() );

	m_pMesh = pShader->CreateMesh( m_iMaxDecals * 9, 0 );
	pShader->ValidateMesh( m_pMesh );

	pSysShader->GetOrderTable( 0 )->SetPreFlushCallback( RenderCallback, this );
}

// $deBlob: FUNCTION 005f2680
void ADecalManager::Deinitialise()
{
	m_bEnabled = TFALSE;
	m_llDecals.Clear();

	for ( TINT i = 0; i < MAX_DECALS; i++ )
	{
		ADecalShader::DestroyMesh( m_aDecals[ i ].m_pMesh );
		m_aDecals[ i ].m_pMesh = TNULL;
	}

	ADecalShader::DestroyMesh( m_pMesh );
	m_pMesh = TNULL;

	if ( ADecalShader::GetSingleton() != TNULL )
		delete ADecalShader::GetSingleton();
}

// $deBlob: FUNCTION 005f7280
TBOOL ADecalManager::IsBackFacing( const TVector4* a_pNormal ) const
{
	return m_vNormal.y * a_pNormal->y + m_vNormal.x * a_pNormal->x + m_vNormal.z * a_pNormal->z < 0.0f;
}
