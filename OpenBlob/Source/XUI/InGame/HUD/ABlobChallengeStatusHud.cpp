#include "pch.h"
#include "ABlobChallengeStatusHud.h"
#include "Buildings/ALandmarkInstance.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/AChallenge.h"
#include "Challenges/ALandmarkChallenge.h"
#include "Challenges/APaintChallenge.h"
#include "Challenges/AUberChallenge.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"

#include <Toshi/TStringManager.h>
#include <XUI/TXUIControl.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING


const TINT ABlobChallengeStatusHud::s_aiColours[ iNUMCOLOURS ] = { -1, 1, 2, 4, 6, 5, 3, 7 };

static const TWCHAR* s_awszPaintAnims[ ABlobChallengeStatusHud::MAX_COLOURS ] = { L"Paint1", L"Paint2", L"Paint3", L"Paint4", L"Paint5", L"Paint6" };

// The colour of a_iColour without its alpha (opaque)
static TUINT32 GetOpaqueColour( TUINT32 a_uiColour, TUINT32& a_ruiR, TUINT32& a_ruiG, TUINT32& a_ruiB, TUINT32& a_ruiA )
{
	a_ruiB = a_uiColour & 0xFF;
	a_ruiG = ( a_uiColour >> 8 ) & 0xFF;
	a_ruiR = ( a_uiColour >> 16 ) & 0xFF;
	a_ruiA = ( a_uiColour >> 24 ) & 0xFF;

	return a_ruiB | ( ( a_ruiG | ( ( a_ruiR | 0xFFFFFF00 ) << 8 ) ) << 8 );
}

// The current challenge (the current sub challenge of an uber challenge)
static AChallenge* GetCurrentChallenge()
{
	AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

	if ( pChallenge != TNULL && pChallenge->GetType() == AChallenge::Type_Uber )
		pChallenge = TSTATICCAST( AUberChallenge, pChallenge )->GetCurrentSubChallenge();

	return pChallenge;
}

// $deBlob: FUNCTION 0064ee00
// $deBlob: FUNCTION 0064ee10
// $deBlob: FUNCTION 0064ee80
// $deBlob: FUNCTION 0064eef0
// $deBlob: FUNCTION 0064ef00
// $deBlob: FUNCTION 0053e090
// $deBlob: FUNCTION 00858ba0
TDEFINE_CLASS( ABlobChallengeStatusHud );

// $deBlob: FUNCTION 0064ef10
ABlobChallengeStatusHud::ABlobChallengeStatusHud()
    : m_pType( TNULL )
    , m_pPaints( TNULL )
    , m_pGenericCounter( TNULL )
    , m_iNumColours( 0 )
{
	for ( TINT i = 0; i < NUM_DROPLETS; i++ )
	{
		m_apDroplets[ i ]       = TNULL;
		m_apDropletNumbers[ i ] = TNULL;
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves the counts uninitialised (OnChallengeStart sets them)
	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
	{
		m_aiColourCount[ i ]        = -1;
		m_awszColourCount[ i ][ 0 ] = L'\0';
	}
#endif
}

// $deBlob: FUNCTION 00650565
// $deBlob: FUNCTION 00650570
ABlobChallengeStatusHud::~ABlobChallengeStatusHud()
{
}

// $deBlob: FUNCTION 0064f100
void ABlobChallengeStatusHud::OnCreated()
{
	m_pType           = FindChildXUIControl( 0x9D );
	m_pPaints         = FindChildXUIElement( 0x46 );
	m_pGenericCounter = m_pPaints->FindChildXUIControl( L"GenericCounter" );

	TWCHAR* wszName = TStringManager::GetTempString16();
	TINT    iFirst  = 0;

	for ( TINT iGroup = 1; iGroup <= MAX_COLOURS; iGroup++ )
	{
		TStringManager::String16Format( wszName, L"ChallengePaint%d", iGroup );
		TXUIElement* pGroup = m_pPaints->FindChildXUIElement( wszName );

		for ( TINT i = 1; i <= iGroup; i++ )
		{
			TStringManager::String16Format( wszName, L"Paint%d", i );
			m_apDroplets[ iFirst + i - 1 ] = TSTATICCAST( TXUIFigure, pGroup->FindChildXUIElement( wszName ) );

			TStringManager::String16Format( wszName, L"PaintNumber%d", i );
			m_apDropletNumbers[ iFirst + i - 1 ] = pGroup->FindChildXUIControl( wszName );
		}

		iFirst += iGroup;
	}
}

// $deBlob: FUNCTION 0064f3b0
void ABlobChallengeStatusHud::UpdateColourCount( const void* a_pTarget, TBOOL a_bUncomplete )
{
	const APaintChallenge::Target* pTarget = TSTATICCAST( const APaintChallenge::Target, a_pTarget );

	// The index of the colour and the one of its droplet (the colours in the challenge before it)
	TINT iIndex       = 0;
	TINT iColourIndex = 0;

	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
	{
		if ( pTarget->m_iColour == s_aiColours[ i ] )
		{
			iIndex = i;
			break;
		}

		if ( m_aiColourCount[ i ] > 0 )
			iColourIndex++;
	}

	TASSERT( iIndex >= 0 && iIndex < iNUMCOLOURS );

	if ( m_aiColourCount[ iIndex ] == 0 )
	{
		TASSERT( TTRUE == a_bUncomplete );
		m_aiColourCount[ iIndex ]++;
	}
	else
	{
		m_aiColourCount[ iIndex ] += a_bUncomplete ? 1 : -1;
		TASSERT( iIndex >= 0 && iIndex < iNUMCOLOURS );

		TWCHAR* wszCount = m_awszColourCount[ iIndex ];
		TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ iIndex ] );

		TXUIControl* pNumber = TNULL;

		switch ( m_iNumColours )
		{
			case 1:
				TASSERT( iColourIndex == 0 );
				pNumber = GetDropletNumber( 1, 0 );
				break;
			case 2:
				TASSERT( iColourIndex >= 0 && iColourIndex < 2 );
				pNumber = GetDropletNumber( 2, iColourIndex );
				break;
			case 3:
				TASSERT( iColourIndex >= 0 && iColourIndex < 3 );
				pNumber = GetDropletNumber( 3, iColourIndex );
				break;
			case 4:
				TASSERT( iColourIndex >= 0 && iColourIndex < 4 );
				pNumber = GetDropletNumber( 4, iColourIndex );
				break;
			case 5:
				TASSERT( iColourIndex >= 0 && iColourIndex < 5 );
				pNumber = GetDropletNumber( 5, iColourIndex );
				break;
			case 6:
				TASSERT( iColourIndex >= 0 && iColourIndex < 6 );
				pNumber = GetDropletNumber( 6, iColourIndex );
				break;
		}

		if ( pNumber != TNULL )
			pNumber->SetText( wszCount, -1 );
	}

	TINT iNumColours = 0;

	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
	{
		if ( m_aiColourCount[ i ] > 0 )
			iNumColours++;
	}

	if ( iNumColours > 0 && iNumColours != m_iNumColours )
	{
		m_iNumColours = iNumColours;
		UpdateDroplets();
	}
}

// $deBlob: FUNCTION 0064fae0
void ABlobChallengeStatusHud::SetupPaintChallenge( AChallenge* a_pChallenge )
{
	const APaintChallenge::TypeData* pData = TSTATICCAST( const APaintChallenge::TypeData, a_pChallenge->GetData() );

	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
		m_aiColourCount[ i ] = -1;

	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
	{
		for ( TUINT32 j = 0; j < pData->m_uiNumTargets; j++ )
		{
			if ( pData->m_pTargets[ j ].m_iColour == s_aiColours[ i ] )
			{
				if ( m_aiColourCount[ i ] == -1 )
					m_aiColourCount[ i ] = 0;

				m_aiColourCount[ i ]++;
			}
		}
	}

	// The targets already painted
	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
	{
		for ( TUINT32 j = 0; j < pData->m_uiNumTargets; j++ )
		{
			if ( pData->m_pTargets[ j ].m_iDone != 0 && pData->m_pTargets[ j ].m_iColour == s_aiColours[ i ] )
				m_aiColourCount[ i ]--;
		}
	}

	m_iNumColours = 0;

	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
	{
		if ( m_aiColourCount[ i ] > 0 )
			m_iNumColours++;
	}

	UpdateDroplets();
}

// $deBlob: FUNCTION 0064fc50
void ABlobChallengeStatusHud::SetupLandmarkChallenge( AChallenge* a_pChallenge )
{
	const ALandmarkChallenge::TypeData* pData = TSTATICCAST( const ALandmarkChallenge::TypeData, a_pChallenge->GetData() );

	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
		m_aiColourCount[ i ] = -1;

	m_iNumColours = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] );

		for ( TINT j = 0; j < pLandmark->GetNumTriggers(); j++ )
		{
			if ( pLandmark->GetTriggerPaintLeft( j ) > 0 )
				m_iNumColours++;
		}
	}

	if ( m_iNumColours == 0 )
		return;

	TASSERT( m_iNumColours > 0 && m_iNumColours < 7 );

	if ( m_iNumColours >= 1 && m_iNumColours <= MAX_COLOURS )
		m_pPaints->PlayAnimation( s_awszPaintAnims[ m_iNumColours - 1 ], s_awszPaintAnims[ m_iNumColours - 1 ] );

	static TUINT32 s_uiR, s_uiG, s_uiB, s_uiA;

	TINT iIndex = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		for ( TINT j = 0; j < TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] )->GetNumTriggers(); j++ )
		{
			ALandmarkInstance* pLandmark  = TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] );
			TINT               iPaintLeft = pLandmark->GetTriggerPaintLeft( j );

			if ( iPaintLeft <= 0 )
				continue;

			m_aiColourCount[ iIndex ] = iPaintLeft;
			TASSERT( iIndex >= 0 && iIndex < iNUMCOLOURS );

			TWCHAR* wszCount = m_awszColourCount[ iIndex ];
			TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ iIndex ] );

			TUINT32 uiColour = GetOpaqueColour( ABlobColourComponent::ms_auiColours[ TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] )->GetTriggerColour( j ) ], s_uiR, s_uiG, s_uiB, s_uiA );

			if ( m_iNumColours >= 1 && m_iNumColours <= MAX_COLOURS )
			{
				GetDroplet( m_iNumColours, iIndex )->SetFillColour( uiColour );
				GetDropletNumber( m_iNumColours, iIndex )->SetText( wszCount, -1 );
			}

			iIndex++;
		}
	}
}

// $deBlob: FUNCTION 0064f720
void ABlobChallengeStatusHud::SetTriggerPaintLeft( ALandmarkInstance* a_pLandmark, TINT a_iTrigger, TINT a_iPaintLeft )
{
	AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

	if ( pChallenge != TNULL && pChallenge->GetType() == AChallenge::Type_Uber )
		pChallenge = TSTATICCAST( AUberChallenge, pChallenge )->GetCurrentSubChallenge();

	if ( pChallenge->GetType() != AChallenge::Type_Landmark )
		return;

	const ALandmarkChallenge::TypeData* pData = TSTATICCAST( const ALandmarkChallenge::TypeData, pChallenge->GetData() );

	if ( pData->m_uiNumLandmarks == 0 )
		return;

	TUINT32 uiLandmark = 0;

	while ( pData->m_ppLandmarks[ uiLandmark ] != a_pLandmark )
	{
		if ( ++uiLandmark >= pData->m_uiNumLandmarks )
			return;
	}

	// The droplet of the trigger: the triggers with paint left before it
	TINT iTriggerIndex = 0;

	for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
	{
		ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pData->m_ppLandmarks[ i ] );
		TBOOL              bFound    = TFALSE;

		for ( TINT j = 0; j < pLandmark->GetNumTriggers(); j++ )
		{
			if ( pLandmark == a_pLandmark && j == a_iTrigger )
			{
				bFound = TTRUE;
				break;
			}

			if ( pLandmark->GetTriggerPaintLeft( j ) > 0 )
				iTriggerIndex++;
		}

		if ( bFound )
			break;
	}

	TASSERT( iTriggerIndex >= 0 && iTriggerIndex < iNUMCOLOURS );
	TASSERT( m_aiColourCount[ iTriggerIndex ] != -1 );

	m_aiColourCount[ iTriggerIndex ] = a_iPaintLeft;
	TStringManager::String16Format( m_awszColourCount[ iTriggerIndex ], L"%d", a_iPaintLeft );

	if ( a_iPaintLeft <= 0 )
		SetupLandmarkChallenge( pChallenge );
}

// $deBlob: FUNCTION 006500b0
void ABlobChallengeStatusHud::UpdateDroplets()
{
	TASSERT( m_iNumColours > 0 && m_iNumColours < 7 );

	if ( m_iNumColours >= 1 && m_iNumColours <= MAX_COLOURS )
		m_pPaints->PlayAnimation( s_awszPaintAnims[ m_iNumColours - 1 ], s_awszPaintAnims[ m_iNumColours - 1 ] );

	static TUINT32 s_uiR, s_uiG, s_uiB, s_uiA;

	TINT iColourIndex = 0;

	for ( TINT i = 0; i < iNUMCOLOURS; i++ )
	{
		TINT iCount = m_aiColourCount[ i ];

		if ( iCount == 0 || iCount == -1 )
			continue;

		TUINT32 uiColour = ( s_aiColours[ i ] == -1 ) ? ABlobColourComponent::ms_auiColours[ 0 ] : ABlobColourComponent::ms_auiColours[ s_aiColours[ i ] ];
		uiColour         = GetOpaqueColour( uiColour, s_uiR, s_uiG, s_uiB, s_uiA );

		TASSERT( iColourIndex >= 0 && iColourIndex < 6 );

		TWCHAR* wszCount = m_awszColourCount[ i ];

		switch ( m_iNumColours )
		{
			case 1:
				GetDroplet( 1, 0 )->SetFillColour( uiColour );
				TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ i ] );
				GetDropletNumber( 1, 0 )->SetText( wszCount, -1 );
				// NOTE: retail doesn't move to the next droplet here (there's only one)
				break;
			case 2:
				TASSERT( iColourIndex >= 0 && iColourIndex < 2 );
				GetDroplet( 2, iColourIndex )->SetFillColour( uiColour );
				TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ i ] );
				GetDropletNumber( 2, iColourIndex )->SetText( wszCount, -1 );
				iColourIndex++;
				break;
			case 3:
				TASSERT( iColourIndex >= 0 && iColourIndex < 3 );
				GetDroplet( 3, iColourIndex )->SetFillColour( uiColour );
				TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ i ] );
				GetDropletNumber( 3, iColourIndex )->SetText( wszCount, -1 );
				iColourIndex++;
				break;
			case 4:
				TASSERT( iColourIndex >= 0 && iColourIndex < 4 );
				GetDroplet( 4, iColourIndex )->SetFillColour( uiColour );
				TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ i ] );
				GetDropletNumber( 4, iColourIndex )->SetText( wszCount, -1 );
				iColourIndex++;
				break;
			case 5:
				TASSERT( iColourIndex >= 0 && iColourIndex < 5 );
				GetDroplet( 5, iColourIndex )->SetFillColour( uiColour );
				TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ i ] );
				GetDropletNumber( 5, iColourIndex )->SetText( wszCount, -1 );
				iColourIndex++;
				break;
			case 6:
				TASSERT( iColourIndex >= 0 && iColourIndex < 6 );
				GetDroplet( 6, iColourIndex )->SetFillColour( uiColour );
				TStringManager::String16Format( wszCount, L"%d", m_aiColourCount[ i ] );
				GetDropletNumber( 6, iColourIndex )->SetText( wszCount, -1 );
				iColourIndex++;
				break;
		}
	}
}

// $deBlob: FUNCTION 0064f8f0
void ABlobChallengeStatusHud::OnChallengeStart( AChallenge* a_pChallenge )
{
	TXUIElement* pTypeVisual = m_pType->GetVisual();

	switch ( a_pChallenge->GetType() )
	{
		case AChallenge::Type_Paint:
			pTypeVisual->PlayAnimation( L"Paint", L"Paint" );
			SetupPaintChallenge( a_pChallenge );
			break;
		case AChallenge::Type_Landmark:
			pTypeVisual->PlayAnimation( L"Landmark", L"Landmark" );
			SetupLandmarkChallenge( a_pChallenge );
			break;
		case AChallenge::Type_Civilian:
		{
			m_pPaints->PlayAnimation( L"Paint1", L"Paint1" );
			pTypeVisual->PlayAnimation( L"Paint", L"Paint" );

			static TUINT32 s_uiR, s_uiG, s_uiB, s_uiA;
			GetDroplet( 1, 0 )->SetFillColour( GetOpaqueColour( ABlobColourComponent::ms_auiColours[ 0 ], s_uiR, s_uiG, s_uiB, s_uiA ) );
			break;
		}
		case AChallenge::Type_Combat:
			m_pPaints->PlayAnimation( L"Combat", L"Combat" );
			pTypeVisual->PlayAnimation( L"Combat", L"Combat" );
			break;
		case AChallenge::Type_Race:
			m_pPaints->PlayAnimation( L"Race", L"Race" );
			pTypeVisual->PlayAnimation( L"Race", L"Race" );
			break;
		case AChallenge::Type_Car:
			pTypeVisual->PlayAnimation( L"Paint", L"Paint" );
			break;
	}
}

// $deBlob: FUNCTION 0064fa80
void ABlobChallengeStatusHud::SetCounterText( const TWCHAR* a_wszText )
{
	AChallenge* pChallenge = GetCurrentChallenge();

	if ( pChallenge == TNULL )
		return;

	if ( pChallenge->GetType() == AChallenge::Type_Civilian )
		GetDropletNumber( 1, 0 )->SetText( a_wszText, -1 );
	else
		m_pGenericCounter->SetText( a_wszText, -1 );
}
