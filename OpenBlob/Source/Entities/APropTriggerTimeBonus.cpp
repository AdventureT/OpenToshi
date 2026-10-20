#include "pch.h"
#include "APropTriggerTimeBonus.h"
#include "APickupManager.h"
#include "AEntityParams.h"
#include "ABlobEntity/ABlobEntity.h"
#include "GameController/AGameStatus.h"
#include "Render/AInstanceRenderer.h"
#include "Challenges/AChallengeManager.h"
#include "Sound/ASoundInterface.h"
#include "APropTriggerHelp.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropTriggerTimeBonus ) == 0x264 );

// $deBlob: FUNCTION 0079c5e0
// $deBlob: FUNCTION 0079c5f0
// $deBlob: FUNCTION 0079c690
// $deBlob: FUNCTION 0079c730
// $deBlob: FUNCTION 0079c740
// $deBlob: FUNCTION 00540e20
// $deBlob: FUNCTION 00859e10
TDEFINE_CLASS( APropTriggerTimeBonus );

// $deBlob: FUNCTION 0079c750
APropTriggerTimeBonus::APropTriggerTimeBonus()
    : APickup( 6, 0, 0.0f, TTRUE )
{
	m_iType    = 0;
	m_iSeconds = 0;
}

// $deBlob: FUNCTION 0079ca15
// $deBlob: FUNCTION 0079ca20
// $deBlob: FUNCTION 0079c790
APropTriggerTimeBonus::~APropTriggerTimeBonus()
{
}

// $deBlob: FUNCTION 0079c7b0
// $deBlob: FUNCTION 0079ca81
void APropTriggerTimeBonus::ProcessParams( const AEntityParams& a_rParams )
{
	a_rParams.GetValue( m_iType, "TimeBonusType" );
	TASSERT( m_iType != -1 );

	const TCHAR* szModel;

	if ( m_iType == TYPE_30Seconds )
	{
		m_iSeconds = 30;
		szModel    = "model\\pickups\\PICKUP_Time_3.trb";
	}
	else if ( m_iType == TYPE_60Seconds )
	{
		m_iSeconds = 60;
		szModel    = "model\\pickups\\PICKUP_Time_2.trb";
	}
	else if ( m_iType == TYPE_180Seconds )
	{
		m_iSeconds = 180;
		szModel    = "model\\pickups\\PICKUP_Time_1.trb";
	}
	else
	{
		APickup::ProcessParams( a_rParams );
		return;
	}

	m_iMesh = AInstanceRenderer::GetInstance()->RegisterModel( szModel );
	APickup::ProcessParams( a_rParams );
}

// $deBlob: FUNCTION 0079c860
void APropTriggerTimeBonus::InitPooled( TINT a_eType )
{
	const TCHAR* sModelName = TNULL;

	switch ( a_eType )
	{
		case 1:
		{
			m_iType    = TYPE_30Seconds;
			sModelName = "model\\pickups\\PICKUP_Time_3.trb";
			m_iSeconds = 30;
		}
		break;
		case 2:
		{
			m_iType    = TYPE_60Seconds;
			sModelName = "model\\pickups\\PICKUP_Time_2.trb";
			m_iSeconds = 60;
		}
		break;
		case 3:
		{
			m_iType    = TYPE_180Seconds;
			sModelName = "model\\pickups\\PICKUP_Time_1.trb";
			m_iSeconds = 180;
		}
		break;
	}

	TASSERT( sModelName && *sModelName );
	m_iMesh = AInstanceRenderer::GetInstance()->RegisterModel( sModelName );
	APickup::InitPooled( a_eType );
}

// $deBlob: FUNCTION 0079c920
void APropTriggerTimeBonus::OnCollected( ABaseEntity* a_pCollector )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	APickupManager::StartEffect( APickupManager::PICKUPEFFECT_Time, TSTATICCAST( ABlobEntity, a_pCollector )->GetBodyPosition() );

	TFLOAT fTimeLeft = pGameStatus->GetTimeLeft();

	if ( TMath::MINFLOAT < fTimeLeft )
		pGameStatus->SetTimeLeft( TFLOAT( m_iSeconds ) + fTimeLeft );

	// The challenge being played gets the time too
	AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

	if ( pChallenge != TNULL && TMath::MINFLOAT < pChallenge->GetTimeLeft() )
		pChallenge->AddTime( TFLOAT( m_iSeconds ) );

	TStringManager::String16Format( m_wszMessage, 0x80, T2Locale::GetSingleton()->GetString( 27 ), m_iSeconds );
	pGameStatus->GetLevelData().ShowMessage( m_wszMessage, 3, &GetTranslation(), 0 );
	pGameStatus->GetLevelData().ShowMessage( m_wszMessage, 4, TNULL, 0 );

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCollector );
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 44 ), pBlob->GetBodyPosition(), FMOD_EVENT_MODE( 0 ) );
	APropTriggerHelp::ShowContextHelp( 0x100 );

	APickup::OnCollected( a_pCollector );
}
