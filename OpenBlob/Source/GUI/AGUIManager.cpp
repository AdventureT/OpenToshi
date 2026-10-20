#include "pch.h"
#include "AGUIManager.h"
#include "AGUICursor.h"
#include "ASafeZone.h"
#include "XUI/FrontEnd/ABlobFrontEnd.h"
#include "XUI/ABlobCursorTrail.h"

#include <GUI/T2GUI.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIInput.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Input/TInputDeviceController.h>
#include <Input/TInputDeviceMouse.h>
#include <Platform/Windows/Input/TInputDeviceController_Wiin.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The margins of the safe area (zero: the whole screen)
static TINT s_iSafeAreaTop    = 0;
static TINT s_iSafeAreaBottom = 0;
static TINT s_iSafeAreaLeft   = 0;
static TINT s_iSafeAreaRight  = 0;

// The time the cursor stays visible after the mouse was last used
static TCONSTEXPR_VAR TFLOAT CURSOR_SHOW_TIME = 3.0f;

// NOTE: retail's cursor calls loop four times over the same cursor (the Wii build has a cursor per remote)
static TCONSTEXPR_VAR TINT NUM_CURSOR_CALLS = 4;

// $deBlob: FUNCTION 005d6190
AGUIManager::AGUIManager()
{
	m_pSafeArea    = TNULL;
	m_fCursorTime  = 0.0f;
	m_fHideDelay   = -1.0f;
	m_iLastMouseX  = 0;
	m_iLastMouseY  = 0;
	m_bCursorShown = TFALSE;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail doesn't initialise m_bCursorEnabled (the instance is zeroed by nothing, the menus set it)
	m_bCursorEnabled = TFALSE;
#endif

	m_pCursor = new AGUICursor( TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 ), 0xFFFFFFFF, 0 );
	m_pCursor->SetActive( TTRUE );

	CreateSafeArea();

	m_pSafeZone = new ASafeZone();
	m_pSafeZone->Create();
	m_pSafeArea->AddChildTail( m_pSafeZone );

	m_GUIMasks.Push( T2GUIElement::STATE_MENU );
	T2GUIElement::s_uiGlobalVisMask = T2GUIElement::STATE_MENU;
}

// $deBlob: FUNCTION 005d62e0
// $deBlob: FUNCTION 0056bfe0
AGUIManager::~AGUIManager()
{
	m_fCursorTime = 0.0f;
	m_fHideDelay  = -1.0f;
	m_iLastMouseX = 0;
	m_iLastMouseY = 0;

	m_pSafeZone->RemoveSelf();

	if ( m_pSafeZone != TNULL )
		delete m_pSafeZone;

	m_pSafeZone = TNULL;
	RemoveCurrentGUIMask();

	if ( m_pCursor != TNULL )
		delete m_pCursor;

	if ( m_pSafeArea != TNULL )
	{
		m_pSafeArea->RemoveSelf();

		if ( m_pSafeArea != TNULL )
			delete m_pSafeArea;

		m_pSafeArea = TNULL;
	}
}

// $deBlob: FUNCTION 005d6410
void AGUIManager::Reinitialize()
{
	m_fCursorTime = 0.0f;
	m_fHideDelay  = -1.0f;
	m_iLastMouseX = 0;
	m_iLastMouseY = 0;
	m_pCursor->Reinitialize( 0 );
}

// $deBlob: FUNCTION 005d6430
void AGUIManager::Tick( TFLOAT a_fDeltaTime )
{
	if ( !m_bCursorEnabled )
	{
		if ( m_bCursorShown )
		{
			m_bCursorShown = TFALSE;

			for ( TINT i = 0; i < NUM_CURSOR_CALLS; i++ )
				m_pCursor->SetVisible( m_bCursorShown, TFALSE );
		}

		m_pCursor->Tick( a_fDeltaTime );
		return;
	}


	// A Wii remote always shows its pointer in a menu
	if ( TInputDevice::GetRegisteredDevices()[ TInputDevice::GetLastXUIDeviceIndex() ]->IsA( &TGetClass( TInputWiinDeviceController ) ) )
	{
		m_bCursorShown = TTRUE;

		if ( TXUI::GetSingleton()->GetActiveScene() == TNULL )
			m_bCursorShown = TFALSE;

		for ( TINT i = 0; i < NUM_CURSOR_CALLS; i++ )
			m_pCursor->SetVisible( m_bCursorShown, TFALSE );

		m_pCursor->Tick( a_fDeltaTime );
		return;
	}

	TBOOL bWasShown   = m_bCursorShown;
	TBOOL bHidden     = TFALSE;
	TINT  iMouseX     = TXUI::GetSingleton()->GetInput()->GetMouseX();
	TINT  iMouseY     = TXUI::GetSingleton()->GetInput()->GetMouseY();
	TBOOL bMouseMoved = m_iLastMouseX != iMouseX || m_iLastMouseY != iMouseY;

	// A key or a button hides the cursor
	if ( bWasShown )
	{
		TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

		for ( TINT iDoodad = TInputDeviceKeyboard::DOODAD_START; iDoodad < TInputDeviceKeyboard::DOODAD_END; iDoodad++ )
		{
			if ( pKeyboard->IsDown( iDoodad ) )
			{
				m_iLastMouseX  = iMouseX;
				m_iLastMouseY  = iMouseY;
				m_bCursorShown = TFALSE;
				bHidden        = m_fCursorTime < CURSOR_SHOW_TIME;
				break;
			}
		}
	}

	TINT iNumDevices = TInputInterface::GetSingleton()->GetDeviceList().Count();

	for ( TINT i = 0; m_bCursorShown && i < iNumDevices; i++ )
	{
		TInputDeviceController* pController = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceController>( i );

		if ( pController != TNULL && pController->IsEnabled() )
		{
			for ( TINT iDoodad = 0x10000; iDoodad < 0x1001C; iDoodad++ )
			{
				if ( pController->IsDown( iDoodad ) )
				{
					m_iLastMouseX  = iMouseX;
					m_iLastMouseY  = iMouseY;
					m_bCursorShown = TFALSE;
					bHidden        = m_fCursorTime < CURSOR_SHOW_TIME;
					break;
				}
			}
		}
	}

	// A mouse button shows it
	TBOOL bMouseButton = TFALSE;

	if ( !m_bCursorShown )
	{
		// NOTE: retail doesn't check that there is a mouse
		TInputDeviceMouse* pMouse = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceMouse>( 0 );

		for ( TINT iDoodad = 0x30000; iDoodad < 0x3000D; iDoodad++ )
		{
			if ( pMouse->IsDown( iDoodad ) )
			{
				bMouseButton = TTRUE;
				break;
			}
		}
	}

	if ( m_iLastMouseX != iMouseX || m_iLastMouseY != iMouseY || bMouseButton )
	{
		m_fCursorTime  = 0.0f;
		m_bCursorShown = TTRUE;
		m_iLastMouseX  = iMouseX;
		m_iLastMouseY  = iMouseY;
	}

	if ( m_fCursorTime < CURSOR_SHOW_TIME )
		m_fCursorTime += a_fDeltaTime;

	if ( TXUI::GetSingleton()->GetActiveScene() == TNULL )
		m_bCursorShown = TFALSE;

	m_bCursorShown &= m_fCursorTime < CURSOR_SHOW_TIME;

	// Hidden by a key while the mouse was in use: shown again after a short delay
	if ( !m_bCursorShown && bHidden )
	{
		TBOOL bShow = bMouseMoved;

		if ( !bMouseMoved )
		{
			if ( m_fHideDelay == -1.0f )
			{
				m_fHideDelay = 0.1f;
				bShow        = TTRUE;
			}
			else if ( m_fHideDelay <= 0.0f )
			{
				m_fHideDelay = -1.0f;
			}
			else
			{
				m_fHideDelay -= a_fDeltaTime;

				if ( m_fHideDelay <= 0.0f )
					m_fHideDelay = -1.0f;
				else
					bShow = TTRUE;
			}
		}

		if ( bShow )
		{
			m_bCursorShown = TTRUE;
			m_fCursorTime  = 2.99f;
		}
	}

	for ( TINT i = 0; i < NUM_CURSOR_CALLS; i++ )
		m_pCursor->SetVisible( m_bCursorShown, TFALSE );

	// The cursor got hidden: the cursor trails fade out
	if ( !m_bCursorShown && bWasShown && TXUI::GetSingleton()->GetActiveScene() != TNULL )
	{
		TXUIScene* pScene = TXUI::GetSingleton()->GetActiveScene();

		// NOTE: retail calls FindParentScene and ignores the result: the active scene itself is checked
		pScene->FindParentScene();

		if ( pScene->IsA( &TGetClass( ABlobFrontEnd ) ) )
		{
			// ABlobFrontEnd (the cursor trails of the frontend)
			TSTATICCAST( ABlobFrontEnd, pScene )->ClampCursorTrailFades();
		}
		else
		{
			// All the cursor trails
			ABlobCursorTrail::ClampAllFades();
		}
	}

	m_pCursor->Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 005d6820
TUINT32 AGUIManager::GetGUIMask()
{
	if ( m_GUIMasks.Empty() )
		return 0;

	return m_GUIMasks.Top();
}

// $deBlob: FUNCTION 005d6870
void AGUIManager::SetGUIMask( TUINT32 a_uiMask )
{
	m_GUIMasks.Push( a_uiMask );
	T2GUIElement::s_uiGlobalVisMask = TUINT8( a_uiMask );
}

// $deBlob: FUNCTION 005d6890
void AGUIManager::RemoveCurrentGUIMask()
{
	TBOOL bIsEmpty = m_GUIMasks.Empty();
	TASSERT( TFALSE == bIsEmpty );

	m_GUIMasks.Pop();

	if ( m_GUIMasks.Empty() )
	{
		T2GUIElement::s_uiGlobalVisMask = 0;
		return;
	}

	T2GUIElement::s_uiGlobalVisMask = TUINT8( m_GUIMasks.Top() );
}

// $deBlob: FUNCTION 005d6950
void AGUIManager::SetCursorVisibleCount( TINT a_iCount )
{
	if ( a_iCount > 0 )
		m_pCursor->SetVisible( TTRUE, TTRUE );
	else
		m_pCursor->SetVisible( TFALSE, TTRUE );
}

// $deBlob: FUNCTION 005d6980
void AGUIManager::SetCursorActiveCount( TINT a_iCount )
{
	if ( a_iCount > 0 )
		m_pCursor->SetActive( TTRUE );
	else
		m_pCursor->SetActive( TFALSE );
}

// $deBlob: FUNCTION 005d69b0
void AGUIManager::SetCursorVisible( TBOOL a_bVisible, TBOOL a_bUnused )
{
	m_bCursorEnabled = a_bVisible;

	for ( TINT i = 0; i < NUM_CURSOR_CALLS; i++ )
		m_pCursor->SetVisible( a_bVisible, a_bUnused );
}

// $deBlob: FUNCTION 005d69e0
void AGUIManager::SetCursorActive( TBOOL a_bActive )
{
	for ( TINT i = 0; i < NUM_CURSOR_CALLS; i++ )
		m_pCursor->SetActive( a_bActive );
}

// $deBlob: FUNCTION 005d6a10
TUINT32 AGUIManager::GetCursorMask()
{
	return m_pCursor->IsVisible() ? 4 : 0;
}

// $deBlob: FUNCTION 005d6a30
void AGUIManager::SetCursorEnabled( TBOOL a_bEnabled )
{
	m_bCursorEnabled = a_bEnabled;
	m_pCursor->SetVisible( a_bEnabled, TTRUE );
}

// $deBlob: FUNCTION 005d6a50
TBOOL AGUIManager::ToggleSafeZone( void* a_pUnused )
{
	ASafeZone* pSafeZone = AGUIManager::GetSingleton()->m_pSafeZone;
	pSafeZone->SetVisible( !pSafeZone->IsVisible() );
	return TTRUE;
}

// $deBlob: FUNCTION 005d6aa0
void AGUIManager::CreateSafeArea()
{
	TASSERT( TNULL == m_pSafeArea );

	m_pSafeArea = new T2GUIElement();
	m_pSafeArea->SetVisible( TTRUE );
	m_pSafeArea->SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );

	T2GUIElement* pRoot = T2GUI::GetSingleton()->GetRootElement();

	TFLOAT fWidth, fHeight;
	pRoot->GetDimensions( fWidth, fHeight );
	m_pSafeArea->SetDimensions( ( fWidth - TFLOAT( s_iSafeAreaLeft ) ) - TFLOAT( s_iSafeAreaRight ), ( fHeight - TFLOAT( s_iSafeAreaTop ) ) - TFLOAT( s_iSafeAreaBottom ) );

	pRoot->AddChildTail( m_pSafeArea );
	m_pSafeArea->SetState( T2GUIElement::STATE_ALL & ~T2GUIElement::STATE_UNK1 );
}
