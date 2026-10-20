#include "pch.h"
#include "AProjectile.h"
#include "ADataProjectile.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Entities/AAttackMessage.h"
#include "Entities/ABaseEntity.h"
#include "FXComposition/AFXCompositionInstance.h"
#include "Sound/APositionalSoundManager.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundMap.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Cameras/ACameraManager.h"
#include "Collision/ACollisionManager2.h"
#include "Collision/ACollisionObject.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/AEntityHash.h"
#include "Entities/AEntityIOPacket.h"
#include "Terrain2/ATerrain2.h"
#include "Terrain2/ATerrain2Interface.h"
#include <Math/TAABox.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007d4880
void AProjectile::Kill( TBOOL a_bPlayEffects )
{
	TASSERT( IsActive() );

	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1E, 0x143 );

	if ( a_bPlayEffects )
	{
		if ( m_pFXUnk != TNULL )
		{
			Toshi::TMatrix44 mTransform = Toshi::TMatrix44::IDENTITY;
			mTransform.m_f41            = m_vPosition.x;
			mTransform.m_f42            = m_vPosition.y;
			mTransform.m_f43            = m_vPosition.z;
			mTransform.m_f44            = m_vPosition.w;
			m_pFXUnk->TriggerEffect( mTransform );
		}

		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x1E ), ASoundMap::EVENTID( 0x142 ), m_vPosition, 0 );
	}

	if ( m_pFXTrail != TNULL )
		m_pFXTrail->StopInterrupt();

	if ( m_pParticleEffectTrail != TNULL )
	{
		m_pParticleEffectTrail->SetFinishedCallback( TNULL, TNULL );
		m_pParticleEffectTrail->Stop();
		m_pParticleEffectTrail = TNULL;
	}

	if ( m_pModel != TNULL )
		m_pModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

	m_uiFlags &= ~( 2 | 4 );
	m_fLifeTime = -1.0f;
	m_pOverride = TNULL;
}

// $deBlob: FUNCTION 007d4a60
void AProjectile::Activate( ABaseEntity* a_pOwner, const Toshi::TVector4& a_rvPosition, const Toshi::TVector4& a_rvDirection, TUINT a_uiFlags )
{
	m_pOwner    = a_pOwner;
	m_fLifeTime = 0.0f;
	m_vPosition = a_rvPosition;

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x1E ), ASoundMap::EVENTID( 0x13D ), a_rvPosition, 0 );
	APositionalSoundManager::GetSingleton()->AddTargetSound( this, 0x1E, 0x143 );

	Toshi::TVector4 vDirection = a_rvDirection;

	if ( !vDirection.IsNormalised() )
	{
		// Not a unit vector: it is the velocity itself
		m_vVelocity = vDirection;
		vDirection.Normalise();
	}
	else
	{
		// A unit direction: the arc (flag 0x20) bends it, then the speed scales it
		if ( ( a_uiFlags & 0x20 ) != 0 && m_pData->m_fArcAngle >= 0.0f )
		{
			Toshi::TVector4 vAxis;
			vAxis.CrossProduct( vDirection, Toshi::TVector4::VEC_NEGY );

			Toshi::TVector4 vRotated;
			Toshi::TVector4::RotateArbitaryVector( vRotated, vDirection, vAxis, m_pData->m_fArcAngle );
			vDirection = vRotated;
		}

		TFLOAT fSpeed = ( m_pOverride == TNULL ) ? m_pData->m_fSpeed : m_pOverride->m_fSpeed;
		m_vVelocity.Multiply( vDirection, fSpeed );
	}

	m_vAccel.y = ( m_pOverride == TNULL ) ? m_pData->m_fGravity : m_pOverride->m_fGravity;
	m_vAccel.w = 1.0f;

	vDirection.w = 1.0f;
	SetVelocity( vDirection );

	m_uiFlags = a_uiFlags;
}

// $deBlob: FUNCTION 007d4bb0
void AProjectile::Fire( ABaseEntity* a_pOwner, const Toshi::TVector4& a_rvPosition, const Toshi::TVector4& a_rvTarget, TUINT a_uiFlags )
{
	TFLOAT          fGravity = ( m_pOverride == TNULL ) ? m_pData->m_fGravity : m_pOverride->m_fGravity;
	Toshi::TVector4 vDirection;

	if ( fGravity == 0.0f )
	{
		// Straight: the unit direction to the target at the speed
		vDirection.x = a_rvTarget.x - a_rvPosition.x;
		vDirection.y = a_rvTarget.y - a_rvPosition.y;
		vDirection.z = a_rvTarget.z - a_rvPosition.z;
		vDirection.w = a_rvTarget.w;
		vDirection.Normalise();

		TFLOAT fSpeed = ( m_pOverride == TNULL ) ? m_pData->m_fSpeed : m_pOverride->m_fSpeed;
		vDirection.Multiply( fSpeed );
	}
	else
	{
		// Ballistic: the horizontal direction and the launch velocity that reaches the target under gravity
		Toshi::TVector4 vFlat;
		vFlat.x = a_rvTarget.x - a_rvPosition.x;
		vFlat.y = a_rvTarget.y - a_rvPosition.y;
		vFlat.z = a_rvTarget.z - a_rvPosition.z;
		vFlat.w = a_rvTarget.w;
		vFlat.NormaliseXZ();

		TFLOAT fDist   = Toshi::TVector4::DistanceXZ( a_rvTarget, a_rvPosition );
		TFLOAT fSpeed  = ( m_pOverride == TNULL ) ? m_pData->m_fSpeed : m_pOverride->m_fSpeed;
		TFLOAT fTime   = fDist / fSpeed;
		TFLOAT fHeight = fTime * 0.5f * fTime * fGravity;
		TFLOAT fArc    = m_pData->m_fArcAngle;

		if ( fArc >= 0.0f )
		{
			TFLOAT fSin;
			TFLOAT fCos;
			Toshi::TMath::SinCos( fArc, fSin, fCos );

			TFLOAT fDY           = a_rvTarget.y - a_rvPosition.y;
			TFLOAT fDiscriminant = fDY * fDY - ( -( fGravity * fDist ) / ( fCos * 2.0f ) ) * fSin * 4.0f;
			TFLOAT fRoot         = Toshi::TMath::Sqrt( fDiscriminant );

			TFLOAT fT = ( -fDY - fRoot ) / ( fSin * 2.0f );
			if ( fT < 0.0f )
				fT = ( fRoot - fDY ) / ( fSin * 2.0f );

			// The speed component is clamped to [-speed, speed]
			TFLOAT fClamped = ( -fSpeed <= fT ) ? fT : -fSpeed;
			TFLOAT fV       = ( fClamped <= fSpeed ) ? fClamped : fSpeed;

			vDirection.x = vFlat.x * fV * fCos;
			vDirection.y = -( fV * fSin );
			vDirection.z = vFlat.z * fV * fCos;
		}
		else
		{
			vDirection.x = vFlat.x;
			vDirection.y = vFlat.y;
			vDirection.z = vFlat.z;
			vDirection.w = vFlat.w;
			vDirection.Multiply( fSpeed );

			// The vertical component of the flat shot, clamped to [-2.5 speed, 2.5 speed]
			TFLOAT fY   = ( a_rvTarget.y - a_rvPosition.y ) - fHeight / fTime;
			TFLOAT fLow = -fSpeed * 2.5f;
			if ( fLow > fY )
				fY = fLow;

			TFLOAT fHigh = fSpeed * 2.5f;
			if ( fY > fHigh )
				fY = fHigh;

			vDirection.y = fY;
		}
	}

	vDirection.w = 1.0f;
	Activate( a_pOwner, a_rvPosition, vDirection, a_uiFlags & ~0x20 );
}

// $deBlob: FUNCTION 007d4f90
void AProjectile::HitEntity( ABaseEntity* a_pEntity )
{
	if ( m_pOwner != TNULL && !m_pOwner->IgnoreProjectileCollision( a_pEntity ) )
	{
		const Toshi::TVector4& rvTarget = a_pEntity->GetTranslation();

		Toshi::TVector4 vDirection;
		vDirection.x = rvTarget.x - m_vPosition.x;
		vDirection.y = rvTarget.y - m_vPosition.y;
		vDirection.z = rvTarget.z - m_vPosition.z;
		vDirection.w = rvTarget.w;
		vDirection.NormaliseXZ();

		TINT iType = m_uiAttackType;
		if ( iType == 0 )
		{
			iType = 2;
			if ( ( m_pOwner->GetFlags() >> 4 & 1 ) == 0 )
				iType = 9;
		}

		TUINT uiDamage;
		TUINT uiUnk18;

		if ( m_pOverride == TNULL )
		{
			uiDamage = m_pData->m_uiDamage;
			uiUnk18  = m_pData->m_uiUnk24;
		}
		else
		{
			uiDamage = m_pOverride->m_uiDamage;
			uiUnk18  = m_pOverride->m_uiUnk24;
		}

		AAttackMessage oAttack( static_cast<AAttackMessage::ATTACKTYPE>( iType ), m_vPosition, uiDamage, uiUnk18, vDirection, m_pData->m_bUnk28 );

		AEntityIOPacket oPacket;
		if ( ( m_pOwner->GetFlags() >> 4 & 1 ) == 0 )
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE( AEntityIOPacket::MESSAGE_PROPATTACK - ( ( m_pOwner->GetFlags() >> 5 & 1 ) != 0 ) );
		else
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PLAYERATTACK;

		oPacket.m_iData = TREINTERPRETCAST( TINT, &oAttack );
		a_pEntity->HandleEntityIO( m_pOwner, oPacket );
	}
}

// $deBlob: FUNCTION 007d50a0
void AProjectile::HitInstance( TerrainTypes::CellInstance* a_pInstance )
{
	TUINT32 uiType = a_pInstance->m_uiFlags & 7;

	if ( uiType == TerrainTypes::InstanceType_Tree )
	{
		// A painted tree that isn't changing is inked
		ATreeInstance* pTree = TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 );

		if ( pTree->IsTransformed() && !pTree->IsTransforming() )
			ATreeInstance::Ink( a_pInstance );
	}
	else if ( uiType == TerrainTypes::InstanceType_Building )
	{
		// A building that can be inked and isn't locked out of paint (Flags_NotPaintable) turns black
		ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, a_pInstance->m_pModelInstance2 );

		if ( ( pBuilding->GetTerrainInstance()->m_uiFlags & 0x400 ) != 0 && ( pBuilding->GetBuildingFlags() & ABuildingInstance::Flags_NotPaintable ) == 0 )
			pBuilding->SetColour( ABlobColourComponent::COLOUR_BLACK, -1, TFALSE, TFALSE );
	}
}

// $deBlob: FUNCTION 007d5110
void AProjectile::UpdateLifeTime( TFLOAT a_fDeltaTime )
{
	if ( ( m_uiFlags & 2 ) == 0 )
		return;

	m_fLifeTime += a_fDeltaTime;

	if ( m_fLifeTime > GetMaxLifeTime() )
	{
		// Expired
		Kill( TTRUE );
		return;
	}

	TVector4 vAccel;
	vAccel.Multiply( m_vAccel, a_fDeltaTime );
	m_vVelocity.x += vAccel.x;
	m_vVelocity.y += vAccel.y;
	m_vVelocity.z += vAccel.z;

	// Where it's going to be (NOTE: retail scales the velocity by radius + delta time, not by the delta time alone)
	TFLOAT fTime  = GetRadius() + a_fDeltaTime;
	TFLOAT fNextX = fTime * m_vVelocity.x + m_vPosition.x;
	TFLOAT fNextY = m_vVelocity.y * fTime + m_vPosition.y;
	TFLOAT fNextZ = m_vVelocity.z * fTime + m_vPosition.z;

	const TPhysicsManagerInitData& rcWorld = TPhysicsManager::GetSingleton()->GetInitData();
	TVector4                       vMin    = rcWorld.m_vBroadPhaseMin;
	TVector4                       vMax    = rcWorld.m_vBroadPhaseMax;

	if ( vMin.x > fNextX || vMin.y > fNextY || vMin.z > fNextZ || fNextX > vMax.x || fNextY > vMax.y || fNextZ > vMax.z )
	{
		// Leaving the physics world
		Kill( TTRUE );
		return;
	}

	TVector4 vDirection = m_vVelocity;
	TFLOAT   fDistance  = vDirection.NormaliseMag() * a_fDeltaTime;
	TFLOAT   fStep      = 0.0f;

	// The layers the owner's projectiles collide with
	TUINT32 uiLayer      = 0xE7;
	TUINT8  uiOwnerFlags = TUINT8( m_pOwner->GetCollisionGroupAndType() );

	if ( ( uiOwnerFlags & 1 ) != 0 )
		uiLayer = 0xE3;

	if ( ( uiOwnerFlags & 2 ) != 0 )
		uiLayer &= ~2;

	if ( ( uiOwnerFlags & 0x10 ) != 0 )
		uiLayer &= ~4;

	if ( ( uiOwnerFlags & 8 ) != 0 )
		uiLayer &= 0xFFFFFF7E;

	TVector4          vNormal;
	ACollisionObject* pIgnore = m_pOwner->GetCollisionObject();
	ACollisionObject* pHit    = ACollisionManager2::CollideSweptSphere( m_vPosition, GetRadius(), vDirection, fDistance, vNormal, 0.0f, fStep, pIgnore, TNULL, uiLayer );

	TVector4 vMove;
	vMove.Multiply( vDirection, fDistance );
	m_vPosition.x += vMove.x;
	m_vPosition.y += vMove.y;
	m_vPosition.z += vMove.z;

	if ( pHit != TNULL )
	{
		ABaseEntity* pEntity = TNULL;

		if ( ( pHit->GetFlags() & ( ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) ) != 0 )
		{
			// An entity
			pEntity = TSTATICCAST( ABaseEntity, pHit->GetOwner() );
			TASSERT( pEntity == TNULL || pEntity->IsA( &TGetClass( ABaseEntity ) ) );
			HitEntity( pEntity );
		}
		else if ( ( pHit->GetFlags() & ACollisionObject::FLAG_OwnerType0 ) != 0 )
		{
			HitWorld();
		}

		if ( m_pOwner != TNULL )
		{
			// Tells the owner that its projectile hit something
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PROJECTILEHIT;
			oPacket.m_iData    = TREINTERPRETCAST( TINT, this );
			m_pOwner->HandleEntityIO( TNULL, oPacket );
		}

		AreaDamage( pEntity );
		Kill( TTRUE );
		ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 8, m_vPosition, 1.0f, -1 );
	}
}

// $deBlob: FUNCTION 007d53b0
void AProjectile::UpdateEffects( TFLOAT a_fDeltaTime )
{
	if ( ( m_uiFlags & 8 ) == 0 )
		return;

	// Running: a composition that isn't finished (the trail and the hit, then the third one)
	TBOOL bRunning = ( m_pFXTrail != TNULL && !m_pFXTrail->IsFinished() ) || ( m_pFXHit != TNULL && !m_pFXHit->IsFinished() );
	bRunning       = bRunning || ( m_pFXUnk != TNULL && !m_pFXUnk->IsFinished() );

	if ( !bRunning )
	{
		m_uiFlags &= ~8;
		return;
	}

	if ( ( m_uiFlags & 2 ) == 0 )
	{
		// Not moving: the compositions only update
		if ( m_pFXTrail != TNULL )
			m_pFXTrail->Update( a_fDeltaTime );

		if ( m_pFXHit != TNULL )
			m_pFXHit->Update( a_fDeltaTime );

		if ( m_pFXUnk != TNULL )
			m_pFXUnk->Update( a_fDeltaTime );

		return;
	}

	// Moving: the compositions follow the position
	Toshi::TMatrix44 mTransform = Toshi::TMatrix44::IDENTITY;
	mTransform.m_f41            = m_vPosition.x;
	mTransform.m_f42            = m_vPosition.y;
	mTransform.m_f43            = m_vPosition.z;
	mTransform.m_f44            = m_vPosition.w;

	if ( m_pFXTrail != TNULL )
	{
		m_pFXTrail->SetTransform( mTransform );
		m_pFXTrail->Update( a_fDeltaTime );
	}

	if ( m_pFXHit != TNULL )
		m_pFXHit->Update( a_fDeltaTime );

	if ( m_pParticleEffectTrail != TNULL )
		m_pParticleEffectTrail->GetTransform() = mTransform;

	m_uiFlags |= 8;
}

// $deBlob: FUNCTION 007d5530
void AProjectile::SetVelocity( const Toshi::TVector4& a_rvVelocity )
{
	Toshi::TMatrix44 mTransform = Toshi::TMatrix44::IDENTITY;

	if ( a_rvVelocity.y * Toshi::TVector4::VEC_NEGY.y + a_rvVelocity.x * Toshi::TVector4::VEC_NEGY.x + a_rvVelocity.z * Toshi::TVector4::VEC_NEGY.z < 0.9f )
		mTransform.LookAtDirection( a_rvVelocity, Toshi::TVector4::VEC_NEGY );

	mTransform.m_f41 = m_vPosition.x;
	mTransform.m_f42 = m_vPosition.y;
	mTransform.m_f43 = m_vPosition.z;
	mTransform.m_f44 = m_vPosition.w;

	InitEffects( mTransform );

	if ( m_pModel != TNULL )
	{
		m_pModel->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
		StopEffects();
	}
}

// $deBlob: FUNCTION 007d5630
void AProjectile::StopEffects()
{
	// NOTE: despite the name this only orients the model along the reverse velocity while the projectile moves
	if ( m_pModel != TNULL && ( m_pModel->GetFlags() & AModelInstance2::FLAG_Visible ) != 0 && ( m_uiFlags & 2 ) != 0 )
	{
		Toshi::TMatrix44 mTransform = Toshi::TMatrix44::IDENTITY;
		mTransform.m_f41            = m_vPosition.x;
		mTransform.m_f42            = m_vPosition.y;
		mTransform.m_f43            = m_vPosition.z;
		mTransform.m_f44            = m_vPosition.w;

		Toshi::TVector4 vDirection = m_vVelocity;
		vDirection.Normalise();
		vDirection.w = 1.0f;

		Toshi::TVector4 vReverse;
		vReverse.Multiply( vDirection, -1.0f );
		mTransform.LookAtDirection( vReverse, Toshi::TVector4::VEC_NEGX );

		m_pModel->SetTransform( mTransform );
	}
}

// $deBlob: FUNCTION 007d56f0
void AProjectile::InitEffects( const Toshi::TMatrix44& a_rTransform )
{
	TBOOL bHit   = m_pFXHit != TNULL;
	TBOOL bTrail = m_pFXTrail != TNULL;

	if ( bHit )
		m_pFXHit->TriggerEffect( a_rTransform );

	if ( bTrail )
		m_pFXTrail->TriggerEffect( a_rTransform );

	TASSERT( m_pParticleEffectTrail == TNULL );

	// Running while any effect was started
	TBOOL bRunning = bHit || bTrail;

	if ( m_pParticleEffect != TNULL && !bRunning )
		bRunning = StartParticleTrail();

	if ( bRunning )
		m_uiFlags |= 8;
	else
		m_uiFlags &= ~8;
}

// $deBlob: FUNCTION 007d5780
TBOOL AProjectile::StartParticleTrail()
{
	if ( m_pParticleEffect == TNULL || m_pParticleEffectTrail != TNULL )
		return TFALSE;

	TMatrix44 mTransform = TMatrix44::IDENTITY;
	TASSERT( m_vPosition.w == 1.0f );
	mTransform.SetTranslation( m_vPosition );

	m_pParticleEffectTrail = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pParticleEffect );

	if ( m_pParticleEffectTrail == TNULL )
		return TFALSE;

	m_pParticleEffectTrail->SetFinishedCallback( OnParticleTrailFinished, this );
	m_pParticleEffectTrail->GetTransform() = mTransform;
	return TTRUE;
}

// $deBlob: FUNCTION 007d5880
void AProjectile::AreaDamage( ABaseEntity* a_pIgnore )
{
	TFLOAT fRadius = GetAreaRadius();

	if ( fRadius <= 0.0f )
		return;

	// The entities in range that collide with the owner's projectiles are attacked
	TINT          iNumEntities = 0;
	ABaseEntity** ppEntities   = AEntityHash::GetSingleton()->Query( m_vPosition, fRadius, &iNumEntities, TNULL, TFALSE, 500, TFALSE, TTRUE, TFALSE );

	for ( TINT i = 0; i < iNumEntities; i++ )
	{
		ABaseEntity* pEntity = ppEntities[ i ];

		if ( m_pOwner != pEntity && a_pIgnore != pEntity && pEntity->GetCollisionObject() != TNULL )
		{
			TUINT8 uiOwnerFlags = TUINT8( m_pOwner->GetCollisionGroupAndType() );

			if ( ( uiOwnerFlags & pEntity->GetCollisionObject()->GetGroupMask() ) != 0 )
			{
				const TVector4& rcTranslation = pEntity->GetTranslation();

				TVector4 vDelta;
				vDelta.x = rcTranslation.x - m_vPosition.x;
				vDelta.y = rcTranslation.y - m_vPosition.y;
				vDelta.z = rcTranslation.z - m_vPosition.z;
				vDelta.w = rcTranslation.w;

				TFLOAT fDistance = vDelta.Magnitude() - pEntity->GetBoundingSphereRadius();

				if ( fDistance <= fRadius )
					HitEntity( pEntity );
			}
		}
	}

	// The terrain instances in range: the vis tree of the cell and its neighbours, then their large instances
	ATerrain2Interface* pInterface = ATerrain2Interface::GetInstance();
	TBoxTree*           pTrees     = pInterface->GetPerCellInstancesVisTrees();
	TSphere             oSphere( m_vPosition.x, m_vPosition.y, m_vPosition.z, fRadius );

	TINT  iCell = ATerrain2::GetInstance()->GetCellIndex( pInterface->GetCurrentRegion(), &m_vPosition, TNULL );
	TINT* pVis  = ATerrain2::GetInstance()->GetCellVis( pInterface->GetCurrentRegion(), iCell );

	for ( TINT i = -1; i < 8; i++ )
	{
#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail reads the neighbours of no cell (GetCellVis returned TNULL) when the hit is outside every cell
		if ( i >= 0 && pVis == TNULL )
			break;
#endif

		TINT iVisCell = ( i >= 0 ) ? pVis[ i ] : iCell;

		if ( iVisCell == -1 )
			continue;

		TBoxTree& rTree = pTrees[ iVisCell ];

		// The tree is rendered with the sphere and this projectile, then restored
		const TSphere*             pOldSphere   = rTree.GetSphere();
		void*                      pOldUserData = rTree.GetUserData();
		TUINT8                     uiOldFlags   = rTree.GetFlags();
		TBoxTree::t_RenderFunction fnOldRender  = rTree.GetRenderFunction();

		rTree.SetSphere( &oSphere );
		rTree.SetRenderFunction( HitInstanceCallback );
		rTree.SetUserData( this );
		rTree.SetFlag( TBoxTree::Flags_Sphere );
		rTree.Render();
		rTree.SetRenderFunction( fnOldRender );
		rTree.SetSphere( pOldSphere );
		rTree.SetUserData( pOldUserData );

		if ( ( uiOldFlags & TBoxTree::Flags_Sphere ) == 0 )
			rTree.ClearFlag( TBoxTree::Flags_Sphere );

		TerrainTypes::Cell* pCell = ATerrain2::GetInstance()->GetCell( pInterface->GetCurrentRegion(), iVisCell );

		for ( TUINT j = 0; j < pCell->m_uiNumUnk2C; j++ )
		{
			TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, pCell->m_ppUnk2C[ j ] );

			TAABox oBox( *pInstance->m_ppCentre[ 0 ], *pInstance->m_ppCentre[ 1 ] );

			if ( oBox.IntersectSphere( oSphere ) )
				HitInstance( pInstance );
		}
	}
}

// $deBlob: FUNCTION 007d5af0
void AProjectile::OnParticleTrailFinished( TParticleEffectInstance* a_pEffectInstance, void* a_pUserData )
{
	AProjectile* pProjectile = TSTATICCAST( AProjectile, a_pUserData );
	TASSERT( pProjectile->m_pParticleEffectTrail == a_pEffectInstance );
	pProjectile->m_pParticleEffectTrail = TNULL;
}

// $deBlob: FUNCTION 007d4f70
void AProjectile::HitInstanceCallback( void* a_pItem, void* a_pUserData )
{
	TSTATICCAST( AProjectile, a_pUserData )->HitInstance( TSTATICCAST( TerrainTypes::CellInstance, a_pItem ) );
}

// $deBlob: FUNCTION 007d5bb0
TFLOAT AProjectile::GetRadius() const
{
	return ( m_pOverride == TNULL ) ? m_pData->m_fRadius : m_pOverride->m_fRadius;
}

// $deBlob: FUNCTION 007d49e0
void AProjectile::Place( ABaseEntity* a_pOwner, const TVector4& a_rvPosition, const TVector4& a_rvDirection, TUINT a_uiFlags )
{
	TVector4 vDirection = a_rvDirection;

	if ( !vDirection.IsNormalised( 0.05f ) )
		vDirection.Normalise();

	m_pOwner    = a_pOwner;
	m_fLifeTime = 0.0f;
	m_vPosition = a_rvPosition;
	m_vVelocity = TVector4::VEC_ZERO;
	m_vAccel    = TVector4::VEC_ZERO;
	SetVelocity( vDirection );
	m_uiFlags = a_uiFlags;
}

// $deBlob: FUNCTION 007d4830
void AProjectile::Tick( TFLOAT a_fDeltaTime )
{
	UpdateLifeTime( a_fDeltaTime );
	UpdateEffects( a_fDeltaTime );
	StopEffects();

	if ( ( m_uiFlags & 0xE ) != 0 )
		m_uiFlags |= 1;
	else
		m_uiFlags &= ~1;
}

// $deBlob: FUNCTION 007d5b50
// $deBlob: FUNCTION 007d47c0
AProjectile::~AProjectile()
{
	AFXCompositionInstance* apFX[ 3 ] = { m_pFXTrail, m_pFXHit, m_pFXUnk };

	for ( TINT i = 0; i < 3; i++ )
	{
		if ( apFX[ i ] != TNULL )
		{
			apFX[ i ]->StopInstant();
			apFX[ i ]->Destroy();
			delete apFX[ i ];
		}
	}

	if ( m_pModel != TNULL )
		AModelFactory::GetSingleton()->DestroyInstance( m_pModel );
}

// $deBlob: FUNCTION 007d4580
AProjectile::AProjectile( AProjectileTypes::PROJECTILETYPES a_eType, ADataProjectile* a_pData )
{
	m_pOwner               = TNULL;
	m_eType                = a_eType;
	m_pData                = a_pData;
	m_pOverride            = TNULL;
	m_fLifeTime            = -1.0f;
	m_vPosition            = Toshi::TVector4::VEC_ZERO;
	m_vVelocity            = Toshi::TVector4::VEC_ZERO;
	m_vAccel               = Toshi::TVector4::VEC_ZERO;
	m_uiAttackType         = 0;
	m_uiFlags              = 0;
	m_pModel               = TNULL;
	m_pFXTrail             = TNULL;
	m_pFXHit               = TNULL;
	m_pFXUnk               = TNULL;
	m_pParticleEffect      = TNULL;
	m_pParticleEffectTrail = TNULL;

	TASSERT( 0 <= m_eType && m_eType < AProjectileTypes::PROJECTILE_Count );

	// The compositions and the model are named by the data (an empty name is no effect)
	if ( TStringManager::String8Compare( "", m_pData->m_szFXTrail ) != 0 )
	{
		m_pFXTrail = new AFXCompositionInstance();
		m_pFXTrail->Initialize( m_pData->m_szFXTrail );
	}

	if ( TStringManager::String8Compare( "", m_pData->m_szFXHit ) != 0 )
	{
		m_pFXHit = new AFXCompositionInstance();
		m_pFXHit->Initialize( m_pData->m_szFXHit );
	}

	if ( TStringManager::String8Compare( "", m_pData->m_szFXUnk ) != 0 )
	{
		m_pFXUnk = new AFXCompositionInstance();
		m_pFXUnk->Initialize( m_pData->m_szFXUnk );
	}

	if ( TStringManager::String8Compare( "", m_pData->m_szModelName ) != 0 )
	{
		m_pModel = AModelFactory::GetSingleton()->CreateInstance( m_pData->m_szModelName, TFALSE, TFALSE );
		m_pModel->SetTransform( Toshi::TMatrix44::IDENTITY );

		if ( m_pModel != TNULL )
			m_pModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}
}
