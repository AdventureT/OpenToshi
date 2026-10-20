#include "pch.h"
#include "AJellyShader_DX11.h"
#include "Animation/AModelInstance2.h"
#include "Buildings/ABuildingInstance.h"
#include "Cameras/ACameraManager.h"
#include "GameController/AGameStatus.h"
#include "Terrain2/TerrainTypes.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Render/ARenderer.h"
#include "Render/RenderLightmappedInstance_DX11.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Cameras/ACamera.h"
#include "ACurrentLevelInformation.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"

#include <Render/TRenderContext.h>
#include <Render/TModelManager.h>
#include <Platform/DX11/TRender_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Toshi/TStringManager.h>
#include <DirectXMath.h>

#include <Render/TModel.h>
#include <Render/TSkeleton.h>
#include <Render/TAnimation.h>
#include <Platform/DX11/TWorldShader_DX11.h>

// Note: Should be the last include!
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TINT AJellyShader_DX11::s_ciIndices[ NUM_SHADER_PASSES ]       = { 0, -1, -1, 1, 2, 3, 4 };
const TINT AJellyShader_DX11::s_ciWindowIndices[ NUM_SHADER_PASSES ] = { -1, -1, -1, -1, 0, 1, 2 };

// StartPaint's sizes of the solids and of the projection of the colour
static TFLOAT         s_fConeHeightScale           = -1.0f;
static TFLOAT         s_fConeWidthScale            = 12.0f;
static TFLOAT         s_fConeScaleY                = 24.0f;
static TFLOAT         s_fFastDripWidthScale        = 6.0f;
static TFLOAT         s_fDripWidthScale            = 8.4f;
static TFLOAT         s_fDripHeightScale           = 2.65f;
static TFLOAT         s_fMinForwardSq              = 1e-6f;
static TFLOAT         s_fDripOffsetY               = -1.0f;
static TFLOAT         s_fMaxTargetOffsetY          = 10.0f;
static TFLOAT         s_fProjectionDistance        = 5.0f; // (set by StartPaint)
static TBOOL          s_bAutoProjectionDistance    = TTRUE;
static TFLOAT         s_fProjectorFOV              = 30.0f; // in degrees
static TFLOAT         s_fMaxProjectionDistance     = 30.0f;
static TFLOAT         s_fTallBuildingDistanceScale = 2.0f;
static TFLOAT         s_fProjectionDropScale       = 0.3f;
static TFLOAT         s_fProjectorTexFOV           = 40.0f; // in degrees
static TFLOAT         s_fUnk1SolidScale            = 144.0f;
static TFLOAT         s_fTallBuildingRatio         = 0.0f; // (never set)
static const TVector4 s_vForward                   = TVector4( 0.0f, 0.0f, 1.0f, 1.0f );

// The solid every paint uses, 2: the one the caller asks for
static TINT s_eForcedSolid = 2;

// The passes of the building's jelly materials and of the solids' jelly skin materials
static const TINT s_iBuildingPasses[ AJellyShader_DX11::NUM_PASSES ]    = { 0, 3, 4, 5, 6 };
static const TINT s_iSolidPasses[ AJellyShader_DX11::NUM_SOLID_PASSES ] = { 1, 2 };

static TBOOL  s_bCullSolids            = TTRUE;
static TFLOAT s_fSplatFrameTime        = 0.04f;                                               // the time of a frame of the splat textures
static TBOOL  s_abRenderPasses[ 7 ]    = { TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE, TTRUE }; // (TINTs)
static TFLOAT s_fSolidCameraDistanceSq = 6400.0f;
static TFLOAT s_fSolidSpeed            = 3.5f;
static TFLOAT s_fMPSolidSpeed          = 5.0f;
static TFLOAT s_fConeSolidSpeed        = 1.0f;
static TFLOAT s_fPass0AlphaRef         = 0.7843137f; // (200/255)
static TFLOAT s_fPass0DstAlpha         = 0.5137255f; // (131/255)
static TUINT8 s_uiPass3Alpha           = 131;
static TFLOAT s_fProjectionFOV         = 40.0f; // in degrees
static TBOOL  s_bDebugSolidOnly        = TFALSE;

// The [-1;1] to [0;1] scale and offset of the projected textures
static const TMatrix44 s_oProjScale = {
	0.5f, 0.0f, 0.0f, 0.0f,
	0.0f, 0.5f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.0f, 0.0f, 0.0f, 1.0f
};
static const TMatrix44 s_oProjOffset = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, 1.0f, 0.0f,
	0.5f, 0.5f, 0.0f, 1.0f
};

// XMMatrixMultiply( a_rA, a_rB ): a_rA then a_rB
static TMatrix44 MultiplyMatrices( const TMatrix44& a_rA, const TMatrix44& a_rB )
{
	TMatrix44 oResult;
	DirectX::XMStoreFloat4x4(
	    TREINTERPRETCAST( DirectX::XMFLOAT4X4*, &oResult ),
	    DirectX::XMMatrixMultiply( DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &a_rA ) ), DirectX::XMLoadFloat4x4( TREINTERPRETCAST( const DirectX::XMFLOAT4X4*, &a_rB ) ) )
	);
	return oResult;
}

// Shows or hides a building: the light mapped (and grass) instances through the visible flag of their cell instance
static void SetBuildingVisible( AModelInstance2* a_pInstance, TBOOL a_bVisible )
{
	TerrainTypes::CellInstance* pTerrain = TSTATICCAST( ABuildingInstance, a_pInstance )->GetTerrainInstance();

	if ( ( pTerrain->m_uiFlags & 0x10 ) || ( pTerrain->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Grass )
	{
		if ( a_bVisible )
			pTerrain->m_uiFlags |= 0x800000;
		else
			pTerrain->m_uiFlags &= ~0x800000;
	}
	else
	{
		a_pInstance->SetFlag( AModelInstance2::FLAG_Visible, a_bVisible );
	}
}

// The solid's animation 0 is playing (its total time hasn't passed its clamped sequence time)
static TBOOL IsSolidAnimating( TModelInstance* a_pSolid )
{
	if ( a_pSolid == TNULL )
		return TFALSE;

	TAnimation* pAnimation = a_pSolid->GetSkeletonInstance()->IsAnimating( 0 );

	return pAnimation != TNULL && !( pAnimation->GetTotalTime() > pAnimation->GetSeqTime() );
}

static TBOOL HasAnimationFinished( TModelInstance* a_pSolid )
{
	return !IsSolidAnimating( a_pSolid );
}

// $deBlob: FUNCTION 005fbc80
PatternSettings* GetPatternSettings()
{
	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( 0 );
	return TSTATICCAST( PatternSettings, pTRB->GetSymbolAddress( "PatternSettings" ) );
}

// The setting of a_szModel (the first one if none)
// $deBlob: FUNCTION 005fbc00
const PatternSetting* FindPatternSetting( const PatternSettings* a_pSettings, const TCHAR* a_szModel )
{
	TUINT32 uiHash = 0;

	for ( const TCHAR* pChar = a_szModel; *pChar != '\0'; pChar++ )
		uiHash = TUINT32( TINT( *pChar ) ) + 17 * uiHash;

	for ( TUINT i = 0; i < a_pSettings->m_uiNumSettings; i++ )
	{
		if ( a_pSettings->m_pSettings[ i ].m_uiHash == uiHash && T2String8::Compare( a_szModel, a_pSettings->m_pSettings[ i ].m_szName ) == 0 )
			return &a_pSettings->m_pSettings[ i ];
	}

	return a_pSettings->m_pSettings;
}

// The number of configs in use: one per viewport, at least two (three while a cone is in use)
static TINT GetNumUsableConfigs( TINT a_iNumCones )
{
	TINT iNumConfigs = ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE ) ? 1 : ACameraManager::GetSingleton()->GetNumActiveCameras();

	if ( iNumConfigs < 2 )
		iNumConfigs = ( 0 < a_iNumCones ) + 2;

	return iNumConfigs;
}

// $deBlob: FUNCTION 00558630
AJellyShader_DX11::AJellyShader_DX11()
{
	m_iCurConfig   = 0;
	m_iCurTexScale = 0;
	m_iNumCones    = 0;

	TShader* pShader = TWorldShader::GetSingleton();

	TBOOL bResult = m_aOrderTables[ 0 ].Create( pShader, -3900, 0xFF );
	bResult       = m_aOrderTables[ 1 ].Create( pShader, -3600, 0xFF ) && bResult;
	bResult       = m_aOrderTables[ 2 ].Create( pShader, -3500, 0xFF ) && bResult;
	bResult       = m_aOrderTables2[ 0 ].Create( pShader, -3499, 0xFF ) && bResult;
	bResult       = m_aOrderTables[ 3 ].Create( pShader, -3400, 0xFF ) && bResult;
	bResult       = m_aOrderTables2[ 1 ].Create( pShader, -3399, 0xFF ) && bResult;
	bResult       = m_aOrderTables[ 4 ].Create( pShader, -3398, 0xFF ) && bResult;
	bResult       = m_aOrderTables2[ 2 ].Create( pShader, -3397, 0xFF ) && bResult;
	TASSERT( bResult );
}

// $deBlob: FUNCTION 005588e0
// $deBlob: FUNCTION 0056c070
AJellyShader_DX11::~AJellyShader_DX11()
{
}

// $deBlob: FUNCTION 0055afb0
void AJellyShader_DX11::Reset()
{
	for ( TINT i = 0; i < NUM_BUILDING_CONFIGS; i++ )
	{
		BuildingConfig& rCurBuilding = m_oBuildingConfigs[ i ];

		if ( rCurBuilding.m_pInstance == TNULL )
		{
			TASSERT( rCurBuilding.m_bIsReset );
		}
		else
		{
			if ( IsSolidAnimating( rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ] ) )
				FinishConfig( rCurBuilding, TFALSE, TFALSE );

			rCurBuilding.m_bUnkF1 = TFALSE;
			TASSERT( rCurBuilding.m_bIsReset );
		}
	}

	m_iCurConfig   = 0;
	m_iCurTexScale = 0;
	m_iNumCones    = 0;
}

// $deBlob: FUNCTION 005589f0
void AJellyShader_DX11::StartPaint( AModelInstance2* a_pInstance, TUINT32 a_uiOldColour, TUINT32 a_uiNewColour, TINT a_eSolid, TINT a_iPlayer, TFLOAT a_fSpeed )
{

	if ( s_eForcedSolid != 2 )
		a_eSolid = s_eForcedSolid;

	ABuildingInstance*          pBuilding = TSTATICCAST( ABuildingInstance, a_pInstance );
	TerrainTypes::CellInstance* pTerrain  = pBuilding->GetTerrainInstance();
	TUINT32                     uiFlags   = pTerrain->m_uiFlags;

	if ( !( uiFlags & 0x1000000 ) && a_eSolid != Solid_Cone && ( !( uiFlags & 0x10 ) || !( uiFlags & 0x100 ) ) )
		return;

	if ( pTerrain->m_pSymbolData == TNULL )
		return;

	if ( GetNumUsableConfigs( m_iNumCones ) <= m_iCurConfig )
		m_iCurConfig = 0;

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE && a_eSolid != Solid_Cone && 1 < m_iCurConfig )
	{
		// No config left: the building takes the colour at once
		a_pInstance->SetLocalAmbientColour( a_uiNewColour );
		return;
	}

	BuildingConfig& rNewConfig = m_oBuildingConfigs[ m_iCurConfig ];

	if ( rNewConfig.m_pInstance != TNULL && IsSolidAnimating( rNewConfig.m_apSolids[ rNewConfig.m_eSolid ] ) )
	{
		// The config's previous building is done
		TINT eOldSolid = rNewConfig.m_eSolid;
		FinishConfig( rNewConfig, TTRUE, rNewConfig.m_bIsReset );

		if ( eOldSolid == Solid_Cone )
			m_iNumCones--;
	}

	TASSERT( rNewConfig.m_bIsReset );

	rNewConfig.m_uiOldColour = a_uiOldColour;
	rNewConfig.m_uiNewColour = a_uiNewColour;
	rNewConfig.m_eSolid      = a_eSolid;
	rNewConfig.m_pInstance   = a_pInstance;
	rNewConfig.m_uiUnk88     = 0;
	rNewConfig.m_fSpeed      = a_fSpeed;
	rNewConfig.m_uiUnkF0     = 0;
	rNewConfig.m_bUnkF1      = TFALSE;

	// The bounding box of the building's meshes
	TMatrix44 oBuildingMatrix;
	a_pInstance->GetTransform( oBuildingMatrix );

	TerrainTypes::InstanceMeshes* pMeshes = pTerrain->m_pSymbolData;
	TVector4                      vCentre( pMeshes->m_vCentre.x, pMeshes->m_vCentre.y, pMeshes->m_vCentre.z, 1.0f );
	TFLOAT                        fExtentX = pMeshes->m_vExtents.x;
	TFLOAT                        fExtentY = pMeshes->m_vExtents.y;
	TFLOAT                        fExtentZ = pMeshes->m_vExtents.z;
	TMatrix44::TransformVector( vCentre, *pTerrain->m_pMatrix, vCentre );

	TVector4 vSolidPos   = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	TVector3 vSolidScale = TVector3( 1.0f, 1.0f, 1.0f );
	TINT     eSolid      = rNewConfig.m_eSolid;

	if ( a_eSolid == Solid_Cone )
	{
		// The cone grows out of the grass
		m_iNumCones++;
		vSolidPos             = TVector4( vCentre.x + 0.0f, s_fConeHeightScale * fExtentY + vCentre.y, vCentre.z + 0.0f, vCentre.w );
		vSolidScale           = TVector3( s_fConeWidthScale * fExtentX, s_fConeScaleY, s_fConeWidthScale * fExtentZ );
		rNewConfig.m_iPattern = -1;
	}
	else if ( a_eSolid == Solid_Drip || a_eSolid == Solid_MPDrip || a_eSolid == Solid_FastDrip )
	{
		// The drip covers the building
		ABlobEntity* pBlob        = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( a_iPlayer ) ) );
		TFLOAT       fWidthScale  = ( a_eSolid == Solid_FastDrip ) ? s_fFastDripWidthScale : s_fDripWidthScale;
		TMatrix44*   pMatrix      = pTerrain->m_pMatrix;
		TMatrix44    oSolidMatrix = TMatrix44::IDENTITY;
		TFLOAT       fWidth;
		TFLOAT       fHeight;
		TFLOAT       fDepth;

		if ( !( ( pTerrain->m_uiFlags >> 22 ) & 1 ) )
		{
			// Turned like the building: its size along its forward axis
			TVector4 vForward( pMatrix->AsBasisVector4( BASISVECTOR_FORWARD ).x, 0.0f, pMatrix->AsBasisVector4( BASISVECTOR_FORWARD ).z, pMatrix->AsBasisVector4( BASISVECTOR_FORWARD ).w );

			if ( vForward.MagnitudeSq() < s_fMinForwardSq )
				vForward = TVector4( pMatrix->AsBasisVector4( BASISVECTOR_UP ).x, 0.0f, pMatrix->AsBasisVector4( BASISVECTOR_UP ).z, pMatrix->AsBasisVector4( BASISVECTOR_UP ).w );

			TVector4 vExtents( fExtentX, fExtentY, fExtentZ, 1.0f );
			TMatrix44::RotateVector( vExtents, *pMatrix, vExtents );
			fHeight    = TMath::Abs( vExtents.y );
			vExtents.y = 0.0f;

			TFLOAT fDot = vExtents.x * vForward.x + vForward.y * 0.0f + vExtents.z * vForward.z;
			fDepth      = TMath::Abs( fDot / vForward.Magnitude() );
			// NOTE: retail: 0 for a building without thickness (a flat wall), the solid then has a singular transform and the skin
			// shader's Invert asserts
			fWidth = TFLOAT( TMath::Sqrt( vExtents.MagnitudeSqXZ() - fDepth * fDepth ) );

			TFLOAT fAngle = vForward.GetAngleBetweenVector( s_vForward );

			if ( vForward.x < 0.0f )
				fAngle = TMath::TWO_PI - fAngle;

			oSolidMatrix.RotateY( fAngle );
		}
		else
		{
			// The box of the turned corners
			TVector4 aCorners[ 8 ];

			for ( TINT i = 0; i < 8; i++ )
			{
				aCorners[ i ].x = ( i < 4 ) ? fExtentX : -fExtentX;
				aCorners[ i ].y = ( i == 0 || i == 1 || i == 4 || i == 5 ) ? fExtentY : -fExtentY;
				aCorners[ i ].z = ( i & 1 ) ? -fExtentZ : fExtentZ;
			}

			TVector4 vMin( TMath::MAXFLOAT, TMath::MAXFLOAT, TMath::MAXFLOAT, 1.0f );
			TVector4 vMax( -TMath::MAXFLOAT, -TMath::MAXFLOAT, -TMath::MAXFLOAT, 1.0f );

			for ( TINT i = 0; i < 8; i++ )
			{
				TMatrix44::RotateVector( aCorners[ i ], *pMatrix, aCorners[ i ] );
				vMin.x = TMath::Min( vMin.x, aCorners[ i ].x );
				vMin.y = TMath::Min( vMin.y, aCorners[ i ].y );
				vMin.z = TMath::Min( vMin.z, aCorners[ i ].z );
				vMax.x = TMath::Max( vMax.x, aCorners[ i ].x );
				vMax.y = TMath::Max( vMax.y, aCorners[ i ].y );
				vMax.z = TMath::Max( vMax.z, aCorners[ i ].z );
			}

			TVector4 vSize( vMax.x - vMin.x, vMax.y - vMin.y, vMax.z - vMin.z, 1.0f );
			vSize.Multiply( 0.5f );
			fWidth  = vSize.x;
			fHeight = TMath::Abs( vSize.y );
			fDepth  = vSize.z;
		}

		vSolidScale = TVector3( fWidth * fWidthScale, fHeight * s_fDripHeightScale, fDepth * fWidthScale );
		vSolidPos   = TVector4( vCentre.x + 0.0f, vCentre.y + fHeight * s_fDripOffsetY, vCentre.z + 0.0f, vCentre.w );
		rNewConfig.m_apSolids[ eSolid ]->GetTransform().SetMatrix( oSolidMatrix );

		// The colour is projected from behind the painter
		const TVector4& vBlobPos = pBlob->GetPhysicsMatrix().GetTranslation();
		TVector4        vTarget( vCentre.x, vCentre.y, vCentre.z, 1.0f );
		TFLOAT          fOffsetY = vCentre.y - vBlobPos.y;

		if ( fOffsetY < -s_fMaxTargetOffsetY )
			vTarget.y = vCentre.y - ( fOffsetY + s_fMaxTargetOffsetY );
		else if ( s_fMaxTargetOffsetY < fOffsetY )
			vTarget.y = vCentre.y - ( fOffsetY - s_fMaxTargetOffsetY );

		if ( s_bAutoProjectionDistance )
		{
			TFLOAT fAvgWidth = ( fExtentX + fExtentZ ) * 0.5f;
			TASSERT( fAvgWidth > 0.f );
			s_fProjectionDistance = fAvgWidth * 0.5f * 3.732051f; // cot( 15 degrees )

			if ( fExtentY / fAvgWidth < s_fTallBuildingRatio )
			{
				rNewConfig.m_bSplatTextures = TFALSE;
			}
			else
			{
				s_fProjectionDistance *= s_fTallBuildingDistanceScale;
				rNewConfig.m_bSplatTextures = TTRUE;
			}

			if ( s_fMaxProjectionDistance < s_fProjectionDistance )
				s_fProjectionDistance = s_fMaxProjectionDistance;
		}

		TVector4 vDir( vBlobPos.x - vTarget.x, vBlobPos.y - vTarget.y, vBlobPos.z - vTarget.z, vBlobPos.w );
		vDir.Normalise();
		vDir.Multiply( s_fProjectionDistance );

		TVector4 vEye( vDir.x + vBlobPos.x, ( vDir.y + vBlobPos.y ) - s_fProjectionDropScale * s_fProjectionDistance, vDir.z + vBlobPos.z, vBlobPos.w );
		vTarget.y -= s_fProjectionDropScale * s_fProjectionDistance;

		ACamera oCamera;
		oCamera.GetCameraMatrix().m_mMatrix.SetTranslation( vEye );
		oCamera.LookAtPoint( vTarget );
		oCamera.GetCameraMatrix().m_fFOV    = ( s_fProjectorFOV / 180.0f ) * 3.1415927f;
		oCamera.GetCameraMatrix().m_fAspect = TFLOAT( tan( oCamera.GetCameraMatrix().m_fFOV * 0.5f ) );

		TMatrix44 oView = oCamera.GetCameraMatrix().m_mMatrix;
		oView.InvertOrthonormal();

		TMatrix44 oModelMatrix;
		a_pInstance->GetTransform( oModelMatrix );

		{
			T2ModelPtr pModel = a_pInstance->GetModelInstance()->GetModel();
			TASSERT( ( pModel->GetLOD( 0 ).GetMesh( 0 ) == TNULL ) || pModel->GetLOD( 0 ).GetMesh( 0 )->IsA( &TGetClass( TWorldMeshHAL ) ) );
		}

		TMatrix44 oProjection;
		oProjection.Multiply( oView, oModelMatrix );

		TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
		TASSERT( pWorldShader == TNULL || pWorldShader->IsA( &TGetClass( TWorldShaderHAL ) ) );

		rNewConfig.m_fSplatTime   = 0.0f;
		rNewConfig.m_mProjection  = oProjection;
		rNewConfig.m_iCurSplatTex = 0;

		// The pattern of the painter
		TUINT32 uiColourFlags = pBlob->GetColourComponent()->GetCurrentColour();

		if ( ( uiColourFlags & 0x10 ) || uiColourFlags == 16 )
		{
			rNewConfig.m_iPattern = 16;
		}
		else
		{
			rNewConfig.m_iPattern = APlayerManager::GetSingleton()->CyclePattern( a_iPlayer );
		}

		// The projected texture of the config
		rNewConfig.m_uiProjTexDataIndex            = TINT16( m_iCurConfig );
		TWorldShaderHAL::ProjTexture& rProjTexture = pWorldShader->GetProjTexture( rNewConfig.m_uiProjTexDataIndex );
		rProjTexture.m_oMatrix1                    = oProjection;
		rProjTexture.m_uiColour                    = rNewConfig.m_uiNewColour;

		TFLOAT fAspect = ( TRender::GetSingleton()->GetAspectRatio() == TRender::ASPECT_RATIO_16_9 ) ? 16.0f / 9.0f : 4.0f / 3.0f;
		TFLOAT fSin;
		TFLOAT fCos;
		DirectX::XMScalarSinCos( &fSin, &fCos, s_fProjectorTexFOV * 0.017453292f * 0.5f );
		TFLOAT fCot = fCos / fSin;

		TMatrix44 oPerspective = {
			fCot / fAspect, 0.0f, 0.0f, 0.0f,
			0.0f, fCot, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 1.0f,
			0.0f, 0.0f, -0.0f, 0.0f
		};

		rProjTexture.m_oMatrix2 = MultiplyMatrices( MultiplyMatrices( oPerspective, s_oProjScale ), s_oProjOffset );

		// The texture scale of the pattern
		rNewConfig.m_uiTexScaleDataIndex = pBuilding->m_iTexScaleIndex;
		TASSERT( rNewConfig.m_uiTexScaleDataIndex >= 0 && rNewConfig.m_uiTexScaleDataIndex < TWorldShaderHAL::MAX_TEX_SCALES );

		const PatternSetting* pSetting = FindPatternSetting( GetPatternSettings(), pTerrain->m_szModel );
		TFLOAT                fScaleU  = fExtentX / pSetting->m_fSizeX;
		TFLOAT                fScaleV  = fExtentY / pSetting->m_fSizeY;

		TWorldShaderHAL::TexScale* pTexScale = pWorldShader->GetTexScale( rNewConfig.m_uiTexScaleDataIndex );
		pTexScale->m_fScaleU                 = fScaleU;
		pTexScale->m_fScaleV                 = fScaleV;
		pBuilding->m_fUnkBC                  = fScaleU;
		pBuilding->m_fUnkC0                  = fScaleV;
		pBuilding->m_uiUnkC6                 = TUINT8( rNewConfig.m_iPattern );
	}
	else if ( a_eSolid == Solid_Unk1 )
	{
		// Faces the painter (there is no solid 1: the config has none)
		ABlobEntity*    pBlob    = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( a_iPlayer ) ) );
		const TVector4& vBlobPos = pBlob->GetPhysicsMatrix().GetTranslation();

		TVector4 vDir( vCentre.x - vBlobPos.x, vCentre.y - vBlobPos.y, vCentre.z - vBlobPos.z, vCentre.w );
		vDir.Normalise();
		vSolidPos = vBlobPos;

		TMatrix44 oLookAt = TMatrix44::IDENTITY;
		oLookAt.LookAtDirection( vDir, s_vForward );
		TMatrix44 oRotation = TMatrix44::IDENTITY;
		oRotation.RotateX( 1.5707964f );

		TMatrix44 oSolidMatrix;
		oSolidMatrix.Multiply( oLookAt, oRotation );
		oSolidMatrix.Scale( s_fUnk1SolidScale, s_fUnk1SolidScale, s_fUnk1SolidScale );
		rNewConfig.m_apSolids[ rNewConfig.m_eSolid ]->GetTransform().SetMatrix( oSolidMatrix );
	}

	if ( rNewConfig.m_eSolid != Solid_Unk1 )
		rNewConfig.m_apSolids[ eSolid ]->GetTransform().SetScale( vSolidScale );

	rNewConfig.m_apSolids[ eSolid ]->GetTransform().SetTranslate( vSolidPos.AsVector3() );

	// The solid grows
	TASSERT( HasAnimationFinished( rNewConfig.m_apSolids[ eSolid ] ) );
	rNewConfig.m_apSolids[ eSolid ]->GetSkeletonInstance()->AddAnimationFull( 0, 1.0f, 0.0f, 0.0f, 0 );
	TASSERT( TFALSE == HasAnimationFinished( rNewConfig.m_apSolids[ eSolid ] ) );

	m_iCurConfig = ( m_iCurConfig + 1 ) % GetNumUsableConfigs( m_iNumCones );
}

// $deBlob: FUNCTION 0055c9c0
void AJellyShader_DX11::BuildingConfig::SetVisible( TBOOL a_bVisible )
{
	TerrainTypes::CellInstance* pTerrain = TSTATICCAST( ABuildingInstance, m_pInstance )->GetTerrainInstance();
	TUINT32                     uiFlags  = pTerrain->m_uiFlags;

	// Flag 0x10 and grass instances are drawn by the terrain: their own visible flag
	if ( ( uiFlags & 0x10 ) || ( uiFlags & 7 ) == TerrainTypes::InstanceType_Grass )
	{
		if ( a_bVisible )
			pTerrain->m_uiFlags = uiFlags | TerrainTypes::InstanceFlags_Visible;
		else
			pTerrain->m_uiFlags = uiFlags & ~TerrainTypes::InstanceFlags_Visible;

		return;
	}

	m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, a_bVisible );
}

// $deBlob: FUNCTION 0055ca20
void __stdcall AJellyShader_DX11::SetSolidMaterials( TModelLOD* a_pLOD, TINT a_iPass, T2DList<MatAlloc>* a_pMaterials )
{
	T2DList<MatAlloc>& rMaterials = a_pMaterials[ a_iPass ];
	auto               it         = rMaterials.Begin();
	TINT               iNumMeshes = TMath::Min( a_pLOD->GetMeshCount(), MAX_ORIGINAL_MATS );
	TASSERT( rMaterials.Size() == iNumMeshes );

	for ( TINT i = 0; i < iNumMeshes; i++ )
	{
		a_pLOD->GetMesh( i )->SetMaterial( it->m_pOriginalMat );
		it++;
	}
}

// $deBlob: FUNCTION 0055cb00
void __stdcall AJellyShader_DX11::SetInstanceMaterials( TerrainTypes::InstanceMeshes* a_pMeshes, TINT a_iPass, T2DList<MatAlloc>* a_pMaterials )
{
	T2DList<MatAlloc>& rMaterials = a_pMaterials[ a_iPass ];
	auto               it         = rMaterials.Begin();
	TINT               iNumMeshes = TMath::Min( TINT( a_pMeshes->m_uiNumMeshes ), MAX_ORIGINAL_MATS );
	TASSERT( rMaterials.Size() == iNumMeshes );

	for ( TINT i = 0; i < iNumMeshes; i++ )
	{
		TWorldMaterialHAL* pMat = TSTATICCAST( TWorldMaterialHAL, it->m_pMat );
		TASSERT( pMat->GetBlendMode() == BLENDMODE_JELLY );
		TASSERT( pMat->GetPass() == s_iBuildingPasses[ a_iPass ] );
		a_pMeshes->m_pMeshes[ i ].m_pMaterial = pMat;
		it++;
	}
}

// $deBlob: FUNCTION 0055cc50
void __stdcall AJellyShader_DX11::CopyMaterial( TMaterial* a_pSource, TMaterial* a_pDest )
{
	TASSERT( a_pDest->GetClass() == a_pSource->GetClass() );

	a_pDest->SetTextureNum( a_pSource->GetTextureNum() );

	for ( TUINT i = 0; i < a_pSource->GetTextureNum(); i++ )
		a_pDest->SetTexture( i, a_pSource->GetTexture( i ) );

	if ( a_pSource->GetClass()->IsA( &TGetClass( TWorldMaterial ) ) )
	{
		TASSERT( a_pSource->IsA( &TGetClass( TWorldMaterialHAL ) ) );
		TASSERT( a_pDest->IsA( &TGetClass( TWorldMaterialHAL ) ) );

		TWorldMaterialHAL* pSource = TSTATICCAST( TWorldMaterialHAL, a_pSource );
		TWorldMaterialHAL* pDest   = TSTATICCAST( TWorldMaterialHAL, a_pDest );

		for ( TINT i = 0; i < TWorldMaterialHAL::s_iMaxPasses; i++ )
		{
			pDest->SetTexture( i, pSource->GetTexture( i ) );
			pDest->m_iUVStream[ i ]            = pSource->m_iUVStream[ i ];
			pDest->m_aMultiPassBlendTypes[ i ] = pSource->m_aMultiPassBlendTypes[ i ];
			pDest->SetUVMode( i, pSource->m_aUVModes[ i ] );
			pDest->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ] = pSource->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ];
			pDest->m_iUserDataIndices[ TWorldMaterialHAL::UData_1 ] = pSource->m_iUserDataIndices[ TWorldMaterialHAL::UData_1 ];
			TASSERT( pDest->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ] == -1 );
		}

		pDest->SetMaterialName( pSource->GetMaterialName() );
	}

	a_pDest->SetUVAnimation( a_pSource->GetUVAnimation() );
}

// $deBlob: FUNCTION 0055cf80
void AJellyShader_DX11::ResetConfig( BuildingConfig& a_oConfig, TBOOL a_bRestoreTextures, TBOOL a_bKeepMaterials )
{

	if ( !a_bKeepMaterials )
		a_bKeepMaterials = a_oConfig.m_bIsReset;

	TerrainTypes::CellInstance*   pTerrain   = TSTATICCAST( ABuildingInstance, a_oConfig.m_pInstance )->GetTerrainInstance();
	TerrainTypes::InstanceMeshes* pMeshes    = pTerrain->m_pSymbolData;
	TINT                          iNumMeshes = TMath::Min( TINT( pMeshes->m_uiNumMeshes ), MAX_ORIGINAL_MATS );

	for ( TINT i = 0; i < iNumMeshes; i++ )
	{
		if ( !a_bKeepMaterials )
		{
			// The building gets its own materials back
			pMeshes->m_pMeshes[ i ].m_pMaterial = TSTATICCAST( TWorldMaterialHAL, a_oConfig.m_aOriginalMats[ i ] );
			TASSERT( TSTATICCAST( TWorldMaterialHAL, a_oConfig.m_aOriginalMats[ i ] )->GetBlendMode() != BLENDMODE_JELLY );
			TASSERT( TSTATICCAST( TWorldMaterialHAL, a_oConfig.m_aOriginalMats[ i ] )->GetPass() == 0 );
		}

		if ( !a_bRestoreTextures )
			continue;

		TWorldMaterialHAL* pWorldMat = pMeshes->m_pMeshes[ i ].m_pMaterial;

		if ( a_oConfig.m_eSolid == Solid_Cone )
		{
			// The grass gets its texture
			TASSERT( pWorldMat->GetTextureNum() > 0 );
			TerrainTypes::GrassData* pGrassData = TSTATICCAST( TerrainTypes::GrassData, pTerrain->m_pTypeData );
			TASSERT( pGrassData->numtextures > 0 );
			pWorldMat->SetTexture( pWorldMat->GetTextureNum() - 1, TTextureManager::GetSingleton()->FindTexture( pGrassData->m_ppTextures[ 0 ] ) );
			continue;
		}

		const TCHAR* szName = pWorldMat->GetMaterialName();

		if ( szName != TNULL && ( TStringManager::String8Compare( szName, "window", 6 ) == 0 || TStringManager::String8Compare( szName, "sticker", 7 ) == 0 ) )
			continue;

		if ( a_oConfig.m_iPattern != -1 )
		{
			// The building keeps the pattern of its painter
			TINT iNumTextures = pWorldMat->GetTextureNum();
			TINT iSlot        = iNumTextures;

			for ( TINT k = 0; k < iNumTextures; k++ )
			{
				if ( pWorldMat->GetTexture( k ) != TNULL && pWorldMat->GetUVMode( k ) == TWorldMaterialHAL::UVMODE_TEXSCALE )
				{
					iSlot = k;
					break;
				}
			}

			pWorldMat->SetTextureNum( iSlot + 1 );
			pWorldMat->SetTexture( iSlot, APlayerManager::GetSingleton()->GetPatternTexture( a_oConfig.m_iPattern )->m_pTex );
			pWorldMat->SetMultiPassBlendType( iSlot, TWorldMaterialHAL::MULTIPASSBLENDTYPE( APlayerManager::GetSingleton()->GetPatternTexture( a_oConfig.m_iPattern )->m_eBlendType ) );
			TASSERT( a_oConfig.m_uiTexScaleDataIndex > -1 );
			pWorldMat->SetUserDataIndex( TWorldMaterialHAL::UData_1, a_oConfig.m_uiTexScaleDataIndex );
			pWorldMat->SetUVMode( iSlot, TWorldMaterialHAL::UVMODE_TEXSCALE );
		}
	}

	// The jelly materials go back to the free list
	for ( TINT i = 0; i < NUM_PASSES; i++ )
	{
		T2DList<MatAlloc>& rMaterials = a_oConfig.m_aMaterials[ i ];

		while ( !rMaterials.IsEmpty() )
		{
			MatAlloc* pAlloc = rMaterials.PopFront();

			if ( pAlloc->m_pMat != TNULL )
				pAlloc->m_pMat->SetUVAnimation( TNULL );

			if ( pAlloc->m_pOriginalMat != TNULL )
				pAlloc->m_pOriginalMat->SetUVAnimation( TNULL );

			for ( TUINT j = 0; j < pAlloc->m_pMat->GetTextureNum(); j++ )
				pAlloc->m_pMat->SetTexture( j, TNULL );

			m_oMatFreeList.PushBack( pAlloc );
		}
	}

	a_oConfig.m_bIsReset = TTRUE;
}

// $deBlob: FUNCTION 00559ea0
void AJellyShader_DX11::PrepareConfigs( TFLOAT a_fDeltaTime )
{

	TASSERT( TWorldShader::GetSingleton() == TNULL || TWorldShader::GetSingleton()->IsA( &TGetClass( TWorldShaderHAL ) ) );
	TINT iNumConfigs = GetNumUsableConfigs( m_iNumCones );

	for ( TINT i = 0; i < iNumConfigs; i++ )
	{
		BuildingConfig& rCurBuilding = m_oBuildingConfigs[ i ];

		if ( rCurBuilding.m_pInstance == TNULL )
			continue;

		TASSERT( rCurBuilding.m_bIsReset );

		TerrainTypes::CellInstance*   pTerrain   = TSTATICCAST( ABuildingInstance, rCurBuilding.m_pInstance )->GetTerrainInstance();
		TerrainTypes::InstanceMeshes* pMeshes    = pTerrain->m_pSymbolData;
		TINT                          iNumMeshes = TMath::Min( TINT( pMeshes->m_uiNumMeshes ), MAX_ORIGINAL_MATS );

		for ( TINT j = 0; j < iNumMeshes; j++ )
		{
			TWorldMaterialHAL* pOriginalMat   = pMeshes->m_pMeshes[ j ].m_pMaterial;
			rCurBuilding.m_aOriginalMats[ j ] = pOriginalMat;
			TASSERT( TSTATICCAST( TWorldMaterialHAL, rCurBuilding.m_aOriginalMats[ j ] )->GetBlendMode() != BLENDMODE_JELLY );
			TASSERT( pOriginalMat->GetPass() == 0 );

			const TCHAR* szName    = pOriginalMat->GetMaterialName();
			TBOOL        bIsWindow = szName != TNULL && ( TStringManager::String8Compare( szName, "window", 6 ) == 0 || TStringManager::String8Compare( szName, "sticker", 7 ) == 0 );

			for ( TINT iPass = 0; iPass < NUM_PASSES; iPass++ )
			{
				MatAlloc* pAlloc = m_oMatFreeList.Front();
				pAlloc->Remove();

				CopyMaterial( pOriginalMat, pAlloc->m_pMat );
				TWorldMaterialHAL* pWorldMat = TSTATICCAST( TWorldMaterialHAL, pAlloc->m_pMat );
				pWorldMat->SetBlendMode( BLENDMODE_JELLY, TUINT8( s_iBuildingPasses[ iPass ] ) );

				if ( rCurBuilding.m_eSolid == Solid_Cone )
				{
					if ( iPass == 3 )
					{
						// The grass is drawn with its texture
						TASSERT( pWorldMat->GetTextureNum() > 0 );
						TerrainTypes::GrassData* pGrassData = TSTATICCAST( TerrainTypes::GrassData, pTerrain->m_pTypeData );
						TASSERT( pGrassData->numtextures > 0 );
						pWorldMat->SetTexture( pWorldMat->GetTextureNum() - 1, TTextureManager::GetSingleton()->FindTexture( pGrassData->m_ppTextures[ 0 ] ) );
					}
				}
				else if ( iPass == 2 )
				{
					// The colour is projected with the splat textures
					TASSERT( pWorldMat->GetTextureNum() >= 2 );
					TINT iNumTextures = pWorldMat->GetTextureNum();
					TASSERT( iNumTextures < TWorldMaterial::TEXTURETYPE_NUMOF );
					pWorldMat->SetTextureNum( iNumTextures + 1 );
					TASSERT( rCurBuilding.m_iCurSplatTex > -1 && rCurBuilding.m_iCurSplatTex < NUM_SPLAT_TEXTURES );
					pWorldMat->SetTexture( iNumTextures, rCurBuilding.m_bSplatTextures ? m_apSplatTextures[ rCurBuilding.m_iCurSplatTex ] : m_apProjTextures[ rCurBuilding.m_iCurSplatTex ] );
					pWorldMat->SetMultiPassBlendType( iNumTextures, TWorldMaterialHAL::MULTIPASSBLENDTYPE_PROJECTEDTEX );
					pWorldMat->SetUVMode( iNumTextures, TWorldMaterialHAL::UVMODE_PROJECTED );
					TASSERT( rCurBuilding.m_uiProjTexDataIndex > -1 );
					pWorldMat->SetUserDataIndex( TWorldMaterialHAL::UData_0, rCurBuilding.m_uiProjTexDataIndex );
				}
				else if ( !bIsWindow && iPass == 3 && rCurBuilding.m_iPattern != -1 )
				{
					// The pattern of the painter
					TINT iNumTextures = pWorldMat->GetTextureNum();
					TINT iSlot        = iNumTextures;

					for ( TINT k = 0; k < iNumTextures; k++ )
					{
						if ( pWorldMat->GetTexture( k ) != TNULL && pWorldMat->GetUVMode( k ) == TWorldMaterialHAL::UVMODE_TEXSCALE )
						{
							iSlot = k;
							break;
						}
					}

					pWorldMat->SetTextureNum( iSlot + 1 );
					pWorldMat->SetTexture( iSlot, APlayerManager::GetSingleton()->GetPatternTexture( rCurBuilding.m_iPattern )->m_pTex );
					pWorldMat->SetMultiPassBlendType( iSlot, TWorldMaterialHAL::MULTIPASSBLENDTYPE( APlayerManager::GetSingleton()->GetPatternTexture( rCurBuilding.m_iPattern )->m_eBlendType ) );
					TASSERT( rCurBuilding.m_uiTexScaleDataIndex > -1 );
					pWorldMat->SetUserDataIndex( TWorldMaterialHAL::UData_1, rCurBuilding.m_uiTexScaleDataIndex );
					pWorldMat->SetUVMode( iSlot, TWorldMaterialHAL::UVMODE_TEXSCALE );
				}

				rCurBuilding.m_aMaterials[ iPass ].PushBack( pAlloc );
			}
		}

		rCurBuilding.m_bIsReset = TFALSE;
		SetBuildingVisible( rCurBuilding.m_pInstance, TFALSE );

		// The frames of the splat
		rCurBuilding.m_fSplatTime += a_fDeltaTime;

		if ( rCurBuilding.m_fSplatTime > s_fSplatFrameTime )
		{
			rCurBuilding.m_fSplatTime = 0.0f;
			TASSERT( rCurBuilding.m_iCurSplatTex < NUM_SPLAT_TEXTURES && rCurBuilding.m_iCurSplatTex >= 0 );

			if ( rCurBuilding.m_iCurSplatTex < NUM_SPLAT_TEXTURES - 1 )
				rCurBuilding.m_iCurSplatTex++;
		}
	}
}

// $deBlob: FUNCTION 0055a670
void AJellyShader_DX11::RenderConfigs( TFLOAT a_fDeltaTime, ACamera* a_pCamera )
{
	TBOOL bMultiplayer = TFALSE;

	if ( AGameStatus::GetSingleton() != TNULL )
		bMultiplayer = AGameStatus::GetSingleton()->GetMultiPlayerGameType() != AGameStatus::GAMETYPE_MP_NONE;

	TINT     iNumConfigs = GetNumUsableConfigs( m_iNumCones );
	TRender* pRender     = TRender::GetSingleton();

	for ( TINT i = 0; i < iNumConfigs; i++ )
	{
		BuildingConfig& rCurBuilding = m_oBuildingConfigs[ i ];

		if ( rCurBuilding.m_pInstance == TNULL || rCurBuilding.m_bIsReset == TTRUE )
			continue;

		TBOOL bAllCameras = TFALSE;
		TBOOL bVisible    = TTRUE;

		if ( s_bCullSolids )
		{
			TFLOAT fDistanceSq;
			bVisible = !IsSolidCulled( rCurBuilding.m_pInstance->GetModelInstance(), a_pCamera, &fDistanceSq );

			if ( bMultiplayer )
			{
				// The solid grows once all the cameras rendered it
				TUINT8 uiCameraBit = TUINT8( 1 << ARenderer::GetCurrentCameraIndex() );
				TUINT8 uiAllBits   = TUINT8( ( 1 << ACameraManager::GetSingleton()->GetNumActiveCameras() ) - 1 );

				if ( !bVisible )
				{
					rCurBuilding.m_uiUnkF0 |= uiCameraBit;

					if ( ( rCurBuilding.m_uiUnkF0 & uiAllBits ) != uiAllBits )
					{
						ResetConfig( rCurBuilding, TFALSE, TFALSE );
						SetBuildingVisible( rCurBuilding.m_pInstance, TFALSE );
						continue;
					}

					bAllCameras = TTRUE;
				}
				else if ( s_fSolidCameraDistanceSq < fDistanceSq || ( rCurBuilding.m_uiUnkF0 & uiCameraBit ) )
				{
					rCurBuilding.m_uiUnkF0 |= uiCameraBit;
					bAllCameras = ( rCurBuilding.m_uiUnkF0 & uiAllBits ) == uiAllBits;

					if ( !bAllCameras )
					{
						// Far away or rendered by this camera already: the building in its new colour
						SetBuildingVisible( rCurBuilding.m_pInstance, TTRUE );
						rCurBuilding.m_pInstance->SetLocalAmbientColour( rCurBuilding.m_uiNewColour );
						ATerrain2Interface::RenderInstance( TSTATICCAST( ABuildingInstance, rCurBuilding.m_pInstance )->GetTerrainInstance(), TNULL );
						pRender->FlushShaders();
						rCurBuilding.m_pInstance->SetLocalAmbientColour( rCurBuilding.m_uiOldColour );
						SetBuildingVisible( rCurBuilding.m_pInstance, TFALSE );
						ResetConfig( rCurBuilding, TFALSE, TFALSE );
						continue;
					}
				}
			}
		}

		TFLOAT fSpeedScale = s_fSolidSpeed;

		if ( bMultiplayer )
			fSpeedScale = s_fMPSolidSpeed;
		else if ( rCurBuilding.m_eSolid == Solid_Cone )
			fSpeedScale = s_fConeSolidSpeed;

		TBOOL bUpdate = bVisible && !bAllCameras && !rCurBuilding.m_bUnkF1;
		TINT  eSolid  = rCurBuilding.m_eSolid;

		if ( ( ( eSolid != Solid_Drip && eSolid != Solid_MPDrip && eSolid != Solid_FastDrip ) || rCurBuilding.m_iCurSplatTex > 2 ) && bUpdate )
		{
			// The solid grows (the drips once the splat is over)
			TSkeletonInstance* pSkeletonInstance = rCurBuilding.m_apSolids[ eSolid ]->GetSkeletonInstance();
			pSkeletonInstance->UpdateTime( rCurBuilding.m_fSpeed * a_fDeltaTime * fSpeedScale );
			rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ]->GetSkeletonInstance()->UpdateState( TFALSE );
			rCurBuilding.m_bUnkF1 = TTRUE;
		}

		TAnimation* pAnimation = TNULL;

		if ( ( !bMultiplayer && !bVisible ) || bAllCameras || rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ] == TNULL ||
		     ( pAnimation = rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ]->GetSkeletonInstance()->IsAnimating( 0 ) ) == TNULL ||
		     pAnimation->GetTotalTime() > pAnimation->GetSeqTime() )
		{
			// The solid is over (or not seen): the building takes its new colour
			SetBuildingVisible( rCurBuilding.m_pInstance, TTRUE );
			rCurBuilding.m_pInstance->SetLocalAmbientColour( rCurBuilding.m_uiNewColour );
			ResetConfig( rCurBuilding, TTRUE, TFALSE );
			rCurBuilding.m_pInstance->Render();
			rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ]->GetSkeletonInstance()->RemoveAllAnimations( 0.0f );
			rCurBuilding.m_pInstance = TNULL;

			if ( rCurBuilding.m_eSolid == Solid_Cone )
				m_iNumCones--;

			continue;
		}

		TASSERT( TFALSE == rCurBuilding.m_bIsReset );
		T2ModelPtr pSolidModel = rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ]->GetModel();

		if ( s_bDebugSolidOnly )
		{
			// Only the solid, without its jelly materials
			pRender->FlushShaders();

			for ( TINT j = 0; j < pSolidModel->GetLOD( 0 ).GetMeshCount(); j++ )
			{
				ASkinMaterialHAL* pMat = TSTATICCAST( ASkinMaterialHAL, pSolidModel->GetLOD( 0 ).GetMesh( j )->GetMaterial() );
				TASSERT( ( pMat == TNULL ) || pMat->IsA( &TGetClass( ASkinMaterialHAL ) ) );
				pMat->SetBlendMode( 0, 0 );
				pMat->m_iPass          = 0;
				pMat->m_aTevModes[ 0 ] = 0;
			}

			rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ]->Render();
			SetBuildingVisible( rCurBuilding.m_pInstance, TTRUE );
			ResetConfig( rCurBuilding, TFALSE, TFALSE );
			pRender->FlushShaders();
			return;
		}

		if ( bVisible )
		{
			TWorldShaderHAL::s_bInstancing = TFALSE;
			SetBuildingVisible( rCurBuilding.m_pInstance, TTRUE );

			TModelLOD*                    pSolidLOD  = &pSolidModel->GetLOD( 0 );
			TerrainTypes::InstanceMeshes* pMeshes    = TSTATICCAST( ABuildingInstance, rCurBuilding.m_pInstance )->GetTerrainInstance()->m_pSymbolData;
			T2DList<MatAlloc>*            pSolidMats = m_aSolidMaterials[ rCurBuilding.m_eSolid ];

			if ( s_abRenderPasses[ 0 ] )
			{
				rCurBuilding.m_pInstance->SetLocalAmbientColour( rCurBuilding.m_uiOldColour );
				SetInstanceMaterials( pMeshes, 0, rCurBuilding.m_aMaterials );
				RenderBuilding( rCurBuilding.m_pInstance, TNULL );
				pRender->FlushShaders();
			}

			if ( s_abRenderPasses[ 1 ] )
			{
				SetSolidMaterials( pSolidLOD, 0, pSolidMats );
				rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ]->Render();
			}

			if ( s_abRenderPasses[ 2 ] )
			{
				SetSolidMaterials( pSolidLOD, 1, pSolidMats );
				rCurBuilding.m_apSolids[ rCurBuilding.m_eSolid ]->Render();
				pRender->FlushShaders();
			}

			if ( s_abRenderPasses[ 3 ] )
			{
				SetInstanceMaterials( pMeshes, 1, rCurBuilding.m_aMaterials );
				RenderBuilding( rCurBuilding.m_pInstance, TNULL );
				pRender->FlushShaders();
			}

			if ( s_abRenderPasses[ 4 ] )
			{
				rCurBuilding.m_pInstance->SetLocalAmbientColour( 0xFFFFFFFF );
				SetInstanceMaterials( pMeshes, 2, rCurBuilding.m_aMaterials );
				RenderBuilding( rCurBuilding.m_pInstance, TNULL );
				pRender->FlushShaders();
			}

			if ( s_abRenderPasses[ 5 ] )
			{
				rCurBuilding.m_pInstance->SetLocalAmbientColour( rCurBuilding.m_uiNewColour );
				SetInstanceMaterials( pMeshes, 3, rCurBuilding.m_aMaterials );
				RenderBuilding( rCurBuilding.m_pInstance, TNULL );
				pRender->FlushShaders();
			}

			if ( s_abRenderPasses[ 6 ] && rCurBuilding.m_eSolid != Solid_Cone )
			{
				SetInstanceMaterials( pMeshes, 4, rCurBuilding.m_aMaterials );
				RenderBuilding( rCurBuilding.m_pInstance, "window" );
			}

			SetBuildingVisible( rCurBuilding.m_pInstance, TFALSE );
			pRender->FlushShaders();
			ResetConfig( rCurBuilding, TFALSE, TFALSE );
			TWorldShaderHAL::s_bInstancing = TTRUE;
		}
	}
}

// DOES_NOTHING: the jelly shader's render packet and post render callbacks
static TBOOL __stdcall RenderCallbackFalse( void* a_pData )
{
	return TFALSE;
}

// $deBlob: FUNCTION 0055b290
TBOOL AJellyShader_DX11::LoadSolids()
{
	TWorldShaderHAL* pWorldShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
	ASkinShader*     pSkinShader  = ASkinShader::GetSingleton();

	TWorldShaderHAL::s_fnRenderCallback         = TREINTERPRETCAST( TWorldShaderHAL::t_RenderCallback, RenderCallbackFalse );
	TWorldMaterialHAL::s_fnSetBlendModeCallback = SetBlendModeCallback;
	TWorldMaterialHAL::s_fnPreRenderCallback    = PreRenderCallback;
	TWorldMaterialHAL::s_fnInstanceCallback     = InstanceCallback;
	TWorldMaterialHAL::s_fnPostRenderCallback   = TREINTERPRETCAST( TWorldMaterialHAL::t_RenderCallback, RenderCallbackFalse );

	// The jelly world materials
	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
	{
		TMaterial* pMat = pWorldShader->CreateMaterial( TNULL );
		TASSERT( ( pMat == TNULL ) || pMat->IsA( &TGetClass( TWorldMaterialHAL ) ) );
		TSTATICCAST( TWorldMaterialHAL, pMat )->Create( BLENDMODE_JELLY );

		m_oMatAlloc[ i ].m_pMat         = pMat;
		m_oMatAlloc[ i ].m_pOriginalMat = TNULL;
		m_oMatFreeList.PushFront( &m_oMatAlloc[ i ] );
	}

	// The solids
	T2ModelPtr oDripSolid                                  = TModelManager::GetSingleton()->FindModel( "solid.txs" );
	oDripSolid->GetLOD( 0 ).m_RenderVolume.GetRadius()     = 0.0f;
	T2ModelPtr oFastDripSolid                              = TModelManager::GetSingleton()->FindModel( "solid_fast.txs" );
	oFastDripSolid->GetLOD( 0 ).m_RenderVolume.GetRadius() = 0.0f;
	T2ModelPtr oMPDripSolid                                = TModelManager::GetSingleton()->FindModel( "solid_multiplayer.txs" );
	oMPDripSolid->GetLOD( 0 ).m_RenderVolume.GetRadius()   = 0.0f;
	T2ModelPtr oConeSolid                                  = TModelManager::GetSingleton()->FindModel( "grass_solid.txs" );
	oConeSolid->GetLOD( 0 ).m_RenderVolume.GetRadius()     = 0.0f;

	TASSERT( oDripSolid->GetSkeleton()->GetSequenceCount() > 0 );
	TASSERT( oFastDripSolid->GetSkeleton()->GetSequenceCount() > 0 );
	TASSERT( oMPDripSolid->GetSkeleton()->GetSequenceCount() > 0 );
	TASSERT( oConeSolid->GetSkeleton()->GetSequenceCount() > 0 );
	TASSERT( oDripSolid->GetLOD( 0 ).GetMeshCount() == 1 );
	TASSERT( oFastDripSolid->GetLOD( 0 ).GetMeshCount() == 1 );
	TASSERT( oMPDripSolid->GetLOD( 0 ).GetMeshCount() == 1 );
	TASSERT( oConeSolid->GetLOD( 0 ).GetMeshCount() == 1 );

	m_apSolidMeshMaterials[ Solid_Drip ]     = oDripSolid->GetLOD( 0 ).GetMesh( 0 )->GetMaterial();
	m_apSolidMeshMaterials[ Solid_FastDrip ] = oFastDripSolid->GetLOD( 0 ).GetMesh( 0 )->GetMaterial();
	m_apSolidMeshMaterials[ Solid_MPDrip ]   = oMPDripSolid->GetLOD( 0 ).GetMesh( 0 )->GetMaterial();
	m_apSolidMeshMaterials[ Solid_Cone ]     = oConeSolid->GetLOD( 0 ).GetMesh( 0 )->GetMaterial();

	for ( TINT i = 0; i < NUM_BUILDING_CONFIGS; i++ )
	{
		BuildingConfig& rConfig = m_oBuildingConfigs[ i ];
		rConfig.m_pInstance     = TNULL;

		rConfig.m_apSolids[ Solid_Drip ] = oDripSolid->CreateInstance();
		rConfig.m_apSolids[ Solid_Drip ]->SetDistanceFade( TFALSE );
		rConfig.m_apSolids[ Solid_FastDrip ] = oFastDripSolid->CreateInstance();
		rConfig.m_apSolids[ Solid_FastDrip ]->SetDistanceFade( TFALSE );
		rConfig.m_apSolids[ Solid_Unk1 ]   = TNULL;
		rConfig.m_apSolids[ Solid_MPDrip ] = oMPDripSolid->CreateInstance();
		rConfig.m_apSolids[ Solid_MPDrip ]->SetDistanceFade( TFALSE );
		rConfig.m_apSolids[ Solid_Cone ] = oConeSolid->CreateInstance();
		rConfig.m_apSolids[ Solid_Cone ]->SetDistanceFade( TFALSE );

		rConfig.m_bIsReset       = TTRUE;
		rConfig.m_eSolid         = Solid_Drip;
		rConfig.m_bSplatTextures = TFALSE;
		rConfig.m_bUnkF1         = TFALSE;
	}

	// The jelly skin materials of the solids' meshes (one per pass)
	T2ModelPtr aSolids[ NUM_SOLIDS ] = { oDripSolid, T2ModelPtr(), oMPDripSolid, oConeSolid, oFastDripSolid };

	for ( TINT i = 0; i < NUM_SOLIDS; i++ )
	{
		if ( !aSolids[ i ] )
			continue;

		TINT iNumMeshes = TMath::Min( aSolids[ i ]->GetLOD( 0 ).GetMeshCount(), MAX_ORIGINAL_MATS );

		for ( TINT j = 0; j < iNumMeshes; j++ )
		{
			for ( TINT iPass = 0; iPass < NUM_SOLID_PASSES; iPass++ )
			{
				MatAlloc* pAlloc = m_oMatFreeList.Front();
				pAlloc->Remove();

				TSTATICCAST( TWorldMaterialHAL, pAlloc->m_pMat )->OnDestroy();
				delete pAlloc->m_pMat;
				pAlloc->m_pMat = TNULL;

				TMaterial* pMat = pSkinShader->CreateMaterial( TNULL );
				TASSERT( ( pMat == TNULL ) || pMat->IsA( &TGetClass( ASkinMaterialHAL ) ) );
				pAlloc->m_pOriginalMat = pMat;
				TSTATICCAST( ASkinMaterialHAL, pMat )->Create( BLENDMODE_JELLY );
				TSTATICCAST( ASkinMaterialHAL, pMat )->SetBlendMode( BLENDMODE_JELLY, s_iSolidPasses[ iPass ] );

				m_aSolidMaterials[ i ][ iPass ].PushBack( pAlloc );
			}
		}
	}

	for ( TINT i = 0; i < NUM_SPLAT_TEXTURES; i++ )
		m_apProjTextures[ i ] = TNULL;

	TTextureManager* pTexManager = TTextureManager::GetSingleton();
	m_apSplatTextures[ 0 ]       = pTexManager->FindTexture( "splatlong_frame1.tga" );
	m_apSplatTextures[ 1 ]       = pTexManager->FindTexture( "splatlong_frame2.tga" );
	m_apSplatTextures[ 2 ]       = pTexManager->FindTexture( "splatlong_frame3.tga" );
	m_apSplatTextures[ 3 ]       = pTexManager->FindTexture( "splatlong_frame4.tga" );

	for ( TINT i = 0; i < NUM_SPLAT_TEXTURES; i++ )
		m_apSplatTextures[ i ]->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

	return TTRUE;
}

// $deBlob: FUNCTION 0055bd40
TBOOL AJellyShader_DX11::UnloadSolids()
{
	TWorldShaderHAL::s_fnRenderCallback         = TNULL;
	TWorldMaterialHAL::s_fnSetBlendModeCallback = TNULL;
	TWorldMaterialHAL::s_fnPreRenderCallback    = TNULL;
	TWorldMaterialHAL::s_fnInstanceCallback     = TNULL;
	TWorldMaterialHAL::s_fnPostRenderCallback   = TNULL;

	for ( TINT i = 0; i < NUM_BUILDING_CONFIGS; i++ )
	{
		BuildingConfig& rConfig = m_oBuildingConfigs[ i ];
		TASSERT( m_oBuildingConfigs[ i ].m_bIsReset );

		for ( TINT j = 0; j < NUM_SOLIDS; j++ )
		{
			TModelInstance* pSolid = rConfig.m_apSolids[ j ];

			if ( pSolid == TNULL )
				continue;

			// The solid's mesh gets its own material back
			T2ModelPtr pModel    = pSolid->GetModel();
			TModelLOD& rSolidLOD = pModel->GetLOD( 0 );
			TASSERT( rSolidLOD.GetMeshCount() == 1 );
			rSolidLOD.GetMesh( 0 )->SetMaterial( m_apSolidMeshMaterials[ j ] );

			delete pSolid;
			rConfig.m_apSolids[ j ] = TNULL;
		}
	}

	for ( TINT i = 0; i < NUM_SOLIDS; i++ )
	{
		for ( TINT j = 0; j < NUM_SOLID_PASSES; j++ )
		{
			while ( !m_aSolidMaterials[ i ][ j ].IsEmpty() )
				m_oMatFreeList.PushFront( m_aSolidMaterials[ i ][ j ].PopFront() );
		}
	}

	TASSERT( m_oMatFreeList.Size() == MAX_MATERIALS );
	m_oMatFreeList.Clear();

	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
	{
		TASSERT( TFALSE == m_oMatAlloc[ i ].IsLinked() );

		if ( m_oMatAlloc[ i ].m_pMat != TNULL )
		{
			TSTATICCAST( TWorldMaterialHAL, m_oMatAlloc[ i ].m_pMat )->OnDestroy();
			delete m_oMatAlloc[ i ].m_pMat;
		}

		if ( m_oMatAlloc[ i ].m_pOriginalMat != TNULL )
		{
			TSTATICCAST( ASkinMaterialHAL, m_oMatAlloc[ i ].m_pOriginalMat )->OnDestroy();
			delete m_oMatAlloc[ i ].m_pOriginalMat;
		}
	}

	for ( TINT i = 0; i < NUM_SPLAT_TEXTURES; i++ )
	{
		m_apProjTextures[ i ]  = TNULL;
		m_apSplatTextures[ i ] = TNULL;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0055d500
void __stdcall AJellyShader_DX11::RenderBuilding( AModelInstance2* a_pInstance, const TCHAR* a_szMaterialFilter )
{
	TerrainTypes::CellInstance* pTerrain = TSTATICCAST( ABuildingInstance, a_pInstance )->GetTerrainInstance();
	TUINT32                     uiFlags  = pTerrain->m_uiFlags;

	if ( ( ( uiFlags & 0x10 ) || ( uiFlags & 7 ) == TerrainTypes::InstanceType_Grass ) && ( uiFlags & 0x1000000 ) )
	{
		TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
		TMatrix44       oProjection;
		TRenderContext::ComputePerspectiveProjection( oProjection, pContext->GetViewportParameters(), pContext->GetProjectionParameters() );
		RenderLightmappedInstance::RenderInstance( pTerrain->m_pCell, pTerrain, !s_bCullSolids, a_szMaterialFilter, -1.0f, &oProjection );
	}
	else
	{
		TASSERT( a_szMaterialFilter == 0 && "Unsupported." );
		a_pInstance->Render();
	}
}

// $deBlob: FUNCTION 0055d600
TBOOL __stdcall AJellyShader_DX11::IsSolidCulled( TModelInstance* a_pModelInstance, ACamera* a_pCamera, TFLOAT* a_pOutDistanceSq )
{
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

	*a_pOutDistanceSq = TVector3::DistanceSq( vCentre, a_pCamera->GetMatrix().GetTranslation3() );

	return 0.0f < fRadius && !TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), a_pCamera->GetViewPlanes(), 6 );
}

// $deBlob: FUNCTION 0055c0b0
TBOOL __stdcall AJellyShader_DX11::SetBlendModeCallback( TWorldMaterialHAL* a_pMaterial, TWorldMaterial::BLENDMODE a_eBlendMode )
{
	if ( a_eBlendMode != BLENDMODE_JELLY )
		return TFALSE;

	a_pMaterial->SetFlags( TMaterial::FLAGS_GLOW, TTRUE );
	TASSERT( ( a_pMaterial == TNULL ) || a_pMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );

	const TCHAR*       szName       = a_pMaterial->GetMaterialName();
	TUINT8             uiPass       = a_pMaterial->GetPass();
	AJellyShader_DX11* pJellyShader = GetSingleton();
	TOrderTable*       pOrderTable;

	if ( szName != TNULL && ( TStringManager::String8Compare( szName, "window", 6 ) == 0 || TStringManager::String8Compare( szName, "sticker", 7 ) == 0 ) &&
	     ( uiPass == 4 || uiPass == 5 || uiPass == 6 ) )
		pOrderTable = pJellyShader->GetWindowOrderTable( uiPass );
	else
		pOrderTable = pJellyShader->GetOrderTable( uiPass );

	a_pMaterial->SetOrderTable( pOrderTable, 0, TTRUE );
	a_pMaterial->m_eBlendMode = BLENDMODE_JELLY;

	return TTRUE;
}

// The render state of the jelly passes (TWorldMaterialHAL's PreRender callback, and the instance callback through
// InstanceCallback)
// $deBlob: FUNCTION 0055c1a0
TBOOL __stdcall AJellyShader_DX11::PreRenderCallback( TWorldMaterialHAL* a_pMaterial )
{
	TASSERT( ( a_pMaterial == TNULL ) || a_pMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) );
	TRenderDX11* pRender = TRenderDX11::Interface();

	if ( a_pMaterial->GetBlendMode() != BLENDMODE_JELLY )
		return TFALSE;

	TINT               iPass        = a_pMaterial->GetPass();
	AJellyShader_DX11* pJellyShader = GetSingleton();
	TASSERT( a_pMaterial->GetOrderTable() == pJellyShader->GetOrderTable( iPass ) || a_pMaterial->GetOrderTable() == pJellyShader->GetWindowOrderTable( iPass ) );
	TASSERT( 1 != iPass && 2 != iPass && "The solid should be a skin shaded mesh." );

	const TCHAR* szName     = a_pMaterial->GetMaterialName();
	TBOOL        bIsSticker = szName != TNULL && ( TStringManager::String8Compare( szName, "window", 6 ) == 0 || TStringManager::String8Compare( szName, "sticker", 7 ) == 0 );
	TBOOL        bIsWindow  = a_pMaterial->GetMaterialName() != TNULL && TStringManager::String8Compare( a_pMaterial->GetMaterialName(), "window", 6 ) == 0;

	if ( iPass == 0 )
	{
		// The building in its old colour
		s_vAlphaTest.x = 1.0f;
		s_vAlphaTest.y = s_fPass0AlphaRef;
		pRender->SetAlphaUpdate( TTRUE );
		pRender->SetColorUpdate( TTRUE );
		pRender->SetDstAlpha( s_fPass0DstAlpha );

		if ( !bIsWindow )
		{
			if ( bIsSticker )
				pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );

			pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
			return TFALSE;
		}

		pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
		s_vAlphaTest.x = 1.0f;
		s_vAlphaTest.y = 1.0f / 255.0f;
		pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
		return TFALSE;
	}

	if ( iPass == 3 )
	{
		// Subtracts the alpha of the solid
		s_vFogStartEnd.z = 0.0f;
		s_vAlphaTest.x   = 0.0f;
		pRender->SetDstAlpha( -1.0f );
		pRender->SetAlphaUpdate( TTRUE );
		s_iLighting = 1;
		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_REV_SUBTRACT, D3D11_BLEND_ONE, D3D11_BLEND_ONE );
		pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK( !bIsSticker ) );
		pRender->SetColorUpdate( TFALSE );
		pRender->SetAlphaUpdate( TFALSE );

		TVector4 vColour;
		vColour.w = TFLOAT( s_uiPass3Alpha ) / 255.0f;
		vColour.x = TFLOAT( s_uiPass3Alpha | 0x7F ) / 255.0f;
		vColour.y = vColour.x;
		vColour.z = vColour.x;
		pRender->SetVec4InPSBuffer( 7, &vColour, 1 );
		s_iIndirect = 4;
		return TTRUE;
	}

	if ( iPass == 4 )
	{
		// The projected colour
		if ( !bIsSticker )
		{
			pRender->SetAlphaUpdate( TTRUE );
			pRender->SetColorUpdate( TFALSE );
			s_vAlphaTest.y = 0.0f;
		}
		else
		{
			pRender->SetAlphaUpdate( TFALSE );
			pRender->SetColorUpdate( TFALSE );
			s_vAlphaTest.y = 2.0f;
		}

		s_vAlphaTest.x = 1.0f;
		pRender->SetZMode( TTRUE, bIsSticker ? D3D11_COMPARISON_NEVER : D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK( !bIsSticker ) );
		pRender->SetDstAlpha( -1.0f );

		for ( TUINT i = 0; i < a_pMaterial->GetTextureNum(); i++ )
		{
			if ( a_pMaterial->GetUVMode( i ) != TWorldMaterialHAL::UVMODE_PROJECTED )
				continue;

			TTexture* pTex = a_pMaterial->GetTexture( i );

			if ( pTex == TNULL )
				break;

			pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_SRC_ALPHA );
			s_iLighting          = 1;
			s_aiTexCoordGen[ 0 ] = 13;
			TASSERT( a_pMaterial->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ] > -1 );

			TWorldShaderHAL::ProjTexture& rProjTexture = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() )->GetProjTexture( a_pMaterial->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ] );
			TASSERT( D3D11_TEXTURE_ADDRESS_CLAMP == pTex->GetWrapS() && D3D11_TEXTURE_ADDRESS_CLAMP == pTex->GetWrapT() );
			pTex->Bind( 0 );

			// The projection of the texture: the matrix of the projection, a perspective of s_fProjectionFOV and
			// the [-1;1] to [0;1] scale and offset
			TFLOAT fAspect = ( TRender::GetSingleton()->GetAspectRatio() == TRender::ASPECT_RATIO_16_9 ) ? 16.0f / 9.0f : 4.0f / 3.0f;
			TFLOAT fSin;
			TFLOAT fCos;
			DirectX::XMScalarSinCos( &fSin, &fCos, s_fProjectionFOV * 0.017453292f * 0.5f );
			TFLOAT fCot = fCos / fSin;

			TMatrix44 oPerspective = {
				fCot / fAspect, 0.0f, 0.0f, 0.0f,
				0.0f, fCot, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 1.0f,
				0.0f, 0.0f, -0.0f, 0.0f
			};

			TMatrix44 oTexMatrix = MultiplyMatrices( rProjTexture.m_oMatrix1, MultiplyMatrices( MultiplyMatrices( oPerspective, s_oProjScale ), s_oProjOffset ) );

			TVector4 vColour;
			UnpackColour( rProjTexture.m_uiColour, vColour );
			pRender->SetVec4InPSBuffer( 2, &vColour, 1 );
			pRender->SetVec4InVSBuffer( 12, &oTexMatrix, 4 );
			s_iIndirect = 5;
			return TTRUE;
		}

		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_ONE );
		s_iLighting = 1;
		TVector4 vOne( 1.0f, 1.0f, 1.0f, 1.0f );
		pRender->SetVec4InPSBuffer( 7, &vOne, 1 );
		s_iIndirect = 4;
		return TTRUE;
	}

	if ( iPass == 5 )
	{
		// The building in its new colour where the alpha is
		pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
		s_vAlphaTest.x = 1.0f;

		if ( !bIsWindow )
		{
			s_vAlphaTest.y = 128.0f / 255.0f;
			pRender->SetAlphaUpdate( TTRUE );
			pRender->SetDstAlpha( 0.0f );
		}
		else
		{
			s_vAlphaTest.y = 1.0f / 255.0f;
			pRender->SetDstAlpha( -1.0f );
			pRender->SetAlphaUpdate( TTRUE );
		}

		pRender->SetColorUpdate( TTRUE );
		pRender->SetBlendMode( TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA );
		return TFALSE;
	}

	if ( iPass == 6 )
	{
		// The windows
		TASSERT( bIsWindow );
		pRender->SetZMode( TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL );
		pRender->SetAlphaUpdate( TTRUE );
		pRender->SetColorUpdate( TFALSE );
		s_vAlphaTest.x = 1.0f;
		s_vAlphaTest.y = 1.0f / 255.0f;
		pRender->SetDstAlpha( -1.0f );
		pRender->SetBlendMode( TFALSE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA );
		return TFALSE;
	}

	return TTRUE;
}

// TWorldMaterialHAL's instance callback: the PreRender callback
// $deBlob: FUNCTION 0055c9a0
TBOOL __stdcall AJellyShader_DX11::InstanceCallback( TWorldMaterialHAL* a_pMaterial )
{
	return PreRenderCallback( a_pMaterial );
}

// $deBlob: FUNCTION 0055af90
void AJellyShader_DX11::Update()
{
	for ( TINT i = 0; i < NUM_BUILDING_CONFIGS; i++ )
		m_oBuildingConfigs[ i ].m_bUnkF1 = TFALSE;
}

// $deBlob: FUNCTION 0055b100
TBOOL AJellyShader_DX11::IsPainting( AModelInstance2* a_pInstance ) const
{
	for ( TINT i = 0; i < NUM_BUILDING_CONFIGS; i++ )
	{
		if ( m_oBuildingConfigs[ i ].m_pInstance == a_pInstance )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0055b150
void AJellyShader_DX11::FinishAll()
{
	for ( TINT i = 0; i < NUM_BUILDING_CONFIGS; i++ )
	{
		BuildingConfig& rConfig = m_oBuildingConfigs[ i ];

		if ( rConfig.m_pInstance != TNULL )
			FinishConfig( rConfig, TTRUE, rConfig.m_bIsReset );
	}
}

// $deBlob: FUNCTION 0055b1f0
void AJellyShader_DX11::OnInstanceRemoved( TerrainTypes::CellInstance* a_pInstance )
{
	for ( TINT i = 0; i < NUM_BUILDING_CONFIGS; i++ )
	{
		BuildingConfig& rConfig = m_oBuildingConfigs[ i ];

		if ( rConfig.m_pInstance != TNULL && TSTATICCAST( ABuildingInstance, rConfig.m_pInstance )->GetTerrainInstance() == a_pInstance )
			FinishConfig( rConfig, TTRUE, TFALSE );
	}
}

void AJellyShader_DX11::FinishConfig( BuildingConfig& a_rConfig, TBOOL a_bRestoreTextures, TBOOL a_bKeepMaterials )
{
	TerrainTypes::CellInstance* pTerrain = TSTATICCAST( ABuildingInstance, a_rConfig.m_pInstance )->GetTerrainInstance();

	if ( !( ( pTerrain->m_uiFlags >> 4 ) & 1 ) && ( pTerrain->m_uiFlags & 7 ) != TerrainTypes::InstanceType_Grass )
		a_rConfig.m_pInstance->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	else
		pTerrain->m_uiFlags |= 0x800000;

	a_rConfig.m_pInstance->SetLocalAmbientColour( a_rConfig.m_uiNewColour );
	ResetConfig( a_rConfig, a_bRestoreTextures, a_bKeepMaterials );
	a_rConfig.m_apSolids[ a_rConfig.m_eSolid ]->GetSkeletonInstance()->RemoveAllAnimations( 0.0f );
	a_rConfig.m_pInstance = TNULL;
}
