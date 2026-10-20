#include "ToshiPCH.h"
#include "Render/TWorldVIS.h"
#include "Render/TRender.h"
#include "Render/TRenderContext.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TWorldVis::StackValue* TWorldVis::s_pStack;
TVector4               TWorldVis::avScreen[ 8 ];

// $deBlob: FUNCTION 006ac150
void Frustum::Transform( const Frustum& a_rSource, const TMatrix44& a_rMatrix )
{
	for ( TINT i = 0; i < 6; i++ )
	{
		TMatrix44::TransformPlaneOrthogonal( m_aPlanes[ i ].m_Plane, a_rMatrix, a_rSource.m_aPlanes[ i ].m_Plane );
		m_aPlanes[ i ].m_uiClipFlag = a_rSource.m_aPlanes[ i ].m_uiClipFlag;
	}
}

// $deBlob: FUNCTION 006ac210
void Frustum::InitReduce()
{
	m_iActivePlaneCount = 6;

	for ( TINT i = 0; i < m_iActivePlaneCount; i++ )
		m_apPlanes[ i ] = &m_aPlanes[ i ];
}

// $deBlob: FUNCTION 006ac2d0
TINT Frustum::IntersectSphere( const TSphere& a_rSphere )
{
	TASSERT( ( TREINTERPRETCAST( TUINTPTR, &a_rSphere ) & 0xF ) == 0 );

	// NOTE: retail steps through m_aPlanes by sizeof( TPlane ) (0x10) while they are 0x14 bytes apart, so the planes
	// after the first are read from the wrong place
	const TPlane* pPlane    = &m_aPlanes[ 0 ].m_Plane;
	TUINT32       uiCrossed = 0;

	for ( TINT i = 0; i < 6; i++, pPlane++ )
	{
		TASSERT( ( TREINTERPRETCAST( TUINTPTR, pPlane ) & 0xF ) == 0 );

		TFLOAT fDist = pPlane->GetNormal().y * a_rSphere.GetOrigin().y + pPlane->GetNormal().x * a_rSphere.GetOrigin().x + pPlane->GetNormal().z * a_rSphere.GetOrigin().z - pPlane->GetD();

		if ( fDist > a_rSphere.GetRadius() )
			return SphereTest_Outside;

		if ( a_rSphere.GetRadius() > -fDist )
			uiCrossed |= 1 << i;
	}

	return ( uiCrossed != 0 ) ? SphereTest_Intersecting : SphereTest_Inside;
}

// $deBlob: FUNCTION 006ac3e0
TINT Frustum::IntersectSphereReduce( const TSphere& a_rSphere )
{
	TINT i = 0;

	while ( i < m_iActivePlaneCount )
	{
		const TPlane& rPlane = m_apPlanes[ i ]->m_Plane;
		TFLOAT        fDist  = TVector3::DotProduct( rPlane.GetNormal(), a_rSphere.GetOrigin() ) - rPlane.GetD();

		if ( a_rSphere.GetRadius() < fDist )
			return SphereTest_Outside;

		if ( a_rSphere.GetRadius() <= -fDist )
		{
			// Fully inside of this plane: deactivate it (swap it with the last active one)
			TASSERT( m_iActivePlaneCount > 0 );
			TASSERT( i >= 0 && i <= 6 );
			TASSERT( m_iActivePlaneCount >= 0 && m_iActivePlaneCount <= 6 );

			m_iActivePlaneCount--;
			ClipPlane* pLast                     = m_apPlanes[ m_iActivePlaneCount ];
			m_apPlanes[ m_iActivePlaneCount ] = m_apPlanes[ i ];
			m_apPlanes[ i ]                      = pLast;
		}
		else
		{
			i++;
		}
	}

	return ( m_iActivePlaneCount != 0 ) ? SphereTest_Intersecting : SphereTest_Inside;
}

// $deBlob: FUNCTION 006ac680
TUINT32 Frustum::GetActiveClipFlags()
{
	TUINT32 uiFlags = 0;

	for ( TINT i = 0; i < m_iActivePlaneCount; i++ )
	{
		TASSERT( m_apPlanes[ i ] >= &m_aPlanes[ 0 ] && m_apPlanes[ i ] <= &m_aPlanes[ 5 ] );
		uiFlags |= m_apPlanes[ i ]->m_uiClipFlag;
	}

	return uiFlags;
}

// $deBlob: FUNCTION 006ac7c0
TWorldVis::TWorldVis()
{
	m_pWorld                  = TNULL;
	m_pCellNodes              = TNULL;
	m_iNumBuiltCells          = 0;
	m_iNumCellSettings        = 0;
	m_pfnRenderCallback       = TNULL;
	m_pfnRenderCallbackUser   = TNULL;
	m_pRenderCallbackUserData = TNULL;
	m_uiFlags                 = 0;
	m_oTransform              = TMatrix44::IDENTITY;
}

// $deBlob: FUNCTION 006ac830
// $deBlob: FUNCTION 0061d9c0
TWorldVis::~TWorldVis()
{
	Reset();

	if ( m_pCellNodes )
	{
		operator delete( m_pCellNodes );
		m_pCellNodes = TNULL;
	}
}

// $deBlob: FUNCTION 006ac860
void TWorldVis::Create( TTMDWin::TRBWorldCellGroup* a_pWorld, const TMatrix44* a_pTransform )
{
	TASSERT( TNULL == m_pWorld );
	m_pWorld     = a_pWorld;
	m_pCellNodes = TSTATICCAST( CellNode, TMemalign( 16, a_pWorld->m_iNumCells * sizeof( CellNode ) ) );
	Reset();

	if ( a_pTransform != TNULL )
	{
		m_oTransform = *a_pTransform;
		m_uiFlags |= Flags_Transform;
	}
}

// $deBlob: FUNCTION 006ac900
void TWorldVis::Destroy()
{
	Reset();

	if ( m_pCellNodes )
	{
		operator delete( m_pCellNodes );
		m_pCellNodes = TNULL;
	}
}

// $deBlob: FUNCTION 006ac930
void TWorldVis::Reset()
{
	for ( TINT i = 0; i < m_iNumBuiltCells; i++ )
		m_pWorld->m_ppCells[ m_pCellNodes[ i ].m_iCellIndex ]->m_pVisNode = TNULL;

	m_iNumBuiltCells   = 0;
	m_iNumCellSettings = 0;
}

// $deBlob: FUNCTION 006ac980
void TWorldVis::Build( const TMatrix44& a_rModelView, const TMatrix44& a_rProjection, TINT a_iCellIndex, TBOOL a_bInvert )
{
	TMatrix44 mViewModel = a_rModelView;

	if ( !a_bInvert )
		mViewModel.InvertOrthonormal();
	else
		mViewModel.Invert( mViewModel );

	m_ViewModelPos = mViewModel.GetTranslation3();

	CullBox oCullBox;
	oCullBox.m_fMinX = 0.0f;
	oCullBox.m_fMinY = 0.0f;
	oCullBox.m_fMaxX = TFLOAT( TRender::GetSingleton()->GetCurrentDisplayParams()->uiWidth );
	oCullBox.m_fMaxY = TFLOAT( TRender::GetSingleton()->GetCurrentDisplayParams()->uiHeight );

	Build( a_rModelView, a_rProjection, a_iCellIndex, &oCullBox );
}

// $deBlob: FUNCTION 006aca40
void TWorldVis::Build( const TMatrix44& a_rModelView, const TMatrix44& a_rProjection, TINT a_iCellIndex, const CullBox* a_pCullBox )
{
	TTMDWin::TRBWorldCell* pCell = m_pWorld->m_ppCells[ a_iCellIndex ];

	if ( pCell->m_uiFlags & TTMDWin::TRBWorldCell::Flags_Building )
		return;

	pCell->m_uiFlags |= TTMDWin::TRBWorldCell::Flags_Building;

	TRenderContext* pRenderContext = TRender::GetSingleton()->GetCurrentContext();
	TFLOAT          fWidth         = pRenderContext->GetWidth();
	TFLOAT          fHeight        = pRenderContext->GetHeight();

	CellNode* pNode = TSTATICCAST( CellNode, pCell->m_pVisNode );

	if ( pNode == TNULL )
	{
		pNode               = &m_pCellNodes[ m_iNumBuiltCells++ ];
		pCell->m_pVisNode   = pNode;
		pNode->m_iCellIndex = a_iCellIndex;
		pNode->m_pSettings  = TNULL;
	}

	TINT iSettings = m_iNumCellSettings++;
	TASSERT( iSettings >= 0 && iSettings < MAX_VISIBLE_CELLS );

	CellSettings* pSettings = &m_aCellSettings[ iSettings ];
	pSettings->m_pNext      = pNode->m_pSettings;
	pNode->m_pSettings      = pSettings;
	pSettings->m_CullBox    = *a_pCullBox;

	for ( TINT i = 0; i < pCell->m_iNumPortals; i++ )
	{
		TTMDWin::TRBWorldPortal* pPortal = pCell->m_ppPortals[ i ];

		// Only the portals facing the camera
		TVector3 vToCamera = m_ViewModelPos - pPortal->m_pVerts[ 0 ];
		if ( TVector3::DotProduct( pPortal->m_Normal, vToCamera ) < 0.0f )
			continue;

		TFLOAT fMinX       = TMath::MAXFLOAT;
		TFLOAT fMinY       = TMath::MAXFLOAT;
		TFLOAT fMaxX       = -TMath::MAXFLOAT;
		TFLOAT fMaxY       = -TMath::MAXFLOAT;
		TBOOL  bAllBehind  = TTRUE;
		TFLOAT fHalfWidth  = fWidth * 0.5f;
		TFLOAT fHalfHeight = fHeight * 0.5f;

		for ( TINT k = 0; k < pPortal->m_iNumVerts; k++ )
		{
			TVector3 vView;
			TMatrix44::TransformVector( vView, a_rModelView, pPortal->m_pVerts[ k ] );

			if ( vView.z >= 0.1f )
				bAllBehind = TFALSE;
			else
				vView.z = 0.1f;

			TVector4 vProj;
			TMatrix44::TransformVector( vProj, a_rProjection, TVector4( vView.x, vView.y, vView.z, 1.0f ) );

			TVector4& rScreen = avScreen[ k ];
			rScreen.z         = 0.0f;
			rScreen.x         = ( vProj.x / vProj.w ) * fHalfWidth + fHalfWidth;
			rScreen.y         = fHalfHeight - ( vProj.y / vProj.w ) * fHalfHeight;

			fMinX = TMath::Min( fMinX, rScreen.x );
			fMinY = TMath::Min( fMinY, rScreen.y );
			fMaxX = TMath::Max( fMaxX, rScreen.x );
			fMaxY = TMath::Max( fMaxY, rScreen.y );
		}

		if ( !bAllBehind && fMinX <= a_pCullBox->m_fMaxX && a_pCullBox->m_fMinX <= fMaxX && fMinY <= a_pCullBox->m_fMaxY && a_pCullBox->m_fMinY <= fMaxY )
		{
			// The part of the cull box the portal covers
			CullBox oPortalBox;
			oPortalBox.m_fMinX = ( fMinX < a_pCullBox->m_fMinX ) ? a_pCullBox->m_fMinX : fMinX;
			oPortalBox.m_fMaxX = ( fMaxX > a_pCullBox->m_fMaxX ) ? a_pCullBox->m_fMaxX : fMaxX;
			oPortalBox.m_fMinY = ( fMinY < a_pCullBox->m_fMinY ) ? a_pCullBox->m_fMinY : fMinY;
			oPortalBox.m_fMaxY = ( fMaxY > a_pCullBox->m_fMaxY ) ? a_pCullBox->m_fMaxY : fMaxY;

			for ( TINT k = 0; k < pPortal->m_iNumCells; k++ )
				Build( a_rModelView, a_rProjection, pPortal->m_piCells[ k ], &oPortalBox );
		}
	}

	pCell->m_uiFlags &= ~TTMDWin::TRBWorldCell::Flags_Building;
}

// $deBlob: FUNCTION 006ace40
void TWorldVis::Render( const TMatrix44& a_rModelView, const TMatrix44& a_rProjection, TBOOL a_bInvert )
{
	StackValue aStack[ s_iStackSize ];
	s_pStack = aStack;

	if ( m_pfnRenderCallback == TNULL && m_pfnRenderCallbackUser == TNULL )
		return;

	TRender::GetSingleton()->GetCurrentContext();

	TMatrix44 mViewModel = a_rModelView;

	if ( !a_bInvert )
		mViewModel.InvertOrthonormal();
	else
		mViewModel.Invert( mViewModel );

	for ( TINT i = 0; i < m_iNumBuiltCells; i++ )
	{
		TTMDWin::TRBWorldCell* pCell = m_pWorld->m_ppCells[ m_pCellNodes[ i ].m_iCellIndex ];

		for ( CellSettings* pSettings = TSTATICCAST( CellNode, pCell->m_pVisNode )->m_pSettings; pSettings != TNULL; pSettings = pSettings->m_pNext )
		{
			// The frustum planes of the render context in model space
			Frustum oViewFrustum;
			Frustum oModelFrustum;

			const TPlane* pFrustumPlanes = TRender::GetSingleton()->GetCurrentContext()->GetFrustumPlanes();

			for ( TINT k = 0; k < 6; k++ )
			{
				oViewFrustum.m_aPlanes[ k ].m_Plane      = pFrustumPlanes[ k ];
				oViewFrustum.m_aPlanes[ k ].m_uiClipFlag = 1 << k;
			}

			oModelFrustum.Transform( oViewFrustum, mViewModel );

			RenderData oRenderData;
			oRenderData.m_pCell    = pCell;
			oRenderData.m_pFrustum = &oModelFrustum;

			oModelFrustum.InitReduce();
			RenderTreeIntersectNonRecurse( pCell->m_pTreeRoot, &oRenderData );
		}
	}
}

// $deBlob: FUNCTION 006ad5b0
void TWorldVis::TransformFrustums( const TMatrix44& a_rModelView, TINT a_iUnused )
{
	TMatrix44 mViewModel = a_rModelView;
	mViewModel.InvertOrthonormal();

	for ( TINT i = 0; i < m_iNumBuiltCells; i++ )
	{
		for ( CellSettings* pSettings = m_pCellNodes[ i ].m_pSettings; pSettings != TNULL; pSettings = pSettings->m_pNext )
		{
			Frustum oViewFrustum;
			Frustum oModelFrustum;

			const TPlane* pFrustumPlanes = TRender::GetSingleton()->GetCurrentContext()->GetFrustumPlanes();

			for ( TINT k = 0; k < 6; k++ )
			{
				oViewFrustum.m_aPlanes[ k ].m_Plane      = pFrustumPlanes[ k ];
				oViewFrustum.m_aPlanes[ k ].m_uiClipFlag = 1 << k;
			}

			oModelFrustum.Transform( oViewFrustum, mViewModel );
		}
	}
}

// The frustum planes of the current render context with their clip flags
// $deBlob: FUNCTION 006ad770
static void SetupViewFrustum( Frustum* a_pFrustum, void* a_pUnused1, void* a_pUnused2 )
{
	const TPlane* pFrustumPlanes = TRender::GetSingleton()->GetCurrentContext()->GetFrustumPlanes();

	for ( TINT k = 0; k < 6; k++ )
	{
		a_pFrustum->m_aPlanes[ k ].m_Plane      = pFrustumPlanes[ k ];
		a_pFrustum->m_aPlanes[ k ].m_uiClipFlag = 1 << k;
	}
}

// $deBlob: FUNCTION 006ad0e0
void TWorldVis::RenderTreeIntersectNonRecurse( TTMDWin::CellSphereTreeNode* a_pNode, RenderData* a_pRenderData )
{
	StackValue* pStack = s_pStack;
	Frustum*    pFrustum = a_pRenderData->m_pFrustum;

	// The last leaf of the tree (right after it ends the tree)
	TTMDWin::CellSphereTreeNode* pLast = a_pNode;
	while ( pLast->m_pRight != TNULL )
		pLast = pLast->m_pRight;

	// A fake parent whose right child is the end of the tree
	TTMDWin::CellSphereTreeNode oRoot;
	oRoot.m_Sphere = TSphere( 0.0f, 0.0f, 0.0f, 0.0f );
	oRoot.m_pRight = pLast->GetLeafEnd();

	pStack[ 0 ].m_pNode             = a_pNode;
	pStack[ 0 ].m_pParent           = &oRoot;
	pStack[ 0 ].m_iActivePlaneCount = pFrustum->m_iActivePlaneCount;
	pStack[ 1 ].m_pNode             = a_pNode;
	pStack[ 1 ].m_pParent           = &oRoot;
	pStack[ 1 ].m_iActivePlaneCount = pFrustum->m_iActivePlaneCount;

	TINT        iStackPos = 0;
	StackValue* pTop      = &pStack[ 1 ];

	do
	{
		TTMDWin::CellSphereTreeNode* pNode = pTop->m_pNode;
		pFrustum->m_iActivePlaneCount      = pTop->m_iActivePlaneCount;

		TBOOL bPop = TFALSE;

		if ( pNode->m_pRight == TNULL )
		{
			RenderLeafNodeIntersect( pNode, a_pRenderData );
			bPop = TTRUE;
		}
		else
		{
			TSphere oSphere = pNode->m_Sphere;

			if ( m_uiFlags & Flags_Transform )
				TMatrix44::TransformVector( oSphere.GetOrigin(), m_oTransform, oSphere.GetOrigin() );

			TINT iResult = pFrustum->IntersectSphereReduce( oSphere );

			if ( iResult == Frustum::SphereTest_Inside )
			{
				// Fully inside: every leaf of the subtree
				TASSERT( iStackPos >= 0 );
				TASSERT( iStackPos >= 0 );

				TTMDWin::CellSphereTreeNode* pEnd = pTop->m_pParent->m_pRight;
				TINT                         iActivePlaneCount = pFrustum->m_iActivePlaneCount;
				pFrustum->m_iActivePlaneCount = 0;

				while ( pNode < pEnd )
				{
					if ( pNode->m_pRight == TNULL )
					{
						RenderLeafNodeIn( pNode, a_pRenderData );
						pNode = pNode->GetLeafEnd();
					}
					else
					{
						pNode = pNode->GetLeft();
					}
				}

				pFrustum->m_iActivePlaneCount = iActivePlaneCount;
				TASSERT( iStackPos >= 0 );
				bPop = TTRUE;
			}
			else if ( iResult == Frustum::SphereTest_Outside )
			{
				TASSERT( iStackPos >= 0 );
				bPop = TTRUE;
			}
			else if ( iResult == Frustum::SphereTest_Intersecting )
			{
				// Intersecting: its left child next
				iStackPos++;
				TASSERT( iStackPos < s_iStackSize );

				pTop[ 1 ].m_iActivePlaneCount = pFrustum->m_iActivePlaneCount;
				pTop[ 1 ].m_pNode             = pNode->GetLeft();
				pTop[ 1 ].m_pParent           = pNode;
				pTop++;
			}
		}

		if ( bPop )
		{
			// Back to the parent, which continues with its right child
			iStackPos--;
			pTop--;
			TASSERT( iStackPos >= -1 );
			pTop->m_pNode = pTop->m_pNode->m_pRight;
		}

	} while ( iStackPos >= 0 );
}

// $deBlob: FUNCTION 006ad3c0
void TWorldVis::RenderLeafNodeIntersect( TTMDWin::CellSphereTreeNode* a_pNode, RenderData* a_pRenderData )
{
	TASSERT( a_pNode->m_pRight == TNULL );

	TINT iActivePlaneCount = a_pRenderData->m_pFrustum->m_iActivePlaneCount;
	TASSERT( TNULL != m_pfnRenderCallback || TNULL != m_pfnRenderCallbackUser );

	for ( TINT i = 0; i < a_pNode->m_iNumMeshes; i++ )
	{
		TTMDWin::TRBWorldCellMesh* pCellMesh = a_pRenderData->m_pCell->m_ppMeshes[ a_pNode->m_aMeshIndices[ i ] ];
		TSphere                    oSphere   = pCellMesh->m_Sphere;

		if ( m_uiFlags & Flags_Transform )
			TMatrix44::TransformVector( oSphere.GetOrigin(), m_oTransform, oSphere.GetOrigin() );

		if ( a_pRenderData->m_pFrustum->IntersectSphereReduce( oSphere ) != Frustum::SphereTest_Outside )
		{
			if ( m_pfnRenderCallback != TNULL )
				m_pfnRenderCallback( pCellMesh, a_pRenderData );
			else if ( m_pfnRenderCallbackUser != TNULL )
				m_pfnRenderCallbackUser( pCellMesh, a_pRenderData, m_pRenderCallbackUserData );
		}

		a_pRenderData->m_pFrustum->m_iActivePlaneCount = iActivePlaneCount;
	}
}

// $deBlob: FUNCTION 006ad4e0
void TWorldVis::RenderLeafNodeIn( TTMDWin::CellSphereTreeNode* a_pNode, RenderData* a_pRenderData )
{
	TASSERT( a_pNode->m_pRight == TNULL );
	TASSERT( TNULL != m_pfnRenderCallback || TNULL != m_pfnRenderCallbackUser );

	for ( TINT i = 0; i < a_pNode->m_iNumMeshes; i++ )
	{
		TTMDWin::TRBWorldCellMesh* pCellMesh = a_pRenderData->m_pCell->m_ppMeshes[ a_pNode->m_aMeshIndices[ i ] ];

		if ( m_pfnRenderCallback != TNULL )
			m_pfnRenderCallback( pCellMesh, a_pRenderData );
		else if ( m_pfnRenderCallbackUser != TNULL )
			m_pfnRenderCallbackUser( pCellMesh, a_pRenderData, m_pRenderCallbackUserData );
	}
}

TOSHI_NAMESPACE_END
