#pragma once
#include "T2GUIElement.h"
#include "T2GUIMaterial.h"
#include "Toshi/Render/TTexture.h"

TOSHI_NAMESPACE_START

class T2GUIRenderer
{
public:
	T2GUIRenderer()          = default;
	virtual ~T2GUIRenderer() = default;

	virtual TTexture* GetTexture(const TCHAR* texName) const;
	virtual void      BeginScene()                                                                                                                                                                                       = 0;
	virtual void      EndScene()                                                                                                                                                                                         = 0;
	virtual void      SetMaterial(T2GUIMaterial* pMat)                                                                                                                                                                   = 0;
	virtual void      PushTransform(const T2GUITransform& transform, const TVector2& vec1, const TVector2& vec2)                                                                                                         = 0;
	virtual void      PopTransform()                                                                                                                                                                                     = 0;
	virtual void      SetTransform(const T2GUITransform& transform)                                                                                                                                                      = 0;
	virtual void      SetColour(TUINT32 colour)                                                                                                                                                                          = 0;
	virtual void      RenderRectangle(const TVector2& a, const TVector2& b, const TVector2& uv1, const TVector2& uv2)                                                                                                    = 0;
	virtual void      RenderTriStrip(TVector2* vertices, TVector2* UV, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY)                                                                                           = 0;
	virtual void      RenderLine(const TVector2& a, const TVector2& b)                                                                                                                                                   = 0;
	virtual void      RenderLine(TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2)                                                                                                                                             = 0;
	virtual void      RenderOutlineRectangle(const TVector2& a, const TVector2& b)                                                                                                                                       = 0;
	virtual void      RenderFilledRectangle(const TVector2& a, const TVector2& b)                                                                                                                                        = 0;
	virtual void      RenderIndexedTriList(TVector2* pVertices, TVector2* pUV, void* pIndices, TUINT32 numindices, TINT indexSize, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY)                               = 0;
	virtual void      RenderIndexedTriStripColoursList(TVector2* pVertices, TVector2* pUV, TUINT32* pColors, void* pIndices, TUINT32 numindices, TINT indexSize, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY) = 0;
	virtual void      ScaleCoords(TFLOAT& x, TFLOAT& y)                                                                                                                                                                  = 0;
	virtual void      SetScissors(TFLOAT f1, TFLOAT f2, TFLOAT f3, TFLOAT f4)                                                                                                                                            = 0;
	virtual void      ClearScissors()                                                                                                                                                                                    = 0;

private:
	TINT m_WhiteGlyphsCount;
};

TOSHI_NAMESPACE_END
