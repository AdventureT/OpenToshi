#pragma once
#include "Toshi/Render/TRender.h"
#include "Toshi2/T2GUI/T2GUIMaterial.h"
#include "Toshi2/T2GUI/T2GUITransform.h"
#include "Toshi2/T2GUI/T2GUIRenderer.h"
#include "Platform/DX11/TRender_DX11.h"
#include "Platform/DX11/TPrimShader_DX11.h"

TOSHI_NAMESPACE_START

class TXUIRenderer : public T2GUIRenderer
{
public:
	static constexpr size_t TRANSFORMSTACKSIZE = 32;
	static constexpr size_t MAXVERTS           = 1024;

public:
	TXUIRenderer();
	~TXUIRenderer();

	virtual void    BeginScene() override;
	virtual void    EndScene() override;
	virtual void    SetMaterial(T2GUIMaterial* pMat) override;
	virtual void    PushTransform(const T2GUITransform& transform, const TVector2& vec1, const TVector2& vec2) override;
	virtual void    PopTransform() override;
	virtual void    SetTransform(const T2GUITransform& transform) override;
	virtual void    SetColour(TUINT32 colour) override;
	virtual void    RenderRectangle(const TVector2& a, const TVector2& b, const TVector2& uv1, const TVector2& uv2) override;
	virtual void    RenderTriStrip(TVector2* vertices, TVector2* UV, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY) override;
	virtual void    RenderLine(const TVector2& a, const TVector2& b) override;
	virtual void    RenderLine(TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2) override;
	virtual void    RenderOutlineRectangle(const TVector2& a, const TVector2& b) override;
	virtual void    RenderFilledRectangle(const TVector2& a, const TVector2& b) override;
	virtual void    RenderIndexedTriList(TVector2* pVertices, TVector2* pUV, void* pIndices, TUINT32 numindices, TINT indexSize, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY) override;
	virtual void    RenderIndexedTriStripColoursList(TVector2* pVertices, TVector2* pUV, TUINT32* pColors, void* pIndices, TUINT32 numindices, TINT indexSize, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY) override;
	virtual void    ScaleCoords(TFLOAT& x, TFLOAT& y) override;
	virtual void    SetScissors(TFLOAT f1, TFLOAT f2, TFLOAT f3, TFLOAT f4) override;
	virtual void    ClearScissors() override;
	virtual TUINT32 GetWidth(T2GUIMaterial* pMat);
	virtual TUINT32 GetHeight(T2GUIMaterial* pMat);
	virtual void    RenderTriStripColours(TVector2* pVertices, TVector2* pUV, TUINT32* pColors, size_t numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY);

private:
	inline static TPrimShader::Vertex s_Vertices[MAXVERTS];

private:
	T2GUITransform*         m_pTransforms;
	TINT                    m_iTransformStackPointer;
	TUINT32                 m_ui32Colour;
	TBOOL                   m_bIsInScene;
	T2GUIMaterial*          m_pMaterial;
	TRenderDX11::DepthState m_OldDepthState;
};

TOSHI_NAMESPACE_END
