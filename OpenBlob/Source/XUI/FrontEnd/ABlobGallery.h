#pragma once
#include "XUI/ABlobMenu.h"

class ABlobBackgroundImage;

TOSHI_NAMESPACE_START
class TXUIFigure;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a gallery of concept pictures: shows the pictures of the gallery AGameStatus selected
// (ABlobGalleries) in an ABlobBackgroundImage, previous/next with the buttons or the shoulder buttons (4/5); the
// backdrop figure is white or black depending on the picture.
//-----------------------------------------------------------------------------
class ABlobGallery : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobGallery, ABlobMenu );

	static TCONSTEXPR_VAR TINT NUM_GALLERIES = 10;

	// A picture (8 bytes)
	struct Picture
	{
		const TCHAR* m_szName; // 0x0 the matlib and the textures (ABlobBackgroundImage::Load)
		TINT         m_iWhite; // 0x4 the backdrop is white, else black
	};

public:
	ABlobGallery();
	~ABlobGallery();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIControl
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override; // +0xF8

private:
	void ShowNextPicture();
	void ShowPreviousPicture();
	void ShowPicture();

private:
	ABlobBackgroundImage* m_pImage;    // 0x140 control 2
	TINT                  m_iPicture;  // 0x144
	Toshi::TXUIControl*   m_pPrevious; // 0x148 control 7
	Toshi::TXUIControl*   m_pNext;     // 0x14C control 6
	TINT                  m_iGallery;  // 0x150 AGameStatus's selected gallery
	Toshi::TXUIFigure*    m_pBackdrop; // 0x154 element 1
};
