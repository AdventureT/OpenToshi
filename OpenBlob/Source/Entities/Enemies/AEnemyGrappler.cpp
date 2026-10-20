#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "AEnemyGrappler.h"
#include "Entities/AEntityIOPacket.h"
#include "Collision/ACollisionObject.h"
#include "Entities/AGndMotorComponent.h"
#include "GameInterface/APlayerManager.h"
#include "AEnemyBehaviour.h"
#include "Entities/AI/Schedule/AAISchedule.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/PathFinding/APathfindingComponent.h"
#include "Sound/ASoundInterface.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Dynamic initializer
// $deBlob: FUNCTION 00541740
AEnemyGrappler::PlayerBones AEnemyGrappler::sm_PlayerBones[ 4 ];

TSTATICASSERT( sizeof( AEnemyGrappler ) == 0x11C );

// $deBlob: FUNCTION 007b6530
// $deBlob: FUNCTION 007b6540
// $deBlob: FUNCTION 007b6550
// $deBlob: FUNCTION 007b6560
// $deBlob: FUNCTION 007b6570
// $deBlob: FUNCTION 00541700
// $deBlob: FUNCTION 0085a040
TDEFINE_CLASS_NORUNTIME( AEnemyGrappler );

// $deBlob: FUNCTION 007b6580
AEnemyGrappler::AEnemyGrappler()
{
	m_eGrapplerState = AEnemyGrapplerState( -1 );
	m_iPlayerIndex   = ENEMYGRAPPLER_INVALID_PLAYERINDEX;
	m_iHitsLeft      = 0;
	m_pPlayer        = TNULL;
	m_vPlayerNormal  = TVector4::VEC_NEGY;
	m_vPlayerTangent = TVector4::VEC_POSX;
	m_uiEnemyFlags |= EFLAG_Grappler;
	m_pLiquidRayCastHelper  = TNULL;
	m_iBoneIndex            = ENEMYGRAPPLER_INVALID_BONE;
	m_uiSavedCollisionGroup = 0;
	m_uiSavedGroupMask      = 0;
}

// $deBlob: FUNCTION 007b8250
TBOOL AEnemyGrappler::PlayerGrappled( ABaseEntity* a_pPlayer, const TVector4& a_rcNormal )
{
	if ( a_pPlayer != TNULL && ( TUINT8( a_pPlayer->GetFlags() >> 4 ) & 1 ) )
	{
		ABlobEntity* pBlob = a_pPlayer->CastTo<ABlobEntity>();

		// Not while it Z-jumps
		if ( pBlob->GetZJumpState() != ABlobZJumpComponent::STATE_ATTACKSLAM )
		{
			if ( FindFreeBone( pBlob, a_rcNormal ) )
			{
				APositionalSoundManager::GetSingleton()->AddEntitySound( this, 0x1F, 0x154 );
				SetGrapplerState( GRAPPLERSTATE_Grappling );
				return TTRUE;
			}

			SetGrapplerState( GRAPPLERSTATE_None );
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007b8300
TBOOL AEnemyGrappler::FindFreeBone( ABaseEntity* a_pPlayer, const TVector4& a_rcNormal )
{
	if ( a_pPlayer == TNULL )
		return TFALSE;

	for ( TINT iPlayer = 0; iPlayer < TINT( TARRAYSIZE( sm_PlayerBones ) ); iPlayer++ )
	{
		if ( sm_PlayerBones[ iPlayer ].m_pPlayer != a_pPlayer )
			continue;

		for ( TINT iBone = 0; iBone < ENEMYGRAPPLER_MAX_GRAPPLERS; iBone++ )
		{
			if ( sm_PlayerBones[ iPlayer ].m_apGrapplers[ iBone ] == TNULL )
			{
				sm_PlayerBones[ iPlayer ].m_apGrapplers[ iBone ] = this;
				m_iBoneIndex                                     = TINT8( iBone );
				m_pPlayer                                        = a_pPlayer;
				m_iPlayerIndex                                   = TINT8( iPlayer );
				m_vPlayerNormal                                  = a_rcNormal;
				TASSERT( m_vPlayerNormal.IsNormalised() );

				// The tangent: across the normal and down (or the side axis when the normal is vertical)
				static const TVector4 s_vSide( 1.0f, 0.0f, 0.0f, 1.0f );
				const TVector4&       rcAxis = ( 0.99f <= TMath::Abs( m_vPlayerNormal.y * TVector4::VEC_NEGY.y + m_vPlayerNormal.x * TVector4::VEC_NEGY.x + m_vPlayerNormal.z * TVector4::VEC_NEGY.z ) ) ? s_vSide : TVector4::VEC_NEGY;
				m_vPlayerTangent.CrossProduct( a_rcNormal, rcAxis );
				m_vPlayerTangent.Normalise();

				// Both into the space of the blob's body
				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pPlayer );
				TMatrix44    oBody = pBlob->GetPhysicsMatrix();
				oBody.m_f41        = TVector3::VEC_ZERO.x;
				oBody.m_f42        = TVector3::VEC_ZERO.y;
				oBody.m_f43        = TVector3::VEC_ZERO.z;
				oBody.Invert( oBody );
				TMatrix44::TransformVector( m_vPlayerNormal, oBody, m_vPlayerNormal );
				TMatrix44::TransformVector( m_vPlayerTangent, oBody, m_vPlayerTangent );

				pBlob->AddLeechbot();
				return TTRUE;
			}
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007b84d0
void AEnemyGrappler::ReleaseBone()
{
	if ( m_pPlayer != TNULL )
	{
		TASSERT( ENEMYGRAPPLER_INVALID_PLAYERINDEX != m_iPlayerIndex );
		TASSERT( ENEMYGRAPPLER_INVALID_BONE != m_iBoneIndex );

		// The grapplers behind move up
		for ( TINT j = m_iBoneIndex; j < ENEMYGRAPPLER_MAX_GRAPPLERS - 1; j++ )
		{
			sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ j ] = sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ j + 1 ];

			if ( sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ j ] != TNULL )
			{
				TASSERT( j == sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ j ]->m_iBoneIndex - 1 );
				sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ j ]->m_iBoneIndex = TINT8( j );
			}
		}

		sm_PlayerBones[ m_iPlayerIndex ].m_apGrapplers[ ENEMYGRAPPLER_MAX_GRAPPLERS - 1 ] = TNULL;

		TSTATICCAST( ABlobEntity, m_pPlayer )->RemoveLeechbot();

		m_pPlayer      = TNULL;
		m_iPlayerIndex = ENEMYGRAPPLER_INVALID_PLAYERINDEX;
		m_iBoneIndex   = ENEMYGRAPPLER_INVALID_BONE;
	}
}

// $deBlob: FUNCTION 007b8640
void AEnemyGrappler::GetBoneMatrix( TMatrix44& a_rMatrix, ABaseEntity* a_pPlayer, TINT a_iIndex )
{
	TINT i = 0;

	while ( sm_PlayerBones[ i ].m_pPlayer != a_pPlayer )
	{
		i++;

		if ( i >= TINT( TARRAYSIZE( sm_PlayerBones ) ) )
			return;
	}

	TASSERT( a_pPlayer == sm_PlayerBones[ i ].m_apGrapplers[ a_iIndex ]->m_pPlayer );
	AEnemyGrappler* pGrappler = sm_PlayerBones[ i ].m_apGrapplers[ a_iIndex ];

	TVector4 vNormal  = pGrappler->m_vPlayerNormal;
	TVector4 vTangent = pGrappler->m_vPlayerTangent;

	// The grapple's rotation: the tangent, the normal and their cross product
	TMatrix44 oBone;
	oBone.m_f11 = vTangent.x;
	oBone.m_f12 = vTangent.y;
	oBone.m_f13 = vTangent.z;
	oBone.m_f14 = TMatrix44::IDENTITY.m_f14;
	oBone.m_f21 = vNormal.x;
	oBone.m_f22 = vNormal.y;
	oBone.m_f23 = vNormal.z;
	oBone.m_f24 = TMatrix44::IDENTITY.m_f24;
	oBone.m_f31 = vTangent.y * vNormal.z - vNormal.y * vTangent.z;
	oBone.m_f32 = vNormal.x * vTangent.z - vTangent.x * vNormal.z;
	oBone.m_f33 = vNormal.y * vTangent.x - vTangent.y * vNormal.x;
	oBone.m_f34 = TMatrix44::IDENTITY.m_f34;
	oBone.m_f41 = TMatrix44::IDENTITY.m_f41;
	oBone.m_f42 = TMatrix44::IDENTITY.m_f42;
	oBone.m_f43 = TMatrix44::IDENTITY.m_f43;
	oBone.m_f44 = TMatrix44::IDENTITY.m_f44;

	TMatrix44 oMatrix = a_rMatrix;
	a_rMatrix.Multiply( oBone, oMatrix );

	// On the body's surface
	TFLOAT fOffset = -a_pPlayer->GetBoundingSphereRadius();
	a_rMatrix.m_f41 += vNormal.x * fOffset;
	a_rMatrix.m_f42 += vNormal.y * fOffset;
	a_rMatrix.m_f43 += vNormal.z * fOffset;
}

// $deBlob: FUNCTION 007b65f0
// $deBlob: FUNCTION 007b88a0
AEnemyGrappler::~AEnemyGrappler()
{
}

// $deBlob: FUNCTION 007b6600
void AEnemyGrappler::AddPlayer( ABaseEntity* a_pPlayer )
{
	for ( TINT i = 0; i < TINT( TARRAYSIZE( sm_PlayerBones ) ); i++ )
	{
		if ( sm_PlayerBones[ i ].m_pPlayer == TNULL )
		{
			sm_PlayerBones[ i ].m_pPlayer = a_pPlayer;
			return;
		}
	}
}

// $deBlob: FUNCTION 007b6640
void AEnemyGrappler::ClearPlayers()
{
	for ( TINT i = 0; i < TINT( TARRAYSIZE( sm_PlayerBones ) ); i++ )
	{
		sm_PlayerBones[ i ].m_pPlayer = TNULL;

		for ( TINT j = 0; j < ENEMYGRAPPLER_MAX_GRAPPLERS; j++ )
			sm_PlayerBones[ i ].m_apGrapplers[ j ] = TNULL;
	}
}

// $deBlob: FUNCTION 007b66b0
TBOOL AEnemyGrappler::IsPlayerGrappled( ABaseEntity* a_pPlayer )
{
	TINT iPlayerIndex;

	for ( iPlayerIndex = 0; iPlayerIndex < TINT( TARRAYSIZE( sm_PlayerBones ) ); iPlayerIndex++ )
	{
		if ( a_pPlayer == sm_PlayerBones[ iPlayerIndex ].m_pPlayer )
			break;
	}

	if ( iPlayerIndex == TARRAYSIZE( sm_PlayerBones ) )
		return TFALSE;

	for ( TINT i = 0; i < ENEMYGRAPPLER_MAX_GRAPPLERS; i++ )
	{
		if ( sm_PlayerBones[ iPlayerIndex ].m_apGrapplers[ i ] != TNULL )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007b6700
void AEnemyGrappler::PlayerStruggle( ABaseEntity* a_pPlayer )
{
	TASSERT( a_pPlayer );

	TINT iPlayerIndex;

	for ( iPlayerIndex = 0; iPlayerIndex < TINT( TARRAYSIZE( sm_PlayerBones ) ); iPlayerIndex++ )
	{
		if ( a_pPlayer == sm_PlayerBones[ iPlayerIndex ].m_pPlayer )
			break;
	}

	TASSERT( iPlayerIndex != APlayerManager::MAX_PLAYERS );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail goes on with sm_PlayerBones[ 4 ], past the array, when the player isn't in it
	if ( iPlayerIndex == APlayerManager::MAX_PLAYERS )
		return;
#endif

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_GRAPPLESTRUGGLE;
	oPacket.m_iData    = 0;

	for ( TINT i = 0; i < ENEMYGRAPPLER_MAX_GRAPPLERS; i++ )
	{
		AEnemyGrappler* pGrappler = sm_PlayerBones[ iPlayerIndex ].m_apGrapplers[ i ];

		if ( pGrappler != TNULL )
		{
			pGrappler->HandleEntityIO( a_pPlayer, oPacket );
			return;
		}
	}
}

// $deBlob: FUNCTION 007b67e0
void AEnemyGrappler::ReleasePlayer( ABaseEntity* a_pPlayer )
{
	TASSERT( a_pPlayer );

	TINT iPlayerIndex;

	for ( iPlayerIndex = 0; iPlayerIndex < TINT( TARRAYSIZE( sm_PlayerBones ) ); iPlayerIndex++ )
	{
		if ( a_pPlayer == sm_PlayerBones[ iPlayerIndex ].m_pPlayer )
			break;
	}

	TASSERT( iPlayerIndex != APlayerManager::MAX_PLAYERS );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail goes on with sm_PlayerBones[ 4 ], past the array, when the player isn't in it
	if ( iPlayerIndex == APlayerManager::MAX_PLAYERS )
		return;
#endif

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_GRAPPLERELEASE;
	oPacket.m_iData    = 0;

	for ( TINT i = 0; i < ENEMYGRAPPLER_MAX_GRAPPLERS; i++ )
	{
		if ( sm_PlayerBones[ iPlayerIndex ].m_apGrapplers[ i ] != TNULL )
			sm_PlayerBones[ iPlayerIndex ].m_apGrapplers[ i ]->HandleEntityIO( a_pPlayer, oPacket );
	}
}

// $deBlob: FUNCTION 007b6670
void AEnemyGrappler::ReleaseAllGrapples( ABaseEntity* a_pPlayer )
{
	TINT8 iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pPlayer );

	for ( TINT i = ENEMYGRAPPLER_MAX_GRAPPLERS - 1; i >= 0; i-- )
	{
		if ( sm_PlayerBones[ iPlayer ].m_apGrapplers[ i ] != TNULL )
			sm_PlayerBones[ iPlayer ].m_apGrapplers[ i ]->ReleaseGrapple();
	}
}

// $deBlob: FUNCTION 007b68b0
TINT AEnemyGrappler::GetStruggleProgress( ABaseEntity* a_pPlayer )
{
	TINT iPlayerIndex;

	for ( iPlayerIndex = 0; iPlayerIndex < TINT( TARRAYSIZE( sm_PlayerBones ) ); iPlayerIndex++ )
	{
		if ( a_pPlayer == sm_PlayerBones[ iPlayerIndex ].m_pPlayer )
			break;
	}

	TASSERT( iPlayerIndex != APlayerManager::MAX_PLAYERS );

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail goes on with sm_PlayerBones[ 4 ], past the array, when the player isn't in it
	if ( iPlayerIndex == APlayerManager::MAX_PLAYERS )
		return 0;
#endif

	ABlobEntity* pBlob      = TSTATICCAST( ABlobEntity, a_pPlayer );
	TINT         iTotalHits = -1;
	TINT         iHitsLeft  = 0;

	for ( TINT i = 0; i < ENEMYGRAPPLER_MAX_GRAPPLERS; i++ )
	{
		AEnemyGrappler* pGrappler = sm_PlayerBones[ iPlayerIndex ].m_apGrapplers[ i ];

		if ( pGrappler != TNULL )
		{
			if ( iTotalHits == -1 )
				iTotalHits = pGrappler->GetGrapplerData()->m_iStruggleCount * pBlob->GetMaxLeechbotsAttached();

			iHitsLeft += pGrappler->m_iHitsLeft;
		}
	}

	// NOTE: with no grappler on the blob, -1 / -1: 100
	return TINT( ( TFLOAT( iTotalHits - iHitsLeft ) / TFLOAT( iTotalHits ) ) * 100.0f );
}

// $deBlob: FUNCTION 007b69e0
void AEnemyGrappler::Create()
{
	ACharEntity::Create();

	if ( m_pLiquidRayCastHelper == TNULL )
	{
		m_pLiquidRayCastHelper = TPhysicsManager::GetSingleton()->CreateAabbPhantom();
		TPhysicsManager::SetCollisionTestFlags( m_pLiquidRayCastHelper, 0x20 );
	}
}

// $deBlob: FUNCTION 007b6a40
void AEnemyGrappler::Destroy()
{
	ABaseEntity::Destroy();

	if ( m_pLiquidRayCastHelper != TNULL )
	{
		TPhysicsManager::GetSingleton()->DestroyPhantom( m_pLiquidRayCastHelper );
		m_pLiquidRayCastHelper = TNULL;
	}
}

// $deBlob: FUNCTION 007b6a70
void AEnemyGrappler::SetActive( TBOOL a_bActive, TBOOL a_bUnk )
{
	AEnemy::SetActive( a_bActive, a_bUnk );

	if ( a_bActive )
	{
		m_uiSavedCollisionGroup = TUINT8( m_iCollisionGroupAndType );
		m_uiSavedGroupMask      = GetCollisionObject()->GetGroupMask();

		TASSERT( TNULL == m_pPlayer );
		TASSERT( ENEMYGRAPPLER_INVALID_PLAYERINDEX == m_iPlayerIndex );

		m_iBoneIndex = ENEMYGRAPPLER_INVALID_BONE;
		m_iHitsLeft  = 0;
	}

	SetGrapplerState( GRAPPLERSTATE_None );
}

// $deBlob: FUNCTION 007b6b40
void AEnemyGrappler::Tick( TFLOAT a_fDeltaTime )
{
	AEnemy::Tick( a_fDeltaTime );

	if ( m_eGrapplerState == GRAPPLERSTATE_Attacking )
	{
		// The leap: hits liquid between the last and the current position?
		TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

		TVector4 vMove;
		vMove.Multiply( TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetVelocity(), a_fDeltaTime );

		TPhysicsRay oRay;
		oRay.m_vFrom.x          = m_oMatrix.m_f41 - vMove.x;
		oRay.m_vFrom.y          = ( m_oMatrix.m_f42 - vMove.y ) - m_vBBox.y;
		oRay.m_vFrom.z          = m_oMatrix.m_f43 - vMove.z;
		oRay.m_vFrom.w          = m_oMatrix.m_f44;
		oRay.m_vTo              = m_oMatrix.GetTranslation();
		oRay.m_uiCollisionLayer = 0x20;

		// The liquid ray cast helper's box around the ray
		TVector4 aBox[ 2 ];
		TVector4 vSum( oRay.m_vFrom.x + oRay.m_vTo.x, oRay.m_vFrom.y + oRay.m_vTo.y, oRay.m_vFrom.z + oRay.m_vTo.z, oRay.m_vTo.w );
		aBox[ 0 ].Multiply( vSum, 0.5f );
		TVector4 vDiff( oRay.m_vFrom.x - oRay.m_vTo.x, oRay.m_vFrom.y - oRay.m_vTo.y, oRay.m_vFrom.z - oRay.m_vTo.z, oRay.m_vTo.w );
		aBox[ 1 ].Multiply( vDiff, 0.5f );
		aBox[ 1 ].Abs4();
		pPhysicsManager->SetAabbPhantomBox( m_pLiquidRayCastHelper, aBox );

		TASSERT( pPhysicsManager->GetCollisionTestFlags( m_pLiquidRayCastHelper ) == oRay.m_uiCollisionLayer, "pPM->GetCollisionTestFlags(m_pLiquidRayCastHelper) == oRayCast.m_iTestFlags" );

		TPhysicsShapeContact oContact;

		if ( pPhysicsManager->CollideRay( oRay, oContact, m_pLiquidRayCastHelper ) )
		{
			// Into the water: it dies with a splash
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_DEATHZONE;
			oPacket.m_iData    = 0;
			HandleEntityIO( TNULL, oPacket );

			TParticleEffectInstance* pSplash = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ABlobEntity::ms_pWaterSplashEffect );

			if ( pSplash != TNULL )
				pSplash->GetTransform().SetTranslation( oContact.m_vPosition );
		}
	}
	else if ( m_eGrapplerState == GRAPPLERSTATE_Grappling )
	{
		if ( m_pPlayer == TNULL )
		{
			SetGrapplerState( GRAPPLERSTATE_None );
			return;
		}

		// Sticks to the blob
		TMatrix44 oTransform;
		GetGrappleTransform( oTransform );

		TASSERT( oTransform.AsBasisVector3( BASISVECTOR_RIGHT ).IsNormalised() );
		TASSERT( oTransform.AsBasisVector3( BASISVECTOR_UP ).IsNormalised() );
		TASSERT( oTransform.AsBasisVector3( BASISVECTOR_FORWARD ).IsNormalised() );
		SetMatrix( oTransform );

		// The first grappler counts the time
		if ( m_iBoneIndex == 0 )
			sm_PlayerBones[ m_iPlayerIndex ].m_fGrappleTime += a_fDeltaTime;
	}
}

// $deBlob: FUNCTION 007b70f0
void AEnemyGrappler::OnEnterState( TINT a_iState )
{
	switch ( a_iState )
	{
		case 0xE:
		{
			StartAttack();
		}
		break;
		case 0x11:
		{
			m_iCollisionGroupAndType = ( m_iCollisionGroupAndType & ~0xFF ) | m_uiSavedCollisionGroup;
			GetCollisionObject()->SetGroupMask( m_uiSavedGroupMask );
		}
		break;
		default:
		{
			AEnemy::OnEnterState( a_iState );

			if ( a_iState < 0xD && m_eGrapplerState == GRAPPLERSTATE_Attacking )
				SetGrapplerState( GRAPPLERSTATE_None );
		}
		break;
	}
}

// $deBlob: FUNCTION 007b7160
void AEnemyGrappler::OnLeaveState( TINT a_iState )
{
	if ( a_iState == 0x10 && m_eGrapplerState == GRAPPLERSTATE_Attacking )
		SetGrapplerState( GRAPPLERSTATE_None );

	AEnemy::OnLeaveState( a_iState );
}

// $deBlob: FUNCTION 007b7190
void AEnemyGrappler::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( TUINT8( m_Flags >> 22 ) & 1 )
		return;

	TINT iMessage = a_rPacket.m_eMessage;

	switch ( iMessage )
	{
		case AEntityIOPacket::MESSAGE_TOUCH:
		case AEntityIOPacket::MESSAGE_TOUCHEDBY:
			// Touched: the leap grabs the blob (the message's data is the contact, its normal at 0x10)
			if ( m_eGrapplerState == GRAPPLERSTATE_Attacking && m_pPlayer == TNULL )
				PlayerGrappled( a_pCaller, *TREINTERPRETCAST( const TVector4*, a_rPacket.m_iData + 0x10 ) );
			break;
		case AEntityIOPacket::MESSAGE_GRAPPLESTRUGGLE:
			if ( a_pCaller != m_pPlayer )
				return;

			if ( m_eGrapplerState == GRAPPLERSTATE_Grappling )
			{
				// The blob struggles
				if ( --m_iHitsLeft <= 0 )
				{
					ReleaseGrapple();
					AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
					return;
				}

				SetState( 0x12 );
				AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
				return;
			}
			break;
	}

	AEnemy::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007b7240
void AEnemyGrappler::StartCurrentStateTask()
{
	switch ( m_iCurrentTask )
	{
		case 0xA:
			SetState( 0xD );
			break;
		case 0x24:
			SetState( 0x11 );
			break;
		default:
			// NOTE: AEnemy::StartCurrentStateTask
			AEnemy::StartCurrentStateTask();
			break;
	}
}

// $deBlob: FUNCTION 007b7270
TBOOL AEnemyGrappler::UpdateCurrentStateTask( TFLOAT a_fDeltaTime )
{
	if ( m_iCurrentTask == 0xA )
	{
		switch ( m_iState )
		{
			case 0xD:
			case 0xE:
				if ( HasStateFinished() )
				{
					SetState( m_iState + 1 );
					return TFALSE;
				}
				break;
			case 0xF:
			{
				// The leap lands
				AGndMotorComponent* pMotor = TSTATICCAST( AGndMotorComponent, GetMotorComponent() );

				if ( pMotor->IsOnGround( 0.03f ) && 0.0f <= TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetVelocity().y )
					SetState( m_iState + 1 );

				break;
			}
			case 0x10:
				return HasStateFinished();
			default:
				return TTRUE;
		}

		return TFALSE;
	}

	if ( m_iCurrentTask != 0x24 )
		return AEnemy::UpdateCurrentStateTask( a_fDeltaTime );

	if ( m_eGrapplerState != GRAPPLERSTATE_Grappling )
		return TTRUE;

	if ( 0x12 == m_iState && HasStateFinished() )
		SetState( 0x11 );

	return TFALSE;
}

// $deBlob: FUNCTION 007b73a0
void AEnemyGrappler::ReleaseGrapple()
{
	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1f, 0x154 );

	SetGrapplerState( GRAPPLERSTATE_None );
}

// $deBlob: FUNCTION 007b73d0
void AEnemyGrappler::InitIdleState()
{
	AEnemy::InitIdleState();

	if ( m_eGrapplerState == GRAPPLERSTATE_Attacking )
		SetGrapplerState( GRAPPLERSTATE_None );
}

// $deBlob: FUNCTION 007b73f0
void AEnemyGrappler::SetGrapplerState( AEnemyGrapplerState a_eState )
{
	AEnemyGrapplerState eOldState = m_eGrapplerState;
	m_eGrapplerState              = a_eState;
	TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->ClearFlags( AGndMotorComponent::FLAG_CustomGravity );

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( eOldState == GRAPPLERSTATE_Attacking )
	{
		TASSERT( m_pLiquidRayCastHelper, "phantomContainer" );
		TASSERT( m_pLiquidRayCastHelper->IsInSimulation(), "pPM->IsQueryHelperInSimulation(m_pLiquidRayCastHelper)" );
		pPhysicsManager->RemovePhantomFromSimulation( m_pLiquidRayCastHelper, TNULL );
	}

	switch ( m_eGrapplerState )
	{
		case GRAPPLERSTATE_None:
		{
			if ( eOldState == GRAPPLERSTATE_Grappling )
			{
				// Let go: drops off the blob, beside it where there's room
				ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x1F ), ASoundMap::EVENTID( 0x155 ), m_oMatrix.GetTranslation(), 0 );
				APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1F, 0x154 );

				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, m_pPlayer );

				TVector4 vBlobVelocity;
				pPhysicsManager->GetLinearVelocity( AEnemy::GetBlobBody( pBlob ), vBlobVelocity );
				TFLOAT fBlobVelocityY = vBlobVelocity.y;
				TFLOAT fBlobSpeedSqXZ = vBlobVelocity.MagnitudeSqXZ();

				const TVector4& rcBlobPos  = pBlob->GetMatrix().GetTranslation();
				const TVector4& rcBodyPos  = pBlob->GetBodyPosition();
				TFLOAT          fBlobSize  = pBlob->GetBoundingSphereRadius();
				TFLOAT          fRayLength = m_fBoundingSphereRadius * 3.0f;

				TVector4 vAway( m_oMatrix.m_f41 - rcBlobPos.x, m_oMatrix.m_f42 - rcBlobPos.y, m_oMatrix.m_f43 - rcBlobPos.z, m_oMatrix.m_f44 );
				vAway.NormaliseXZ();

				TPhysicsRay oRay;
				oRay.m_uiCollisionLayer = 0x81;

				TVector4 vOffset;
				vOffset.Multiply( vAway, fBlobSize );
				oRay.m_vFrom = TVector4( rcBodyPos.x + vOffset.x, rcBodyPos.y + vOffset.y, rcBodyPos.z + vOffset.z, rcBodyPos.w );
				vOffset.Multiply( vAway, fRayLength );
				oRay.m_vTo = TVector4( oRay.m_vFrom.x + vOffset.x, oRay.m_vFrom.y + vOffset.y, oRay.m_vFrom.z + vOffset.z, rcBodyPos.w );

				TBOOL                bFree = TTRUE;
				TPhysicsShapeContact oContact;

				if ( pPhysicsManager->CollideRay( oRay, oContact, TNULL ) )
				{
					// Away from the blob is blocked: the sides
					TVector4 vAwayDir = vAway;
					vAway.RotateY( TMath::PI / 2 );

					vOffset.Multiply( vAwayDir, fBlobSize );
					oRay.m_vFrom = TVector4( rcBodyPos.x + vOffset.x, rcBodyPos.y + vOffset.y, rcBodyPos.z + vOffset.z, rcBodyPos.w );
					vOffset.Multiply( vAway, fRayLength );
					oRay.m_vTo = TVector4( oRay.m_vFrom.x + vOffset.x, oRay.m_vFrom.y + vOffset.y, oRay.m_vFrom.z + vOffset.z, rcBodyPos.w );

					if ( pPhysicsManager->CollideRay( oRay, oContact, TNULL ) )
					{
						vOffset.Multiply( vAwayDir, fBlobSize );
						oRay.m_vFrom = TVector4( rcBodyPos.x - vOffset.x, rcBodyPos.y - vOffset.y, rcBodyPos.z - vOffset.z, rcBodyPos.w );
						vOffset.Multiply( vAway, fRayLength );
						oRay.m_vTo = TVector4( oRay.m_vFrom.x - vOffset.x, oRay.m_vFrom.y - vOffset.y, oRay.m_vFrom.z - vOffset.z, rcBodyPos.w );

						if ( pPhysicsManager->CollideRay( oRay, oContact, TNULL ) )
						{
							// NOTE: the last try starts on the side, not away from the blob
							vOffset.Multiply( vAway, fBlobSize );
							oRay.m_vFrom = TVector4( rcBodyPos.x - vOffset.x, rcBodyPos.y - vOffset.y, rcBodyPos.z - vOffset.z, rcBodyPos.w );
							vOffset.Multiply( vAway, fRayLength );
							oRay.m_vTo = TVector4( oRay.m_vFrom.x - vOffset.x, oRay.m_vFrom.y - vOffset.y, oRay.m_vFrom.z - vOffset.z, rcBodyPos.w );

							bFree = !pPhysicsManager->CollideRay( oRay, oContact, TNULL );
						}
					}
				}

				TVector4 vPosition = m_oMatrix.GetTranslation();

				if ( bFree )
				{
					// The middle of the free ray
					TVector4 vMiddle;
					vMiddle.Divide( TVector4( oRay.m_vTo.x + oRay.m_vFrom.x, oRay.m_vTo.y + oRay.m_vFrom.y, oRay.m_vTo.z + oRay.m_vFrom.z, oRay.m_vTo.w ), 2.0f );
					vPosition.x = vMiddle.x;
					vPosition.z = vMiddle.z;
					vPosition.w = vMiddle.w;
				}
				else
				{
					// Nowhere to go: the blob is pushed away and the grappler drops under it
					if ( fBlobSpeedSqXZ < 10.0f )
					{
						TVector4 vPush = vAway;
						vPush.y        = fBlobVelocityY - 0.1f;
						vPush.Multiply( 10.0f );
						pPhysicsManager->SetLinearVelocity( GetBlobBody( pBlob ), vPush );
					}

					vPosition.y = ( rcBodyPos.y - fBlobSize ) - 0.5f;
				}

				TMatrix44 oMatrix = TMatrix44::IDENTITY;
				oMatrix.SetTranslation( vPosition );
				SetMatrix( oMatrix );
				SyncBodyTransform();
			}

			TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TTRUE );
			TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TTRUE );
			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( GetEnemyData()->m_fRunSpeed );

			m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & ~0xFF ) | m_uiSavedCollisionGroup );
			GetCollisionObject()->SetGroupMask( m_uiSavedGroupMask );
			ReleaseBone();

			if ( eOldState == GRAPPLERSTATE_Grappling && m_pPhysicsData != TNULL )
			{
				// Its body is back in the simulation
				if ( m_pPhysicsData->m_pPhysicsMover != TNULL )
					pPhysicsManager->AddCustomMover( m_pPhysicsData->m_pPhysicsMover );

				pPhysicsManager->AddToSimulation( m_pPhysicsData->m_pBody, TFALSE );
				pPhysicsManager->UpdateBodyTransform( m_pPhysicsData->m_pBody, TFALSE );
				return;
			}

			break;
		}
		case GRAPPLERSTATE_Attacking:
		{
			// Leaps at the selected enemy (where it will be in 0.3 seconds)
			TVector4 vVelocity( m_oMatrix.m_f31, -2.0f, m_oMatrix.m_f33, m_oMatrix.m_f34 );

			TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TTRUE );
			TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TTRUE );

			TFLOAT fSpeed = GetGrapplerData()->m_fAttackSpeed;
			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( fSpeed );

			TFLOAT fTime;

			if ( TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy() == TNULL )
			{
				fTime = 0.5f;
			}
			else
			{
				ABlobEntity* pBlob = TSTATICCAST( AAIBehaviour, GetBehaviourComponent() )->GetSelectedEnemy()->CastTo<ABlobEntity>();

				TVector4 vTarget = TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->GetEnemyPredictedPosition( pBlob, 0.3f, TNULL );
				vTarget.y -= pBlob->GetBBox().y;

				vVelocity.z = vTarget.z - m_oMatrix.m_f43;
				vVelocity.x = vTarget.x - m_oMatrix.m_f41;
				vVelocity.y = vTarget.y - m_oMatrix.m_f42;
				vVelocity.w = vTarget.w;

				TFLOAT fDistance = vVelocity.MagnitudeXZ();
				vVelocity.Divide( fDistance );
				vVelocity.Multiply( GetGrapplerData()->m_fAttackSpeed );

				fTime = fDistance / GetGrapplerData()->m_fAttackSpeed;

				// A parabola that reaches the target in fTime
				AGndMotorComponent* pMotor   = TSTATICCAST( AGndMotorComponent, GetMotorComponent() );
				TFLOAT              fGravity = pMotor->GetGravity() * GetGrapplerData()->m_fAttackGravityScale;
				pMotor->SetCustomGravity( fGravity );

				vVelocity.y = -( TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->GetGravity() * 0.5f * fTime );
			}

			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetVelocity( vVelocity, fTime, 0.0f, TTRUE );

			TUINT8 uiGroupMask       = m_uiSavedGroupMask & 0xEE;
			m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & ~0xFF ) | ( m_uiSavedGroupMask & 0xEE ) );
			GetCollisionObject()->SetGroupMask( uiGroupMask );

			TASSERT( m_pLiquidRayCastHelper, "phantomContainer" );
			TASSERT( !m_pLiquidRayCastHelper->IsInSimulation(), "!pPM->IsQueryHelperInSimulation(m_pLiquidRayCastHelper)" );
			pPhysicsManager->AddPhantomToSimulation( m_pLiquidRayCastHelper, m_oMatrix.GetTranslation(), TNULL );
			return;
		}
		case GRAPPLERSTATE_Grappling:
		{
			TASSERT( m_pPlayer, "Grappler: Requires player before grappling" );

			TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TFALSE );
			TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TFALSE );
			TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( GetEnemyData()->m_fRunSpeed );

			TUINT8 uiGroupMask       = m_uiSavedGroupMask & 0xEE;
			m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & ~0xFF ) | ( m_uiSavedGroupMask & 0xEE ) );
			GetCollisionObject()->SetGroupMask( uiGroupMask );

			if ( m_pPhysicsData != TNULL )
			{
				// Its body leaves the simulation while it's on the blob
				if ( m_pPhysicsData->m_pPhysicsMover != TNULL )
					pPhysicsManager->RemoveCustomMover( m_pPhysicsData->m_pPhysicsMover );

				pPhysicsManager->RemoveFromSimulation( m_pPhysicsData->m_pBody, TFALSE );
			}

			m_iHitsLeft = GetGrapplerData()->m_iStruggleCount;

			TMatrix44 oTransform;
			GetGrappleTransform( oTransform );
			TASSERT( oTransform.AsBasisVector3( BASISVECTOR_UP ).IsNormalised() );
			SetMatrix( oTransform );

			TSTATICCAST( AAISchedule, GetScheduleComponent() )->StartSchedule( 0x18 );
			return;
		}
		case GRAPPLERSTATE_Released:
			m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & ~0xFF ) | m_uiSavedCollisionGroup );
			GetCollisionObject()->SetGroupMask( m_uiSavedGroupMask );
			ReleaseBone();
			break;
	}
}

void AEnemyGrappler::GetGrappleTransform( TMatrix44& a_rOutMatrix )
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, m_pPlayer );

	// The body's rotation
	TMatrix44 oBody = pBlob->GetPhysicsMatrix();
	oBody.m_f41     = TVector3::VEC_ZERO.x;
	oBody.m_f42     = TVector3::VEC_ZERO.y;
	oBody.m_f43     = TVector3::VEC_ZERO.z;

	TVector4 vNormal = m_vPlayerNormal;
	TMatrix44::TransformVector( vNormal, oBody, vNormal );
	TVector4 vTangent = m_vPlayerTangent;
	TMatrix44::TransformVector( vTangent, oBody, vTangent );

	a_rOutMatrix.m_f11 = vTangent.x;
	a_rOutMatrix.m_f12 = vTangent.y;
	a_rOutMatrix.m_f13 = vTangent.z;
	a_rOutMatrix.m_f14 = TMatrix44::IDENTITY.m_f14;
	a_rOutMatrix.m_f21 = vNormal.x;
	a_rOutMatrix.m_f22 = vNormal.y;
	a_rOutMatrix.m_f23 = vNormal.z;
	a_rOutMatrix.m_f24 = TMatrix44::IDENTITY.m_f24;
	a_rOutMatrix.m_f31 = vTangent.y * vNormal.z - vNormal.y * vTangent.z;
	a_rOutMatrix.m_f32 = vNormal.x * vTangent.z - vTangent.x * vNormal.z;
	a_rOutMatrix.m_f33 = vNormal.y * vTangent.x - vTangent.y * vNormal.x;
	a_rOutMatrix.m_f34 = TMatrix44::IDENTITY.m_f34;

	// On the body's surface
	TVector4 vOffset;
	vOffset.Multiply( vNormal, -( pBlob->GetBoundingSphereRadius() - 1.0f ) );

	const TVector4& rcBodyPos = pBlob->GetBodyPosition();
	a_rOutMatrix.m_f41        = vOffset.x + rcBodyPos.x;
	a_rOutMatrix.m_f42        = rcBodyPos.y + vOffset.y;
	a_rOutMatrix.m_f43        = rcBodyPos.z + vOffset.z;
	a_rOutMatrix.m_f44        = rcBodyPos.w;
}
