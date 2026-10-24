#pragma once
#include <Render/TMaterial.h>

//-----------------------------------------------------------------------------
// Purpose: a material of the sprite shader: the line and the fill materials.
//-----------------------------------------------------------------------------
class ASpriteMaterial
    : public Toshi::TMaterial
{
	friend class ASpriteShaderHAL; // sets the owner shader
	friend class ASpriteShader;    // destroys the line and fill materials

public:
	TDECLARE_CLASS( ASpriteMaterial, Toshi::TMaterial );

	ASpriteMaterial();
	virtual ~ASpriteMaterial();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TINT SetBlendMode( TINT a_eBlendMode );         // +0x1C
	virtual TINT SetPrimitiveType( TINT a_ePrimitiveType ); // +0x20

protected:
	TINT m_ePrimitiveType; // 0x54
	TINT m_eBlendMode;     // 0x58
};                         // 0x5C
