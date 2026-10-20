#include "pch.h"
#include "AStreetSpawner.h"
#include "Entities/ASquad/ASquad.h"
#include "Animation/AModelInstance2.h"
#include "Cameras/ACamera.h"
#include "Collision/ACollisionInstance2.h"
#include "Collision/ACollisionObject.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/AEntityParams.h"
#include "Entities/Enemies/AEnemy.h"
#include "Entities/Enemies/AEnemyManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Terrain2/ATerrain2.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT                    AStreetSpawner::ms_iNumSpawned    = 0;
Toshi::TMaterial*       AStreetSpawner::sm_pMaterial      = TNULL;
Toshi::TParticleEffect* AStreetSpawner::sm_despawnEffect  = TNULL;
TINT                    AStreetSpawner::ms_iMaxNumSpawned = 10;
TFLOAT                  AStreetSpawner::ms_fSpawnRange    = 175.0f;

TSTATICASSERT( sizeof( AStreetSpawner ) == 0xC4 );

// The flag of a terrain instance whose model is ready (CellInstance::m_uiFlags, as in AEnemyWrapper)
static TCONSTEXPR_VAR TUINT32 INSTANCEFLAG_Unk1000000 = 0x1000000;

// $deBlob: FUNCTION 00792210
// $deBlob: FUNCTION 00792220
// $deBlob: FUNCTION 00792290
// $deBlob: FUNCTION 007922f0
// $deBlob: FUNCTION 00792300
// $deBlob: FUNCTION 00540ca0
// $deBlob: FUNCTION 00859db0
TDEFINE_CLASS( AStreetSpawner );

// $deBlob: FUNCTION 00792310
AStreetSpawner::AStreetSpawner()
{
	m_iMaxSpawns          = 0;
	m_iNumSpawns          = 0;
	m_fnOldRenderCallback = TNULL;
	m_pOldRenderUserData  = TNULL;
	m_ppSpawns            = TNULL;
	m_pSquad              = TNULL;
	m_pCollisionObject    = TNULL;
	m_fSpawnDelay         = 0.0f;
	m_iLastSpawn          = -1;
	m_uiSSFlags           = 0;
	m_uiInkyType          = 0;
	m_iLinkedInstance     = -1;
	m_fTimer              = 0.0f;
	SetState( 0 );
}

// $deBlob: FUNCTION 007933d0
AStreetSpawner::~AStreetSpawner()
{
	sm_despawnEffect = TNULL;
}

// $deBlob: FUNCTION 007923d0
void __stdcall AStreetSpawner::RenderCallback( TModelInstance* a_pModelInstance, void* a_pSpawner )
{
	AStreetSpawner* pSpawner = TSTATICCAST( AStreetSpawner, a_pSpawner );

	if ( ( pSpawner->m_uiSSFlags & SSFLAG_Deactivated ) == 0 )
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

TBOOL AStreetSpawner::CanSpawn() const
{
	TerrainTypes::CellInstance* pInstance          = TNULL;
	ACollisionInstance2*        pCollisionInstance = TNULL;

	if ( m_iLinkedInstance != -1 )
	{
		pInstance = ATerrain2::GetInstance()->GetCellInstance( m_iLinkedInstance );

		if ( pInstance != TNULL && pInstance->m_pModelInstance2 != TNULL )
			pCollisionInstance = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance() );
	}

	return m_iNumSpawns < m_iMaxSpawns && ms_iNumSpawned + 1 <= ms_iMaxNumSpawned &&
	    ( pInstance == TNULL || ( ( pInstance->m_uiFlags & INSTANCEFLAG_Unk1000000 ) != 0 && pCollisionInstance != TNULL && pCollisionInstance->IsInSimulation( ACollisionInstance2::PhysicsObjectType_Standard ) ) );
}

// $deBlob: FUNCTION 007925a0
void AStreetSpawner::Tick( TFLOAT a_fDeltaTime )
{
	AVisibleEntity::Tick( a_fDeltaTime );
	m_uiSSFlags &= ~SSFLAG_Reset;

	switch ( GetState() )
	{
		case 0:
		{
			SetTicking( TFALSE );

			if ( ( m_uiSSFlags & SSFLAG_HasSpawnDelay ) == 0 || ( m_uiSSFlags & SSFLAG_Enabled ) == 0 || ( m_uiSSFlags & SSFLAG_Deactivated ) != 0 )
			{
				m_Flags |= FLAG_DontTick;
				return;
			}

			m_fTimer -= a_fDeltaTime;

			if ( m_fTimer >= 0.0f )
				return;

			m_fTimer = 0.0f;

			if ( CanSpawn() )
			{
				// A player's camera must be in range
				AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
				TFLOAT               fRangeSq         = ms_fSpawnRange * ms_fSpawnRange;

				for ( TINT i = 0; i < AXYZViewportManager::VIEWPORT_Count; i++ )
				{
					if ( pViewportManager->IsViewportUsed( AXYZViewportManager::VIEWPORT( i ) ) )
					{
						ACamera* pCamera = pViewportManager->GetCamera( AXYZViewportManager::VIEWPORT( i ) );

						if ( pCamera != TNULL && TVector4::DistanceSq( pCamera->GetMatrix().GetTranslation(), GetTranslation() ) <= fRangeSq )
						{
							SetTicking( TTRUE );
							SetState( 1 );
							return;
						}
					}
				}
			}

			m_fTimer = m_fSpawnDelay;
			return;
		}
		case 1:
			if ( HasStateFinished() )
			{
				if ( ( m_uiSSFlags & SSFLAG_Enabled ) != 0 )
				{
					SetState( 2 );
					m_fTimer = 1.0f;
				}
				else
				{
					SetState( 3 );
				}
			}
			return;
		case 2:
		{
			TFLOAT fOldTimer = m_fTimer;
			m_fTimer         = fOldTimer - a_fDeltaTime;

			// The enemy spawns at half a second
			if ( fOldTimer > 0.5f && m_fTimer < 0.5f && CanSpawn() )
			{
				TVector4 vPosition = GetTranslation();
				vPosition.y        = GetTranslation().y - 1.4f;

				TVector4 vFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
				vFacing.w        = 1.0f;

				AEnemyManager::EnemyType eType = AEnemyManager::EnemyType( ( m_uiInkyType == 0 ) ? 0 : 3 );

				// NOTE: retail passes the spawner itself as the wrapper of the enemy
				AEnemy* pEnemy = AEnemyManager::GetSingleton()->CreateEnemy( eType, vPosition, vFacing, TREINTERPRETCAST( AEnemyWrapper*, this ), -1, 0, TFALSE, m_pSquad, m_uiInkyType );

				if ( pEnemy != TNULL )
				{
					TINT i = 0;

					for ( ; i < m_iMaxSpawns; i++ )
					{
						if ( m_ppSpawns[ i ] == TNULL )
						{
							m_ppSpawns[ i ] = pEnemy;
							break;
						}
					}

					TASSERT( i < m_iMaxSpawns );
					m_iNumSpawns++;
					ms_iNumSpawned++;
					m_iLastSpawn = TINT16( i );
				}
			}

			if ( m_fTimer < 0.0f )
				SetState( 3 );

			return;
		}
		case 3:
			// The last spawn rises with the spawner
			if ( m_iLastSpawn != -1 && m_ppSpawns[ m_iLastSpawn ] != TNULL )
			{
				TVector4 vPosition = GetTranslation();
				vPosition.y        = GetTranslation().y - 1.4f;
				m_ppSpawns[ m_iLastSpawn ]->SetPosition( vPosition );
			}

			if ( HasStateFinished() )
			{
				m_iLastSpawn = -1;
				m_fTimer     = m_fSpawnDelay;
				SetState( 0 );
			}
			return;
	}
}

// $deBlob: FUNCTION 007929b0
void AStreetSpawner::Create()
{
	AVisibleEntity::Create();

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
		// The spawner colour material: a copy of the first mesh's material with spawner_colour.tga
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
		pMaterial->SetTexture( 0, TTextureManager::GetSingleton()->FindTexture( "spawner_colour.tga" ) );
	}
}

// $deBlob: FUNCTION 00793320
void AStreetSpawner::CreateCollisionObject()
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

// $deBlob: FUNCTION 00792cc0
void AStreetSpawner::Destroy()
{
	TMaterial* pMaterial = sm_pMaterial;

	if ( pMaterial != TNULL )
	{
		// OnDestroy and delete
		pMaterial->Destroy();
		sm_pMaterial = TNULL;
	}

	delete[] m_ppSpawns;

	if ( m_pCollisionObject != TNULL )
		delete m_pCollisionObject;

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 00792d10
void AStreetSpawner::ProcessParams( const AEntityParams& a_rParams )
{
	if ( sm_despawnEffect == TNULL )
	{
		const TCHAR* szDespawnEffect = m_pTypeData->m_pTRBData->m_szDespawnEffect;

		if ( szDespawnEffect != TNULL && *szDespawnEffect != '\0' )
		{
			sm_despawnEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( szDespawnEffect );
			TASSERT( TDebug_IsValidAddress( sm_despawnEffect ), "Couldn't find despawn particle effect %s for street spawner!" );
		}
	}

	AVisibleEntity::ProcessParams( a_rParams );

	TUINT32 uiMaxSpawns = 0;

	if ( a_rParams.GetUInt( uiMaxSpawns, "MaxSpawns" ) )
	{
		m_iMaxSpawns = TINT16( uiMaxSpawns );
		m_ppSpawns   = new AEnemy*[ uiMaxSpawns ];

		for ( TINT i = 0; i < TINT( uiMaxSpawns ); i++ )
			m_ppSpawns[ i ] = TNULL;
	}

	TUINT32 uiInkyType = 0;

	if ( a_rParams.GetUInt( uiInkyType, "Inky Type" ) )
		m_uiInkyType = TUINT8( uiInkyType );

	// The high byte of m_iCollisionGroupAndType
	TUINT8 uiPriorityType    = ( m_uiInkyType == 0 ) ? 21 : 30;
	m_iCollisionGroupAndType = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( uiPriorityType << 8 ) );

	TFLOAT fSpawnDelay;

	if ( a_rParams.GetValue( fSpawnDelay, "Spawn Delay" ) )
	{
		m_uiSSFlags |= SSFLAG_HasSpawnDelay;
		m_fSpawnDelay = fSpawnDelay;

		TBOOL bStartEnabled = TTRUE;
		a_rParams.GetBool( bStartEnabled, "Start Enabled" );

		if ( bStartEnabled )
			m_uiSSFlags |= SSFLAG_StartEnabled | SSFLAG_Enabled;
	}

	TINT iLinkedInstance = -1;

	if ( a_rParams.GetValue( iLinkedInstance, "LinkedInstance" ) )
		m_iLinkedInstance = TINT16( iLinkedInstance );
}

// $deBlob: FUNCTION 00792eb0
void AStreetSpawner::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE:
		case AEntityIOPacket::MESSAGE_CHALLENGESTART:
			Enable();
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
				m_fTimer = m_fSpawnDelay;
				SetState( 0 );
				m_uiSSFlags |= SSFLAG_Reset;
				m_fTimer = m_fSpawnDelay * 2.0f;
			}
			break;
		case AEntityIOPacket::MESSAGE_ENEMYRELEASED:
		case AEntityIOPacket::MESSAGE_CULLED:
		case AEntityIOPacket::MESSAGE_RECLAIM:
		{
			// One of the spawned enemies is gone
			TINT i = 0;

			for ( ; i < m_iMaxSpawns; i++ )
			{
				if ( m_ppSpawns[ i ] == TSTATICCAST( AEnemy, a_pCaller ) )
					break;
			}

			TASSERT( i < m_iMaxSpawns );
			TASSERT( a_pCaller == TNULL || a_pCaller->IsA( &TGetClass( AEnemy ) ) );

			AEnemyManager::GetSingleton()->ReleaseEnemy( TSTATICCAST( AEnemy, a_pCaller ) );

#ifdef DEBLOB_COMMUNITY_PATCH
			// NOTE: retail clears m_ppSpawns[ m_iMaxSpawns ] (past the array) when the enemy isn't one of the spawns
			if ( i < m_iMaxSpawns )
				m_ppSpawns[ i ] = TNULL;
#else
			m_ppSpawns[ i ] = TNULL;
#endif

			m_iNumSpawns--;
			ms_iNumSpawned--;
			break;
		}
		case AEntityIOPacket::MESSAGE_TOGGLE:
			// Deactivated for good
			m_fTimer = m_fSpawnDelay;
			SetState( 0 );
			m_uiSSFlags |= SSFLAG_Deactivated;

			if ( m_pSquad != TNULL )
				TSTATICCAST( ASquad, m_pSquad )->OnMemberKilled();
			break;
	}

	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00793110
void AStreetSpawner::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( ( a_iLOD & 0x200 ) != 0 && GetState() != 0 )
		SetState( 0 );
}

// $deBlob: FUNCTION 00793140
void AStreetSpawner::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );

	m_fTimer = m_fSpawnDelay;
	SetState( 0 );
	SetTicking( TFALSE );

	for ( TINT i = 0; i < m_iMaxSpawns; i++ )
		m_ppSpawns[ i ] = TNULL;

	m_uiSSFlags &= ~( SSFLAG_Reset | SSFLAG_Deactivated );
	m_iNumSpawns = 0;
	m_iLastSpawn = -1;

	if ( ( m_uiSSFlags & SSFLAG_StartEnabled ) != 0 )
	{
		m_uiSSFlags |= SSFLAG_Enabled;
		m_Flags &= ~FLAG_DontTick;
	}
	else
	{
		m_uiSSFlags &= ~SSFLAG_Enabled;
	}
}

// $deBlob: FUNCTION 007931f0
void AStreetSpawner::Enable()
{
	if ( ( m_uiSSFlags & SSFLAG_HasSpawnDelay ) != 0 )
	{
		// Spawns every m_fSpawnDelay seconds from now
		if ( ( m_uiSSFlags & ( SSFLAG_Enabled | SSFLAG_Deactivated ) ) == 0 )
		{
			m_fTimer = 0.0f;
			m_Flags &= ~FLAG_DontTick;
			m_uiSSFlags |= SSFLAG_Enabled;
		}
	}
	else if ( ( m_uiSSFlags & SSFLAG_Deactivated ) == 0 && GetState() == 0 && m_iNumSpawns < m_iMaxSpawns )
	{
		// Spawns once
		m_Flags &= ~FLAG_DontTick;
		SetTicking( TTRUE );
		SetState( 1 );
	}
}

// $deBlob: FUNCTION 00793250
void AStreetSpawner::Stop()
{
	m_uiSSFlags &= ~SSFLAG_Enabled;

	if ( 2 == GetState() )
		SetState( 3 );
}

// $deBlob: FUNCTION 00793270
void AStreetSpawner::DeactivateSpawns()
{
	for ( TINT i = 0; i < m_iMaxSpawns; i++ )
	{
		if ( m_ppSpawns[ i ] != TNULL )
			m_ppSpawns[ i ]->Despawn( TTRUE );
	}
}

// $deBlob: FUNCTION 007932b0
void AStreetSpawner::Despawn( TBOOL a_bPlayEffect )
{
	AVisibleEntity::Despawn( a_bPlayEffect );

	for ( TINT i = 0; i < m_iMaxSpawns; i++ )
	{
		if ( m_ppSpawns[ i ] != TNULL )
		{
			m_ppSpawns[ i ]->Despawn( a_bPlayEffect );
			m_ppSpawns[ i ] = TNULL;
		}
	}
}

// $deBlob: FUNCTION 007933b0
void AStreetSpawner::RestartTimer()
{
	m_fTimer = m_fSpawnDelay;
	SetState( 0 );
}

// $deBlob: FUNCTION 00793440
TBOOL AStreetSpawner::IsDead() const
{
	return ( m_uiSSFlags & SSFLAG_Reset ) != 0;
}

// $deBlob: FUNCTION 00793410
ACollisionObject* AStreetSpawner::GetCollisionObject()
{
	return m_pCollisionObject;
}

// $deBlob: FUNCTION 007931e0
TBOOL AStreetSpawner::IsTargetable()
{
	return 0 != GetState();
}

// $deBlob: FUNCTION 00793420
void* AStreetSpawner::GetDespawnEffect()
{
	return sm_despawnEffect;
}

// $deBlob: FUNCTION 00793310
const TCHAR* AStreetSpawner::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}
