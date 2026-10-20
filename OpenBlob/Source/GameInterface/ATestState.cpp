#include "pch.h"
#include "GUI/AGUIManager.h"
#include "ATestState.h"
#include "GUI/AFontManager.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextBox.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ATestState ) == 0x2C );

// $deBlob: FUNCTION 005d2d10
// $deBlob: FUNCTION 005d2d20
// $deBlob: FUNCTION 005d2d90
// $deBlob: FUNCTION 005d2e00
// $deBlob: FUNCTION 005d2e10
// $deBlob: FUNCTION 0053c7b0
// $deBlob: FUNCTION 00858530
TDEFINE_CLASS( ATestState );

// An ARGB colour from 0-1 components
// $deBlob: FUNCTION 005d2fb0
static TUINT32 MakeColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return TUINT32( TINT( a_fBlue * 255.0f ) ) |
	    ( ( TUINT32( TINT( a_fGreen * 255.0f ) ) | ( ( ( TUINT32( TINT( a_fAlpha * 255.0f ) ) << 8 ) | TUINT32( TINT( a_fRed * 255.0f ) ) ) << 8 ) ) << 8 );
}

// $deBlob: FUNCTION 005d2e20
ATestState::ATestState()
{
}

// $deBlob: FUNCTION 005d2e40
// $deBlob: FUNCTION 005d3010
ATestState::~ATestState()
{
}

// $deBlob: FUNCTION 005d2e50
AGameState::UpdateResult ATestState::OnUpdate( TFLOAT a_fDeltaTime )
{
	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005d2e70
void ATestState::OnInsertion()
{
	T2GUIRectangle* pRectangle = new T2GUIRectangle();
	pRectangle->Create( 100.0f, 100.0f );
	pRectangle->SetColour( 0x3F00FF00 );

	AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( pRectangle );

	T2GUITextBox* pTextBox = new T2GUITextBox();
	pTextBox->Create( AFontManager::sm_pMsgFont, 300.0f );
	pTextBox->SetText( L"Blob07" );
	pTextBox->SetMaxChars( -1 );

	AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( pTextBox );

	AGameState::OnInsertion();
}

// $deBlob: FUNCTION 005d2f80
void ATestState::OnRemoval()
{
	AGameState::OnRemoval();
}

// $deBlob: FUNCTION 005d2f90
void ATestState::OnActivate()
{
	AGameState::OnActivate();
}

// $deBlob: FUNCTION 005d2fa0
void ATestState::OnDeactivate()
{
	AGameState::OnDeactivate();
}
