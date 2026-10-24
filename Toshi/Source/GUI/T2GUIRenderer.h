#pragma once
#include "T2GUIMaterial.h"
#include "T2GUITransform.h"

TOSHI_NAMESPACE_START

// de Blob's T2GUIRenderer. Materials are created by T2GUI (static functions) or by the
// platform renderers' own extra virtuals (A2GUIRenderer +0x50.., TXUIRenderer +0x50..), not by the base class.
// NOTE: Barnyard's T2GUIRenderer has CreateMaterial/DestroyMaterial/GetTexture/GetWidth/GetHeight/ResetRenderer/
// PrepareRenderer/ResetZCoordinate as base virtuals, de Blob's doesn't.
class T2GUIRenderer
{
public:
	T2GUIRenderer();

	virtual ~T2GUIRenderer(); // +0x00

	// $deBlob: FUNCTION 006d6e90
	virtual TTexture* GetTexture( const TCHAR* a_szTextureName ) { return TNULL; }                                                // +0x04
	virtual void      BeginScene()                                                                                                = 0; // +0x08
	virtual void      EndScene()                                                                                                  = 0; // +0x0C
	virtual void      SetMaterial( T2GUIMaterial* a_pMaterial )                                                                   = 0; // +0x10
	virtual void      PushTransform( const T2GUITransform& a_rTransform, const TVector2& a_rVec1, const TVector2& a_rVec2 )       = 0; // +0x14
	virtual void      PopTransform()                                                                                              = 0; // +0x18
	virtual void      SetTransform( const T2GUITransform& a_rTransform )                                                          = 0; // +0x1C
	virtual void      SetColour( TUINT32 a_uiColour )                                                                             = 0; // +0x20
	virtual void      RenderRectangle( const TVector2& a_rA, const TVector2& a_rB, const TVector2& a_rUV1, const TVector2& a_rUV2 ) = 0; // +0x24
	virtual void      RenderTriStrip( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) = 0; // +0x28
	// NOTE: MSVC puts overloaded virtuals in reverse declaration order: the float overload is declared first so that
	// the TVector2 one gets the lower slot, as in retail.
	virtual void      RenderLine( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 )                                        = 0; // +0x30
	virtual void      RenderLine( const TVector2& a_rA, const TVector2& a_rB )                                                    = 0; // +0x2C
	virtual void      RenderOutlineRectangle( const TVector2& a_rA, const TVector2& a_rB )                                        = 0; // +0x34
	virtual void      RenderFilledRectangle( const TVector2& a_rA, const TVector2& a_rB )                                         = 0; // +0x38
	virtual void      RenderIndexedTriList( TVector2* a_pVertices, TVector2* a_pUVs, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) = 0; // +0x3C
	virtual void      RenderIndexedTriStripColoursList( TVector2* a_pVertices, TVector2* a_pUVs, TUINT32* a_pColours, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) = 0; // +0x40
	virtual void      ScaleCoords( TFLOAT& a_rX, TFLOAT& a_rY )                                                                   = 0; // +0x44
	virtual void      SetScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )                                = 0; // +0x48
	virtual void      ClearScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 )                              = 0; // +0x4C

	// $deBlob: FUNCTION 006f4df0
	void SetWhiteGlyphs( TUINT32* a_puiWhiteGlyphs, TINT a_iWhiteGlyphsCount )
	{
		m_puiWhiteGlyphs   = a_puiWhiteGlyphs;
		m_WhiteGlyphsCount = a_iWhiteGlyphsCount;
	}

	// $deBlob: FUNCTION 006f4e10
	void SetWhiteGlyph( TUINT16 a_uiGlyph )
	{
		if ( TINT( a_uiGlyph ) < m_WhiteGlyphsCount )
		{
			TUINT32 wordIndex = a_uiGlyph >> 5;
			TUINT32 bitShift  = a_uiGlyph & 31;
			TASSERT( wordIndex < m_WhiteGlyphsCount / 32 );
			TASSERT( bitShift < 32 );
			m_puiWhiteGlyphs[ wordIndex ] |= 1 << bitShift;
		}
	}

protected:
	TUINT32* m_puiWhiteGlyphs;   // 0x04 bit field, one bit per glyph
	TINT     m_WhiteGlyphsCount; // 0x08
};

TOSHI_NAMESPACE_END
