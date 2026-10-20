#include "pch.h"
#include "ABlobVistaHud.h"
#include "XUI/InGame/ABlobInGame.h"
#include "GameInterface/APlayerManager.h"
#include "GameController/AGameStatus.h"
#include "Input/ABlobInputMap.h"
#include "Locale/ALocaleManager.h"
#include <XUI/TXUI.h>
#include <XUI/TXUIBackButton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0065bc80
// $deBlob: FUNCTION 0065bc90
// $deBlob: FUNCTION 0065bd30
// $deBlob: FUNCTION 0065bdc0
// $deBlob: FUNCTION 0065bdd0
// $deBlob: FUNCTION 0053e640
// $deBlob: FUNCTION 00858d00
TDEFINE_CLASS( ABlobVistaHud );

// $deBlob: FUNCTION 0065c040
ABlobVistaHud::ABlobVistaHud()
    : m_pUnk130( TNULL ) // NOTE: retail's ctor only clears 0x130, 0x134 and 0x144
    , m_pUnk134( TNULL )
#ifdef DEBLOB_COMMUNITY_PATCH
    , m_pUnk138( TNULL )
    , m_pUnk13C( TNULL )
    , m_pUnk140( TNULL )
#endif
    , m_iUnk144( 0 )
{
}

// $deBlob: FUNCTION 0065bdf0
// $deBlob: FUNCTION 0065c080
// $deBlob: FUNCTION 0065c077
ABlobVistaHud::~ABlobVistaHud()
{
	APlayerManager::GetSingleton()->SetVistaHud( TNULL );
	ABlobInputMap::GetSingleton()->DisableFor( 0.1f );
}

// $deBlob: FUNCTION 0065bde0
TBOOL ABlobVistaHud::Create( TXUIResource& a_rResource, XURXUIElementData* a_pData, TBOOL a_bCreateChildren )
{
	return TXUIScene::Create( a_rResource, a_pData, a_bCreateChildren );
}

// $deBlob: FUNCTION 0065be30
void ABlobVistaHud::OnCreated()
{
	SetAcceptsInput( TFALSE );
	APlayerManager::GetSingleton()->SetVistaHud( this );

	m_pUnk130 = TSTATICCAST( TXUIControl, FindChildXUIElement( 0xE ) );
	m_pUnk134 = FindChildXUIElement( 0xD );
	m_pUnk138 = TSTATICCAST( TXUIControl, FindChildXUIElement( 0xA ) );
	m_pUnk13C = FindChildXUIElement( 0xC );
	m_pUnk13C->SetVisible( TFALSE );
	m_pUnk140 = FindChildXUIElement( 9 );

	m_pUnk130->SetTextPointer( T2Locale::GetSingleton()->GetString( APlayerManager::GetTempVistaScore().m_iRank ) );

	if ( AGameStatus::GetSingleton()->IsLevelComplete() )
	{
		if ( m_pBackButton != TNULL )
			m_pBackButton->SetVisible( TFALSE );
	}
	else
	{
		FindChildXUIControl( 0xF )->SetVisible( TFALSE );
	}
}

// $deBlob: FUNCTION 0065bfa0
void ABlobVistaHud::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pPrevScene != TNULL && m_pPrevScene->IsA( &TGetClass( ABlobInGame ) ) )
		APlayerManager::GetSingleton()->UpdateVista( a_fDeltaTime );

	TXUI::GetSingleton()->TickInput( a_fDeltaTime );
	TXUIScene::Tick( a_fDeltaTime );
}

// TXUIScene::TransBackFrom, resetting the player's vista data first unless the level is complete
// $deBlob: FUNCTION 0065c010
void ABlobVistaHud::TransBackFromVista()
{
	if ( !AGameStatus::GetSingleton()->IsLevelComplete() )
		APlayerManager::GetSingleton()->ResetPlayerUnk( 0 );

	TXUIScene::TransBackFrom();
}

// $deBlob: FUNCTION 0065bf60
TBOOL ABlobVistaHud::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Accept && APlayerManager::GetSingleton()->GetVistaState() == APlayerManager::VistaState_Shown )
	{
		TXUI::GetSingleton()->TransitionBack( this );
		return TTRUE;
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 0065bef0
void ABlobVistaHud::SetScore( TINT a_iScore )
{
	TStringManager::String16Format( m_awszText, T2Locale::GetSingleton()->GetString( 0xBE ), a_iScore );
	m_pUnk138->SetTextPointer( m_awszText );
}
