#pragma once
#include "Includes.h"

TOSHI_NAMESPACE_START

class TToneMap
{
public:
	TToneMap();
	~TToneMap();

private:
	ID3D11ShaderResourceView* m_pTextureView;
	ID3D11RenderTargetView*   m_pRenderTarget;
	TUINT32                   m_Unk1;
	TUINT32                   m_Unk2;
	TUINT32                   m_Unk3;
	TUINT32                   m_Unk4;
	TUINT32                   m_Unk5;
};

TOSHI_NAMESPACE_END
