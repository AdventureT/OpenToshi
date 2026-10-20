#pragma once
#include "Render/TRender.h"
#include "Render/TRenderContext.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The setters return the previous value.
//-----------------------------------------------------------------------------
class TOSHI_API TViewport
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

	TFLOAT SetX( TFLOAT a_fValue );
	TFLOAT SetY( TFLOAT a_fValue );
	TFLOAT SetWidth( TFLOAT a_fValue );
	TFLOAT SetHeight( TFLOAT a_fValue );
	TFLOAT SetMinZ( TFLOAT a_fValue );
	TFLOAT SetMaxZ( TFLOAT a_fValue );

	void SetBackgroundColor( TUINT8 r, TUINT8 g, TUINT8 b, TUINT8 a );
	void SetMemoryAllocatorBlock( TMemoryHeap* a_pMemBlock );

	TFLOAT GetX() const;
	TFLOAT GetY() const;
	TFLOAT GetWidth() const;
	TFLOAT GetHeight() const;
	TFLOAT GetMinZ() const;
	TFLOAT GetMaxZ() const;

	void GetBackgroundColor( TUINT8& r, TUINT8& g, TUINT8& b, TUINT8& a );

	TBOOL AllowBackgroundClear( TBOOL a_bAllow );
	TBOOL AllowDepthClear( TBOOL a_bAllow );
	TBOOL EnableDefaultBeginRender( TBOOL a_bEnable );

	void Begin();
	void End();

	void Enable( TBOOL a_bEnable ) { m_bEnable = a_bEnable; }

	TRenderContext* GetRenderContext() const { return m_pRenderCtx; }
	// $deBlob: FUNCTION 006bb330
	TBOOL IsBackgroundClearAllowed() const { return m_bAllowBackgroundClear; }
	// $deBlob: FUNCTION 006bb340
	TBOOL IsDepthClearAllowed() const { return m_bAllowDepthClear; }

	// Platform part (TViewport_DX11.cpp)
	void CreateSKU();
	void DestroySKU();
	void ChangeSKU( ChangeEvent a_eEvent );
	void BeginSKU();
	void EndSKU();

protected:
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
}; // 0x14

TOSHI_NAMESPACE_END
