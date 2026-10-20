#include "pch.h"
#include "AInky_Baton.h"
#include "Sound/ASoundInterface.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AAttackMessage.h"
#include "Entities/AEntityPhysicsMover.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Entities/PathFinding/APathfindingComponent.h"
#include "Entities/AI/Schedule/AAISchedule.h"
#include "Animation/AModel2.h"
#include "Animation/AModelInstance2.h"
#include "GameController/AGameStatus.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include <Physics/TCapsuleData.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void*  AInky_Baton::sm_pInkySplatFX = TNULL;
void*  AInky_Baton::sm_pStrikeFX    = TNULL;
TUINT8 AInky_Baton::sm_uiSkinFlags  = 0;

TSTATICASSERT( sizeof( AInky_Baton ) == 0xF0 );

// $deBlob: FUNCTION 007aab90
// $deBlob: FUNCTION 007aaba0
// $deBlob: FUNCTION 007aac10
// $deBlob: FUNCTION 007aac70
// $deBlob: FUNCTION 007aac80
// $deBlob: FUNCTION 00541260
// $deBlob: FUNCTION 00859f20
TDEFINE_CLASS( AInky_Baton );

// $deBlob: FUNCTION 007aac90
AInky_Baton::AInky_Baton()
{
	m_uiType = 0;

	if ( sm_pInkySplatFX == TNULL )
	{
		TParticleEffectManager* pManager = TParticleSystem::GetSingleton()->GetEffectManager();
		sm_pInkySplatFX                  = pManager->FindEffect( "Blob_InkySplat" );
		sm_pStrikeFX                     = pManager->FindEffect( "Blob_InkyBaton_Strike" );
	}

	sm_uiSkinFlags = 0;
}

// $deBlob: FUNCTION 007aad20
// $deBlob: FUNCTION 007ab370
AInky_Baton::~AInky_Baton()
{
	sm_pInkySplatFX = TNULL;
	sm_pStrikeFX    = TNULL;
}

// $deBlob: FUNCTION 007aad40
void AInky_Baton::Create()
{
	ACharEntity::Create();

	// A capsule (height 2.3, radius 1.0, along z turned upright) standing on the origin, with its physics mover
	AEntityTypeData::TRBData* pTRBData = m_pTypeData->m_pTRBData;

	TCapsuleData     oCapsule;
	TShapeBaseClass* pShape = &oCapsule;

	TMatrix44 matOffset = TMatrix44::IDENTITY;
	oCapsule.SetHeight( 2.3f );
	oCapsule.SetRadius( 1.0f );
	matOffset.RotateX( TMath::PI / 2 );

	TVector4 vTranslation;
	vTranslation.Multiply( TVector4::VEC_NEGY, oCapsule.GetHeight() * 0.5f + oCapsule.GetRadius() );
	matOffset.SetTranslation( vTranslation );

	CreatePhysics( pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
	SetPhysicsMover( new AEntityPhysicsMover( this ) );

	InitIdleState();
}

// $deBlob: FUNCTION 007aaec0
void AInky_Baton::PostCreate()
{
	AEnemy::PostCreate();

	TSTATICCAST( APathfindingComponent, GetPathfinding() )->SetCollisionFlags( 2, TTRUE );

	if ( sm_uiSkinFlags == 3 )
		return;

	// The sphere map of the skin materials, once per skin (ss_inky, ss_elite_inky) for all the batons
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	T2ModelPtr       pModel         = pModelInstance->GetModel()->GetModelPtr();
	TModelLOD*       pLOD           = &pModel->GetLOD( 0 );

	TTexture* pSphereMap = TTextureManager::GetSingleton()->FindTexture( "spheremap_inky.tga" );
	pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

	TINT i = 0;

	if ( pLOD->GetMeshCount() > 0 )
	{
		TBOOL bElite;
		TBOOL bInky;

		do
		{
			ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ i ]->GetMaterial() );
			const TCHAR*      szName   = pSkinMat->GetName();

			bElite = TStringManager::String8FindString( szName, "ss_elite_inky" ) != TNULL;
			bInky  = TStringManager::String8FindString( szName, "ss_inky" ) != TNULL;

			if ( ( bInky && ( sm_uiSkinFlags & 1 ) ) || ( bElite && ( sm_uiSkinFlags & 2 ) ) )
				break;

			// NOTE: retail changes every mesh of the model, not only the ss_inky / ss_elite_inky ones
			pSkinMat->SetTextureNum( 2 );
			pSkinMat->SetTevMode( 1, 0xC );
			pSkinMat->SetTexture( 1, pSphereMap );
			pSkinMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
			pSkinMat->SetBlendMode( 13, 0 );

			i++;
		} while ( i < pLOD->GetMeshCount() );

		if ( bInky )
			sm_uiSkinFlags |= 1;
		else if ( bElite )
			sm_uiSkinFlags |= 2;
	}
}

// $deBlob: FUNCTION 007ab060
void AInky_Baton::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_SLAM:
		{
			// Slammed by a blob: the blob pays the baton's health with its paint
			if ( ( m_Flags & FLAG_Dead ) || !( a_pCaller->GetFlags() & 0x10 ) )
				break;

			TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

			if ( pBlob->GetBlobFlags() & 0x8000 )
				break;

			if ( m_uiType != 0 )
			{
				TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

				if ( uiColour & 0x10 )
					uiColour = 0x10;

				// A coloured baton is only hurt by its colour (or the rainbow)
				if ( m_uiType != uiColour && !( pBlob->GetBlobFlags() & 0x200000 ) )
					break;
			}

			TASSERT( pBlob->GetZJumpComponent()->GetState() == ABlobZJumpComponent::STATE_ATTACKSLAM );

			AAttackMessage* pAtkMsg = TREINTERPRETCAST( AAttackMessage*, a_rPacket.m_iData );
			TASSERT( ( pAtkMsg->m_eType == AAttackMessage::ATTACK_PlayerSlamDirect ) || ( pAtkMsg->m_eType == AAttackMessage::ATTACK_PlayerSlamSplash ) );

			TINT iHealth = TINT( pBlob->RemovePaintPoints( m_iLife, TFALSE ) );

			if ( iHealth <= 0 )
			{
				Die( pBlob );
				AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
				return;
			}

			m_iLife = TINT16( iHealth );

			if ( pAtkMsg->m_eType == AAttackMessage::ATTACK_PlayerSlamDirect )
			{
				// Still alive: the blob bounces off
				AAttackMessage  oRecoil( AAttackMessage::ATTACK_SlamFailedRecoil, m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 0, 3, TVector4::VEC_NEGY, TFALSE );
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RECOIL;
				oPacket.m_iData    = TREINTERPRETCAST( TINT, &oRecoil );
				pBlob->HandleEntityIO( this, oPacket );

				TSTATICCAST( AAISchedule, GetScheduleComponent() )->StartSchedule( 4 );
			}
			break;
		}
		case AEntityIOPacket::MESSAGE_PROPATTACK:
		case AEntityIOPacket::MESSAGE_DEATHZONE:
		case AEntityIOPacket::MESSAGE_RAMATTACK:
		{
			SpawnDecal();
			PlayDeathSound();

			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PLAYERATTACK;
			oPacket.m_iData    = 0;
			OnDeath( oPacket, a_pCaller );

			AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
			return;
		}
	}

	AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007ab2b0
void AInky_Baton::Die( ABaseEntity* a_pKiller )
{
	SpawnDecal();
	PlayDeathSound();

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PLAYERATTACK;
	oPacket.m_iData    = 0;
	OnDeath( oPacket, a_pKiller );
}

// $deBlob: FUNCTION 007ab2f0
void AInky_Baton::PlayDeathSound()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x19 ), ASoundMap::EVENTID( 0x11E ), m_oMatrix.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION ), 0 );
}

// $deBlob: FUNCTION 007ab310
TINT AInky_Baton::GetTargetColour()
{
	TASSERT( m_uiType == 0 );
	return 0;
}

// $deBlob: FUNCTION 007ab3f0
TINT AInky_Baton::GetMaxLife()
{
	return *TREINTERPRETCAST( TINT*, GetEnemyData()->m_aUnk54 );
}

// $deBlob: FUNCTION 007ab3b0
void* AInky_Baton::GetHitEffect()
{
	return sm_pStrikeFX;
}

// $deBlob: FUNCTION 007ab3d0
void* AInky_Baton::GetDespawnEffect()
{
	return sm_pInkySplatFX;
}

// $deBlob: FUNCTION 007ab3e0
void* AInky_Baton::GetSplatEffect()
{
	return sm_pInkySplatFX;
}

// $deBlob: FUNCTION 007ab350
void AInky_Baton::SetCustomData( TINT a_iCustomData )
{
	m_uiType                                                        = TUINT8( a_iCustomData );
	*( TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType ) + 1 ) = 0x14; // 0x63: the high byte of m_iCollisionGroupAndType
}

// $deBlob: FUNCTION 007ab400
TBOOL AInky_Baton::ShouldFlee( TINT a_iEnemy )
{
	return TFALSE;
}

// $deBlob: FUNCTION 007ab3c0
TINT AInky_Baton::GetKillScore()
{
	// The score of a killed baton (BlobScoringProperties+0x8C)
	return TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x8C / 4 ];
}
