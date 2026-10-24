#include "pch.h"
#include "APickupBonus.h"
#include "APickupManager.h"
#include "APropTriggerHelp.h"
#include "AEntityParams.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobSoundComponent.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"

#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APickupBonus ) == 0x160 );

// $deBlob: FUNCTION 007b0040
// $deBlob: FUNCTION 007b0050
// $deBlob: FUNCTION 007b00f0
// $deBlob: FUNCTION 007b0180
// $deBlob: FUNCTION 007b0190
// $deBlob: FUNCTION 00541540
// $deBlob: FUNCTION 00859fd0
TDEFINE_CLASS( APickupBonus );

// $deBlob: FUNCTION 007b01a0
APickupBonus::APickupBonus()
    : APickup( 1, 0, 0.0f, TTRUE )
{
	m_iBonusType = -1;
	m_iBonusSet  = -1;
}

// $deBlob: FUNCTION 007b01e0
// $deBlob: FUNCTION 007b0464
APickupBonus::~APickupBonus()
{
}

// $deBlob: FUNCTION 007b0200
// $deBlob: FUNCTION 007b04d8
void APickupBonus::ProcessParams( const AEntityParams& a_rParams )
{
	APickup::ProcessParams( a_rParams );

	a_rParams.GetValue( m_iBonusType, "PickupType" );
	TASSERT( m_iBonusType >= 0 && m_iBonusType < PT_BONUS_COUNT );

	a_rParams.GetValue( m_iBonusSet, "BonusSet" );
	TASSERT( m_iBonusSet >= 0 && m_iBonusSet < PT_TOTAL_BONUS_SETS );

	AGameStatus::GetSingleton()->GetLevelData().AddBonusPickup();
}

// $deBlob: FUNCTION 007b02c0
void APickupBonus::OnCollected( ABaseEntity* a_pCollector )
{
	TSTATICCAST( ABlobEntity, a_pCollector )->GetSoundComponent()->PlayBonusPickup();

	APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( a_pCollector );

	TASSERT( GetBonusSet() >= 0 && GetBonusSet() < PT_TOTAL_BONUS_SETS );
	pBlobData->SetBonusCollected( m_iBonusSet, m_iBonusType, TTRUE );

	TINT iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pCollector );

	if ( pBlobData->IsBonusSetComplete( m_iBonusSet ) )
	{
		TASSERT( iPlayer >= 0 && iPlayer < APlayerManager::MAX_PLAYERS );

		AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
		rLevelData.AddScore( iPlayer, 2000, -1 );

		TWCHAR* wszText = TStringManager::GetTempString16();
		TStringManager::String16Format( wszText, T2Locale::GetSingleton()->GetString( 56 ), 2000 );

		TVector4 vPosition = m_oMatrix.GetTranslation();
		vPosition.y        = m_oMatrix.GetTranslation().y - m_vBBox.y * 2.0f;
		rLevelData.ShowMessage( wszText, 2, &vPosition, 0 );
	}

	AGameStatus::GetSingleton()->GetLevelData().AddBonusCollected( iPlayer );

	APickupManager::StartEffect( APickupManager::PICKUPEFFECT( m_iBonusSet + APickupManager::PICKUPEFFECT_TagA ), TSTATICCAST( ABlobEntity, a_pCollector )->GetBodyPosition() );
	APropTriggerHelp::ShowContextHelp( 4 );

	APickup::OnCollected( a_pCollector );
}
