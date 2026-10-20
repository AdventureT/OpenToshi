#include "pch.h"
#include "AJetbikeSpawner.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionInstance2.h"
#include "Collision/ACollisionObject.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/AEntityParams.h"
#include "Entities/AInkJet/AInkJet.h"
#include "GameInterface/APlayerManager.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include <Math/TMath.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Platform/DX11/TTexture_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::TMaterial* AJetbikeSpawner::sm_pMaterial = TNULL;
// $deBlob: FUNCTION 005414a0
TFLOAT AJetbikeSpawner::ms_fTriggerFOV = Toshi::TMath::PI * 0.125f;

TSTATICASSERT( sizeof( AJetbikeSpawner ) == 0xB8 );

// $deBlob: FUNCTION 007ae9f0
// $deBlob: FUNCTION 007aea00
// $deBlob: FUNCTION 007aea70
// $deBlob: FUNCTION 007aead0
// $deBlob: FUNCTION 007aeae0
// $deBlob: FUNCTION 00541460
// $deBlob: FUNCTION 00859fa0
TDEFINE_CLASS( AJetbikeSpawner );

// $deBlob: FUNCTION 007aeaf0
AJetbikeSpawner::AJetbikeSpawner()
{
	m_uiJSFlags           = 0;
	m_pInkJet             = TNULL;
	m_pCollisionObject    = TNULL;
	m_fTriggerDistance    = 0.0f;
	m_fnOldRenderCallback = TNULL;
	m_pOldRenderUserData  = TNULL;
	SetState( 0 );
	m_fTimer = 5.0f;
}

// $deBlob: FUNCTION 007af7e0
// $deBlob: FUNCTION 007af7d0
AJetbikeSpawner::~AJetbikeSpawner()
{
}

// $deBlob: FUNCTION 007aeb80
void __stdcall AJetbikeSpawner::RenderCallback( TModelInstance* a_pModelInstance, void* a_pSpawner )
{
	AJetbikeSpawner* pSpawner = TSTATICCAST( AJetbikeSpawner, a_pSpawner );

	if ( ( pSpawner->m_uiJSFlags & JSFLAG_Deactivated ) == 0 )
	{
		pSpawner->m_fnOldRenderCallback( a_pModelInstance, pSpawner->m_pOldRenderUserData );
		return;
	}

	TASSERT( 1 == a_pModelInstance->GetModel()->GetLODCount() );

	// The meshes with the material of the first one render with the spawner colour material
	T2ModelPtr pModel = a_pModelInstance->GetModel();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	TMaterial* pMaterial = rLOD.GetMeshes()[ 0 ]->GetMaterial();

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		if ( rLOD.GetMeshes()[ i ]->GetMaterial() == pMaterial )
			rLOD.GetMeshes()[ i ]->SetMaterial( sm_pMaterial );
	}

	pSpawner->m_fnOldRenderCallback( a_pModelInstance, pSpawner->m_pOldRenderUserData );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		if ( rLOD.GetMeshes()[ i ]->GetMaterial() == sm_pMaterial )
			rLOD.GetMeshes()[ i ]->SetMaterial( pMaterial );
	}
}

void AJetbikeSpawner::LaunchJet()
{
	if ( ( m_pInkJet->GetFlags() & FLAG_Dead ) == 0 )
	{
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
		oPacket.m_iData    = 0;
		m_pInkJet->HandleEntityIO( this, oPacket );
	}
}

// $deBlob: FUNCTION 007aed50
void AJetbikeSpawner::Tick( TFLOAT a_fDeltaTime )
{
	AVisibleEntity::Tick( a_fDeltaTime );
	m_uiJSFlags &= ~JSFLAG_Reset;

	switch ( GetState() )
	{
		case 0:
			SetTicking( TFALSE );

			if ( ( m_uiJSFlags & JSFLAG_Enabled ) == 0 || ( m_uiJSFlags & JSFLAG_Deactivated ) != 0 )
			{
				m_Flags |= FLAG_DontTick;
			}
			else
			{
				m_fTimer -= a_fDeltaTime;

				if ( m_fTimer < 0.0f )
				{
					m_fTimer = 0.0f;

					// The jet is gone: it comes back when the blob is in the cone (or at once without a distance)
					if ( ( m_pInkJet->GetFlags() & FLAG_Dead ) != 0 )
					{
						if ( ( m_uiJSFlags & JSFLAG_HasTriggerDistance ) == 0 ||
						     TMath::ConeVsSphere( GetTranslation(), m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), ms_fTriggerFOV, m_fTriggerDistance, TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) )->GetBodyPosition() ) )
						{
							SetTicking( TTRUE );
							SetState( 1 );
							m_fTimer = 1.4f;
						}
					}
				}
			}
			break;
		case 1:
			if ( m_fTimer > 0.0f )
			{
				m_fTimer -= a_fDeltaTime;

				if ( m_fTimer <= 0.0f )
				{
					m_pInkJet->Activate();
					m_uiJSFlags |= JSFLAG_JetPrepared;
				}
			}

			if ( HasStateFinished() )
			{
				SetState( 2 );
				m_fTimer = 1.0f;
			}
			break;
		case 2:
		{
			TFLOAT fOldTimer = m_fTimer;
			m_fTimer         = fOldTimer - a_fDeltaTime;

			if ( fOldTimer > 0.5f && m_fTimer < 0.5f )
			{
				LaunchJet();
				m_uiJSFlags &= ~JSFLAG_JetPrepared;
			}

			if ( m_fTimer < 0.0f )
				SetState( 3 );

			break;
		}
		case 3:
			if ( HasStateFinished() )
			{
				m_fTimer = 5.0f;
				SetState( 0 );
			}
			break;
	}
}

// $deBlob: FUNCTION 007aef70
void AJetbikeSpawner::Create()
{
	AVisibleEntity::Create();

	// The high byte of m_iCollisionGroupAndType
	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( 32 << 8 ) );

	// The collision of the model (layer 18)
	TModelInstance* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();

	ACollisionInstance2* pCollision = new ACollisionInstance2();
	pCollision->SetModelInstance( pModelInstance, TNULL );

	ACollisionInstance2::OwnerInfo oOwner;
	oOwner.m_eType  = 2;
	oOwner.m_pOwner = this;
	pCollision->Create( pModelInstance, &oOwner, TTRUE, 0, TNULL );

	for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
	{
		TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

		if ( pBody != TNULL )
			TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, 18 );
	}

	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetCollisionInstance( pCollision );

	CreateCollisionObject();

	// The model renders through RenderCallback
	pModelInstance        = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();
	m_fnOldRenderCallback = pModelInstance->GetCustomRenderCb();
	m_pOldRenderUserData  = pModelInstance->GetCustomRenderCbUserData();
	pModelInstance->SetCustomRenderMethod( RenderCallback, this );

	if ( sm_pMaterial == TNULL )
	{
		// The spawner colour material: a copy of the first mesh's material with jetbike_spawner_colour.tga
		ASkinShader* pShader = ASkinShader::GetSingleton();

		T2ModelPtr pModel    = pModelInstance->GetModel();
		TMaterial* pOriginal = pModel->GetLOD( 0 ).GetMeshes()[ 0 ]->GetMaterial();

		ASkinMaterial* pMaterial = TSTATICCAST( ASkinMaterial, pShader->CreateMaterial( TNULL ) );
		sm_pMaterial             = pMaterial;
		pMaterial->Create( 0 );

		pMaterial->SetTextureNum( pOriginal->GetTextureNum() );

		for ( TUINT i = 0; i < pOriginal->GetTextureNum(); i++ )
			pMaterial->SetTexture( i, pOriginal->GetTexture( i ) );

		pMaterial->SetName( pOriginal->GetName() );
		pMaterial->SetTexture( 0, TTextureManager::GetSingleton()->FindTexture( "jetbike_spawner_colour.tga" ) );
	}
}

// $deBlob: FUNCTION 007af720
void AJetbikeSpawner::CreateCollisionObject()
{
	if ( GetCollisionObject() == TNULL )
	{
		m_pCollisionObject = new ACollisionObject( this, 1, 0xFF );
		GetCollisionObject()->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
		m_pCollisionObject->SetFlags( ACollisionObject::FLAG_Created );
	}

	// The low byte of m_iCollisionGroupAndType: the projectiles' collision byte (AProjectile::UpdateLifeTime)
	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF00 ) | 0x20 );
}

// $deBlob: FUNCTION 007af290
void AJetbikeSpawner::Destroy()
{
	TMaterial* pMaterial = sm_pMaterial;

	if ( pMaterial != TNULL )
	{
		// OnDestroy and delete
		pMaterial->Destroy();
		sm_pMaterial = TNULL;
	}

	if ( m_pCollisionObject != TNULL )
		delete m_pCollisionObject;

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 007af2e0
void AJetbikeSpawner::ProcessParams( const AEntityParams& a_rParams )
{
	AVisibleEntity::ProcessParams( a_rParams );

	TFLOAT fTriggerDistance;

	if ( a_rParams.GetValue( fTriggerDistance, "Trigger Distance" ) )
	{
		m_uiJSFlags |= JSFLAG_HasTriggerDistance | JSFLAG_Enabled;
		m_fTriggerDistance = fTriggerDistance;
	}

	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );

	if ( rEntityData.GetTargetCount() > 0 )
	{
		// The jet is the first target
		TINT iID = rEntityData.GetTarget( 0 );
		TASSERT( ABaseEntity::FindIDEntity( iID )->IsA( &TGetClass( AInkJet ) ) );

		m_pInkJet = TSTATICCAST( AInkJet, ABaseEntity::FindIDEntity( iID ) );

		m_pInkJet->SetSpawned();
	}

	TASSERT( m_pInkJet, "No jetbike linked to jetbike spawner!" );
}

// $deBlob: FUNCTION 007af440
void AJetbikeSpawner::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE:
		case AEntityIOPacket::MESSAGE_CHALLENGESTART:
			if ( ( m_uiJSFlags & JSFLAG_Deactivated ) == 0 )
			{
				m_Flags &= ~FLAG_DontTick;
				m_uiJSFlags |= JSFLAG_Enabled;
				m_fTimer = 0.0f;
			}
			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
			// Touched by a blob: only one that Z-jumps (state 5) resets the spawner
			if ( ( a_pCaller->GetFlags() & 0x10 ) == 0 )
				break;

			TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );

			if ( TSTATICCAST( ABlobEntity, a_pCaller )->GetZJumpState() != ABlobZJumpComponent::STATE_ATTACKSLAM )
				break;

			TFALLTHROUGH;
		case AEntityIOPacket::MESSAGE_SLAM:
			if ( GetState() == 1 || GetState() == 3 || GetState() == 2 )
			{
				m_fTimer = 5.0f;
				SetState( 0 );
				m_uiJSFlags |= JSFLAG_Reset;
				m_fTimer = 10.0f;

				// A prepared jet still goes
				if ( ( m_uiJSFlags & JSFLAG_JetPrepared ) != 0 )
					LaunchJet();

				m_uiJSFlags &= ~JSFLAG_JetPrepared;
			}
			break;
		case AEntityIOPacket::MESSAGE_TOGGLE:
			// Deactivated for good
			m_fTimer = 5.0f;
			SetState( 0 );

			if ( ( m_uiJSFlags & JSFLAG_JetPrepared ) != 0 && ( m_pInkJet->GetFlags() & FLAG_Dead ) == 0 )
				m_pInkJet->Explode();

			m_uiJSFlags = ( m_uiJSFlags & ~( JSFLAG_Deactivated | JSFLAG_JetPrepared ) ) | JSFLAG_Deactivated;
			break;
	}

	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007af620
void AJetbikeSpawner::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( ( a_iLOD & 0x200 ) != 0 && GetState() != 0 )
		SetState( 0 );
}

// $deBlob: FUNCTION 007af650
void AJetbikeSpawner::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );

	m_fTimer = 5.0f;
	SetState( 0 );
	SetTicking( TFALSE );

	m_uiJSFlags &= ~( JSFLAG_Deactivated | JSFLAG_Reset | JSFLAG_JetPrepared );
	m_Flags &= ~FLAG_DontTick;

	if ( ( m_uiJSFlags & JSFLAG_HasTriggerDistance ) == 0 )
		m_uiJSFlags &= ~JSFLAG_Enabled;
}

// $deBlob: FUNCTION 007af6b0
void AJetbikeSpawner::Enable()
{
	if ( ( m_uiJSFlags & JSFLAG_Deactivated ) == 0 )
	{
		m_uiJSFlags |= JSFLAG_Enabled;
		m_fTimer = 0.0f;
		m_Flags &= ~FLAG_DontTick;
	}
}

// $deBlob: FUNCTION 007af7b0
void AJetbikeSpawner::RestartTimer()
{
	m_fTimer = 5.0f;
	SetState( 0 );
}

// $deBlob: FUNCTION 007af6e0
void AJetbikeSpawner::Despawn( TBOOL a_bPlayEffect )
{
	AVisibleEntity::Despawn( a_bPlayEffect );

	if ( m_pInkJet != TNULL )
		m_pInkJet->Despawn( a_bPlayEffect );
}

// $deBlob: FUNCTION 007af820
TBOOL AJetbikeSpawner::IsDead() const
{
	return ( m_uiJSFlags & JSFLAG_Reset ) != 0;
}

// $deBlob: FUNCTION 007af810
ACollisionObject* AJetbikeSpawner::GetCollisionObject()
{
	return m_pCollisionObject;
}

// $deBlob: FUNCTION 007af6a0
TBOOL AJetbikeSpawner::IsTargetable()
{
	return 0 != GetState();
}

// $deBlob: FUNCTION 007af710
const TCHAR* AJetbikeSpawner::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}
