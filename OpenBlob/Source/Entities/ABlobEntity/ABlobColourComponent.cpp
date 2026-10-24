#include "pch.h"
#include "Render/ARenderer.h"
#include "ABlobColourComponent.h"
#include "XUI/InGame/HUD/ABlobAmmoHud.h"
#include "ABlobColourMaps.h"
#include "ABlobEntity.h"
#include "ABlobZJumpComponent.h"
#include "ARandom.h"
#include "ABlobAnimationComponent.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "XUI/InGame/HUD/ABlobHud.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TUINT32 ABlobColourComponent::ms_auiColours[ COLOUR_NUMOF ] = {
	0x00ffffff,
	0xfff1d507,
	0xff0e697b,
	0xff668527,
	0xffae280d,
	0xfff38701,
	0xffa582bc,
	0xff706741,
	0xffff9480,
	0xff24d6cd,
	0xff8ce8b3,
	0xff7342ba,
	0xff005ea3,
	0xff94e735,
	0xffae780d,
	0xffe340a7,
	0xff2a2718,
};
const TUINT32 ABlobColourComponent::ms_auiDarkColours[ COLOUR_NUMOF ] = {
	0xff424242,
	0xff7e6500,
	0xff002c4a,
	0xff1a3800,
	0xff540000,
	0xff984411,
	0xff501757,
	0xff453100,
	0xff7a4a20,
	0xff126b33,
	0xff46742d,
	0xff39212e,
	0xff002f28,
	0xff4a730d,
	0xff573c03,
	0xff712028,
	0xff2a2718,
};

// The saturation and value of the rainbow colour (Tick)
static TFLOAT s_fRainbowSaturation = 1.0f;
static TFLOAT s_fRainbowValue      = 1.0f;

// $deBlob: FUNCTION 0075c650
ABlobColourComponent::ABlobColourComponent( ABlobEntity* a_pOwner )
    : AEntityComponent2( a_pOwner )
{
	m_fRainbowHue                                   = 0.0f;
	m_fRainbowHue2                                  = 0.0f;
	m_uiStartColour                                 = COLOUR_WHITE;
	m_uiUnk20                                       = 0;
	m_uiCurrentColour                               = COLOUR_WHITE;
	m_uiFadeColour                                  = 0;
	m_uiFadeColour2                                 = 0;
	*TREINTERPRETCAST( TUINT32*, m_aRainbowColour ) = 0;
	m_fFade                                         = 0.0f;
	m_bFadeUp                                       = TTRUE;

	TASSERT( ( a_pOwner == TNULL ) || a_pOwner->IsA( &TGetClass( ABlobEntity ) ) );
	m_pOwner = a_pOwner;
}

// $deBlob: FUNCTION 0075d3f0
ABlobColourComponent::~ABlobColourComponent()
{
}

// $deBlob: FUNCTION 0075c740
void ABlobColourComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( TUINT8( m_pOwner->m_uiFlags >> 0x15 ) & 1 )
	{
		m_fRainbowHue  = TFLOAT( fmod( TDOUBLE( 90.0f * a_fDeltaTime + m_fRainbowHue ), 360.0 ) );
		m_fRainbowHue2 = TFLOAT( fmod( TDOUBLE( 180.0f * a_fDeltaTime + m_fRainbowHue2 ), 360.0 ) );

		TUINT32 uiR, uiG, uiB;
		ARenderer::HSVToRGB( m_fRainbowHue2, s_fRainbowSaturation, s_fRainbowValue, &uiR, &uiG, &uiB );
		m_aRainbowColour[ 2 ] = TUINT8( uiR );
		m_aRainbowColour[ 1 ] = TUINT8( uiG );
		m_aRainbowColour[ 0 ] = TUINT8( uiB );
	}

	if ( !m_bFadeUp )
	{
		m_fFade -= a_fDeltaTime * 0.1f;
		if ( m_fFade < 0.0f ) m_bFadeUp = TTRUE;
	}
	else
	{
		m_fFade += a_fDeltaTime * 0.1f;
		if ( 1.0f < m_fFade ) m_bFadeUp = TFALSE;
	}

	TMath::Clip( m_fFade, 0.0f, 1.0f );

	TUINT32* pColours = ABlobColourMaps::ms_pColourInterpolationMap;
	TUINT32  uiColour = m_uiCurrentColour;
	m_uiFadeColour    = LerpColour( pColours[ uiColour * 2 ], pColours[ uiColour * 2 + 1 ], m_fFade );
	m_uiFadeColour2   = LerpColour( pColours[ uiColour * 2 ], pColours[ uiColour * 2 + 1 ], 1.0f - m_fFade );
}

// $deBlob: FUNCTION 0075c8e0
TUINT32 ABlobColourComponent::MixColours( TUINT32 a_uiColour, TUINT32 a_uiNewColour )
{
	if ( a_uiNewColour < 8 && ( a_uiColour < 8 || a_uiColour >= 0x10 ) )
	{
		if ( ( a_uiNewColour & a_uiColour ) != 0 )
			a_uiColour &= 0x10;

		return a_uiColour | a_uiNewColour;
	}

	return a_uiNewColour;
}

// $deBlob: FUNCTION 0075c910
void ABlobColourComponent::GetMixedColour( TUINT32& a_rOutColour, TUINT32 a_uiColour ) const
{
	TASSERT( 0 <= a_uiColour && a_uiColour < COLOUR_NUMOF );
	a_rOutColour = ms_auiColours[ MixColours( m_uiCurrentColour, a_uiColour ) ];
}

// $deBlob: FUNCTION 0075c990
void ABlobColourComponent::MixColour( TUINT32 a_uiColour )
{
	// The ink frame of the HUD (single player)
	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		if ( a_uiColour == COLOUR_BLACK )
		{
			if ( m_uiCurrentColour != COLOUR_BLACK && ABlobHud::GetSingleton() )
				ABlobHud::GetSingleton()->GetMaskParent()->PlayAnimation( L"Inked", L"EndInked" );
		}
		else if ( m_uiCurrentColour == COLOUR_BLACK && ABlobHud::GetSingleton() )
		{
			ABlobHud::GetSingleton()->GetMaskParent()->PlayAnimation( L"FromInked", L"EndFromInked" );
		}
	}

	m_uiCurrentColour = MixColours( m_uiCurrentColour, a_uiColour );
	TASSERT( 0 <= a_uiColour && a_uiColour < COLOUR_NUMOF );

	if ( m_pOwner->GetAmmoHud() != TNULL )
		m_pOwner->GetAmmoHud()->SetColour( m_uiCurrentColour );

	if ( ABlobZJumpComponent* pZJump = m_pOwner->GetZJumpComponent() )
		pZJump->OnColourChanged();

	if ( m_pOwner->GetAnimationComponent() )
		m_pOwner->GetAnimationComponent()->OnColourChanged();

	Tick( 0.0f );
}

// $deBlob: FUNCTION 0075cab0
void ABlobColourComponent::SetColour( TUINT32 a_uiColour )
{
	// The ink frame of the HUD (single player)
	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && !AGameStatus::GetSingleton()->IsMultiplayer() )
	{
		if ( a_uiColour == COLOUR_BLACK )
		{
			if ( m_uiCurrentColour != COLOUR_BLACK && ABlobHud::GetSingleton() )
				ABlobHud::GetSingleton()->GetMaskParent()->PlayAnimation( L"Inked", L"EndInked" );
		}
		else if ( m_uiCurrentColour == COLOUR_BLACK && ABlobHud::GetSingleton() )
		{
			ABlobHud::GetSingleton()->GetMaskParent()->PlayAnimation( L"FromInked", L"EndFromInked" );
		}
	}

	m_uiCurrentColour = a_uiColour;

	if ( m_pOwner->GetAmmoHud() != TNULL )
		m_pOwner->GetAmmoHud()->SetColour( a_uiColour );

	if ( ABlobZJumpComponent* pZJump = m_pOwner->GetZJumpComponent() )
		pZJump->OnColourChanged();

	if ( m_pOwner->GetAnimationComponent() )
		m_pOwner->GetAnimationComponent()->OnColourChanged();

	Tick( 0.0f );
}

// $deBlob: FUNCTION 0075d220
TINT ABlobColourComponent::GetRandomColour()
{
	return ARandom::GetSingleton()->GetRandom().GetInt( 7 ) + 1;
}

// $deBlob: FUNCTION 0075d240
TUINT32 ABlobColourComponent::GetComplementaryColour( TUINT32 a_uiColour )
{
	switch ( a_uiColour )
	{
		case 1: return 7;
		case 2: return 6;
		case 3: return 2;
		case 5: return 1;
		case 6: return 4;
		case 7: return 3;
		default: return 5;
	}
}

// $deBlob: FUNCTION 0075d2b0
void ABlobColourComponent::Reset()
{
	m_uiCurrentColour                               = m_uiStartColour;
	m_uiFadeColour                                  = 0;
	m_uiFadeColour2                                 = 0;
	*TREINTERPRETCAST( TUINT32*, m_aRainbowColour ) = 0;
	m_fFade                                         = 0.0f;
	m_bFadeUp                                       = TTRUE;
}

// $deBlob: FUNCTION 0075d2e0
TUINT32 ABlobColourComponent::GetInterpolatedColour( TINT a_iIndex, TFLOAT a_fT )
{
	TMath::Clip( a_fT, 0.0f, 1.0f );
	TUINT32* pColours = ABlobColourMaps::ms_pColourInterpolationMap;
	return LerpColour( pColours[ a_iIndex * 2 ], pColours[ a_iIndex * 2 + 1 ], a_fT );
}

// $deBlob: FUNCTION 0075d330
TUINT32 ABlobColourComponent::LerpColour( TUINT32 a_uiFrom, TUINT32 a_uiTo, TFLOAT a_fT )
{
	TMath::Clip( a_fT, 0.0f, 1.0f );

	TUINT32 uiResult = 0;

	for ( TINT i = 0; i < 4; i++ )
	{
		TINT   iFrom  = ( a_uiFrom >> ( i * 8 ) ) & 0xFF;
		TINT   iTo    = ( a_uiTo >> ( i * 8 ) ) & 0xFF;
		TUINT8 uiByte = TUINT8( TINT( TFLOAT( iTo - iFrom ) * a_fT ) + iFrom );
		uiResult |= TUINT32( uiByte ) << ( i * 8 );
	}

	return uiResult;
}

// $deBlob: FUNCTION 0075d440
TUINT32* ABlobColourComponent::GetInterpolationColours( TINT a_iIndex )
{
	return ABlobColourMaps::ms_pColourInterpolationMap + a_iIndex * 2;
}
