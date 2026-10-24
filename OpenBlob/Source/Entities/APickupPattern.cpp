#include "pch.h"
#include "Entities/APropTriggerHelp.h"
#include "APickupPattern.h"
#include "AEntityParams.h"
#include "APickupManager.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobSoundComponent.h"
#include "GameInterface/APlayerManager.h"
#include "GameController/AGameStatus.h"
#include <T2Locale/T2Locale.h>
#include "APropTriggerHelp.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APickupPattern ) == 0x160 );

// $deBlob: FUNCTION 007afc00
// $deBlob: FUNCTION 007afc10
// $deBlob: FUNCTION 007afcb0
// $deBlob: FUNCTION 007afd40
// $deBlob: FUNCTION 007afd50
// $deBlob: FUNCTION 00541500
// $deBlob: FUNCTION 00859fc0
TDEFINE_CLASS( APickupPattern );

// $deBlob: FUNCTION 007afd60
APickupPattern::APickupPattern()
    : APickup( 3, 0, 0.0f, TTRUE )
{
	m_iPatternType = -1;
}

// $deBlob: FUNCTION 007aff60
// $deBlob: FUNCTION 007afda0
// $deBlob: FUNCTION 007aff4e
APickupPattern::~APickupPattern()
{
}

// $deBlob: FUNCTION 007afdc0
// $deBlob: FUNCTION 007b0006
void APickupPattern::ProcessParams( const AEntityParams& a_rParams )
{
	APickup::ProcessParams( a_rParams );
	a_rParams.GetValue( m_iPatternType, "PickupType" );
	TASSERT( m_iPatternType >= 0 && m_iPatternType < PT_PATTERN_COUNT_NORMAL );

	// The level's number of pattern pickups (shown as "%d/%d" by ShowMessage's type 7)
	AGameStatus::GetSingleton()->GetLevelData().m_auiUnk160[ 0 ]++;
}

// $deBlob: FUNCTION 007afe30
void APickupPattern::OnCollected( ABaseEntity* a_pCollector )
{
	TSTATICCAST( ABlobEntity, a_pCollector )->GetSoundComponent()->PlayPatternPickup(); // no null check

	// The pattern is unlocked for the collector's player: BlobData m_uiPatternMask, m_uiUnk58 (count), m_iPattern
	APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( a_pCollector );
	pBlobData->m_uiPatternMask |= 1 << ( GetPatternType() & 0x1F );
	TINT iPatternType = GetPatternType();
	pBlobData->m_uiUnk58++;
	pBlobData->m_iPattern = iPatternType;

	// The pattern message
	AGameStatus::LevelData& rLevelData = AGameStatus::GetSingleton()->GetLevelData();
	rLevelData.IncrementUnk44( APlayerManager::GetSingleton()->GetPlayerIndex( a_pCollector ) );
	rLevelData.ShowMessage( T2Locale::GetSingleton()->GetString( 0xD1 ), 7, TNULL, 0 );

	APickupManager::StartEffect( APickupManager::PICKUPEFFECT_Pattern, TSTATICCAST( ABlobEntity, a_pCollector )->GetBodyPosition() );

	APropTriggerHelp::ShowContextHelp( 0x10 );

	APickup::OnCollected( a_pCollector );
}
