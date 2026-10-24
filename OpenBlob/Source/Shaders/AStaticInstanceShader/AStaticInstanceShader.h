#pragma once
#include <Render/TShader.h>
#include <Toshi/TSingleton.h>

class AStaticInstanceMesh;

//-----------------------------------------------------------------------------
// Purpose: the static instance shader ("si_", can't be created at runtime): an abstract shader without a platform
// implementation in de Blob PC (no class derives from it and nothing creates it).
//-----------------------------------------------------------------------------
class AStaticInstanceShader
    : public Toshi::TShader
    , public Toshi::TSingleton<AStaticInstanceShader>
{
public:
	TDECLARE_CLASS( AStaticInstanceShader, Toshi::TShader );

public:
	AStaticInstanceShader();
	virtual ~AStaticInstanceShader();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual const TCHAR* GetShaderPrefix() override; // +0x28

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual Toshi::TMaterial*    CreateMaterial( const TCHAR* a_szName ) = 0; // +0x38
	virtual AStaticInstanceMesh* CreateMesh( const TCHAR* a_szName )     = 0; // +0x3C
};
