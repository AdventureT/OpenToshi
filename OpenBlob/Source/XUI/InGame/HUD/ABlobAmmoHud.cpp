#include "pch.h"
#include "ABlobAmmoHud.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameInterface/APlayerManager.h"

#include <XUI/TXUIFigure.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobAmmoHud ) == 0x134 );

// $deBlob: FUNCTION 0064d8a0
// $deBlob: FUNCTION 0064d8b0
// $deBlob: FUNCTION 0064d930
// $deBlob: FUNCTION 0064d9b0
// $deBlob: FUNCTION 0064d9c0
// $deBlob: FUNCTION 0053dfd0
// $deBlob: FUNCTION 00858b70
TDEFINE_CLASS( ABlobAmmoHud );

// $deBlob: FUNCTION 0064dbd0
ABlobAmmoHud::ABlobAmmoHud()
    : m_uiPlayer( 0 )
{
}

// $deBlob: FUNCTION 0064d9d0
// $deBlob: FUNCTION 0064dc00
// $deBlob: FUNCTION 0064dbf0
ABlobAmmoHud::~ABlobAmmoHud()
{
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( m_uiPlayer ) );

	if ( pBlob != TNULL )
		pBlob->SetAmmoHud( TNULL );
}

// $deBlob: FUNCTION 0064da50
void ABlobAmmoHud::SetPlayer( TUINT8 a_uiPlayer )
{
	TASSERT( a_uiPlayer < APlayerManager::MAX_PLAYERS );
	m_uiPlayer = a_uiPlayer;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( a_uiPlayer ) );

	if ( pBlob != TNULL )
		pBlob->SetAmmoHud( this );
}

// $deBlob: FUNCTION 0064dab0
void ABlobAmmoHud::SetColour( TUINT32 a_uiColour )
{
	if ( a_uiColour == 0 || m_pVisual == TNULL )
	{
		SetVisible( TFALSE );
		return;
	}

	SetVisible( TTRUE );

	for ( T2GUIElement* pChild = m_pVisual->ChildBegin(); pChild != TNULL; pChild = m_pVisual->GetNextChild( pChild ) )
	{
		if ( !pChild->GetFlag( T2GUIElement::FLAGS_XUIELEMENT ) )
			continue;

		TXUIElement* pXUIChild = TSTATICCAST( TXUIElement, pChild );
		TASSERT( TTRUE == pXUIChild->IsA( &TGetClass( TXUIFigure ) ) );

		// NOTE: retail splits the colour into static components and joins them again
		TSTATICCAST( TXUIFigure, pXUIChild )->SetFillColour( ABlobColourComponent::ms_auiColours[ a_uiColour ] );
	}
}
