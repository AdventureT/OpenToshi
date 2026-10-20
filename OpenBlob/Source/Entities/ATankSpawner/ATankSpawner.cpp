#include "pch.h"
#include "ATankSpawner.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/ACar/ACar.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AEntityParams.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ATankSpawner ) == 0xA4 );

// $deBlob: FUNCTION 0077ecc0
// $deBlob: FUNCTION 0077ecd0
// $deBlob: FUNCTION 0077ed60
// $deBlob: FUNCTION 0077ede0
// $deBlob: FUNCTION 0077edf0
// $deBlob: FUNCTION 00540aa0
// $deBlob: FUNCTION 00859d20
TDEFINE_CLASS( ATankSpawner );

// $deBlob: FUNCTION 0077ee00
ATankSpawner::ATankSpawner()
{
	m_pTank            = TNULL;
	m_iTanksAvailable  = -1;
	m_iTanksDestroyed  = 0;
	m_bDisabled        = TFALSE;
	m_bChallengeLinked = TFALSE;
}

// $deBlob: FUNCTION 0077f470
// $deBlob: FUNCTION 0077f460
ATankSpawner::~ATankSpawner()
{
}

// $deBlob: FUNCTION 0077ee30
void ATankSpawner::Create()
{
	AVisibleEntity::Create();
	SetState( 0 );
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 0077f010
void ATankSpawner::ProcessParams( const AEntityParams& a_rParams )
{
	AVisibleEntity::ProcessParams( a_rParams );

	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );

	if ( rEntityData.GetTargetCount() > 0 )
	{
		// The tank is the first target
		TINT iID = rEntityData.GetTarget( 0 );
		TASSERT( ABaseEntity::FindIDEntity( iID )->IsA( &TGetClass( ACar ) ) );

		m_pTank = TSTATICCAST( ACar, ABaseEntity::FindIDEntity( iID ) );
		m_pTank->SetTankSpawner( this );
		m_pTank->m_iLife = 0;
		m_pTank->m_Flags |= FLAG_Dead;
	}

	a_rParams.GetValue( m_iTanksAvailable, "NumTanksAvailable" );
}

// $deBlob: FUNCTION 0077ee50
void ATankSpawner::Tick( TFLOAT a_fDeltaTime )
{
	AVisibleEntity::Tick( a_fDeltaTime );

	switch ( GetState() )
	{
		case 0:
			// The tank was destroyed and waits
			if ( !m_bDisabled && m_pTank != TNULL && ( m_pTank->m_uiCarFlags & ACar::CARFLAG_Waiting ) != 0 )
			{
				TASSERT( ( m_iTanksAvailable < 0 ) || ( m_iTanksDestroyed < m_iTanksAvailable ) );
				TASSERT( m_pTank->IsStopped() );
				SetState( 1 );
			}
			break;
		case 1:
			if ( m_fStateTime > 5.0f )
				SetState( 2 );
			break;
		case 2:
			if ( HasStateFinished() )
			{
				// The tank comes back
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
				oPacket.m_iData    = 0;
				m_pTank->HandleEntityIO( this, oPacket );

				m_pTank->m_iLife = TINT16( m_pTank->GetMaxLife() );
				m_pTank->m_Flags &= ~FLAG_Dead;
				SetState( 3 );
			}
			break;
		case 3:
			if ( m_fStateTime > 3.0f )
				SetState( 4 );
			break;
		case 4:
			if ( HasStateFinished() )
				SetState( 0 );
			break;
	}
}

// $deBlob: FUNCTION 0077f130
void ATankSpawner::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			m_bDisabled = TTRUE;
			SetTicking( TFALSE );
			m_Flags |= FLAG_DontTick;
			break;
		case AEntityIOPacket::MESSAGE_CHALLENGEINIT:
			// A combat challenge counts the tanks
			m_bDisabled        = TTRUE;
			m_bChallengeLinked = TTRUE;
			m_iTanksDestroyed  = 0;
			SetTicking( TFALSE );
			m_Flags |= FLAG_DontTick;
			break;
		case AEntityIOPacket::MESSAGE_CHALLENGERESET:
			m_pTank->Explode( TFALSE );
			m_iTanksDestroyed = 0;
			m_bDisabled       = TTRUE;
			SetTicking( TFALSE );
			m_Flags |= FLAG_DontTick;
			break;
		case AEntityIOPacket::MESSAGE_CHALLENGESTART:
			m_Flags &= ~FLAG_DontTick;
			m_bDisabled = TFALSE;
			SetTicking( TTRUE );
			break;
	}

	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 0077f250
void ATankSpawner::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );
	SetState( 0 );

	if ( m_bChallengeLinked )
	{
		m_bDisabled = TTRUE;
		SetTicking( TFALSE );
		m_Flags |= FLAG_DontTick;
	}
	else
	{
		m_Flags &= ~FLAG_DontTick;
		m_bDisabled = TFALSE;
		SetTicking( TTRUE );
	}

	m_iTanksDestroyed = 0;
}

// $deBlob: FUNCTION 0077f2c0
void ATankSpawner::OnTankDestroyed()
{
	if ( m_iTanksAvailable > 0 )
	{
		TASSERT( m_iTanksDestroyed < m_iTanksAvailable );
		m_iTanksDestroyed++;

		if ( m_iTanksDestroyed == m_iTanksAvailable )
		{
			// The last tank: the spawner stops and the challenge is told
			m_bDisabled = TTRUE;
			SetTicking( TFALSE );
			m_Flags |= FLAG_DontTick;

			TASSERT( !IsPooled() );
			AChallengeManager::GetSingleton()->OnCombatEvent( TREINTERPRETCAST( void*, TUINTPTR( m_iID ) ) );
		}
	}
}

// $deBlob: FUNCTION 0077f380
TBOOL ATankSpawner::IsComplete() const
{
	TASSERT( m_bChallengeLinked );
	TASSERT( m_iTanksAvailable > 0 );
	TASSERT( m_iTanksDestroyed <= m_iTanksAvailable );

	return m_iTanksDestroyed == m_iTanksAvailable;
}

// $deBlob: FUNCTION 0077f450
const TCHAR* ATankSpawner::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}
