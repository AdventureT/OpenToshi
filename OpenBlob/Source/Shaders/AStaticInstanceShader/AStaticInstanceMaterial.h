#pragma once
#include <Render/TMaterial.h>

//-----------------------------------------------------------------------------
// Purpose: the material of AStaticInstanceShader (can't be created at runtime): a TMaterial with one texture.
//-----------------------------------------------------------------------------
class AStaticInstanceMaterial : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( AStaticInstanceMaterial, Toshi::TMaterial );

public:
	AStaticInstanceMaterial();
	~AStaticInstanceMaterial();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual TBOOL Create() override; // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Create( Toshi::TTexture* a_pTexture );     // +0x1C
	virtual void SetTexture( Toshi::TTexture* a_pTexture ); // +0x20

private:
	Toshi::TTexture* m_pTexture; // 0x54
};
