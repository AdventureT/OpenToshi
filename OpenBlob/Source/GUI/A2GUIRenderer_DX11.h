#pragma once
#include <GUI/T2GUIRenderer.h>

//-----------------------------------------------------------------------------
// Purpose: the renderer of T2GUI (the game's 2D elements: the load icon, the fades...).
// Draws with TPrimShader in display coordinates (the canvas isn't scaled, unlike TXUIRenderer).
//-----------------------------------------------------------------------------
class A2GUIRenderer : public Toshi::T2GUIRenderer
{
public:
	static TCONSTEXPR_VAR TINT TRANSFORMSTACKSIZE = 32;
	static TCONSTEXPR_VAR TINT MAXVERTS           = 128;

public:
	A2GUIRenderer();
	virtual ~A2GUIRenderer(); // +0x00

	//-----------------------------------------------------------------------------
	// T2GUIRenderer
	//-----------------------------------------------------------------------------
	virtual Toshi::TTexture* GetTexture( const TCHAR* a_szTextureName ) override;                                                                                                                                                                                                  // +0x04
	virtual void             BeginScene() override;                                                                                                                                                                                                                                // +0x08
	virtual void             EndScene() override;                                                                                                                                                                                                                                  // +0x0C
	virtual void             SetMaterial( Toshi::T2GUIMaterial* a_pMaterial ) override;                                                                                                                                                                                            // +0x10
	virtual void             PushTransform( const Toshi::T2GUITransform& a_rTransform, const Toshi::TVector2& a_rVec1, const Toshi::TVector2& a_rVec2 ) override;                                                                                                                  // +0x14
	virtual void             PopTransform() override;                                                                                                                                                                                                                              // +0x18
	virtual void             SetTransform( const Toshi::T2GUITransform& a_rTransform ) override;                                                                                                                                                                                   // +0x1C
	virtual void             SetColour( TUINT32 a_uiColour ) override;                                                                                                                                                                                                             // +0x20
	virtual void             RenderRectangle( const Toshi::TVector2& a_rA, const Toshi::TVector2& a_rB, const Toshi::TVector2& a_rUV1, const Toshi::TVector2& a_rUV2 ) override;                                                                                                   // +0x24
	virtual void             RenderTriStrip( Toshi::TVector2* a_pVertices, Toshi::TVector2* a_pUVs, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) override;                                                                                                     // +0x28
	virtual void             RenderLine( const Toshi::TVector2& a_rA, const Toshi::TVector2& a_rB ) override;                                                                                                                                                                      // +0x2C
	virtual void             RenderLine( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 ) override;                                                                                                                                                                        // +0x30
	virtual void             RenderOutlineRectangle( const Toshi::TVector2& a_rA, const Toshi::TVector2& a_rB ) override;                                                                                                                                                          // +0x34
	virtual void             RenderFilledRectangle( const Toshi::TVector2& a_rA, const Toshi::TVector2& a_rB ) override;                                                                                                                                                           // +0x38
	virtual void             RenderIndexedTriList( Toshi::TVector2* a_pVertices, Toshi::TVector2* a_pUVs, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) override;                                  // +0x3C
	virtual void             RenderIndexedTriStripColoursList( Toshi::TVector2* a_pVertices, Toshi::TVector2* a_pUVs, TUINT32* a_pColours, void* a_pIndices, TUINT32 a_uiNumIndices, TINT a_iIndexSize, TUINT32 a_uiNumVerts, TFLOAT a_fPosScaleX, TFLOAT a_fPosScaleY ) override; // +0x40
	virtual void             ScaleCoords( TFLOAT& a_rX, TFLOAT& a_rY ) override;                                                                                                                                                                                                   // +0x44
	virtual void             SetScissor( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2 ) override;                                                                                                                                                                        // +0x48
	virtual void             ClearScissor( TFLOAT a_fVal1, TFLOAT a_fVal2, TFLOAT a_fVal3, TFLOAT a_fVal4 ) override;                                                                                                                                                              // +0x4C

	//-----------------------------------------------------------------------------
	// A2GUIRenderer
	//-----------------------------------------------------------------------------
	// NOTE: MSVC puts overloaded virtuals in the vtable in reverse declaration order
	virtual Toshi::T2GUIMaterial* CreateMaterial( Toshi::TTexture* a_pTexture );        // +0x54
	virtual Toshi::T2GUIMaterial* CreateMaterial( const TCHAR* a_szTextureName );       // +0x50
	virtual void                  DestroyMaterial( Toshi::T2GUIMaterial* a_pMaterial ); // +0x58
	virtual TUINT                 GetWidth( Toshi::T2GUIMaterial* a_pMaterial );        // +0x5C
	virtual TUINT                 GetHeight( Toshi::T2GUIMaterial* a_pMaterial );       // +0x60

private:
	// Sets the top transform as the model view (when it changed)
	void UpdateTransform();

private:
	Toshi::T2GUITransform* m_pTransforms;            // 0x0C transform stack
	TINT                   m_iTransformStackPointer; // 0x10
	TUINT32                m_uiColour;               // 0x14
	TBOOL                  m_bTransformChanged;      // 0x18 the top transform isn't the model view yet
	Toshi::T2GUIMaterial*  m_pMaterial;              // 0x1C
};
