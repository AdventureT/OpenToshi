#include "pch.h"
#include "ABigPaintBot.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityPhysicsMover.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Animation/AModel2.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionObject.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include <Physics/TCapsuleData.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL ABigPaintBot::sm_bSkinDone = TFALSE;

TSTATICASSERT( sizeof( ABigPaintBot ) == 0xA4 );

// $deBlob: FUNCTION 007a8c40
// $deBlob: FUNCTION 007a8c50
// $deBlob: FUNCTION 007a8cc0
// $deBlob: FUNCTION 007a8d20
// $deBlob: FUNCTION 007a8d30
// $deBlob: FUNCTION 005411a0
// $deBlob: FUNCTION 00859ef0
TDEFINE_CLASS( ABigPaintBot );

// $deBlob: FUNCTION 007a8d40
ABigPaintBot::ABigPaintBot()
{
	m_uiColour         = 1;
	m_pCollisionObject = TNULL;
	m_fAmbientScale    = 1.0f;
	sm_bSkinDone       = TFALSE;

	CreateShadow( 5.5f, 0.75f );
}

// $deBlob: FUNCTION 007a9930
ABigPaintBot::~ABigPaintBot()
{
}

// $deBlob: FUNCTION 007a8dd0
void ABigPaintBot::Create()
{
	AVisibleEntity::Create();

	// A capsule (height 2.5, radius 1.5, along z turned upright) standing on the origin, with its physics mover
	AEntityTypeData::TRBData* pTRBData = m_pTypeData->m_pTRBData;

	TCapsuleData     oCapsule;
	TShapeBaseClass* pShape = &oCapsule;

	TMatrix44 matOffset = TMatrix44::IDENTITY;
	oCapsule.SetHeight( 2.5f );
	oCapsule.SetRadius( 1.5f );
	matOffset.RotateX( TMath::PI / 2 );

	TVector4 vTranslation;
	vTranslation.Multiply( TVector4::VEC_NEGY, oCapsule.GetHeight() * 0.5f + oCapsule.GetRadius() );
	matOffset.SetTranslation( vTranslation );

	CreatePhysics( pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
	SetPhysicsMover( new AEntityPhysicsMover( this ) );

	SetState( STATE_Idle );
	CreateCollisionObject();
}

// $deBlob: FUNCTION 007a8f60
void ABigPaintBot::Destroy()
{
	if ( m_pCollisionObject != TNULL )
		delete m_pCollisionObject;

	// NOTE: AVisibleEntity has no Destroy of its own
	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 007a8f80
void ABigPaintBot::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_iState == STATE_Popped )
	{
		// Comes back once the closest player is far enough
		if ( HasStateFinished() )
		{
			const TVector4& rPosition = m_oMatrix.GetTranslation();
			TINT8           iPlayer   = APlayerManager::GetSingleton()->GetClosestPlayer( rPosition );

			if ( iPlayer != -1 )
			{
				ABaseEntity* pBlob   = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iPlayer ) );
				TFLOAT       fRadius = pBlob->GetBoundingSphereRadius() + 4.0f;

				if ( fRadius * fRadius <= TVector4::DistanceSq( pBlob->GetMatrix().GetTranslation(), rPosition ) )
				{
					SetHash( TTRUE );
					m_Flags &= ~FLAG_Dead;
					SetPhysicsEnabled( TTRUE );
					SetState( STATE_Respawn );
				}
			}
		}
	}
	else if ( m_iState == STATE_Respawn && HasStateFinished() )
	{
		SetState( STATE_Idle );
	}

	AVisibleEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007a9050
void ABigPaintBot::ProcessParams( const AEntityParams& a_rParams )
{
	AVisibleEntity::ProcessParams( a_rParams );
	a_rParams.GetValue( m_uiColour, "Colour" );
}

// $deBlob: FUNCTION 007a9080
void ABigPaintBot::PostCreate()
{
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	T2ModelPtr       pModel         = pModelInstance->GetModel()->GetModelPtr();
	TModelLOD*       pLOD           = &pModel->GetLOD( 0 );

	TASSERT( 1 == pModelInstance->GetModel()->GetModelPtr()->GetLODCount() );

	// The glow (mesh 2) gets the sphere map once (shared by all the big paint bots)
	ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ 2 ]->GetMaterial() );

	if ( !sm_bSkinDone )
	{
		TTexture* pSphereMap = TTextureManager::GetSingleton()->FindTexture( "spheremap_leechbot.tga" );
		pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

		TASSERT( pSkinMat->GetNumTextures() == 1 );

		pSkinMat->SetTextureNum( 2 );
		pSkinMat->SetTevMode( 1, 0xC );
		pSkinMat->SetTexture( 1, pSphereMap );
		pSkinMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
		pSkinMat->SetBlendMode( 0x18, 0 );
		sm_bSkinDone = TTRUE;
	}

	TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ 0 ]->GetMaterial() )->SetBlendMode( 0, 0 );
	TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ 1 ]->GetMaterial() )->SetBlendMode( 0, 0 );

	TASSERT( TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ 2 ]->GetMaterial() )->GetBlendMode() == 0x18 );

	pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetMeshFlags( 0, 1, TFALSE );

	pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->SetMeshFlags( 2, 1, TTRUE );
	pModelInstance->SetMeshColour( 2, 0x59000000 );

	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->UseLocalAmbientLight( TTRUE );
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetMeshFlags( 1, 1, TTRUE );

	SetColour( m_uiColour );
}

// $deBlob: FUNCTION 007a9330
void ABigPaintBot::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( TUINT8( m_Flags >> 22 ) & 1 )
		return;

	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TOUCH:
		case AEntityIOPacket::MESSAGE_TOUCHEDBY:
		case AEntityIOPacket::MESSAGE_SLAM:
			if ( TUINT8( a_pCaller->GetFlags() >> 4 ) & 1 )
				break;

			AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
			return;
		case AEntityIOPacket::MESSAGE_PROPATTACK:
		case AEntityIOPacket::MESSAGE_RAMATTACK:
			Pop();
			return;
		default:
			AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
			return;
	}

	// A blob touched it: it takes the colour (random for 0) and 30 points
	TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
	ABlobEntity*          pBlobEntity = TSTATICCAST( ABlobEntity, a_pCaller );
	ABlobColourComponent* pColour     = pBlobEntity->GetColourComponent();

	TUINT32 uDefColor = m_uiColour;
	TINT    eGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( eGameType >= 0 )
		uDefColor = pColour->GetStartColour();

	if ( uDefColor == 0 )
		uDefColor = ABlobColourComponent::GetRandomColour();

	if ( eGameType == 1 )
	{
		ABaseEntity* pPainted = APlayerManager::GetSingleton()->GetPaintedBlob();

		if ( TUINT8( pColour->GetCurrentColour() >> 4 ) & 1 )
		{
			// Inked: only the points
			pBlobEntity->AddPaintPoints( 30 );
		}
		else if ( pPainted == TNULL )
		{
			TASSERT( uDefColor == pBlobEntity->GetColourComponent()->GetStartColour() );
			pBlobEntity->ChangeColour( uDefColor );
			pBlobEntity->AddPaintPoints( 30 );
			pColour->SetColour( uDefColor );
			pBlobEntity->GetAnimationComponent()->StartUnk28Timer();

			AGameStatus::GetSingleton()->SetItBlob( pBlobEntity );
		}
		else if ( pBlobEntity == pPainted )
		{
			TASSERT( uDefColor == pBlobEntity->GetColourComponent()->GetStartColour() );
			pBlobEntity->ChangeColour( uDefColor );
			pBlobEntity->AddPaintPoints( 30 );
			pColour->SetColour( uDefColor );
			pBlobEntity->GetAnimationComponent()->StartUnk28Timer();
		}
	}
	else if ( !( TUINT8( pBlobEntity->GetBlobFlags() >> 21 ) & 1 ) )
	{
		if ( TUINT8( pColour->GetCurrentColour() >> 4 ) & 1 )
		{
			// Inked: only the points
			pBlobEntity->AddPaintPoints( 30 );
		}
		else if ( pColour->GetCurrentColour() != 0 )
		{
			pBlobEntity->ChangeColour( uDefColor );
			pBlobEntity->AddPaintPoints( 30 );

			if ( eGameType >= 0 )
				pColour->SetColour( uDefColor );

			pBlobEntity->GetAnimationComponent()->StartUnk28Timer();
		}
		else
		{
			pBlobEntity->ChangeColour( uDefColor );
			pBlobEntity->AddPaintPoints( 30 );
			pColour->SetColour( uDefColor );
			pBlobEntity->GetAnimationComponent()->StartUnk28Timer();
		}
	}

	SetColour( TUINT8( uDefColor ) );
	Pop();
}

// $deBlob: FUNCTION 007a9590
void ABigPaintBot::Reset( ABaseEntity* a_pEntity )
{
	SetHash( TTRUE );
	m_Flags &= ~FLAG_Dead;
	SetPhysicsEnabled( TTRUE );
	SetState( STATE_Idle );

	AVisibleEntity::Reset( a_pEntity );
}

// $deBlob: FUNCTION 007a95d0
TINT ABigPaintBot::GetTargetColour()
{
	return m_uiColour & 0xF;
}

// In multiplayer each viewport shows the big paint bot in the colour of its player (game type 1: the painted blob's,
// game type 3: the second colour of the blob)
// $deBlob: FUNCTION 007a95e0
void ABigPaintBot::PreRenderUpdate()
{
	TINT eGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( eGameType >= 0 )
	{
		AXYZViewportManager*          pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
		AXYZViewportManager::VIEWPORT eViewport        = pViewportManager->GetViewportBeingRendered();

		if ( eViewport != AXYZViewportManager::VIEWPORT_NONE )
		{
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetBlobOfCamera( pViewportManager->GetCamera( eViewport ) ) );

			if ( pBlob != TNULL )
			{
				TUINT32 uiColour = pBlob->GetColourComponent()->GetStartColour();

				switch ( eGameType )
				{
					case 1:
					{
						ABlobEntity* pPainted = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPaintedBlob() );

						if ( pPainted != TNULL )
							uiColour = pPainted->GetColourComponent()->GetStartColour();
					}
					break;
					case 3:
					{
						uiColour = pBlob->GetColourComponent()->GetUnk20();
					}
					break;
				}

				TUINT32 uiLight = ABlobColourMaps::ms_pEnemyLightColourMap[ uiColour ];
				TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetAmbientColour( uiLight );
				TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetMeshColour( 1, ABlobColourMaps::ms_pEnemyBodyColourMap[ uiColour ] );
			}
		}
	}

	AVisibleEntity::PreRenderUpdate();
}

// $deBlob: FUNCTION 007a9690
void ABigPaintBot::SetColour( TUINT8 a_uiColour )
{
	m_uiColour = a_uiColour;

	TUINT8* pLODType = TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType ) + 1; // 0x63: the high byte of m_iCollisionGroupAndType

	if ( a_uiColour == 1 )
		*pLODType = 10;
	else if ( a_uiColour == 2 )
		*pLODType = 11;
	else if ( a_uiColour == 4 )
		*pLODType = 9;
	else
		*pLODType = 5;

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetAmbientColour( ABlobColourMaps::ms_pEnemyLightColourMap[ a_uiColour ] );
	pModelInstance->SetMeshColour( 1, ABlobColourMaps::ms_pEnemyBodyColourMap[ a_uiColour ] );
}

// $deBlob: FUNCTION 007a9700
const TCHAR* ABigPaintBot::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}

// $deBlob: FUNCTION 007a9710
void ABigPaintBot::Pop()
{
	// The LeechBot destroy effect of the colour
	TParticleEffect* pEffect;

	if ( m_uiColour >= 1 && m_uiColour <= 7 )
		pEffect = ALeechBot2::ms_apPrimaryDestroyEffects[ m_uiColour - 1 ];
	else if ( TUINT8( m_uiColour - 8 ) < 8 )
		pEffect = ALeechBot2::ms_apSecondaryDestroyEffects[ m_uiColour - 8 ];
	else
		pEffect = ALeechBot2::ms_apPrimaryDestroyEffects[ 0 ];

	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pEffect );

	if ( pInstance != TNULL )
	{
		TVector4 vOffset;
		vOffset.Multiply( TVector4::VEC_POSZ, 2.5f );

		TVector4 vPosition;
		TMatrix44::RotateVector( vPosition, m_pPhysicsData->m_oMatrix, vOffset );
		vPosition.x += m_oMatrix.m_f41;
		vPosition.y += m_oMatrix.m_f42;
		vPosition.z += m_oMatrix.m_f43;

		TMatrix44 matEffect = TMatrix44::IDENTITY;
		matEffect.SetTranslation( vPosition );
		pInstance->GetTransform() = matEffect;
	}

	m_Flags |= FLAG_Dead;
	m_iLife = 0;
	SetHash( TFALSE );
	SetPhysicsEnabled( TFALSE );
	SetState( STATE_Popped );
}

// $deBlob: FUNCTION 007a9890
void ABigPaintBot::CreateCollisionObject()
{
	if ( GetCollisionObject() == TNULL )
	{
		m_pCollisionObject = new ACollisionObject( this, 1, 0xFF );
		GetCollisionObject()->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
		m_pCollisionObject->SetFlags( ACollisionObject::FLAG_Created );
	}

	// The low byte of m_iCollisionGroupAndType: the projectiles' collision byte (AProjectile::UpdateLifeTime)
	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF00 ) | 0x20 );
}

// $deBlob: FUNCTION 007a9960
ACollisionObject* ABigPaintBot::GetCollisionObject()
{
	return m_pCollisionObject;
}

// $deBlob: FUNCTION 007a9970
TBOOL ABigPaintBot::IsTargetable()
{
	return TTRUE;
}
