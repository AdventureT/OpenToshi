#include "pch.h"
#include "AToggleVisInstance.h"
#include "ABuildingManager.h"
#include "ATreeInstance.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Platform/DX11/TWorldShader_DX11.h"

#include <Core/TMemoryDebugOn.h>
#include <Render/TRender.h>

TOSHI_NAMESPACE_USING

// The scale curve of the transition in (8 keys, the last key's time ends it)
static const ATreeInstance::ScaleKey s_aScaleInKeys[ 8 ] = {
	{ 0.0f, 0.01f, 0.01f },
	{ 0.33f, 0.01f, 0.01f },
	{ 0.36f, 0.2f, 0.2f },
	{ 0.43f, 0.25f, 2.2f },
	{ 0.46f, 1.1f, 0.6f },
	{ 0.53f, 1.0f, 1.1f },
	{ 0.6f, 1.0f, 0.85f },
	{ 0.66f, 1.0f, 1.0f },
};
static const TINT s_iNumScaleInKeys = 8;

// The scale curve of the transition out (4 keys)
static const ATreeInstance::ScaleKey s_aScaleOutKeys[ 4 ] = {
	{ 0.0f, 1.0f, 1.0f },
	{ 0.06f, 0.5f, 2.6f },
	{ 0.1f, 0.6f, 0.1f },
	{ 0.23f, 0.01f, 0.01f },
};
static const TINT s_iNumScaleOutKeys = 4;

// The glow of the transition: it rises, holds, then falls (seconds)
static const TFLOAT s_fGlowRise = 0.5f;
static const TFLOAT s_fGlowFall = 1.5f;
static const TFLOAT s_fGlowHold = 1.0f;

static TFLOAT s_fGlowTimer = -1.0f; // time of the glow, -1: none runs
static TFLOAT s_fGlowFrame = 0.0f;  // the frame count of the last glow step (0: none)
// $deBlob: FUNCTION 0053b740
static TFLOAT  s_fGlowEndTime  = s_fGlowRise + s_fGlowFall + s_fGlowHold; // 0x00990d4c (retail's static init 0053b740)
static TFLOAT  s_fGlowPeakTime = 0.0f;                                    // 0x00990d60 the end of the hold (set once)
static TBOOL   s_bGlowPeakSet  = TFALSE;                                  // 0x00990d64
static TUINT32 s_uiGlowColour  = 0;                                       // 0x00990d44 the colour of the glow (ABlobColourMaps)
static TBOOL   s_bOutAsserted  = TFALSE;                                  // 0x00990d68 the TransitioningOut assert was logged

// $deBlob: FUNCTION 00581120
void AToggleVisInstance::OnCreated( TerrainTypes::CellInstance* a_pInstance )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_ToggleVis );

	ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 )->GetCollisionInstance() );

	if ( pCollision == TNULL )
		return;

	// The bodies lose the layer bit 8 (the layer the toggle sets, restored by the tick)
	for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
	{
		TPhysicsObjectContainer* pBody   = pCollision->GetPhysicsInfo( i ).m_pBody;
		TUINT32                  uiLayer = pBody ? ( pBody->m_uiCollisionFilterInfo & 0xFFF ) : 0;

		if ( uiLayer & 8 )
			TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, uiLayer & ~8 );
	}
}

// $deBlob: FUNCTION 005811c0
void AToggleVisInstance::OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const TVector4* a_pPosition )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_ToggleVis );

	TUINT32 uiFlags = a_pInstance->m_uiFlags;

	if ( !( uiFlags & 0x400 ) )
		return;

	if ( uiFlags & ( TerrainTypes::InstanceFlags_TransitioningIn | TerrainTypes::InstanceFlags_TransitioningOut ) )
		return;

	// NOTE: ABaseEntity::m_Flags (+0x70) bit 4, not the blob flags at 0x554
	if ( !( a_pBlob->GetFlags() & ABaseEntity::FLAG_Player ) )
		return;

	TUINT32 uiColour = a_pBlob->GetPaintColour();

	if ( uiColour & 0x10 )
	{
		if ( uiFlags & TerrainTypes::InstanceFlags_Visible )
			return;

		if ( uiFlags & 0x4000 )
			ABuildingManager::TriggerTransformation( a_pInstance, a_pBlob );
	}
	else
	{
		if ( uiColour == 0 )
			return;

		if ( uiFlags & TerrainTypes::InstanceFlags_Visible )
			ABuildingManager::TriggerTransformation( a_pInstance, a_pBlob );
	}
}

// $deBlob: FUNCTION 00581270
void AToggleVisInstance::OnAddedToWorld( TerrainTypes::CellInstance* a_pInstance )
{
	TUINT32 uiFlags = a_pInstance->m_uiFlags;

	if ( !( uiFlags & 0x200 ) && ( uiFlags & 0x400 ) )
	{
		ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 )->GetCollisionInstance() );

		if ( pCollision != TNULL )
			pCollision->AddToWorld( 96 );
	}
}

// $deBlob: FUNCTION 00581300
TBOOL AToggleVisInstance::OnTick( TerrainTypes::CellInstance* a_pInstance, TFLOAT a_fDeltaTime, TFLOAT a_fTime )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_ToggleVis );
	TASSERT( a_pInstance->m_uiFlags & ( TerrainTypes::InstanceFlags_ScaleOnVisToggle | TerrainTypes::InstanceFlags_GlowOnVisToggle ) );

	AModelInstance2* pModel = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );

	if ( !( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_TransitioningIn ) )
	{
		// The transition out: the scale curve runs until its last key
		if ( !s_bOutAsserted && !( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_TransitioningOut ) )
		{
			TASSERT( TFALSE && "a_pInstance->IsFlagSet(TerrainTypes::InstanceFlags_TransitioningOut)" );
			s_bOutAsserted = TTRUE;
		}

		ATreeInstance::SetScaleFromCurve( pModel, a_fTime, s_aScaleOutKeys, s_iNumScaleOutKeys );

		if ( a_fTime <= s_aScaleOutKeys[ s_iNumScaleOutKeys - 1 ].m_fTime )
			return TFALSE;

		a_pInstance->m_uiFlags &= ~( TerrainTypes::InstanceFlags_Visible | TerrainTypes::InstanceFlags_TransitioningOut );

		if ( !( a_pInstance->m_uiFlags & 0x400 ) && pModel->GetCollisionInstance() != TNULL )
			TSTATICCAST( ACollisionInstance2, pModel->GetCollisionInstance() )->RemoveFromWorld( 96 );

		return TTRUE;
	}

	// The transition in: the scale grows along its curve (ScaleOnVisToggle), the glow runs once per frame
	TBOOL bScaleDone = TFALSE;

	if ( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_ScaleOnVisToggle )
	{
		ATreeInstance::SetScaleFromCurve( pModel, a_fTime, s_aScaleInKeys, s_iNumScaleInKeys );
		bScaleDone = a_fTime > s_aScaleInKeys[ s_iNumScaleInKeys - 1 ].m_fTime;
	}

	if ( !( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_GlowOnVisToggle ) )
	{
		if ( !bScaleDone )
			return TFALSE;
	}
	else
	{
		TFLOAT fFrame = TFLOAT( TRender::GetSingleton()->GetFrameCount() );

		if ( s_fGlowFrame == fFrame )
			return TFALSE;

		s_fGlowFrame = fFrame;

		if ( s_fGlowTimer < 0.0f )
		{
			if ( s_fGlowTimer == -1.0f )
				ResetGlowMaterials( a_pInstance );
		}
		else
		{
			s_fGlowTimer += a_fDeltaTime;

			if ( s_fGlowTimer < s_fGlowEndTime )
			{
				// The intensity rises, holds, then falls
				if ( !s_bGlowPeakSet )
				{
					s_fGlowPeakTime = s_fGlowHold + s_fGlowRise;
					s_bGlowPeakSet  = TTRUE;
				}

				TFLOAT fT;

				if ( s_fGlowTimer <= s_fGlowPeakTime )
					fT = s_fGlowTimer / s_fGlowRise;
				else
					fT = 1.0f - ( ( s_fGlowTimer - s_fGlowPeakTime ) / s_fGlowFall );

				TMath::Clip( fT, 0.0f, 1.0f );

				TWorldMaterialHAL::s_vLerpConstColour = TVector4( fT, fT, fT, fT );

				// The glow colour (UnpackColour, the w lane is 0)
				TVector4 vColour;
				UnpackColour( s_uiGlowColour, vColour );
				TWorldMaterialHAL::s_vMultiPassColour.x = vColour.x;
				TWorldMaterialHAL::s_vMultiPassColour.y = vColour.y;
				TWorldMaterialHAL::s_vMultiPassColour.z = vColour.z;
				TWorldMaterialHAL::s_vMultiPassColour.w = 0.0f;

				return TFALSE;
			}

			s_fGlowTimer = -1.0f;
			ResetGlowMaterials( a_pInstance );
		}
	}

	// The transition is over
	a_pInstance->m_uiFlags &= ~TerrainTypes::InstanceFlags_TransitioningIn;
	s_fGlowFrame = 0.0f;
	return TTRUE;
}

// $deBlob: FUNCTION 005812b0
void AToggleVisInstance::OnReset( TerrainTypes::CellInstance* a_pInstance )
{
	if ( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_ScaleOnVisToggle )
	{
		TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 )->SetScale( 1.0f );
	}

	if ( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_GlowOnVisToggle )
	{
		s_fGlowFrame = 0.0f;
		ResetGlowMaterials( a_pInstance );
	}
}

// $deBlob: FUNCTION 00581640
void AToggleVisInstance::OnPaint( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob )
{
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_ToggleVis );

	if ( a_pInstance->m_uiFlags & ( TerrainTypes::InstanceFlags_TransitioningIn | TerrainTypes::InstanceFlags_TransitioningOut ) )
		return;

	ABuildingManager::TriggerLinks( a_pInstance, AEntityIOPacket::MESSAGE_TRIGGER );

	TUINT32 uiFlags = a_pInstance->m_uiFlags;

	if ( !( uiFlags & TerrainTypes::InstanceFlags_Visible ) )
	{
		if ( ( uiFlags & 0x200 ) || ( uiFlags & 0x4000 ) )
		{
			a_pInstance->m_uiFlags = uiFlags | TerrainTypes::InstanceFlags_Visible;

			AModelInstance2* pModel = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );

			if ( ( uiFlags & 0x1000000 ) && pModel->GetCollisionInstance() != TNULL )
				TSTATICCAST( ACollisionInstance2, pModel->GetCollisionInstance() )->AddToWorld( 96 );

			if ( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_ScaleOnVisToggle )
			{
				a_pInstance->m_uiFlags |= TerrainTypes::InstanceFlags_TransitioningIn;
				pModel->SetScale( 0.01f ); // the scale of the instance starts small
			}

			if ( ( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_GlowOnVisToggle ) && a_pInstance->m_pSymbolData != TNULL )
			{
				// The glow colour of the blob's paint (the colour is 0x10 for the colours above white)
				TUINT32 uiColour = a_pBlob->GetPaintColour();

				if ( uiColour & 0x10 )
					uiColour = 0x10;

				s_uiGlowColour = ABlobColourMaps::ms_pLandmarkGlowColourMap[ uiColour ];

				// The meshes (but the windows and stickers) get a white stage that glows (blend 0x16)
				TerrainTypes::InstanceMeshes* pMeshes = a_pInstance->m_pSymbolData;

				for ( TUINT i = 0; i < pMeshes->m_uiNumMeshes; i++ )
				{
					TWorldMaterialHAL* pMaterial = pMeshes->m_pMeshes[ i ].m_pMaterial;
					const TCHAR*       szName    = pMaterial->GetMaterialName();

					if ( szName == TNULL || ( TStringManager::String8Compare( szName, "window", 6 ) != 0 && TStringManager::String8Compare( szName, "sticker", 7 ) != 0 ) )
					{
						a_pInstance->m_uiFlags |= TerrainTypes::InstanceFlags_TransitioningIn;
						TUINT32 uiStage = pMaterial->GetTextureNum();
						s_fGlowTimer    = 0.0f;
						pMaterial->SetTextureNum( uiStage + 1 );
						pMaterial->SetTexture( uiStage, TTextureManager::GetSingleton()->GetWhiteTexture() );
						pMaterial->SetBlendMode( 13 );
						pMaterial->SetMultiPassBlendType( uiStage, TWorldMaterialHAL::MULTIPASSBLENDTYPE( 0x16 ) );
					}
				}
			}

			if ( a_pInstance->m_uiFlags & TerrainTypes::InstanceFlags_TransitioningIn )
				ABuildingManager::GetSingleton()->AddTickInstance( a_pInstance );
		}
	}
	else if ( ( uiFlags & 0x4000 ) || !( uiFlags & 0x200 ) )
	{
		if ( !( uiFlags & TerrainTypes::InstanceFlags_ScaleOnVisToggle ) )
		{
			AModelInstance2* pModel = TSTATICCAST( AModelInstance2, a_pInstance->m_pModelInstance2 );

			if ( !( uiFlags & 0x400 ) && pModel->GetCollisionInstance() != TNULL )
				TSTATICCAST( ACollisionInstance2, pModel->GetCollisionInstance() )->RemoveFromWorld( 96 );

			a_pInstance->m_uiFlags &= ~TerrainTypes::InstanceFlags_Visible;
			return;
		}

		ABuildingManager::GetSingleton()->AddTickInstance( a_pInstance );
		a_pInstance->m_uiFlags |= TerrainTypes::InstanceFlags_TransitioningOut;
	}
}

// $deBlob: FUNCTION 00581860
void AToggleVisInstance::ResetGlowMaterials( TerrainTypes::CellInstance* a_pInstance )
{
	// The white glow stage added by OnPaint is removed again
	TerrainTypes::InstanceMeshes* pMeshes = a_pInstance->m_pSymbolData;

	if ( pMeshes == TNULL )
		return;

	for ( TUINT i = 0; i < pMeshes->m_uiNumMeshes; i++ )
	{
		TWorldMaterialHAL* pMaterial = pMeshes->m_pMeshes[ i ].m_pMaterial;
		TUINT32            uiNumTex  = pMaterial->GetTextureNum();

		if ( pMaterial->GetMultiPassBlendType( uiNumTex - 1 ) == 0x16 )
		{
			pMaterial->SetTexture( uiNumTex - 1, TNULL );
			pMaterial->SetBlendMode( 0 );
			pMaterial->SetMultiPassBlendType( uiNumTex - 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE( 1 ) );
			pMaterial->SetTextureNum( uiNumTex - 1 );
		}
	}
}
