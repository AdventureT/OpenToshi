#include "pch.h"
#include "ABlobCarouselList.h"
#include "GameInterface/APlayerProfileManager.h"
#include "XUI/FrontEnd/ABlobGalleries.h"
#include "XUI/FrontEnd/ABlobMovies.h"
#include "XUI/FrontEnd/ABlobCutscenes.h"
#include "XUI/FrontEnd/Multiplayer/ABlobMPLevelSelection.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"

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

// The images of the music moods
static TCONSTEXPR_VAR TINT NUMMOODS = 13;

static const TWCHAR* s_awszMoodImages[ NUMMOODS ] = {
	L"../Images/Mood5.tga",
	L"../Images/Mood1.tga",
	L"../Images/Mood2.tga",
	L"../Images/Mood10.tga",
	L"../Images/Mood11.tga",
	L"../Images/Mood7.tga",
	L"../Images/Mood4.tga",
	L"../Images/Mood3.tga",
	L"../Images/Mood6.tga",
	L"../Images/Mood8.tga",
	L"../Images/Mood12.tga",
	L"../Images/Mood9.tga",
	L"../Images/Mood13.tga",
};

// The numbers of items of the extras
static TCONSTEXPR_VAR TINT GalleryUnlock_NUMOF    = 11;
static TCONSTEXPR_VAR TINT MovieUnlock_NUMOF      = 10;
static TCONSTEXPR_VAR TINT CutsceneUnlock_NUMOF   = 12;
static TCONSTEXPR_VAR TINT MultiplayerLevel_NUMOF = 9;

// The images of the multiplayer levels
static const TWCHAR* s_awszMPLevelImages[ MultiplayerLevel_NUMOF ] = {
	L"../Images/MP_level1.tga",
	L"../Images/MP_level2.tga",
	L"../Images/MP_level3.tga",
	L"../Images/MP_level4.tga",
	L"../Images/MP_level5.tga",
	L"../Images/MP_level6.tga",
	L"../Images/MP_level7.tga",
	L"../Images/MP_level8.tga",
	L"../Images/MP_level9.tga",
};

// The images of the gallery pictures
static const TWCHAR* s_awszGalleryImages[ GalleryUnlock_NUMOF ] = {
	L"../Images/Gallery1.tga",
	L"../Images/Gallery2.tga",
	L"../Images/Gallery3.tga",
	L"../Images/Gallery4.tga",
	L"../Images/Gallery5.tga",
	L"../Images/Gallery6.tga",
	L"../Images/Gallery7.tga",
	L"../Images/Gallery8.tga",
	L"../Images/Gallery9.tga",
	L"../Images/Gallery10.tga",
	TNULL,
};

// The images of the cutscenes
static const TWCHAR* s_awszCutsceneImages[ CutsceneUnlock_NUMOF ] = {
	L"../Images/Cutscene1.tga",
	L"../Images/Cutscene2.tga",
	L"../Images/Cutscene3.tga",
	L"../Images/Cutscene4.tga",
	L"../Images/Cutscene5.tga",
	L"../Images/Cutscene6.tga",
	L"../Images/Cutscene7.tga",
	L"../Images/Cutscene8.tga",
	L"../Images/Cutscene9.tga",
	L"../Images/Cutscene10.tga",
	L"../Images/Cutscene11.tga",
	L"../Images/Cutscene12.tga",
};

// The images of the movies
static const TWCHAR* s_awszMovieImages[ MovieUnlock_NUMOF ] = {
	L"../Images/Movie1.tga",
	L"../Images/Movie2.tga",
	L"../Images/Movie3.tga",
	L"../Images/Movie4.tga",
	L"../Images/Movie5.tga",
	L"../Images/Movie6.tga",
	L"../Images/Movie7.tga",
	L"../Images/Movie8.tga",
	L"../Images/Movie9.tga",
	L"../Images/Movie10.tga",
};

// The names of the gallery pictures
static const TINT s_aiGalleryNames[ GalleryUnlock_NUMOF ] = { 784, 791, 788, 787, 792, 794, 785, 798, 793, 789, 13 };

// The names of the worlds (items 1 to 10 of Type_Cutscenes)
static const TINT s_aiWorldNames[ 11 ] = { 0, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133 };

// $deBlob: FUNCTION 0062bd20
// $deBlob: FUNCTION 0062bd30
// $deBlob: FUNCTION 0062bda0
// $deBlob: FUNCTION 0062be10
// $deBlob: FUNCTION 0062be20
// $deBlob: FUNCTION 0053d1d0
// $deBlob: FUNCTION 008587f0
TDEFINE_CLASS( ABlobCarouselList );

// $deBlob: FUNCTION 0062ceb0
ABlobCarouselList::ABlobCarouselList()
{
	m_pParentScene    = TNULL;
	m_iNumSideItems   = 2;
	m_fItemTime       = 0.0f;
	m_fItemStep       = 0.0f;
	m_afItemTimes     = TNULL;
	m_pDescription    = TNULL;
	m_awszItemNumbers = TNULL;
	m_eType           = -1;
	m_bScrolling      = TFALSE;
	m_fFlip1          = 0.0f;
	m_fFlip2          = 0.0f;
	m_bSortItems      = TFALSE;
}

// $deBlob: FUNCTION 0062cf50
// $deBlob: FUNCTION 0062be30
// $deBlob: FUNCTION 0062cf3b
ABlobCarouselList::~ABlobCarouselList()
{
	TFree( m_afItemTimes );

	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
		TFree( m_awszItemNumbers[ i ] );

	TFree( m_awszItemNumbers );
}

// $deBlob: FUNCTION 0062bea0
void ABlobCarouselList::OnCreated()
{
	m_pParentScene = FindParentScene();

	// The number of items and the type come from the parent menu
	if ( m_pParentScene->IsA( &TGetClass( ABlobGalleries ) ) )
	{
		m_uiNumItems = 10;
		m_eType      = Type_Galleries;
	}
	else if ( m_pParentScene->IsA( &TGetClass( ABlobMovies ) ) )
	{
		m_uiNumItems = 10;
		m_eType      = Type_Movies;
	}
	else if ( m_pParentScene->IsA( &TGetClass( ABlobCutscenes ) ) )
	{
		m_uiNumItems = 12;
		m_eType      = Type_Cutscenes;
	}
	else if ( m_pParentScene->IsA( &TGetClass( ABlobMPLevelSelection ) ) )
	{
		m_uiNumItems = 9;
		m_eType      = Type_MPLevels;
	}
	else if ( TStringManager::String16Compare( m_pParentScene->GetID(), L"MusicScene" ) == 0 )
	{
		m_eType = Type_MusicTracks;
		TASSERT( m_uiNumItems > 0 );
	}

	m_awszItemNumbers = TSTATICCAST( TWCHAR*, TMalloc( m_uiNumItems * sizeof( TWCHAR* ), TXUI::ms_pXUIMemoryBlock ) );

	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
		m_awszItemNumbers[ i ] = TSTATICCAST( TWCHAR, TMalloc( 16, TXUI::ms_pXUIMemoryBlock ) );

	m_afItemTimes   = TSTATICCAST( TFLOAT, TMalloc( m_uiNumItems * sizeof( TFLOAT ), TXUI::ms_pXUIMemoryBlock ) );
	m_iNumSideItems = 2;

	TXUIList::OnCreated();
	SetEnabled( TFALSE );

	if ( m_uiNumItems == 0 )
		return;

	EnSaveData::SlotData* pData = TNULL;

	m_fItemTime  = GetListItem( 0 )->GetVisual()->GetTimelineControl()->GetTimelineLength();
	TFLOAT fTime = m_fItemTime * 0.5f;
	m_fItemStep  = fTime / TFLOAT( m_iNumSideItems + 1 );

	const TFLOAT fHalfTime = fTime;

	if ( m_eType != Type_MusicTracks )
		pData = APlayerProfileManager::GetSingleton()->GetCurrentProfile()->GetData();

	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		TStringManager::String16Format( m_awszItemNumbers[ i ], L"%d", i + 1 );

		TXUIListItem* pItem = GetListItem( i );

		if ( m_eType != Type_MusicTracks )
			pItem->SetText( m_awszItemNumbers[ i ], -1 );

		AddChildHead( pItem );
		pItem->SetVisible( TTRUE );
		pItem->SetPosition( 0, 0 );
		pItem->GetVisual()->GetTimelineControl()->StopAnimation();
		pItem->GetVisual()->GetTimelineControl()->SetTimeLineTime( fTime, TTRUE );
		m_afItemTimes[ i ] = fTime;

		fTime -= m_fItemStep;

		if ( fTime < 0.0f )
			fTime = 0.0f;

		TXUIElement* pLocked = pItem->GetVisual()->FindChildXUIElement( L"Locked" );
		TXUIElement* pStatic = pItem->GetVisual()->FindChildXUIElement( L"Static" );
		TUINT32      uiBit   = 1 << i;

		switch ( m_eType )
		{
			case Type_Movies:
				pLocked->SetVisible( ( pData->m_iUnlockedVideos & uiBit ) == 0 );
				pStatic->SetVisible( ( pData->m_iUnlockedVideos & uiBit ) == 0 );
				break;
			case Type_Cutscenes:
				pLocked->SetVisible( ( pData->m_iUnlockedMovies & uiBit ) == 0 );
				pStatic->SetVisible( ( pData->m_iUnlockedMovies & uiBit ) == 0 );
				break;
			case Type_Galleries:
				pLocked->SetVisible( ( pData->m_uiUnlockedGalleryItems & uiBit ) == 0 );
				pStatic->SetVisible( ( pData->m_uiUnlockedGalleryItems & uiBit ) == 0 );
				break;
			case Type_MPLevels:
				pLocked->SetVisible( ( pData->m_iUnlockedBlobParty & uiBit ) == 0 );
				pStatic->SetVisible( ( pData->m_iUnlockedBlobParty & uiBit ) == 0 );
				break;
			case Type_MusicTracks:
				pLocked->SetVisible( IsItemUnlocked( i ) == TFALSE );
				pStatic->SetVisible( IsItemUnlocked( i ) == TFALSE );
				break;
		}

		if ( i == 0 )
		{
			pItem->GetVisual()->FindChildXUIElement( L"XuiMPFrame" )->SetVisible( TFALSE );
			m_fFlip1 = pItem->GetVisual()->GetTimelineControl()->GetNamedFrameTime( L"Flip1" );
			m_fFlip2 = pItem->GetVisual()->GetTimelineControl()->GetNamedFrameTime( L"Flip2" );
		}
	}

	for ( TINT i = 0; i < m_iNumSideItems; i++ )
	{
		TINT iItem = ( m_uiFocussedItem - i ) - 1;

		if ( iItem < 0 )
			iItem += m_uiNumItems;

		TXUIListItem* pItem     = GetListItem( iItem );
		TFLOAT        fItemTime = TFLOAT( i + 1 ) * m_fItemStep + fHalfTime;
		m_afItemTimes[ iItem ]  = fItemTime;
		pItem->GetVisual()->GetTimelineControl()->SetTimeLineTime( fItemTime, TTRUE );
	}

	SortItems();
}

// $deBlob: FUNCTION 0062c430
const TWCHAR* ABlobCarouselList::GetListItemImage( TINT a_iItemIndex )
{
	if ( m_pParentScene->IsA( &TGetClass( ABlobGalleries ) ) )
	{
		// NOTE: retail's assert lets the index 10 through (GalleryUnlock_NUMOF is 11), its image is TNULL
		TASSERT( a_iItemIndex >= 0 && a_iItemIndex < GalleryUnlock_NUMOF );
		return s_awszGalleryImages[ a_iItemIndex ];
	}

	if ( m_pParentScene->IsA( &TGetClass( ABlobMovies ) ) )
	{
		TASSERT( a_iItemIndex >= 0 && a_iItemIndex < MovieUnlock_NUMOF );
		return s_awszMovieImages[ a_iItemIndex ];
	}

	if ( m_pParentScene->IsA( &TGetClass( ABlobCutscenes ) ) )
	{
		TASSERT( a_iItemIndex >= 0 && a_iItemIndex < CutsceneUnlock_NUMOF );
		return s_awszCutsceneImages[ a_iItemIndex ];
	}

	if ( m_pParentScene->IsA( &TGetClass( ABlobMPLevelSelection ) ) )
	{
		TASSERT( a_iItemIndex >= 0 && a_iItemIndex < MultiplayerLevel_NUMOF );
		return s_awszMPLevelImages[ a_iItemIndex ];
	}


	if ( TStringManager::String16Compare( m_pParentScene->GetID(), L"MusicScene" ) == 0 )
	{
		TASSERT( a_iItemIndex >= 0 && a_iItemIndex < NUMMOODS );
		return s_awszMoodImages[ a_iItemIndex ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 0062c630
void ABlobCarouselList::Tick( TFLOAT a_fDeltaTime )
{
	for ( TUINT32 i = 0; i < m_uiNumItems; i++ )
	{
		TXUIListItem* pItem   = GetListItem( i );
		TFLOAT        fTime   = pItem->GetVisual()->GetTimelineControl()->GetTimeLineTime();
		TFLOAT        fTarget = m_afItemTimes[ i ];

		if ( fTime < fTarget )
		{
			fTime += a_fDeltaTime;

			if ( i == m_uiFocussedItem && m_bSortItems && m_fFlip1 < fTime )
			{
				SortItems();
				m_bSortItems = TFALSE;
			}

			if ( m_afItemTimes[ i ] < fTime )
				fTime = m_afItemTimes[ i ];

			if ( i == m_uiFocussedItem && fTime >= m_afItemTimes[ i ] )
			{
				m_bScrolling = TFALSE;
				pItem->GetVisual()->FindChildXUIElement( L"XuiMPFrame" )->SetVisible( TFALSE );
				m_pDescription->SetVisible( TTRUE );
			}
		}
		else if ( fTarget < fTime )
		{
			fTime -= a_fDeltaTime;

			if ( i == m_uiFocussedItem && m_bSortItems && fTime < m_fFlip2 )
			{
				SortItems();
				m_bSortItems = TFALSE;
			}

			if ( fTime < m_afItemTimes[ i ] )
				fTime = m_afItemTimes[ i ];

			if ( i == m_uiFocussedItem && m_afItemTimes[ i ] >= fTime )
			{
				m_bScrolling = TFALSE;
				pItem->GetVisual()->FindChildXUIElement( L"XuiMPFrame" )->SetVisible( TFALSE );
				m_pDescription->SetVisible( TTRUE );
			}
		}

		pItem->GetVisual()->GetTimelineControl()->SetTimeLineTime( fTime, TTRUE );
	}

	T2GUIElement::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 0062c7b0
TBOOL ABlobCarouselList::ScrollRight()
{
	if ( !m_bScrolling )
	{
		m_bScrolling = TTRUE;

		TUINT32 uiItem = m_uiFocussedItem - m_iNumSideItems;
		TINT    iCount = m_iNumSideItems * 2 + 2;

		if ( TINT( uiItem ) < 0 )
			uiItem += m_uiNumItems;

		for ( ; iCount > 0; iCount-- )
		{
			m_afItemTimes[ uiItem ] += m_fItemStep;

			if ( TMath::Abs( m_fItemTime - m_afItemTimes[ uiItem ] ) < 0.001f )
				m_afItemTimes[ uiItem ] = m_fItemTime;

			if ( m_fItemTime < m_afItemTimes[ uiItem ] )
			{
				m_afItemTimes[ uiItem ] -= m_fItemTime;

				TFLOAT fTime = m_afItemTimes[ uiItem ] - m_fItemStep;

				if ( fTime < 0.0f )
					fTime = 0.0f;

				GetListItem( uiItem )->GetVisual()->GetTimelineControl()->SetTimeLineTime( fTime, TTRUE );
			}

			uiItem = ( uiItem + 1 < m_uiNumItems ) ? uiItem + 1 : 0;
		}

		GetListItem( m_uiFocussedItem )->GetVisual()->FindChildXUIElement( L"XuiMPFrame" )->SetVisible( TTRUE );
		m_pDescription->SetVisible( TFALSE );

		m_uiFocussedItem++;

		if ( m_uiNumItems <= m_uiFocussedItem )
			m_uiFocussedItem = 0;

		m_bSortItems = TTRUE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0062c920
TBOOL ABlobCarouselList::ScrollLeft()
{
	if ( !m_bScrolling )
	{
		m_bScrolling = TTRUE;

		TUINT32 uiItem = ( m_uiFocussedItem - m_iNumSideItems ) - 1;
		TINT    iCount = m_iNumSideItems * 2 + 2;

		if ( TINT( uiItem ) < 0 )
			uiItem += m_uiNumItems;

		for ( ; iCount > 0; iCount-- )
		{
			m_afItemTimes[ uiItem ] -= m_fItemStep;

			if ( TMath::Abs( m_afItemTimes[ uiItem ] ) < 0.001f )
				m_afItemTimes[ uiItem ] = 0.0f;

			if ( m_afItemTimes[ uiItem ] < 0.0f )
			{
				m_afItemTimes[ uiItem ] = m_fItemTime + m_afItemTimes[ uiItem ];

				TFLOAT fTime = m_afItemTimes[ uiItem ] + m_fItemStep;

				if ( m_fItemTime < fTime )
					fTime = m_fItemTime;

				GetListItem( uiItem )->GetVisual()->GetTimelineControl()->SetTimeLineTime( fTime, TTRUE );
			}

			uiItem = ( uiItem + 1 < m_uiNumItems ) ? uiItem + 1 : 0;
		}

		GetListItem( m_uiFocussedItem )->GetVisual()->FindChildXUIElement( L"XuiMPFrame" )->SetVisible( TTRUE );
		m_pDescription->SetVisible( TFALSE );

		if ( TINT( --m_uiFocussedItem ) < 0 )
			m_uiFocussedItem = m_uiNumItems - 1;

		m_bSortItems = TTRUE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0062ca90
void ABlobCarouselList::PulseLocked()
{
	if ( ASoundLoadHelper::GetSingleton()->IsFrontEndLoaded() )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID_FRONT_END, ASoundMap::EVENTID( 0x40 ), FMOD_EVENT_DEFAULT, TTRUE );

	GetListItem( m_uiFocussedItem )->GetVisual()->FindChildXUIElement( L"Locked" )->PlayAnimation( L"Pulse", L"EndPulse" );
}

// $deBlob: FUNCTION 0062cae0
const TWCHAR* ABlobCarouselList::GetFocussedItemText()
{

	switch ( m_eType )
	{
		case Type_Cutscenes:
			break;
		case Type_MusicTracks:
			TStringManager::String16Format( m_wszText, 32, L"%ls", GetListItem( GetFocussedItemIndex() )->GetText() );
			return m_wszText;
		case Type_Movies:
			return T2Locale::GetSingleton()->GetString( ABlobMovies::sm_aMovies[ GetFocussedItemIndex() ].m_iTitleId );
		case Type_Galleries:
			return T2Locale::GetSingleton()->GetString( s_aiGalleryNames[ GetFocussedItemIndex() ] );
		default:
			return TNULL;
	}

	TINT iItem = GetFocussedItemIndex();

	if ( iItem == 0 )
		return T2Locale::GetSingleton()->GetString( 0x335 );

	if ( iItem == 11 )
		return T2Locale::GetSingleton()->GetString( 0x336 );

	return T2Locale::GetSingleton()->GetString( s_aiWorldNames[ iItem ] );
}

// $deBlob: FUNCTION 0062cbd0
void ABlobCarouselList::SetFocussedItemIndex( TINT a_iItemIndex )
{
	if ( TINT( m_uiFocussedItem ) != a_iItemIndex )
		GetListItem( m_uiFocussedItem )->GetVisual()->FindChildXUIElement( L"XuiMPFrame" )->SetVisible( TTRUE );

	TASSERT( a_iItemIndex >= 0 && TUINT32( a_iItemIndex ) < m_uiNumItems );

	TFLOAT fHalfTime = m_fItemTime * 0.5f;
	m_uiFocussedItem = a_iItemIndex;

	TFLOAT fTime = fHalfTime;

	for ( TINT i = a_iItemIndex; i >= 0; i-- )
	{
		TXUIListItem* pItem = GetListItem( i );
		pItem->GetVisual()->GetTimelineControl()->StopAnimation();
		pItem->GetVisual()->GetTimelineControl()->SetTimeLineTime( fTime, TTRUE );
		m_afItemTimes[ i ] = fTime;

		fTime += m_fItemStep;

		if ( m_fItemTime < fTime )
			fTime = m_fItemTime;
	}

	fTime = fHalfTime - m_fItemStep;

	for ( TUINT32 i = m_uiFocussedItem + 1; i < m_uiNumItems; i++ )
	{
		TXUIListItem* pItem = GetListItem( i );
		pItem->GetVisual()->GetTimelineControl()->StopAnimation();
		pItem->GetVisual()->GetTimelineControl()->SetTimeLineTime( fTime, TTRUE );
		m_afItemTimes[ i ] = fTime;

		fTime -= m_fItemStep;

		if ( fTime < 0.0f )
			fTime = 0.0f;
	}

	for ( TINT i = 0; i < m_iNumSideItems; i++ )
	{
		TINT iItem = ( m_uiFocussedItem - i ) - 1;

		if ( iItem < 0 )
			iItem += m_uiNumItems;

		TXUIListItem* pItem     = GetListItem( iItem );
		TFLOAT        fItemTime = TFLOAT( i + 1 ) * m_fItemStep + fHalfTime;
		m_afItemTimes[ iItem ]  = fItemTime;
		pItem->GetVisual()->GetTimelineControl()->SetTimeLineTime( fItemTime, TTRUE );
	}

	SortItems();

	GetListItem( m_uiFocussedItem )->GetVisual()->FindChildXUIElement( L"XuiMPFrame" )->SetVisible( TFALSE );
	m_bScrolling = TFALSE;
}

// $deBlob: FUNCTION 0062ce10
void ABlobCarouselList::SortItems()
{
	for ( TINT i = m_iNumSideItems + 1; i > 0; i-- )
	{
		TINT iItem = m_uiFocussedItem - i;

		if ( iItem < 0 )
			iItem += m_uiNumItems;

		AddChildTail( GetListItem( iItem ) );
	}

	for ( TINT i = 1; i <= m_iNumSideItems + 1; i++ )
	{
		TUINT32 uiItem = m_uiFocussedItem + i;

		if ( m_uiNumItems <= uiItem )
			uiItem -= m_uiNumItems;

		AddChildHead( GetListItem( uiItem ) );
	}

	AddChildTail( GetListItem( m_uiFocussedItem ) );
}
