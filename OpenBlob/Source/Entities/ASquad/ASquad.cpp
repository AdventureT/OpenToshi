#include "pch.h"
#include "Entities/APickupManager.h"
#include "ASquad.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"
#include "Entities/AStreetSpawner/AStreetSpawner.h"
#include "Entities/Enemies/AEnemy.h"
#include "Entities/AI/Behaviour/AAIBehaviour.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/AChallenge.h"
#include "Challenges/ACombatChallenge.h"
#include "Challenges/AUberChallenge.h"
#include "GameController/AGameStatus.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ASquad ) == 0xC8 );

// The squad tells a member's behaviour about a target (only once)
// NOTE: an AAIBehaviour method in retail (only called by ASquad); written here to keep AAIBehaviour's files untouched
// $deBlob: FUNCTION 007e1b50
static void ASquad_AddEnemy( AAIBehaviour* a_pBehaviour, ABaseEntity* a_pEnemy )
{
	if ( !a_pBehaviour->IsKnownEnemy( a_pEnemy ) )
		a_pBehaviour->AddToEnemyList( a_pEnemy );
}

// $deBlob: FUNCTION 007a3750
// $deBlob: FUNCTION 007a3760
// $deBlob: FUNCTION 007a37d0
// $deBlob: FUNCTION 00541060
// $deBlob: FUNCTION 00859ea0
TDEFINE_CLASS( ASquad );

// $deBlob: FUNCTION 007a3850
ASquad::ASquad()
    : m_iNumSquadIDs( 0 )
    , m_pSquadIDs( TNULL )
    , m_pSquadPool( TNULL )
    , m_iNumTargets( 0 )
    , m_fMinX( 0.0f )
    , m_fRangeX( -1.0f )
{
	m_apTargets[ 0 ] = TNULL;
	m_apTargets[ 1 ] = TNULL;
	m_apTargets[ 2 ] = TNULL;
	m_apTargets[ 3 ] = TNULL;

	SetHash( TFALSE );
}

// $deBlob: FUNCTION 007a3930
// $deBlob: FUNCTION 007a46d0
ASquad::~ASquad()
{
}

// $deBlob: FUNCTION 007a39a0
void ASquad::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_iNumTargets == 0 || m_oActiveSquadList.IsEmpty() )
		return;

	for ( TINT i = 0; i < MAX_TARGETS; i++ )
	{
		if ( m_apTargets[ i ] == TNULL )
			return;

		m_afTargetTime[ i ] -= a_fDeltaTime;

		if ( m_afTargetTime[ i ] < 0.0f )
		{
			// The last target takes the place of the forgotten one (and is checked again)
			TINT iLast = i + 1;

			if ( i < MAX_TARGETS - 1 )
			{
				while ( m_apTargets[ iLast ] != TNULL )
				{
					iLast++;

					if ( iLast >= MAX_TARGETS )
						break;
				}
			}

			iLast--;

			if ( i != iLast )
			{
				m_apTargets[ i ]    = m_apTargets[ iLast ];
				m_afTargetTime[ i ] = m_afTargetTime[ iLast ];
				i--;
			}

			m_apTargets[ iLast ] = TNULL;

			if ( --m_iNumTargets == 0 )
				SetTicking( TFALSE );
		}
	}
}

// $deBlob: FUNCTION 007a3a90
void ASquad::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );

	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );
	m_iNumSquadIDs                               = rEntityData.GetTargetCount();

	TASSERT( TNULL == m_pSquadIDs );
	m_pSquadIDs = new TINT[ m_iNumSquadIDs ];

	for ( TINT i = 0; i < m_iNumSquadIDs; i++ )
		m_pSquadIDs[ i ] = rEntityData.GetTarget( i );
}

// $deBlob: FUNCTION 007a3b70
void ASquad::PostCreate()
{
	TINT   iNumMembers = 0;
	TFLOAT fMaxX       = 0.0f;

	for ( TINT i = 0; i < m_iNumSquadIDs; i++ )
	{
		ABaseEntity* pEntity = FindIDEntity( m_pSquadIDs[ i ] );

		if ( pEntity->IsA( &TGetClass( AEnemyWrapper ) ) )
		{
			iNumMembers++;

			AEnemyWrapper* pWrapper = TSTATICCAST( AEnemyWrapper, pEntity );
			pWrapper->SetSquad( this );

			// NOTE: retail takes the x of the wrappers (m_oMatrix.m_f41), AEnemyWrapper::SpawnEnemy uses the range for the height
			TFLOAT fX = pWrapper->GetMatrix().m_f41;

			if ( m_fRangeX < 0.0f )
			{
				m_fMinX   = fX;
				m_fRangeX = 0.0f;
				fMaxX     = fX;
			}
			else if ( m_fMinX > fX )
			{
				m_fMinX   = fX;
				m_fRangeX = fMaxX - fX;
			}
			else if ( fX > fMaxX )
			{
				fMaxX     = fX;
				m_fRangeX = fX - m_fMinX;
			}
		}
		else if ( pEntity->IsA( &TGetClass( AStreetSpawner ) ) )
		{
			AStreetSpawner* pSpawner = pEntity->CastTo<AStreetSpawner>();
			iNumMembers += pSpawner->GetMaxSpawns();
			pSpawner->SetSquad( this );
		}
	}

	TASSERT( TNULL == m_pSquadPool );
	m_pSquadPool = new SquadNode[ iNumMembers ];

	for ( TINT i = 0; i < iNumMembers; i++ )
		m_oFreeSquadList.PushBack( &m_pSquadPool[ i ] );

	TASSERT( m_oActiveSquadList.IsEmpty() );
}

// $deBlob: FUNCTION 007a3e20
void ASquad::Destroy()
{
	m_oFreeSquadList.Clear();
	m_oActiveSquadList.Clear();

	::operator delete( m_pSquadIDs );
	m_pSquadIDs = TNULL;

	delete[] m_pSquadPool;
	m_pSquadPool = TNULL;

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 007a3e90
void ASquad::Reset( ABaseEntity* a_pEntity )
{
	m_Flags = ( m_Flags & ~FLAG_LODMask ) | FLAG_LOD0;

	if ( !m_oActiveSquadList.IsEmpty() )
		m_oFreeSquadList.Splice( m_oFreeSquadList.End(), m_oActiveSquadList );

	m_apTargets[ 0 ] = TNULL;
	m_apTargets[ 1 ] = TNULL;
	m_apTargets[ 2 ] = TNULL;
	m_apTargets[ 3 ] = TNULL;
	m_iNumTargets    = 0;
}

// $deBlob: FUNCTION 007a3f10
void ASquad::TellTargets( ABaseEntity* a_pTarget )
{
	for ( auto it = m_oActiveSquadList.Begin(); it != m_oActiveSquadList.End(); it++ )
	{
		AAIBehaviour* pBehaviour = TSTATICCAST( AAIBehaviour, it->GetNodeValue()->GetBehaviourComponent() );
		ASquad_AddEnemy( pBehaviour, a_pTarget );
	}
}

// $deBlob: FUNCTION 007a3fa0
void ASquad::AddMember( AEnemy* a_pEnemy )
{
	SquadNode* pNode = m_oFreeSquadList.PopFront();
	pNode->SetNodeValue( a_pEnemy );
	m_oActiveSquadList.PushBack( pNode );

	if ( m_iNumTargets > 0 )
	{
		SetTicking( TTRUE );

		for ( TINT i = 0; i < MAX_TARGETS; i++ )
		{
			if ( m_apTargets[ i ] == TNULL )
				return;

			ASquad_AddEnemy( TSTATICCAST( AAIBehaviour, a_pEnemy->GetBehaviourComponent() ), m_apTargets[ i ] );
		}
	}
}

// $deBlob: FUNCTION 007a4050
void ASquad::RemoveMember( AEnemy* a_pEnemy )
{
	for ( auto it = m_oActiveSquadList.Begin(); it != m_oActiveSquadList.End(); it++ )
	{
		if ( it->GetNodeValue() == a_pEnemy )
		{
			SquadNode* pNode = it;
			pNode->Remove();
			m_oFreeSquadList.PushBack( pNode );

			if ( m_oActiveSquadList.IsEmpty() )
				SetTicking( TFALSE );

			return;
		}
	}
}

// $deBlob: FUNCTION 007a4110
void ASquad::AddTarget( ABaseEntity* a_pTarget, TFLOAT a_fTime )
{
	for ( TINT i = 0; i < MAX_TARGETS; i++ )
	{
		if ( m_apTargets[ i ] == TNULL )
		{
			m_apTargets[ i ]    = a_pTarget;
			m_afTargetTime[ i ] = a_fTime;
			m_iNumTargets++;

			if ( !m_oActiveSquadList.IsEmpty() )
				SetTicking( TTRUE );

			return;
		}

		if ( a_pTarget == m_apTargets[ i ] )
		{
			if ( m_afTargetTime[ i ] < a_fTime )
				m_afTargetTime[ i ] = a_fTime;

			return;
		}
	}
}

// $deBlob: FUNCTION 007a4190
TBOOL ASquad::IsTarget( ABaseEntity* a_pTarget ) const
{
	for ( TINT i = 0; i < MAX_TARGETS; i++ )
	{
		if ( m_apTargets[ i ] == TNULL )
			return TFALSE;

		if ( a_pTarget == m_apTargets[ i ] )
			return m_afTargetTime[ i ] > 0.0f;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007a41e0
void ASquad::OnMemberKilled()
{
	if ( !AreAllMembersDead() || AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel != 0 )
		return;

	AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

	if ( pChallenge != TNULL )
	{
		// NOTE: retail doesn't check the sub challenge of an uber challenge for TNULL
		if ( pChallenge->GetType() == AChallenge::Type_Uber )
			pChallenge = TSTATICCAST( AUberChallenge, pChallenge )->GetCurrentSubChallenge();

		// No pickup for the enemies of the current combat challenge
		if ( pChallenge->GetType() == AChallenge::Type_Combat )
		{
			for ( TINT i = 0; i < m_iNumSquadIDs; i++ )
			{
				ABaseEntity* pEntity = FindIDEntity( m_pSquadIDs[ i ] );

				if ( pEntity->IsA( &TGetClass( AEnemyWrapper ) ) &&
				     TSTATICCAST( ACombatChallenge, pChallenge )->IsLinkedEnemy( pEntity->CastTo<AEnemyWrapper>() ) )
					return;
			}
		}
	}

	// A pickup near the first player
	APickupManager::SpawnPickupNearPlayer( 1, 0 );
}

// $deBlob: FUNCTION 007a42b0
TBOOL ASquad::AreAllMembersDead() const
{
	for ( TINT i = 0; i < m_iNumSquadIDs; i++ )
	{
		ABaseEntity* pEntity = FindIDEntity( m_pSquadIDs[ i ] );

		if ( pEntity->IsA( &TGetClass( AEnemyWrapper ) ) )
		{
			if ( !( pEntity->CastTo<AEnemyWrapper>()->GetFlags() & AEnemyWrapper::WFLAG_Released ) )
				return TFALSE;
		}
		else if ( pEntity->IsA( &TGetClass( AStreetSpawner ) ) )
		{
			AStreetSpawner* pSpawner = pEntity->CastTo<AStreetSpawner>();

			if ( !pSpawner->IsDeactivated() || pSpawner->GetNumSpawns() != 0 )
				return TFALSE;
		}
	}

	return TTRUE;
}
