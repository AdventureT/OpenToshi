#include "pch.h"
#include "APickupEnergy.h"
#include "APickupManager.h"
#include "AEntityParams.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobSoundComponent.h"
#include "APropTriggerHelp.h"
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

TSTATICASSERT( sizeof( APickupEnergy ) == 0x160 );

// $deBlob: FUNCTION 007b0cb0
// $deBlob: FUNCTION 007b0cc0
// $deBlob: FUNCTION 007b0d60
// $deBlob: FUNCTION 007b0df0
// $deBlob: FUNCTION 007b0e00
// $deBlob: FUNCTION 00541600
// $deBlob: FUNCTION 0085a000
TDEFINE_CLASS( APickupEnergy );

// $deBlob: FUNCTION 007b0e10
APickupEnergy::APickupEnergy()
    : APickup( 5, 0, 0.0f, TTRUE )
{
	m_iEnergy = 0;
}

// $deBlob: FUNCTION 007b0f60
// $deBlob: FUNCTION 007b0f70
// $deBlob: FUNCTION 007b0f65
APickupEnergy::~APickupEnergy()
{
}

// $deBlob: FUNCTION 007b0e50
// $deBlob: FUNCTION 007b0fa8
void APickupEnergy::ProcessParams( const AEntityParams& a_rParams )
{
	APickup::ProcessParams( a_rParams );
	a_rParams.GetValue( m_iEnergy, "Energy" );
}

// $deBlob: FUNCTION 007b0e80
void APickupEnergy::OnCollected( ABaseEntity* a_pCollector )
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCollector );

	// The score and its message ("+%d") above the pickup
	TINT8 iPlayer = APlayerManager::GetSingleton()->GetPlayerIndex( a_pCollector );
	AGameStatus::GetSingleton()->GetLevelData().AddScore( iPlayer, m_iEnergy, -1 );

	TWCHAR* wszText = TStringManager::GetTempString16();
	TStringManager::String16Format( wszText, T2Locale::GetSingleton()->GetString( 0xBE ), TINT( m_iEnergy ) );

	APickupManager::StartEffect( APickupManager::PICKUPEFFECT_Energy, pBlob->GetBodyPosition() );

	TVector4 vPosition = GetTranslation();
	vPosition.y        = vPosition.y - GetBBox().y * 2.0f;
	AGameStatus::GetSingleton()->GetLevelData().ShowMessage( wszText, 2, &vPosition, 0 );

	// The energy pickup's sound (sound component 0x3D4 is null-checked)
	if ( pBlob->GetSoundComponent() != TNULL )
		pBlob->GetSoundComponent()->PlayEnergyPickup();

	APropTriggerHelp::ShowContextHelp( 0x4000 );

	APickup::OnCollected( a_pCollector );
}
