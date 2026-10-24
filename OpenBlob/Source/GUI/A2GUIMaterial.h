#pragma once
#include <Render/TMaterial.h>
#include <Render/TTexture.h>

//-----------------------------------------------------------------------------
// Purpose: the game's own GUI material: a TMaterial with one texture and a blend state, no virtuals of
// its own. Only its TClass creates it (retail uses T2GUIMaterial for the GUI).
//-----------------------------------------------------------------------------
class A2GUIMaterial : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( A2GUIMaterial, Toshi::TMaterial );

public:
	A2GUIMaterial();
	~A2GUIMaterial();

private:
	Toshi::TTexture* m_pTextureResource; // 0x54
	TUINT32          m_eBlendState;      // 0x58 1: alpha
};
