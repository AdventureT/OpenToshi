#pragma once
#include <XUI/TXUIScene.h>
#include <XUI/TXUIGroup.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIText.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Purpose: the paint meter of a player (ABlobHud's element
// 0xAA): the blob figure filled with the paint colour, the paint points (m_wszSize) moving the "BlobPaintMeter"
// timeline, the rainbow blob while painting a landmark and the landmark's progress bar ("LandmarkShake"). The blob
// keeps it in ABlobEntity+0xB1C (SetPlayer).
//-----------------------------------------------------------------------------
class ABlobSizeHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobSizeHud, Toshi::TXUIScene );

	static TCONSTEXPR_VAR TINT NUM_COLOURED = 4;

public:
	ABlobSizeHud();
	~ABlobSizeHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	// Attaches the HUD to a player's blob
	void SetPlayer( TUINT8 a_uiPlayer );

	// Shows the blob's paint points (0-100 moves the meter)
	void SetSize( TINT a_iSize );

	// Fills the blob with a_uiColour (ARGB); a_bLandmark: the rainbow blob with the landmark's paint left
	void SetColour( TUINT32 a_uiColour, TBOOL a_bLandmark, TFLOAT a_fPaintLeft );

	// Back to the normal blob (respawn/reset)
	void Reset();

	// The landmark's progress bar fills with a_uiColour (ARGB) to a_iPaintLeft of a_iTotal
	void GrowLandmarkMeter( TUINT32 a_uiColour, TINT a_iPaintLeft, TINT a_iTotal );

	// Plays "ExitLandmark" once the HUD is the active scene again
	// $deBlob: FUNCTION 0065aad0
	void SetExitLandmarkPending() { m_bExitLandmarkPending = TTRUE; }

	// Plays "ExitLandmark" now
	void ExitLandmark();

	// The landmark's progress bar shows a_iPaintLeft of a_iTotal
	void SetLandmarkProgress( TINT a_iPaintLeft, TINT a_iTotal );

	// The elements also filled with the paint colour (SetColour, set by ABlobHud)
	void SetColoured( TINT a_iIndex, Toshi::TXUIElement* a_pElement ) { m_apColoured[ a_iIndex ] = a_pElement; }

private:
	TWCHAR              m_wszSize[ 8 ];               // 0x130
	TWCHAR              m_wszPaintLeft[ 8 ];          // 0x140
	TINT                m_iSize;                      // 0x150
	TUINT32             m_uiColour;                   // 0x154
	Toshi::TXUIFigure*  m_pBlob;                      // 0x158 "Blob"
	Toshi::TXUIFigure*  m_pTop;                       // 0x15C "top" of "Blob1"
	Toshi::TXUIFigure*  m_pBody;                      // 0x160 "body" of "Blob1"
	Toshi::TXUIElement* m_pRainbowBlob;               // 0x164 "RainbowBlob"
	Toshi::TXUIElement* m_pUnk168;                    // 0x168 element 0xB9 (single player)
	Toshi::TXUIText*    m_pPaintLeftText;             // 0x16C element 0xBD (single player)
	TUINT8              m_uiPlayer;                   // 0x170
	TINT                m_iPaintLeft;                 // 0x174
	Toshi::TXUIElement* m_apColoured[ NUM_COLOURED ]; // 0x178
	Toshi::TXUIControl* m_pLandmarkShake;             // 0x188 "LandmarkShake" (single player)
	Toshi::TXUIGroup*   m_pProgressBody;              // 0x18C "ProgressBody" of m_pLandmarkShake
	Toshi::TXUIFigure*  m_pPaint;                     // 0x190 "Paint" of m_pProgressBody
	TFLOAT              m_fProgressEmpty;             // 0x194
	TFLOAT              m_fProgressFull;              // 0x198
	TBOOL               m_bExitLandmarkPending;       // 0x19C
	Toshi::TXUIGroup*   m_pPaintMeter;                // 0x1A0 "BlobPaintMeter"
	Toshi::TXUIGroup*   m_pBlob1;                     // 0x1A4 "Blob1" of m_pPaintMeter
	TFLOAT              m_fMeterEmpty;                // 0x1A8
	TFLOAT              m_fMeterFull;                 // 0x1AC
};
