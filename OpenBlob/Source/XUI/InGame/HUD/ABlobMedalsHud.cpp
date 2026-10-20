#include "pch.h"
#include "ABlobMedalsHud.h"
#include "GameController/AGameStatus.h"

#include <XUI/TXUIControl.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00657960
// $deBlob: FUNCTION 00657970
// $deBlob: FUNCTION 006579e0
// $deBlob: FUNCTION 00657a50
// $deBlob: FUNCTION 00657a60
// $deBlob: FUNCTION 0053e400
// $deBlob: FUNCTION 00858c70
TDEFINE_CLASS( ABlobMedalsHud );

// $deBlob: FUNCTION 00657b30
ABlobMedalsHud::ABlobMedalsHud()
    : m_pMedal( TNULL )
    , m_pMedalText( TNULL )
    , m_pMedals( TNULL )
    , m_pUnk13C( TNULL )
    , m_iUnk140( -1 )
    , m_iUnk144( -1 )
{
}

// $deBlob: FUNCTION 00657a70
// $deBlob: FUNCTION 00657b90
// $deBlob: FUNCTION 00657b85
ABlobMedalsHud::~ABlobMedalsHud()
{
	AGameStatus::GetSingleton()->GetLevelData().m_pMedalsHud = TNULL;
}

// $deBlob: FUNCTION 00657aa0
void ABlobMedalsHud::OnCreated()
{
	AGameStatus::GetSingleton()->GetLevelData().m_pMedalsHud = this;

	TXUIControl* pMedal = FindChildXUIControl( L"Medal" );
	m_pMedals           = pMedal->GetVisual()->FindChildXUIElement( L"Medals" );
}

// $deBlob: FUNCTION 00657ad0
void ABlobMedalsHud::ShowNormal()
{
	m_iUnk140 = -1;
	m_iUnk144 = -1;
	PlayAnimation( L"Normal", L"EndNormal" );
}

// $deBlob: FUNCTION 00657b00
Toshi::TXUIElement* ABlobMedalsHud::SetMedal( Toshi::TXUIElement* a_pMedal )
{
	m_pMedal     = a_pMedal;
	m_pMedalText = TSTATICCAST( TXUIControl, a_pMedal->FindChildXUIElement( 0x145 ) );
	return m_pMedalText;
}
