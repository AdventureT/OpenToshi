#pragma once
#include <GUI/T2GUIElement.h>

namespace Toshi
{
class TTRB;
class T2GUIPolygon;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: a full screen background of six tiles: loads
// "data/matlibs/<name>.trb" and shows its textures "<name>_1.tga" to "<name>_6.tga" in a 3x2 grid, each tile sized
// by its texture's share of the total size.
//-----------------------------------------------------------------------------
class A2GUIBackground : public Toshi::T2GUIElement
{
public:
	static TCONSTEXPR_VAR TINT MAX_FILENAME_LENGTH = 128;
	static TCONSTEXPR_VAR TINT MAX_TILES           = 6;

	// The offset of the tiles
	static TFLOAT s_fOffsetX;
	static TFLOAT s_fOffsetY;

public:
	A2GUIBackground();
	~A2GUIBackground();

	// NOTE: retail has a second argument that it doesn't use (every caller passes 0)
	void Create( const TCHAR* a_szFileName, TINT a_iUnused = 0 );
	void Destroy();
	// $deBlob: FUNCTION 005d5f80
	Toshi::TTRB* GetTRB() const { return m_pTRB; }

private:
	void LoadMaterialLibrary( const TCHAR* a_szFileName );
	void SetupBackground();

private:
	Toshi::TTRB*         m_pTRB;                              // 0x3C
	Toshi::T2GUIPolygon* m_apPoly[ MAX_TILES ];               // 0x40
	TCHAR                m_szFileName[ MAX_FILENAME_LENGTH ]; // 0x58
};
