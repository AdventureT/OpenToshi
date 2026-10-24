#include "pch.h"
#include "AGodRay.h"
#include "AEntityFactory.h"
#include "AEntityIOPacket.h"
#include "AEntityLoader.h"
#include "AEntityParams.h"
#include "AAssetStreaming.h"

#include <Render/TModel.h>
#include <Render/TModelManager.h>
#include <Render/TRender.h>
#include <Render/TRenderContext.h>
#include <Render/TTransformObject.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AGodRay ) == 0x88 );

// $deBlob: FUNCTION 007a4a40
// $deBlob: FUNCTION 007a4a50
// $deBlob: FUNCTION 007a4b00
// $deBlob: FUNCTION 007a4ba0
// $deBlob: FUNCTION 007a4bb0
// $deBlob: FUNCTION 005410a0
// $deBlob: FUNCTION 00859eb0
TDEFINE_CLASS( AGodRay );

// $deBlob: FUNCTION 007a4bc0
AGodRay::AGodRay()
{
	m_pModelInstance = TNULL;
	m_uiGodRayFlags  = 0;
	SetHash( TFALSE );
	AEntityFactory::GetSingleton()->AddCallback( this, RenderCallback );
}

// $deBlob: FUNCTION 007a52f0
// $deBlob: FUNCTION 007a4c40
AGodRay::~AGodRay()
{
}

// $deBlob: FUNCTION 007a4c50
void AGodRay::Destroy()
{
	if ( m_pModelInstance )
		delete m_pModelInstance;

	AEntityFactory::GetSingleton()->RemoveCallback( this );
	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 007a4cd0
void AGodRay::ProcessParams( const AEntityParams& a_rParams )
{
	const TCHAR* szMesh;
	a_rParams.GetString( szMesh, "Mesh" );

	TCHAR* szTempString = TStringManager::GetTempString8();
	TCHAR* szTXSName    = szTempString + AAssetStreaming::GetTXSNameFromTRB( szMesh, szTempString, 1024 );

	T2ModelPtr pModel = TModelManager::GetSingleton()->FindModel( szTXSName );

	if ( pModel.IsValid() )
	{
		m_pModelInstance = pModel->CreateInstance();
		m_pModelInstance->GetTransform().SetMatrix( *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams )->m_pMatrix );
	}

	TBOOL bValue = TFALSE;
	a_rParams.GetBool( bValue, "Start Invisible" );

	if ( !bValue )
		m_uiGodRayFlags |= GODRAYFLAG_Visible;
	else
		m_uiGodRayFlags |= GODRAYFLAG_StartInvisible;

	a_rParams.GetBool( bValue, "Toggle Vis When Triggered" );

	if ( bValue )
		m_uiGodRayFlags |= GODRAYFLAG_ToggleVis;
}

// $deBlob: FUNCTION 007a4de0
void AGodRay::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( m_uiGodRayFlags & GODRAYFLAG_ToggleVis )
	{
		TINT iPacket = a_rPacket.m_eMessage;

		if ( iPacket == AEntityIOPacket::MESSAGE_TRIGGER || iPacket == AEntityIOPacket::MESSAGE_TOGGLE || iPacket == AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE )
			m_uiGodRayFlags ^= GODRAYFLAG_Visible;
	}
}

// $deBlob: FUNCTION 007a4c90
void AGodRay::Reset( ABaseEntity* a_pEntity )
{
	ABaseEntity::Reset( a_pEntity );

	if ( m_uiGodRayFlags & GODRAYFLAG_StartInvisible )
		m_uiGodRayFlags &= ~GODRAYFLAG_Visible;
	else
		m_uiGodRayFlags |= GODRAYFLAG_Visible;
}

// $deBlob: FUNCTION 007a4e10
void AGodRay::RenderCallback( void* a_pUserData )
{
	AGodRay* pGodRay = TSTATICCAST( AGodRay, a_pUserData );
	TASSERT( ( pGodRay == TNULL ) || pGodRay->IsA( &TGetClass( AGodRay ) ) );

	if ( !( pGodRay->m_uiGodRayFlags & GODRAYFLAG_Visible ) )
		return;

	TRenderContext* pContext  = TRender::GetSingleton()->GetCurrentContext();
	TModelInstance* pInstance = pGodRay->m_pModelInstance;
	pInstance->SetLOD( 0 );
	pContext->SetSkeletonInstance( pInstance->GetSkeletonInstance() );

	// The ray turns around its up axis to face the camera
	TTransformObject oTransform( pInstance->GetTransform() );
	TMatrix44        oMatrix = oTransform.GetMatrix();
	TVector4         vUp     = oMatrix.AsBasisVector4( BASISVECTOR_UP );
	TVector4         vView   = pContext->GetViewWorldMatrix().AsBasisVector4( BASISVECTOR_FORWARD );

	TVector4 vRight;
	TVector4 vForward;
	vRight.CrossProduct( vUp, vView );
	vForward.CrossProduct( vRight, vUp );
	oMatrix.SetBasisVector4( BASISVECTOR_RIGHT, vRight );
	oMatrix.SetBasisVector4( BASISVECTOR_FORWARD, vForward );
	oTransform.SetMatrix( oMatrix );

	// Faded by its distance to the camera along its facing: full between 5 and 75, none closer or past 200
	const TVector4& rCamera   = pContext->GetViewWorldMatrix().GetTranslation();
	const TVector3& rPosition = oTransform.GetTranslation();
	TFLOAT          fDistance = TMath::Abs( ( rCamera.x * vForward.x + rCamera.y * vForward.y + rCamera.z * vForward.z ) - ( rPosition.y * vForward.y + rPosition.x * vForward.x + rPosition.z * vForward.z ) );
	TFLOAT          fAlpha;

	if ( fDistance <= 30.0f )
		fAlpha = ( 30.0f - fDistance ) / 25.0f;
	else
		fAlpha = ( fDistance - 75.0f ) / 125.0f;

	fAlpha = 1.0f - fAlpha;
	TMath::Clip( fAlpha, 0.0f, 1.0f );

	if ( 0.0f < fAlpha )
	{
		TFLOAT fOldAlpha = pContext->GetAlphaBlend();
		pContext->SetAlphaBlend( fAlpha );
		oTransform.Push();

		T2ModelPtr pModel = pInstance->GetModel();
		TModelLOD& rLOD   = pModel->GetLOD( 0 );

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
			rLOD.GetMeshes()[ i ]->Render();

		oTransform.Pop();
		pContext->SetAlphaBlend( fOldAlpha );
	}
}
