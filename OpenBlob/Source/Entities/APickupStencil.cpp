#include "pch.h"
#include "APickupStencil.h"
#include "AEntityParams.h"
#include "GameInterface/APlayerManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APickupStencil ) == 0x160 );

// $deBlob: FUNCTION 007b0520
// $deBlob: FUNCTION 007b0530
// $deBlob: FUNCTION 007b05d0
// $deBlob: FUNCTION 007b0660
// $deBlob: FUNCTION 007b0670
// $deBlob: FUNCTION 00541580
// $deBlob: FUNCTION 00859fe0
TDEFINE_CLASS( APickupStencil );

// $deBlob: FUNCTION 007b0680
APickupStencil::APickupStencil()
    : APickup( 2, 0, 0.0f, TTRUE )
{
	m_iStencilType = -1;
}

// $deBlob: FUNCTION 007b06c0
// $deBlob: FUNCTION 007b07b4
APickupStencil::~APickupStencil()
{
}

// $deBlob: FUNCTION 007b06e0
// $deBlob: FUNCTION 007b0846
void APickupStencil::ProcessParams( const AEntityParams& a_rParams )
{
	APickup::ProcessParams( a_rParams );

	a_rParams.GetValue( m_iStencilType, "PickupType" );
	TASSERT( m_iStencilType >= 0 && m_iStencilType < PT_STENCIL_COUNT );
}

// $deBlob: FUNCTION 007b0740
void APickupStencil::OnCollected( ABaseEntity* a_pCollector )
{
	APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetBlobDataOf( a_pCollector );
	pBlobData->SetStencilCollected( GetStencilType(), TTRUE );

	APickup::OnCollected( a_pCollector );
}
