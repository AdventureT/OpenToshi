#include "pch.h"
#include "AInky_Pen.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AAttackMessage.h"
#include "Entities/AEntityPhysicsMover.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/AEntityPoseController.h"
#include "Entities/AGndMotorComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Entities/Enemies/AEnemyBehaviour.h"
#include "Entities/PathFinding/APathfindingComponent.h"
#include "Entities/AI/Schedule/AAISchedule.h"
#include "Animation/AModel2.h"
#include "Animation/AModelInstance2.h"
#include "Fx/Shadows/AShadow.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Sound/ASoundInterface.h"
#include "Sound/APositionalSoundManager.h"
#include <Math/TMath.h>
#include <Physics/TCapsuleData.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void* AInky_Pen::sm_pInkySplatFX = TNULL;
TBOOL AInky_Pen::sm_bSkinDone    = TFALSE;

TSTATICASSERT( sizeof( AInky_Pen ) == 0xF0 );

static TCONSTEXPR_VAR TFLOAT s_fSprayLengthTime = 0.5f;  // the cone reaches its length after this time
static TCONSTEXPR_VAR TFLOAT s_fSprayDownY      = -0.9f; // the facing y that gives the full "up" overlay weight

// $deBlob: FUNCTION 007a9e50
// $deBlob: FUNCTION 007a9e60
// $deBlob: FUNCTION 007a9ed0
// $deBlob: FUNCTION 007a9f30
// $deBlob: FUNCTION 007a9f40
// $deBlob: FUNCTION 007a9ff0
// $deBlob: FUNCTION 00541220
// $deBlob: FUNCTION 00859f10
TDEFINE_CLASS( AInky_Pen );

// $deBlob: FUNCTION 007a9f50
AInky_Pen::AInky_Pen()
{
	m_fSprayTimer = 0.0f;

	if ( sm_pInkySplatFX == TNULL )
		sm_pInkySplatFX = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_InkySplat" );

	sm_bSkinDone = TFALSE;
	m_pShadow->SetSize( 4.0f, 4.0f );

	*( TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType ) + 1 ) = 0x1F; // 0x63: the high byte of m_iCollisionGroupAndType
}

// $deBlob: FUNCTION 007aaaf0
AInky_Pen::~AInky_Pen()
{
	sm_pInkySplatFX = TNULL;
}

// $deBlob: FUNCTION 007aa300
void AInky_Pen::Tick( TFLOAT a_fDeltaTime )
{
	AEnemy::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007aa010
void AInky_Pen::Create()
{
	ACharEntity::Create();

	// A capsule (height 1.5, radius 1.5, along z turned upright) standing on the origin, 0.5 behind it
	AEntityTypeData::TRBData* pTRBData = m_pTypeData->m_pTRBData;

	TCapsuleData     oCapsule;
	TShapeBaseClass* pShape = &oCapsule;

	TMatrix44 matOffset = TMatrix44::IDENTITY;
	oCapsule.SetHeight( 1.5f );
	oCapsule.SetRadius( 1.5f );
	matOffset.RotateX( TMath::PI / 2 );
	matOffset.SetTranslation( TVector4( 0.0f, -( oCapsule.GetHeight() * 0.5f + oCapsule.GetRadius() ), -0.5f, 1.0f ) );

	CreatePhysics( pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
	SetPhysicsMover( new AEntityPhysicsMover( this ) );

	InitIdleState();
}

// $deBlob: FUNCTION 007aa190
void AInky_Pen::PostCreate()
{
	AEnemy::PostCreate();

	TSTATICCAST( APathfindingComponent, GetPathfinding() )->SetCollisionFlags( 2, TTRUE );

	if ( sm_bSkinDone )
		return;

	// The sphere map of the skin materials (shared by all the pens)
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	T2ModelPtr       pModel         = pModelInstance->GetModel()->GetModelPtr();
	TModelLOD*       pLOD           = &pModel->GetLOD( 0 );

	TTexture* pSphereMap = TTextureManager::GetSingleton()->FindTexture( "spheremap_inky.tga" );
	pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

	for ( TINT i = 0; i < pLOD->GetMeshCount(); i++ )
	{
		ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ i ]->GetMaterial() );
		TASSERT( pSkinMat->GetNumTextures() == 1 );

		pSkinMat->SetTextureNum( 2 );
		pSkinMat->SetTevMode( 1, 0xC );
		pSkinMat->SetTexture( 1, pSphereMap );
		pSkinMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
		pSkinMat->SetBlendMode( 13, 0 );
	}

	sm_bSkinDone = TTRUE;
}

// $deBlob: FUNCTION 007aa320
void AInky_Pen::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_SLAM:
		{
			// Slammed by a blob: the blob pays the pen's health with its paint
			if ( ( m_Flags & FLAG_Dead ) || !( a_pCaller->GetFlags() & 0x10 ) )
				break;

			TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

			if ( pBlob->GetBlobFlags() & 0x8000 )
				break;

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
			Die( a_pCaller );
			AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
			return;
	}

	AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007aa5f0
void AInky_Pen::OnLeaveState( TINT a_iState )
{
	if ( a_iState != 0x11 )
	{
		AEnemy::OnLeaveState( a_iState );
		return;
	}

	// The end of the spray
	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1C, 0x12F );
	TSTATICCAST( AEntityPoseController, GetPoseController() )->StopManualOverlayAnim( 0.2f );
}

// $deBlob: FUNCTION 007aa640
void AInky_Pen::OnEnterState( TINT a_iState )
{
	if ( a_iState != 0x11 )
	{
		AEnemyMelee::OnEnterState( a_iState );
		return;
	}

	StartSpray();
}

// $deBlob: FUNCTION 007aa730
const TCHAR* AInky_Pen::GetModelName()
{
	return GetEnemyData()->m_szModelName;
}

// $deBlob: FUNCTION 007aa510
void AInky_Pen::StartCurrentStateTask()
{
	switch ( m_iCurrentTask )
	{
		case 0x25:
			m_fSprayTimer = 0.0f;
			SetState( 0x10 );
			break;
		default:
			AEnemyMelee::StartCurrentStateTask();
			break;
	}
}

// $deBlob: FUNCTION 007aa530
TBOOL AInky_Pen::UpdateCurrentStateTask( TFLOAT a_fDeltaTime )
{
	if ( m_iState == 0x10 )
	{
		// Faces the enemy, then sprays
		TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->FaceEnemyPredictedPosition( 0.5f, TFALSE, TNULL );

		if ( HasStateFinished() )
			SetState( 0x11 );

		return TFALSE;
	}

	if ( m_iState != 0x11 )
		return AEnemyMelee::UpdateCurrentStateTask( a_fDeltaTime );

	if ( TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy() == TNULL )
	{
		if ( m_iCurrentTask != -1 )
			EndSpray();

		return TTRUE;
	}

	m_fSprayTimer += a_fDeltaTime;
	UpdateSpray();

	if ( m_fSprayTimer > TSTATICCAST( PenData, m_pTypeData->m_pTRBData )->m_fSprayTime )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007aab80
TINT AInky_Pen::GetTargettingPriority()
{
	return 0;
}

// $deBlob: FUNCTION 007aab70
TINT AInky_Pen::GetMaxLife()
{
	return *TREINTERPRETCAST( TINT*, GetEnemyData()->m_aUnk54 );
}

// $deBlob: FUNCTION 007aa720
TBOOL AInky_Pen::ShouldFlee( TINT a_iEnemy )
{
	return TFALSE;
}

// $deBlob: FUNCTION 007aa660
TFLOAT AInky_Pen::GetAttackedTimeOut( TINT a_iSchedule, TINT a_iTask )
{
	if ( a_iSchedule == 0x19 && a_iTask == 7 && TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy() != TNULL )
	{
		TFLOAT* pPathTaskTime = &GetEnemyData()->m_fPathTaskTime;
		return *pPathTaskTime - TSTATICCAST( AAISchedule, GetScheduleComponent() )->GetTaskTime();
	}

	return -1.0f;
}

// $deBlob: FUNCTION 007aab30
TINT AInky_Pen::GetAttackSchedule()
{
	return 0x19;
}

// $deBlob: FUNCTION 007aab50
TINT AInky_Pen::GetKillScore()
{
	// The score of a killed pen (BlobScoringProperties+0x94)
	return TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x94 / 4 ];
}

// $deBlob: FUNCTION 007aab60
void* AInky_Pen::GetSplatEffect()
{
	return sm_pInkySplatFX;
}

// The death of the pen
// $deBlob: FUNCTION 007aa6c0
void AInky_Pen::Die( ABaseEntity* a_pKiller )
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x1C ), ASoundMap::EVENTID( 300 ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 0 );
	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1C, 0x12F );
	SpawnDecal();

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PLAYERATTACK;
	oPacket.m_iData    = 0;
	OnDeath( oPacket, a_pKiller );
}

// The start of the spray (state 0x11)
// $deBlob: FUNCTION 007aa740
void AInky_Pen::StartSpray()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x1C ), ASoundMap::EVENTID( 0x130 ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 0 );
	APositionalSoundManager::GetSingleton()->AddEntitySound( this, 0x1C, 0x12F );
	TSTATICCAST( AEntityPoseController, GetPoseController() )->PlayManualOverlayAnim( "Attack_2_loop_up", 0.0f, 0.0f, 1.0f );
}

// The end of the spray
// $deBlob: FUNCTION 007aa7a0
void AInky_Pen::EndSpray()
{
	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1C, 0x12F );
	TSTATICCAST( AEntityPoseController, GetPoseController() )->StopManualOverlayAnim( 0.2f );
}

// The spray: turns to the enemy, then hits the blobs in the cone of the spray bone (MESSAGE_ENEMYATTACK, attack type 9)
// $deBlob: FUNCTION 007aa7d0
void AInky_Pen::UpdateSpray()
{
	ABaseEntity* pSelected = TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy();

	if ( pSelected != TNULL )
	{
		TVector4 vPredicted = TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->GetEnemyPredictedPosition( pSelected, 0.05f, TNULL );

		const TVector4& rPosition = m_oMatrix.GetTranslation();
		TVector4        vFacing( vPredicted.x - rPosition.x, vPredicted.y - rPosition.y, vPredicted.z - rPosition.z, vPredicted.w );
		vFacing.Normalise();

		TFLOAT fWeight = vFacing.y;
		TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetFacing( vFacing );

		// Aims up (the overlay) when the enemy is above
		fWeight /= s_fSprayDownY;
		TMath::Clip( fWeight, 0.0f, 1.0f );
		TSTATICCAST( AEntityPoseController, GetPoseController() )->SetManualOverlayAnimWeight( fWeight );
	}

	PenData*  pData = TSTATICCAST( PenData, m_pTypeData->m_pTRBData );
	TMatrix44 matBone;
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( pData->m_iSprayBone, matBone );

	for ( TINT i = 0; i < 4; i++ )
	{
		TFLOAT fRatio = m_fSprayTimer / s_fSprayLengthTime;
		TMath::Clip( fRatio, 0.0f, 1.0f );
		TFLOAT fLength = TSTATICCAST( PenData, m_pTypeData->m_pTRBData )->m_fSprayLength * fRatio;

		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( i ) ) );

		if ( pBlob == TNULL || pBlob->GetBody() == TNULL || !pBlob->GetBody()->IsInSimulation() )
			continue;

		TFLOAT   fRadius = pBlob->GetBoundingSphereRadius();
		TVector4 vConeDir;
		vConeDir.Multiply( matBone.AsBasisVector4( BASISVECTOR_UP ), -1.0f );

		const TVector4& rBlobPos = pBlob->GetBodyPosition();

		if ( TMath::ConeVsSphere( matBone.GetTranslation(), vConeDir, TSTATICCAST( PenData, m_pTypeData->m_pTRBData )->m_fSprayFOV, fLength, rBlobPos, fRadius ) )
		{
			PenData* pHitData = TSTATICCAST( PenData, m_pTypeData->m_pTRBData );

			TVector4 vDirection;
			vDirection.Multiply( matBone.AsBasisVector4( BASISVECTOR_UP ), -1.0f );

			TVector4 vOffset;
			vOffset.Multiply( matBone.AsBasisVector4( BASISVECTOR_UP ), pBlob->GetBoundingSphereRadius() );

			TVector4       vHit( rBlobPos.x + vOffset.x, rBlobPos.y + vOffset.y, rBlobPos.z + vOffset.z, rBlobPos.w );
			AAttackMessage oMessage( AAttackMessage::ATTACKTYPE( 9 ), vHit, pHitData->m_iAttackDamage, pHitData->m_iAttackUnk18, vDirection, TFALSE );
			AdjustAttackMessage( pBlob, &oMessage );

			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_ENEMYATTACK;
			oPacket.m_iData    = TREINTERPRETCAST( TINT, &oMessage );
			pBlob->HandleEntityIO( this, oPacket );
		}
	}
}
