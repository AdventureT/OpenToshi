#include "pch.h"
#include "RenderLightmappedInstance_DX11.h"
#include "Terrain2/TerrainTypes.h"
#include "Animation/AModelInstance2.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Buildings/ALandmarkInstance.h"
#include "GameController/AGameStatus.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Render/Lighting/ALightingData.h"
#include "Render/Material/AMaterialManager.h"
#include "Cameras/ACamera.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Fx/AEnvironmentFX.h"

#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Render/TModel.h>
#include <Render/TMesh.h>
#include <Render/TSkeleton.h>
#include <Render/TTMDWin.h>
#include <Render/TWorldVIS.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

using namespace TerrainTypes;

// Set to the a_szMeshFilter argument of RenderInstance while it draws: only the meshes of this material are drawn
// NOTE: nothing ported reads it yet
static const TCHAR* s_szMeshFilter = TNULL;

static TFLOAT s_fPaintedCountDistanceSq       = 60000.0f;
static TINT   s_iLandmarkPaintedWeight        = 10;
static TFLOAT s_fLowLODPaintedCountDistanceSq = 60000.0f;
static TFLOAT s_fLowLODLandmarkPaintedWeight  = 5.0f;

// The lighting of a low LOD instance without an AModelInstance2: the current light, a_uiAmbientColour (ARGB) as the
// ambient colour, scaled by the environment's lighting scale at a_fAmbientScale
// $deBlob: FUNCTION 005558d0
static void SetLowLODRenderLighting( TFLOAT a_fAmbientScale, TUINT32 a_uiAmbientColour )
{
	TRender*              pRender = TRender::GetSingleton();
	ALightingData::Light* pLight  = ALightingData::GetSingleton()->GetCurrent();

	TASSERT( pLight->vLightDirection.w == 0.0f );
	pRender->GetLightDirection().AsBasisVector4( BASISVECTOR_RIGHT ) = pLight->vLightDirection;
	pRender->GetLightColour().AsBasisVector4( BASISVECTOR_RIGHT )    = pLight->vLightColour;

	TRenderContext* pContext = pRender->GetCurrentContext();
	TVector4        vAmbient;
	vAmbient.x = ( TFLOAT( ( a_uiAmbientColour >> 16 ) & 0xFF ) + 0.0f ) * ( 1.0f / 255.0f );
	vAmbient.y = ( TFLOAT( ( a_uiAmbientColour >> 8 ) & 0xFF ) + 0.0f ) * ( 1.0f / 255.0f );
	vAmbient.z = ( TFLOAT( a_uiAmbientColour & 0xFF ) + 0.0f ) * ( 1.0f / 255.0f );
	vAmbient.w = ( TFLOAT( a_uiAmbientColour >> 24 ) + 0.0f ) * ( 1.0f / 255.0f );

	if ( AEnvironmentFX::GetSingleton() != TNULL )
	{
		TVector4 vScale;
		AEnvironmentFX::GetLightingScale( &vScale, a_fAmbientScale );
		vScale.Multiply( 0.003921569f );
		vAmbient.Multiply( vScale );
	}

	// NOTE: retail writes the context's ambient colour first and scales it in place
	pContext->SetAmbientColour( vAmbient );
}

// Counts a drawn paintable instance near the camera in AGameStatus (a painted landmark weighs a_iLandmarkWeight)
static void CountPaintedInstance( TRenderContext* a_pContext, CellInstance* a_pInstance, TFLOAT a_fDistanceSq, TFLOAT a_fMaxDistanceSq, TINT a_iLandmarkWeight )
{
	if ( a_fDistanceSq < 0.0f )
	{
		const TVector3& rCamPos = a_pContext->GetCameraObject()->GetTransformObject().GetTranslation();
		a_fDistanceSq           = TVector4::DistanceSq( TVector4( rCamPos.x, rCamPos.y, rCamPos.z, 1.0f ), **a_pInstance->m_ppCentre );
	}

	ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, a_pInstance->m_pModelInstance2 );

	if ( pBuilding == TNULL || a_fDistanceSq >= a_fMaxDistanceSq || !( a_pInstance->m_uiFlags & 0x800000 ) )
		return;

	switch ( a_pInstance->m_uiFlags & 7 )
	{
		case InstanceType_Building:
			if ( pBuilding->GetBuildingFlags() & ABuildingInstance::Flags_Painted )
				AGameStatus::AddVisiblePaintedInstances( 1 );
			break;
		case InstanceType_Tree:
			if ( TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 )->IsTransformed() )
				AGameStatus::AddVisiblePaintedInstances( 1 );
			break;
		case InstanceType_Landmark:
			// ALandmarkInstance (an ABuildingInstance)
			if ( ( pBuilding->GetBuildingFlags() >> 14 ) & 1 )
				AGameStatus::AddVisiblePaintedInstances( a_iLandmarkWeight );
			break;
		default:
			return;
	}

	AGameStatus::AddVisiblePaintableInstance();
}

// TRUE if the LOD 0 bounding sphere of the model instance is outside the frustum of the camera being rendered
// $deBlob: FUNCTION 00554470
static TBOOL IsInstanceCulled( TModelInstance* a_pModelInstance )
{
	TRender::GetSingleton()->GetCurrentContext();

	T2ModelPtr pModel  = a_pModelInstance->GetModel();
	TSphere    oSphere = pModel->GetLOD( 0 ).GetRenderVolume();

	TTransformObject& rTransform = a_pModelInstance->GetTransform();
	const TVector3&   rScale     = rTransform.GetScale();

	TVector3 vCentre( oSphere.GetOrigin().x * rScale.x, oSphere.GetOrigin().y * rScale.y, oSphere.GetOrigin().z * rScale.z );

	TFLOAT fMaxScale = rScale.x;
	if ( fMaxScale <= rScale.y ) fMaxScale = rScale.y;
	if ( fMaxScale < rScale.z ) fMaxScale = rScale.z;

	TFLOAT fRadius = fMaxScale * oSphere.GetRadius();

	TMatrix44 oLocalMatrix = rTransform.GetLocalMatrix();
	TMatrix44::TransformVector( vCentre, oLocalMatrix, vCentre );

	if ( 0.0f < fRadius )
	{
		AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
		TASSERT( pViewportManager->GetViewportBeingRendered() != AXYZViewportManager::VIEWPORT_NONE );

		ACamera* pCamera = pViewportManager->GetCamera( pViewportManager->GetViewportBeingRendered() );

		if ( !TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), pCamera->GetViewPlanes(), 6 ) )
			return TTRUE;
	}

	return TFALSE;
}

// The low LOD copy of IsInstanceCulled: tests the world planes of the current render context instead of the frustum
// of the camera being rendered
// $deBlob: FUNCTION 00555670
static TBOOL IsLowLODInstanceCulled( TModelInstance* a_pModelInstance )
{
	TRenderContext* pRenderContext = TRender::GetSingleton()->GetCurrentContext();

	T2ModelPtr pModel  = a_pModelInstance->GetModel();
	TSphere    oSphere = pModel->GetLOD( 0 ).GetRenderVolume();

	TTransformObject& rTransform = a_pModelInstance->GetTransform();
	const TVector3&   rScale     = rTransform.GetScale();

	TVector3 vCentre( oSphere.GetOrigin().x * rScale.x, oSphere.GetOrigin().y * rScale.y, oSphere.GetOrigin().z * rScale.z );

	TFLOAT fMaxScale = rScale.x;
	if ( fMaxScale <= rScale.y ) fMaxScale = rScale.y;
	if ( fMaxScale < rScale.z ) fMaxScale = rScale.z;

	TFLOAT fRadius = fMaxScale * oSphere.GetRadius();

	TMatrix44 oLocalMatrix = rTransform.GetLocalMatrix();
	TMatrix44::TransformVector( vCentre, oLocalMatrix, vCentre );

	if ( 0.0f < fRadius )
	{
		if ( !TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), pRenderContext->GetWorldPlanes(), 6 ) )
			return TTRUE;
	}

	return TFALSE;
}

// Selects LOD 0, resets the alpha, sets the skeleton and pushes the transform of a_pModelInstance
// $deBlob: FUNCTION 00554960
// $deBlob: FUNCTION 00555830
static void BeginModelInstanceRender( TModelInstance* a_pModelInstance )
{
	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();

	a_pModelInstance->SetLOD( 0 );
	pContext->SetAlphaBlend( 1.0f );
	pContext->SetSkeletonInstance( a_pModelInstance->GetSkeletonInstance() );
	a_pModelInstance->GetTransform().Push();
}

// Draws the LOD 0 meshes of a low LOD model instance and pops its transform
// An ARGB colour from 0-1 components
// $deBlob: FUNCTION 00554fa0
TUINT32 MakeColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return TUINT32( TINT( a_fBlue * 255.0f ) ) |
	    ( ( TUINT32( TINT( a_fGreen * 255.0f ) ) | ( ( ( TUINT32( TINT( a_fAlpha * 255.0f ) ) << 8 ) | TUINT32( TINT( a_fRed * 255.0f ) ) ) << 8 ) ) << 8 );
}

// $deBlob: FUNCTION 00555550
// $deBlob: FUNCTION 00555555
void RenderLowLODModel( TModelInstance* a_pModelInstance )
{
	T2ModelPtr pModel = a_pModelInstance->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	BeginModelInstanceRender( a_pModelInstance );

	TRenderContext* pContext  = TRender::GetSingleton()->GetCurrentContext();
	TFLOAT          fOldAlpha = pContext->GetAlphaBlend();

	if ( a_pModelInstance->IsForceFade() )
		pContext->SetAlphaBlend( a_pModelInstance->GetFade() );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		rLOD.GetMesh( i )->Render();

	pContext->SetAlphaBlend( fOldAlpha );
	a_pModelInstance->GetTransform().Pop();
}

static TBOOL s_bRenderWorldVisMeshes = TTRUE;
static TBOOL s_bDebugDrawSpheres     = TTRUE; // (the drawing is compiled out)

// Adds the meshes of a_pMeshes that are instances of a_pMesh (a_bMatch: only the ones naming it, else all) to its
// instance list and renders it
// $deBlob: FUNCTION 005546a0
static TBOOL RenderMeshInstances( TWorldMeshHAL* a_pMesh, InstanceMeshes* a_pMeshes, TBOOL a_bMatch )
{
	TBOOL bResult           = TFALSE;
	TINT  iNumInstances     = 0;
	TBOOL bOpaque           = TFALSE;
	TBOOL bOrderTable       = TFALSE;
	TINT  iSecondOrderTable = -1;

	for ( TUINT i = 0; i < a_pMeshes->m_uiNumMeshes; i++ )
	{
		TWorldMeshInstanceData::Source* pSource = &a_pMeshes->m_pMeshes[ i ];

		if ( a_bMatch && ( pSource->m_szName != a_pMesh->GetTRBMesh()->m_szMaterialName || pSource->m_uiMeshId != a_pMesh->GetTRBMesh()->m_uiUnk20 ) )
			continue;

		if ( s_szMeshFilter != TNULL )
		{
			if ( TStringManager::String8Compare( pSource->m_pMaterial->GetMaterialName(), s_szMeshFilter, TStringManager::String8Length( s_szMeshFilter ) ) == 0 )
			{
				a_pMesh->AddInstance( pSource, 0 );
				iNumInstances++;
			}
		}
		else
		{
			TWorldMaterialHAL* pOverrideMat = pSource->m_pMaterial;
			TUINT32            uiFlags;

			if ( TWorldShaderHAL::s_bInstancing )
				uiFlags = ( pOverrideMat->GetFlags() & 0x400 ) ? 10 : 5;
			else
				uiFlags = 6;

			if ( pOverrideMat->GetFlags() & 0x400 )
			{
				bOrderTable = TTRUE;
				TASSERT( -1 == iSecondOrderTable || pOverrideMat->GetOrderTable()->GetIndex() == iSecondOrderTable );
				iSecondOrderTable = pOverrideMat->GetOrderTable()->GetIndex();
			}
			else
			{
				bOpaque = TTRUE;
			}

			a_pMesh->AddInstance( pSource, uiFlags );
			iNumInstances++;
		}
	}

	if ( iNumInstances > 0 )
	{
		if ( TWorldShaderHAL::s_bInstancing )
		{
			if ( bOpaque )
				a_pMesh->RenderWithCallback( TNULL, TNULL );

			bResult = bOrderTable;

			if ( bOrderTable )
				bResult = a_pMesh->RenderWithCallback( TNULL, TNULL, TINT16( iSecondOrderTable ) );
		}
		else
		{
			bResult = a_pMesh->RenderWithCallback( TNULL, TNULL );
		}
	}

	return bResult;
}

// DOES_NOTHING: the debug drawing of the spheres of the cell meshes (compiled out)
static void DebugDrawSphere( const TMatrix44& a_rMatrix, TFLOAT a_fRadius, TUINT32 a_uiColour ) {}

// TWorldVis's render callback of the flag 0x40 instances with a world vis
// $deBlob: FUNCTION 00554860
static void __stdcall RenderWorldVisMesh( TTMDWin::TRBWorldCellMesh* a_pCellMesh, TWorldVis::RenderData* a_pRenderData, void* a_pUserData )
{
	CellInstance*  pInstance = TSTATICCAST( CellInstance, a_pUserData );
	TTRBWorldMesh* pTRBMesh  = TSTATICCAST( TTRBWorldMesh, a_pCellMesh->m_pWorldMesh );
	TASSERT( ( (TMesh*)( pTRBMesh->m_pMesh ) )->IsA( &TGetClass( TWorldMeshHAL ) ) );

	if ( s_bRenderWorldVisMeshes )
		RenderMeshInstances( pTRBMesh->m_pMesh, pInstance->m_pSymbolData, TTRUE );

	if ( s_bDebugDrawSpheres )
	{
		TMatrix44 oMatrix = *pInstance->m_pMatrix;
		TVector3  vCentre;
		TMatrix44::TransformVector( vCentre, oMatrix, a_pCellMesh->m_Sphere.GetOrigin() );
		oMatrix.SetTranslation( TVector4( vCentre.x, vCentre.y, vCentre.z, 1.0f ) );
		DebugDrawSphere( oMatrix, a_pCellMesh->m_Sphere.GetRadius(), 0xFFFF0000 );
	}
}

// $deBlob: FUNCTION 00553ef0
void __stdcall RenderLightmappedInstance::RenderInstance( Cell* a_pCell, CellInstance* a_pInstance, TBOOL a_bCull, const TCHAR* a_szMeshFilter, TFLOAT a_fDistanceSq, void* a_pUserData )
{
	AModelInstance2* pModelInstance2 = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );
	TASSERT( a_pInstance->m_uiFlags & 0x800000 );

	TModelInstance* pModelInstance = pModelInstance2->GetModelInstance();
	T2ModelPtr      pModel         = pModelInstance->GetModel();
	TModelLOD&      rLOD           = pModel->GetLOD( 0 );

	if ( rLOD.GetMeshCount() <= 0 )
		return;

	if ( a_bCull && IsInstanceCulled( pModelInstance ) )
		return;

	TRender*        pRender  = TRender::GetSingleton();
	TRenderContext* pContext = pRender->GetCurrentContext();

	CountPaintedInstance( pContext, a_pInstance, a_fDistanceSq, s_fPaintedCountDistanceSq, s_iLandmarkPaintedWeight );

	TSkeletonInstance* pSkeletonInstance = pModelInstance->GetSkeletonInstance();

	if ( a_pInstance->m_uiFlags & 0x20 )
		pSkeletonInstance = ( *TSTATICCAST( TModelInstance*, a_pInstance->m_pAnimRecord ) )->GetSkeletonInstance();

	if ( pSkeletonInstance->GetOverlayAnimationCount() + pSkeletonInstance->GetBaseAnimationCount() != 0 )
		pSkeletonInstance->UpdateState( TFALSE );

	BeginModelInstanceRender( pModelInstance );
	s_szMeshFilter = a_szMeshFilter;

	TFLOAT fOldAlpha = pContext->GetAlphaBlend();

	if ( pModelInstance2->GetModelInstance()->IsForceFade() )
		pContext->SetAlphaBlend( pModelInstance2->GetModelInstance()->GetFade() );

	const TVector4& rScale = pModelInstance2->GetScale();
	TBOOL           bScaled;
	TMatrix44       oOldModelView;

	if ( rScale.x == 1.0f && rScale.y == 1.0f && rScale.z == 1.0f && rScale.w == 1.0f )
	{
		bScaled = TFALSE;
	}
	else
	{
		bScaled       = TTRUE;
		oOldModelView = pContext->GetModelViewMatrix();

		TMatrix44 oModelView = pContext->GetModelViewMatrix();
		oModelView.Scale( rScale );
		pContext->SetModelViewMatrix( oModelView );
	}

	TVector4  vOldAmbient     = pContext->GetAmbientColour();
	TMatrix44 oOldLightColour = pRender->GetLightColour();

	TFLOAT fAmbientScale = 1.0f;
	if ( !( a_pInstance->m_uiFlags & 0x40 ) )
		fAmbientScale = ALightingData::GetSingleton()->GetInstanceAmbientScale( a_pInstance->m_iAmbientScaleIndex );

	pModelInstance2->SetRenderLighting( fAmbientScale, !( ( a_pInstance->m_uiFlags >> 4 ) & 1 ) );

	if ( !( a_pInstance->m_uiFlags & 0x40 ) )
	{
		AMaterialManager::GetSingleton()->UpdateUVs( pModelInstance->GetModel().GetData() );

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
			rLOD.GetMesh( i )->Render();
	}
	else
	{
		// The meshes of the instance (a symbol of the cell's TRB) are drawn as instances of the cell model's world meshes
		InstanceMeshes* pMeshes = a_pInstance->m_pSymbolData;
		AMaterialManager::GetSingleton()->UpdateUVs( pMeshes );

		T2ModelPtr             pInstanceModel = pModelInstance->GetModel();
		TTMDWin::TRBWorldData* pWorldData     = TSTATICCAST( TTMDWin::TTRBWorldHeader, pInstanceModel->GetAssetData() )->m_pWorldData;

		if ( pWorldData->m_iNumGroups > 0 )
		{
			TTMDWin::TRBWorldCellGroup* pGroup = pWorldData->m_ppGroups[ 0 ];

			if ( pGroup != TNULL && pGroup->m_iNumCells > 0 )
			{
				if ( a_pInstance->worldVisIndex == 0xFF )
				{
					for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
					{
						TWorldMeshHAL* pMesh = TSTATICCAST( TWorldMeshHAL, rLOD.GetMesh( i ) );
						TASSERT( ( pMesh == TNULL ) || pMesh->IsA( &TGetClass( TWorldMeshHAL ) ) );
						RenderMeshInstances( pMesh, pMeshes, pGroup->m_ppCells[ 0 ]->m_iMeshCount != 1 );
					}
				}
				else
				{
					TWorldVis* pWorldVis  = ATerrain2Interface::GetInstance()->GetWorldVis( a_pInstance );
					TMatrix44  oModelView = TRender::GetSingleton()->GetCurrentContext()->GetModelViewMatrix();
					pWorldVis->SetRenderCallbackUser( RenderWorldVisMesh, a_pInstance );
					pWorldVis->Reset();
					pWorldVis->Build( oModelView, *TSTATICCAST( TMatrix44, a_pUserData ), 0, TFALSE );
					pWorldVis->Render( oModelView, *TSTATICCAST( TMatrix44, a_pUserData ), TFALSE );
				}
			}
		}
	}

	if ( bScaled )
		pContext->SetModelViewMatrix( oOldModelView );

	pContext->SetAlphaBlend( fOldAlpha );
	pContext->SetAmbientColour( vOldAmbient );
	pRender->SetLightColourMatrix( oOldLightColour );
	s_szMeshFilter = TNULL;
	pModelInstance->GetTransform().Pop();
}

// $deBlob: FUNCTION 00555130
void __stdcall RenderLightmappedInstance::RenderLowLODInstance( Cell* a_pCell, CellInstance* a_pInstance, TBOOL a_bCull, TFLOAT a_fDistanceSq )
{
	TRender*         pRender         = TRender::GetSingleton();
	TModelInstance*  pLowLODInstance = a_pInstance->m_pLowLODInstance;
	AModelInstance2* pModelInstance2 = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );

	T2ModelPtr pModel = pLowLODInstance->GetModel();

	if ( pModel->GetLOD( 0 ).GetMeshCount() <= 0 )
		return;

	if ( a_bCull && IsLowLODInstanceCulled( pLowLODInstance ) )
		return;

	ALandmarkInstance* pLandmark = TNULL;
	if ( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark )
		pLandmark = TSTATICCAST( ALandmarkInstance, pModelInstance2 );

	TRenderContext* pContext = pRender->GetCurrentContext();

	CountPaintedInstance( pContext, a_pInstance, a_fDistanceSq, s_fLowLODPaintedCountDistanceSq, TINT( s_fLowLODLandmarkPaintedWeight ) );
	TVector4  vOldAmbient     = pContext->GetAmbientColour();
	TMatrix44 oOldLightColour = pRender->GetLightColour();

	TFLOAT fAmbientScale = ALightingData::GetSingleton()->GetInstanceAmbientScale( a_pInstance->m_iAmbientScaleIndex );

	if ( pModelInstance2 != TNULL )
	{
		pModelInstance2->SetRenderLighting( fAmbientScale, TTRUE );
	}
	else
	{
		SetLowLODRenderLighting( fAmbientScale, ( a_pInstance->m_uiFlags & 7 ) == 1 ? TUINT32( TUINTPTR( a_pInstance->m_pTypeData ) ) : TUINT32( -1 ) );
	}

	if ( a_pInstance->m_uiFlags & 0x20 )
	{
		TSkeletonInstance* pSkeletonInstance = ( *TSTATICCAST( TModelInstance*, a_pInstance->m_pAnimRecord ) )->GetSkeletonInstance();

		if ( pSkeletonInstance->GetOverlayAnimationCount() + pSkeletonInstance->GetBaseAnimationCount() != 0 )
			pSkeletonInstance->UpdateState( TFALSE );
	}

	if ( pLandmark == TNULL )
	{
		if ( a_pInstance->m_uiFlags & 0x2000000 )
		{
			// Swaps the materials of the low LOD meshes for the instance's ones
			T2ModelPtr pModel = pLowLODInstance->GetModel();
			TModelLOD& rLOD   = pModel->GetLOD( 0 );
			pModel.Release();

			for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
			{
				TMesh* pMesh = rLOD.GetMeshes()[ i ];

				for ( TerrainTypes::TextureOverride* pOverride = TSTATICCAST( TerrainTypes::TextureOverride, a_pInstance->m_pTypeData ); pOverride != TNULL; pOverride = pOverride->m_pNext )
				{
					if ( pOverride->m_pMaterial != TNULL && pMesh->GetMaterial() == pOverride->m_pSource )
					{
						pMesh->SetMaterial( pOverride->m_pMaterial );
						break;
					}
				}
			}
		}
	}
	else
	{
		pLandmark->SetLODMeshMaterials();
	}

	RenderLowLODModel( pLowLODInstance );

	if ( pLandmark == TNULL )
	{
		if ( a_pInstance->m_uiFlags & 0x2000000 )
		{
			// Restores the materials of the low LOD meshes
			T2ModelPtr pModel = pLowLODInstance->GetModel();
			TModelLOD& rLOD   = pModel->GetLOD( 0 );
			pModel.Release();

			for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
			{
				TMesh* pMesh = rLOD.GetMeshes()[ i ];

				for ( TerrainTypes::TextureOverride* pOverride = TSTATICCAST( TerrainTypes::TextureOverride, a_pInstance->m_pTypeData ); pOverride != TNULL; pOverride = pOverride->m_pNext )
				{
					if ( pMesh->GetMaterial() == pOverride->m_pMaterial )
					{
						pMesh->SetMaterial( pOverride->m_pSource );
						break;
					}
				}
			}
		}
	}
	else
	{
		pLandmark->RestoreLODMeshMaterials();
	}

	pContext = pRender->GetCurrentContext();
	pContext->SetAmbientColour( vOldAmbient );
	pRender->SetLightColourMatrix( oOldLightColour );
}
