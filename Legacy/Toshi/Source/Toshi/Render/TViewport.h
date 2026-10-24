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

	// $deBlob: FUNCTION 006b2ac0
	TFLOAT GetX() const { return m_pRenderCtx->GetX(); }

	// $deBlob: FUNCTION 006b2ad0
	TFLOAT GetY() const { return m_pRenderCtx->GetY(); }

	// $deBlob: FUNCTION 006b2ae0
	TFLOAT GetWidth() const { return m_pRenderCtx->GetWidth(); }

	// $deBlob: FUNCTION 006b2af0
	TFLOAT GetHeight() const { return m_pRenderCtx->GetHeight(); }

	// $deBlob: FUNCTION 006b2650
	void Begin()
	{
		TASSERT(m_bInViewport == TFALSE);
		m_bInViewport = TTRUE;
		BeginSKU();
	}

	// $deBlob: FUNCTION 006b26a0
	void End()
	{
		EndSKU();
		TASSERT(m_bInViewport == TTRUE);
		m_bInViewport = TFALSE;
	}

	// $deBlob: FUNCTION 006b2b50
	TBOOL AllowBackgroundClear(TBOOL allow)
	{
		TBOOL oldState          = m_bAllowBackgroundClear;
		m_bAllowBackgroundClear = allow;
		ChangeSKU(ChangeEvent_BackgroundClear);
		return oldState;
	}

	// $deBlob: FUNCTION 006b2b80
	TBOOL AllowDepthClear(TBOOL allow)
	{
		TBOOL oldState     = m_bAllowDepthClear;
		m_bAllowDepthClear = allow;
		ChangeSKU(ChangeEvent_DepthClear);
		return oldState;
	}

	void Enable(TBOOL enable) { m_bEnable = enable; }

	// $deBlob: FUNCTION 006b2bb0
	TBOOL EnableDefaultBeginRender(TBOOL enable)
	{
		TBOOL oldState        = m_bDefaultBeginRender;
		m_bDefaultBeginRender = enable;
		return oldState;
	}

	// $deBlob: FUNCTION 006b2aa0
	void SetMemoryAllocatorBlock(TMemoryHeap* pHeap)
	{
		m_MemAllocatorBlock = pHeap;
		ChangeSKU(ChangeEvent_AllocatorBlock);
	}

	// $deBlob: FUNCTION 006b26f0
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

	// $deBlob: FUNCTION 006b2780
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

	// $deBlob: FUNCTION 006b2810
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

	// $deBlob: FUNCTION 006b28a0
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

	// $deBlob: FUNCTION 006b2930
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

	// $deBlob: FUNCTION 006b29c0
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
