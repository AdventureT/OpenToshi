#include "pch.h"
#include "ABlobGallery.h"
#include "XUI/ElementTypes/ABlobBackgroundImage.h"
#include "GameController/AGameStatus.h"

#include <XUI/TXUIFigure.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobGallery ) == 0x158 );

// The pictures of the galleries
static const ABlobGallery::Picture s_aBlobPictures[] = {
	{ "bkg_blob2", 1 },
	{ "bkg_blob1", 0 },
	{ "bkg_blob3", 0 },
	{ "bkg_blob4", 0 },
	{ "bkg_blob5", 1 },
	{ "bkg_blob6", 1 },
	{ "bkg_blob7", 1 }
};
static const ABlobGallery::Picture s_aRevPictures[] = {
	{ "bkg_rev1", 1 },
	{ "bkg_rev2", 1 },
	{ "bkg_rev3", 0 },
	{ "bkg_rev4", 1 },
	{ "bkg_rev5", 1 },
	{ "bkg_rev6", 1 },
	{ "bkg_rev7", 0 },
	{ "bkg_rev8", 1 },
	{ "bkg_rev9", 1 },
	{ "bkg_rev10", 1 },
	{ "bkg_rev11", 1 },
	{ "bkg_rev12", 1 },
	{ "bkg_rev13", 1 },
	{ "bkg_rev14", 1 },
	{ "bkg_rev15", 1 }
};
static const ABlobGallery::Picture s_aGraydiansPictures[] = {
	{ "bkg_graydians1", 1 },
	{ "bkg_graydians2", 0 },
	{ "bkg_graydians3", 0 },
	{ "bkg_graydians4", 1 },
	{ "bkg_graydians5", 0 },
	{ "bkg_graydians6", 0 }
};
static const ABlobGallery::Picture s_aRaydiansPictures[] = {
	{ "bkg_raydians1", 0 },
	{ "bkg_raydians2", 1 },
	{ "bkg_raydians3", 1 },
	{ "bkg_raydians4", 1 },
	{ "bkg_raydians5", 1 },
	{ "bkg_raydians6", 1 }
};
static const ABlobGallery::Picture s_aChromacityPictures[] = {
	{ "bkg_chromacity1", 1 },
	{ "bkg_chromacity2", 0 },
	{ "bkg_chromacity3", 0 },
	{ "bkg_chromacity4", 0 },
	{ "bkg_chromacity5", 0 },
	{ "bkg_chromacity6", 0 },
	{ "bkg_chromacity7", 0 },
	{ "bkg_chromacity8", 1 },
	{ "bkg_chromacity9", 0 },
	{ "bkg_chromacity10", 0 },
	{ "bkg_chromacity11", 0 },
	{ "bkg_chromacity12", 0 },
	{ "bkg_chromacity13", 0 },
	{ "bkg_chromacity14", 0 },
	{ "bkg_chromacity15", 0 },
	{ "bkg_chromacity16", 1 },
	{ "bkg_chromacity17", 0 },
	{ "bkg_chromacity18", 0 },
	{ "bkg_chromacity19", 0 },
	{ "bkg_chromacity20", 0 }
};
static const ABlobGallery::Picture s_aLandmarksPictures[] = {
	{ "bkg_landmarks1", 1 },
	{ "bkg_landmarks2", 0 },
	{ "bkg_landmarks3", 1 },
	{ "bkg_landmarks4", 1 },
	{ "bkg_landmarks5", 1 },
	{ "bkg_landmarks6", 0 },
	{ "bkg_landmarks7", 1 }
};
static const ABlobGallery::Picture s_aInkiesPictures[] = {
	{ "bkg_inkies1", 1 },
	{ "bkg_inkies2", 1 },
	{ "bkg_inkies3", 1 },
	{ "bkg_inkies4", 1 },
	{ "bkg_inkies5", 0 },
	{ "bkg_inkies6", 1 },
	{ "bkg_inkies7", 1 },
	{ "bkg_inkies8", 1 },
	{ "bkg_inkies9", 0 }
};
static const ABlobGallery::Picture s_aStylePictures[] = {
	{ "bkg_style1", 0 },
	{ "bkg_style2", 0 },
	{ "bkg_style3", 0 },
	{ "bkg_style4", 0 },
	{ "bkg_style5", 1 },
	{ "bkg_style6", 0 },
	{ "bkg_style7", 0 },
	{ "bkg_style8", 1 },
	{ "bkg_style9", 0 },
	{ "bkg_style10", 0 },
	{ "bkg_style11", 0 },
	{ "bkg_style12", 0 }
};
static const ABlobGallery::Picture s_aBillboardPictures[] = {
	{ "bkg_bill_lastresort", 1 },
	{ "bkg_bill_bridge", 1 },
	{ "bkg_bill_slums", 1 },
	{ "bkg_bill_dam", 1 },
	{ "bkg_bill_guggen", 1 },
	{ "bkg_bill_docks", 1 },
	{ "bkg_bill_abyss", 1 },
	{ "bkg_bill_harvest", 1 },
	{ "bkg_bill_military", 1 },
	{ "bkg_bill_raydia", 1 }
};
static const ABlobGallery::Picture s_aVehiclesPictures[] = {
	{ "bkg_vehicles1", 0 },
	{ "bkg_vehicles2", 0 },
	{ "bkg_vehicles3", 0 },
	{ "bkg_vehicles4", 0 },
	{ "bkg_vehicles5", 0 },
	{ "bkg_vehicles6", 0 }
};

static const ABlobGallery::Picture* s_apPictures[ ABlobGallery::NUM_GALLERIES ] = {
	s_aBlobPictures, s_aRevPictures, s_aGraydiansPictures, s_aRaydiansPictures, s_aChromacityPictures,
	s_aLandmarksPictures, s_aInkiesPictures, s_aStylePictures, s_aBillboardPictures, s_aVehiclesPictures
};

// The number of pictures of each gallery
static const TINT s_aiNumPictures[ ABlobGallery::NUM_GALLERIES ] = { 7, 15, 6, 6, 20, 7, 9, 12, 10, 6 };

// $deBlob: FUNCTION 00631900
// $deBlob: FUNCTION 00631910
// $deBlob: FUNCTION 00631980
// $deBlob: FUNCTION 006319f0
// $deBlob: FUNCTION 00631a00
// $deBlob: FUNCTION 0053d4d0
// $deBlob: FUNCTION 008588b0
TDEFINE_CLASS( ABlobGallery );

// $deBlob: FUNCTION 00631a10
ABlobGallery::ABlobGallery()
    : ABlobMenu( -1 )
{
	m_iPicture  = 0;
	m_pPrevious = TNULL;
	m_pNext     = TNULL;
	m_iGallery  = AGameStatus::GetSingleton()->GetSelectedGallery();
	m_pBackdrop = TNULL;
}

// $deBlob: FUNCTION 00631a70
// $deBlob: FUNCTION 00631e00
// $deBlob: FUNCTION 00631e10
ABlobGallery::~ABlobGallery()
{
}

// $deBlob: FUNCTION 00631a90
void ABlobGallery::OnCreated()
{
	ABlobMenu::OnCreated();

	m_pPrevious = FindChildXUIControl( 7 );
	m_pPrevious->SetVisible( TFALSE );
	m_pNext     = FindChildXUIControl( 6 );
	m_pImage    = TSTATICCAST( ABlobBackgroundImage, FindChildXUIControl( 2 ) );
	m_pBackdrop = TSTATICCAST( TXUIFigure, FindChildXUIElement( 1 ) );

	PlayAnimation( L"169", L"End169" );
	ShowPicture();
}

// $deBlob: FUNCTION 00631b00
TBOOL ABlobGallery::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	if ( a_pSource == m_pPrevious )
		ShowPreviousPicture();
	else if ( a_pSource == m_pNext )
		ShowNextPicture();

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 00631b80
TBOOL ABlobGallery::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Left )
	{
		ShowPreviousPicture();
		return TTRUE;
	}

	if ( a_eButton == XUIButton_Right )
	{
		ShowNextPicture();
		return TTRUE;
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 00631bf0
void ABlobGallery::ShowNextPicture()
{
	if ( m_iPicture < s_aiNumPictures[ m_iGallery ] - 1 )
	{
		m_iPicture++;
		ShowPicture();

		if ( m_iPicture == s_aiNumPictures[ m_iGallery ] - 1 )
			m_pNext->SetVisible( TFALSE );

		m_pPrevious->SetVisible( TTRUE );
	}
}

// $deBlob: FUNCTION 00631c50
void ABlobGallery::ShowPreviousPicture()
{
	if ( 0 < m_iPicture )
	{
		m_iPicture--;
		ShowPicture();

		if ( m_iPicture == 0 )
			m_pPrevious->SetVisible( TFALSE );

		m_pNext->SetVisible( TTRUE );
	}
}

// $deBlob: FUNCTION 00631c90
void ABlobGallery::ShowPicture()
{
	if ( m_iPicture == -1 )
		return;

	// NOTE: retail switches on AGameStatus's selected gallery (not m_iGallery) and leaves the picture uninitialised
	// for a value above 9
	const Picture& rPicture = s_apPictures[ AGameStatus::GetSingleton()->GetSelectedGallery() ][ m_iPicture ];

	TUINT32 uiColour;

	if ( rPicture.m_iWhite == 0 )
	{
		static TUINT32 s_uiBlack = 0xFF000000;
		uiColour                 = s_uiBlack;
	}
	else
	{
		static TUINT32 s_uiWhite = 0xFFFFFFFF;
		uiColour                 = s_uiWhite;
	}

	m_pBackdrop->SetFillColour( uiColour );
	m_pImage->Load( rPicture.m_szName );
}
