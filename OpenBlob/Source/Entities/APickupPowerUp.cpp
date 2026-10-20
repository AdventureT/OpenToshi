#include "pch.h"
#include "APickupPowerUp.h"
#include "AEntityParams.h"
#include "GameInterface/APlayerManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APickupPowerUp ) == 0x164 );

// $deBlob: FUNCTION 007af830
// $deBlob: FUNCTION 007af840
// $deBlob: FUNCTION 007af8e0
// $deBlob: FUNCTION 007af980
// $deBlob: FUNCTION 007af990
// $deBlob: FUNCTION 005414c0
// $deBlob: FUNCTION 00859fb0
TDEFINE_CLASS( APickupPowerUp );

// $deBlob: FUNCTION 007af9a0
APickupPowerUp::APickupPowerUp()
    : APickup( 0, 0, 0.0f, TTRUE )
{
	m_iPowerUpType = -1;
	m_fEffectTime  = -1.0f;
}

// $deBlob: FUNCTION 007af9e0
// $deBlob: FUNCTION 007afb09
APickupPowerUp::~APickupPowerUp()
{
}

// $deBlob: FUNCTION 007afa00
// $deBlob: FUNCTION 007afbe6
void APickupPowerUp::ProcessParams( const AEntityParams& a_rParams )
{
	APickup::ProcessParams( a_rParams );

	a_rParams.GetValue( m_iPowerUpType, "PickupType" );
	a_rParams.GetValue( m_fEffectTime, "EffectTime" );
	TASSERT( m_iPowerUpType >= 0 && m_iPowerUpType < PT_POWERUPS_COUNT );
}

// $deBlob: FUNCTION 007afa80
void APickupPowerUp::OnCollected( ABaseEntity* a_pCollector )
{
	APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( a_pCollector );

	TFLOAT fEffectTime = GetEffectTime();
	pBlobData->SetPowerUp( TINT8( GetPowerUpType() ), fEffectTime, fEffectTime );

	APickup::OnCollected( a_pCollector );
}
