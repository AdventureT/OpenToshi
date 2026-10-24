#include "pch.h"
#include "ACiv.h"
#include "ACivSpawner.h"
#include "AEntityPhysicsMover.h"
#include <Physics/TCapsuleData.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include "AEntityModelComponent.h"
#include "AEntityPoseController.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AModel2.h"
#include "Entities/AEntityFactory.h"
#include "AEntityFactory.h"
#include "AEntityParams.h"
#include "AEntityData.h"
#include "ARandom.h"
#include "AEntityHash.h"
#include "AEntityIOPacket.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobColourComponent.h"
#include "ABlobEntity/ABlobColourMaps.h"
#include "ABlobEntity/ABlobAnimationComponent.h"
#include "GameInterface/APlayerManager.h"
#include "GameController/AGameStatus.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "Input/AVibrationManager.h"
#include "Challenges/AChallengeManager.h"
#include "Fx/AParticleEffectHelper.h"
#include <TParticleSystem/TParticleEffectInstance.h>
#include <Render/TModel.h>
#include <Render/TSkeleton.h>
#include <Platform/DX11/TTexture_DX11.h>
#include "Platform/DX11/ASkinShader_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Sound/ASoundBreakpointComponent.h"

#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL                           ACiv::ms_bSkinMaterialsSet = TFALSE;
AEntityPoseController::PoseMap* ACiv::ms_apPoseMaps[ 2 ];

TSTATICASSERT( sizeof( AVisibleEntity ) == 0x9C );
TSTATICASSERT( sizeof( AProp ) == 0xB4 );
TSTATICASSERT( sizeof( APropPlatform ) == 0x1F0 );
TSTATICASSERT( sizeof( ACiv ) == 0x204 );

// $deBlob: FUNCTION 0076c300
// $deBlob: FUNCTION 0076c310
// $deBlob: FUNCTION 0076c380
// $deBlob: FUNCTION 0076c3e0
// $deBlob: FUNCTION 0076c3f0
// $deBlob: FUNCTION 005408a0
// $deBlob: FUNCTION 00859ca0
TDEFINE_CLASS( ACiv );

// $deBlob: FUNCTION 0076c400
ACiv::ACiv()
{
	m_uiCivFlags           = 0;
	m_pShellModelComponent = TNULL;
	m_fLaughTimer          = -1.0f;
	m_pShellPopFX          = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_CivShellPop" );
	CreateShadow( 3.0f, 0.75f );
	ms_bSkinMaterialsSet     = TFALSE;
	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( 4 << 8 ) );
}

// $deBlob: FUNCTION 0076c4b0
// $deBlob: FUNCTION 0076dbf0
// $deBlob: FUNCTION 0076dbd9
ACiv::~ACiv()
{
	if ( m_pShellModelComponent )
		delete TSTATICCAST( AEntityModelComponent, m_pShellModelComponent );
}

// $deBlob: FUNCTION 0076c520
void ACiv::LoadPoseMaps()
{
	const AEntityPoseController::PoseMap* pPoseMap = TSTATICCAST( const AEntityPoseController::PoseMap, AEntityData::GetSingleton()->GetPoseMapData( "ACiv_PoseMap_Main" ) );

	for ( TINT i = 0; i < 2; i++ )
		ms_apPoseMaps[ i ] = AEntityPoseController::ClonePoseMap( pPoseMap );
}

// $deBlob: FUNCTION 0076c560
void ACiv::UnloadPoseMaps()
{
	for ( TINT i = 1; i >= 0; i-- )
		AEntityPoseController::DestroyPoseMap( ms_apPoseMaps[ i ] );
}

// $deBlob: FUNCTION 0076c580
// $deBlob: FUNCTION 0076de16
void ACiv::ProcessParams( const AEntityParams& a_rParams )
{
	APropPlatform::ProcessParams( a_rParams );

	TUINT8 uiCivType;

	if ( a_rParams.GetValue( uiCivType, "CivType" ) )
		SetCivType( uiCivType );

	TBOOL bNoGraydian;

	if ( a_rParams.GetValue( bNoGraydian, "No Graydian" ) )
	{
		if ( bNoGraydian )
			m_uiCivFlags |= CIVFLAG_NoGraydian;
		else
			m_uiCivFlags &= ~CIVFLAG_NoGraydian;
	}

	TBOOL bNoLiberate = TFALSE;

	if ( a_rParams.GetValue( bNoLiberate, "No Liberate" ) )
	{
		if ( bNoLiberate )
			m_uiCivFlags |= CIVFLAG_NoLiberate;
		else
			m_uiCivFlags &= ~CIVFLAG_NoLiberate;
	}

	AEntityParam::Collision** ppCollision   = TNULL;
	TBOOL                     bHasCollision = a_rParams.GetCollision( ppCollision );

	if ( bHasCollision )
	{
		CreatePhysics( m_pTypeData->m_pTRBData, *ppCollision, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}

	if ( CanBeLiberated() )
		AGameStatus::GetSingleton()->GetLevelData().AddCivilian();

	CreateCivModel( !bHasCollision );
}

// Becomes a pooled civilian of ACivSpawner (the shell model)
// $deBlob: FUNCTION 0076c6d0
void ACiv::InitPooled( TINT a_eType )
{
	m_uiPropFlags |= 1;
	SetMesh( "model\\chars\\CivShell.trb" );
	m_fSpeed          = GetCivData()->fSpeed;
	m_uiPlatformFlags = ( m_uiPlatformFlags & ~PLATFORMFLAG_Reverse ) | PLATFORMFLAG_Targetable | PLATFORMFLAG_LinearSpeed;
	FinishParams();
	m_uiCivFlags &= ~CIVFLAG_TypeMask;
	CreateCivModel( TTRUE );
	SetBBox( *GetCivData()->pShellBBox );
}

// $deBlob: FUNCTION 0076c740
// $deBlob: FUNCTION 0076de0b
void ACiv::PostCreate()
{
	if ( !( m_uiCivFlags & CIVFLAG_NoGraydian ) && ( m_uiCivFlags & CIVFLAG_TypeMask ) == CIVFLAG_TypeMask )
	{
		// A Graydian: in its shell, coloured at random
		if ( !IsTransformed() )
			Transform( ABlobColourComponent::ms_auiColours[ 0 ], TTRUE );

		TUINT8 uiColour = TUINT8( ABlobColourComponent::GetRandomColour() );

		if ( TUINT8( m_uiCivFlags ) != uiColour )
			SetColour( uiColour );

		SetState( m_pWaypointSet != TNULL ? 4 : 5 );

		TFLOAT fPathLength = m_fPathLength;
		m_fSpeed           = GetCivData()->fSpeed;

		if ( fPathLength != 0.0f )
			m_fSpeed = m_fSpeed / fPathLength;
	}

	APropPlatform::PostCreate();

	if ( !ms_bSkinMaterialsSet )
	{
		// The inky sphere map on the skin materials of the shell's and the civilian's model (their LOD 0)
		T2ModelPtr pShellModel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetModelPtr();
		TModelLOD* pShellLOD   = &pShellModel->GetLOD( 0 );
		T2ModelPtr pCivModel   = TSTATICCAST( AEntityModelComponent, m_pShellModelComponent )->GetModelInstance()->GetModel()->GetModelPtr();
		TModelLOD* pCivLOD     = &pCivModel->GetLOD( 0 );
		TModelLOD* apLODs[ 2 ] = { pShellLOD, pCivLOD };

		TTexture* pSphereMap = TTextureManager::GetSingleton()->FindTexture( "spheremap_inky.tga" );
		pSphereMap->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );

		for ( TINT i = 0; i < 2; i++ )
		{
			TModelLOD* pLOD = apLODs[ i ];

			for ( TINT j = 0; j < pLOD->GetMeshCount(); j++ )
			{
				ASkinMaterialHAL* pSkinMat = TSTATICCAST( ASkinMaterialHAL, pLOD->GetMeshes()[ j ]->GetMaterial() );
				TASSERT( pSkinMat->GetTextureNum() == 1 );

				pSkinMat->SetTextureNum( 2 );
				pSkinMat->SetTevMode( 1, 0xC );
				pSkinMat->SetTexture( 1, pSphereMap );
				pSkinMat->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_ENVMAP );
				pSkinMat->SetBlendMode( 13, 0 );
			}
		}
		ms_bSkinMaterialsSet = TTRUE;
	}
}

// $deBlob: FUNCTION 0076c9d0
void ACiv::Tick( TFLOAT a_fDeltaTime )
{
	APropPlatform::Tick( a_fDeltaTime );

	if ( ( m_uiCivFlags & CIVFLAG_Laughing ) && 0.0f < m_fLaughTimer )
	{
		m_fLaughTimer -= a_fDeltaTime;

		if ( m_fLaughTimer <= 0.0f )
		{
			m_fLaughTimer = -1.0f;
			m_uiCivFlags &= ~CIVFLAG_Laughing;

			switch ( GetState() )
			{
				case 4:
				case 5:
				case 0xB:
				case 0xC:
					SetState( 0xC );
			}
		}
	}

	switch ( GetState() )
	{
		case 6:
		case 7:
		case 8:
			if ( HasStateFinished() )
			{
				SetState( 9 );
				return;
			}

			break;
		case 9:
			if ( HasStateFinished() )
			{
				SetState( 1 );
				return;
			}

			break;
		case 10:
			m_fLaughTimer -= a_fDeltaTime;

			if ( m_fLaughTimer < 0.0f )
			{
				if ( TUINT8( m_uiCivFlags ) == ABlobColourComponent::COLOUR_BLACK )
				{
					Explode();
					return;
				}

				SetState( 0xB );
				return;
			}

			break;
		case 0xC:
		{
			// Faces the player
			const TVector4& rBlobPosition = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) )->GetBodyPosition();
			const TVector4& rPosition     = GetMatrix().GetTranslation();

			TVector4 vToBlob( rBlobPosition.x - rPosition.x, rBlobPosition.y - rPosition.y, rBlobPosition.z - rPosition.z, rBlobPosition.w );
			vToBlob.NormaliseXZ();
			SetFacing( vToBlob );
		}
		case 3:
		case 0xB:
			if ( HasStateFinished() )
				ResumeWalking();
	}
}

// $deBlob: FUNCTION 0076cbb0
void ACiv::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			// Liberated by a trigger
			if ( !IsTransformed() )
			{
				const TVector4& rPosition = GetMatrix().GetTranslation();
				AMusicManager::GetSingleton()->PlayMusicSFX( "civilian_liberated", 0x100, &rPosition, TNULL );
				Liberate( ABlobColourComponent::GetRandomColour(), TFALSE );

				if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
				{
					TVector4 vPosition( rPosition.x, rPosition.y - m_vBBox.y * 2.0f, rPosition.z, rPosition.w );
					AGameStatus::GetSingleton()->AddBonusScore( 0, TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x74 / 4 ], &vPosition );
				}
			}
		default:
			APropPlatform::HandleEntityIO( a_pCaller, a_rPacket );
			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
			// Touched by Blob: liberated with Blob's colour
			if ( a_pCaller->GetFlags() & ABaseEntity::FLAG_Player )
			{
				TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
				TUINT32 uiColour = TSTATICCAST( ABlobEntity, a_pCaller )->GetColourComponent()->GetCurrentColour();

				if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
					uiColour = ABlobColourComponent::COLOUR_BLACK;
				else if ( uiColour == 0 )
					return;

				if ( !( uiColour & ABlobColourComponent::COLOUR_BLACK ) )
				{
					if ( !IsTransformed() )
					{
						if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
						{
							// The score and the combo of the player
							AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
							TINT                    iPlayer    = APlayerManager::GetSingleton()->GetPlayerIndex( a_pCaller );

							if ( rLevelData.GetComboState( iPlayer ) == 3 )
								rLevelData.IncrementComboCount( iPlayer );
							else
								rLevelData.StartCombo( iPlayer, 3 );

							// The civ's position (a_pCiv's feet) and the score of BlobScoringProperties+0x74 float above it
							TVector4 vScorePosition( GetMatrix().GetTranslation().x, GetMatrix().GetTranslation().y - m_vBBox.y * 2.0f, GetMatrix().GetTranslation().z, GetMatrix().GetTranslation().w );
							rLevelData.OnCivFreed( iPlayer, &vScorePosition, TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x74 / 4 ] );
						}

						AMusicManager::GetSingleton()->PlayMusicSFX( "civilian_liberated", 0x100, &GetMatrix().GetTranslation(), TNULL );
					}

					Liberate( uiColour, TFALSE );
					return;
				}
			}

			break;
		case AEntityIOPacket::MESSAGE_TOUCHEDBY:
			// Squashed by Blob
			if ( IsTransformed() )
			{
				switch ( GetState() )
				{
					case 4:
					case 5:
					case 10:
					case 0xB:
					case 0xC:
						if ( TUINT8( m_uiCivFlags ) != ABlobColourComponent::COLOUR_BLACK )
						{
							TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
							ABlobEntity* pBlob    = TSTATICCAST( ABlobEntity, a_pCaller );
							TUINT32      uiColour = pBlob->GetColourComponent()->GetCurrentColour();

							if ( !( uiColour & ABlobColourComponent::COLOUR_BLACK ) && uiColour != ABlobColourComponent::COLOUR_BLACK )
							{
								m_fLaughTimer = RandomRange( ARandom::GetSingleton()->GetRandom(), 0.9f, 0.6f );
							}
							else
							{
								// Inky Blob: the civilian turns black and explodes
								pBlob->GetAnimationComponent()->StartBlink( 0.2f );
								SetColour( ABlobColourComponent::COLOUR_BLACK );
								m_fLaughTimer = 2.0f;
							}

							m_uiCivFlags &= ~CIVFLAG_Laughing;
							m_fSpeed = 0.0f;
							SetState( 10 );
							return;
						}
				}
			}
	}
}

// $deBlob: FUNCTION 0076cf50
TBOOL ACiv::IsTargetable()
{
	return 3 != GetState() && !IsTransformed() && TUINT8( m_Flags >> 2 ) & 1;
}

// $deBlob: FUNCTION 0076cf80
TINT ACiv::GetTargetColour()
{
	TUINT uiFlags = TUINT8( ~( m_uiCivFlags >> 8 ) );
	return ( uiFlags & 4 ) << 2;
}

// $deBlob: FUNCTION 0076cfa0
void ACiv::Reset( ABaseEntity* a_pEntity )
{
	APropPlatform::Reset( TNULL );

	m_uiCivFlags &= ~0x1000;
	m_Flags &= ~FLAG_DontTick;

	if ( m_uiCivFlags & 0x800 )
	{
		SetVisible( TFALSE );
		m_Flags |= FLAG_DontTick;
	}
	else if ( ( m_uiCivFlags & 0x300 ) == 0x300 )
	{
		if ( TUINT8( m_uiCivFlags ) == 0x10 )
		{
			Liberate( 0, TTRUE );
		}

		SetState( m_pWaypointSet ? 4 : 5 );

		TFLOAT fPathLength = m_fPathLength;
		TFLOAT fSpeed      = *TREINTERPRETCAST( TFLOAT*, TREINTERPRETCAST( TUINT8*, GetTypeData()->m_pTRBData ) + 0x58 );
		m_fSpeed           = fSpeed;

		if ( fPathLength != 0.0f )
			m_fSpeed = fSpeed / fPathLength;

		return;
	}

	if ( m_uiCivFlags & 0x400 )
	{
		// Swap back to the civilian's own model
		AEntityModelComponent* pShellModel = m_pModelComponent;
		m_pModelComponent                  = TSTATICCAST( AEntityModelComponent, m_pShellModelComponent );
		m_pShellModelComponent             = pShellModel;

		m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, pShellModel->GetModelInstance()->GetFlags() & AModelInstance2::FLAG_Visible );

		AModelInstance2* pShellInstance = TSTATICCAST( AEntityModelComponent, m_pShellModelComponent )->GetModelInstance();
		TSTATICCAST( ASoundBreakpointComponent, pShellInstance->GetSoundBreakpointComponent() )->StopAndClearSounds();
		pShellInstance->GetModelInstance()->GetSkeletonInstance()->RemoveAllAnimations( 0.0f );
		pShellInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

		m_pModelComponent->SetTransform( m_oMatrix );
		TSTATICCAST( AEntityPoseController, GetPoseController() )->SetStateDirty();

		m_uiCivFlags &= ~0x400;
		SetPhysicsEnabled( !( m_uiCivFlags & 0x400 ) );
	}

	TUINT uiType = ( m_uiCivFlags >> 8 ) & 3;
	m_uiCivFlags &= ~0xFF;

	if ( uiType == 0 )
		SetState( 2 );
	else if ( uiType == 1 )
		SetState( 0 );
	else if ( uiType == 2 )
		SetState( 1 );
}

// $deBlob: FUNCTION 0076d410
void ACiv::SetWaypointSet( TINT a_iWaypointSetID, TINT a_iStartPoint, const TVector4* a_pPosition, RespawnData* a_pRespawnData )
{
	APropPlatform::SetWaypointSet( a_iWaypointSetID, a_iStartPoint, a_pPosition, a_pRespawnData );

	if ( 5 == GetState() )
		SetState( 4 );
}

// $deBlob: FUNCTION 0076d680
void ACiv::StopMoving()
{
	if ( IsTransformed() )
		SetState( 5 );

	APropPlatform::StopMoving();
}

// $deBlob: FUNCTION 0076d6e0
const TCHAR* ACiv::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}

// Liberates the civilian: out of its shell (unless it is already), then a random colour
// $deBlob: FUNCTION 0076cf00
void ACiv::Liberate( TUINT32 a_uiColour, TBOOL a_bQuiet )
{
	if ( !IsTransformed() )
		Transform( ABlobColourComponent::ms_auiColours[ TUINT8( a_uiColour ) ], a_bQuiet );

	TUINT8 uiColour = TUINT8( ABlobColourComponent::GetRandomColour() );

	if ( TUINT8( m_uiCivFlags ) != uiColour )
		SetColour( uiColour );
}

// Sets the type and resets the civilian where it is (ACivSpawner)
// $deBlob: FUNCTION 0076d150
void ACiv::SetTypeAndReset( TUINT8 a_uiType )
{
	TASSERT( a_uiType < CivType_Count );
	SetCivType( a_uiType );

	m_vInitialPosition = GetMatrix().GetTranslation();
	TMatrix44::MatToQuat( m_qInitialRotation, GetMatrix() );
	Reset( TNULL );
}

// Walks on along the spline (or idles without one)
// $deBlob: FUNCTION 0076d1d0
void ACiv::ResumeWalking()
{
	SetState( m_pWaypointSet != TNULL ? 4 : 5 );

	TFLOAT fPathLength = m_fPathLength;
	TFLOAT fSpeed      = GetCivData()->fSpeed;
	m_fSpeed           = fSpeed;

	if ( fPathLength != 0.0f )
		m_fSpeed = fSpeed / fPathLength;
}

// An inked civilian explodes
// $deBlob: FUNCTION 0076d220
void ACiv::Explode()
{
	TASSERT( IsTransformed() );

	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x24 ), ASoundMap::EVENTID( 0x1B2 ), GetMatrix().GetTranslation(), 0 );
	AParticleEffectHelper::SetColour( m_pShellPopFX, ABlobColourComponent::ms_auiColours[ ABlobColourComponent::COLOUR_BLACK ], TNULL );

	TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pShellPopFX );

	if ( pEffect != TNULL )
	{
		TMatrix44& rTransform = pEffect->GetTransform();
		rTransform            = TMatrix44::IDENTITY;
		rTransform.SetTranslation( GetMatrix().GetTranslation() );
	}

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
		AVibrationManager::VibrateOnce( pPlayerManager->GetPlayerIndex( pPlayerManager->GetCurrentPlayerBlob() ), 4 );
	}

	if ( IsPooled() )
	{
		// ACivSpawner releases its transformed civilian
		TASSERT( TREINTERPRETCAST( ABaseEntity*, GetPoolOwner() )->IsA( &TGetClass( ACivSpawner ) ) );
		TREINTERPRETCAST( ACivSpawner*, GetPoolOwner() )->ReleaseTransformed( this );
	}

	m_Flags |= FLAG_DontTick;
	SetVisible( TFALSE );

	AGameStatus::GetSingleton()->GetLevelData().AddCivReleased( 0 );
}

// The colour of the liberated civilian (the enemy colour maps)
// $deBlob: FUNCTION 0076d440
void ACiv::SetColour( TUINT8 a_uiColour )
{
	TASSERT( HASANYFLAG( m_uiCivFlags, CIVFLAG_Transformed ) );

	if ( TUINT8( m_uiCivFlags ) != a_uiColour )
	{
		m_uiCivFlags = ( m_uiCivFlags & 0xFF00 ) | a_uiColour;

		AModelInstance2* pModelInstance = m_pModelComponent->GetModelInstance();
		pModelInstance->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
		pModelInstance->UseLocalAmbientLight( TTRUE );
		pModelInstance->SetAmbientColour( ABlobColourMaps::ms_pEnemyLightColourMap[ TUINT8( m_uiCivFlags ) ] );
		pModelInstance->SetMeshFlags( 0, 1, TTRUE );
		pModelInstance->SetMeshColour( 0, ABlobColourMaps::ms_pEnemyBodyColourMap[ TUINT8( m_uiCivFlags ) ] );
	}
}

// The civilians around Blob (SquashTransformedCivs' local AEntityHashFilterFunctorCivOnly)
class ACivHashFilter : public AEntityHashFilter
{
public:
	// $deBlob: FUNCTION 0076dbb0
	virtual TBOOL operator()( ABaseEntity* a_pEntity ) override
	{
		return a_pEntity->GetClass() == &TGetClass( ACiv );
	}
};

// Blob squashes the transformed civilians it lands on
// $deBlob: FUNCTION 0076d4f0
void ACiv::SquashTransformedCivs( ABlobEntity* a_pBlob )
{
	const TVector4& rvPosition = a_pBlob->GetBodyPosition();
	TFLOAT          fRadius    = ABlobEntity::GetProperties( a_pBlob->GetSize() )->m_fRadius;
	TFLOAT          fRangeSq   = ( fRadius + 1.0f ) * ( fRadius + 1.0f );

	ACivHashFilter oFilter;
	TINT           iCount;
	ABaseEntity**  ppEntities = AEntityHash::GetSingleton()->Query( rvPosition, fRadius - 1.0f, &iCount, &oFilter, TTRUE, 500, TFALSE, TTRUE, TFALSE );

	for ( TUINT i = 0; i < TUINT( iCount ); i++ )
	{
		ABaseEntity* pEntity = ppEntities[ i ];

		TASSERT( pEntity->IsExactly( &TGetClass( ACiv ) ) );

		if ( TVector4::DistanceSq( rvPosition, pEntity->GetMatrix().GetTranslation() ) < fRangeSq )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TOUCHEDBY;
			oPacket.m_iData    = 0;
			pEntity->HandleEntityIO( a_pBlob, oPacket );
		}
	}
}

// A transformed civilian starts laughing after a random delay
// $deBlob: FUNCTION 0076d600
void ACiv::StartLaughTimer()
{
	if ( IsTransformed() && m_fLaughTimer <= 0.0f && m_fLaughTimer != 0.0f )
	{
		m_uiCivFlags |= CIVFLAG_Laughing;
		m_fLaughTimer = ARandom::GetSingleton()->GetRandom().GetFloat() * 0.9f + 0.1f;
	}
}

// $deBlob: FUNCTION 0076d6a0
TBOOL ACiv::CanBeLiberated() const
{
	TUINT8 uiFlags = TUINT8( m_uiCivFlags >> 8 );

	if ( !( uiFlags & TUINT8( ( CIVFLAG_NoGraydian | CIVFLAG_NoLiberate ) >> 8 ) ) && ( uiFlags & TUINT8( CIVFLAG_TypeMask >> 8 ) ) != TUINT8( CIVFLAG_TypeMask >> 8 ) )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 0076d6c0
TBOOL ACiv::IsExploding() const
{
	return 10 == GetState() && TUINT8( m_uiCivFlags ) == ABlobColourComponent::COLOUR_BLACK;
}

// The civilian's own model (Civ1-3.trb, swapped with the shell by Transform) and its capsule
// $deBlob: FUNCTION 0076d6f0
void ACiv::CreateCivModel( TBOOL a_bCreateCollision )
{
	TUINT uiType = ( m_uiCivFlags & CIVFLAG_TypeMask ) >> 8;

	if ( uiType == 0 )
		SetState( 2 );
	else if ( uiType == 1 )
		SetState( 0 );
	else if ( uiType == 2 )
		SetState( 1 );

	if ( m_uiCivFlags & CIVFLAG_NoGraydian )
	{
		SetVisible( TFALSE );
		m_Flags |= FLAG_DontTick;
	}

	CreatePoseController();
	CreateStateComponent();

	TINT         iModel         = ARandom::GetSingleton()->GetRandom().GetInt( 3 );
	const TCHAR* aszModels[ 3 ] = { "Civ1.trb", "Civ2.trb", "Civ3.trb" };

	AEntityModelComponent* pModel = new AEntityModelComponent( this, aszModels[ iModel ] );
	m_pShellModelComponent        = pModel;
	pModel->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

	// Civ1.trb and Civ2.trb have their own pose map (Civ3.trb keeps the shell's)
	if ( iModel != 2 )
		TSTATICCAST( AEntityPoseController, GetPoseController() )->SetPoseMap( ms_apPoseMaps[ iModel ] );

	if ( a_bCreateCollision )
	{
		// A capsule (height 1.0, radius 1.1, along z turned upright) standing on the origin, with its physics mover
		TCapsuleData oCapsule;
		oCapsule.SetHeight( 1.0f );
		oCapsule.SetRadius( 1.1f );
		TShapeBaseClass* pShape = &oCapsule;

		TMatrix44 matOffset = TMatrix44::IDENTITY;
		matOffset.RotateX( TMath::PI / 2 );

		TVector4 vTranslation;
		vTranslation.Multiply( TVector4::VEC_NEGY, 1.0f * 0.5f + 1.1f );
		matOffset.SetTranslation( vTranslation );

		CreatePhysics( m_pTypeData->m_pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
		SetPhysicsMover( new AEntityPhysicsMover( this ) );
	}
}

// Out of the shell: swaps to the civilian's own model, pops the shell in a_uiColour
// $deBlob: FUNCTION 0076d950
void ACiv::Transform( TUINT32 a_uiColour, TBOOL a_bQuiet )
{

	SetPhysicsEnabled( TFALSE );
	m_Flags &= ~FLAG_DontTick;
	SetVisible( TTRUE );

	AEntityModelComponent* pShellModel = m_pModelComponent;
	AEntityModelComponent* pCivModel   = TSTATICCAST( AEntityModelComponent, m_pShellModelComponent );
	m_pShellModelComponent             = pShellModel;
	m_pModelComponent                  = pCivModel;

	pCivModel->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, pShellModel->GetModelInstance()->GetFlags() & AModelInstance2::FLAG_Visible );
	pShellModel->GetModelInstance()->GetModelInstance()->GetSkeletonInstance()->RemoveAllAnimations( 0.0f );

	// The sounds of the shell's animations
	TSTATICCAST( ASoundBreakpointComponent, pShellModel->GetModelInstance()->GetSoundBreakpointComponent() )->StopAndClearSounds();

	pShellModel->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	m_pModelComponent->SetTransform( GetMatrix() );
	TSTATICCAST( AEntityPoseController, GetPoseController() )->SetStateDirty();

	if ( !a_bQuiet )
	{
		AParticleEffectHelper::SetColour( m_pShellPopFX, a_uiColour, TNULL );

		TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pShellPopFX );

		if ( pEffect != TNULL )
		{
			TMatrix44& rTransform = pEffect->GetTransform();
			rTransform            = TMatrix44::IDENTITY;
			rTransform.SetTranslation( GetMatrix().GetTranslation() );
		}

		if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
		{
			APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
			AVibrationManager::VibrateOnce( pPlayerManager->GetPlayerIndex( pPlayerManager->GetCurrentPlayerBlob() ), 4 );
		}

		if ( !IsPooled() )
		{
			// NOTE: retail passes the ID as the argument
			AChallengeManager::GetSingleton()->OnCivilianEvent( TREINTERPRETCAST( void*, TUINTPTR( GetID() ) ) );
		}
		else
		{
			// The spawner (the pooled owner, GetPoolOwner) counts its transformed civilian
			TASSERT( TREINTERPRETCAST( ABaseEntity*, GetPoolOwner() )->IsA( &TGetClass( ACivSpawner ) ) );
			TREINTERPRETCAST( ACivSpawner*, GetPoolOwner() )->OnCivTransformed( this );
		}
	}

	m_fSpeed = 0.0f;
	SetState( 3 );
	m_uiCivFlags |= CIVFLAG_Transformed;
}

// $deBlob: FUNCTION 0076dd20
TFLOAT ACiv::RandomRange( TRandom& a_rRandom, TFLOAT a_fMin, TFLOAT a_fRange )
{
	return a_rRandom.GetFloat() * a_fRange + a_fMin;
}

// $deBlob: FUNCTION 0076de60
void ACiv::SetCivType( TUINT8 a_uiType )
{
	TASSERT( a_uiType < CivType_Count );
	TASSERT( 0 == ( a_uiType & ( ~( CIVFLAG_TypeMask >> 8 ) ) ) );
	m_uiCivFlags = ( m_uiCivFlags & ~CIVFLAG_TypeMask ) | ( a_uiType << 8 );
}
