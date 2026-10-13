#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TShader.h"
#include "TSysMaterial.h"
#include "TSysMesh.h"

TOSHI_NAMESPACE_START

class TSysShader
    : public TGenericClassDerived<TSysShader, TShader, "TSysShader", TMAKEVERSION(1, 0), TFALSE>
    , public TSingleton<TSysShader>
{
public:
	virtual const TCHAR* GetShaderPrefix() override;

	virtual TSysMaterial* CreateMaterial()              = 0;
	virtual TSysMesh*     CreateMesh(const TCHAR* name) = 0;
	virtual TBOOL         IsLightScattering() const;
	virtual TBOOL         SetLightScattering(TBOOL scattering) const;
};

TOSHI_NAMESPACE_END
