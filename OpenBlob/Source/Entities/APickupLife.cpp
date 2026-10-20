#include "pch.h"
#include "APickupLife.h"
#include "APickupManager.h"
#include "ABlobEntity/ABlobEntity.h"
#include "ABlobEntity/ABlobSoundComponent.h"
#include "Render/AInstanceRenderer.h"
#include "APropTriggerHelp.h"
#include "GameInterface/APlayerManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APickupLife ) == 0x168 );

// $deBlob: FUNCTION 007b0890
// $deBlob: FUNCTION 007b08a0
// $deBlob: FUNCTION 007b0940
// $deBlob: FUNCTION 007b09d0
// $deBlob: FUNCTION 007b09e0
// $deBlob: FUNCTION 005415c0
// $deBlob: FUNCTION 00859ff0
TDEFINE_CLASS( APickupLife );

// $deBlob: FUNCTION 007b09f0
APickupLife::APickupLife()
    : APickup( 4, 0, 0.0f, TTRUE )
{
	m_oLifeNode.m_pOwner = this;
}

// $deBlob: FUNCTION 007b0a30
// $deBlob: FUNCTION 007b0c70
// $deBlob: FUNCTION 007b0c56
APickupLife::~APickupLife()
{
	if ( m_oLifeNode.IsLinked() )
		m_oLifeNode.Remove();
}

// $deBlob: FUNCTION 007b0ae0
// $deBlob: FUNCTION 007b0c91
void APickupLife::ProcessParams( const AEntityParams& a_rParams )
{
	APickup::ProcessParams( a_rParams );
}

// $deBlob: FUNCTION 007b0af0
void APickupLife::InitPooled( TINT a_eType )
{
	m_iMesh = AInstanceRenderer::GetInstance()->RegisterModel( "model\\pickups\\PICKUP_Blob.trb" );
	APickup::InitPooled( a_eType );
}

// $deBlob: FUNCTION 007b0b90
void APickupLife::OnPoolRelease()
{
	APickup::OnPoolRelease();

	if ( m_oLifeNode.IsLinked() )
		m_oLifeNode.Remove();
}

// $deBlob: FUNCTION 007b0b20
void APickupLife::OnCollected( ABaseEntity* a_pCollector )
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCollector );

	// The life pickup's sound (sound component 0x3D4 is null-checked)
	if ( pBlob->GetSoundComponent() != TNULL )
		pBlob->GetSoundComponent()->PlayLifePickup();

	if ( m_oLifeNode.IsLinked() )
		m_oLifeNode.Remove();

	pBlob->EndElectrocution();
	APlayerManager::GetSingleton()->GetBlobDataOf( pBlob )->AddLife();

	APickupManager::StartEffect( APickupManager::PICKUPEFFECT_Life, pBlob->GetBodyPosition() );
	APropTriggerHelp::ShowContextHelp( 8 );

	APickup::OnCollected( a_pCollector );
}
