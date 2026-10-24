#pragma once
#include "Render/TRenderContext.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The render context of TRenderDX11.
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
