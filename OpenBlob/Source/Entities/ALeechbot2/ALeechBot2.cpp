#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "ALeechBot2.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityPoseController.h"
#include "Entities/AGndMotorComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Entities/AAttackMessage.h"
#include "Entities/AEntityFactory.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include <Render/TModel.h>
#include <Render/TMesh.h>
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Render/AXYZViewportManager.h"
#include "Render/ARenderer.h"
#include "Entities/AI/Behaviour/AAIBehaviour.h"
#include "Fx/AParticleEffectHelper.h"
#include "Fx/Shadows/AShadow.h"
#include "Entities/AEntityPhysicsMover.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/PathFinding/APathfindingComponent.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AModel2.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Sound/ASoundInterface.h"
#include "ARandom.h"
#include "Terrain2/ATerrain2.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"

#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffect.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/TCapsuleData.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* ALeechBot2::ms_pExplosionEffect;
Toshi::TParticleEffect* ALeechBot2::ms_apSecondaryDestroyEffects[ NUM_SECONDARY_COLOURS ];
Toshi::TParticleEffect* ALeechBot2::ms_apPrimaryDestroyEffects[ NUM_PRIMARY_COLOURS ];
TBOOL                   ALeechBot2::sm_bSkinMaterialSetUp;

TSTATICASSERT( sizeof( ALeechBot2 ) == 0x130 );

// $deBlob: FUNCTION 0077a9d0
// $deBlob: FUNCTION 0077a9e0
// $deBlob: FUNCTION 0077aa50
// $deBlob: FUNCTION 0077aab0
// $deBlob: FUNCTION 0077aac0
// $deBlob: FUNCTION 00540a20
// $deBlob: FUNCTION 00859d00
TDEFINE_CLASS( ALeechBot2 );

// $deBlob: FUNCTION 0077aad0
ALeechBot2::ALeechBot2()
{
	m_eLeechbotType = LeechBot_Invalid;
	m_uiLeechFlags  = 0;
	m_uiColour      = 0;
	m_fTankFill     = 0.0f;
	m_fGrabCooldown = 0.0f;
	m_fRespawnTimer = -1.0f;

	m_pShadow->SetSize( 5.0f, 5.0f );
	sm_bSkinMaterialSetUp = TFALSE;
	m_pShadow->SetAlpha( 0.75f );
}

// $deBlob: FUNCTION 0077c050
void ALeechBot2::SetColour( TUINT32 a_uiColour )
{
	if ( a_uiColour - 8 < 8 )
	{
		SetPriorityType( 0xC );
		m_uiColour = a_uiColour;
	}
	else if ( m_eLeechbotType == LeechBot_Multi || m_eLeechbotType == LeechBot_Single )
	{
		SetPriorityType( 5 );
		m_uiColour = a_uiColour;
	}
	else
	{
		// A mixed colour: takes one of its primaries at random
		TUINT32 uiBit1 = a_uiColour & 1;
		TUINT32 uiBit2 = a_uiColour & 2;
		TUINT32 uiRed  = a_uiColour & ABlobColourComponent::COLOUR_RED;

		TINT8 iColourComponents = ( uiBit1 != 0 );
		if ( uiBit2 != 0 ) iColourComponents++;
		if ( uiRed != 0 ) iColourComponents++;

		TASSERT( iColourComponents > 0 );

		TINT iRandom     = ARandom::GetSingleton()->GetRandom().GetInt( iColourComponents );
		TINT iColourComp = ( uiBit1 != 0 ) ? iRandom : iRandom + 1;

		if ( iColourComp == 0 )
		{
			m_uiColour = 1;
			SetPriorityType( 7 );
		}
		else
		{
			if ( uiBit2 != 0 )
				iColourComp--;

			if ( iColourComp == 0 )
			{
				m_uiColour = 2;
				SetPriorityType( 8 );
			}
			else
			{
				TASSERT( a_uiColour & ABlobColourComponent::COLOUR_RED );
				TASSERT( 1 == iColourComp );
				m_uiColour = ABlobColourComponent::COLOUR_RED;
				SetPriorityType( 6 );
			}
		}

		if ( m_eLeechbotType == LeechBot_Empty && m_uiColour == 0 )
			SetPriorityType( 0x16 );
		else if ( m_eLeechbotType == LeechBot_Multi || m_eLeechbotType == LeechBot_Single )
			SetPriorityType( 5 );
	}

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();

	pModelInstance->GetModel()->GetModelPtr().GetData()->GetLOD( 0 );
	TASSERT( 1 == pModelInstance->GetModel()->GetModelPtr().GetData()->GetLODCount() );

	pModelInstance->SetFlag( 0x1000, TTRUE );
	pModelInstance->SetMeshFlags( 0, 1, TFALSE );
	pModelInstance->SetMeshFlags( 2, 1, TTRUE );
	pModelInstance->SetMeshColour( 2, 0x59000000 );
	pModelInstance->UseLocalAmbientLight( TTRUE );

	TUINT32 uiColour = m_uiColour;
	pModelInstance->SetAmbientColour( ABlobColourMaps::ms_pEnemyLightColourMap[ uiColour ] );
	pModelInstance->SetMeshFlags( 1, 1, TTRUE );
	pModelInstance->SetMeshColour( 1, ABlobColourMaps::ms_pEnemyBodyColourMap[ uiColour ] );
}

// $deBlob: FUNCTION 0077c3d0
void ALeechBot2::SetEmptyShape()
{
	if ( m_uiLeechFlags & LFLAG_Full )
	{
		TPhysicsManager*    pPhysicsManager = TPhysicsManager::GetSingleton();
		AEntityPhysicsData* pPhysicsData    = m_pPhysicsData;
		TPhysicsResponder*  pHandler        = pPhysicsData->m_pCollisionEventHandler;
		TPhysicsMotionBase* pMover          = pPhysicsData->m_pPhysicsMover;

		pPhysicsManager->RemoveCustomMover( pMover );
		pPhysicsManager->RemoveFromSimulation( pPhysicsData->m_pBody, TFALSE );
		pPhysicsManager->DestroyBody( pPhysicsData->m_pBody );

		CreateCapsule( 0.3f, 0.75f, pPhysicsData );
		SetPhysicsMover( pMover );
		SetCollisionEventHandler( pHandler );

		TSTATICCAST( APathfindingComponent, GetPathfinding() )->SetCollisionFlags( 2, TTRUE );
		m_uiLeechFlags &= ~LFLAG_Full;

		TINT iMaxLife = GetMaxLife();
		m_Flags &= ~FLAG_Dead;
		m_iLife = TINT16( iMaxLife );
	}
}

// $deBlob: FUNCTION 0077c5a0
void ALeechBot2::SetFullShape()
{
	if ( !( m_uiLeechFlags & LFLAG_Full ) )
	{
		TPhysicsManager*    pPhysicsManager = TPhysicsManager::GetSingleton();
		AEntityPhysicsData* pPhysicsData    = m_pPhysicsData;
		TPhysicsResponder*  pHandler        = pPhysicsData->m_pCollisionEventHandler;
		TPhysicsMotionBase* pMover          = pPhysicsData->m_pPhysicsMover;

		pPhysicsManager->RemoveCustomMover( pMover );
		pPhysicsManager->RemoveFromSimulation( pPhysicsData->m_pBody, TFALSE );
		pPhysicsManager->DestroyBody( pPhysicsData->m_pBody );

		CreateCapsule( 1.75f, 1.0f, pPhysicsData );
		SetPhysicsMover( pMover );
		SetCollisionEventHandler( pHandler );

		TSTATICCAST( APathfindingComponent, GetPathfinding() )->SetCollisionFlags( 2, TFALSE );
		m_uiLeechFlags |= LFLAG_Full;
		m_Flags &= ~FLAG_Dead;
		m_iLife = 0;
	}
}

// $deBlob: FUNCTION 0077c770
void ALeechBot2::Destroy( ABaseEntity* a_pKiller, TBOOL a_bNoRespawn )
{
	ASoundInterface::PlayEvent( ASoundMap::GROUPID( 0x1F ), ASoundMap::EVENTID( 0x150 ), FMOD_EVENT_DEFAULT, TTRUE );

	TParticleEffect* pEffect = ms_pExplosionEffect;

	if ( m_uiColour != 0 && 0x14 != m_iState )
	{
		pEffect = ms_apPrimaryDestroyEffects[ 0 ];

		if ( m_uiColour >= 1 && m_uiColour < 8 )
			pEffect = ms_apPrimaryDestroyEffects[ m_uiColour - 1 ];
		else if ( m_uiColour - 8 <= 7 )
			pEffect = ms_apSecondaryDestroyEffects[ m_uiColour - 8 ];
	}

	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pEffect );

	if ( pInstance )
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

	if ( m_eLeechbotType != LeechBot_Empty && !a_bNoRespawn )
	{
		m_fRespawnTimer = GetLeechBotData()->m_fRespawnDelay;

		// NOTE: retail passes the packet's own address as its data
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PLAYERATTACK;
		oPacket.m_iData    = TINT( &oPacket );
		OnDeath( oPacket, a_pKiller );

		SetHash( TFALSE );
		RemoveFromPhysics();
		TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TFALSE );
		TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TFALSE );
		TSTATICCAST( AEntityComponent2, GetScheduleComponent() )->SetEnabled( TFALSE );
		SetState( 0x14 );
		return;
	}

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 0x22 ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 0 );

	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1f, 0x154 );

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PLAYERATTACK;
	oPacket.m_iData    = 0;
	OnDeath( oPacket, a_pKiller );
}

// $deBlob: FUNCTION 0077c9b0
void ALeechBot2::ResetColour()
{
	TASSERT( LeechBot_Invalid != GetLeechbotType() );

	m_fGrabCooldown = 0.0f;

	if ( m_eLeechbotType == LeechBot_Empty )
	{
		EmptyTank();
		return;
	}

	m_fTankFill = 1.0f;
	TSTATICCAST( AEntityPoseController, GetPoseController() )->SetManualOverlayAnimWeight( 1.0f );
	TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( GetWalkSpeed() );
	SetFullShape();

	switch ( m_eLeechbotType )
	{
		case LeechBot_Colour1:
			m_uiColour = 1;
			break;
		case LeechBot_Colour2:
			m_uiColour = 2;
			break;
		case LeechBot_Colour4:
			m_uiColour = 4;
			break;
		case LeechBot_Multi:
		case LeechBot_Single:
			m_uiColour = ABlobColourComponent::GetRandomColour();
			break;
		default:
			// Types 8..15 hold that secondary colour
			if ( m_eLeechbotType >= 8 && m_eLeechbotType <= 15 )
				m_uiColour = m_eLeechbotType;
			break;
	}

	SetColour( m_uiColour );
}

// $deBlob: FUNCTION 0077cc10
void ALeechBot2::EmptyTank()
{
	m_fTankFill = 0.0f;
	TSTATICCAST( AEntityPoseController, GetPoseController() )->SetManualOverlayAnimWeight( 0.0f );
	TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( GetRunSpeed() );

	m_uiColour = 0;
	SetPriorityType( 0x16 );
	SetEmptyShape();

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->SetFlag( 0x1000, TFALSE );
	pModelInstance->SetMeshFlags( 2, 1, TFALSE );
	pModelInstance->SetMeshColour( 2, 3, 0 );
	pModelInstance->SetMeshFlags( 1, 1, TFALSE );
	pModelInstance->SetMeshColour( 1, 0 );
}

// $deBlob: FUNCTION 0077ccd0
// $deBlob: FUNCTION 0077ab40
ALeechBot2::~ALeechBot2()
{
}

// $deBlob: FUNCTION 0077ab50
void ALeechBot2::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fRespawnTimer <= 0.0f )
	{
		AEnemyGrappler::Tick( a_fDeltaTime );

		if ( m_fGrabCooldown > 0.0f )
			m_fGrabCooldown -= a_fDeltaTime;

		if ( m_eGrapplerState == GRAPPLERSTATE_Grappling )
		{
			// The paint of the grappled blob is drained into the tank
			ABlobEntity* pBlob    = TSTATICCAST( ABlobEntity, m_pPlayer );
			TUINT32      uiColour = pBlob->GetPaintColour();

			if ( uiColour & 0x10 )
				uiColour = 0x10;

			if ( ( m_uiColour & uiColour ) == 0 )
			{
				// Nothing (or ink) left to take
				if ( uiColour == 0 || ( uiColour & 0x10 ) )
				{
					ReleaseGrapple();
					return;
				}

				SetColour( uiColour );
			}

			TFLOAT fDrain       = GetLeechBotData()->m_fDrainRate / 10.0f;
			TINT   iPaintPoints = pBlob->GetPaintPoints();
			TFLOAT fGrappleTime = sm_PlayerBones[ m_iPlayerIndex ].m_fGrappleTime;
			TFLOAT fFill        = ( ( 1.0f - m_fTankFill ) * a_fDeltaTime ) / ( ( fDrain - fGrappleTime ) + a_fDeltaTime + TFLOAT( iPaintPoints ) * fDrain ) + m_fTankFill;
			m_fTankFill         = fFill;
			m_fTankFill         = ( fFill <= 1.0f ) ? fFill : 1.0f;

			TSTATICCAST( AEntityPoseController, GetPoseController() )->SetManualOverlayAnimWeight( m_fTankFill );

			if ( fDrain < fGrappleTime )
			{
				sm_PlayerBones[ m_iPlayerIndex ].m_fGrappleTime -= fDrain;

				if ( pBlob->GetPaintPoints() == 1 )
				{
					// The last point: the blob is drained, every grappler lets go
					pBlob->ChangeColour( 0x10 );

					for ( TINT i = ENEMYGRAPPLER_MAX_GRAPPLERS - 1; i >= 0; i-- )
					{
						if ( sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ i ] != TNULL )
							sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ i ]->OnBlobDrained();
					}
				}

				if ( iPaintPoints != 0 )
					pBlob->RemovePaintPoints( 1, TTRUE );
			}
		}
	}
	else
	{
		m_fRespawnTimer -= a_fDeltaTime;

		if ( m_fRespawnTimer <= 0.0f )
		{
			// Waits while the closest player is near
			const TVector4& rcPosition = m_oMatrix.GetTranslation();
			TINT8           iPlayer    = APlayerManager::GetSingleton()->GetClosestPlayer( rcPosition );

			if ( iPlayer != -1 )
			{
				ABaseEntity* pBlob   = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iPlayer ) );
				TFLOAT       fRadius = pBlob->GetBoundingSphereRadius() + 3.0f;

				if ( TVector4::DistanceSq( pBlob->GetMatrix().GetTranslation(), rcPosition ) < fRadius * fRadius )
				{
					m_fRespawnTimer = 0.5f;
					return;
				}
			}

			SetHash( TTRUE );

			if ( !( GetLOD() & FLAG_LowLOD ) )
				AddToPhysics();

			TINT iMaxLife = GetMaxLife();
			m_Flags &= ~FLAG_Dead;
			m_iLife = TINT16( iMaxLife );
			SetState( 0x15 );
		}
	}
}

// $deBlob: FUNCTION 0077adf0
void ALeechBot2::OnLODChanging( TINT a_iLOD )
{
	// Without a wrapper it dies once out of sight
	if ( GetPoolOwner() == 0 && a_iLOD == FLAG_LowLOD )
	{
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_CULLED;
		oPacket.m_iData    = 0;
		OnDeath( oPacket, TNULL );
		return;
	}

	AEnemy::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 0077ae50
void ALeechBot2::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_CHALLENGERESET )
	{
		TUINT8 uiType = m_eLeechbotType & 0x7F;

		if ( uiType != LeechBot_Colour2 && uiType != LeechBot_Colour1 && uiType != LeechBot_Colour4 )
			return;

		Destroy( a_pCaller, TTRUE );
		return;
	}

	if ( IsFlagged( 0x400000 ) )
		return;

	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TOUCH:
		case AEntityIOPacket::MESSAGE_TOUCHEDBY:
			break;
		case AEntityIOPacket::MESSAGE_SLAM:
			if ( m_uiColour == 0 )
			{
				// An empty LeechBot slammed by a Z-jumping blob drains its paint, or makes it recoil
				TASSERT( a_pCaller->IsFlagged( FLAG_Player ) );
				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

				if ( pBlob->GetZJumpState() == ABlobZJumpComponent::STATE_ATTACKSLAM )
				{
					TINT iRemaining = pBlob->RemovePaintPoints( m_iLife, TFALSE );

					if ( iRemaining <= 0 )
					{
						Destroy( pBlob, TFALSE );
						pBlob->GetAnimationComponent()->StartUnk28Timer();
					}
					else
					{
						m_iLife = TINT16( iRemaining );

						AAttackMessage  oAttack( AAttackMessage::ATTACK_SlamFailedRecoil, m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 0, 3, TVector4::VEC_NEGY, TFALSE );
						AEntityIOPacket oPacket;
						oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RECOIL;
						oPacket.m_iData    = TINT( &oAttack );
						pBlob->HandleEntityIO( this, oPacket );
					}
				}
			}
			break;
		case AEntityIOPacket::MESSAGE_GRAPPLERELEASE:
			TASSERT( a_pCaller == m_pPlayer );

			if ( m_eGrapplerState == GRAPPLERSTATE_Grappling )
			{
				SetGrapplerState( GRAPPLERSTATE_Released );
				Destroy( TNULL, TFALSE );
			}

			AEnemyGrappler::HandleEntityIO( a_pCaller, a_rPacket );
			return;
		case AEntityIOPacket::MESSAGE_PROPATTACK:
		case AEntityIOPacket::MESSAGE_RAMATTACK:
			Destroy( a_pCaller, TFALSE );
			return;
		case AEntityIOPacket::MESSAGE_DEATHZONE:
			if ( m_eLeechbotType != LeechBot_Empty )
				return;

			Destroy( a_pCaller, TFALSE );
			return;
		default:
			AEnemyGrappler::HandleEntityIO( a_pCaller, a_rPacket );
			return;
	}

	if ( IsGrappleState() || m_pPlayer != TNULL || m_uiColour == 0 || !a_pCaller->IsFlagged( FLAG_Player ) || 0.0f < m_fGrabCooldown )
	{
		if ( m_eGrapplerState == GRAPPLERSTATE_Attacking && m_pPlayer == TNULL && a_pCaller->IsFlagged( FLAG_Player ) )
		{
			ABlobEntity* pBlob    = a_pCaller->CastTo<ABlobEntity>();
			TUINT32      uiColour = pBlob->GetPaintColour();

			if ( ( uiColour & 0x10 ) == 0 && uiColour == 0 )
			{
				// The attack hits an unpainted blob: it gets inked
				TVector4 vPosition = pBlob->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
				TVector4 vDirection( vPosition.x - m_oMatrix.m_f41, vPosition.y - m_oMatrix.m_f42, vPosition.z - m_oMatrix.m_f43, vPosition.w );

				const TUINT32* pData = TREINTERPRETCAST( const TUINT32*, m_pTypeData->m_pTRBData ); // NOTE: 0xDC/0xE0 of the LeechBot's enemy data

				TVector4 vOffset;
				vOffset.Multiply( vDirection, vDirection.NormaliseMag() );
				vPosition.x -= vOffset.x;
				vPosition.y -= vOffset.y;
				vPosition.z -= vOffset.z;

				AAttackMessage  oAttack( AAttackMessage::ATTACKTYPE( 0 ), vPosition, pData[ 0xDC / 4 ], pData[ 0xE0 / 4 ], vDirection, TFALSE );
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_ENEMYATTACK;
				oPacket.m_iData    = TINT( &oAttack );
				pBlob->HandleEntityIO( this, oPacket );
				return;
			}
		}

		AEnemyGrappler::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}

	// A painted LeechBot touched by a blob: the blob takes its paint
	TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
	ABlobEntity*          pBlob       = TSTATICCAST( ABlobEntity, a_pCaller );
	ABlobColourComponent* pBlobColour = pBlob->GetColourComponent();

	TUINT32 uiColour = m_uiColour;

	if ( m_eLeechbotType == LeechBot_Multi || m_eLeechbotType == LeechBot_Single )
		uiColour = pBlobColour->GetStartColour();

	if ( uiColour == 0 )
		uiColour = ABlobColourComponent::GetRandomColour();

	// NOTE: the asserts call the start colour GetDefaultColour()
	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 )
	{
		ABaseEntity* pPaintedBlob = APlayerManager::GetSingleton()->GetPaintedBlob();

		if ( ( pBlobColour->GetCurrentColour() & 0x10 ) == 0 )
		{
			if ( pPaintedBlob == TNULL )
			{
				TASSERT( uiColour == pBlobColour->GetStartColour() );
				pBlob->ChangeColour( uiColour );
				pBlob->RefillPaint();
				pBlobColour->SetColour( uiColour );
				pBlob->GetAnimationComponent()->StartUnk28Timer();
				AGameStatus::GetSingleton()->SetItBlob( pBlob );
			}
			else if ( pBlob == pPaintedBlob )
			{
				TASSERT( uiColour == pBlobColour->GetStartColour() );
				pBlob->ChangeColour( uiColour );
				pBlob->RefillPaint();
				pBlobColour->SetColour( uiColour );
				pBlob->GetAnimationComponent()->StartUnk28Timer();
			}
		}
		else
		{
			TASSERT( !pBlob->IsEvilBlob() );
			pBlob->RefillPaint();
		}
	}
	else if ( ( pBlob->GetBlobFlags() & 0x200000 ) == 0 )
	{
		TUINT32 uiCurrentColour = pBlobColour->GetCurrentColour();

		if ( TUINT8( uiCurrentColour >> 4 ) & 1 )
		{
			pBlob->RefillPaint();
		}
		else if ( uiCurrentColour == 0 )
		{
			pBlob->ChangeColour( uiColour );
			pBlob->RefillPaint();
			pBlobColour->SetColour( uiColour );
			pBlob->GetAnimationComponent()->StartUnk28Timer();
		}
		else
		{
			if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() >= 0 )
			{
				TASSERT( LeechBot_Multi == GetLeechbotType() || LeechBot_Single == GetLeechbotType() );
				pBlobColour->SetColour( uiColour );
			}

			pBlob->ChangeColour( uiColour );
			pBlob->RefillPaint();
			pBlob->GetAnimationComponent()->StartUnk28Timer();
		}
	}

	SetColour( uiColour );
	Destroy( pBlob, TFALSE );
}

// $deBlob: FUNCTION 0077b400
TBOOL ALeechBot2::ShouldIgnoreBlob( void* a_pEntity )
{
	if ( m_uiColour == 0 && !( TSTATICCAST( ABlobEntity, a_pEntity )->GetColourComponent()->GetCurrentColour() & 0x10 ) )
		return AEnemy::ShouldIgnoreBlob( a_pEntity );

	return TTRUE;
}

// $deBlob: FUNCTION 0077b440
TBOOL ALeechBot2::IsTargetable()
{
	return m_eGrapplerState == GRAPPLERSTATE_None && 0x14 != m_iState;
}

// $deBlob: FUNCTION 0077b470
void ALeechBot2::OnBlobDrained()
{
	SetFullShape();
	SetGrapplerState( GRAPPLERSTATE_None );

	m_fTankFill     = 1.0f;
	m_fGrabCooldown = 1.0f;
	TSTATICCAST( AEntityPoseController, GetPoseController() )->SetManualOverlayAnimWeight( 1.0f );

	TFLOAT fSpeed = GetWalkSpeed();
	TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( fSpeed );
}

// $deBlob: FUNCTION 0077b4f0
void ALeechBot2::AddAttachedShape( TPhysicsObjectContainer* a_pBody, ABaseEntity* a_pPlayer, TINT a_iIndex )
{
	TCapsuleData oCapsule;
	oCapsule.SetHeight( 0.4f );
	oCapsule.SetRadius( 0.75f );

	// Upright, its centre above the grapple point
	TMatrix44 oMatrix = TMatrix44::IDENTITY;
	oMatrix.RotateX( TMath::HALF_PI );

	TVector4 vTranslation;
	vTranslation.Multiply( TVector4::VEC_NEGY, oCapsule.GetHeight() * 0.5f + oCapsule.GetRadius() );
	oMatrix.SetTranslation( vTranslation );

	AEnemyGrappler::GetBoneMatrix( oMatrix, a_pPlayer, a_iIndex );
	TPhysicsManager::AddListCapsule( a_pBody, oMatrix, oCapsule.GetHeight(), oCapsule.GetRadius() );
}

// $deBlob: FUNCTION 0077b770
void ALeechBot2::SetActive( TBOOL a_bActive, TBOOL a_bUnk )
{
	if ( a_bActive )
	{
		m_fRespawnTimer = -1.0f;

		ResetColour();
	}

	AEnemyGrappler::SetActive( a_bActive, a_bUnk );
}

// $deBlob: FUNCTION 0077b7a0
void ALeechBot2::InitMoveState()
{
	if ( m_fTankFill < 1.0f )
		SetState( 4 );
	else
		SetState( 3 );
}

// $deBlob: FUNCTION 0077b7d0
void ALeechBot2::InitIdleState()
{
	if ( m_fTankFill < 1.0f )
		AEnemyGrappler::InitIdleState();
	else
		SetState( 0x13 );
}

// $deBlob: FUNCTION 0077b7f0
void ALeechBot2::OnEnterState( TINT a_iState )
{
	AEnemyGrappler::OnEnterState( a_iState );

	if ( a_iState > 2 )
	{
		if ( a_iState < 5 )
		{
			// Moves slower once full
			TFLOAT fSpeed = ( m_fTankFill >= 1.0f ) ? GetWalkSpeed() : GetRunSpeed();
			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( fSpeed );
		}
		else if ( a_iState == 0x14 )
		{
			m_fTankFill = 0.0f;
			TSTATICCAST( AEntityPoseController, GetPoseController() )->SetManualOverlayAnimWeight( 0.0f );
		}
	}
}

// $deBlob: FUNCTION 0077b8a0
void ALeechBot2::OnLeaveState( TINT a_iState )
{
	AEnemyGrappler::OnLeaveState( a_iState );

	if ( TUINT( a_iState - 3 ) < 2 )
	{
		TFLOAT fSpeed = ( m_fTankFill >= 1.0f ) ? GetWalkSpeed() : GetRunSpeed();
		TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( fSpeed );
	}
}

// $deBlob: FUNCTION 0077b910
TBOOL ALeechBot2::UpdateCurrentStateTask( TFLOAT a_fDeltaTime )
{
	if ( m_iState == 0x14 )
		return TFALSE;

	if ( m_iState != 0x15 )
		return AEnemyGrappler::UpdateCurrentStateTask( a_fDeltaTime );

	if ( !HasStateFinished() )
		return TFALSE;

	// Waits for the collision of the wrapper's linked terrain instance
	ABaseEntity* pWrapper = TREINTERPRETCAST( ABaseEntity*, GetPoolOwner() );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail calls GetPoolOwner()'s GetClass without checking it for TNULL
	if ( pWrapper != TNULL && pWrapper->IsA( &TGetClass( AEnemyWrapper ) ) )
#else
	if ( pWrapper->IsA( &TGetClass( AEnemyWrapper ) ) )
#endif
	{
		AEnemyWrapper* pEnemyWrapper = TREINTERPRETCAST( ABaseEntity*, GetPoolOwner() )->CastTo<AEnemyWrapper>();

		if ( pEnemyWrapper != TNULL && pEnemyWrapper->GetLinkedInstance() != -1 )
		{
			TerrainTypes::CellInstance* pInstance = ATerrain2::GetInstance()->GetCellInstance( pEnemyWrapper->GetLinkedInstance() );

			if ( pInstance != TNULL )
			{
				ACollisionInstance2* pCollision = pInstance->m_pModelInstance2 ? TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance() ) : TNULL;

				if ( pCollision != TNULL && !pCollision->IsInSimulation( ACollisionInstance2::PhysicsObjectType_Standard ) )
					return TFALSE;
			}
		}
	}


	m_fRespawnTimer = -1.0f;

	ResetColour();
	InitMoveState();

	if ( !( m_uiLeechFlags & LFLAG_Static ) )
	{
		TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TTRUE );
		TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TTRUE );
		TSTATICCAST( AEntityComponent2, GetScheduleComponent() )->SetEnabled( TTRUE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0077ba20
void ALeechBot2::PreRenderUpdate()
{
	// The multiplayer LeechBots take the colour of the viewport's player
	if ( m_eLeechbotType == LeechBot_Multi || m_eLeechbotType == LeechBot_Single )
	{
		AXYZViewportManager*          pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
		AXYZViewportManager::VIEWPORT eViewport        = pViewportManager->GetViewportBeingRendered();
		TASSERT( AXYZViewportManager::VIEWPORT_NONE != eViewport );

		ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetBlobOfCamera( pViewportManager->GetCamera( eViewport ) );

		if ( pBlob != TNULL )
		{
			TUINT32 uiColour = TSTATICCAST( ABlobEntity, pBlob )->GetColourComponent()->GetStartColour();

			if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 )
			{
				ABaseEntity* pPaintedBlob = APlayerManager::GetSingleton()->GetPaintedBlob();

				if ( pPaintedBlob != TNULL )
					uiColour = TSTATICCAST( ABlobEntity, pPaintedBlob )->GetColourComponent()->GetStartColour();
			}
			else if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 3 )
			{
				uiColour = TSTATICCAST( ABlobEntity, pBlob )->GetColourComponent()->GetUnk20();
			}

			SetColour( uiColour );
		}
	}

	AVisibleEntity::PreRenderUpdate();
}

// $deBlob: FUNCTION 0077bb20
void ALeechBot2::Reset( ABaseEntity* a_pEntity )
{
	TVector4 vPosition = m_vSpawnPosition;
	m_fRespawnTimer    = -1.0f;
	SetPosition( vPosition );

	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1f, 0x154 );

	ResetColour();

	AEnemy::Reset( a_pEntity );
}

// $deBlob: FUNCTION 0077bb80
void ALeechBot2::StartDrop()
{
	m_fTankFill = 0.0f;
	TSTATICCAST( AEntityPoseController, GetPoseController() )->SetManualOverlayAnimWeight( 0.0f );
	SetState( 0x15 );
	TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TFALSE );
	TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TFALSE );
	TSTATICCAST( AEntityComponent2, GetScheduleComponent() )->SetEnabled( TFALSE );
}

// The capsule (along z, turned upright) standing on the entity's origin
void ALeechBot2::CreateCapsule( TFLOAT a_fHeight, TFLOAT a_fRadius, AEntityPhysicsData* a_pPhysicsData )
{
	const AEntityTypeData::TRBData* pTRBData = m_pTypeData->m_pTRBData;

	TCapsuleData oCapsule;
	oCapsule.SetHeight( a_fHeight );
	oCapsule.SetRadius( a_fRadius );
	TShapeBaseClass* pShape = &oCapsule;

	TMatrix44 matOffset = TMatrix44::IDENTITY;
	matOffset.RotateX( TMath::PI / 2 );

	TVector4 vTranslation;
	vTranslation.Multiply( TVector4::VEC_NEGY, a_fHeight * 0.5f + a_fRadius );
	matOffset.SetTranslation( vTranslation );

	CreatePhysics( pTRBData, &pShape, &matOffset, 1, a_pPhysicsData, TFALSE );
}

// $deBlob: FUNCTION 0077bc10
void ALeechBot2::Create()
{
	AEnemyGrappler::Create();

	CreateCapsule( 1.75f, 1.0f, TNULL );
	SetPhysicsMover( new AEntityPhysicsMover( this ) );
	m_uiLeechFlags |= LFLAG_Full;
}

// $deBlob: FUNCTION 0077bd90
void ALeechBot2::PostCreate()
{
	AEnemy::PostCreate();
	TSTATICCAST( AEntityPoseController, GetPoseController() )->PlayManualOverlayAnim( "Tank_Full", 0.0f, m_fTankFill, 1.0f );

	T2ModelPtr pModel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetModelPtr();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	TASSERT( 1 == TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetModelPtr()->GetLODCount() );

	// The tank (mesh 2) gets the sphere map once, shared by every LeechBot
	ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ 2 ]->GetMaterial() );

	if ( !sm_bSkinMaterialSetUp )
	{
		TTexture* pSphereMap = TTextureManager::GetSingleton()->FindTexture( "spheremap_leechbot.tga" );
		pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

		TASSERT( pSkinMat->GetNumTextures() == 1 );
		pSkinMat->SetTextureNum( 2 );
		pSkinMat->SetTevMode( 1, 0xC );
		pSkinMat->SetTexture( 1, pSphereMap );
		pSkinMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
		pSkinMat->SetBlendMode( ASkinMaterial::BLENDMODE( 0x18 ), 0 );

		sm_bSkinMaterialSetUp = TTRUE;
	}

	TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ 0 ]->GetMaterial() )->SetBlendMode( ASkinMaterial::BLENDMODE( 0 ), 0 );
	TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ 1 ]->GetMaterial() )->SetBlendMode( ASkinMaterial::BLENDMODE( 0 ), 0 );
}

// $deBlob: FUNCTION 0077bfd0
void ALeechBot2::ReleaseGrapple()
{
	TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->SetScheduleTimer( 3.0f );

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x1F ), ASoundMap::EVENTID( 0x153 ), m_oMatrix.GetTranslation(), 0 );
	AEnemyGrappler::ReleaseGrapple();

	EmptyTank();
}

// $deBlob: FUNCTION 0077c010
void ALeechBot2::InitialiseComponents()
{
	ACharEntity::InitialiseComponents();

	if ( m_uiLeechFlags & LFLAG_Static )
	{
		TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TFALSE );
		TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TFALSE );
		TSTATICCAST( AEntityComponent2, GetScheduleComponent() )->SetEnabled( TFALSE );
	}
}

// $deBlob: FUNCTION 0077cd60
TINT ALeechBot2::GetKillScore()
{
	// The score of a type 2 LeechBot killed before its tank is full (BlobScoringProperties 0x90)
	if ( m_eLeechbotType == LeechBot_Empty && m_fTankFill < 1.0f )
		return TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x90 / 4 ];

	return 0;
}

// $deBlob: FUNCTION 0077cdc0
TFLOAT ALeechBot2::GetFidgetFreq()
{
	return m_fTankFill < 1.0f ? GetEnemyData()->m_fFidgetFreq : 0.0f;
}

// $deBlob: FUNCTION 0077ce40
TFLOAT ALeechBot2::GetAttackRangeMax()
{
	return ( m_uiLeechFlags & LFLAG_Static ) ? 0.0f : GetEnemyData()->m_fCharAttackRangeMax;
}

// $deBlob: FUNCTION 0077ce90
TFLOAT ALeechBot2::GetAttackRangeOptimal()
{
	return ( m_uiLeechFlags & LFLAG_Static ) ? 0.0f : GetEnemyData()->m_fCharAttackRangeOptimal;
}

// $deBlob: FUNCTION 0077d140
TBOOL ALeechBot2::IsDead() const
{
	if ( 0x14 == m_iState )
		return TTRUE;

	TBOOL bDead = ( m_Flags >> 22 ) & 1;
	return bDead;
}

// $deBlob: FUNCTION 0077d180
void ALeechBot2::SetCustomData( TINT a_iCustomData )
{
	m_eLeechbotType = LeechbotType( a_iCustomData & 0x7F );

	if ( TINT8( a_iCustomData ) < 0 )
		m_uiLeechFlags |= LFLAG_Static;
	else
		m_uiLeechFlags &= ~LFLAG_Static;
}

// $deBlob: FUNCTION 0077b650
void ALeechBot2::LoadEffects()
{
	TParticleEffect* pDestroyEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_LeechBotDestroy" );

	for ( TINT i = 0; i < NUM_PRIMARY_COLOURS; i++ )
	{
		TParticleEffect* pEffect = pDestroyEffect->Copy();
		AParticleEffectHelper::SetColour( pEffect, ABlobColourComponent::ms_auiColours[ 1 + i ], "Blob_InkPod_Paint" );
		AParticleEffectHelper::SetColour( pEffect, ABlobColourComponent::ms_auiColours[ 1 + i ], "Blob_InkPod_Spray" );
		ms_apPrimaryDestroyEffects[ i ] = pEffect;
	}

	for ( TINT i = 0; i < NUM_SECONDARY_COLOURS; i++ )
	{
		TParticleEffect* pEffect = pDestroyEffect->Copy();
		AParticleEffectHelper::SetColour( pEffect, ABlobColourComponent::ms_auiColours[ 8 + i ], "Blob_InkPod_Paint" );
		AParticleEffectHelper::SetColour( pEffect, ABlobColourComponent::ms_auiColours[ 8 + i ], "Blob_InkPod_Spray" );
		ms_apSecondaryDestroyEffects[ i ] = pEffect;
	}

	ms_pExplosionEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_LeechBot_Explosion" );
}

// $deBlob: FUNCTION 0077b710
void ALeechBot2::ClearEffects()
{
	for ( TINT i = 0; i < NUM_SECONDARY_COLOURS; i++ )
	{
		if ( ms_apSecondaryDestroyEffects[ i ] != TNULL )
			delete ms_apSecondaryDestroyEffects[ i ];

		ms_apSecondaryDestroyEffects[ i ] = TNULL;
	}

	for ( TINT i = 0; i < NUM_PRIMARY_COLOURS; i++ )
	{
		if ( ms_apPrimaryDestroyEffects[ i ] != TNULL )
			delete ms_apPrimaryDestroyEffects[ i ];

		ms_apPrimaryDestroyEffects[ i ] = TNULL;
	}
}
