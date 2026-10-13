#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TMaterial.h"

TOSHI_NAMESPACE_START

class TSysMaterial : public TGenericClassDerived<TSysMaterial, TMaterial, "TSysMaterial", TMAKEVERSION(1, 0), TFALSE>
{
public:
	enum class BlendMode
	{
		Default,
		Mode1,
		Mode2,
		Mode3,
		Mode4,
		Mode5,
		Mode6,
		Mode7,
	};

public:
	// $deBlob: FUNCTION 0070aa30
	TSysMaterial() { m_eBlendMode = BlendMode::Default; }

	virtual TBOOL Create(BlendMode eBlendMode);
	virtual void  SetBlendMode(BlendMode eBlendMode);

protected:
	BlendMode m_eBlendMode;
};

TOSHI_NAMESPACE_END
