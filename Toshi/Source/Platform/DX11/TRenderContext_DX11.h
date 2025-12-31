#pragma once
#include "Toshi/Render/TRender.h"

TOSHI_NAMESPACE_START

class TRenderContextDX11 : public TRenderContext
{
public:
	// $deBlob: FUNCTION 006b9680
	TRenderContextDX11(TRender* pRender)
	    : TRenderContext(pRender) {}

	TMatrix44& GetProjectionMatrix() { return m_Projection; }

	void ComputePerspectiveProjection();
	void ComputePerspectiveFrustum();
	void ComputeOrthographicProjection();
	void ComputeOrthographicFrustum();

	virtual void Update() override;

private:
	TMatrix44 m_Projection;
};

TOSHI_NAMESPACE_END
