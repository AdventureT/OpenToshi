#include "pch.h"
#include "APropWindCannon.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Entities/ABlobEntity/ABlobSoundComponent.h"
#include "Animation/AModel2.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Sound/APositionalSoundManager.h"
#include "Sound/ASoundInterface.h"

#include <Physics/TCapsuleData.h>
#include <Render/TModel.h>
#include <Render/TSkeleton.h>
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

APropWindCannon::Anims APropWindCannon::ms_oAnims = { TFALSE, -1, -1, -1, -1, -1, -1 };

TSTATICASSERT( sizeof( APropWindCannon ) == 0x120 );

// The time the blob shrinks into the barrel before the shot
static const TFLOAT s_fFireTime = 0.07f;

// The distance (beyond the blob's radius) from the target's position within which a blob is offered the cannon
static const TFLOAT s_fOfferDistance = 5.0f;

// The gravity of the shot
static const TFLOAT s_fShotGravity = 80.0f;

// The size index of a_pBlob from its paint points (at most 9)
// $deBlob: FUNCTION 007c2c00
static TUINT __fastcall GetBlobSizeIndex( ABlobEntity* a_pBlob )
{
	TINT iSize = TUINT( a_pBlob->GetPaintPoints() ) / 10;
	return TMath::Min( iSize, 9 );
}

// $deBlob: FUNCTION 007c0a50
// $deBlob: FUNCTION 007c0a60
// $deBlob: FUNCTION 007c0ad0
// $deBlob: FUNCTION 007c0b30
// $deBlob: FUNCTION 007c0b40
// $deBlob: FUNCTION 005417b0
// $deBlob: FUNCTION 0085a060
TDEFINE_CLASS( APropWindCannon );

// $deBlob: FUNCTION 007c0b50
APropWindCannon::APropWindCannon()
{
	m_eState          = STATE_CLOSED;
	m_uiCannonFlags   = 0;
	m_pBlob           = TNULL;
	m_pEntryInstance  = TNULL;
	m_pEntryEffect    = TNULL;
	m_pFireInstance   = TNULL;
	m_pFireEffect     = TNULL;
	m_pTargetInstance = TNULL;
	m_pTargetEffect   = TNULL;
	m_fEnterTime      = 0.0f;
	m_fLoadTime       = 0.0f;
	m_fFireTime       = 0.0f;
	m_vSpawnEnd       = TVector4::VEC_ZERO;
	m_vTargetPos      = TVector4::VEC_ZERO;
	m_vDirection      = TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 007c2bd0
// $deBlob: FUNCTION 007c0c20
// $deBlob: FUNCTION 007c2bc1
APropWindCannon::~APropWindCannon()
{
	StopEffect( m_pFireInstance, TTRUE );
	StopEffect( m_pEntryInstance, TTRUE );
	StopEffect( m_pTargetInstance, TTRUE );
}

void APropWindCannon::StopEffect( TParticleEffectInstance*& a_rpInstance, TBOOL a_bInstant )
{
	if ( a_rpInstance != TNULL )
	{
		a_rpInstance->SetFinishedCallback( TNULL, TNULL );

		if ( a_bInstant )
			a_rpInstance->StopInstant();
		else
			a_rpInstance->Stop();

		a_rpInstance = TNULL;
	}
}

// $deBlob: FUNCTION 007c0d10
// $deBlob: FUNCTION 007c2c47
void APropWindCannon::ProcessParams( const AEntityParams& a_rParams )
{
	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );

	AProp::ProcessParams( a_rParams );

	a_rParams.GetValue( m_vTargetPos, "TargetPos" );

	TBOOL bStartOn = TFALSE;
	a_rParams.GetValue( bStartOn, "StartOn" );

	if ( bStartOn || AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint )
		m_uiCannonFlags |= CANNONFLAG_StartOn | CANNONFLAG_On;

	if ( !a_rParams.GetValue( m_fEnterWait, "EnterWait" ) )
		m_fEnterWait = 0.5f;

	if ( !a_rParams.GetValue( m_fLoadWait, "LoadWait" ) )
		m_fLoadWait = 1.0f;

	TParticleEffectManager* pEffectManager = TParticleSystem::GetSingleton()->GetEffectManager();

	m_pFireEffect = pEffectManager->FindEffect( "Blob_WindLauncher_Fire" );
	TASSERT( m_pFireEffect != TNULL );

	m_pEntryEffect = pEffectManager->FindEffect( "Blob_WindLauncher_Suck2" );
	TASSERT( m_pEntryEffect != TNULL );

	// The body: a capsule (10 long, radius 3.5) along the barrel, 6 from the origin
	TCapsuleData     oCapsule;
	TShapeBaseClass* pShape = &oCapsule;
	oCapsule.SetHeight( 10.0f );
	oCapsule.SetRadius( 3.5f );

	TMatrix44 mCapsule = *rEntityData.m_pMatrix;
	mCapsule.RotateX( TMath::PI / 2 );

	TVector4 vOffset( 0.0f, 0.0f, 6.0f, 1.0f );
	TMatrix44::TransformVector( mCapsule.AsBasisVector4( BASISVECTOR_TRANSLATION ), mCapsule, vOffset );

	CreatePhysics( m_pTypeData->m_pTRBData, &pShape, &mCapsule, 1, TNULL, TFALSE );
}

// $deBlob: FUNCTION 007c0f10
// $deBlob: FUNCTION 007c2c3c
void APropWindCannon::PostCreate()
{
	AProp::PostCreate();

	m_vDirection.x = -m_oMatrix.m_f21;
	m_vDirection.y = -m_oMatrix.m_f22;
	m_vDirection.z = -m_oMatrix.m_f23;
	m_vDirection.w = 1.0f;

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();

	{
		// The barrel's sphere map
		TModel*    pModel = pModelInstance->GetModel()->GetModelPtr().GetData();
		TModelLOD& rLOD   = pModel->GetLOD( 0 );
		TASSERT( rLOD.GetMeshCount() == MESH_COUNT && "The number of meshes in the wind cannon model has changed. Code needs to be updated to support this." );

		ASkinMaterialHAL* pMaterial = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMesh( 1 )->GetMaterial() );
		TTexture*         pTexture  = TTextureManager::GetSingleton()->FindTexture( "spheremap_windcannon.tga" );

		if ( pMaterial->GetTexture( 0 ) != pTexture )
		{
			pTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );
			pMaterial->SetTevMode( 0, 0 );
			pMaterial->SetTexture( 0, pTexture );
			pMaterial->SetFlags( TMaterial::Flags( 0x80 ), TTRUE );
			pMaterial->SetPassMode( 0, ASkinMaterialHAL::PASSMODE_ENVMAP );
			pMaterial->SetBlendMode( 0x18, 0 );
			pMaterial->SetFlags( TMaterial::Flags( 1 ), TTRUE );
		}
	}

	TASSERT( GetNumEntityTargets() == 1 );
	ABaseEntity* pTarget = GetEntityTarget( 0 );
	TASSERT( pTarget->IsExactly( &TGetClass( AProp ) ) );
	m_pTarget = pTarget;

	if ( !ms_oAnims.m_bLoaded )
	{
		// NOTE: m_bLoaded is never set by retail, so every cannon looks them up again
		AToshiAnimInterface2* pTargetAnims = TSTATICCAST( AEntityModelComponent, pTarget->GetModelComponent() )->GetModelInstance()->GetAnimInterface();
		ms_oAnims.m_iOpen                  = TINT16( pTargetAnims->GetAnimID( "Open" ) );
		ms_oAnims.m_iOpenLoop              = TINT16( pTargetAnims->GetAnimID( "Open_Loop" ) );
		ms_oAnims.m_iClose                 = TINT16( pTargetAnims->GetAnimID( "Close" ) );
		ms_oAnims.m_iCloseLoop             = TINT16( pTargetAnims->GetAnimID( "Close_Loop" ) );

		ms_oAnims.m_iShoot       = TINT16( TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetAnimID( "shoot" ) );
		ms_oAnims.m_iSpawnBoneID = TINT16( TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->FindBone( "blob_spawn" ) );
		TASSERT( -1 < ms_oAnims.m_iSpawnBoneID );
	}

	TINT16 iBoneID = TINT16( TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->FindBone( "blob_spawnend" ) );
	TASSERT( -1 < iBoneID );

	TMatrix44 mBone;
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( iBoneID, mBone );
	m_vSpawnEnd = mBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// $deBlob: FUNCTION 007c1280
void APropWindCannon::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pBlob != TNULL )
	{
		switch ( m_eState )
		{
			case STATE_OFFERED:
			{
				// Gone away
				TFLOAT fDistance = TVector4::Distance( m_pBlob->GetBodyPosition(), m_pTarget->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) ) - ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ].m_fRadius;

				if ( s_fOfferDistance < fDistance )
					SetState( STATE_SEEK );

				break;
			}
			case STATE_ENTERING:
			{
				// Once the blob's entering animation is over and after EnterWait
				ABlobAnimationComponent::Anim* pAnim = &ABlobAnimationComponent::ms_pAnimationProperties->m_pCommon->m_pAnims[ 0x170 / sizeof( ABlobAnimationComponent::Anim ) ];

				if ( !m_pBlob->GetAnimationComponent()->IsAnimPlaying( pAnim ) || m_pBlob->GetAnimationComponent()->IsAnimFinished( pAnim ) )
				{
					m_fEnterTime = m_fEnterTime - a_fDeltaTime;

					if ( m_fEnterTime < 0.0f )
						SetState( STATE_LOADED );
				}

				break;
			}
			case STATE_LOADED:
				m_fLoadTime = m_fLoadTime - a_fDeltaTime;

				if ( m_fLoadTime < 0.0f )
					SetState( STATE_FIRING );

				break;
			case STATE_FIRING:
			{
				m_fFireTime = m_fFireTime + a_fDeltaTime;

				if ( s_fFireTime <= m_fFireTime )
				{
					// Out of the end of the barrel, towards TargetPos
					TVector4 vPosition;
					vPosition.Multiply( m_vDirection, ABlobEntity::ms_pBlobProperties[ GetBlobSizeIndex( m_pBlob ) ].m_fRadius );
					vPosition.x = vPosition.x + m_vSpawnEnd.x;
					vPosition.y = m_vSpawnEnd.y + vPosition.y;
					vPosition.z = m_vSpawnEnd.z + vPosition.z;

					TVector4 vVelocity;
					ABlobEntity::CalculateJumpVelocityAtSpeed( vPosition, m_vTargetPos, ABlobEntity::ms_pBlobCommonProperties->m_afUnk38[ 3 ] /* 0x44 */, s_fShotGravity, vVelocity );
					m_pBlob->LaunchFromCannon( vPosition, vVelocity );
					m_pBlob = TNULL;

					SetState( STATE_SEEK );
				}
				else
				{
					// Shrinks into the barrel
					TMatrix44 mBone;
					TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( ms_oAnims.m_iSpawnBoneID, mBone );

					TFLOAT fScale = 1.0f - ( m_fFireTime * 0.5f ) / s_fFireTime;
					mBone.Scale( fScale, fScale, fScale );
					m_pBlob->SetCannonTransform( mBone );
				}

				break;
			}
			default:
				break;
		}
	}

	if ( !( m_uiCannonFlags & CANNONFLAG_On ) )
	{
		switch ( m_eState )
		{
			case STATE_OPENING:
			case STATE_SEEK:
			case STATE_OFFERED:
				if ( GetLOD() != FLAG_LowLOD )
				{
					SetState( STATE_CLOSING );
					break;
				}

				SetState( STATE_CLOSED );
				break;
			case STATE_CLOSING:
				if ( GetLOD() != FLAG_LowLOD && !TSTATICCAST( AProp, m_pTarget )->IsManualAnimationFinished( ms_oAnims.m_iClose ) )
					break;

				SetState( STATE_CLOSED );
				break;
			default:
				break;
		}
	}
	else if ( m_eState == STATE_CLOSED || m_eState == STATE_CLOSING )
	{
		if ( GetLOD() == FLAG_LowLOD )
			SetState( STATE_SEEK );
		else
			SetState( STATE_OPENING );
	}
	else if ( m_eState == STATE_OPENING )
	{
		if ( GetLOD() == FLAG_LowLOD || TSTATICCAST( AProp, m_pTarget )->IsManualAnimationFinished( ms_oAnims.m_iOpen ) )
			SetState( STATE_SEEK );
	}
	else if ( GetLOD() != FLAG_LowLOD && m_pBlob == TNULL )
	{
		TASSERT( m_eState == STATE_SEEK );

		// A blob near the target is offered the cannon
		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
		TINT            iNumPlayers    = pPlayerManager->GetNumPlayers();

		for ( TINT i = 0; i < iNumPlayers; i++ )
		{
			ABlobEntity* pBlob     = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( i ) ) );
			TFLOAT       fDistance = TVector4::Distance( pBlob->GetBodyPosition(), m_pTarget->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) ) - ABlobEntity::ms_pBlobProperties[ pBlob->m_iSize ].m_fRadius;

			if ( fDistance < s_fOfferDistance )
			{
				m_pBlob = pBlob;

				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
				oPacket.m_iData    = 0;
				pBlob->HandleEntityIO( this, oPacket );

				SetState( STATE_OFFERED );
			}
		}
	}

	AProp::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007c1d00
void APropWindCannon::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			// Toggles
			SetActive( !( ( m_uiCannonFlags >> 1 ) & 1 ) );
			break;
		case AEntityIOPacket::MESSAGE_ENTER:
			// The blob is in
			TASSERT( m_pBlob == a_pCaller );

			if ( m_eState != STATE_ENTERING )
				SetState( STATE_ENTERING );

			break;
		case AEntityIOPacket::MESSAGE_CHALLENGEINIT:
		case AEntityIOPacket::MESSAGE_CHALLENGERESET:
			SetActive( TFALSE );
			break;
		case AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE:
			SetActive( TTRUE );
			break;
		default:
			break;
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007c1eb0
void APropWindCannon::OnLODChanging( TINT a_iLOD )
{
	TBOOL bCheckSound = TFALSE;

	if ( !( m_uiCannonFlags & CANNONFLAG_On ) )
	{
		bCheckSound = a_iLOD != FLAG_LowLOD;
	}
	else if ( a_iLOD != FLAG_HighLOD || ( m_uiCannonFlags & CANNONFLAG_EntryEnded ) )
	{
		StopEffect( m_pFireInstance, TFALSE );
		StopEffect( m_pEntryInstance, TFALSE );
		StopEffect( m_pTargetInstance, TFALSE );
		bCheckSound = a_iLOD != FLAG_LowLOD;
	}
	else
	{
		if ( m_eState == STATE_FIRING )
			StartFireEffect();

		StartEntry();
		bCheckSound = TTRUE;
	}

	if ( bCheckSound && ( m_uiCannonFlags & CANNONFLAG_On ) && !( m_uiCannonFlags & CANNONFLAG_EntryEnded ) )
	{
		if ( !( m_uiCannonFlags & CANNONFLAG_Sound ) )
		{
			APositionalSoundManager::GetSingleton()->AddEntitySound( m_pTarget, 5, 0x6B );
			m_uiCannonFlags |= CANNONFLAG_Sound;
		}

		AVisibleEntity::OnLODChanging( a_iLOD );
		return;
	}

	if ( m_uiCannonFlags & CANNONFLAG_Sound )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( m_pTarget, 5, 0x6B );
		m_uiCannonFlags &= ~CANNONFLAG_Sound;
	}

	AVisibleEntity::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 007c1ff0
void APropWindCannon::Destroy()
{
	if ( m_uiCannonFlags & CANNONFLAG_Sound )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( m_pTarget, 5, 0x6B );
		m_uiCannonFlags &= ~CANNONFLAG_Sound;
	}

	if ( ms_oAnims.m_bLoaded )
	{
		ms_oAnims.m_bLoaded      = TFALSE;
		ms_oAnims.m_iOpen        = -1;
		ms_oAnims.m_iOpenLoop    = -1;
		ms_oAnims.m_iClose       = -1;
		ms_oAnims.m_iCloseLoop   = -1;
		ms_oAnims.m_iShoot       = -1;
		ms_oAnims.m_iSpawnBoneID = -1;
	}

	StopEffect( m_pTargetInstance, TTRUE );
	StopEffect( m_pEntryInstance, TTRUE );
	StopEffect( m_pFireInstance, TTRUE );

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 007c20f0
void APropWindCannon::OnEffectFinished( TParticleEffectInstance* a_pInstance, void* a_pCannon )
{
	APropWindCannon* pCannon = TSTATICCAST( APropWindCannon, a_pCannon );

	if ( pCannon->m_pFireInstance == a_pInstance )
		pCannon->m_pFireInstance = TNULL;

	if ( pCannon->m_pEntryInstance == a_pInstance )
		pCannon->m_pEntryInstance = TNULL;

	if ( pCannon->m_pTargetInstance == a_pInstance )
		pCannon->m_pTargetInstance = TNULL;
}

// $deBlob: FUNCTION 007c2140
void APropWindCannon::SetActive( TBOOL a_bActive )
{
	if ( AGameStatus::GetSingleton()->GetLevelMode() == AGameStatus::Mode_FreePaint || a_bActive )
	{
		if ( !( m_uiCannonFlags & CANNONFLAG_On ) )
		{
			ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 5 ), ASoundMap::EVENTID( 0x6C ), GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), 0 );
			m_uiCannonFlags |= CANNONFLAG_On;

			if ( GetLOD() == FLAG_HighLOD && m_eState != STATE_CLOSED && m_eState != STATE_CLOSING )
				StartEntry();
		}
	}
	else if ( m_uiCannonFlags & CANNONFLAG_On )
	{
		if ( m_uiCannonFlags & CANNONFLAG_Sound )
		{
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pTarget, 5, 0x6B );
			m_uiCannonFlags &= ~CANNONFLAG_Sound;
		}

		m_uiCannonFlags &= ~CANNONFLAG_On;
	}
}

// $deBlob: FUNCTION 007c21e0
void APropWindCannon::Reset( ABaseEntity* a_pEntity )
{
	if ( m_eState != STATE_CLOSED )
		SetState( STATE_CLOSED );

	m_pBlob      = TNULL;
	m_fEnterTime = 0.0f;
	m_fLoadTime  = 0.0f;

	StopEffect( m_pFireInstance, TFALSE );
	StopEffect( m_pEntryInstance, TFALSE );
	StopEffect( m_pTargetInstance, TFALSE );

	if ( m_uiCannonFlags & CANNONFLAG_Sound )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( m_pTarget, 5, 0x6B );
		m_uiCannonFlags &= ~CANNONFLAG_Sound;
	}

	m_uiCannonFlags &= CANNONFLAG_StartOn;

	if ( m_uiCannonFlags & CANNONFLAG_StartOn )
		m_uiCannonFlags |= CANNONFLAG_On;

	AProp::Reset( a_pEntity );
}

// $deBlob: FUNCTION 007c23a0
void APropWindCannon::OnBlobEntering()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 5 ), ASoundMap::EVENTID( 0x6D ), m_pBlob->GetBodyPosition(), 0 );
	m_uiCannonFlags |= CANNONFLAG_EntryEnded;

	StopEffect( m_pEntryInstance, TFALSE );
	StopEffect( m_pTargetInstance, TFALSE );

	if ( m_uiCannonFlags & CANNONFLAG_Sound )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( m_pTarget, 5, 0x6B );
		m_uiCannonFlags &= ~CANNONFLAG_Sound;
	}
}

// $deBlob: FUNCTION 007c2450
void APropWindCannon::OnBlobLeaving()
{
	m_uiCannonFlags &= ~CANNONFLAG_EntryEnded;

	if ( m_uiCannonFlags & CANNONFLAG_On )
		StartEntry();
}

// $deBlob: FUNCTION 007c2470
void APropWindCannon::OnBlobMoved( ABlobEntity* a_pBlob )
{
	if ( m_eState == STATE_OFFERED && m_pBlob == a_pBlob &&
	     s_fOfferDistance < TVector4::Distance( m_pBlob->GetBodyPosition(), m_pTarget->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) ) - ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ].m_fRadius )
	{
		SetState( STATE_SEEK );
	}
}

// $deBlob: FUNCTION 007c25c0
void APropWindCannon::SetState( STATE a_eState )
{
	if ( a_eState == m_eState )
		return;

	if ( m_eState == STATE_FIRING )
		StopEffect( m_pFireInstance, TFALSE );

	// Lets the blob go (MESSAGE_UNTOUCH)
	AEntityIOPacket oRelease;
	oRelease.m_eMessage = AEntityIOPacket::MESSAGE_UNTOUCH;
	oRelease.m_iData    = 0;

	TINT16 iTargetAnim = -1;

	switch ( a_eState )
	{
		case STATE_CLOSED:
			StopEntryEffects( TFALSE );

			if ( m_pBlob != TNULL )
			{
				m_pBlob->HandleEntityIO( this, oRelease );
				m_pBlob = TNULL;
			}

			iTargetAnim = ms_oAnims.m_iCloseLoop;
			break;
		case STATE_OPENING:
			StartEntry();
			iTargetAnim = ms_oAnims.m_iOpen;
			break;
		case STATE_CLOSING:
			StopEntryEffects( TFALSE );

			if ( m_pBlob != TNULL )
			{
				m_pBlob->HandleEntityIO( this, oRelease );
				m_pBlob = TNULL;
			}

			iTargetAnim = ms_oAnims.m_iClose;
			break;
		case STATE_SEEK:
			StartEntry();

			if ( m_pBlob != TNULL )
			{
				m_pBlob->HandleEntityIO( this, oRelease );
				m_pBlob = TNULL;
			}

			break;
		case STATE_OFFERED:
			break;
		case STATE_ENTERING:
			m_fEnterTime = m_fEnterWait;
			break;
		case STATE_LOADED:
		{
			// The blob sits in the barrel
			TMatrix44 mBone;
			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetBoneTransform( ms_oAnims.m_iSpawnBoneID, mBone );
			m_pBlob->EnterCannon( mBone, 9 );

			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TOUCH;
			oPacket.m_iData    = 0;
			AProp::HandleEntityIO( m_pBlob, oPacket );

			m_fLoadTime = m_fLoadWait;
			break;
		}
		case STATE_FIRING:
			StartFireEffect();
			AddManualAnimation( ms_oAnims.m_iShoot, 0.0f, 0.0f, 1.0f, 1.0f );
			m_fFireTime = 0.0f;
			m_pBlob->GetSoundComponent()->PlayCannonFire();
			break;
		default:
			m_eState = a_eState;
			return;
	}

	AProp* pTarget = TSTATICCAST( AProp, m_pTarget );

	if ( a_eState == STATE_CLOSED || a_eState == STATE_OPENING || a_eState == STATE_CLOSING )
	{
		pTarget->AddManualAnimation( iTargetAnim, 0.0f, 0.15f, 1.0f, 1.0f );
	}
	else
	{
		// Keeps the target open
		TSkeletonInstance* pSkeleton = TSTATICCAST( AEntityModelComponent, pTarget->GetModelComponent() )->GetModelInstance()->GetAnimInterface()->GetSkeletonInstance();

		if ( pSkeleton == TNULL || pSkeleton->IsAnimating( ms_oAnims.m_iOpenLoop ) == TNULL )
			pTarget->AddManualAnimation( ms_oAnims.m_iOpenLoop, 0.0f, 0.15f, 1.0f, 1.0f );
	}

	m_eState = a_eState;
}

void APropWindCannon::GetEffectTransform( TMatrix44& a_rOutMatrix ) const
{
	a_rOutMatrix = m_oMatrix;
	a_rOutMatrix.SetTranslation( m_vSpawnEnd );
}

// The loop sound of the target
// $deBlob: FUNCTION 007c2840
void APropWindCannon::StartLoopSound()
{
	if ( !( m_uiCannonFlags & CANNONFLAG_Sound ) )
	{
		APositionalSoundManager::GetSingleton()->AddEntitySound( m_pTarget, 5, 0x6B );
		m_uiCannonFlags |= CANNONFLAG_Sound;
	}
}

// $deBlob: FUNCTION 007c2870
void APropWindCannon::StopLoopSound()
{
	if ( m_uiCannonFlags & CANNONFLAG_Sound )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( m_pTarget, 5, 0x6B );
		m_uiCannonFlags &= ~CANNONFLAG_Sound;
	}
}

// $deBlob: FUNCTION 007c28a0
void APropWindCannon::StartFireEffect()
{
	if ( m_pFireEffect == TNULL || m_pFireInstance != TNULL )
		return;

	m_pFireInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pFireEffect );

	if ( m_pFireInstance != TNULL )
	{
		GetEffectTransform( m_pFireInstance->GetTransform() );
		m_pFireInstance->SetFinishedCallback( OnEffectFinished, this );
	}
}

// Stops the fire effect
// $deBlob: FUNCTION 007c2920
void APropWindCannon::StopFireEffect( TBOOL a_bInstant )
{
	StopEffect( m_pFireInstance, a_bInstant );
}

// $deBlob: FUNCTION 007c2980
void APropWindCannon::StartEntry()
{
	if ( !( m_uiCannonFlags & CANNONFLAG_Sound ) )
	{
		APositionalSoundManager::GetSingleton()->AddEntitySound( m_pTarget, 5, 0x6B );
		m_uiCannonFlags |= CANNONFLAG_Sound;
	}

	if ( m_pEntryEffect != TNULL && m_pEntryInstance == TNULL )
	{
		m_pEntryInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pEntryEffect );

		if ( m_pEntryInstance != TNULL )
		{
			GetEffectTransform( m_pEntryInstance->GetTransform() );
			m_pEntryInstance->SetFinishedCallback( OnEffectFinished, this );
		}
	}

	StartTargetEffect();
}

// $deBlob: FUNCTION 007c2a30
void APropWindCannon::StopEntryEffects( TBOOL a_bInstant )
{
	StopEffect( m_pEntryInstance, a_bInstant );
	StopEffect( m_pTargetInstance, a_bInstant );
}

// $deBlob: FUNCTION 007c2ac0
void APropWindCannon::StartTargetEffect()
{
	if ( m_pTargetEffect == TNULL || m_pTargetInstance != TNULL )
		return;

	m_pTargetInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pTargetEffect );

	if ( m_pTargetInstance != TNULL )
	{
		m_pTargetInstance->GetTransform() = m_pTarget->GetMatrix();
		m_pTargetInstance->SetFinishedCallback( OnEffectFinished, this );
	}
}

// Stops the target effect
// $deBlob: FUNCTION 007c2b40
void APropWindCannon::StopTargetEffect( TBOOL a_bInstant )
{
	StopEffect( m_pTargetInstance, a_bInstant );
}
