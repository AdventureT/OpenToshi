#pragma once
#include "XUI/ElementTypes/ABlobCarouselList.h"

//-----------------------------------------------------------------------------
// Purpose: the music tracks of the level selection's "MusicScene": a carousel of AMusicManager's tracks plus
// "RANDOM"; the focussed track is previewed, pressing one sets
// the music override and starts the level (or opens the music mixer).
//-----------------------------------------------------------------------------
class ABlobTrackList : public ABlobCarouselList
{
public:
	TDECLARE_CLASS( ABlobTrackList, ABlobCarouselList );

	static TCONSTEXPR_VAR TINT MAX_TRACKS = 27;

public:
	ABlobTrackList();
	~ABlobTrackList();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

	// $deBlob: FUNCTION 0063bc20
	virtual TBOOL OnNotifySetFocus( TXUIElement* a_pSource ) override { return TFALSE; } // +0xB4

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual const TWCHAR* GetListItemText( TINT a_iItemIndex ) override;          // +0x124
	virtual void          OnItemPressed( Toshi::TXUIListItem* a_pItem ) override; // +0x158

	//-----------------------------------------------------------------------------
	// ABlobCarouselList
	//-----------------------------------------------------------------------------
	virtual TBOOL IsItemUnlocked( TINT a_iItemIndex ) override; // +0x15C

	// The level whose tracks are unlocked, focusses its track (-1: none)
	void SetLevel( TINT a_iLevel );

	// Pressing a track opens the music mixer
	void OpenMixer();

	// The track previewed once the stream player is ready
	void SetPreviewTrack( const TCHAR* a_szTrack );

	// $deBlob: FUNCTION 0063beb0
	void CancelPreviewTrack() { m_bPreviewPending = TFALSE; }

	const TCHAR* GetTrack( TINT a_iItemIndex ) const { return m_aszTracks[ a_iItemIndex ]; }

	void SetStartLevel( TBOOL a_bStartLevel ) { m_bStartLevel = a_bStartLevel; }
	void SetPlaying( TBOOL a_bPlaying ) { m_bPlaying = a_bPlaying; }

private:
	void InitTracks();

private:
	const TWCHAR* m_awszTrackNames[ MAX_TRACKS ]; // 0x148
	const TCHAR*  m_aszTracks[ MAX_TRACKS ];      // 0x1B4
	TCHAR         m_szPreviewTrack[ 128 ];        // 0x220
	TINT          m_iRandomItem;                  // 0x2A0 the item "RANDOM"
	TINT          m_iNumTracks;                   // 0x2A4
	TINT          m_iLevel;                       // 0x2A8 -1 none, 9: the boss music
	TBOOL         m_bOpenMixer;                   // 0x2AC pressing a track opens musicmixer.xur
	TBOOL         m_bStartLevel;                  // 0x2AD pressing a track starts the level (ABlobLevelSelection)
	TBOOL         m_bPlaying;                     // 0x2AE previews the tracks
	TBOOL         m_bPreviewPending;              // 0x2AF
};
