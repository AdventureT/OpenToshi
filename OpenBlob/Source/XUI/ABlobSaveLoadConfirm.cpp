#include "pch.h"
#include "ABlobSaveLoadConfirm.h"
#include "ABlobMenu.h"

#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0062b020
// $deBlob: FUNCTION 0062b030
// $deBlob: FUNCTION 0062b0c0
// $deBlob: FUNCTION 0062b150
// $deBlob: FUNCTION 0062b160
// $deBlob: FUNCTION 0053d110
// $deBlob: FUNCTION 008587c0
TDEFINE_CLASS( ABlobSaveLoadConfirm );

// $deBlob: FUNCTION 0062b220
ABlobSaveLoadConfirm::ABlobSaveLoadConfirm()
{
	m_pParentMenu = TNULL;
	m_bHasCancel  = TFALSE;
}

// $deBlob: FUNCTION 0062b250
// $deBlob: FUNCTION 0062b255
// $deBlob: FUNCTION 0062b260
ABlobSaveLoadConfirm::~ABlobSaveLoadConfirm()
{
}

// $deBlob: FUNCTION 0062b200
void ABlobSaveLoadConfirm::SetMessageText( const TWCHAR* a_wszText )
{
	SetPresenterText( a_wszText, 0, -1 );
}

// $deBlob: FUNCTION 0062b170
void ABlobSaveLoadConfirm::OnCreated()
{
	// NOTE: retail doesn't call TXUIScene::OnCreated
	m_pParentScene = FindParentScene();
	m_bHasCancel   = FindChildXUIControl( L"Cancel" ) != TNULL;

	TXUIElement* pNineGrid = m_pVisual->FindChildXUIElement( L"XuiNineGrid1" );

	if ( pNineGrid != TNULL )
		pNineGrid->SetColour( 0xFF3B839C );
}

// $deBlob: FUNCTION 0062b1c0
TBOOL ABlobSaveLoadConfirm::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Accept || ( a_eButton == XUIButton_Back && m_bHasCancel ) )
	{
		if ( m_pParentMenu != TNULL )
		{
			m_pParentMenu->OnConfirmationClosed( a_eButton == XUIButton_Accept );
			return TTRUE;
		}
	}

	return TFALSE;
}
