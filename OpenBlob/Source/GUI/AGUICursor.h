#pragma once
#include <XUI/TXUIScene.h>
#include <Input/TInputInterface.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: the mouse cursor of the menus: a small XUI scene with the
// "Cursor" visual of the skin, moved to TXUIInput's mouse position and rendered by AGUIManager after XUI.
//-----------------------------------------------------------------------------
class AGUICursor
{
public:
	AGUICursor( Toshi::TInputDevice* a_pDevice, TUINT32 a_uiUnused, TINT a_iPlayer );
	~AGUICursor();

	// Recreates the cursor scene for a_iPlayer
	void Reinitialize( TINT a_iPlayer );

	void Tick( TFLOAT a_fDeltaTime );
	void Render();

	// NOTE: a_bUnused isn't used
	void SetVisible( TBOOL a_bVisible, TBOOL a_bUnused );

	// Enables TXUIInput (the cursor's input)
	void SetActive( TBOOL a_bActive );

	void GetPosition( Toshi::TVector2& a_rPosition );

	// The colour of the cursor in multiplayer (the player's)
	void SetMPColour( TUINT32 a_uiColour );
	void SetColour( TUINT32 a_uiColour );

	// Shows the Blob Vision rings of the cursor
	void SetBlobVision( TBOOL a_bBlobVision );

	// $deBlob: FUNCTION 005d6e80
	TBOOL IsVisible() const { return m_bVisible; }

	// $deBlob: FUNCTION 00629e30
	TUINT32 GetColour() const { return m_uiColour; }

	// $deBlob: FUNCTION 00629e40
	TBOOL IsMPColour() const { return m_bMPColour; }

	// The Blob Party engagement menu clears it without touching the colour
	// $deBlob: FUNCTION 0063ecf0
	void SetMPColourFlag( TBOOL a_bMPColour ) { m_bMPColour = a_bMPColour; }

	// $deBlob: FUNCTION 00629e50
	TBOOL IsSceneVisible() const { return m_pScene->IsVisible(); }

private:
	void CreateXUIScene( TINT a_iPlayer );

private:
	Toshi::TXUIScene*    m_pScene;            // 0x00
	TUINT32              m_aUnk04[ 4 ];       // 0x04
	Toshi::TInputDevice* m_pDevice;           // 0x14
	TBOOL                m_bVisible;          // 0x18
	TFLOAT               m_fOffsetX;          // 0x1C the hot spot
	TFLOAT               m_fOffsetY;          // 0x20
	Toshi::TXUIElement*  m_pPlayerIndex;      // 0x24 "PlayerIndex", shown with 2+ players
	Toshi::TXUIElement*  m_pCircleGroup;      // 0x28 "CircleGroup"
	Toshi::TXUIElement*  m_pCircleImage;      // 0x2C "XuiImage1" of the circle group
	Toshi::TXUIElement*  m_pBlobVisionGroup;  // 0x30 "BlobVisionGroup"
	Toshi::TXUIElement*  m_pBlobVisionGroup1; // 0x34 "BlobVisionGroup1"
	TUINT32              m_uiColour;          // 0x38
	TINT                 m_iPlayer;           // 0x3C
	TBOOL                m_bMPColour;         // 0x40
};

TSTATICASSERT( sizeof( AGUICursor ) == 0x44 );
