#pragma once
#include <XUI/TXUIControl.h>
#include <File/TTRB.h>
#include <GUI/T2GUIMaterial.h>

//-----------------------------------------------------------------------------
// Purpose: a control that draws a large picture made of 3x2 textures: Load( "name" ) loads
// data/matlibs/name.trb and the textures name_1.tga to
// name_6.tga (the top row, then the bottom row) and lays them out over the control's size by their sizes. Used by
// the gallery (ABlobGallery).
//-----------------------------------------------------------------------------
class ABlobBackgroundImage : public Toshi::TXUIControl
{
public:
	TDECLARE_CLASS( ABlobBackgroundImage, Toshi::TXUIControl );

	static TCONSTEXPR_VAR TINT NUM_COLUMNS = 3;
	static TCONSTEXPR_VAR TINT NUM_ROWS    = 2;
	static TCONSTEXPR_VAR TINT NUM_TILES   = NUM_COLUMNS * NUM_ROWS;

	// A texture of the picture (0x14 bytes)
	struct Tile
	{
		TFLOAT                m_fX;        // 0x00 from the control's centre
		TFLOAT                m_fY;        // 0x04
		TFLOAT                m_fWidth;    // 0x08
		TFLOAT                m_fHeight;   // 0x0C
		Toshi::T2GUIMaterial* m_pMaterial; // 0x10
	};

public:
	ABlobBackgroundImage();
	~ABlobBackgroundImage();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() override; // +0x14

	// Loads the picture (unloads the previous one first)
	void Load( const TCHAR* a_szName );

	// Destroys the materials and the TRB
	void Unload();

private:
	void LoadMatLib( const TCHAR* a_szName );

private:
	Tile        m_aTiles[ NUM_TILES ]; // 0x094
	Toshi::TTRB m_oTRB;                // 0x10C data/matlibs/<name>.trb
	TBOOL       m_bLoaded;             // 0x120
};
