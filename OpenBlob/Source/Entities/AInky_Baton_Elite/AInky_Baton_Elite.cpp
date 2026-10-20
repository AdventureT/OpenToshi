#include "pch.h"
#include "AInky_Baton_Elite.h"
#include "Entities/AEntityModelComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Animation/AModelInstance2.h"
#include "Sound/ASoundInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AInky_Baton_Elite ) == 0xF0 );

// $deBlob: FUNCTION 007ab410
// $deBlob: FUNCTION 007ab420
// $deBlob: FUNCTION 007ab4a0
// $deBlob: FUNCTION 007ab510
// $deBlob: FUNCTION 007ab520
// $deBlob: FUNCTION 005412a0
// $deBlob: FUNCTION 00859f30
TDEFINE_CLASS( AInky_Baton_Elite );

// $deBlob: FUNCTION 007ab670
AInky_Baton_Elite::AInky_Baton_Elite()
{
	// NOTE: the base ctor (AInky_Baton) and the vtable are the compiler's
}

// $deBlob: FUNCTION 007ab6a0
AInky_Baton_Elite::~AInky_Baton_Elite()
{
}

// $deBlob: FUNCTION 007ab550
TINT AInky_Baton_Elite::GetTargetColour()
{
	TASSERT( m_uiType != 0 );
	return ( m_uiType & 0xF ) | 0x10;
}

// $deBlob: FUNCTION 007ab6d0
TINT AInky_Baton_Elite::GetMaxLife()
{
	return 4;
}

// $deBlob: FUNCTION 007ab530
void AInky_Baton_Elite::PlayDeathSound()
{
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x20 ), ASoundMap::EVENTID( 0x157 ), m_oMatrix.AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION ), 0 );
}

// $deBlob: FUNCTION 007ab5a0
void AInky_Baton_Elite::SetCustomData( TINT a_iCustomData )
{
	AInky_Baton::SetCustomData( a_iCustomData );

	if ( TUINT8( a_iCustomData ) != 0 )
	{
		// The colour of the baton type (the enemy colour maps) on the baton mesh (1)
		AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
		pModelInstance->SetFlag( AModelInstance2::FLAG( 0x1000 ), TTRUE );
		pModelInstance->UseLocalAmbientLight( TTRUE );
		pModelInstance->SetAmbientColour( ABlobColourMaps::ms_pEnemyLightColourMap[ TUINT8( a_iCustomData ) ] );
		pModelInstance->SetMeshFlags( 1, 1, TTRUE );
		pModelInstance->SetMeshColour( 1, ABlobColourMaps::ms_pEnemyBodyColourMap[ TUINT8( a_iCustomData ) ] );
	}

	// The animation of the baton type (+0x63, the high byte of m_iCollisionGroupAndType)
	TUINT8* pPriorityType = TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType ) + 1;
	switch ( m_uiType )
	{
		case 1: *pPriorityType = 0x18; break;
		case 2: *pPriorityType = 0x19; break;
		case 3: *pPriorityType = 0x1A; break;
		case 4: *pPriorityType = 0x17; break;
		case 5: *pPriorityType = 0x1B; break;
		case 6: *pPriorityType = 0x1C; break;
		case 7: *pPriorityType = 0x1D; break;
	}
}
