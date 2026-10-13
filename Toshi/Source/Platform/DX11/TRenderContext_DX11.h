#pragma once
#include "Render/TRenderContext.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// de Blob: Source\Render\TRenderContext_DX11.cpp, class TRenderContextDX11 (vftable ??_7TRenderContextDX11@Toshi@@6B@,
// size 0x404). Ported from the old repo (Legacy/Toshi/Source/Platform/DX11/TRenderContext_DX11.*) onto
// OpenBarnyard's TRenderContext.
//-----------------------------------------------------------------------------
class TRenderContextDX11 : public TRenderContext
{
public:
	TRenderContextDX11( TRender* a_pRender );
	~TRenderContextDX11();

	virtual void Update() OVERRIDE;

	void ComputePerspectiveProjection();
	void ComputePerspectiveFrustum();
	void ComputeOrthographicProjection();
	void ComputeOrthographicFrustum();

	TMatrix44& GetProjectionMatrix() { return m_oProjection; }

private:
	TMatrix44 m_oProjection; // 0x3C4
};

TOSHI_NAMESPACE_END
