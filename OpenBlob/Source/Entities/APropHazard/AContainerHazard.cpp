#include "pch.h"
#include "AContainerHazard.h"
#include "APropHazard.h"
#include "Entities/Helpers/ASpawnPointPool.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityFactory.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AContainerHazard ) == 0x94 );

// $deBlob: FUNCTION 007b5bc0
// $deBlob: FUNCTION 007b5bd0
// $deBlob: FUNCTION 007b5c80
// $deBlob: FUNCTION 007b5d30
// $deBlob: FUNCTION 007b5d40
// $deBlob: FUNCTION 00541680
// $deBlob: FUNCTION 0085a020
TDEFINE_CLASS( AContainerHazard );

// $deBlob: FUNCTION 007b5d50
AContainerHazard::AContainerHazard()
{
	m_pSpawnPoints = TNULL;
	SetHash( TFALSE );
	m_Flags |= FLAG_DontTick;
}

// $deBlob: FUNCTION 007b6070
// $deBlob: FUNCTION 007b5de0
// $deBlob: FUNCTION 007b6063
AContainerHazard::~AContainerHazard()
{
	if ( m_pSpawnPoints != TNULL )
	{
		delete m_pSpawnPoints;
		m_pSpawnPoints = TNULL;
	}
}

// $deBlob: FUNCTION 007b5e80
void AContainerHazard::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_OPEN:
		case AEntityIOPacket::MESSAGE_CLOSE:
		case AEntityIOPacket::MESSAGE_DESPAWN:
			// Forwarded to the hazards (exactly APropHazard) that aren't disabled
			for ( TINT i = 0; i < GetNumEntityTargets(); i++ )
			{
				ABaseEntity* pTarget = GetEntityTarget( i );

				if ( pTarget != TNULL && pTarget->GetTypeData()->m_pClass == &TGetClass( APropHazard ) && !( pTarget->GetFlags() & FLAG_Despawned ) )
					pTarget->HandleEntityIO( a_pCaller, a_rPacket );
			}

			break;
		default:
			break;
	}
}

// $deBlob: FUNCTION 007b5f30
void AContainerHazard::Reset( ABaseEntity* a_pEntity )
{
	if ( m_pSpawnPoints != TNULL )
		m_pSpawnPoints->ReleaseAll();
}

// $deBlob: FUNCTION 007b5f50
// $deBlob: FUNCTION 007b609c
void AContainerHazard::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );
	ATriggerUtil::ProcessParams( a_rParams );

	void* pPoints    = TNULL;
	TINT  iNumPoints = a_rParams.GetArray( pPoints, "HazardPointList" );

	if ( iNumPoints <= 0 )
		m_pSpawnPoints = TNULL;
	else
		m_pSpawnPoints = new ASpawnPointPool( TREINTERPRETCAST( TVector4* const&, pPoints ), TUINT8( iNumPoints ) );
}

// $deBlob: FUNCTION 007b6020
// $deBlob: FUNCTION 007b6091
void AContainerHazard::PostCreate()
{
	for ( TINT i = 0; i < GetNumEntityTargets(); i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget != TNULL && pTarget->GetTypeData()->m_pClass == &TGetClass( APropHazard ) )
			TSTATICCAST( APropHazard, pTarget )->SetContainer( this );
	}
}
