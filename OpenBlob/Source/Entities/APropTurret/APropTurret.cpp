#include "pch.h"
#include "APropTurret.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AAttackMessage.h"
#include "Entities/AEntityData.h"
#include "Entities/Projectiles/ADataProjectile.h"
#include "Entities/Projectiles/AProjectile.h"
#include "Collision/ACollisionInstance2.h"
#include <Physics/Havok/TPhysicsManager.h>
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include "Animation/AModelFactory.h"
#include "Sound/APositionalSoundManager.h"
#include "Sound/ASoundInterface.h"
#include "ARandom.h"
#include "Entities/APickupManager.h"
#include "Cameras/ACameraManager.h"
#include "Challenges/AChallengeManager.h"
#include "GameController/AGameStatus.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTurret ) == 0x1A4 );

// $deBlob: FUNCTION 00781250
// $deBlob: FUNCTION 00781260
// $deBlob: FUNCTION 007812d0
// $deBlob: FUNCTION 00781330
// $deBlob: FUNCTION 00781340
// $deBlob: FUNCTION 00540b60
// $deBlob: FUNCTION 00859d50
TDEFINE_CLASS( APropTurret );

// $deBlob: FUNCTION 00781350
APropTurret::APropTurret()
    : m_Helper( this )
{
	m_uiTurretFlags                           = 0;
	m_iIdleAnim                               = -1;
	m_iFireAnim                               = -1;
	m_iFireBone                               = -1;
	m_iDamage25Bone                           = -1;
	m_iDamage50Bone                           = -1;
	m_iDamage75Bone                           = -1;
	m_iHitAnim                                = -1;
	m_iSpawnAnim                              = -1;
	m_iDiasSpawnAnim                          = -1;
	m_iCurrentAnim                            = -1;
	m_pHAnim                                  = TNULL;
	m_fHAnimLength                            = 0.0f;
	m_pVAnim                                  = TNULL;
	m_fVAnimLength                            = 0.0f;
	m_iLife                                   = 50;
	reinterpret_cast<TUINT8*>( this )[ 0x63 ] = 0x21; // NOTE: the padding byte after m_iLife, written by the retail ctor
	m_Flags                                   = ( m_Flags & ~FLAG_Dead ) | 0x40;
	m_pFXDamage25                             = TNULL;
	m_pFXDamage50                             = TNULL;
	m_pFXDamage75                             = TNULL;
	m_pDiasModel                              = TNULL;
}

// $deBlob: FUNCTION 00782c00
// $deBlob: FUNCTION 00781490
// $deBlob: FUNCTION 00782bf3
APropTurret::~APropTurret()
{
	if ( m_pFXDamage25 != TNULL )
	{
		m_pFXDamage25->StopInstant();
		m_pFXDamage25 = TNULL;
	}

	if ( m_pFXDamage50 != TNULL )
	{
		m_pFXDamage50->StopInstant();
		m_pFXDamage50 = TNULL;
	}

	if ( m_pFXDamage75 != TNULL )
	{
		m_pFXDamage75->StopInstant();
		m_pFXDamage75 = TNULL;
	}

	AModelFactory::GetSingleton()->DestroyInstance( m_pDiasModel );

	// NOTE: m_Helper's destructor (StopEffects) runs here
}

// $deBlob: FUNCTION 00781560
// $deBlob: FUNCTION 00782c63
void APropTurret::PostCreate()
{
	AProp::PostCreate();
	AModelInstance2*      pModel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	AToshiAnimInterface2* pAnim  = pModel->GetAnimInterface();

	m_iFireBone = TINT16( pModel->FindBone( "turret_gun_tip" ) );
	TASSERT( m_iFireBone != -1, "-1 != m_iFireBone" );

	m_iDamage25Bone = TINT16( pModel->FindBone( "Tank_LB_Damage25" ) );
	TASSERT( m_iDamage25Bone != -1, "-1 != m_iDamage25Bone" );

	m_iDamage50Bone = TINT16( pModel->FindBone( "Tank_LB_Damage50" ) );
	TASSERT( m_iDamage50Bone != -1, "-1 != m_iDamage50Bone" );

	m_iDamage75Bone = TINT16( pModel->FindBone( "Tank_LB_Damage75" ) );
	TASSERT( m_iDamage75Bone != -1, "-1 != m_iDamage75Bone" );

	TINT iInkyBone = pModel->FindBone( "turret_inky" );
	TASSERT( iInkyBone != -1, "-1 != iInkyBone" );

	// the helper's origin is the position of the turret bone
	Toshi::TMatrix44 mBone;
	pModel->GetBoneTransform( iInkyBone, mBone );
	m_Helper.m_vOrigin = mBone.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION );

	m_iIdleAnim = TINT16( pAnim->GetAnimID( "idle_loop" ) );
	TASSERT( m_iIdleAnim >= 0, "m_IdleAnim >= 0" );

	m_iFireAnim = TINT16( pAnim->GetAnimID( "Fire" ) );
	TASSERT( m_iFireAnim >= 0, "m_FireAnim >= 0" );

	m_iSpawnAnim = TINT16( pAnim->GetAnimID( "Spawn" ) );
	TASSERT( m_iSpawnAnim >= 0, "m_iSpawnAnimID >= 0" );

	m_iHitAnim = TINT16( pAnim->GetAnimID( "Hit" ) );
	TASSERT( m_iHitAnim >= 0, "m_iHitAnimID >= 0" );

	if ( ( m_uiTurretFlags & TURRETFLAG_SpawnsIn ) != 0 )
	{
		m_iCurrentAnim = m_iSpawnAnim;
		pAnim->AddAnim( m_iCurrentAnim, 0.0f, 1.0f, 0.0f );
		SetPhysicsEnabled( TFALSE ); // vtable +0x90
		m_Flags |= FLAG_DontTick;
	}
	else
	{
		m_iCurrentAnim = m_iIdleAnim;
		pAnim->AddAnim( m_iCurrentAnim, 0.0f, 1.0f, 1.0f );
	}

	// the horizontal (H_Rotate) and vertical (P_Rotate) overlays, timed by the helper's yaw and its turning amount
	TINT iHAnim = pAnim->GetAnimID( "H_Rotate" );
	TASSERT( iHAnim >= 0, "iHorizontalOverlayAnim >= 0" );

	m_pHAnim = pAnim->AddAnim( TINT16( iHAnim ), 0.0f, 1.0f, 0.0f );
	TASSERT( m_pHAnim != TNULL, "TNULL != m_pHAnim" );

	m_fHAnimLength = m_pHAnim->GetSequencePtr()->GetDuration();
	m_pHAnim->SetTime( m_Helper.m_fAngleFrac * m_fHAnimLength );

	TINT iVAnim = pAnim->GetAnimID( "P_Rotate" );
	TASSERT( iVAnim >= 0, "iVerticalOverlayAnim >= 0" );

	m_pVAnim = pAnim->AddAnim( TINT16( iVAnim ), 0.0f, 1.0f, 0.0f );
	TASSERT( m_pVAnim != TNULL, "TNULL != m_pVAnim" );

	m_fVAnimLength = m_pVAnim->GetSequencePtr()->GetDuration();
	m_pVAnim->SetTime( m_Helper.m_fPitchFrac * m_fVAnimLength );

	// the dias model: its ambient scale is the entity's, it's placed at the entity and its spawn sequence is looked up
	m_pDiasModel = AModelFactory::GetSingleton()->CreateInstance( "turret_dias_animated.trb", TFALSE, TFALSE );
	m_pDiasModel->SetAmbientScale( m_fAmbientScale );

	m_iDiasSpawnAnim = TINT16( m_pDiasModel->GetModelInstance()->GetSkeletonInstance()->GetSkeleton()->GetSequenceID( "Spawn", -1 ) );
	TASSERT( m_iSpawnAnim >= 0, "m_iSpawnAnimID >= 0" );

	m_pDiasModel->SetTransform( m_oMatrix );

	// the helper's facing is the entity's forward vector
	m_Helper.SetFacing( m_oMatrix.AsBasisVector4( Toshi::BASISVECTOR_FORWARD ) );

	// NOTE: AProjectileTypes value 0 is the turret bullet (ATurretHelper::GetProjectileDataName 0), the pool size is 6
	AProjectileManager::GetSingleton()->InitPool( AProjectileTypes::PROJECTILETYPES( 0 ), 6 );

	m_pFXExplosion    = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_Explosion" );
	m_pFXInkySplat    = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_InkySplat" );
	m_pFXDamageSparks = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_VehicleDamageSparks" );
	m_pFXDamageSmoke  = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_VehicleDamageSmoke" );
	m_pFXDamage25     = TNULL;
	m_pFXDamage50     = TNULL;
	m_pFXDamage75     = TNULL;
}

// $deBlob: FUNCTION 00781c00
// $deBlob: FUNCTION 00782c6e
void APropTurret::ProcessParams( const AEntityParams& a_rParams )
{
	AProp::ProcessParams( a_rParams );

	TFLOAT fSearchAngle = 0.0f;
	a_rParams.GetFloat( fSearchAngle, "VisAngle" );
	fSearchAngle = fSearchAngle / 360.0f;
	TASSERT( fSearchAngle >= 0.0f && fSearchAngle < 0.5f );

	TBOOL bSpawnsIn = TFALSE;
	a_rParams.GetBool( bSpawnsIn, "Spawns In" );

	if ( bSpawnsIn )
	{
		m_uiTurretFlags &= ~TURRETFLAG_NoSpawnsIn;
		m_uiTurretFlags |= TURRETFLAG_SpawnsIn;
	}
	else
	{
		m_uiTurretFlags |= TURRETFLAG_NoSpawnsIn;
	}

	TBOOL bTargetable = TTRUE;
	a_rParams.GetBool( bTargetable, "Targetable" );

	if ( bTargetable )
		m_uiTurretFlags |= TURRETFLAG_Targetable;

	// The turret's type data (the TRB of the entity type) and the data of its projectile
	TurretData* pTurretData = TSTATICCAST( TurretData, m_pTypeData->m_pTRBData );

	AEntityParam::Collision** ppCollision = TNULL;

	if ( a_rParams.GetCollision( ppCollision ) )
	{
		CreatePhysics( pTurretData, *ppCollision, TNULL, TFALSE );
	}
	else
	{
		AModelInstance2* pModelInstance  = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
		TModelInstance*  pTModelInstance = pModelInstance->GetModelInstance();

		ACollisionInstance2* pCollision = new ACollisionInstance2();
		pCollision->SetModelInstance( pTModelInstance, TNULL );

		ACollisionInstance2::OwnerInfo oOwner;
		oOwner.m_eType  = 2;
		oOwner.m_pOwner = this;

		TINT aShapeTypes[ 3 ] = { 4, 4, 4 };
		pCollision->Create( pTModelInstance, &oOwner, TTRUE, 3, aShapeTypes );
		pModelInstance->SetCollisionInstance( pCollision );

		for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
		{
			TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

			TASSERT( ( TPhysicsManager::GetCollisionLayer( pBody ) & 8 ) == 0 );

			TUINT32 uiLayer = TPhysicsManager::GetCollisionLayer( pBody );
			TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, uiLayer & ~0x10 );

			if ( pBody != TNULL )
			{
				pBody->m_uiSurface    = 0x1000000;
				pBody->m_fFriction    = 0.0f;
				pBody->m_fRestitution = 0.2f;
			}
		}
	}

	// The projectile type of the turret (type 0 of the projectile manager)
	ADataProjectile* pProjData = TSTATICCAST( ADataProjectile, AEntityData::GetSingleton()->GetProjectileData( ATurretHelper::GetProjectileDataName( 0 ) ) );

	m_Helper.Initialise( fSearchAngle, pTurretData->m_fFireDelay, pProjData->m_fGravity, pProjData->m_fSpeed, pTurretData->m_fRotateSpeed, pTurretData->m_fVisualRangeA, pTurretData->m_fVisualRangeB, pTurretData->m_fMaxVisualAngleA, pTurretData->m_fMaxVisualAngleB, pTurretData->m_iMaxBurst, pTurretData->m_fBurstDelay );

	m_uiPropFlags &= ~PROPFLAG_Paintable;
}

// $deBlob: FUNCTION 00781f20
void APropTurret::Tick( TFLOAT a_fDeltaTime )
{
	AProp::Tick( a_fDeltaTime );

	// The dias model's spawn animation: once it has reached its end, the animations are removed
	TSkeletonInstance* pSkeleton = m_pDiasModel->GetModelInstance()->GetSkeletonInstance();
	TAnimation*        pAnim     = pSkeleton->IsAnimating( m_iDiasSpawnAnim );

	if ( pAnim != TNULL )
	{
		pSkeleton->UpdateTime( a_fDeltaTime );

		TFLOAT fTotalTime = pAnim->GetTotalTime();
		TINT16 iSeqID     = pAnim->GetSequence();

		// NOTE: retail reads the skeleton from the animation (anim->m_pSkeletonInstance), which is this one
		if ( pSkeleton->GetSkeleton()->GetSequence( iSeqID )->m_fDuration <= fTotalTime )
		{
			pSkeleton->RemoveAllAnimations( 0.0f );
			pSkeleton->SetStateFromBasePose();
		}
	}

	if ( m_iIdleAnim != m_iCurrentAnim && TSTATICCAST( AEntityModelComponent, m_pModelComponent )->GetModelInstance()->GetAnimInterface()->IsAnimationFinished( m_iCurrentAnim ) )
	{
		PlayAnimation( m_iIdleAnim );
	}

	if ( ( m_uiTurretFlags & TURRETFLAG_NoSpawnsIn ) != 0 && m_iCurrentAnim != m_iSpawnAnim )
	{
		if ( m_Helper.Tick( a_fDeltaTime ) )
		{
			AProjectile* pProjectile = AProjectileManager::GetSingleton()->CreateProjectile( static_cast<AProjectileTypes::PROJECTILETYPES>( 0 ) );

			if ( pProjectile != TNULL )
			{
				TMatrix44 mBone;
				TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( m_iFireBone, mBone );

				pProjectile->Activate( this, mBone.AsBasisVector4( BASISVECTOR_TRANSLATION ), m_Helper.m_vAim, 0x2F );
				pProjectile->m_uiAttackType = 0xC;
				PlayAnimation( m_iFireAnim );
				m_Helper.OnFire( pProjectile );
			}
		}

		StopFiring();
	}

	if ( ( m_Flags & FLAG_Dead ) != 0 )
		return;

	GetModelComponent(); // discarded in retail (vtable +0x4C), no effect
	TFLOAT fRatio = 1.0f - (TFLOAT)m_iLife / (TFLOAT)GetMaxLife();

	// The damage effects: sparks above 25% and 50% damage, smoke above 75% (the bones are the damage bones)
	if ( fRatio <= 0.25f )
	{
		if ( m_pFXDamage25 != TNULL )
		{
			m_pFXDamage25->StopInstant();
			m_pFXDamage25 = TNULL;
		}
	}
	else
	{
		if ( m_pFXDamageSparks != TNULL && m_pFXDamage25 == TNULL )
			m_pFXDamage25 = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pFXDamageSparks );

		if ( m_pFXDamage25 != TNULL )
		{
			TMatrix44 mBone;
			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( m_iDamage25Bone, mBone );
			m_pFXDamage25->GetTransform() = mBone;
		}
	}

	if ( fRatio <= 0.5f )
	{
		if ( m_pFXDamage50 != TNULL )
		{
			m_pFXDamage50->StopInstant();
			m_pFXDamage50 = TNULL;
		}
	}
	else
	{
		if ( m_pFXDamageSparks != TNULL && m_pFXDamage50 == TNULL )
			m_pFXDamage50 = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pFXDamageSparks );

		if ( m_pFXDamage50 != TNULL )
		{
			TMatrix44 mBone;
			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( m_iDamage50Bone, mBone );
			m_pFXDamage50->GetTransform() = mBone;
		}
	}

	if ( fRatio <= 0.75f )
	{
		if ( m_pFXDamage75 != TNULL )
		{
			m_pFXDamage75->StopInstant();
			m_pFXDamage75 = TNULL;
		}
	}
	else
	{
		APositionalSoundManager::GetSingleton()->AddEntitySound( this, 0x1E, 0x13F );

		if ( m_pFXDamageSmoke != TNULL && m_pFXDamage75 == TNULL )
			m_pFXDamage75 = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pFXDamageSmoke );

		if ( m_pFXDamage75 != TNULL )
		{
			TMatrix44 mBone;
			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( m_iDamage75Bone, mBone );
			m_pFXDamage75->GetTransform() = mBone;
		}
	}
}

// $deBlob: FUNCTION 007825d0
void APropTurret::Reset( ABaseEntity* a_pEntity )
{
	AProp::Reset( a_pEntity );
	SetHash( TTRUE );

	m_Flags &= ~FLAG_DontTick;
	SetVisible( TTRUE );

	if ( m_pFXDamage25 != TNULL )
	{
		m_pFXDamage25->StopInstant();
		m_pFXDamage25 = TNULL;
	}

	if ( m_pFXDamage50 != TNULL )
	{
		m_pFXDamage50->StopInstant();
		m_pFXDamage50 = TNULL;
	}

	if ( m_pFXDamage75 != TNULL )
	{
		m_pFXDamage75->StopInstant();
		m_pFXDamage75 = TNULL;
	}

	APositionalSoundManager::GetSingleton()->RemoveSound( this, 30, 319 );
	m_Helper.StopEffects();
	m_Helper.Reset();
	StopFiring();

	if ( ( m_uiTurretFlags & TURRETFLAG_SpawnsIn ) != 0 )
	{
		PlayAnimation( m_iSpawnAnim );

		// the spawn animation of the model is stopped at its first frame (speed 0)
		TSkeletonInstance* pSkeleton = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();
		TAnimation*        pAnim     = pSkeleton->IsAnimating( m_iSpawnAnim );
		TASSERT( pAnim != TNULL, "pAnim" );
		pAnim->SetSpeed( 0.0f );

		m_uiTurretFlags &= ~TURRETFLAG_NoSpawnsIn;
		SetPhysicsEnabled( TFALSE ); // vtable +0x90
		m_Flags |= FLAG_DontTick;
	}
	else
	{
		PlayAnimation( m_iIdleAnim );
		m_uiTurretFlags |= TURRETFLAG_NoSpawnsIn;
		SetPhysicsEnabled( TTRUE ); // vtable +0x90
	}

	TSkeletonInstance* pDiasSkeleton = m_pDiasModel->GetModelInstance()->GetSkeletonInstance();
	pDiasSkeleton->RemoveAllAnimations( 0.0f );
	pDiasSkeleton->SetStateFromBasePose();

	m_iLife = TINT16( GetMaxLife() );
	m_Flags &= ~FLAG_Dead;
}

// $deBlob: FUNCTION 00782300
void APropTurret::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TRIGGER )
	{
		// The turret spawns in (its spawn animation plays once; the "Spawns In" flag is not cleared again)
		if ( ( m_uiTurretFlags & TURRETFLAG_SpawnsIn ) != 0 && ( m_uiTurretFlags & TURRETFLAG_NoSpawnsIn ) == 0 )
		{
			m_uiTurretFlags |= TURRETFLAG_NoSpawnsIn;
			PlayAnimation( m_iSpawnAnim );
			m_pDiasModel->GetModelInstance()->GetSkeletonInstance()->AddAnimation( m_iDiasSpawnAnim, 1.0f, 0.0f );
			SetPhysicsEnabled( TTRUE );
			m_Flags &= ~FLAG_DontTick;
		}
	}
	else if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_SLAM )
	{
		// A blob (a_pCaller) touched the turret: it takes paint from the blob, and the blob gets an attack back
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

		if ( ( m_Flags & FLAG_Dead ) == 0 && ( pBlob->GetBlobFlags() & 0x8000 ) == 0 ) // BFLAG 0x8000 (not named yet)
		{
			TINT iPoints = static_cast<TINT>( pBlob->RemovePaintPoints( m_iLife, TFALSE ) );

			ASoundInterface::PlayEventAtPosition( static_cast<ASoundMap::GROUPID>( 0x1E ), static_cast<ASoundMap::EVENTID>( 0x13B ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), FMOD_EVENT_DEFAULT );

			if ( iPoints < 1 )
			{
				Explode();
				AProp::HandleEntityIO( a_pCaller, a_rPacket );
				return;
			}

			m_iLife = static_cast<TINT16>( iPoints );
			PlayAnimation( m_iHitAnim );

			AAttackMessage oAttack( AAttackMessage::ATTACK_SlamFailedRecoil, m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 0, 3, Toshi::TVector4::VEC_NEGY, TFALSE );

			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RECOIL; // NOTE: 0x0A is not named in AEntityIOPacket yet
			oPacket.m_iData    = TREINTERPRETCAST( TINT, &oAttack );

			pBlob->HandleEntityIO( this, oPacket );
			AProp::HandleEntityIO( a_pCaller, a_rPacket );
			return;
		}
	}
	else if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_PROJECTILEHIT )
	{
		// m_iData points at a TVector4 (+0x18) of the sender: the turret aims a ray from it, 2 units along +Y.
		// NOTE: the sender's type is not named yet; the ray is a TPhysicsRay on the layer 0x81
		Toshi::TVector4* pFrom = TREINTERPRETCAST( Toshi::TVector4*, TREINTERPRETCAST( TUINT8*, a_rPacket.m_iData ) + 0x18 );

		Toshi::TVector4 vOffset;
		vOffset.Multiply( Toshi::TVector4::VEC_POSY, 2.0f );

		TPhysicsRay oRay;
		oRay.m_vFrom            = *pFrom;
		oRay.m_vTo              = oRay.m_vFrom + vOffset;
		oRay.m_uiCollisionLayer = 0x81;

		TPhysicsShapeContact oContact;

		if ( TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) )
		{
			TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pFXInkySplat );

			if ( pEffect != TNULL )
			{
				// The splat sits on the hit point, a little below it, turned by a random angle around Y
				TMatrix44 mTransform = TMatrix44::IDENTITY;
				mTransform.m_f41     = oContact.m_vPosition.x;
				mTransform.m_f42     = oContact.m_vPosition.y - 0.02f;
				mTransform.m_f43     = oContact.m_vPosition.z;
				mTransform.m_f44     = oContact.m_vPosition.w;

				mTransform.RotateY( ARandom::GetSingleton()->GetRandom().GetFloat() * 6.2831855f );
				pEffect->GetTransform() = mTransform;
			}
		}

		// NOTE: retail also calls an empty stub here
		AProp::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00782b20
void APropTurret::PlayAnimation( TINT16 a_iAnim )
{
	AToshiAnimInterface2* pI = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();
	pI->StopAnim( m_iCurrentAnim, 0.0f );
	m_iCurrentAnim = a_iAnim;
	pI->AddAnim( a_iAnim, 0.0f, 1.0f, 1.0f );

	TASSERT( 1 == pI->GetSkeletonInstance()->GetBaseAnimationCount() );
	TASSERT( 2 == pI->GetSkeletonInstance()->GetOverlayAnimationCount() );
}

// $deBlob: FUNCTION 00782880
void APropTurret::StopFiring()
{
	// The overlays follow the aim, the shot starts at the gun's tip
	m_pVAnim->SetTime( m_fVAnimLength * m_Helper.m_fPitchFrac );
	m_pHAnim->SetTime( m_fHAnimLength * m_Helper.m_fAngleFrac );

	TMatrix44 mBone;
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( m_iFireBone, mBone );
	m_Helper.m_vLookAt = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $deBlob: FUNCTION 00782770
void APropTurret::SetVisible( TBOOL a_bVisible )
{
	AVisibleEntity::SetVisible( a_bVisible );

	if ( m_pDiasModel != TNULL )
		m_pDiasModel->SetFlag( AModelInstance2::FLAG_Visible, a_bVisible && ( TUINT8( m_Flags >> 11 ) & 1 ) );
}

// $deBlob: FUNCTION 007827b0
void APropTurret::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( m_pDiasModel != TNULL )
		m_pDiasModel->SetFlag( AModelInstance2::FLAG_Visible, ( a_iLOD & FLAG_HighLOD ) ? ( ( m_Flags & FLAG_Visible ) != 0 ) : TFALSE );

	// The low LOD: no target, no damage effects
	if ( a_iLOD & FLAG_LowLOD )
	{
		m_Helper.ReleaseTarget();

		if ( m_pFXDamage25 != TNULL )
		{
			m_pFXDamage25->StopInstant();
			m_pFXDamage25 = TNULL;
		}

		if ( m_pFXDamage50 != TNULL )
		{
			m_pFXDamage50->StopInstant();
			m_pFXDamage50 = TNULL;
		}

		if ( m_pFXDamage75 != TNULL )
		{
			m_pFXDamage75->StopInstant();
			m_pFXDamage75 = TNULL;
		}
	}
}

// $deBlob: FUNCTION 00782850
TBOOL APropTurret::IsTargetable()
{
	// A targetable turret that spawned in (not while its spawn animation plays)
	return ( m_uiTurretFlags & TURRETFLAG_NoSpawnsIn ) && ( m_uiTurretFlags & TURRETFLAG_Targetable ) && m_iCurrentAnim != m_iSpawnAnim;
}

// $deBlob: FUNCTION 00782c50
TINT APropTurret::GetMaxLife()
{
	return 0x32;
}

// $deBlob: FUNCTION 00782c60
TINT APropTurret::GetTargettingPriority()
{
	return 0;
}

// $deBlob: FUNCTION 00782900
void APropTurret::Explode()
{
	// Destroyed: the explosion, the turret disappears and counts for the combat challenge
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 0x21 ), GetBasePosition(), FMOD_EVENT_MODE( 0 ) );

	SetVisible( TFALSE );
	SetHash( TFALSE );
	SetPhysicsEnabled( TFALSE );
	m_Flags |= FLAG_DontTick;

	TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pFXExplosion );

	if ( pEffect != TNULL )
	{
		TMatrix44 mTransform = TMatrix44::IDENTITY;
		mTransform.SetTranslation( GetBasePosition() );
		pEffect->GetTransform() = mTransform;
	}

	m_Flags |= FLAG_Dead;
	m_iLife = 0;
	m_Helper.SetState( 3 );

	ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 10, m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, -1 );

	TASSERT( !IsPooled() );
	AChallengeManager::GetSingleton()->OnCombatEvent( TREINTERPRETCAST( void*, TUINTPTR( GetID() ) ) );

	if ( AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 )
		APickupManager::SpawnPickupNearPlayer( 1, 0 );

	if ( m_pFXDamage25 != TNULL )
	{
		m_pFXDamage25->StopInstant();
		m_pFXDamage25 = TNULL;
	}

	if ( m_pFXDamage50 != TNULL )
	{
		m_pFXDamage50->StopInstant();
		m_pFXDamage50 = TNULL;
	}

	if ( m_pFXDamage75 != TNULL )
	{
		m_pFXDamage75->StopInstant();
		m_pFXDamage75 = TNULL;
	}

	APositionalSoundManager::GetSingleton()->RemoveSound( this, 30, 319 );
}
