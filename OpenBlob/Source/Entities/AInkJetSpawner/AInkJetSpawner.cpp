#include "pch.h"
#include "AInkJetSpawner.h"
#include "Entities/AEntityLoader.h"
#include "Entities/AInkJet/AInkJet.h"
#include "Entities/AEntityParams.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AInkJetSpawner ) == 0xA4 );

// $deBlob: FUNCTION 007ae070
// $deBlob: FUNCTION 007ae080
// $deBlob: FUNCTION 007ae110
// $deBlob: FUNCTION 007ae190
// $deBlob: FUNCTION 007ae1a0
// $deBlob: FUNCTION 00541360
// $deBlob: FUNCTION 00859f60
TDEFINE_CLASS( AInkJetSpawner );

// $deBlob: FUNCTION 007ae1b0
AInkJetSpawner::AInkJetSpawner()
{
	m_pInkJet    = TNULL;
	m_bTriggered = TFALSE;
}

// $deBlob: FUNCTION 007ae4f0
// $deBlob: FUNCTION 007ae1e0
AInkJetSpawner::~AInkJetSpawner()
{
}

// $deBlob: FUNCTION 007ae1f0
void AInkJetSpawner::Create()
{
	AVisibleEntity::Create();
	SetState( 0 );
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007ae370
void AInkJetSpawner::ProcessParams( const AEntityParams& a_rParams )
{
	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );

	if ( rEntityData.GetTargetCount() > 0 )
	{
		// The jet is the first target
		TINT iID = rEntityData.GetTarget( 0 );
		TASSERT( ABaseEntity::FindIDEntity( iID )->IsA( &TGetClass( AInkJet ) ) );

		m_pInkJet = TSTATICCAST( AInkJet, ABaseEntity::FindIDEntity( iID ) );
		m_pInkJet->SetSpawned();
	}
}

// $deBlob: FUNCTION 007ae460
void AInkJetSpawner::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			m_bTriggered = TTRUE;
			SetTicking( TFALSE );
			m_Flags |= 0x4000;
			break;
	}

	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007ae4a0
void AInkJetSpawner::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );
	SetState( 0 );
	m_Flags &= TUINT32( ~0x4000 );
	m_bTriggered = TFALSE;
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007ae4e0
const TCHAR* AInkJetSpawner::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}

// $deBlob: FUNCTION 007ae220
void AInkJetSpawner::Tick( TFLOAT a_fDeltaTime )
{
	AVisibleEntity::Tick( a_fDeltaTime );

	switch ( GetState() )
	{
		case 0:
			if ( !m_bTriggered && m_pInkJet != TNULL && m_pInkJet->IsWaiting() )
			{
				TASSERT( m_pInkJet->IsWaitingForTrigger() );
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
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
				m_pInkJet->HandleEntityIO( this, oPacket );
				SetState( 3 );
			}
			break;
		case 3:
			if ( m_fStateTime > 0.8f )
				SetState( 4 );
			break;
		case 4:
			if ( HasStateFinished() )
				SetState( 0 );
			break;
	}
}
