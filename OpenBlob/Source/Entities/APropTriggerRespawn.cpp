#include "pch.h"
#include "APropTriggerRespawn.h"
#include "AEntityParams.h"
#include "ABlobEntity/ABlobEntity.h"
#include "Terrain2/TerrainTypes.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerRespawn ) == 0xEC );

// $deBlob: FUNCTION 007a18c0
// $deBlob: FUNCTION 007a18d0
// $deBlob: FUNCTION 007a1940
// $deBlob: FUNCTION 007a19a0
// $deBlob: FUNCTION 007a19b0
// $deBlob: FUNCTION 00540f60
// $deBlob: FUNCTION 00859e60
TDEFINE_CLASS( APropTriggerRespawn );

// $deBlob: FUNCTION 007a19c0
APropTriggerRespawn::APropTriggerRespawn()
{
	m_vRespawnPosition = TVector4::VEC_ZERO;
	m_vRespawnRotation = TVector4::VEC_ZERO;
	m_fSeqStartRatio   = 0.75f;
	m_fSeqEndRatio     = 1.0f;
	m_uiRespawnFlags   = RESPAWNFLAG_TransSeq;
}

// $deBlob: FUNCTION 007a1cf0
// $deBlob: FUNCTION 007a1a20
// $deBlob: FUNCTION 007a1cd9
APropTriggerRespawn::~APropTriggerRespawn()
{
}

// $deBlob: FUNCTION 007a1a40
// $deBlob: FUNCTION 007a1d35
void APropTriggerRespawn::ProcessParams( const AEntityParams& a_rParams )
{
	void* pPoint = TNULL;

	if ( a_rParams.GetRespawnPoint( pPoint ) )
	{
		m_vRespawnPosition = TSTATICCAST( TVector4, pPoint )[ 0 ];
		m_uiRespawnFlags |= RESPAWNFLAG_HasPoint;
		m_vRespawnRotation = TSTATICCAST( TVector4, pPoint )[ 1 ];
	}

	TINT iValue;

	if ( a_rParams.GetValue( iValue, "PlaySeq" ) && iValue != 0 )
		m_uiRespawnFlags |= RESPAWNFLAG_PlaySeq;

	if ( a_rParams.GetValue( iValue, "TransSeq" ) && iValue != 0 )
		m_uiRespawnFlags |= RESPAWNFLAG_TransSeq;

	TFLOAT fValue;

	if ( a_rParams.GetValue( fValue, "StartRatio" ) )
	{
		m_fSeqStartRatio = fValue;
		TMath::Clip( m_fSeqStartRatio, 0.0f, 1.0f );
	}

	if ( a_rParams.GetValue( fValue, "EndRatio" ) )
	{
		m_fSeqEndRatio = fValue;
		TMath::Clip( m_fSeqEndRatio, 0.0f, 1.0f );
	}

	TASSERT( m_fSeqStartRatio < m_fSeqEndRatio );
	APropTrigger::ProcessParams( a_rParams );
}

// $deBlob: FUNCTION 007a1b90
void APropTriggerRespawn::Trigger()
{
	ABaseEntity* pEntity = TREINTERPRETCAST( ABaseEntity*, m_uiTriggeredBy );

	if ( pEntity && pEntity->IsA( &TGetClass( ABlobEntity ) ) )
	{
		ABlobEntity* pBlob = pEntity->CastTo<ABlobEntity>();

		if ( GetNumInstances() != 0 )
		{
			// The respawn point is at the bottom of the first target instance (half a unit below)
			TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, GetInstanceTarget( 0 ) );

			TVector4 vPosition = *pInstance->m_ppCentre[ 0 ];
			vPosition.y        = ( vPosition.y - pInstance->m_ppCentre[ 1 ]->y ) - 0.5f;

			pBlob->SetRespawnPoint( vPosition, m_vRespawnRotation, m_uiRespawnFlags & RESPAWNFLAG_HasPoint, ( m_uiRespawnFlags >> 1 ) & 1, ( m_uiRespawnFlags >> 2 ) & 1, m_fSeqStartRatio, m_fSeqEndRatio );
			return;
		}

		pBlob->SetRespawnPoint( m_vRespawnPosition, m_vRespawnRotation, m_uiRespawnFlags & RESPAWNFLAG_HasPoint, ( m_uiRespawnFlags >> 1 ) & 1, ( m_uiRespawnFlags >> 2 ) & 1, m_fSeqStartRatio, m_fSeqEndRatio );
	}
}
