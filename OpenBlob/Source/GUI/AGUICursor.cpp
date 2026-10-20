#include "pch.h"
#include "AGUICursor.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <XUI/TXUIRenderer.h>
#include <XUI/TXUIVisual.h>
#include <GUI/T2GUI.h>
#include <Platform/Windows/Input/TInputDeviceController_Wiin.h>

#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TRender_DX11.h>
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The hot spot of the cursor image
static const TFLOAT s_fCursorHotSpotX = 5.49f;
static const TFLOAT s_fCursorHotSpotY = 9.69f;

// $deBlob: FUNCTION 005d6f50
AGUICursor::AGUICursor( TInputDevice* a_pDevice, TUINT32 a_uiUnused, TINT a_iPlayer )
{
	m_pDevice      = a_pDevice;
	m_bVisible     = TTRUE;
	m_pPlayerIndex = TNULL;
	m_uiColour     = 0xFFFFFFFF;
	m_iPlayer      = a_iPlayer;
	m_bMPColour    = TFALSE;

	CreateXUIScene( a_iPlayer );
	TXUI::GetSingleton()->GetInput()->SetEnabled( TFALSE );

	// Only shown in windowed mode (the system cursor is hidden over the window)
	if ( TRender::GetSingleton()->GetCurrentDisplayParams()->bWindowed )
		m_pScene->SetVisible( TFALSE );
	else
		m_pScene->SetVisible( TTRUE );
}

// $deBlob: FUNCTION 005d6fc0
// $deBlob: FUNCTION 005d6e00
AGUICursor::~AGUICursor()
{
	TXUI::GetSingleton()->GetInput()->SetClickElement( TNULL );

	if ( m_pScene != TNULL )
		delete m_pScene;
}

// $deBlob: FUNCTION 005d6fe0
void AGUICursor::Reinitialize( TINT a_iPlayer )
{
	if ( m_pScene != TNULL )
		delete m_pScene;

	CreateXUIScene( a_iPlayer );
}

// $deBlob: FUNCTION 005d7010
void AGUICursor::Tick( TFLOAT a_fDeltaTime )
{
	if ( !m_bVisible )
		return;

	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();
	TXUIInput*              pInput         = TXUI::GetSingleton()->GetInput();

	m_pScene->SetPosition( TFLOAT( pInput->GetMouseX() ) - m_fOffsetX, TFLOAT( pInput->GetMouseY() ) - m_fOffsetY );

	if ( pDisplayParams->bWindowed == TTRUE )
	{
		// The cursor is shown while the mouse is over the window, or with a Wii remote
#ifdef TRENDERINTERFACE_DX11
		TBOOL bCursorHidden = TRenderDX11::Interface()->GetMSWindow()->IsCursorHidden();
#else
		TBOOL bCursorHidden = TFALSE;
#endif

		if ( bCursorHidden == TTRUE )
			m_pScene->SetVisible( TTRUE );
		else
			m_pScene->SetVisible( TInputDevice::GetRegisteredDevices()[ TInputDevice::GetLastXUIDeviceIndex() ]->IsA( &TGetClass( TInputWiinDeviceController ) ) );
	}

	// The player index is shown with 2+ players, the multiplayer colour is dropped when the player is gone
	if ( APlayerManager* pPlayerManager = APlayerManager::GetSingleton() )
	{
		TINT iNumPlayers = pPlayerManager->GetNumPlayers();
		m_pPlayerIndex->SetVisible( iNumPlayers > 1 );

		if ( iNumPlayers == 1 || pPlayerManager->GetPlayerColour( m_iPlayer ) == 0 )
			m_bMPColour = TFALSE;
	}

	// The scene is ticked with the XUI renderer
	T2GUIRenderer* pOldRenderer = T2GUI::GetSingleton()->GetRenderer();
	T2GUI::GetSingleton()->SetRenderer( TXUI::GetSingleton()->GetRenderer() );
	m_pScene->Tick( a_fDeltaTime );
	T2GUI::GetSingleton()->SetRenderer( pOldRenderer );
}

// $deBlob: FUNCTION 005d7150
void AGUICursor::Render()
{
	TBOOL bSceneVisible = m_pScene->IsVisible();

	if ( m_bVisible == TTRUE && bSceneVisible == TTRUE )
	{
		m_pScene->PreRender();
		m_pScene->Render();
		m_pScene->PostRender();
	}
}

// $deBlob: FUNCTION 005d7190
void AGUICursor::SetVisible( TBOOL a_bVisible, TBOOL a_bUnused )
{
	m_bVisible = a_bVisible;
	TXUI::GetSingleton()->GetInput()->SetCursorVisible( a_bVisible );
}

// $deBlob: FUNCTION 005d71b0
void AGUICursor::SetActive( TBOOL a_bActive )
{
	TXUI::GetSingleton()->GetInput()->SetEnabled( a_bActive );
}

// $deBlob: FUNCTION 005d71d0
void AGUICursor::GetPosition( TVector2& a_rPosition )
{
	const TVector2& rPos = m_pScene->GetTransform().GetPos();
	a_rPosition.x        = m_fOffsetX + rPos.x;
	a_rPosition.y        = rPos.y + m_fOffsetY;
}

// NOTE: retail splits the colour into four static bytes and puts it back together
// $deBlob: FUNCTION 005d7200
void AGUICursor::SetMPColour( TUINT32 a_uiColour )
{
	m_uiColour = a_uiColour;
	m_pCircleImage->SetColour( a_uiColour );
	m_bMPColour = TTRUE;
}

// $deBlob: FUNCTION 005d7270
void AGUICursor::SetColour( TUINT32 a_uiColour )
{
	m_pCircleImage->SetColour( a_uiColour );
}

// $deBlob: FUNCTION 005d7280
void AGUICursor::SetBlobVision( TBOOL a_bBlobVision )
{
	if ( a_bBlobVision )
	{
		m_pBlobVisionGroup->SetVisible( TTRUE );
		m_pBlobVisionGroup1->SetVisible( TTRUE );
		m_pCircleGroup->SetVisible( TFALSE );

		// The colour of player 0's paint on the "image" of both groups
		ABlobEntity* pBlob    = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
		TUINT32      uiColour = pBlob->GetColourComponent()->GetCurrentColour();

		if ( uiColour & ABlobColourComponent::COLOUR_BLACK )
			uiColour = ABlobColourComponent::COLOUR_BLACK;

		// NOTE: retail splits the colour into four static bytes and puts it back together
		TUINT32 uiARGB = ABlobColourComponent::ms_auiColours[ uiColour ];

		m_pBlobVisionGroup->FindChildXUIElement( L"image" )->SetColour( uiARGB );
		m_pBlobVisionGroup1->FindChildXUIElement( L"image" )->SetColour( uiARGB );
		return;
	}

	m_pBlobVisionGroup->SetVisible( TFALSE );
	m_pBlobVisionGroup1->SetVisible( TFALSE );

	m_pCircleGroup->SetVisible( TTRUE );
}

// $deBlob: FUNCTION 005d7370
void AGUICursor::CreateXUIScene( TINT a_iPlayer )
{
	m_pScene = new ( TXUI::ms_pXUIMemoryBlock ) TXUIScene();
	m_pScene->SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	m_pScene->SetWidth( 40.0f );
	m_pScene->SetHeight( 40.0f );

	m_fOffsetX = m_pScene->GetWidth() * -0.5f;
	m_fOffsetY = m_pScene->GetHeight() * -0.5f;
	m_fOffsetX += s_fCursorHotSpotX;
	m_fOffsetY += s_fCursorHotSpotY;
	m_pScene->SetXUIPivot( m_fOffsetX, m_fOffsetY );

	// NOTE: retail passes a TNULL resource: it's only used when the skin has no "Cursor" visual
	m_pScene->CreateVisual( *TSTATICCAST( TXUIResource, TNULL ), L"Cursor" );

	m_pCircleGroup      = m_pScene->GetVisual()->FindChildXUIElement( L"CircleGroup" );
	m_pCircleImage      = m_pCircleGroup->FindChildXUIElement( L"XuiImage1" );
	m_pBlobVisionGroup  = m_pScene->GetVisual()->FindChildXUIElement( L"BlobVisionGroup" );
	m_pBlobVisionGroup1 = m_pScene->GetVisual()->FindChildXUIElement( L"BlobVisionGroup1" );

	m_pBlobVisionGroup->SetVisible( TFALSE );
	m_pBlobVisionGroup1->SetVisible( TFALSE );
	m_pScene->PlayVisual( TFALSE, TFALSE, TFALSE );

	TXUI::GetSingleton()->GetInput()->SetClickElement( m_pCircleGroup );

	switch ( a_iPlayer )
	{
		case 0: m_pScene->SetText( L"1", -1 ); break;
		case 1: m_pScene->SetText( L"2", -1 ); break;
		case 2: m_pScene->SetText( L"3", -1 ); break;
		case 3: m_pScene->SetText( L"4", -1 ); break;
	}

	m_pPlayerIndex = m_pScene->GetVisual()->FindChildXUIElement( L"PlayerIndex" );
	m_pPlayerIndex->SetVisible( TFALSE );
	m_bMPColour = TFALSE;
}
