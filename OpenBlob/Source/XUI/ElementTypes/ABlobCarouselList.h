#pragma once
#include <XUI/TXUIList.h>

namespace Toshi
{
class TXUIScene;
}

//-----------------------------------------------------------------------------
// Purpose: a list shown as a carousel: the items sit
// on the timeline of their visual, the focussed one in the middle (half the timeline) and m_iNumSideItems on each
// side; scrolling moves them along the timeline. What the items are depends on the parent scene (galleries,
// movies, cutscenes, multiplayer levels or the music tracks of "MusicScene").
//-----------------------------------------------------------------------------
class ABlobCarouselList : public Toshi::TXUIList
{
public:
	TDECLARE_CLASS( ABlobCarouselList, Toshi::TXUIList );

	// What the items are (m_eType), set by OnCreated from the parent scene
	enum Type
	{
		Type_Movies,      // ABlobMovies
		Type_Cutscenes,   // ABlobCutscenes
		Type_Galleries,   // ABlobGalleries
		Type_MPLevels,    // ABlobMPLevelSelection
		Type_MusicTracks, // a scene named "MusicScene"
	};

public:
	ABlobCarouselList();
	~ABlobCarouselList();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0062d0a0
	virtual TXUIElement* RayTraceDeepest( const Toshi::TVector2& a_rPoint, const Toshi::TVector2& a_rScale, TINT a_iController ) override { return TNULL; } // +0x64 DOES_NOTHING
	virtual void         OnCreated() override;                                                                                                              // +0x94

	//-----------------------------------------------------------------------------
	// TXUIList
	//-----------------------------------------------------------------------------
	virtual const TWCHAR* GetListItemImage( TINT a_iItemIndex ) override;        // +0x12C
	virtual void          SetFocussedItemIndex( TINT a_iFocussedItem ) override; // +0x14C

	//-----------------------------------------------------------------------------
	// ABlobCarouselList
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0062d020
	virtual TBOOL IsItemUnlocked( TINT a_iItemIndex ) { return TTRUE; } // +0x15C

	// Scroll the items one step (the focussed item moves), return TTRUE
	TBOOL ScrollLeft();
	TBOOL ScrollRight();

	// $deBlob: FUNCTION 0062cde0
	TBOOL IsFirstItemFocussed() const { return m_uiFocussedItem == 0; }
	// $deBlob: FUNCTION 0062cdf0
	TBOOL IsLastItemFocussed() const { return m_uiFocussedItem == m_uiNumItems - 1; }

	// The text of the focussed item
	const TWCHAR* GetFocussedItemText();

	// Pulses "Locked" of the focussed item (pressed while locked)
	void PulseLocked();

	// $deBlob: FUNCTION 0062fa80
	void SetDescription( Toshi::TXUIElement* a_pDescription ) { m_pDescription = a_pDescription; }

	// $deBlob: FUNCTION 0062fa70
	TBOOL IsScrolling() const { return m_bScrolling; }

protected:
	// Re-sorts the children so the items closer to the focussed one are drawn on top
	void SortItems();

protected:
	Toshi::TXUIScene*   m_pParentScene;    // 0xD8
	TINT                m_iNumSideItems;   // 0xDC 2
	TFLOAT              m_fItemTime;       // 0xE0 the timeline length of the items' visual
	TFLOAT              m_fItemStep;       // 0xE4 the timeline distance between two items
	TFLOAT*             m_afItemTimes;     // 0xE8 the target time of each item
	Toshi::TXUIElement* m_pDescription;    // 0xEC hidden while scrolling
	TWCHAR**            m_awszItemNumbers; // 0xF0 "1", "2"...
	TINT                m_eType;           // 0xF4 Type, -1 none
	TBOOL               m_bScrolling;      // 0xF8
	TFLOAT              m_fFlip1;          // 0xFC the time of the named frame "Flip1"
	TFLOAT              m_fFlip2;          // 0x100 "Flip2"
	TBOOL               m_bSortItems;      // 0x104 sorts the items once the focussed item passes a flip frame
	TWCHAR              m_wszText[ 32 ];   // 0x106 GetFocussedItemText of Type_MusicTracks
};
