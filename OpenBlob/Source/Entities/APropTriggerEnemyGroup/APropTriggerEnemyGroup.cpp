#include "pch.h"
#include "APropTriggerEnemyGroup.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityFactory.h"
#include "Entities/ACharEntity.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerEnemyGroup ) == 0xEC );
TSTATICASSERT( sizeof( APropTriggerEnemyGroup::EnemyInfo ) == 0x1C );
TSTATICASSERT( sizeof( APropTriggerEnemyGroup::SpawnedEnemy ) == 0x8 );

// The entity's flag 0x80: set by the AEnemyWrapper ctor (ABaseEntity.h "IsEnemyWrapper")
static TBOOL APropTriggerEnemyGroup_IsEnemyWrapper( ABaseEntity* a_pEntity )
{
	return ( a_pEntity->GetFlags() >> 7 ) & 1;
}

// $deBlob: FUNCTION 0077f4c0
// $deBlob: FUNCTION 0077f4d0
// $deBlob: FUNCTION 0077f540
// $deBlob: FUNCTION 0077f5a0
// $deBlob: FUNCTION 0077f5b0
// $deBlob: FUNCTION 00540ae0
// $deBlob: FUNCTION 00859d30
TDEFINE_CLASS( APropTriggerEnemyGroup );

// $deBlob: FUNCTION 0077f5c0
APropTriggerEnemyGroup::APropTriggerEnemyGroup()
{
	m_iDropRange      = 40;
	m_iNumEnemies     = 0;
	m_iEnemyToSpawn   = 0;
	m_pEnemyInfo      = TNULL;
	m_pSpawnedEnemies = TNULL;
	m_uiNumWaves      = 1;
	m_uiCurrentWave   = 0;
	m_fDelay          = 0.0f;
	m_fTimer          = 0.0f;
	m_fPickupChance   = 0.5f;
	m_fHealthChance   = 0.5f;
	m_iPickupCount    = 1;
	m_iDropHeight     = 90;
	m_uiGroupFlags    = 0;
}

// $deBlob: FUNCTION 00780350
// $deBlob: FUNCTION 0077f660
// $deBlob: FUNCTION 00780340
APropTriggerEnemyGroup::~APropTriggerEnemyGroup()
{
}

// $deBlob: FUNCTION 0077f680
// $deBlob: FUNCTION 007803d6
void APropTriggerEnemyGroup::ProcessParams( const AEntityParams& a_rParams )
{
	APropTrigger::ProcessParams( a_rParams );

	a_rParams.GetValue( m_uiNumWaves, "EnemyWavesCount" );
	a_rParams.GetValue( m_fDelay, "Delay" );
	m_fTimer = m_fDelay;

	TBOOL bSpawnOnTriggerOnly = TFALSE;
	a_rParams.GetValue( bSpawnOnTriggerOnly, "SpawnOnTriggerOnly" );
	m_uiGroupFlags |= bSpawnOnTriggerOnly ? GROUPFLAG_SpawnOnTriggerOnly : 0;

	a_rParams.GetValue( m_iPickupCount, "PickupCount" );
	a_rParams.GetValue( m_fPickupChance, "PickupChance" );
	a_rParams.GetValue( m_fHealthChance, "HealthChance" );

	TINT iValue;

	if ( a_rParams.GetValue( iValue, "DropHeight" ) )
		m_iDropHeight = TINT16( iValue );

	if ( a_rParams.GetValue( iValue, "DropRange" ) )
		m_iDropRange = TINT16( iValue );

	TBOOL bDespawn = TFALSE;
	a_rParams.GetValue( bDespawn, "DespawnEnemiesOnChallengeReset" );
	m_uiGroupFlags |= bDespawn ? GROUPFLAG_DespawnEnemiesOnChallengeReset : 0;

	// A negative chance takes the type data's (0x04 the pickup chance, 0x08 the health chance)
	TFLOAT* pTypeChances = TREINTERPRETCAST( TFLOAT*, m_pTypeData->m_pTRBData );

	if ( m_fPickupChance < 0.0f )
		m_fPickupChance = pTypeChances[ 1 ];

	if ( m_fHealthChance < 0.0f )
		m_fHealthChance = pTypeChances[ 2 ];
}

// $deBlob: FUNCTION 0077f7c0
// $deBlob: FUNCTION 007803cb
void APropTriggerEnemyGroup::PostCreate()
{
	APropTrigger::PostCreate();

	// The enemies: the entity targets that are enemy wrappers
	m_iNumEnemies = 0;

	for ( TINT i = 0; i < GetNumEntityTargets(); i++ )
	{
		if ( APropTriggerEnemyGroup_IsEnemyWrapper( GetEntityTarget( i ) ) )
			m_iNumEnemies++;
	}

	TASSERT( TNULL == m_pEnemyInfo );
	m_pEnemyInfo = new EnemyInfo[ m_iNumEnemies ];

	TASSERT( TNULL == m_pSpawnedEnemies );
	m_pSpawnedEnemies = new SpawnedEnemy[ m_iNumEnemies ];

	for ( TINT i = m_iNumEnemies - 1; i >= 0; i-- )
	{
		m_pSpawnedEnemies[ i ].pEnemyWrapper = TNULL;
		m_pSpawnedEnemies[ i ].pEnemyInfo    = TNULL;
	}

	TINT iEnemy = 0;

	for ( TINT i = 0; i < GetNumEntityTargets(); i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( APropTriggerEnemyGroup_IsEnemyWrapper( pTarget ) )
		{
			AEnemyWrapper* pWrapper = pTarget->CastTo<AEnemyWrapper>();

			EnemyInfo& rInfo   = m_pEnemyInfo[ iEnemy++ ];
			rInfo.m_iTarget    = i;
			rInfo.m_eEnemyType = pWrapper->GetEnemyType();
			rInfo.m_vPosition  = pWrapper->GetMatrix().GetTranslation();

			pWrapper->SetOwnerTrigger( this );
		}
	}
}

// $deBlob: FUNCTION 0077fa10
void APropTriggerEnemyGroup::Destroy()
{
	delete[] m_pEnemyInfo;
	m_pEnemyInfo = TNULL;

	delete[] m_pSpawnedEnemies;
	m_pSpawnedEnemies = TNULL;

	// NOTE: APropTrigger has no Destroy of its own
	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 0077fa50
void APropTriggerEnemyGroup::Tick( TFLOAT a_fDeltaTime )
{
	APropTrigger::Tick( a_fDeltaTime );

	TUINT8 uiFlags = m_uiGroupFlags;
	TINT   eState  = uiFlags & GROUPFLAG_StateMask;

	if ( eState == TRIGGERSTATE_Spawn )
	{
		if ( !( uiFlags & GROUPFLAG_NoSpawn ) )
			SpawnEnemyGroup( a_fDeltaTime );
	}
	else if ( eState == TRIGGERSTATE_Spawned )
	{
		// Waits until every enemy of the wave is gone
		TBOOL bAllDead = TTRUE;

		for ( TINT i = 0; i < m_iNumEnemies; i++ )
		{
			SpawnedEnemy&  rSpawned = m_pSpawnedEnemies[ i ];
			AEnemyWrapper* pWrapper = rSpawned.pEnemyWrapper;

			if ( pWrapper == TNULL )
			{
				if ( rSpawned.pEnemyInfo != TNULL && rSpawned.pEnemyInfo->m_eState == EnemyInfo::STATE_Suicider )
					bAllDead = TFALSE;
			}
			else
			{
				TUINT8 uiWrapperFlags = pWrapper->GetFlags();

				if ( !( uiWrapperFlags & AEnemyWrapper::WFLAG_Released ) && ( !( uiWrapperFlags & AEnemyWrapper::WFLAG_HasOwner ) || ( uiWrapperFlags & AEnemyWrapper::WFLAG_Activated ) ) )
				{
					bAllDead = TFALSE;
				}
				else
				{
					rSpawned.pEnemyInfo->m_eState        = EnemyInfo::STATE_Dead;
					m_pSpawnedEnemies[ i ].pEnemyWrapper = TNULL;
				}
			}
		}

		if ( bAllDead )
			SpawnGroupAllDeadUpdate( a_fDeltaTime );
	}
	else if ( eState == TRIGGERSTATE_Delay )
	{
		m_fTimer -= a_fDeltaTime;

		if ( m_fTimer <= 0.0f )
		{
			m_fTimer = m_fDelay;

			if ( uiFlags & GROUPFLAG_SpawnOnTriggerOnly )
				m_uiGroupFlags = uiFlags & ~GROUPFLAG_StateMask;
			else
				m_uiGroupFlags = ( uiFlags & 0xF9 ) | TRIGGERSTATE_Spawn;
		}
	}
}

// $deBlob: FUNCTION 0077fb80
void APropTriggerEnemyGroup::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	TINT eMessage = a_rPacket.m_eMessage;

	if ( eMessage == AEntityIOPacket::MESSAGE_STOP )
	{
		m_uiGroupFlags |= GROUPFLAG_NoSpawn;
		m_Flags &= ~0x4000;
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_ENEMYKILLED )
	{
		// The enemy of a wrapper left the group
		m_Flags &= ~0x4000;
		FlagEnemyAsSuicider( a_pCaller->CastTo<AEnemyWrapper>() );
		APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}
	else if ( ( eMessage == AEntityIOPacket::MESSAGE_TOUCH && ( TUINT8( a_pCaller->GetFlags() >> 4 ) & 1 ) ) || eMessage == AEntityIOPacket::MESSAGE_TRIGGER )
	{
		m_Flags &= ~0x4000;

		if ( m_uiGroupFlags & GROUPFLAG_NoSpawn )
			return;

		APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGEINIT )
	{
		m_uiGroupFlags |= GROUPFLAG_Disabled;
		APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGERESET )
	{
		m_uiGroupFlags |= GROUPFLAG_KeepEnemies;
		APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
		return;
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGESTART )
	{
		// Triggers it
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
		oPacket.m_iData    = 0;
		APropTrigger::HandleEntityIO( a_pCaller, oPacket );
		return;
	}

	APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 0077fc90
void APropTriggerEnemyGroup::ReleaseEnemy( AEnemyWrapper* a_pWrapper )
{
	for ( TINT i = 0; i < m_iNumEnemies; i++ )
	{
		if ( m_pSpawnedEnemies[ i ].pEnemyWrapper == a_pWrapper )
		{
			m_pSpawnedEnemies[ i ].pEnemyInfo->m_eState = EnemyInfo::STATE_Dead;
			m_pSpawnedEnemies[ i ].pEnemyWrapper        = TNULL;
			return;
		}
	}
}

// $deBlob: FUNCTION 0077fcf0
void APropTriggerEnemyGroup::Reset( ABaseEntity* a_pEntity )
{
	APropTrigger::Reset( a_pEntity );

	m_uiGroupFlags &= 0xD8;
	TUINT8 uiFlags  = m_uiGroupFlags;
	m_fTimer        = m_fDelay;
	m_iEnemyToSpawn = 0;
	m_uiCurrentWave = 0;

	for ( TINT i = 0; i < m_iNumEnemies; i++ )
	{
		TBOOL bClear = TTRUE;

		if ( uiFlags & GROUPFLAG_KeepEnemies )
		{
			// The spawned enemies stay, unless the challenge reset despawns them
			if ( m_pEnemyInfo[ i ].m_eState == EnemyInfo::STATE_Spawned && !( uiFlags & GROUPFLAG_DespawnEnemiesOnChallengeReset ) )
				bClear = TFALSE;
			else if ( ( uiFlags & GROUPFLAG_DespawnEnemiesOnChallengeReset ) && m_pSpawnedEnemies[ i ].pEnemyWrapper != TNULL )
				m_pSpawnedEnemies[ i ].pEnemyWrapper->Deactivate();
		}

		if ( bClear )
		{
			m_pEnemyInfo[ i ].m_eState           = EnemyInfo::STATE_None;
			m_pSpawnedEnemies[ i ].pEnemyWrapper = TNULL;
			m_pSpawnedEnemies[ i ].pEnemyInfo    = TNULL;
		}
	}

	m_uiGroupFlags &= ~GROUPFLAG_KeepEnemies;
	SetTicking( TFALSE );
}

// The enemies of the group that are spawned
// $deBlob: FUNCTION 0077fdd0
TINT APropTriggerEnemyGroup::GetNumSpawnedEnemies() const
{
	TINT iCount = 0;

	for ( TINT i = 0; i < m_iNumEnemies; i++ )
	{
		if ( m_pSpawnedEnemies[ i ].pEnemyWrapper != TNULL )
			iCount++;
	}

	return iCount;
}

// $deBlob: FUNCTION 0077fdf0
TBOOL APropTriggerEnemyGroup::CanObjectTrigger( TObject* a_pObject )
{
	if ( m_uiGroupFlags & GROUPFLAG_Disabled )
		return TFALSE;

	TBOOL bCanTrigger = APropTrigger::CanObjectTrigger( a_pObject );

	// The characters (the enemies themselves) don't trigger it
	if ( bCanTrigger && a_pObject->IsA( &TGetClass( ACharEntity ) ) )
		bCanTrigger = TFALSE;

	return bCanTrigger;
}

// $deBlob: FUNCTION 0077fe40
void APropTriggerEnemyGroup::Trigger()
{
	TUINT8 uiFlags = m_uiGroupFlags;

	if ( uiFlags & GROUPFLAG_Disabled )
	{
		m_uiCurrentWave = 0;
		m_Flags &= ~0x4000;
		m_uiGroupFlags = uiFlags & ~GROUPFLAG_StateMask;
	}

	if ( ( m_uiCurrentWave < m_uiNumWaves || m_uiNumWaves == 0 || ( m_uiGroupFlags & GROUPFLAG_SpawnOnTriggerOnly ) ) && ( m_uiGroupFlags & GROUPFLAG_StateMask ) == TRIGGERSTATE_Idle )
		m_uiGroupFlags = ( m_uiGroupFlags & 0xF9 ) | TRIGGERSTATE_Spawn;

	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 0077fea0
void APropTriggerEnemyGroup::SpawnEnemy( SpawnedEnemy& a_rSpawned, EnemyInfo* a_pInfo )
{
	ABaseEntity* pWrapper = GetEntityTarget( a_pInfo->m_iTarget );
	TASSERT( APropTriggerEnemyGroup_IsEnemyWrapper( pWrapper ) );

	TINT ePickupType;
	TINT iPickupCount;
	GetPickupAttribs( ePickupType, iPickupCount );

	a_rSpawned.pEnemyWrapper = pWrapper->CastTo<AEnemyWrapper>();
	a_rSpawned.pEnemyWrapper->Activate( ePickupType, iPickupCount );

	if ( a_rSpawned.pEnemyInfo == TNULL )
	{
		a_rSpawned.pEnemyInfo = a_pInfo;
		a_pInfo->m_eState     = EnemyInfo::STATE_Spawned;
		return;
	}

	a_rSpawned.pEnemyInfo->m_eState = EnemyInfo::STATE_Spawned;
}

// $deBlob: FUNCTION 0077ff90
void APropTriggerEnemyGroup::AddEnemyInfo( TINT a_iTarget, TINT a_eEnemyType, const TVector4& a_rPosition, TINT a_iIndex )
{
	m_pEnemyInfo[ a_iIndex ].m_iTarget    = a_iTarget;
	m_pEnemyInfo[ a_iIndex ].m_eEnemyType = a_eEnemyType;
	m_pEnemyInfo[ a_iIndex ].m_vPosition  = a_rPosition;
}

// $deBlob: FUNCTION 0077ffe0
void APropTriggerEnemyGroup::SpawnGroupAllDeadUpdate( TFLOAT a_fDeltaTime )
{
	TUINT8 uiFlags;
	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer > 0.0f )
	{
		uiFlags = ( m_uiGroupFlags & 0xFA ) | TRIGGERSTATE_Delay;
	}
	else
	{
		m_fTimer = m_fDelay;
		uiFlags  = m_uiGroupFlags;

		if ( uiFlags & GROUPFLAG_SpawnOnTriggerOnly )
			uiFlags &= ~GROUPFLAG_StateMask;
		else
			uiFlags = ( uiFlags & 0xF9 ) | TRIGGERSTATE_Spawn;
	}

	TUINT8 uiNumWaves = m_uiNumWaves;
	m_uiGroupFlags    = uiFlags;

	if ( uiNumWaves == 0 )
	{
		// Endless waves: the targets hear about each one
		TriggerAllTargets( this );
		return;
	}

	m_uiCurrentWave++;

	if ( uiNumWaves <= m_uiCurrentWave && !( m_uiGroupFlags & GROUPFLAG_SpawnOnTriggerOnly ) )
	{
		// The last wave is dead
		m_uiGroupFlags = ( m_uiGroupFlags & 0xFC ) | TRIGGERSTATE_Finished;
		TriggerAllTargets( this );
		SetTicking( TFALSE );
	}
}

// $deBlob: FUNCTION 007800a0
void APropTriggerEnemyGroup::SpawnEnemyGroup( TFLOAT a_fDeltaTime )
{
	if ( m_iNumEnemies == 0 )
		return;

	for ( ; m_iEnemyToSpawn < m_iNumEnemies; m_iEnemyToSpawn++ )
	{
		EnemyInfo* it = &m_pEnemyInfo[ m_iEnemyToSpawn ];

		if ( it->m_eState != EnemyInfo::STATE_Spawned && ( !( m_uiTriggerFlags & TRIGGERFLAG_TriggerOnce ) || m_uiCurrentWave < m_uiNumWaves || m_uiNumWaves == 0 ) )
		{
			TASSERT( m_pSpawnedEnemies[ m_iEnemyToSpawn ].pEnemyWrapper == TNULL );
			TASSERT( !it->IsSuicider() || m_pSpawnedEnemies[ m_iEnemyToSpawn ].pEnemyInfo == &( *it ) );

			SpawnEnemy( m_pSpawnedEnemies[ m_iEnemyToSpawn ], it );
		}
	}

	m_iEnemyToSpawn = 0;
	m_uiGroupFlags  = ( m_uiGroupFlags & 0xFB ) | TRIGGERSTATE_Spawned;
}

// $deBlob: FUNCTION 007801e0
TBOOL APropTriggerEnemyGroup::AbleToSpawn( TFLOAT a_fDeltaTime )
{
	m_fTimer -= a_fDeltaTime;

	if ( m_fTimer > 0.0f )
		return TFALSE;

	m_fTimer = m_fDelay;
	return TTRUE;
}

// The pickups of the next spawned enemy: m_iPickupCount with the chance m_fPickupChance (the type is always -1)
// $deBlob: FUNCTION 00780220
void APropTriggerEnemyGroup::GetPickupAttribs( TINT& a_reType, TINT& a_riCount )
{
	if ( ARandom::GetSingleton()->GetRandom().GetFloat() <= m_fPickupChance )
	{
		a_riCount = m_iPickupCount;
		a_reType  = -1;
		return;
	}

	a_riCount = 0;
	a_reType  = -1;
}

// $deBlob: FUNCTION 007802a0
void APropTriggerEnemyGroup::FlagEnemyAsSuicider( AEnemyWrapper* a_pWrapper )
{
	for ( TINT i = 0; i < m_iNumEnemies; i++ )
	{
		if ( m_pSpawnedEnemies[ i ].pEnemyWrapper == a_pWrapper )
		{
			m_pSpawnedEnemies[ i ].pEnemyInfo->m_eState = EnemyInfo::STATE_Suicider;
			m_pSpawnedEnemies[ i ].pEnemyWrapper        = TNULL;
			return;
		}
	}
}
