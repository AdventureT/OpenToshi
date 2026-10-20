#pragma once
#include "XUI/ABlobMenu.h"

class ABlobCarouselList;

//-----------------------------------------------------------------------------
// Purpose: the gallery list of the extras: a carousel of the
// gallery pictures, the unlocked ones (EnSaveData::SlotData::m_uiUnlockedGalleryItems) open in gallery.xur.
//-----------------------------------------------------------------------------
class ABlobGalleries : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobGalleries, ABlobMenu );

public:
	ABlobGalleries();
	~ABlobGalleries();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;     // +0x114
	virtual void TransFrom() override;   // +0x118
	virtual void TransBackTo() override; // +0x11C

private:
	Toshi::TXUIControl* m_pLeft;     // 0x140 control 7
	Toshi::TXUIControl* m_pRight;    // 0x144 control 6
	ABlobCarouselList*  m_pCarousel; // 0x148 control 4
	Toshi::TXUIControl* m_pSelect;   // 0x14C control 5
	Toshi::TXUIControl* m_pTitle;    // 0x150 control 8
};
