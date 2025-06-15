#pragma once
#include "Toshi/Render/TRender.h"

TOSHI_NAMESPACE_START

class TViewport
{
public:
	using ChangeEvent = TUINT32;
	enum ChangeEvent_ : ChangeEvent
	{
		ChangeEvent_BackgroundColor,
		ChangeEvent_MaxZ,
		ChangeEvent_MinZ,
		ChangeEvent_Height,
		ChangeEvent_Width,
		ChangeEvent_X,
		ChangeEvent_Y,
		ChangeEvent_AllocatorBlock,
		ChangeEvent_BackgroundClear,
		ChangeEvent_DepthClear,
	};

public:
	TViewport();
	~TViewport();

	TFLOAT GetX() const { return m_pRenderCtx->GetX(); }

	TFLOAT GetY() const { return m_pRenderCtx->GetY(); }

	TFLOAT GetWidth() const { return m_pRenderCtx->GetWidth(); }

	TFLOAT GetHeight() const { return m_pRenderCtx->GetHeight(); }

	void Begin()
	{
		TASSERT(m_bInViewport == TFALSE);
		m_bInViewport = TTRUE;
		BeginSKU();
	}

	void End()
	{
		EndSKU();
		TASSERT(m_bInViewport == TTRUE);
		m_bInViewport = TFALSE;
	}

	TBOOL AllowBackgroundClear(TBOOL allow)
	{
		TBOOL oldState          = m_bAllowBackgroundClear;
		m_bAllowBackgroundClear = allow;
		ChangeSKU(ChangeEvent_BackgroundClear);
		return oldState;
	}

	TBOOL AllowDepthClear(TBOOL allow)
	{
		TBOOL oldState     = m_bAllowDepthClear;
		m_bAllowDepthClear = allow;
		ChangeSKU(ChangeEvent_DepthClear);
		return oldState;
	}

	void Enable(TBOOL enable) { m_bEnable = enable; }

	TBOOL EnableDefaultBeginRender(TBOOL enable)
	{
		TBOOL oldState        = m_bDefaultBeginRender;
		m_bDefaultBeginRender = enable;
		return oldState;
	}

	void SetMemoryAllocatorBlock(TMemoryHeap* pHeap)
	{
		m_MemAllocatorBlock = pHeap;
		ChangeSKU(ChangeEvent_AllocatorBlock);
	}

	void SetX(TFLOAT value)
	{
		TRenderContext::Params params = m_pRenderCtx->GetParams();

		if (params.fX != value)
		{
			params.fX = value;
			m_pRenderCtx->SetParams(params);
			ChangeSKU(ChangeEvent_X);
		}
	}

	void SetY(TFLOAT value)
	{
		TRenderContext::Params params = m_pRenderCtx->GetParams();

		if (params.fY != value)
		{
			params.fY = value;
			m_pRenderCtx->SetParams(params);
			ChangeSKU(ChangeEvent_Y);
		}
	}

	void SetWidth(TFLOAT value)
	{
		TRenderContext::Params params = m_pRenderCtx->GetParams();

		if (params.fWidth != value)
		{
			params.fWidth = value;
			m_pRenderCtx->SetParams(params);
			ChangeSKU(ChangeEvent_Width);
		}
	}

	void SetHeight(TFLOAT value)
	{
		TRenderContext::Params params = m_pRenderCtx->GetParams();

		if (params.fHeight != value)
		{
			params.fHeight = value;
			m_pRenderCtx->SetParams(params);
			ChangeSKU(ChangeEvent_Height);
		}
	}

	void SetMinZ(TFLOAT value)
	{
		TRenderContext::Params params = m_pRenderCtx->GetParams();

		if (params.fMinZ != value)
		{
			params.fMinZ = value;
			m_pRenderCtx->SetParams(params);
			ChangeSKU(ChangeEvent_MinZ);
		}
	}

	void SetMaxZ(TFLOAT value)
	{
		TRenderContext::Params params = m_pRenderCtx->GetParams();

		if (params.fMaxZ != value)
		{
			params.fMaxZ = value;
			m_pRenderCtx->SetParams(params);
			ChangeSKU(ChangeEvent_MaxZ);
		}
	}

	TRenderContext* GetRenderContext() const { return m_pRenderCtx; }

	void SetBackgroundColor(TUINT8 r, TUINT8 g, TUINT8 b, TUINT8 a);
	void GetBackgroundColor(TUINT8& r, TUINT8& g, TUINT8& b, TUINT8& a);

	void ChangeSKU(ChangeEvent a_eEvent);
	void BeginSKU();
	void EndSKU();

private:
	TMemoryHeap*    m_MemAllocatorBlock;     // 0x00
	TRenderContext* m_pRenderCtx;            // 0x04
	TUINT8          m_ColorR;                // 0x08
	TUINT8          m_ColorG;                // 0x09
	TUINT8          m_ColorB;                // 0x0A
	TUINT8          m_ColorA;                // 0x0B
	TBOOL           m_bAllowBackgroundClear; // 0x0C
	TBOOL           m_bAllowDepthClear;      // 0x0D
	TBOOL           m_bDefaultBeginRender;   // 0x0E
	TBOOL           m_bEnable;               // 0x0F
	TBOOL           m_bInViewport;           // 0x10
};

TOSHI_NAMESPACE_END
