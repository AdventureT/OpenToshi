#include "pch.h"
#include "ABlobTrackList.h"
#include "ARandom.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AXUIState.h"
#include "Sound/AMusicManager.h"
#include "Sound/AStreamingSoundPlayer.h"
#include "XUI/ABlobMenu.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUIVisual.h>
#include <XUI/TXUIListItem.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0063b650
// $deBlob: FUNCTION 0063b660
// $deBlob: FUNCTION 0063b6d0
// $deBlob: FUNCTION 0063b740
// $deBlob: FUNCTION 0063b750
// $deBlob: FUNCTION 0053d810
// $deBlob: FUNCTION 00858980
TDEFINE_CLASS( ABlobTrackList );

// $deBlob: FUNCTION 0063b760
ABlobTrackList::ABlobTrackList()
{
	m_iRandomItem     = -1;
	m_iNumTracks      = 0;
	m_iLevel          = -1;
	m_bOpenMixer      = TFALSE;
	m_bStartLevel     = TFALSE;
	m_bPlaying        = TTRUE;
	m_bPreviewPending = TFALSE;

	for ( TINT i = 0; i < MAX_TRACKS; i++ )
		m_aszTracks[ i ] = TNULL;

	InitTracks();
	m_uiNumItems = m_iNumTracks;
}

// $deBlob: FUNCTION 0063bef0
// $deBlob: FUNCTION 0063b840
// $deBlob: FUNCTION 0063bee2
ABlobTrackList::~ABlobTrackList()
{
	if ( AStreamingSoundPlayer::GetSingleton() != TNULL )
		AStreamingSoundPlayer::GetSingleton()->ResetAll();

	if ( AMusicManager::GetSingleton() != TNULL )
		AMusicManager::GetSingleton()->PauseMusic( TFALSE, 1.0f );
}

// $deBlob: FUNCTION 0063be10
void ABlobTrackList::InitTracks()
{
	m_iNumTracks = 12;

	for ( TINT i = 0; i < m_iNumTracks; i++ )
		m_aszTracks[ i ] = AMusicManager::GetTrackName( i );

	m_iRandomItem                  = m_iNumTracks;
	m_aszTracks[ m_iNumTracks ]    = "RANDOM";
	m_iNumTracks                  += 1;

	for ( TINT i = 0; i < m_iNumTracks; i++ )
	{
		if ( m_iRandomItem == i )
			m_awszTrackNames[ i ] = T2Locale::GetSingleton()->GetString( 0x253 );
		else
			m_awszTrackNames[ i ] = AMusicManager::GetTrackDisplayName( m_aszTracks[ i ] );
	}
}

// $deBlob: FUNCTION 0063bc50
void ABlobTrackList::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_bPlaying )
	{

		// Plays the preview once the streams are ready
		if ( m_bPreviewPending && AStreamingSoundPlayer::GetSingleton()->AreStreamsReady() == TTRUE )
		{
			if ( !AStreamingSoundPlayer::GetSingleton()->IsPlaying( m_szPreviewTrack ) )
			{
				AStreamingSoundPlayer::GetSingleton()->Play( m_szPreviewTrack, 0x20 );
				m_bPreviewPending = TFALSE;
			}
		}

		AStreamingSoundPlayer::GetSingleton()->Tick( a_fDeltaTime );
	}

	ABlobCarouselList::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0063bc30
void ABlobTrackList::OnCreated()
{
	AStreamingSoundPlayer::GetSingleton()->LoadTrackSelection();

	ABlobCarouselList::OnCreated();
	SetLevel( -1 );
}

// $deBlob: FUNCTION 0063b8d0
const TWCHAR* ABlobTrackList::GetListItemText( TINT a_iItemIndex )
{
	if ( m_iNumTracks == 0 )
		InitTracks();

	return m_awszTrackNames[ a_iItemIndex ];
}

// $deBlob: FUNCTION 0063bec0
TBOOL ABlobTrackList::IsItemUnlocked( TINT a_iItemIndex )
{
	return AMusicManager::GetSingleton()->IsTrackUnlocked( m_aszTracks[ a_iItemIndex ], m_iLevel );
}

// $deBlob: FUNCTION 0063bce0
void ABlobTrackList::OpenMixer()
{
	m_bOpenMixer = TTRUE;
}

// $deBlob: FUNCTION 0063bcf0
void ABlobTrackList::SetLevel( TINT a_iLevel )
{
	m_iLevel = a_iLevel;

	TINT iTrack;

	if ( a_iLevel == 0 )
		iTrack = 0;
	else if ( a_iLevel == 7 )
		iTrack = 8;
	else if ( a_iLevel == 8 )
		iTrack = 9;
	else
		iTrack = a_iLevel + 1;

	TASSERT( iTrack >= 0 && iTrack < AMusicManager::MUSIC_NUM_SP_TRACKS );
	SetFocussedItemIndex( iTrack );

	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		TXUIListItem* pItem   = GetListItem( i );
		TXUIElement*  pLocked = pItem->GetVisual()->FindChildXUIElement( L"Locked" );
		TXUIElement*  pStatic = pItem->GetVisual()->FindChildXUIElement( L"Static" );
		TBOOL         bUnlocked = IsItemUnlocked( i );
		pLocked->SetVisible( bUnlocked == TFALSE );
		pStatic->SetVisible( bUnlocked == TFALSE );
	}
}

// $deBlob: FUNCTION 0063bde0
void ABlobTrackList::SetPreviewTrack( const TCHAR* a_szTrack )
{
	T2String8::Copy( m_szPreviewTrack, a_szTrack, -1 );
	m_bPreviewPending = TTRUE;
}

// $deBlob: FUNCTION 0063b900
void ABlobTrackList::OnItemPressed( TXUIListItem* a_pItem )
{
	TBOOL   bFound = TFALSE;
	TUINT32 uiItem = 0;

	while ( uiItem < m_uiNumItems )
	{
		if ( GetListItem( uiItem ) == a_pItem )
		{
			bFound = TTRUE;
			break;
		}

		uiItem++;
	}

	if ( !bFound )
		return;

	if ( TINT( uiItem ) == m_iRandomItem && m_iLevel != 9 )
	{
		TASSERT( m_uiNumItems > 0 );

		TBOOL abUnlocked[ MAX_TRACKS ];
		TINT  iNumUnlocked = 0;

		for ( TINT i = 0; i < m_iNumTracks - 1; i++ )
		{
			abUnlocked[ i ] = AMusicManager::GetSingleton()->IsTrackUnlocked( m_aszTracks[ i ], m_iLevel );

			if ( abUnlocked[ i ] )
				iNumUnlocked++;
		}

		TINT iRandom = ARandom::GetSingleton()->GetRandom().GetInt( 0, iNumUnlocked );

		TBOOL bFoundItem = TFALSE;

		for ( TINT i = 0; i < m_iNumTracks - 1 && !bFoundItem; i++ )
		{
			if ( abUnlocked[ i ] == TTRUE )
			{
				if ( iRandom == 0 )
				{
					bFoundItem = TTRUE;
					uiItem     = i;
				}

				iRandom--;
			}
		}

		TASSERT( bFoundItem );
	}
	else if ( !AMusicManager::GetSingleton()->IsTrackUnlocked( m_aszTracks[ uiItem ], m_iLevel ) )
	{
		PulseLocked();
		return;
	}

	if ( m_iLevel == 9 )
		AMusicManager::GetSingleton()->SetBossMusicOverride();
	else
		AMusicManager::GetSingleton()->SetMusicOverride( m_aszTracks[ uiItem ] );

	if ( m_bStartLevel )
	{
		AGameState* pState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( TTRUE == pState->IsA( &TGetClass( AXUIState ) ) );

		TSTATICCAST( AXUIState, pState )->SetNextState( AXUIState::NextState_LoadLevel );
		ABlobMenu::PlayPressSound( a_pItem );
		return;
	}

	if ( m_bOpenMixer )
	{
		TXUIScene* pMixer  = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "musicmixer.xur" ), TUINT32( 0 ) );
		TXUIScene* pParent = FindParentScene();

		if ( TXUIScene* pParentParent = pParent->FindParentScene() )
		{
			pMixer->RemoveSelf();
			pParentParent->AddChildTail( pMixer );
		}

		TXUI::GetSingleton()->TransitionTo( pParent, pMixer );
		return;
	}

	TXUI::GetSingleton()->TransitionBack( FindParentScene() );
}
