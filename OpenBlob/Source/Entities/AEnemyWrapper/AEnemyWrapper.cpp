#include "pch.h"
#include "AEnemyWrapper.h"
#include "Entities/APropTriggerEnemyGroup/APropTriggerEnemyGroup.h"
#include "Entities/ASquad/ASquad.h"
#include "ARandom.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEntityParams.h"
#include "Entities/Enemies/AEnemy.h"
#include "Entities/Enemies/AEnemyManager.h"
#include "Animation/AModelInstance2.h"
#include "Terrain2/ATerrain2.h"
#include "Collision/ACollisionInstance2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AEnemyWrapper ) == 0x90 );

// The instance flag set once a linked terrain instance is ready for its enemy (CellInstance::m_uiFlags)
static TCONSTEXPR_VAR TUINT32 INSTANCEFLAG_Unk1000000 = 0x1000000;

// $deBlob: FUNCTION 00780470
// $deBlob: FUNCTION 00780480
// $deBlob: FUNCTION 007804f0
// $deBlob: FUNCTION 00780550
// $deBlob: FUNCTION 00780560
// $deBlob: FUNCTION 00540b20
// $deBlob: FUNCTION 00859d40
TDEFINE_CLASS( AEnemyWrapper );

// $deBlob: FUNCTION 00780570
AEnemyWrapper::AEnemyWrapper()
{
	// NOTE: 0x80 isn't named in ABaseEntity::FLAG yet
	m_Flags |= 0x80;

	m_pSpawnedEnemy   = TNULL;
	m_iSpawner        = -1;
	m_iOwnerTrigger   = -1;
	m_iLinkedInstance = -1;
	m_eEnemyType      = ENEMYTYPE_INVALID;
	m_uiFlags         = 0;
	m_uiCustomData    = 0;

	SetHash( TFALSE );
	m_Flags |= FLAG_DontTick;
}

// $deBlob: FUNCTION 00781180
// $deBlob: FUNCTION 00780610
AEnemyWrapper::~AEnemyWrapper()
{
}

// $deBlob: FUNCTION 00780620
void AEnemyWrapper::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity::Tick( a_fDeltaTime );

	if ( m_uiFlags & WFLAG_WaitForLinked )
	{
		TASSERT( m_iLinkedInstance != -1 );

		if ( ATerrain2::GetInstance()->GetCellInstance( m_iLinkedInstance )->m_uiFlags & INSTANCEFLAG_Unk1000000 )
		{
			SpawnEnemy( -1, 0, TFALSE );
			m_Flags |= FLAG_DontTick;
			m_uiFlags &= ~WFLAG_WaitForLinked;
		}
	}
}

// $deBlob: FUNCTION 00780820
void AEnemyWrapper::OnLODChanging( TINT a_iLOD )
{
	if ( a_iLOD & FLAG_MediumLOD )
	{
		if ( ( m_uiFlags & ( WFLAG_Released | WFLAG_SPAWNED ) ) == 0 && ( ( m_uiFlags & WFLAG_HasOwner ) == 0 || ( m_uiFlags & WFLAG_Activated ) != 0 ) )
		{
			TerrainTypes::CellInstance* pInstance          = TNULL;
			void*                       pCollisionInstance = TNULL;

			if ( m_iLinkedInstance != -1 )
			{
				pInstance = ATerrain2::GetInstance()->GetCellInstance( m_iLinkedInstance );

				if ( pInstance && pInstance->m_pModelInstance2 )
					pCollisionInstance = TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance();
			}

			if ( pInstance == TNULL || ( ( pInstance->m_uiFlags & INSTANCEFLAG_Unk1000000 ) && pCollisionInstance && TSTATICCAST( ACollisionInstance2, pCollisionInstance )->IsInSimulation( ACollisionInstance2::PhysicsObjectType_Standard | ACollisionInstance2::PhysicsObjectType_Custom ) ) )
			{
				SpawnEnemy( -1, 0, TFALSE );
			}
			else if ( m_iLinkedInstance != -1 )
			{
				m_uiFlags |= WFLAG_WaitForLinked;
				m_Flags &= ~FLAG_DontTick;
			}
		}
	}
	else
	{
		if ( !( a_iLOD & FLAG_LowLOD ) )
			return;

		if ( ( m_uiFlags & WFLAG_Released ) && ( m_uiFlags & WFLAG_Persistent ) )
			m_uiFlags &= ~WFLAG_Released;
	}

	if ( ( a_iLOD & FLAG_LowLOD ) && ( m_uiFlags & WFLAG_WaitForLinked ) )
	{
		m_Flags |= FLAG_DontTick;
		m_uiFlags &= ~WFLAG_WaitForLinked;
	}
}

// $deBlob: FUNCTION 00780700
void AEnemyWrapper::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );

	TBOOL bFound = a_rParams.GetValue( m_eEnemyType, "EnemyType" );
	TASSERT( bFound );
	TASSERT( TSTATICCAST( const AEntityLoader::EntityData, &a_rParams )->GetWaypointSetTargetCount() == 0 );

	TINT iCustomData;
	if ( a_rParams.GetValue( iCustomData, "CustomData" ) )
		m_uiCustomData = TUINT8( iCustomData );

	if ( a_rParams.GetValue( iCustomData, "CustomData2" ) )
		m_uiCustomData += TUINT8( iCustomData );

	TBOOL bPersistent = TFALSE;
	a_rParams.GetValue( bPersistent, "Persistent" );

	if ( bPersistent )
		m_uiFlags |= WFLAG_Persistent;

	TINT iLinkedInstance = -1;
	if ( a_rParams.GetValue( iLinkedInstance, "LinkedInstance" ) )
		m_iLinkedInstance = TINT16( iLinkedInstance );
}

// $deBlob: FUNCTION 00780910
void AEnemyWrapper::Activate( TINT a_iPickupType, TINT a_iNumPickups )
{
	TASSERT( ENEMYTYPE_INVALID < GetEnemyType() && GetEnemyType() < ENEMYTYPE_COUNT );

	if ( !( GetLOD() & FLAG_LowLOD ) && !( m_uiFlags & WFLAG_SPAWNED ) )
		SpawnEnemy( a_iPickupType, a_iNumPickups, TTRUE );

	m_uiFlags = ( m_uiFlags & ~WFLAG_Released ) | WFLAG_Activated;
}

// $deBlob: FUNCTION 007809e0
void AEnemyWrapper::Deactivate()
{
	m_uiFlags |= WFLAG_Released;

	if ( m_uiFlags & WFLAG_SPAWNED )
		ReleaseEnemy();
}

// $deBlob: FUNCTION 00780a00
void AEnemyWrapper::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_CHALLENGERESET:
			if ( m_uiFlags & WFLAG_SPAWNED )
			{
				// NOTE: retail passes the wrapper as the sender
				m_pSpawnedEnemy->HandleEntityIO( this, a_rPacket );
				return;
			}

			if ( m_eEnemyType != ENEMYTYPE_LeechBot2 || !ALeechBot2::IsFixedColourType( m_uiCustomData ) )
				return;

			// Falls through
		case AEntityIOPacket::MESSAGE_ENEMYRELEASED:
			OnEnemyGone();
			break;
		case AEntityIOPacket::MESSAGE_CULLED:
			OnEnemyRemoved();
			return;
		case AEntityIOPacket::MESSAGE_RECLAIM:
			OnEnemyKilled();
			return;
		case AEntityIOPacket::MESSAGE_DESPAWN:
			m_uiFlags |= WFLAG_Released;

			if ( m_uiFlags & WFLAG_SPAWNED )
				ReleaseEnemy();

			return;
	}
}

// $deBlob: FUNCTION 00780ad0
void AEnemyWrapper::SetOwnerTrigger( ABaseEntity* a_pTrigger )
{
	TASSERT( TNULL != a_pTrigger );
	TASSERT( -1 == m_iOwnerTrigger || a_pTrigger == GetOwnerTrigger() );
	TASSERT( a_pTrigger->GetID() > -1 );
	TASSERT( FindIDEntity( a_pTrigger->GetID() ) == a_pTrigger );

	m_uiFlags |= WFLAG_HasOwner;
	m_iOwnerTrigger = TINT16( a_pTrigger->GetID() );
}

// $deBlob: FUNCTION 00780d20
void AEnemyWrapper::OnEnemyKilled()
{
	if ( m_iOwnerTrigger >= 0 && FindIDEntity( m_iOwnerTrigger ) != TNULL )
	{
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_ENEMYKILLED;
		oPacket.m_iData    = 0;
		GetOwnerTrigger()->HandleEntityIO( this, oPacket );
	}

	m_uiFlags |= WFLAG_Released;

	if ( m_uiFlags & WFLAG_SPAWNED )
		ReleaseEnemy();
}

// $deBlob: FUNCTION 00780d90
void AEnemyWrapper::OnEnemyRemoved()
{
	TASSERT( m_uiFlags & WFLAG_SPAWNED );
	ReleaseEnemy();
}

// $deBlob: FUNCTION 00780e30
void AEnemyWrapper::OnEnemyGone()
{
	if ( m_iOwnerTrigger >= 0 && FindIDEntity( m_iOwnerTrigger ) != TNULL )
	{
		// APropTriggerEnemyGroup (GetOwnerTrigger()) is told the enemy is gone
		TSTATICCAST( APropTriggerEnemyGroup, GetOwnerTrigger() )->ReleaseEnemy( this );
	}

	m_uiFlags |= WFLAG_Released;

	if ( m_uiFlags & WFLAG_SPAWNED )
		ReleaseEnemy();
}

// $deBlob: FUNCTION 00780e90
void AEnemyWrapper::Reset( ABaseEntity* a_pEntity )
{
	m_uiFlags &= ~( WFLAG_Released | WFLAG_SPAWNED | WFLAG_Activated );
	m_pSpawnedEnemy = TNULL;
	m_Flags         = ( m_Flags & ~FLAG_LODMask ) | FLAG_LowLOD;
}

// $deBlob: FUNCTION 00780ec0
const TVector4& AEnemyWrapper::GetPosition() const
{
	if ( m_pSpawnedEnemy != TNULL )
		return m_pSpawnedEnemy->GetMatrix().GetTranslation();

	return m_oMatrix.GetTranslation();
}

// $deBlob: FUNCTION 00780ee0
void AEnemyWrapper::SpawnEnemy( TINT a_iPickupType, TINT a_iNumPickups, TBOOL a_bOffsetHeight )
{
	TASSERT( !( m_uiFlags & WFLAG_SPAWNED ) );

	TVector4     vPosition = m_oMatrix.GetTranslation();
	TBOOL        bLeechBot = TFALSE;
	ABaseEntity* pSpawner  = ( m_iSpawner < 0 ) ? TNULL : FindIDEntity( m_iSpawner );

	if ( a_bOffsetHeight )
	{
		if ( m_eEnemyType == ENEMYTYPE_LeechBot2 && ALeechBot2::IsFixedColourType( m_uiCustomData ) )
		{
			bLeechBot = TTRUE;
		}
		else
		{
			// The drop height of the owner trigger (plus its range: random, or the wrapper's place in the x range of its
			// squad) lifts the position
			TFLOAT fHeight = TFLOAT( TSTATICCAST( APropTriggerEnemyGroup, GetOwnerTrigger() )->GetDropHeight() );
			TFLOAT fRange  = TFLOAT( TSTATICCAST( APropTriggerEnemyGroup, GetOwnerTrigger() )->GetDropRange() );
			TFLOAT fOffset;

			if ( pSpawner == TNULL || TSTATICCAST( ASquad, pSpawner )->GetRangeX() <= 0.0f )
				fOffset = ARandom::GetSingleton()->GetRandom().GetFloat() * fRange + fHeight;
			else
				fOffset = ( ( m_oMatrix.m_f41 - TSTATICCAST( ASquad, pSpawner )->GetMinX() ) / TSTATICCAST( ASquad, pSpawner )->GetRangeX() ) * fRange + fHeight;

			vPosition.y -= fOffset;
		}
	}

	TVector4 vFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vFacing.w        = 1.0f;

	m_pSpawnedEnemy = AEnemyManager::GetSingleton()->CreateEnemy( AEnemyManager::EnemyType( m_eEnemyType ), vPosition, vFacing, this, TINT8( a_iPickupType ), TINT16( a_iNumPickups ), TFALSE, pSpawner, m_uiCustomData );
	TASSERT( m_pSpawnedEnemy );

	if ( m_pSpawnedEnemy )
	{
		m_uiFlags |= WFLAG_SPAWNED;

		if ( a_bOffsetHeight )
		{
			if ( bLeechBot )
			{
				// The LeechBot drops from the sky
				m_pSpawnedEnemy->CastTo<ALeechBot2>()->StartDrop();
				return;
			}

			m_pSpawnedEnemy->EndAtomState();
		}
	}
}

// $deBlob: FUNCTION 00781120
void AEnemyWrapper::ReleaseEnemy()
{
	TASSERT( m_uiFlags & WFLAG_SPAWNED );

	AEnemyManager::GetSingleton()->ReleaseEnemy( m_pSpawnedEnemy );

	m_uiFlags &= ~WFLAG_SPAWNED;
	m_pSpawnedEnemy = TNULL;
}
