#include "pch.h"
#include "Entities/APickupManager.h"
#include "Sound/APositionalSoundManager.h"
#include "ACar.h"
#include "Entities/ATankSpawner/ATankSpawner.h"
#include "Entities/AEntityPhysicsMover.h"
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include "Entities/AEntityParams.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/AModelsAttachedComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "Fx/Shadows/AShadow.h"
#include "Sound/ASoundInterface.h"
#include <TParticleSystem/TParticleEffectInstance.h>
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Animation/AModel2.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AAttackMessage.h"
#include "Entities/AEntityData.h"
#include "Entities/Projectiles/ADataProjectile.h"
#include "Entities/Projectiles/AProjectile.h"
#include "Entities/Projectiles/AProjectileManager.h"
#include "Collision/ACollisionInstance2.h"
#include "Cameras/ACameraManager.h"
#include "Challenges/AChallengeManager.h"
#include "Fx/AParticleEffectHelper.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Input/AVibrationManager.h"
#include "ARandom.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/TBoxData.h>
#include <Physics/TCapsuleData.h>
#include <Physics/TCylinderData.h>
#include <Physics/TSphereData.h>
#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TParticleEffect* ACar::ms_pCollisionFX = TNULL;
Toshi::TParticleEffect* ACar::ms_pExplosionFX = TNULL;

TSTATICASSERT( sizeof( ACar ) == 0x204 );
TSTATICASSERT( sizeof( ACar::CarData ) == 0x7C );

// $deBlob: FUNCTION 00774f70
// $deBlob: FUNCTION 00774f80
// $deBlob: FUNCTION 00774ff0
// $deBlob: FUNCTION 00775050
// $deBlob: FUNCTION 00775060
// $deBlob: FUNCTION 005409a0
// $deBlob: FUNCTION 00859ce0
TDEFINE_CLASS( ACar );

// $deBlob: FUNCTION 00775070
ACar::ACar()
{
	m_uiCarFlags      = CARFLAG_Waiting;
	m_eCarType        = 0;
	m_uiTankerStage   = 0;
	m_iTurretAnim     = -1;
	m_pTurretData     = TNULL;
	m_pModelsAttached = TNULL;
	m_pExtraModel     = TNULL;

	if ( ms_pCollisionFX == TNULL )
	{
		ms_pCollisionFX = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_CarCollision" );
		ms_pExplosionFX = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_Explosion" );
	}

	CreateShadow( 4.0f, 0.75f );
}

// $deBlob: FUNCTION 00775150
// $deBlob: FUNCTION 00779650
// $deBlob: FUNCTION 00779644
ACar::~ACar()
{
	ms_pCollisionFX = TNULL;
	ms_pExplosionFX = TNULL;

	if ( m_pModelsAttached != TNULL )
		delete m_pModelsAttached;

	if ( m_pExtraModel != TNULL )
		delete m_pExtraModel;

	if ( m_pTurretData != TNULL )
	{
		if ( m_pTurretData->pHelper != TNULL )
			delete m_pTurretData->pHelper;

		::operator delete( m_pTurretData );
	}
}

// $deBlob: FUNCTION 00775220
// $deBlob: FUNCTION 007797be
void ACar::ProcessParams( const AEntityParams& a_rParams )
{
	APropPlatform::ProcessParams( a_rParams );

	const CarData* pCarData = TSTATICCAST( const CarData, m_pTypeData->m_pTRBData );

	TUINT8 uiCarType = 0;
	a_rParams.GetValue( uiCarType, "Vehicle Type" );
	m_eCarType = uiCarType;

	CarData oCarData = *pCarData;

	if ( uiCarType == 4 )
	{
		pCarData                  = &oCarData;
		oCarData.m_uiPhysicsFlags = 0x4000;
	}

	TBOOL bTargetable = TFALSE;
	a_rParams.GetValue( bTargetable, "Targetable" );

	if ( bTargetable )
		m_uiCarFlags |= CARFLAG_Targetable;
	else
		m_uiCarFlags &= ~CARFLAG_Targetable;

	TBOOL bTargetableFromBelow = TFALSE;
	a_rParams.GetValue( bTargetableFromBelow, "TargetableFromBelow" );

	if ( bTargetableFromBelow )
		m_uiCarFlags |= CARFLAG_TargetableFromBelow;
	else
		m_uiCarFlags &= ~CARFLAG_TargetableFromBelow;

	// The collision: a collision object of the params, the car's collision mesh (type 4) or built-in shapes
	AEntityParam::Collision** ppCollision = TNULL;

	if ( a_rParams.GetCollision( ppCollision ) )
	{
		CreatePhysics( pCarData, *ppCollision, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}
	else if ( m_eCarType == VEHICLETYPE_TANK )
	{
		// The tank uses the collision meshes of its model
		AModelInstance2* pModelInstance  = m_pModelComponent->GetModelInstance();
		TModelInstance*  pTModelInstance = pModelInstance->GetModelInstance();

		ACollisionInstance2* pCollision = new ACollisionInstance2();
		pCollision->SetModelInstance( pTModelInstance, TNULL );

		ACollisionInstance2::OwnerInfo oOwner;
		oOwner.m_eType  = 2;
		oOwner.m_pOwner = this;

		TINT aShapeTypes[ 3 ] = { 2, 4, 5 };
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
	else
	{
		// Built-in shapes: boxes (and a cylinder, a capsule and spheres) per vehicle type
		TBoxData      aBoxes[ 7 ];
		TCapsuleData  oCapsule;
		TCylinderData oCylinder;
		TSphereData   aSpheres[ 3 ];

		TMatrix44        aMatrices[ 7 ];
		TShapeBaseClass* apShapes[ 7 ];
		TINT             iNumBoxes = -1;

		AEntityTypeData::TRBData oTRBData = *pCarData;

		switch ( m_eCarType )
		{
			case VEHICLETYPE_CAR:
				aBoxes[ 0 ].SetHalfExtents( TVector4( 1.6f, 0.8f, 2.8f, 1.0f ) );
				aMatrices[ 0 ] = TMatrix44::IDENTITY;
				aMatrices[ 0 ].SetTranslation( TVector4( 0.0f, -0.8f, 0.5f, 1.0f ) );
				apShapes[ 0 ] = &aBoxes[ 0 ];

				aBoxes[ 1 ].SetHalfExtents( TVector4( 1.2f, 1.0f, 1.6f, 1.0f ) );
				aMatrices[ 1 ] = TMatrix44::IDENTITY;
				aMatrices[ 1 ].SetTranslation( TVector4( 0.0f, -2.6f, 0.2f, 1.0f ) );
				apShapes[ 1 ] = &aBoxes[ 1 ];

				iNumBoxes = 2;
				break;
			case 1:
			case 2:
			case 3:
				aBoxes[ 0 ].SetHalfExtents( TVector4( 1.5f, 2.7f, 2.9f, 1.0f ) );
				aMatrices[ 0 ] = TMatrix44::IDENTITY;
				aMatrices[ 0 ].SetTranslation( TVector4( 0.0f, -2.7f, 0.3f, 1.0f ) );
				apShapes[ 0 ] = &aBoxes[ 0 ];
				iNumBoxes     = 1;
				break;
			case VEHICLETYPE_TANK:
				aBoxes[ 0 ].SetHalfExtents( TVector4( 4.0f, 6.5f, 7.0f, 1.0f ) );
				aMatrices[ 0 ] = TMatrix44::IDENTITY;
				aMatrices[ 0 ].SetTranslation( TVector4( 0.0f, -6.5f, 0.3f, 1.0f ) );
				apShapes[ 0 ] = &aBoxes[ 0 ];
				iNumBoxes     = 1;
				break;
			case 5:
				aBoxes[ 0 ].SetHalfExtents( TVector4( 3.0f, 2.8f, 11.5f, 1.0f ) );
				aMatrices[ 0 ] = TMatrix44::IDENTITY;
				aMatrices[ 0 ].SetTranslation( TVector4( 0.0f, -2.8f, 0.0f, 1.0f ) );
				apShapes[ 0 ] = &aBoxes[ 0 ];
				iNumBoxes     = 1;
				break;
			case 6:
			{
				static const TFLOAT s_aafMatrices[ 7 ][ 16 ] = {
					{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -0.076035f, -2.838381f, 20.42191f, 1.0f },
					{ 0.824936f, 0.0f, -0.565226f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.565226f, 0.0f, 0.824936f, 0.0f, 5.238693f, -2.838381f, -14.17126f, 1.0f },
					{ 1.0f, 0.0f, -0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -2.838381f, -2.688679f, 1.0f },
					{ 0.7165f, -0.119464f, -0.687281f, 0.0f, 0.00062f, 0.985336f, -0.170626f, 0.0f, 0.697586f, 0.121827f, 0.706068f, 0.0f, -0.044907f, -8.339455f, -17.87584f, 1.0f },
					{ 0.844244f, 0.0f, 0.53596f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, -0.53596f, 0.0f, 0.844244f, 0.0f, -5.26696f, -2.838381f, -14.21069f, 1.0f },
					{ 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -0.076035f, -2.838381f, 15.36182f, 1.0f },
					{ 0.711366f, 0.0f, -0.702821f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.702822f, 0.0f, 0.711366f, 0.0f, -0.05925f, -2.838381f, -19.81689f, 1.0f },
				};

				static const TFLOAT s_aafExtents[ 7 ][ 3 ] = {
					{ 7.167546f, 10.31467f, 9.352392f },
					{ 3.562551f, 6.559392f, 4.927211f },
					{ 10.92801f, 6.559392f, 9.262655f },
					{ 5.726653f, 1.602029f, 5.576131f },
					{ 3.562551f, 6.559392f, 4.927211f },
					{ 10.18032f, 6.559392f, 9.729978f },
					{ 4.192249f, 6.559392f, 4.135876f },
				};

				for ( TINT i = 0; i < 7; i++ )
				{
					aBoxes[ i ].SetHalfExtents( TVector4( s_aafExtents[ i ][ 0 ], s_aafExtents[ i ][ 1 ], s_aafExtents[ i ][ 2 ], 1.0f ) );
					TUtil::MemCopy( &aMatrices[ i ], s_aafMatrices[ i ], sizeof( TMatrix44 ) );
					apShapes[ i ] = &aBoxes[ i ];
				}

				iNumBoxes = 7;
				break;
			}
			case VEHICLETYPE_PAINTTANKER:
			{
				// NOTE: retail keeps these in function-local statics, computed once
				const TFLOAT fTankHeight = 28.0f;
				const TFLOAT fTankRadius = 5.6f;
				const TFLOAT fTankX      = 0.0f;
				const TFLOAT fTankY      = -7.0f;
				const TFLOAT fHalfHeight = fTankHeight * 0.5f;
				const TFLOAT fSphereX    = fTankX;
				const TFLOAT fSphereY    = 2.0f - 14.0f;

				oCylinder.SetHeight( fTankHeight );
				oCylinder.SetRadius( fTankRadius );
				aMatrices[ 0 ] = TMatrix44::IDENTITY;
				aMatrices[ 0 ].SetTranslation( TVector4( fTankX, fTankY, 3.0f, 1.0f ) );
				apShapes[ 0 ] = &oCylinder;

				aBoxes[ 0 ].SetHalfExtents( TVector4( fTankRadius, fTankRadius * 0.5f, fHalfHeight, 1.0f ) );
				aMatrices[ 1 ] = TMatrix44::IDENTITY;
				aMatrices[ 1 ].SetTranslation( TVector4( fTankX, fTankRadius * -0.5f, 3.0f, 1.0f ) );
				apShapes[ 1 ] = &aBoxes[ 0 ];

				aSpheres[ 0 ].SetRadius( 2.0f );
				aMatrices[ 2 ] = TMatrix44::IDENTITY;
				aMatrices[ 2 ].SetTranslation( TVector4( fSphereX, fSphereY, 6.0f, 1.0f ) );
				apShapes[ 2 ] = &aSpheres[ 0 ];

				aSpheres[ 1 ].SetRadius( 2.0f );
				aMatrices[ 3 ] = TMatrix44::IDENTITY;
				aMatrices[ 3 ].SetTranslation( TVector4( fSphereX, fSphereY, fHalfHeight * 0.45f + 6.0f, 1.0f ) );
				apShapes[ 3 ] = &aSpheres[ 1 ];

				aSpheres[ 2 ].SetRadius( 2.0f );
				aMatrices[ 4 ] = TMatrix44::IDENTITY;
				aMatrices[ 4 ].SetTranslation( TVector4( fSphereX, fSphereY, 6.0f - fHalfHeight * 0.45f, 1.0f ) );
				apShapes[ 4 ] = &aSpheres[ 2 ];

				oTRBData.m_uiPhysicsFlags         = 0x1000000;
				oTRBData.m_aUnk10[ 0 ]            = 0;
				oTRBData.m_aUnk10[ 1 ]            = 0x3E4CCCCD;
				oTRBData.m_abCollisionGroups[ 4 ] = 0;

				iNumBoxes = 5;
				break;
			}
			case VEHICLETYPE_BLIMP:
				aBoxes[ 0 ].SetHalfExtents( TVector4( 6.7f, 7.7f, 11.5f, 1.0f ) );
				aMatrices[ 0 ] = TMatrix44::IDENTITY;
				aMatrices[ 0 ].SetTranslation( TVector4( -0.34f, -10.57f, -6.16f, 1.0f ) );
				apShapes[ 0 ] = &aBoxes[ 0 ];

				oCapsule.SetHeight( 29.34f );
				oCapsule.SetRadius( 16.7f );
				aMatrices[ 1 ] = TMatrix44::IDENTITY;
				aMatrices[ 1 ].SetTranslation( TVector4( -0.5f, -31.9f, 0.2f, 1.0f ) );
				aMatrices[ 1 ].Rotate( 0.0f, 0.0f, 1.5707964f );
				apShapes[ 1 ] = &oCapsule;

				oTRBData.m_uiPhysicsFlags         = 0x1000000;
				oTRBData.m_abCollisionGroups[ 4 ] = 1;

				iNumBoxes = 2;
				break;
			case VEHICLETYPE_TRAIN_PRISONCAR:
				iNumBoxes = 0;
				break;
			default:
				break;
		}

		TASSERT( -1 != iNumBoxes );

		if ( iNumBoxes != 0 )
		{
			CreatePhysics( &oTRBData, apShapes, aMatrices, iNumBoxes, TNULL, TFALSE );
			SetPhysicsMover( new AEntityPhysicsMover( this ) );
		}
	}

	// The models
	switch ( m_eCarType )
	{
		case VEHICLETYPE_CAR:
			// The pimped car
			m_pExtraModel = new AEntityModelComponent( this, "car_pimp.trb" );
			m_pExtraModel->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
			m_pExtraModel->GetModelInstance()->SetAmbientScale( m_fAmbientScale );
			SetMovingSound( 0x29, 0x1CC );
			break;
		case 3:
		{
			// The chopper
			m_pModelsAttached = new AModelsAttachedComponent( this );
			m_pModelsAttached->AttachModel( "Chopper_Fuselage.trb", "Chopper_SB_Fuselage", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "Chopper_BayDoor.trb", "Chopper_SB_BayDoor_Lf", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "Chopper_BayDoor.trb", "Chopper_SB_BayDoor_Rt", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "Chopper_Propeller.trb", "Chopper_SB_Blade_1_Lf", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "Chopper_Propeller.trb", "Chopper_SB_Blade_2_Lf", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "Chopper_Propeller.trb", "Chopper_SB_Blade_1_Rt", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "Chopper_Propeller.trb", "Chopper_SB_Blade_2_Rt", TTRUE, TFALSE, TFALSE );

			AToshiAnimInterface2* pAnimInterface = m_pModelComponent->GetModelInstance()->GetAnimInterface();
			pAnimInterface->AddAnim( TINT16( pAnimInterface->GetAnimID( "Idle_InFlight" ) ), 0.15f, 1.0f, 1.0f );
			pAnimInterface->AddAnim( TINT16( pAnimInterface->GetAnimID( "Propeller_Spin" ) ), 0.15f, 1.0f, 1.0f );
			break;
		}
		case VEHICLETYPE_TANK:
		{
			// The tank: its turret
			if ( !( TUINT8( m_uiCarFlags >> 3 ) & 1 ) )
				SetTankSounds();

			m_pTurretData = TREINTERPRETCAST( TurretData*, TMalloc( sizeof( TurretData ) ) );
			TUtil::MemClear( m_pTurretData, sizeof( TurretData ) );

			m_pTurretData->pHelper = new ATurretHelper( this );

			AModelInstance2* pModelInstance = m_pModelComponent->GetModelInstance();

			m_pTurretData->iFireBone = TINT16( pModelInstance->FindBone( "Tank_LB_Cannon_Tip" ) );
			TASSERT( -1 != m_pTurretData->iFireBone );

			m_pTurretData->iInkyBone = TINT16( pModelInstance->FindBone( "Tank_LB_Turret" ) );
			TASSERT( -1 != m_pTurretData->iInkyBone );

			m_pTurretData->iDamage25Bone = TINT16( pModelInstance->FindBone( "Tank_LB_Damage25" ) );
			TASSERT( -1 != m_pTurretData->iDamage25Bone );

			m_pTurretData->iDamage50Bone = TINT16( pModelInstance->FindBone( "Tank_LB_Damage50" ) );
			TASSERT( -1 != m_pTurretData->iDamage50Bone );

			m_pTurretData->iDamage75Bone = TINT16( pModelInstance->FindBone( "Tank_LB_Damage75" ) );
			TASSERT( -1 != m_pTurretData->iDamage75Bone );

			// NOTE: AProjectileTypes value 0 is the turret bullet (ATurretHelper::GetProjectileDataName 0), the pool size is 6
			AProjectileManager::GetSingleton()->InitPool( AProjectileTypes::PROJECTILETYPES( 0 ), 6 );

			ADataProjectile* pProjData = TSTATICCAST( ADataProjectile, AEntityData::GetSingleton()->GetProjectileData( ATurretHelper::GetProjectileDataName( 0 ) ) );

			m_pTurretData->pHelper->Initialise( pCarData->m_fSearchAngle, pCarData->m_fFireDelay, pProjData->m_fGravity, pProjData->m_fSpeed, pCarData->m_fRotateSpeed, pCarData->m_fVisualRangeA, pCarData->m_fVisualRangeB, pCarData->m_fMaxVisualAngleA, pCarData->m_fMaxVisualAngleB, pCarData->m_iMaxBurst, pCarData->m_fBurstDelay );

			m_pTurretData->pHelper->m_fPitchMax = 0.6f;

			TMatrix44 mBone;
			pModelInstance->GetBoneTransform( m_pTurretData->iInkyBone, mBone );
			m_pTurretData->pHelper->m_vOrigin = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

			pModelInstance->GetBoneTransform( m_pTurretData->iFireBone, mBone );
			m_pTurretData->pHelper->m_vLookAt = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

			TVector4 vFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
			vFacing.NormaliseXZ();
			m_pTurretData->pHelper->SetFacing( vFacing );

			AToshiAnimInterface2* pAnimInterface = pModelInstance->GetAnimInterface();

			m_pTurretData->iIdleAnim = TINT16( pAnimInterface->GetAnimID( "Idle_Loop" ) );
			TASSERT( m_pTurretData->iIdleAnim >= 0 );
			PlayTurretAnim( m_pTurretData->iIdleAnim, 0.0f, 1.0f, 1.0f );

			m_pTurretData->iFireAnimBehind = TINT16( pAnimInterface->GetAnimID( "Fire_Behind" ) );
			TASSERT( m_pTurretData->iFireAnimBehind >= 0 );

			m_pTurretData->iFireAnimFront = TINT16( pAnimInterface->GetAnimID( "Fire_Front" ) );
			TASSERT( m_pTurretData->iFireAnimFront >= 0 );

			m_pTurretData->iFireAnimLeft = TINT16( pAnimInterface->GetAnimID( "Fire_Left" ) );
			TASSERT( m_pTurretData->iFireAnimLeft >= 0 );

			m_pTurretData->iFireAnimRight = TINT16( pAnimInterface->GetAnimID( "Fire_Right" ) );
			TASSERT( m_pTurretData->iFireAnimRight >= 0 );

			TINT16 iHorizontalOverlayAnim = TINT16( pAnimInterface->GetAnimID( "Turret_Rotate_H" ) );
			TASSERT( iHorizontalOverlayAnim >= 0 );

			m_pTurretData->pHAnim = pAnimInterface->AddAnim( iHorizontalOverlayAnim, 0.0f, 1.0f, 0.0f );
			TASSERT( TNULL != m_pTurretData->pHAnim );

			m_pTurretData->fHAnimDuration = m_pTurretData->pHAnim->GetSequencePtr()->GetDuration();
			m_pTurretData->pHAnim->SetTime( m_pTurretData->pHelper->m_fAngleFrac * m_pTurretData->fHAnimDuration );

			TINT16 iVerticalOverlayAnim = TINT16( pAnimInterface->GetAnimID( "Turret_Rotate_P" ) );
			TASSERT( iVerticalOverlayAnim >= 0 );

			m_pTurretData->pVAnim = pAnimInterface->AddAnim( iVerticalOverlayAnim, 0.0f, 1.0f, 0.0f );
			TASSERT( TNULL != m_pTurretData->pVAnim );

			m_pTurretData->fVAnimDuration = m_pTurretData->pVAnim->GetSequencePtr()->GetDuration();
			m_pTurretData->pVAnim->SetTime( m_pTurretData->pHelper->m_fPitchFrac * m_pTurretData->fVAnimDuration );

			m_uiPlatformFlags |= 0x400000;

			m_pTurretData->iRecoilAnim = TINT16( pAnimInterface->GetAnimID( "Barrel_Recoil" ) );
			TASSERT( m_pTurretData->iRecoilAnim >= 0 );

			m_pTurretData->iReloadAnim = TINT16( pAnimInterface->GetAnimID( "Barrel_Reload" ) );
			TASSERT( m_pTurretData->iReloadAnim >= 0 );

			m_pTurretData->iHitAnim = TINT16( pAnimInterface->GetAnimID( "Hit" ) );
			TASSERT( m_pTurretData->iHitAnim >= 0 );

			TParticleEffectManager* pEffectManager = TParticleSystem::GetSingleton()->GetEffectManager();
			m_pTurretData->pInkySplatFX            = pEffectManager->FindEffect( "Blob_InkySplat" );
			m_pTurretData->pDamageSparksFX         = pEffectManager->FindEffect( "Blob_VehicleDamageSparks" );
			m_pTurretData->pDamageSmokeFX          = pEffectManager->FindEffect( "Blob_VehicleDamageSmoke" );
			m_pTurretData->apDamageFX[ 0 ]         = TNULL;
			m_pTurretData->apDamageFX[ 1 ]         = TNULL;
			m_pTurretData->apDamageFX[ 2 ]         = TNULL;

			if ( TUINT8( m_uiCarFlags >> 2 ) & 1 )
				m_pTurretData->pHelper->SetState( 3 );

			break;
		}
		case 5:
			SetDefaultMovingSound( 0x2B, 0x1D0 );
			break;
		case 6:
			SetDefaultMovingSound( 0x2A, 0x1CE );
			break;
		case VEHICLETYPE_PAINTTANKER:
		{
			m_pModelsAttached = new AModelsAttachedComponent( this );
			m_pModelsAttached->AttachModel( "PaintTanker_Body.trb", "painttanker_body", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "PaintTanker_Wheels.trb", "painttanker_wheels", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "PaintTanker_Paint.trb", "painttanker_paint1", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "PaintTanker_Crushed.trb", "painttanker_crushed1", TFALSE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "PaintTanker_Paint.trb", "painttanker_paint2", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "PaintTanker_Crushed.trb", "painttanker_crushed2", TFALSE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "PaintTanker_Paint.trb", "painttanker_paint3", TTRUE, TFALSE, TFALSE );
			m_pModelsAttached->AttachModel( "PaintTanker_Crushed.trb", "painttanker_crushed3", TFALSE, TFALSE, TFALSE );

			for ( TINT i = 0; i < m_pModelsAttached->GetNumModelsAttached(); i++ )
				m_pModelsAttached->GetModelInstance( i )->SetAmbientScale( m_fAmbientScale );

			// The paint tanks: the enemies' colours
			AModelInstance2* pPaint = m_pModelsAttached->GetModelInstance( 2 );
			pPaint->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
			pPaint->SetMeshFlags( 1, 1, TTRUE );
			pPaint->SetMeshColour( 1, ABlobColourMaps::ms_pEnemyBodyColourMap[ 4 ] );

			pPaint = m_pModelsAttached->GetModelInstance( 4 );
			pPaint->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
			pPaint->SetMeshFlags( 1, 1, TTRUE );
			pPaint->SetMeshColour( 1, ABlobColourMaps::ms_pEnemyBodyColourMap[ 2 ] );

			pPaint = m_pModelsAttached->GetModelInstance( 6 );
			pPaint->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
			pPaint->SetMeshFlags( 1, 1, TTRUE );
			pPaint->SetMeshColour( 1, ABlobColourMaps::ms_pEnemyBodyColourMap[ 1 ] );

			AToshiAnimInterface2* pAnimInterface = m_pModelComponent->GetModelInstance()->GetAnimInterface();
			pAnimInterface->AddAnim( TINT16( pAnimInterface->GetAnimID( "Idle" ) ), 0.15f, 1.0f, 1.0f );
			break;
		}
		case VEHICLETYPE_BLIMP:
		{
			// The blimp
			SetDefaultMovingSound( 0x1B, 299 );

			AToshiAnimInterface2* pAnimInterface = m_pModelComponent->GetModelInstance()->GetAnimInterface();
			pAnimInterface->AddAnim( TINT16( pAnimInterface->GetAnimID( "idle_loop" ) ), 0.15f, 1.0f, 1.0f );

			AModelInstance2* pModelInstance = m_pModelComponent->GetModelInstance();

			// The sphere map of its skin material (the 4th mesh of its only LOD)
			T2ModelPtr pModel = pModelInstance->GetModel()->GetModelPtr();
			TModelLOD* pLOD   = &pModel->GetLOD( 0 );

			TASSERT( 1 == pModelInstance->GetModel()->GetModelPtr()->GetLODCount() );

			ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ 3 ]->GetMaterial() );

			TTexture* pSphereMap = TTextureManager::GetSingleton()->FindTexture( "spheremap_blimp.tga" );
			pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

			TASSERT( pSkinMat->GetTextureNum() == 1 );

			pSkinMat->SetTextureNum( 2 );
			pSkinMat->SetTevMode( 1, 0xC );
			pSkinMat->SetTexture( 1, pSphereMap );
			pSkinMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
			pSkinMat->SetBlendMode( 24, 0 );

			pModelInstance->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
			pModelInstance->SetMeshFlags( 3, 1, TTRUE );
			pModelInstance->SetMeshColour( 3, 0x59000000 );
			break;
		}
		case VEHICLETYPE_TRAIN_PRISONCAR:
			// The train
			m_pExtraModel = new AEntityModelComponent( this, "train_inkharvest_tranz.trb" );
			m_pExtraModel->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
			m_pExtraModel->GetModelInstance()->SetAmbientScale( m_fAmbientScale );
			SetDefaultMovingSound( 0x2B, 0x1D0 );
			break;
	}

	// The LOD distance type (the high byte of m_iCollisionGroupAndType)
	TUINT8 uiLODType;

	switch ( m_eCarType )
	{
		case VEHICLETYPE_CAR:
		case 1:
		case 5:
		case 6:
		case VEHICLETYPE_BLIMP:
		case VEHICLETYPE_TRAIN_PRISONCAR:
			uiLODType = 1;
			break;
		case VEHICLETYPE_TANK:
			m_uiPropFlags &= ~1;
			uiLODType = 0x22;
			break;
		case VEHICLETYPE_PAINTTANKER:
			m_uiPropFlags &= ~1;
			uiLODType = 0x0E;
			break;
		default:
			uiLODType = 0xFF;
			break;
	}

	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( uiLODType << 8 ) );

	if ( m_eCarType == VEHICLETYPE_CAR )
		m_pShadow->SetSize( 4.0f, 8.0f );
	else if ( m_eCarType == VEHICLETYPE_TANK )
		m_pShadow->SetSize( 14.0f, 22.0f );
	else if ( m_eCarType == VEHICLETYPE_PAINTTANKER )
		m_pShadow->SetSize( 22.0f, 30.0f );

	if ( m_eCarType == VEHICLETYPE_TANK )
		m_uiExtraFlags |= 2;

	TINT iMaxLife = GetMaxLife();
	m_Flags &= ~0x400000;
	m_iLife = TINT16( iMaxLife );
}

// $deBlob: FUNCTION 00776ec0
void ACar::Create()
{
	SetState( 0 );
	AProp::Create();
}

// $deBlob: FUNCTION 00776ee0
void ACar::Destroy()
{
	if ( m_pModelsAttached != TNULL )
		m_pModelsAttached->DestroyModels();

	if ( m_pTurretData != TNULL && m_pTurretData->pHelper != TNULL )
		m_pTurretData->pHelper->StopEffects();

	if ( m_pTurretData != TNULL )
	{
		for ( TINT i = 0; i < TARRAYSIZE( m_pTurretData->apDamageFX ); i++ )
		{
			if ( m_pTurretData->apDamageFX[ i ] != TNULL )
			{
				m_pTurretData->apDamageFX[ i ]->StopInstant();
				m_pTurretData->apDamageFX[ i ] = TNULL;
			}
		}
	}

	APropPlatform::Destroy();
}

// $deBlob: FUNCTION 00776f70
void ACar::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pWaypointSet == TNULL || 0.0f <= m_fSpeed )
	{
		APropPlatform::Tick( a_fDeltaTime );

		if ( m_pTurretData != TNULL && !( TUINT8( m_uiCarFlags >> 2 ) & 1 ) )
		{
			if ( TUINT8( m_Flags >> 15 ) & 1 )
				m_pModelComponent->SetTransform( GetMatrix() );

			// The turret helper follows the turret bone and the car's facing
			TMatrix44 mBone;
			m_pModelComponent->GetModelInstance()->GetBoneTransform( m_pTurretData->iInkyBone, mBone );
			m_pTurretData->pHelper->m_vOrigin = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );

			TVector4 vFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
			vFacing.NormaliseXZ();
			m_pTurretData->pHelper->SetFacing( vFacing );
		}
	}

	if ( GetLOD() != FLAG_LowLOD && m_pTurretData != TNULL )
	{
		if ( TUINT8( m_uiCarFlags >> 4 ) & 1 )
		{
			StopIdleSound();
			StopMovingSound();
		}
		else if ( m_uiCarFlags & 1 )
		{
			PlayIdleSound();
			StopMovingSound();
		}
		else
		{
			StopIdleSound();
			PlayMovingSound();
		}
	}

	if ( m_pTurretData != TNULL )
	{
		TurretData* pTurret = m_pTurretData;

		// The tank's damage effects: sparks above 25% and 50% damage, smoke above 75% (the damage bones)
		if ( ( m_Flags & FLAG_Dead ) == 0 )
		{
			TFLOAT fRatio = 1.0f - (TFLOAT)m_iLife / (TFLOAT)GetMaxLife();

			// 25%: the sparks
			if ( fRatio <= 0.25f )
			{
				if ( pTurret->apDamageFX[ 0 ] != TNULL )
				{
					pTurret->apDamageFX[ 0 ]->StopInstant();
					pTurret->apDamageFX[ 0 ] = TNULL;
				}
			}
			else
			{
				if ( pTurret->pDamageSparksFX != TNULL && pTurret->apDamageFX[ 0 ] == TNULL )
					pTurret->apDamageFX[ 0 ] = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pTurret->pDamageSparksFX );

				if ( pTurret->apDamageFX[ 0 ] != TNULL )
				{
					TMatrix44 mBone;
					m_pModelComponent->GetModelInstance()->GetBoneTransform( pTurret->iDamage25Bone, mBone );
					pTurret->apDamageFX[ 0 ]->GetTransform() = mBone;
				}
			}

			// 50%: the sparks again
			if ( fRatio <= 0.5f )
			{
				if ( pTurret->apDamageFX[ 1 ] != TNULL )
				{
					pTurret->apDamageFX[ 1 ]->StopInstant();
					pTurret->apDamageFX[ 1 ] = TNULL;
				}
			}
			else
			{
				if ( pTurret->pDamageSparksFX != TNULL && pTurret->apDamageFX[ 1 ] == TNULL )
					pTurret->apDamageFX[ 1 ] = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pTurret->pDamageSparksFX );

				if ( pTurret->apDamageFX[ 1 ] != TNULL )
				{
					TMatrix44 mBone;
					m_pModelComponent->GetModelInstance()->GetBoneTransform( pTurret->iDamage50Bone, mBone );
					pTurret->apDamageFX[ 1 ]->GetTransform() = mBone;
				}
			}

			// 75%: the smoke
			if ( fRatio <= 0.75f )
			{
				if ( pTurret->apDamageFX[ 2 ] != TNULL )
				{
					pTurret->apDamageFX[ 2 ]->StopInstant();
					pTurret->apDamageFX[ 2 ] = TNULL;
				}
			}
			else
			{
				if ( pTurret->pDamageSmokeFX != TNULL && pTurret->apDamageFX[ 2 ] == TNULL )
				{
					pTurret->apDamageFX[ 2 ] = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pTurret->pDamageSmokeFX );
					APositionalSoundManager::GetSingleton()->AddEntitySound( this, 0x1E, 0x13F );
				}

				if ( pTurret->apDamageFX[ 2 ] != TNULL )
				{
					TMatrix44 mBone;
					m_pModelComponent->GetModelInstance()->GetBoneTransform( pTurret->iDamage75Bone, mBone );
					pTurret->apDamageFX[ 2 ]->GetTransform() = mBone;
				}
			}
		}

		// Back to the idle animation once the turret animation is over
		if ( m_iTurretAnim != pTurret->iIdleAnim )
		{
			if ( m_pModelComponent->GetModelInstance()->GetAnimInterface()->IsAnimationFinished( m_iTurretAnim ) )
				PlayTurretAnim( pTurret->iIdleAnim, 0.0f, 1.0f, 1.0f );
		}

		if ( !( TUINT8( m_uiCarFlags >> 2 ) & 1 ) )
		{
			// The helper fires
			if ( pTurret->pHelper->Tick( a_fDeltaTime ) )
			{
				AProjectile* pProjectile = AProjectileManager::GetSingleton()->CreateProjectile( AProjectileTypes::PROJECTILETYPES( 0 ) );

				if ( pProjectile != TNULL )
				{
					TMatrix44 mBone;
					m_pModelComponent->GetModelInstance()->GetBoneTransform( pTurret->iFireBone, mBone );

					TVector4 vAim = pTurret->pHelper->m_vAim;
					pProjectile->Activate( this, mBone.AsBasisVector4( BASISVECTOR_TRANSLATION ), vAim, 0x2F );
					pProjectile->m_uiAttackType = 0xC;

					AToshiAnimInterface2* pI = m_pModelComponent->GetModelInstance()->GetAnimInterface();

					vAim.Normalise();

					// The fire animation of the side the blob is on
					TINT16 iFireAnim;
					TFLOAT fForward = vAim.x * m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ).x + vAim.y * m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ).y +
					    vAim.z * m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ).z;

					if ( fForward <= 0.707f )
					{
						if ( fForward >= -0.707f )
						{
							TFLOAT fRight = vAim.x * m_oMatrix.AsBasisVector4( BASISVECTOR_RIGHT ).x + vAim.y * m_oMatrix.AsBasisVector4( BASISVECTOR_RIGHT ).y +
							    vAim.z * m_oMatrix.AsBasisVector4( BASISVECTOR_RIGHT ).z;

							iFireAnim = ( fRight <= 0.0f ) ? pTurret->iFireAnimLeft : pTurret->iFireAnimRight;
						}
						else
						{
							iFireAnim = pTurret->iFireAnimBehind;
						}
					}
					else
					{
						iFireAnim = pTurret->iFireAnimFront;
					}

					TASSERT( iFireAnim >= 0 );
					PlayTurretAnim( iFireAnim, 0.0f, 1.0f, 1.0f );

					if ( TUINT8( m_uiCarFlags >> 7 ) & 1 )
					{
						TASSERT( m_pTurretData->iReloadAnim >= 0 );
						TASSERT( pI->GetSkeletonInstance() != TNULL && pI->GetSkeletonInstance()->IsAnimating( m_pTurretData->iReloadAnim ) != TNULL );

						pI->StopAnim( m_pTurretData->iReloadAnim, 0.15f );
						m_uiCarFlags &= ~0x80;
					}

					TASSERT( m_pTurretData->iRecoilAnim >= 0 );
					pI->AddAnim( m_pTurretData->iRecoilAnim, 0.15f, 1.0f, 1.0f );
					m_uiCarFlags |= 0x40;

					pTurret->pHelper->OnFire( pProjectile );
				}
			}

			// Drives or waits for a target
			ATurretHelper* pHelper = pTurret->pHelper;
			TBOOL          bAim    = TFALSE;

			if ( m_uiCarFlags & 1 )
			{
				if ( pHelper->m_pTarget == TNULL )
				{
					if ( m_uiCarFlags & 2 )
						StopTurning();

					if ( pTurret->fResumeTimer <= TSTATICCAST( const CarData, m_pTypeData->m_pTRBData )->m_fResumeDelay )
					{
						pTurret->fResumeTimer = pTurret->fResumeTimer + a_fDeltaTime;
					}
					else
					{
						m_fSpeed = pTurret->fSavedSpeed;
						m_uiCarFlags &= ~1;
					}
				}
				else
				{
					pTurret->fResumeTimer = 0.0f;

					if ( pHelper->m_pTarget->GetSize() < 9 && !( pHelper->m_uiFlags & ATurretHelper::TURRETFLAG_Turning ) )
					{
						if ( m_uiCarFlags & 2 )
							StopTurning();
					}
					else
					{
						bAim = TTRUE;
					}
				}
			}
			else if ( pHelper->m_pTarget != TNULL )
			{
				pTurret->fSavedSpeed  = m_fSpeed;
				m_fSpeed              = 0.0f;
				pTurret->fResumeTimer = 0.0f;
				m_uiCarFlags |= 1;

				if ( pHelper->m_pTarget->GetSize() >= 9 )
					bAim = TTRUE;
			}

			if ( bAim )
			{
				const TVector4& rTarget = pHelper->m_pTarget->GetTranslation();
				FaceTarget( rTarget.x, rTarget.y, rTarget.z, rTarget.w );
			}

			pTurret->pVAnim->SetTime( pTurret->pHelper->m_fPitchFrac * pTurret->fVAnimDuration );
			pTurret->pHAnim->SetTime( pTurret->pHelper->m_fAngleFrac * pTurret->fHAnimDuration );

			TMatrix44 mBone;
			m_pModelComponent->GetModelInstance()->GetBoneTransform( pTurret->iFireBone, mBone );
			pTurret->pHelper->m_vLookAt = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
		}
	}

	if ( m_pModelsAttached != TNULL )
	{
		m_pModelsAttached->Tick( a_fDeltaTime );

		if ( m_eCarType == VEHICLETYPE_TANK )
		{
			// The turret's reload and recoil animations
			AToshiAnimInterface2* pI = m_pModelComponent->GetModelInstance()->GetAnimInterface();

			if ( TUINT8( m_uiCarFlags >> 7 ) & 1 )
			{
				TASSERT( m_pTurretData->iReloadAnim >= 0 );
				TASSERT( pI->GetSkeletonInstance() != TNULL && pI->GetSkeletonInstance()->IsAnimating( m_pTurretData->iReloadAnim ) != TNULL );

				if ( pI->IsAnimationFinished( m_pTurretData->iReloadAnim ) )
				{
					pI->StopAnim( m_pTurretData->iReloadAnim, 0.0f );
					m_uiCarFlags &= ~0x80;
				}
			}

			if ( TUINT8( m_uiCarFlags >> 6 ) & 1 )
			{
				TASSERT( m_pTurretData->iRecoilAnim >= 0 );
				TASSERT( pI->GetSkeletonInstance() != TNULL && pI->GetSkeletonInstance()->IsAnimating( m_pTurretData->iRecoilAnim ) != TNULL );

				if ( pI->IsAnimationFinished( m_pTurretData->iRecoilAnim ) )
				{
					pI->StopAnim( m_pTurretData->iRecoilAnim, 0.0f );
					m_uiCarFlags &= ~0x40;

					TFLOAT fRechargeTime = m_pTurretData->pHelper->GetCurrentTimer();
					TASSERT( fRechargeTime > 0 );

					TASSERT( m_pTurretData->iReloadAnim >= 0 );
					TFLOAT fAnimLength = pI->GetAnimationLength( m_pTurretData->iReloadAnim );
					TASSERT( fAnimLength > 0 );

					pI->AddAnim( m_pTurretData->iReloadAnim, 0.0f, 1.0f, fAnimLength / fRechargeTime );
					m_uiCarFlags |= 0x80;
				}
			}
		}
	}
}

// $deBlob: FUNCTION 00777ac0
void ACar::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( TINT( a_rPacket.m_eMessage ) )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
			if ( m_eCarType == VEHICLETYPE_TRAIN_PRISONCAR && !( m_uiCarFlags & CARFLAG_BlimpColoured ) )
			{
				// The train starts
				ToggleTrainColoured();
			}

			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
		{
			// Touched by Blob: painted (car colours, the paint tanker...)

			if ( !( a_pCaller->GetFlags() & FLAG_Player ) )
				break;

			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );
			TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );

			if ( m_eCarType <= 1 && pBlob->GetSize() > 2 )
			{
				// A big blob pushes the car down
				TVector4 vVelocity = TVector4::VEC_NEGY;
				vVelocity.Multiply( 5.0f );
				TPhysicsManager::SetLinearVelocity( pBlob->m_pBody, vVelocity );
			}

			if ( !( m_uiPropFlags & PROPFLAG_Paintable ) )
				break;

			TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

			if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
				uiColour = ABlobColourComponent::COLOUR_BLACK;

			if ( m_eCarType == VEHICLETYPE_CAR )
			{
				if ( uiColour != 0 )
				{
					if ( !( uiColour & ABlobColourComponent::COLOUR_BLACK ) )
					{
						if ( !( m_uiCarFlags & CARFLAG_BlimpColoured ) )
						{
							if ( TINT( pBlob->RemovePaintPoints( 1, TFALSE ) ) <= 0 )
							{
								ToggleCarColoured();

								AModelInstance2* pModelInstance = m_pModelComponent->GetModelInstance();
								pModelInstance->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
								OnPainted( pBlob, uiColour );
								pModelInstance->SetMeshFlags( 0, 1, TTRUE );
								pModelInstance->SetMeshColour( 0, ABlobColourMaps::ms_pEnemyBodyColourMap[ ABlobColourComponent::GetRandomColour() ] );
							}
						}
					}
					else if ( m_uiCarFlags & CARFLAG_BlimpColoured )
					{
						ToggleCarColoured();
					}
				}

				return;
			}

			if ( m_eCarType == VEHICLETYPE_BLIMP )
			{
				if ( uiColour != 0 )
				{
					if ( !( uiColour & ABlobColourComponent::COLOUR_BLACK ) )
					{
						if ( !( m_uiCarFlags & CARFLAG_BlimpColoured ) )
						{
							if ( TINT( pBlob->RemovePaintPoints( 1, TFALSE ) ) <= 0 )
								SetBlimpColoured( TTRUE );
						}
					}
					else if ( m_uiCarFlags & CARFLAG_BlimpColoured )
					{
						SetBlimpColoured( TFALSE );
					}
				}

				return;
			}

			if ( uiColour != 0 && m_pModelComponent->GetModelInstance()->GetLocalAmbientColour() != ABlobColourMaps::ms_pEnemyBodyColourMap[ uiColour ] )
			{
				if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
				{
					ResetAmbientColour( m_pModelComponent->GetModelInstance() );
					APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
					return;
				}

				if ( TINT( pBlob->RemovePaintPoints( 1, TFALSE ) ) <= 0 )
				{
					OnPainted( pBlob, uiColour );
					APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
					return;
				}
			}

			break;
		}
		case AEntityIOPacket::MESSAGE_SLAM:
		{
			// Attacked: the tank's damage, the paint tanker's tanks
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

			if ( ( m_Flags & FLAG_Dead ) || ( pBlob->GetBlobFlags() & 0x8000 ) )
				break;

			if ( m_eCarType == VEHICLETYPE_TANK )
			{
				ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 30 ), ASoundMap::EVENTID( 315 ), GetTranslation(), FMOD_EVENT_MODE( 0 ) );

				TINT iPoints = TINT( pBlob->RemovePaintPoints( m_iLife, TFALSE ) );

				if ( iPoints >= 1 )
				{
					m_iLife = TINT16( iPoints );
					PlayTurretAnim( m_pTurretData->iHitAnim, 0.0f, 1.0f, 1.0f );

					AAttackMessage oAttack( AAttackMessage::ATTACK_SlamFailedRecoil, GetTranslation(), 0, 3, TVector4::VEC_NEGY, TFALSE );

					AEntityIOPacket oPacket;
					oPacket.m_eMessage = AEntityIOPacket::MESSAGE_RECOIL; // NOTE: 0x0A is not named in AEntityIOPacket yet
					oPacket.m_iData    = TREINTERPRETCAST( TINT, &oAttack );

					pBlob->HandleEntityIO( this, oPacket );
					APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
				}
				else
				{
					if ( m_uiCarFlags & CARFLAG_BlimpColoured )
					{
						// The ATankSpawner that made the tank is told
						TSTATICCAST( ATankSpawner, m_pTurretData->pSpawner )->OnTankDestroyed();
					}

					Explode( TTRUE );
					APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
				}

				return;
			}

			if ( m_eCarType != VEHICLETYPE_PAINTTANKER )
				break;

			switch ( m_uiTankerStage )
			{
				case 0:
					m_pModelsAttached->SetModelVisible( 2, TFALSE );
					m_pModelsAttached->SetModelVisible( 3, TTRUE );
					m_fSpeed = 0.0f;
					OnTankerHit( pBlob, 4 );
					m_uiTankerStage++;
					m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( 15 << 8 ) );
					ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 34 ), GetTranslation(), FMOD_EVENT_MODE( 0 ) );
					APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
					return;
				case 1:
					m_pModelsAttached->SetModelVisible( 4, TFALSE );
					m_pModelsAttached->SetModelVisible( 5, TTRUE );
					OnTankerHit( pBlob, 2 );
					m_uiTankerStage++;
					ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 34 ), GetTranslation(), FMOD_EVENT_MODE( 0 ) );
					APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
					return;
				case 2:
					OnTankerHit( pBlob, 1 );
					m_pModelsAttached->SetModelVisible( 6, TFALSE );
					m_pModelsAttached->SetModelVisible( 7, TTRUE );

					TASSERT( pBlob->IsA( &TGetClass( ABlobEntity ) ) );

					ApplyBlobColour( pBlob, m_pModelsAttached->GetModelInstance( 0 ) );
					ApplyBlobColour( pBlob, m_pModelsAttached->GetModelInstance( 3 ) );
					ApplyBlobColour( pBlob, m_pModelsAttached->GetModelInstance( 5 ) );
					ApplyBlobColour( pBlob, m_pModelsAttached->GetModelInstance( 7 ) );
					m_uiTankerStage++;
					Explode( TTRUE );
					pBlob->StartRainbow();
					m_uiTankerStage++;
					APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
					return;
				default:
					break;
			}

			break;
		}
		case AEntityIOPacket::MESSAGE_CHALLENGERESET:
			if ( m_eCarType == VEHICLETYPE_TANK )
			{
				Reset( TNULL );
				StartFromCurrentNode();
				APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
				return;
			}

			break;
		case AEntityIOPacket::MESSAGE_CHALLENGESTART:
			if ( m_eCarType == VEHICLETYPE_TANK )
			{
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
				oPacket.m_iData    = 0;

				APropPlatform::HandleEntityIO( a_pCaller, oPacket );
				return;
			}

			break;
		case AEntityIOPacket::MESSAGE_PROJECTILEHIT:
		{
			// The turret's ink splat on the ground: m_iData points at a TVector4 (+0x18) of the sender, the ray is 2 units
			// along +Y on the layer 0x81. NOTE: the sender's type is not named yet
			TVector4* pFrom = TREINTERPRETCAST( TVector4*, TREINTERPRETCAST( TUINT8*, a_rPacket.m_iData ) + 0x18 );

			TVector4 vOffset;
			vOffset.Multiply( TVector4::VEC_POSY, 2.0f );

			TPhysicsRay oRay;
			oRay.m_vFrom            = *pFrom;
			oRay.m_vTo              = oRay.m_vFrom + vOffset;
			oRay.m_uiCollisionLayer = 0x81;

			TPhysicsShapeContact oContact;

			if ( TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) )
			{
				TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pTurretData->pInkySplatFX );

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

			if ( m_pTurretData != TNULL )
			{
				// NOTE: retail also calls an empty stub here
				APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
				return;
			}

			break;
		}
	}

	APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00778210
void ACar::Explode( TBOOL a_bSpawn )
{
	if ( m_eCarType != VEHICLETYPE_TANK || !( TUINT8( m_uiCarFlags >> 3 ) & 1 ) )
	{
		SetVisible( TFALSE );
		SetHash( TFALSE );
		RemoveFromPhysics();
		SetTicking( TFALSE );
		m_Flags |= FLAG_DontTick;
	}

	TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pExplosionFX );

	if ( pEffect != TNULL )
	{
		TVector4 vPosition = GetBasePosition();

		TMatrix44 mTransform = TMatrix44::IDENTITY;
		mTransform.SetTranslation( vPosition );
		pEffect->GetTransform() = mTransform;

		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 33 ), vPosition, FMOD_EVENT_MODE( 0 ) );
	}

	m_Flags |= FLAG_Dead;
	m_iLife = 0;

	if ( m_pTurretData != TNULL && m_pTurretData->pHelper != TNULL )
		m_pTurretData->pHelper->StopEffects();

	StopIdleSound();
	StopMovingSound();

	ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 10, m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), 1.0f, -1 );

	APositionalSoundManager::GetSingleton()->RemoveSound( this, 30, 319 );

	TASSERT( !IsPooled() );
	AChallengeManager::GetSingleton()->OnCombatEvent( TREINTERPRETCAST( void*, TUINTPTR( GetID() ) ) );

	if ( m_eCarType == VEHICLETYPE_TANK )
	{
		if ( a_bSpawn && AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 )
		{
			// A pickup near the first player
			APickupManager::SpawnPickupNearPlayer( 1, 0 );
		}

		if ( TUINT8( m_uiCarFlags >> 3 ) & 1 )
			RestartFromStart();
		else
			m_pTurretData->pHelper->SetState( 3 );

		for ( TINT i = 0; i < TARRAYSIZE( m_pTurretData->apDamageFX ); i++ )
		{
			if ( m_pTurretData->apDamageFX[ i ] != TNULL )
			{
				m_pTurretData->apDamageFX[ i ]->StopInstant();
				m_pTurretData->apDamageFX[ i ] = TNULL;
			}
		}
	}
}

// $deBlob: FUNCTION 007784a0
void ACar::OnLODChanging( TINT a_iLOD )
{
	if ( m_pModelsAttached != TNULL && !( a_iLOD & FLAG_HighLOD ) )
	{
		m_pModelsAttached->SetVisible( TFALSE );

		if ( ( a_iLOD & FLAG_LowLOD ) && m_pTurretData != TNULL )
		{
			if ( m_pTurretData->pHelper != TNULL )
			{
				// The turret stops targetting
				StopTurretSounds();
				m_pTurretData->pHelper->ReleaseTarget();
			}

			for ( TINT i = 0; i < TARRAYSIZE( m_pTurretData->apDamageFX ); i++ )
			{
				if ( m_pTurretData->apDamageFX[ i ] != TNULL )
				{
					m_pTurretData->apDamageFX[ i ]->StopInstant();
					m_pTurretData->apDamageFX[ i ] = TNULL;
				}
			}
		}
	}

	APropPlatform::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 00778570
void ACar::SetVisible( TBOOL a_bVisible )
{
	if ( m_pModelsAttached != TNULL )
		m_pModelsAttached->SetVisible( a_bVisible );

	APropPlatform::SetVisible( a_bVisible );
}

// $deBlob: FUNCTION 007785a0
void ACar::Reset( ABaseEntity* a_pEntity )
{
	SetHash( TTRUE );
	m_Flags &= ~FLAG_DontTick;

	APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x29, 0x1C9 );

	if ( m_eCarType == VEHICLETYPE_CAR && ( TUINT8( m_uiCarFlags >> 3 ) & 1 ) )
	{
		SetMovingSound( 0x29, 0x1CC );
		ToggleCarColoured();
	}
	else if ( m_eCarType == VEHICLETYPE_TRAIN_PRISONCAR && ( TUINT8( m_uiCarFlags >> 3 ) & 1 ) )
	{
		ToggleTrainColoured();
	}
	else if ( m_eCarType == VEHICLETYPE_BLIMP && ( TUINT8( m_uiCarFlags >> 3 ) & 1 ) )
	{
		SetBlimpColoured( TFALSE );
	}

	ResetAmbientColour( m_pModelComponent->GetModelInstance() );
	m_uiTankerStage = 0;
	APropPlatform::Reset( a_pEntity );

	if ( m_pModelsAttached != TNULL )
		m_pModelsAttached->ShowAllModels();

	m_uiCarFlags |= CARFLAG_Waiting;

	if ( m_eCarType == VEHICLETYPE_TANK )
	{
		m_pTurretData->pHelper->Reset();
		m_uiCarFlags &= 0xFFF8;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1E, 0x13F );

		AToshiAnimInterface2* pI = m_pModelComponent->GetModelInstance()->GetAnimInterface();

		if ( TUINT8( m_uiCarFlags >> 7 ) & 1 )
		{
			TASSERT( m_pTurretData->iReloadAnim >= 0 );
			TASSERT( pI->GetSkeletonInstance() != TNULL && pI->GetSkeletonInstance()->IsAnimating( m_pTurretData->iReloadAnim ) != TNULL );

			if ( pI->IsAnimationFinished( m_pTurretData->iReloadAnim ) )
			{
				pI->StopAnim( m_pTurretData->iReloadAnim, 0.0f );
				m_uiCarFlags &= ~0x80;
			}
		}

		if ( TUINT8( m_uiCarFlags >> 6 ) & 1 )
		{
			TASSERT( m_pTurretData->iRecoilAnim >= 0 );
			TASSERT( pI->GetSkeletonInstance() != TNULL && pI->GetSkeletonInstance()->IsAnimating( m_pTurretData->iRecoilAnim ) != TNULL );

			if ( pI->IsAnimationFinished( m_pTurretData->iRecoilAnim ) )
			{
				pI->StopAnim( m_pTurretData->iRecoilAnim, 0.0f );
				m_uiCarFlags &= ~0x40;
			}
		}

		for ( TINT i = 0; i < 3; i++ )
		{
			if ( m_pTurretData->apDamageFX[ i ] != TNULL )
			{
				m_pTurretData->apDamageFX[ i ]->StopInstant();
				m_pTurretData->apDamageFX[ i ] = TNULL;
			}
		}
	}
	else if ( m_eCarType == VEHICLETYPE_PAINTTANKER )
	{
		m_pModelsAttached->SetModelVisible( 3, TFALSE );
		m_pModelsAttached->SetModelVisible( 5, TFALSE );
		m_pModelsAttached->SetModelVisible( 7, TFALSE );
		ResetAmbientColour( m_pModelsAttached->GetModelInstance( 0 ) );
		ResetAmbientColour( m_pModelsAttached->GetModelInstance( 3 ) );
		ResetAmbientColour( m_pModelsAttached->GetModelInstance( 5 ) );
		ResetAmbientColour( m_pModelsAttached->GetModelInstance( 7 ) );
		m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( 0x0E << 8 ) );
	}

	if ( !( TUINT8( m_uiCarFlags >> 3 ) & 1 ) )
	{
		TINT iMaxLife = GetMaxLife();
		m_Flags &= ~0x400000;
		m_iLife = TINT16( iMaxLife );
	}
}

// $deBlob: FUNCTION 00778980
void ACar::SetTankSpawner( ABaseEntity* a_pSpawner )
{
	TASSERT( GetCarType() == VEHICLETYPE_TANK );

	if ( a_pSpawner == TNULL )
		m_uiCarFlags &= ~CARFLAG_BlimpColoured;
	else
		m_uiCarFlags |= CARFLAG_BlimpColoured;

	m_pTurretData->pSpawner = a_pSpawner;

	if ( m_uiCarFlags == CARFLAG_TurretOff )
	{
		SetIdleSound( -1, -1 );
		SetMovingSound( -1, -1 );
	}
}

// $deBlob: FUNCTION 00778a10
void ACar::StartNodeDelay( TINT16 a_iNode )
{
	APropPlatform::StartNodeDelay( a_iNode );

	if ( m_uiTankerStage != 0 )
		m_uiTankerStage--;
}

// $deBlob: FUNCTION 00778a40
void ACar::SetTurretActive( TBOOL a_bActive )
{
	if ( !a_bActive )
		m_uiCarFlags |= CARFLAG_TurretOff;
	else
		m_uiCarFlags &= ~CARFLAG_TurretOff;

	// The turret's helper goes to state 0 (a_bActive) or 3
	if ( m_pTurretData != TNULL && m_pTurretData->pHelper != TNULL )
		m_pTurretData->pHelper->SetState( a_bActive ? 0 : 3 );

	TUINT8 uiMin = ( ( m_uiPlatformFlags >> 5 ) & 1 ) * 2 + 2;
	if ( m_uiTankerStage < uiMin )
		m_uiTankerStage = uiMin;
}

// $deBlob: FUNCTION 00778ac0
void ACar::StartFromCurrentNode()
{
	if ( m_eCarType == VEHICLETYPE_TANK )
	{
		if ( m_uiCarFlags & 1 )
		{
			if ( m_uiCarFlags & 2 )
				StopTurning();

			m_fSpeed = m_pTurretData->fSavedSpeed;
			m_uiCarFlags &= ~1;
		}

		APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x1E, 0x13F );
		m_uiCarFlags |= CARFLAG_Waiting;
		m_pTurretData->pHelper->SetState( 3 );
		m_pTurretData->pHelper->Reset();
		m_pTurretData->pVAnim->SetTime( m_pTurretData->fVAnimDuration * m_pTurretData->pHelper->m_fPitchFrac );
		m_pTurretData->pHAnim->SetTime( m_pTurretData->fHAnimDuration * m_pTurretData->pHelper->m_fAngleFrac );

		for ( TINT i = 0; i < TARRAYSIZE( m_pTurretData->apDamageFX ); i++ )
		{
			if ( m_pTurretData->apDamageFX[ i ] != TNULL )
			{
				m_pTurretData->apDamageFX[ i ]->StopInstant();
				m_pTurretData->apDamageFX[ i ] = TNULL;
			}
		}
	}

	if ( ( m_uiCarFlags & CARFLAG_BlimpColoured ) == 0 )
	{
		TINT iMaxLife = GetMaxLife();
		m_Flags &= ~0x400000;
		m_iLife = TINT16( iMaxLife );
	}
	else if ( m_uiCarFlags & CARFLAG_Waiting )
	{
		SetVisible( TFALSE );
	}

	APropPlatform::StartFromCurrentNode();
}

// $deBlob: FUNCTION 00778c30
void ACar::OnStartMoving()
{
	if ( TUINT8( m_uiCarFlags >> 3 ) & 1 )
		SetVisible( TTRUE );

	m_uiCarFlags &= ~CARFLAG_Waiting;

	if ( m_eCarType == VEHICLETYPE_TANK )
	{
		SetMovingSound( 0x1D, 0x135 );
		SetIdleSound( 0x1D, 0x136 );
		PlayMovingSound();
	}
}

// $deBlob: FUNCTION 00778c90
void ACar::FaceTarget( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW )
{
	TASSERT( VEHICLETYPE_TANK == m_eCarType );

	m_uiCarFlags |= 2;
	m_fSpeed = 5.0f / m_fPathLength;

	const TVector4& rPosition = GetTranslation();
	TVector4        vDirection( a_fX - rPosition.x, a_fY - rPosition.y, a_fZ - rPosition.z, a_fW );
	vDirection.Normalise();

	const TVector4& rForward = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	TFLOAT          fDot     = rForward.y * vDirection.y + rForward.x * vDirection.x + rForward.z * vDirection.z;

	if ( fDot <= 0.1f || m_uiTankerStage != 0 )
	{
		if ( -0.1f <= fDot && fDot <= 0.1f )
		{
			StopTurning();
			return;
		}

		if ( TUINT8( m_uiPlatformFlags >> 5 ) & 1 )
			m_uiPlatformFlags &= ~PLATFORMFLAG_Backwards;
	}
	else if ( !( TUINT8( m_uiPlatformFlags >> 5 ) & 1 ) )
	{
		m_uiPlatformFlags |= PLATFORMFLAG_Backwards;
	}
}

// $deBlob: FUNCTION 00778dd0
void ACar::StopTurning()
{
	TASSERT( VEHICLETYPE_TANK == m_eCarType );

	m_uiCarFlags &= ~2;
	m_fSpeed = 0.0f;

	if ( TUINT8( m_uiPlatformFlags >> 5 ) & 1 )
		m_uiPlatformFlags &= ~PLATFORMFLAG_Backwards;
}

// $deBlob: FUNCTION 00778e40
void ACar::OnTankerHit( ABlobEntity* a_pBlob, TUINT32 a_uiTank )
{
	if ( !( a_pBlob->GetColourComponent()->GetCurrentColour() & ABlobColourComponent::COLOUR_BLACK ) )
	{
		a_pBlob->ChangeColour( a_uiTank );
		a_pBlob->SetPaintPoints( ( a_pBlob->GetPaintPoints() + 10 > 100 ) ? 100 : TUINT32( a_pBlob->GetPaintPoints() + 10 ) );
	}

	// The destroy effect of the colour (the LeechBots' effects)
	TParticleEffect* pEffect = ALeechBot2::ms_apPrimaryDestroyEffects[ 0 ];

	if ( a_uiTank - 1 > 6 )
	{
		if ( a_uiTank - 8 <= 7 )
			pEffect = ALeechBot2::ms_apSecondaryDestroyEffects[ a_uiTank - 8 ];
	}
	else
	{
		pEffect = ALeechBot2::ms_apPrimaryDestroyEffects[ a_uiTank - 1 ];
	}

	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pEffect );

	if ( pInstance != TNULL )
	{
		TVector4 vOffset;
		vOffset.Multiply( TVector4::VEC_POSZ, 2.5f );

		TVector4 vRotated;
		TMatrix44::RotateVector( vRotated, m_pPhysicsData->m_oMatrix, vOffset );

		TMatrix44 mTransform = TMatrix44::IDENTITY;
		mTransform.SetTranslation( TVector4( GetTranslation().x + vRotated.x, GetTranslation().y + vRotated.y, GetTranslation().z + vRotated.z, vRotated.w ) );
		pInstance->GetTransform() = mTransform;
	}
}

// $deBlob: FUNCTION 00778fd0
void ACar::StopTurretSounds()
{
	if ( m_pTurretData != TNULL && m_pTurretData->pHelper != TNULL )
		m_pTurretData->pHelper->StopEffects();
}

// $deBlob: FUNCTION 00778ff0
void ACar::OnPainted( ABlobEntity* a_pBlob, TUINT32 a_uiColour )
{
	TASSERT( !IsPooled() );
	AChallengeManager::GetSingleton()->OnCarEvent( TREINTERPRETCAST( void*, TUINTPTR( GetID() ) ) );

	AVibrationManager::VibrateOnce( APlayerManager::GetSingleton()->GetPlayerIndex( a_pBlob ), 4 );

	AParticleEffectHelper::SetColour( ms_pCollisionFX, ABlobColourComponent::ms_auiColours[ a_uiColour ], TNULL );

	TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ms_pCollisionFX );

	if ( pEffect != TNULL )
	{
		TMatrix44 mTransform = TMatrix44::IDENTITY;
		mTransform.SetTranslation( m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) );
		pEffect->GetTransform() = mTransform;
	}

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 )
	{
		const TVector4& rPosition = GetTranslation();
		TVector4        vPosition( rPosition.x, rPosition.y - m_vBBox.y * 2.0f, rPosition.z, rPosition.w );
		AGameStatus::GetSingleton()->AddBonusScore( 0, TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x78 / 4 ], &vPosition );
	}
}


// $deBlob: FUNCTION 00779860
Toshi::TVector4 ACar::GetTargetOffset()
{
	TVector4 vResult;

	if ( m_eCarType == VEHICLETYPE_TANK )
	{
		vResult.Set( 0.0f, -m_vBBox.y, 0.0f, 1.0f );
		return vResult;
	}

	if ( m_eCarType == VEHICLETYPE_PAINTTANKER )
	{
		TFLOAT fScale;

		if ( m_uiTankerStage == 0 )
		{
			vResult.Set( 0.0f, -m_vBBox.y, 0.0f, 1.0f );
			return vResult;
		}
		else if ( m_uiTankerStage == 1 )
			fScale = -6.0f;
		else if ( m_uiTankerStage == 2 )
			fScale = -12.0f;
		else
			return TVector4::VEC_ZERO;

		vResult.Multiply( m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), fScale );
		vResult.w = 1.0f;
		vResult.y = vResult.y - m_vBBox.y;
		return vResult;
	}

	return TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 00779180
void ACar::ToggleCarColoured()
{
	TASSERT( VEHICLETYPE_CAR == m_eCarType );

	StopMovingSound();

	if ( !( TUINT8( m_uiCarFlags >> 3 ) & 1 ) )
	{
		if ( 0.0f < m_vVelocity.MagnitudeSq() )
		{
			APositionalSoundManager::GetSingleton()->AddMovingEntitySound( this, 0x29, 0x1C9 );
		}

		SetMovingSound( 0x29, 0x1CB );
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x29 ), ASoundMap::EVENTID( 0x1CD ), GetMatrix().GetTranslation(), 0 );
	}
	else
	{
		m_pModelComponent->GetModelInstance()->SetMeshColour( 0, TUINT32( 0 ) );
		SetMovingSound( 0x29, 0x1CC );
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x29 ), ASoundMap::EVENTID( 0x1CA ), GetMatrix().GetTranslation(), 0 );

		APositionalSoundManager::GetSingleton()->RemoveSound( this, 0x29, 0x1C9 );
	}

	if ( 0.0f < m_vVelocity.MagnitudeSq() )
		PlayMovingSound();

	AEntityModelComponent* pOldModel = m_pModelComponent;
	m_pModelComponent                = m_pExtraModel;
	m_pExtraModel                    = pOldModel;

	m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, pOldModel->GetModelInstance()->GetFlags() & AModelInstance2::FLAG_Visible );
	m_pExtraModel->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	m_pModelComponent->SetTransform( GetMatrix() );

	m_uiCarFlags ^= CARFLAG_BlimpColoured;
}

// $deBlob: FUNCTION 007792f0
void ACar::ToggleTrainColoured()
{
	TASSERT( VEHICLETYPE_TRAIN_PRISONCAR == m_eCarType );

	AEntityModelComponent* pOldModel = m_pModelComponent;
	m_pModelComponent                = m_pExtraModel;
	m_pExtraModel                    = pOldModel;

	m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, pOldModel->GetModelInstance()->GetFlags() & AModelInstance2::FLAG_Visible );
	m_pExtraModel->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	m_pModelComponent->SetTransform( GetMatrix() );

	m_uiCarFlags ^= CARFLAG_BlimpColoured;
}

// $deBlob: FUNCTION 00779390
void ACar::SetBlimpColoured( TBOOL a_bColoured )
{
	TASSERT( VEHICLETYPE_BLIMP == m_eCarType );
	TASSERT( HASANYFLAG( m_uiCarFlags, CARFLAG_BlimpColoured ) != a_bColoured );

	// The texture of the model's first mesh: "blimb_flying_coloured.tga" or "blimb_flying.tga"
	T2ModelPtr pModel = m_pModelComponent->GetModelInstance()->GetModelInstance()->GetModel();
	TModelLOD* pLOD   = &pModel->GetLOD( 0 );

	ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ 0 ]->GetMaterial() );
	TTexture*         pTexture = TTextureManager::GetSingleton()->FindTexture( a_bColoured ? "blimb_flying_coloured.tga" : "blimb_flying.tga" );

	if ( a_bColoured )
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x22 ), ASoundMap::EVENTID( 0x18A ), GetMatrix().GetTranslation(), 0 );

	pSkinMat->SetTexture( 0, pTexture );

	if ( !a_bColoured )
		m_uiCarFlags &= ~CARFLAG_BlimpColoured;
	else
		m_uiCarFlags |= CARFLAG_BlimpColoured;

	// The level's colour progress (AGameStatus): the HUD is told that a colour thing was painted
	if ( AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
		AGameStatus::GetSingleton()->GetLevelData().OnBlimpPainted();
}

// The tank's moving and idle sounds
// $deBlob: FUNCTION 00779530
void ACar::SetTankSounds()
{
	SetMovingSound( 0x1D, 0x135 );
	SetIdleSound( 0x1D, 0x136 );
}

// $deBlob: FUNCTION 00779550
Toshi::TAnimation* ACar::PlayTurretAnim( TINT16 a_iAnim, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed )
{
	if ( m_iTurretAnim >= 0 )
		m_pModelComponent->GetModelInstance()->GetAnimInterface()->StopAnim( m_iTurretAnim, a_fBlendInSpeed );

	m_iTurretAnim = a_iAnim;

	if ( a_iAnim >= 0 )
		return m_pModelComponent->GetModelInstance()->GetAnimInterface()->AddAnim( m_iTurretAnim, a_fBlendInSpeed, a_fWeight, a_fSpeed );

	return TNULL;
}

// The animation a_iSequence of the model has finished
// $deBlob: FUNCTION 007795e0
TBOOL ACar::IsAnimationFinished( TINT16 a_iSequence )
{
	return TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->IsAnimationFinished( a_iSequence );
}
