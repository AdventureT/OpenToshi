#include "pch.h"
#include "ABlobSaveLoadMessage.h"

#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0062b290
// $deBlob: FUNCTION 0062b2a0
// $deBlob: FUNCTION 0062b320
// $deBlob: FUNCTION 0062b3a0
// $deBlob: FUNCTION 0062b3b0
// $deBlob: FUNCTION 0053d150
// $deBlob: FUNCTION 008587d0
TDEFINE_CLASS( ABlobSaveLoadMessage );

// $deBlob: FUNCTION 0062b410
ABlobSaveLoadMessage::ABlobSaveLoadMessage()
{
}

// $deBlob: FUNCTION 0062b430
// $deBlob: FUNCTION 0062b435
// $deBlob: FUNCTION 0062b440
ABlobSaveLoadMessage::~ABlobSaveLoadMessage()
{
}

// $deBlob: FUNCTION 0062b3c0
void ABlobSaveLoadMessage::SetMessageText( const TWCHAR* a_wszText )
{
	SetPresenterText( a_wszText, 0, -1 );
}

// $deBlob: FUNCTION 0062b3e0
void ABlobSaveLoadMessage::OnCreated()
{
	// NOTE: retail doesn't call TXUIScene::OnCreated
	m_pParentScene = FindParentScene();

	TXUIElement* pNineGrid = m_pVisual->FindChildXUIElement( L"XuiNineGrid1" );

	if ( pNineGrid != TNULL )
		pNineGrid->SetColour( 0xFF3B839C );
}
