#include "pch.h"
#include "AInkJet.h"
#include "ARandom.h"
#include "Animation/AModel2.h"
#include "Animation/AModelInstance2.h"
#include "Cameras/ACameraManager.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/AAttackMessage.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobZJumpComponent.h"
#include "Entities/ABoss/ABoss.h"
#include "Entities/AEntityHash.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityPhysicsMover.h"
#include "Entities/AJetbikeSpawner/AJetbikeSpawner.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/Enemies/AEnemyManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Sound/ASoundInterface.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/TCapsuleData.h>
#include <Render/TMesh.h>
#include <Render/TModel.h>
#include <Render/TTexture.h>
#include <TParticleSystem/TParticleEffect.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* AInkJet::ms_pExplosionEffect = TNULL;
TBOOL                   AInkJet::ms_bMaterialsSetUp  = TFALSE;

// The message of the jetbike's own collision query to the entity it hit (MESSAGE_TOUCH to itself, Tick)
struct InkJetContact
{
	TVector4 vPosition; // 0x00
	TVector4 vNormal;   // 0x10
	TINT     iUnk20;    // 0x20 0
};

// The leechbots that still want paint (Tick's local AEntityHashFilterFunctorLeechOnly)
class AEntityHashFilterFunctorLeechOnly : public AEntityHashFilter
{
public:
	// $deBlob: FUNCTION 007ade50
	virtual TBOOL operator()( ABaseEntity* a_pEntity ) override
	{
		return a_pEntity->GetClass() == &TGetClass( ALeechBot2 ) && TSTATICCAST( ALeechBot2, a_pEntity )->GetTankFill() < 1.0f;
	}
};

static TBOOL IsQueryHelperInSimulation( TPhysicsPhantomContainer* a_pContainer )
{
	TASSERT( a_pContainer, "phantomContainer" );
	return a_pContainer->IsInSimulation();
}

// The checked cast of an entity to AJetbikeSpawner
// $deBlob: FUNCTION 007ade90
static AJetbikeSpawner* CastJetbikeSpawner( ABaseEntity* a_pEntity )
{
	TASSERT( ( a_pEntity == TNULL ) || a_pEntity->IsA( &TGetClass( AJetbikeSpawner ) ) );
	return TSTATICCAST( AJetbikeSpawner, a_pEntity );
}

// The x and z of a vector
// $deBlob: FUNCTION 007adfd0
static void GetXZ( const TVector4& a_rVec, TVector2& a_rOut )
{
	a_rOut.x = a_rVec.x;
	a_rOut.y = a_rVec.z;
}

// $deBlob: FUNCTION 007ac060
// $deBlob: FUNCTION 007ac070
// $deBlob: FUNCTION 007ac0e0
// $deBlob: FUNCTION 007ac140
// $deBlob: FUNCTION 007ac150
// $deBlob: FUNCTION 00541320
// $deBlob: FUNCTION 00859f50
TDEFINE_CLASS( AInkJet );

// $deBlob: FUNCTION 007ac160
AInkJet::AInkJet()
    : m_iTargetPlayer( -1 )
    , m_uiInkJetFlags( 0 )
    , m_fRandomness( 3.0f )
    , m_fArmTime( -1.0f )
    , m_fTimeSinceSeen( 100.0f )
    , m_fHitCooldown( -1.0f )
    , m_pCollisionQueryHelper( TNULL )
{
	if ( ms_pExplosionEffect == TNULL )
		ms_pExplosionEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_Explosion" );

	CreateShadow( 4.0f, 0.75f );
	ms_bMaterialsSetUp = TFALSE;

	Activate();
	TREINTERPRETCAST( TUINT8*, this )
	[0x63] = 0x20; // NOTE: the high byte of m_iCollisionGroupAndType, written by the retail ctor
}

// $deBlob: FUNCTION 007adf00
// $deBlob: FUNCTION 007ac270
// $deBlob: FUNCTION 007adee9
AInkJet::~AInkJet()
{
	if ( m_pCollisionQueryHelper != TNULL )
	{
		TPhysicsManager::GetSingleton()->DestroyPhantom( m_pCollisionQueryHelper );
		m_pCollisionQueryHelper = TNULL;
	}

	ms_pExplosionEffect = TNULL;
}

// $deBlob: FUNCTION 007ac300
// $deBlob: FUNCTION 007ae041
void AInkJet::ProcessParams( const AEntityParams& a_rParams )
{
	APropPlatform::ProcessParams( a_rParams );
	a_rParams.GetValue( m_fRandomness, "Randomness" );

	AEntityParam::Collision** ppCollision = TNULL;

	if ( a_rParams.GetCollision( ppCollision ) )
	{
		CreatePhysics( m_pTypeData->m_pTRBData, *ppCollision, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}
	else
	{
		// The body: a capsule along z (the bike) and a capsule tilted up behind it (the rider)
		TCapsuleData oBike;
		oBike.SetHeight( 8.6f );
		oBike.SetRadius( 1.0f );

		TMatrix44 aMatrices[ 2 ];
		aMatrices[ 0 ].Identity();

		TCapsuleData oRider;
		oRider.SetHeight( 3.0f );
		oRider.SetRadius( 1.1f );

		aMatrices[ 1 ].Identity();
		aMatrices[ 1 ].RotateX( TMath::PI / 2 + 0.2f );
		aMatrices[ 1 ].m_f41 = 0.0f;
		aMatrices[ 1 ].m_f42 = -1.25f;
		aMatrices[ 1 ].m_f43 = 3.5f;

		TShapeBaseClass* apShapes[ 2 ] = { &oBike, &oRider };
		CreatePhysics( m_pTypeData->m_pTRBData, apShapes, aMatrices, 2, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}

	SetMovingSound( 26, 297 );
	m_uiPropFlags &= ~PROPFLAG_Paintable;

	CreatePoseController();
	CreateStateComponent();
	CreateFXBreakpointComponent( TNULL );
}

// $deBlob: FUNCTION 007ac500
void AInkJet::Create()
{
	AProp::Create();

	TCapsuleData oCapsule;
	oCapsule.SetHeight( 8.6f );
	oCapsule.SetRadius( 1.0f );

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	m_pCollisionQueryHelper          = pPhysicsManager->CreateShapePhantom( &oCapsule );
	pPhysicsManager->SetPhantomCollisionFilterInfo( m_pCollisionQueryHelper, 0x42000 );
}

// $deBlob: FUNCTION 007ac5c0
// $deBlob: FUNCTION 007ae036
void AInkJet::PostCreate()
{
	APropPlatform::PostCreate();
	SetState( 0 );

	if ( m_uiInkJetFlags & IJFLAG_Spawned )
	{
		// Parked until the spawner launches it
		m_uiInkJetFlags |= IJFLAG_Waiting;
		Deactivate();
	}
	else
	{
		TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

		TASSERT( !IsQueryHelperInSimulation( m_pCollisionQueryHelper ) );
		pPhysicsManager->SetPhantomCollisionFilterInfo( m_pCollisionQueryHelper, 0xC3000 );
		pPhysicsManager->AddPhantomToSimulation( m_pCollisionQueryHelper, GetTranslation(), TNULL );
	}

	// The sphere map of the model's materials (once, they're shared)
	if ( !ms_bMaterialsSetUp )
	{
		T2ModelPtr pModel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetModelPtr();
		TModelLOD& rLOD   = pModel->GetLOD( 0 );

		TTexture* pSphereMap = TTextureManager::GetSingleton()->FindTexture( "spheremap_inky.tga" );
		pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ i ]->GetMaterial() );
			TASSERT( pSkinMat->GetTextureNum() == 1 );

			pSkinMat->SetTextureNum( 2 );
			pSkinMat->SetTevMode( 1, 0xC );
			pSkinMat->SetTexture( 1, pSphereMap );
			pSkinMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
			pSkinMat->SetBlendMode( 0xD, 0 );
		}

		ms_bMaterialsSetUp = TTRUE;
	}
}

// $deBlob: FUNCTION 007ac820
void AInkJet::Tick( TFLOAT a_fDeltaTime )
{
	const InkJetData* pData = GetInkJetData();

	// Shortly after a launch the query helper only tests layer 0x42
	if ( m_fArmTime >= 0.0f && !( TUINT8( m_uiInkJetFlags >> 0 ) & 1 ) )
	{
		m_fArmTime -= a_fDeltaTime;

		if ( m_fArmTime < 0.0f )
		{
			TASSERT( IsQueryHelperInSimulation( m_pCollisionQueryHelper ) );
			TPhysicsManager::SetCollisionTestFlags( m_pCollisionQueryHelper, 0xC3 );
		}
	}

	if ( m_fHitCooldown >= 0.0f )
		m_fHitCooldown -= a_fDeltaTime;

	if ( m_fHitCooldown < 0.0f )
		m_uiInkJetFlags &= ~IJFLAG_HitPending;

	if ( ( 2 == GetState() || 4 == GetState() ) && HasStateFinished() )
		SetState( 3 );

	// Turned off but not by a spawner: back on
	if ( ( m_Flags & FLAG_Dead ) && !( TUINT8( m_uiInkJetFlags >> 2 ) & 1 ) )
		Activate();

	if ( GetLOD() != FLAG_LowLOD )
	{
		if ( m_fTimeSinceSeen < 100.0f )
			m_fTimeSinceSeen += a_fDeltaTime;

		m_iTargetPlayer = APlayerManager::GetSingleton()->GetClosestPlayer( GetTranslation() );

		ABlobEntity* pBlob = TNULL;

		if ( m_iTargetPlayer != -1 && !( TUINT8( m_uiInkJetFlags >> 0 ) & 1 ) && m_fArmTime <= 0.0f )
		{
			pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( m_iTargetPlayer ) );

			if ( !IsBlobUnavailable( pBlob ) &&
			     TMath::ConeVsSphere( GetTranslation(), m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), pData->m_fSpotFOV, pData->m_fSpotDistance, pBlob->GetTranslation() ) )
			{
				// Spotted: a stopped jetbike starts again
				if ( TUINT8( m_uiPlatformFlags >> 10 ) & 1 )
				{
					AEntityIOPacket oPacket;
					oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
					oPacket.m_iData    = 0;
					HandleEntityIO( this, oPacket );
				}

				if ( 1 == GetState() )
					SetState( 2 );

				m_fTimeSinceSeen = 0.0f;
			}
		}

		if ( 3 == GetState() )
		{
			if ( m_fTimeSinceSeen <= pData->m_fLoseTime )
			{
				if ( ( m_uiInkJetFlags & IJFLAG_CanDropLeeches ) && !( TUINT8( m_uiInkJetFlags >> 4 ) & 1 ) )
				{
					TVector4 vPosition = GetTranslation();
					TFLOAT   fRadius   = -m_fBoundingSphereRadius;

					// Only while the blob is ahead of its back
					const TVector4& rForward = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

					if ( pBlob == TNULL ||
					     rForward.y * ( ( rForward.y * fRadius + vPosition.y ) - pBlob->GetTranslation().y ) +
					             rForward.x * ( ( rForward.x * fRadius + vPosition.x ) - pBlob->GetTranslation().x ) +
					             rForward.z * ( ( rForward.z * fRadius + vPosition.z ) - pBlob->GetTranslation().z ) >=
					         0.0f )
					{
						// No hungry leechbot around yet: drop three behind it
						AEntityHashFilterFunctorLeechOnly oFilter;
						TINT                              iNumLeeches = 0;
						AEntityHash::GetSingleton()->Query( vPosition, pData->m_fLeechCheckRadius, &iNumLeeches, &oFilter, TFALSE, 1, TFALSE, TTRUE, TFALSE );

						if ( iNumLeeches == 0 )
						{
							TVector4 vFacing = rForward;
							vFacing.w        = 1.0f;

							TVector4 vBack;
							vBack.Multiply( vFacing, 6.0f );

							vPosition.x -= vBack.x;
							vPosition.y -= vBack.y;
							vPosition.z -= vBack.z;

							const TVector4& rRight = m_oMatrix.AsBasisVector4( BASISVECTOR_RIGHT );

							TVector4 vLeft( vPosition.x - rRight.x, vPosition.y - rRight.y, vPosition.z - rRight.z, vPosition.w );
							TVector4 vRight( rRight.x + vPosition.x, rRight.y + vPosition.y, rRight.z + vPosition.z, vPosition.w );

							TRandom& rRandom = ARandom::GetSingleton()->GetRandom();
							vPosition.y -= rRandom.GetFloatMinMax( 0.0f, 0.2f );
							vLeft.y -= rRandom.GetFloatMinMax( 0.2f, 0.4f );
							vRight.y -= rRandom.GetFloatMinMax( 0.4f, 0.6f );

							AEnemyManager* pEnemyManager = AEnemyManager::GetSingleton();
							pEnemyManager->CreateEnemy( AEnemyManager::ENEMYTYPE_LeechBot2, vPosition, vFacing, TNULL, -1, 0, TFALSE, TNULL, 2 );
							pEnemyManager->CreateEnemy( AEnemyManager::ENEMYTYPE_LeechBot2, vLeft, vFacing, TNULL, -1, 0, TFALSE, TNULL, 2 );
							pEnemyManager->CreateEnemy( AEnemyManager::ENEMYTYPE_LeechBot2, vRight, vFacing, TNULL, -1, 0, TFALSE, TNULL, 2 );

							ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 31 ), ASoundMap::EVENTID( 330 ), GetTranslation(), FMOD_EVENT_MODE( 0 ) );
							m_uiInkJetFlags |= IJFLAG_LeechesDropped;
						}
					}
				}
			}
			else
			{
				// Lost the blob
				SetState( 1 );
			}
		}
	}

	APropPlatform::Tick( a_fDeltaTime );

	// The high LOD collides its capsule with what's around: the blob gets a touch message, anything else blows it up
	if ( GetLOD() == FLAG_HighLOD && !( TUINT8( m_uiInkJetFlags >> 0 ) & 1 ) )
	{
		TCapsuleData oCapsule;
		oCapsule.SetHeight( 8.6f );
		oCapsule.SetRadius( 1.0f );

		TUINT32 uiLayer = ( m_fArmTime < 0.0f ) ? 0xC3 : 0x42;

		TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
		TASSERT( IsQueryHelperInSimulation( m_pCollisionQueryHelper ) );

		TPhysicsShapeContact aContacts[ 5 ];
		TINT                 iNumContacts = pPhysicsManager->CollideShape( &oCapsule, m_oMatrix, uiLayer, aContacts, 5, m_pCollisionQueryHelper );

		if ( iNumContacts > 1 )
		{
			// The first contact with something else than its own body
			for ( TINT i = 0; i < iNumContacts; i++ )
			{
				TPhysicsShapeContact& rContact = aContacts[ i ];

				if ( rContact.m_pBody == m_pPhysicsData->m_pBody )
					continue;

				TPhysicsObjectContainer* pBody       = rContact.m_pBody;
				TUINT32                  uiBodyLayer = ( pBody != TNULL ) ? ( pBody->m_uiCollisionFilterInfo & 0xFFF ) : 0;

				if ( uiBodyLayer & 2 )
				{
					if ( pBody != TNULL )
					{
						ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pBody->m_pUserData );

						if ( pOwnerInfo != TNULL && pOwnerInfo->m_eType == 2 )
						{
							InkJetContact oContact;
							oContact.vPosition = rContact.m_vPosition;
							oContact.vNormal   = rContact.m_vNormal;
							oContact.iUnk20    = 0;

							AEntityIOPacket oPacket;
							oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TOUCH;
							oPacket.m_iData    = TREINTERPRETCAST( TINT, &oContact );
							HandleEntityIO( TSTATICCAST( ABaseEntity, pOwnerInfo->m_pOwner ), oPacket );
						}
					}
				}
				else
				{
					Explode();
				}

				break;
			}
		}
	}
}

// $deBlob: FUNCTION 007ad0a0
void AInkJet::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
			if ( m_Flags & FLAG_Dead )
				Activate();

			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
		case AEntityIOPacket::MESSAGE_SLAM:
		{
			if ( m_Flags & FLAG_Dead )
				break;

			if ( !( a_pCaller->GetFlags() & FLAG_Player ) )
			{
				// Not a blob: anything but its own spawner blows it up (and is told so)
				if ( a_pCaller->GetClass() == &TGetClass( AJetbikeSpawner ) && CastJetbikeSpawner( a_pCaller )->GetInkJet() == this )
					return;

				Explode();

				TBOOL bWasOff = ( m_Flags & FLAG_Dead ) != 0;

				if ( !bWasOff )
					m_Flags |= FLAG_Dead;

				if ( !( a_pCaller->GetFlags() & FLAG_Dead ) )
				{
					AEntityIOPacket oPacket;
					oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RAMATTACK;
					oPacket.m_iData    = 0;
					a_pCaller->HandleEntityIO( this, oPacket );
				}

				if ( !bWasOff )
					m_Flags &= ~FLAG_Dead;

				return;
			}

			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

			if ( m_fHitCooldown <= 0.0f )
			{
				m_fHitCooldown = 0.1f;

				if ( !( pBlob->GetBlobFlags() & 0x8000 ) )
				{
					TINT iZJumpState = pBlob->GetZJumpState();

					if ( iZJumpState == ABlobZJumpComponent::STATE_ATTACKFLIGHT || iZJumpState == ABlobZJumpComponent::STATE_ATTACKSLAM )
					{
						// The blob's Z attack: it loses paint points, the jetbike explodes when it has none left
						TINT iPoints = TINT( pBlob->RemovePaintPoints( m_iLife, TFALSE ) );

						if ( iPoints > 0 )
						{
							m_iLife = TINT16( iPoints );

							if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TOUCH )
							{
								AAttackMessage oAttack( AAttackMessage::ATTACK_SlamFailedRecoil, GetTranslation(), 0, 3, TVector4::VEC_NEGY, TFALSE );

								AEntityIOPacket oPacket;
								oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RECOIL;
								oPacket.m_iData    = TREINTERPRETCAST( TINT, &oAttack );
								pBlob->HandleEntityIO( this, oPacket );
							}
							else
							{
								m_uiInkJetFlags |= IJFLAG_HitPending;
							}

							m_uiInkJetFlags &= ~IJFLAG_CanDropLeeches;
							APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
						}
						else
						{
							// Destroyed: an inked combo and the kill score
							AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
							TINT                    iPlayer    = TINT( APlayerManager::GetSingleton()->GetPlayerIndex( pBlob ) );

							if ( rLevelData.m_aPlayerData[ iPlayer ].m_iComboState == 2 )
								rLevelData.IncrementComboCount( iPlayer );
							else
								rLevelData.StartCombo( iPlayer, 2 );

							TVector4 vPosition = GetTranslation();
							vPosition.y        = GetTranslation().y - m_vBBox.y * 2.0f;
							rLevelData.AddKillScore( iPlayer, vPosition, TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x98 / 4 ] );

							Explode();
						}

						return;
					}

					if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TOUCH )
					{
						// Rammed by the jetbike
						AAttackMessage oAttack( AAttackMessage::ATTACKTYPE( 11 ), GetTranslation(), 1, 3, m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), TFALSE );

						AEntityIOPacket oPacket;
						oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RAMATTACK;
						oPacket.m_iData    = TREINTERPRETCAST( TINT, &oAttack );
						pBlob->HandleEntityIO( this, oPacket );

						m_uiInkJetFlags &= ~IJFLAG_CanDropLeeches;

						if ( 4 != GetState() )
						{
							SetState( 4 );
							APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
							return;
						}
					}
				}

				break;
			}

			// The Z attack's touch that follows a hit
			if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TOUCH && ( m_uiInkJetFlags & IJFLAG_HitPending ) )
			{
				m_fHitCooldown = 0.1f;
				m_uiInkJetFlags &= ~IJFLAG_HitPending;

				AAttackMessage oAttack( AAttackMessage::ATTACK_SlamFailedRecoil, GetTranslation(), 0, 3, TVector4::VEC_NEGY, TFALSE );

				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RECOIL;
				oPacket.m_iData    = TREINTERPRETCAST( TINT, &oAttack );
				pBlob->HandleEntityIO( this, oPacket );
			}

			return;
		}
		case AEntityIOPacket::MESSAGE_PROPATTACK:
		case AEntityIOPacket::MESSAGE_RAMATTACK:
		{
			if ( !( m_Flags & FLAG_Dead ) )
				Explode();

			TBOOL bWasOff = ( m_Flags & FLAG_Dead ) != 0;

			if ( !bWasOff )
				m_Flags |= FLAG_Dead;

			if ( !( a_pCaller->GetFlags() & FLAG_Dead ) )
				a_pCaller->HandleEntityIO( this, a_rPacket );

			if ( !bWasOff )
				m_Flags &= ~FLAG_Dead;

			break;
		}
		default:
			break;
	}

	APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007ad430
void AInkJet::Explode()
{
	TParticleEffectInstance* pExplosion = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pExplosionEffect );

	if ( pExplosion != TNULL )
	{
		TVector4 vOffset;
		vOffset.Multiply( TVector4::VEC_POSZ, 2.5f );

		TVector4 vPosition;
		TMatrix44::RotateVector( vPosition, m_pPhysicsData->m_oMatrix, vOffset );
		vPosition.x = GetTranslation().x + vPosition.x;
		vPosition.y = GetTranslation().y + vPosition.y;
		vPosition.z = GetTranslation().z + vPosition.z;

		TMatrix44 oMatrix = TMatrix44::IDENTITY;
		oMatrix.SetBasisVector4( BASISVECTOR_TRANSLATION, vPosition );
		pExplosion->GetTransform() = oMatrix;
	}

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 34 ), GetTranslation(), FMOD_EVENT_MODE( 0 ) );
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 26 ), ASoundMap::EVENTID( 298 ), GetTranslation(), FMOD_EVENT_MODE( 0 ) );
	ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 9, GetTranslation(), 1.0f, -1 );

	// The blobs forget it as a Z-jump target
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	TINT            iNumPlayers    = pPlayerManager->GetNumPlayers();

	for ( TINT i = 0; i < iNumPlayers; i++ )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( i ) ) );

		if ( pBlob != TNULL )
			pBlob->GetZJumpComponent()->OnEntityRemoved( this );
	}

	if ( ( m_uiInkJetFlags & IJFLAG_Spawned ) || ( TUINT8( m_uiPlatformFlags >> 3 ) & 1 ) )
	{
		if ( TUINT8( m_uiPlatformFlags >> 18 ) & 1 )
			RestartFromRoot();
		else
			RestartFromStart();
	}
	else
	{
		Deactivate();
	}
}

// $deBlob: FUNCTION 007ad620
void AInkJet::Activate()
{
	SetVisible( TTRUE );
	m_Flags &= ~FLAG_Dead;
	m_iLife = TINT16( GetMaxLife() );
	m_uiInkJetFlags &= ~IJFLAG_HitPending;
	m_fHitCooldown = -1.0f;
}

// $deBlob: FUNCTION 007ad660
void AInkJet::Deactivate()
{
	SetVisible( TFALSE );
	m_Flags |= FLAG_Dead;
	m_iLife = 0;
}

// $deBlob: FUNCTION 007ad680
void AInkJet::Reset( ABaseEntity* a_pEntity )
{
	APropPlatform::Reset( a_pEntity );

	m_iTargetPlayer  = -1;
	m_fTimeSinceSeen = 100.0f;
	SetState( 0 );

	m_uiInkJetFlags &= ~( IJFLAG_CanDropLeeches | IJFLAG_LeechesDropped );

	if ( m_uiInkJetFlags & IJFLAG_Spawned )
	{
		TPhysicsManager::GetSingleton()->RemovePhantomFromSimulation( m_pCollisionQueryHelper, TNULL );
		m_uiInkJetFlags |= IJFLAG_Waiting;
		Deactivate();
	}
	else
	{
		Activate();
	}
}

// $deBlob: FUNCTION 007ad730
void AInkJet::OnLODChanging( TINT a_iLOD )
{
	if ( a_iLOD & FLAG_LowLOD )
	{
		m_iTargetPlayer  = -1;
		m_fTimeSinceSeen = 100.0f;

		if ( 3 == GetState() || 4 == GetState() )
			SetState( 1 );
	}

	APropPlatform::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 007ad780
AWaypointData::Node* AInkJet::GetNextNode( TINT16 a_iNode, TBOOL a_bReverse, TBOOL a_bToggled, TINT* a_pOutNode )
{
	// The link of the direction whose node is the closest to the target blob (the first one without a target)
	AWaypointData::Node* pBest = TNULL;
	*a_pOutNode                = -1;

	AWaypointData::Node& rNode = m_pWaypointSet->GetNode( a_iNode );
	ABlobEntity*         pBlob = TNULL;

	// NOTE: retail starts the best distance with the node index's bits; it's always set by the first link
	TFLOAT fBestDistSq = 0.0f;

	for ( TINT i = 0; i < rNode.uiLinkCount; i++ )
	{
		TBOOL bForwardLink = rNode.GetLink( i ).uiFlags & AWaypointData::Link::FLAG_Toggled;

		if ( bForwardLink ? a_bReverse : !a_bReverse )
			continue;

		if ( pBest == TNULL )
		{
			*a_pOutNode = TUINT16( rNode.GetLink( i ).iNode );
			pBest       = rNode.GetLinkedNode( i, m_pWaypointSet );

			if ( m_iTargetPlayer == -1 )
				return pBest;

			pBlob       = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( m_iTargetPlayer ) ) );
			fBestDistSq = TVector4::DistanceSq( pBlob->GetTranslation(), pBest->vPosition );
		}
		else
		{
			TUINT16              uiNode  = TUINT16( rNode.GetLink( i ).iNode );
			AWaypointData::Node* pLinked = rNode.GetLinkedNode( i, m_pWaypointSet );
			TFLOAT               fDistSq = TVector4::DistanceSq( pBlob->GetTranslation(), pLinked->vPosition );

			if ( fDistSq < fBestDistSq )
			{
				pBest       = pLinked;
				*a_pOutNode = uiNode;
				fBestDistSq = fDistSq;
			}
		}
	}

	return pBest;
}

// $deBlob: FUNCTION 007ad990
void AInkJet::AdjustNodePosition( TVector4* a_pOut, const TVector4* a_pFrom, const TVector4* a_pNodePos, TBOOL a_bNoRandomness )
{
	// Steers the point sideways towards the target blob, by "Randomness" at most
	if ( !a_bNoRandomness && m_iTargetPlayer >= 0 )
	{
		TVector4 vIncomingDir( a_pOut->x - a_pNodePos->x, a_pOut->y - a_pNodePos->y, a_pOut->z - a_pNodePos->z, a_pOut->w );
		TFLOAT   fLength = vIncomingDir.NormaliseXZ();

		TASSERT( TMath::Abs( vIncomingDir.DotProduct( TVector4::VEC_NEGY ) ) < 0.99f, "Can't have 2 positions on a spline exactly above each other (same xz position) when getting random positions" );

		ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( m_iTargetPlayer ) );

		TVector4 vSide;
		vSide.CrossProduct( vIncomingDir, TVector4::VEC_NEGY );

		const TVector4& rBlob = pBlob->GetTranslation();
		TFLOAT          fT;
		TFLOAT          fAhead = ( rBlob.z - a_pFrom->z ) * vIncomingDir.z + ( rBlob.x - a_pFrom->x ) * vIncomingDir.x;

		TBOOL bIntersects = TFALSE;

		if ( fAhead > 0.0f )
		{
			TFLOAT afLine1Start[ 2 ] = { a_pOut->x, a_pOut->z };
			TFLOAT afLine1End[ 2 ]   = { a_pOut->x + vSide.x, a_pOut->z + vSide.z };
			TFLOAT afLine2Start[ 2 ] = { a_pFrom->x, a_pFrom->z };
			TFLOAT afLine2End[ 2 ]   = { rBlob.x, rBlob.z };

			bIntersects = TMath::LineVsLine2D( afLine1Start, afLine1End, afLine2Start, afLine2End, fT );
		}

		if ( !bIntersects )
		{
			TFLOAT fDiv = -( fAhead / fLength );

			if ( fDiv < 1.0f )
				fDiv = 1.0f;

			fT = ( ( a_pFrom->z - a_pOut->z ) * vSide.z + ( a_pFrom->x - a_pOut->x ) * vSide.x ) / fDiv;
		}

		TMath::Clip( fT, -m_fRandomness, m_fRandomness );

		TVector4 vOffset;
		vOffset.Multiply( vSide, fT );
		a_pOut->x = a_pOut->x + vOffset.x;
		a_pOut->y = a_pOut->y + vOffset.y;
		a_pOut->z = a_pOut->z + vOffset.z;
	}

	a_pOut->y = a_pOut->y - 2.0f;
}

// $deBlob: FUNCTION 007adc10
const TCHAR* AInkJet::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}

// $deBlob: FUNCTION 007adc20
void AInkJet::StartFromCurrentNode()
{
	m_iTargetPlayer  = -1;
	m_fTimeSinceSeen = 100.0f;
	SetState( 0 );

	if ( m_uiInkJetFlags & IJFLAG_Spawned )
	{
		TPhysicsManager::GetSingleton()->RemovePhantomFromSimulation( m_pCollisionQueryHelper, TNULL );
		m_uiInkJetFlags |= IJFLAG_Waiting;
		Deactivate();
	}
	else if ( TUINT8( m_uiPlatformFlags >> 3 ) & 1 )
	{
		Deactivate();
	}

	APropPlatform::StartFromCurrentNode();
	m_uiPlatformFlags |= PLATFORMFLAG_Stopped;
	StopMoving();
}

// $deBlob: FUNCTION 007adca0
void AInkJet::OnStartMoving()
{
	SetState( 1 );

	// Launched by its spawner: the query helper goes back into the simulation, testing layer 0x42 for a while
	if ( ( m_uiInkJetFlags & IJFLAG_Spawned ) && ( m_uiInkJetFlags & IJFLAG_Waiting ) )
	{
		m_fArmTime = 1.3f;
		m_uiInkJetFlags &= ~IJFLAG_Waiting;

		TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

		TASSERT( !IsQueryHelperInSimulation( m_pCollisionQueryHelper ) );
		pPhysicsManager->SetPhantomCollisionFilterInfo( m_pCollisionQueryHelper, 0x42000 );
		pPhysicsManager->AddPhantomToSimulation( m_pCollisionQueryHelper, GetTranslation(), TNULL );
	}
}

// $deBlob: FUNCTION 007ade10
void AInkJet::StopMoving()
{
	if ( 3 == GetState() || 4 == GetState() )
	{
		m_uiPlatformFlags &= ~PLATFORMFLAG_Stopped;
	}
	else
	{
		APropPlatform::StopMoving();
		SetState( 0 );
	}
}

// $deBlob: FUNCTION 007adda0
void AInkJet::OnEnterState( TINT a_iState )
{
	TFLOAT fSpeed;

	switch ( GetState() )
	{
		case 1:
			fSpeed = GetInkJetData()->m_fCruiseSpeed;
			break;
		case 2:
			m_fSpeed        = 0.0f;
			m_uiInkJetFlags = ( m_uiInkJetFlags & ~IJFLAG_LeechesDropped ) | IJFLAG_CanDropLeeches;
			return;
		case 3:
			m_fTimeSinceSeen = 0.0f;
			fSpeed           = GetInkJetData()->m_fChaseSpeed;
			break;
		default:
			return;
	}

	m_fSpeed = fSpeed;

	if ( m_pWaypointSet != TNULL )
		m_fSpeed = fSpeed / m_fPathLength;
}

// $deBlob: FUNCTION 007adfa0
TBOOL AInkJet::TicksAtLowLOD()
{
	return TTRUE;
}

// $deBlob: FUNCTION 007adfc0
TINT AInkJet::GetMaxLife()
{
	return 0x14;
}

// $deBlob: FUNCTION 007ae000
TBOOL AInkJet::IsTargetable()
{
	return TTRUE;
}

// $deBlob: FUNCTION 007ae010
TINT AInkJet::GetTargettingPriority()
{
	return 0;
}

// $deBlob: FUNCTION 007ae020
void AInkJet::OnOutOfWorld()
{
	if ( TUINT8( m_uiPlatformFlags >> 18 ) & 1 )
		RestartFromRoot();
	else
		RestartFromStart();
}
