#pragma once
#include "GUI/T2GUIRenderer.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// The renderer XUI uses while it ticks/renders its context (0x30 bytes): draws with
// TPrimShader (and immediate draws for the indexed lists) through a stack of transforms.
//-----------------------------------------------------------------------------
class TXUIRenderer : public T2GUIRenderer
{
public:
	static TCONSTEXPR_VAR TINT TRANSFORMSTACKSIZE = 32;
	static TCONSTEXPR_VAR TINT MAXVERTS           = 1024;

public:
	TXUIRenderer();
	virtual ~TXUIRenderer(); // +0x00

	//-----------------------------------------------------------------------------
	// T2GUIRenderer
	//-----------------------------------------------------------------------------
	virtual void BeginScene() override;                                                                                    // +0x08
	virtual void EndScene() override;                                                                                      // +0x0C
	virtual void SetMaterial( T2GUIMaterial* a_pMaterial ) override;                                                       // +0x10
	virtual void PushTransform( const T2GUITransform& a_rTransform, const TVector2& a_rVec1, const TVector2& a_rVec2 ) override; // +0x14
	virtual void PopTransform() override;                                                                                  // +0x18
	virtual void SetTransform( const T2GUITransform& a_rTransform ) override;                                              // +0x1C
	virtual void SetColour( TUINT32 a_uiColour ) override;                                                                 // +0x20
	virtual void RenderRectangle( const TVector2& a_rA, const TVector2& a_rB, const TVector2& a_rUV1, const TVector2& a_rUV2 ) override; // +0x24
	virtual void RenderTriStrip( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) override; // +0x28
	virtual void RenderLine( const TVector2& a_rA, const TVector2& a_rB ) override;                                        // +0x2C
	virtual void RenderLine( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 ) override;                            // +0x30
	virtual void RenderOutlineRectangle( const TVector2& a_rA, const TVector2& a_rB ) override;                            // +0x34
	virtual void RenderFilledRectangle( const TVector2& a_rA, const TVector2& a_rB ) override;                             // +0x38
	virtual void RenderIndexedTriList( TVector2* a_pVertices, TVector2* a_pUVs, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) override; // +0x3C
	virtual void RenderIndexedTriStripColoursList( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32* a_pColours, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) override; // +0x40
	virtual void ScaleCoords( TFLOAT& a_rX, TFLOAT& a_rY ) override;                                                       // +0x44
	virtual void SetScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 ) override;                    // +0x48
	virtual void ClearScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 ) override;                  // +0x4C

	//-----------------------------------------------------------------------------
	// TXUIRenderer
	//-----------------------------------------------------------------------------
	virtual TUINT GetWidth( T2GUIMaterial* a_pMaterial );                                                                  // +0x50
	virtual TUINT GetHeight( T2GUIMaterial* a_pMaterial );                                                                 // +0x54
	virtual void  RenderTriStripColours( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32* a_pColours, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ); // +0x58

	TBOOL IsTransformChanged() const { return m_bTransformChanged; }

	// Sets the top transform as the model view (when it changed); AXUIBinkMovie::Render calls it unconditionally
	void UpdateTransform();

	// The D3D viewport is set to the rectangle (a_fLeft, a_fTop) - (a_fRight, a_fBottom) of the top transform (the split
	// screen HUD panels, ABlobMultiPlayerHud::Render)
	void SetViewportRect( TFLOAT a_fLeft, TFLOAT a_fTop, TFLOAT a_fRight, TFLOAT a_fBottom );

	// The D3D viewport is the whole display again
	void ResetViewportRect();

private:
	T2GUITransform* m_pTransforms;            // 0x0C transform stack
	TINT            m_iTransformStackPointer; // 0x10
	TUINT32         m_uiColour;               // 0x14
	TBOOL           m_bTransformChanged;      // 0x18 the top transform isn't the model view yet
	T2GUIMaterial*  m_pMaterial;              // 0x1C
	TUINT64         m_uiOldDepthState;        // 0x20 TRenderDX11's depth state while the scissor (stencil) is set
	TUINT8          m_aUnk28[ 8 ];            // 0x28
};

TOSHI_NAMESPACE_END
