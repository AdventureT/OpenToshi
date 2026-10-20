#include "pch.h"
#include "ACombatChallenge.h"
#include "AChallengeManager.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"
#include "Entities/APropTriggerEnemyGroup/APropTriggerEnemyGroup.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/ACar/ACar.h"
#include "Entities/APropTurret/APropTurret.h"
#include "Entities/ATankSpawner/ATankSpawner.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Does the enemy of a_pWrapper count (not a LeechBot with the custom data that keeps it out of the combat)?
static TBOOL ACombatChallenge_IsIgnoredLeechBot( AEnemyWrapper* a_pWrapper )
{
	return a_pWrapper->GetEnemyType() == AEnemyWrapper::ENEMYTYPE_LeechBot2 && ALeechBot2::IsFixedColourType( a_pWrapper->GetCustomData() );
}

// $deBlob: FUNCTION 0059c6b0
ACombatChallenge::ACombatChallenge()
    : AChallenge( Type_Combat )
{
	m_iNumEnemies  = 0;
	m_iEnemiesLeft = 0;
}

// $deBlob: FUNCTION 0059d500
TBOOL ACombatChallenge::Create( TypeData* a_pData )
{
	if ( !AChallenge::Create( a_pData ) )
		return TFALSE;

	for ( TUINT32 i = 0; i < a_pData->m_uiNumEntries; i++ )
	{
		Entry& rEntry = a_pData->m_pEntries[ i ];
		TINT   iID    = rEntry.m_iID;
		TASSERT( iID != -1 );

		ABaseEntity* pEntity = ABaseEntity::FindIDEntity( iID );

		switch ( rEntry.m_eType )
		{
			case 0:
				TASSERT( pEntity->IsA( &TGetClass( APropTriggerEnemyGroup ) ) );
				break;
			case 1:
				TASSERT( pEntity->IsA( &TGetClass( AEnemyWrapper ) ) );
				break;
			case 2:
				TASSERT( pEntity->IsA( &TGetClass( ACar ) ) );
				break;
			case 3:
				TASSERT( pEntity->IsA( &TGetClass( APropTurret ) ) );
				break;
			case 4:
				TASSERT( pEntity->IsA( &TGetClass( ATankSpawner ) ) );
				break;
			default:
				rEntry.m_iDone = TFALSE;
				continue;
		}

		rEntry.m_pEntity = pEntity;
		rEntry.m_iDone   = TFALSE;
	}

	return TTRUE;
}

// An enemy linked to the challenge was killed (a_pEntityID: the ID of its group trigger, or of its wrapper)
// $deBlob: FUNCTION 0059d770
void ACombatChallenge::OnCombatEvent( void* a_pEntityID )
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	TINT      iID   = TINT( TREINTERPRETCAST( TUINTPTR, a_pEntityID ) );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		Entry& rEntry = pData->m_pEntries[ i ];

		if ( rEntry.m_iID != iID )
			continue;

		if ( rEntry.m_eType == 0 )
		{
			// The group is done once none of its enemies is left
			APropTriggerEnemyGroup* pGroup = TSTATICCAST( APropTriggerEnemyGroup, rEntry.m_pEntity );
			TINT                    iCount = pGroup->GetNumEnemies();
			TBOOL                   bAlive = TFALSE;

			for ( TINT j = 0; j < iCount; j++ )
			{
				AEnemyWrapper* pWrapper = pGroup->GetEntityTarget( j )->CastTo<AEnemyWrapper>();

				if ( pWrapper != TNULL && pWrapper->IsAlive() && !ACombatChallenge_IsIgnoredLeechBot( pWrapper ) )
				{
					bAlive = TTRUE;
					break;
				}
			}

			if ( !bAlive )
				pData->m_pEntries[ i ].m_iDone = TTRUE;
		}
		else
		{
			rEntry.m_iDone = TTRUE;
		}

		m_iEnemiesLeft--;
	}

	AChallengeManager::GetSingleton()->SetCounter( m_iNumEnemies - m_iEnemiesLeft, m_iNumEnemies );
}

// $deBlob: FUNCTION 0059d8d0
void ACombatChallenge::Update( TFLOAT a_fDeltaTime )
{
	AChallenge::Update( a_fDeltaTime );

	if ( ( m_uiFlags & Flags_Completed ) || ( m_uiFlags & Flags_Failed ) )
		return;

	// Completed once every linked entity is done
	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	TBOOL     bDone = TTRUE;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( !pData->m_pEntries[ i ].m_iDone )
			bDone = TFALSE;
	}

	if ( bDone )
		AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );
}

// $deBlob: FUNCTION 0059d940
void ACombatChallenge::Reset( TBOOL a_bUnk )
{
	AChallenge::Reset( a_bUnk );

	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
		pData->m_pEntries[ i ].m_iDone = TFALSE;

	m_iEnemiesLeft = m_iNumEnemies;
}

// $deBlob: FUNCTION 0059d990
void ACombatChallenge::Start()
{
	AChallenge::Start();

	TypeData* pData    = TSTATICCAST( TypeData, m_pData );
	TINT      iIgnored = 0;
	m_iNumEnemies      = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		Entry& rEntry = pData->m_pEntries[ i ];
		TINT   eType  = rEntry.m_eType;

		if ( eType == 0 )
		{
			// Every enemy of the group counts (but the ignored LeechBots)
			APropTriggerEnemyGroup* pGroup  = TSTATICCAST( APropTriggerEnemyGroup, rEntry.m_pEntity );
			TINT                    iIgnore = 0;

			for ( TINT j = 0; j < pGroup->GetNumEnemies(); j++ )
			{
				AEnemyWrapper* pWrapper = pGroup->GetEntityTarget( j )->CastTo<AEnemyWrapper>();

				if ( ACombatChallenge_IsIgnoredLeechBot( pWrapper ) )
					iIgnore++;
			}

			m_iNumEnemies += pGroup->GetNumEnemies();
			iIgnored += iIgnore;

			pData->m_pEntries[ i ].m_iDone = ( iIgnore == pGroup->GetNumEnemies() );
		}
		else if ( ( eType == 1 && TSTATICCAST( AEnemyWrapper, rEntry.m_pEntity )->IsAlive() ) || ( eType == 2 && !( rEntry.m_pEntity->GetFlags() & ABaseEntity::FLAG_Dead ) ) || ( eType == 3 && !( rEntry.m_pEntity->GetFlags() & ABaseEntity::FLAG_Dead ) ) || eType == 4 )
		{
			m_iNumEnemies++;
			pData->m_pEntries[ i ].m_iDone = TFALSE;
		}
		else
		{
			// Already dead
			rEntry.m_iDone = TTRUE;
		}
	}

	m_iNumEnemies -= iIgnored;
	m_iEnemiesLeft = m_iNumEnemies;

	AChallengeManager* pManager = AChallengeManager::GetSingleton();
	TSTATICCAST( ABlobChallengeStatusHud, pManager->GetHudElement() )->OnChallengeStart( this );
	pManager->SetCounter( 0, m_iNumEnemies );
}

// The share of the enemies killed (percent); NOTE: divides by zero without enemies
// $deBlob: FUNCTION 0059db70
TINT ACombatChallenge::GetScore()
{
	return TINT( ( TFLOAT( m_iNumEnemies - m_iEnemiesLeft ) / TFLOAT( m_iNumEnemies ) ) * 100.0f );
}

// Are all the linked entities dead?
// $deBlob: FUNCTION 0059dba0
TBOOL ACombatChallenge::IsAlreadyCompleted()
{
	TypeData* pData  = TSTATICCAST( TypeData, m_pData );
	TUINT32   uiDead = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		Entry& rEntry = pData->m_pEntries[ i ];
		TINT   eType  = rEntry.m_eType;

		if ( ( eType == 1 && !TSTATICCAST( AEnemyWrapper, rEntry.m_pEntity )->IsAlive() ) ||
		     ( eType == 2 && ( rEntry.m_pEntity->GetFlags() & ABaseEntity::FLAG_Dead ) ) ||
		     ( eType == 3 && ( rEntry.m_pEntity->GetFlags() & ABaseEntity::FLAG_Dead ) ) ||
		     ( eType == 4 && TSTATICCAST( ATankSpawner, rEntry.m_pEntity )->IsComplete() ) )
		{
			uiDead++;
		}
	}

	return uiDead == pData->m_uiNumEntries;
}

// $deBlob: FUNCTION 0059dc30
TBOOL ACombatChallenge::IsLinkedEnemy( ABaseEntity* a_pEntity ) const
{
	TypeData* pData = TSTATICCAST( TypeData, m_pData );

	for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
	{
		if ( pData->m_pEntries[ i ].m_eType == 1 && pData->m_pEntries[ i ].m_pEntity == a_pEntity )
			return TTRUE;
	}

	return TFALSE;
}
