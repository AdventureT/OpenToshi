#pragma once
#include "Toshi/Render/TMaterial.h"
#include "Platform/SDL/TTexture_SDL.h"

TOSHI_NAMESPACE_START

class T2GUIMaterial : public TMaterial
{
public:
	enum BLENDMODE
	{
		BLENDMODE_NORMAL,
		BLENDMODE_ADD,
		BLENDMODE_SUBSTRACT,
		BLENDMODE_MAX
	};

	T2GUIMaterial();

	void SetTexture(TTexture* pTexture) { m_pTexture = pTexture; }

	TTexture* GetTexture() { return m_pTexture; }

	void SetSamplerId(TINT iSamplerId) { m_iSamplerId = iSamplerId; }

	TINT GetSamplerId() const { return m_iSamplerId; }

private:
	TTexture* m_pTexture;   // 0x54
	TINT      m_iSamplerId; // 0x5C
};

TOSHI_NAMESPACE_END
