#include "ToshiPCH.h"
#include "TXUIRenderer_DX11.h"
#include "Toshi2/T2GUI/T2GUI.h"
#include "Platform/DX11/TRenderContext_DX11.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006d5030
TXUIRenderer::TXUIRenderer()
{
	m_pTransforms            = new T2GUITransform[TRANSFORMSTACKSIZE];
	m_iTransformStackPointer = 0;
	m_bIsInScene             = TFALSE;
	m_OldDepthState.Raw      = 0;
}

// $deBlob: FUNCTION 006d50e0
TXUIRenderer::~TXUIRenderer()
{
	delete[] m_pTransforms;
}

// $deBlob: FUNCTION 006d51c0
void TXUIRenderer::BeginScene()
{
	TASSERT(m_iTransformStackPointer == 0);

	auto p2GUI          = T2GUI::GetSingleton();
	auto pRender        = TRenderDX11::Interface();
	auto pDisplayParams = pRender->GetCurrentDisplayParams();
	auto pRenderContext = TSTATICCAST(TRenderContextDX11, pRender->GetCurrentRenderContext());
	pRender->FlushShaders();

	TFLOAT fCanvasWidth, fCanvasHeight;
	p2GUI->GetRootElement()->GetDimensions(fCanvasWidth, fCanvasHeight);

	auto& pTransform = m_pTransforms[m_iTransformStackPointer];
	pTransform.m_Rot[0].SetX(pDisplayParams->Width / fCanvasWidth);
	pTransform.m_Rot[0].SetY(0);
	pTransform.m_Rot[1].SetX(0);
	pTransform.m_Rot[1].SetY(pDisplayParams->Height / fCanvasHeight);
	pTransform.m_Pos.x = 0;
	pTransform.m_Pos.y = 0;

	static TBOOL     s_IsMatrixSet = TFALSE;
	static TMatrix44 s_IdentityMatrix;

	if (!s_IsMatrixSet)
	{
		s_IsMatrixSet = TTRUE;
		s_IdentityMatrix.Identity();
	}

	pRenderContext->SetModelViewMatrix(s_IdentityMatrix);
	pRenderContext->SetWorldViewMatrix(s_IdentityMatrix);

	TRenderContext::PROJECTIONPARAMS projParams;
	projParams.m_Proj.x    = 1.0f;
	projParams.m_Proj.y    = -1.0f;
	projParams.m_Centre.x  = pDisplayParams->Width * 0.5f;
	projParams.m_Centre.y  = pDisplayParams->Height * 0.5f;
	projParams.m_fNearClip = 0.0f;
	projParams.m_fFarClip  = 1.0f;

	pRenderContext->SetCameraMode(TRenderContext::CameraMode_Orthographic);
	pRenderContext->SetProjectionParams(projParams);
	pRenderContext->Update();
	SetColour(0xFFFFFFFF);
}

// $deBlob: FUNCTION 006d53a0
void TXUIRenderer::EndScene()
{
	TASSERT(0 == m_iTransformStackPointer);
}

// $deBlob: FUNCTION 006d53e0
void TXUIRenderer::SetMaterial(T2GUIMaterial* pMat)
{
	auto      pPrimShader = TPrimShader::GetSingleton();
	TTexture* pTex        = TNULL;

	if (pMat != TNULL)
	{
		TASSERT(pMat->IsA(TGetClass(T2GUIMaterial)));
		TRenderDX11::Interface()->SetSamplerState(0, pMat->GetSamplerId(), TRUE);
		pTex = pMat->GetTexture();
	}

	pPrimShader->SetImageAndUnlock(pTex);
	m_pMaterial = pMat;
}

// $deBlob: FUNCTION 006d5480
void TXUIRenderer::PushTransform(const T2GUITransform& transform, const TVector2& vec1, const TVector2& vec2)
{
	T2GUITransform transform1;
	T2GUITransform transform2;

	m_iTransformStackPointer += 1;
	TASSERT(m_iTransformStackPointer < TRANSFORMSTACKSIZE);

	T2GUITransform& currTransform = *(m_pTransforms + m_iTransformStackPointer);
	T2GUITransform& prevTransform = *(&currTransform - 1);

	transform1 = transform;
	transform2 = prevTransform;
	transform2.Transform(transform2.GetPos(), vec1);
	transform1.Transform(transform1.GetPos(), vec2);
	T2GUITransform::Multiply(currTransform, transform2, transform1);
	m_bIsInScene = TTRUE;
}

// $deBlob: FUNCTION 006d5690
void TXUIRenderer::PopTransform()
{
	TASSERT(m_iTransformStackPointer > 0);
	m_iTransformStackPointer -= 1;
	m_bIsInScene = TTRUE;
}

// $deBlob: FUNCTION 006d56e0
void TXUIRenderer::SetTransform(const T2GUITransform& transform)
{
	m_pTransforms[m_iTransformStackPointer] = transform;
	m_bIsInScene                            = TTRUE;
}

// $deBlob: FUNCTION 006d6eb0
void TXUIRenderer::SetColour(TUINT32 colour)
{
	m_ui32Colour = colour;
}

// $deBlob: FUNCTION 006d5800
void TXUIRenderer::RenderRectangle(const TVector2& a, const TVector2& b, const TVector2& uv1, const TVector2& uv2)
{
	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto                 pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock(1);
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = uv1;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = { uv2.x, uv1.y };
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { b.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = { uv1.x, uv2.y };
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = uv2;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
}

// $deBlob: FUNCTION 006d59b0
void TXUIRenderer::RenderTriStrip(TVector2* vertices, TVector2* UV, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY)
{
	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TASSERT(numverts < MAXVERTS);

	TPrimShader::Vertex* pVertex;
	auto                 pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock(1);
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	if (UV != TNULL)
	{
		for (size_t i = 0; i < numverts; i++)
		{
			pVertex           = pPrimShader->GetCurrentVertex();
			pVertex->UV       = { UV[i].x, UV[i].y };
			pVertex->Color    = m_ui32Colour;
			pVertex->Position = { vertices[i].x * fPosScaleX, vertices[i].y * fPosScaleY, 0.0f };
			pPrimShader->AddVert();
		}
	}
	else
	{
		for (size_t i = 0; i < numverts; i++)
		{
			pVertex           = pPrimShader->GetCurrentVertex();
			pVertex->UV       = TVector2::VEC_ZERO;
			pVertex->Color    = m_ui32Colour;
			pVertex->Position = { vertices[i].x * fPosScaleX, vertices[i].y * fPosScaleY, 0.0f };
			pPrimShader->AddVert();
		}
	}

	pPrimShader->StopRendering();
}

void TXUIRenderer::RenderLine(const TVector2& a, const TVector2& b)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto                 pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock(0);
	pPrimShader->StartRendering(TPrimShader::PrimType_LineList);

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

void TXUIRenderer::RenderLine(TFLOAT x1, TFLOAT y1, TFLOAT x2, TFLOAT y2)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto                 pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock(0);
	pPrimShader->StartRendering(TPrimShader::PrimType_LineList);

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { x1, y1, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { x1, y2, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

// $deBlob: FUNCTION 006d5f50
void TXUIRenderer::RenderOutlineRectangle(const TVector2& a, const TVector2& b)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto                 pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock(0);
	pPrimShader->StartRendering(TPrimShader::PrimType_LineStrip);

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

// $deBlob: FUNCTION 006d6100
void TXUIRenderer::RenderFilledRectangle(const TVector2& a, const TVector2& b)
{
	auto pOldMat = m_pMaterial;
	SetMaterial(TNULL);

	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TPrimShader::Vertex* pVertex;
	auto                 pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock(0);
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { b.x, a.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { a.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pVertex           = pPrimShader->GetCurrentVertex();
	pVertex->UV       = TVector2::VEC_ZERO;
	pVertex->Color    = m_ui32Colour;
	pVertex->Position = { b.x, b.y, 0.0f };
	pPrimShader->AddVert();

	pPrimShader->StopRendering();
	SetMaterial(pOldMat);
}

// $deBlob: FUNCTION 006d62b0
void TXUIRenderer::RenderIndexedTriList(TVector2* pVertices, TVector2* pUV, void* pIndices, TUINT32 numindices, TINT indexSize, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY)
{
	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TASSERT(numverts < MAXVERTS);

	auto pRender        = TRenderDX11::Interface();
	auto pRenderContext = TSTATICCAST(TRenderContextDX11, pRender->GetCurrentRenderContext());
	auto pDeviceContext = pRender->GetDeviceContext();
	auto pPrimShader    = TPrimShader::GetSingleton();

	pRender->SetZMode(TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);
	pRender->SetBlendMode(TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);

	TMatrix44 worldViewProjection;
	worldViewProjection.Multiply(pRenderContext->GetProjectionMatrix(), pRenderContext->GetModelViewMatrix());

	pRender->SetVec4InVSBuffer(0, &worldViewProjection, 4);
	pDeviceContext->IASetInputLayout(pPrimShader->GetInputLayout());
	pRender->m_CurrentRasterizerId.Flags.Parts.CullMode = D3D11_CULL_NONE;

	TASSERT(pRender->IsInScene());
	pDeviceContext->VSSetShader(pPrimShader->GetVertexShader(), NULL, 0);

	if (pUV != TNULL)
	{
		for (size_t i = 0; i < numverts; i++)
		{
			TPrimShader::Vertex* pVertex = &s_Vertices[i];
			pVertex->UV                  = { pUV[i].x, pUV[i].y };
			pVertex->Color               = m_ui32Colour;
			pVertex->Position            = { pVertices[i].x * fPosScaleX, pVertices[i].y * fPosScaleY, 0.0f };
		}
	}
	else
	{
		for (size_t i = 0; i < numverts; i++)
		{
			TPrimShader::Vertex* pVertex = &s_Vertices[i];
			pVertex->UV                  = TVector2::VEC_ZERO;
			pVertex->Color               = m_ui32Colour;
			pVertex->Position            = { pVertices[i].x * fPosScaleX, pVertices[i].y * fPosScaleY, 0.0f };
		}
	}

	if (m_pMaterial)
	{
		TASSERT(TTRUE == m_pMaterial->IsA(TGetClass(T2GUIMaterial)));
		m_pMaterial->GetTexture()->Bind(0);
		pDeviceContext->PSSetShader(pPrimShader->GetPixelImageShader(), NULL, 0);
	}
	else
	{
		pDeviceContext->PSSetShader(pPrimShader->GetPixelBasicShader(), NULL, 0);
	}

	DXGI_FORMAT eFormat = (indexSize == 2) ? DXGI_FORMAT_R16_UINT : (indexSize == 4) ? DXGI_FORMAT_R32_UINT :
	                                                                                   DXGI_FORMAT_UNKNOWN;

	TASSERT(eFormat != DXGI_FORMAT_UNKNOWN);

	pRender->DrawImmediately(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, numindices, pIndices, eFormat, s_Vertices, sizeof(TPrimShader::Vertex), numverts);
	pRender->SetZMode(TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL);
	pRender->SetBlendMode(TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
}

// $deBlob: FUNCTION 006d66d0
void TXUIRenderer::RenderIndexedTriStripColoursList(TVector2* pVertices, TVector2* pUV, TUINT32* pColors, void* pIndices, TUINT32 numindices, TINT indexSize, TUINT32 numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY)
{
	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TASSERT(numverts < MAXVERTS);

	auto pRender        = TRenderDX11::Interface();
	auto pRenderContext = TSTATICCAST(TRenderContextDX11, pRender->GetCurrentRenderContext());
	auto pDeviceContext = pRender->GetDeviceContext();
	auto pPrimShader    = TPrimShader::GetSingleton();

	TMatrix44 worldViewProjection;
	worldViewProjection.Multiply(pRenderContext->GetProjectionMatrix(), pRenderContext->GetModelViewMatrix());

	pRender->SetVec4InVSBuffer(0, &worldViewProjection, 4);
	pDeviceContext->IASetInputLayout(pPrimShader->GetInputLayout());
	pRender->m_CurrentRasterizerId.Flags.Parts.CullMode = D3D11_CULL_NONE;

	TASSERT(pRender->IsInScene());
	pDeviceContext->VSSetShader(pPrimShader->GetVertexShader(), NULL, 0);

	if (pUV != TNULL)
	{
		for (size_t i = 0; i < numverts; i++)
		{
			TPrimShader::Vertex* pVertex = &s_Vertices[i];
			pVertex->UV                  = { pUV[i].x, pUV[i].y };
			pVertex->Color               = pColors[i];
			pVertex->Position            = { pVertices[i].x * fPosScaleX, pVertices[i].y * fPosScaleY, 0.0f };
		}
	}
	else
	{
		for (size_t i = 0; i < numverts; i++)
		{
			TPrimShader::Vertex* pVertex = &s_Vertices[i];
			pVertex->UV                  = TVector2::VEC_ZERO;
			pVertex->Color               = pColors[i];
			pVertex->Position            = { pVertices[i].x * fPosScaleX, pVertices[i].y * fPosScaleY, 0.0f };
		}
	}

	if (m_pMaterial)
	{
		TASSERT(TTRUE == m_pMaterial->IsA(TGetClass(T2GUIMaterial)));
		m_pMaterial->GetTexture()->Bind(0);
		pDeviceContext->PSSetShader(pPrimShader->GetPixelImageShader(), NULL, 0);
	}
	else
	{
		pDeviceContext->PSSetShader(pPrimShader->GetPixelBasicShader(), NULL, 0);
	}

	DXGI_FORMAT eFormat = (indexSize == 2) ? DXGI_FORMAT_R16_UINT : (indexSize == 4) ? DXGI_FORMAT_R32_UINT :
	                                                                                   DXGI_FORMAT_UNKNOWN;

	TASSERT(eFormat != DXGI_FORMAT_UNKNOWN);

	pRender->DrawImmediately(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, numindices, pIndices, eFormat, s_Vertices, sizeof(TPrimShader::Vertex), numverts);
}

// $deBlob: FUNCTION 006d6ab0
void TXUIRenderer::ScaleCoords(TFLOAT& x, TFLOAT& y)
{
	auto          pDisplayParams = TRenderDX11::Interface()->GetCurrentDisplayParams();
	T2GUIElement* pRoot          = T2GUI::GetSingleton()->GetRootElement();

	TFLOAT fRootWidth, fRootHeight;
	pRoot->GetDimensions(fRootWidth, fRootHeight);

	x /= pDisplayParams->Width / fRootWidth;
	y /= pDisplayParams->Height / fRootHeight;
}

// $deBlob: FUNCTION 006d5730
void TXUIRenderer::SetScissors(TFLOAT f1, TFLOAT f2, TFLOAT f3, TFLOAT f4)
{
	TASSERT(m_OldDepthState.Raw == 0);

	auto pRender    = TRenderDX11::Interface();
	m_OldDepthState = pRender->GetCurrentDepthState();
	pRender->SetZMode(TFALSE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);
	SetColour(0x00000000);
	pRender->SetZMode(TTRUE, D3D11_COMPARISON_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL);
}

// $deBlob: FUNCTION 006d57b0
void TXUIRenderer::ClearScissors()
{
	auto pRender = TRenderDX11::Interface();

	pRender->SetZMode(TTRUE, D3D11_COMPARISON_ALWAYS, D3D11_DEPTH_WRITE_MASK_ALL);
	SetColour(0x00000000);
	pRender->SetCurrentDepthState(m_OldDepthState);
	m_OldDepthState.Raw = 0;
}

// $deBlob: FUNCTION 006d5100
TUINT32 TXUIRenderer::GetWidth(T2GUIMaterial* pMat)
{
	TASSERT(pMat->IsA(TGetClass(T2GUIMaterial)));
	TTexture* pTex = pMat->GetTexture();

	if (pTex != TNULL)
	{
		return pTex->GetWidth();
	}

	return 0;
}

// $deBlob: FUNCTION 006d5160
TUINT32 TXUIRenderer::GetHeight(T2GUIMaterial* pMat)
{
	TASSERT(pMat->IsA(TGetClass(T2GUIMaterial)));
	TTexture* pTex = pMat->GetTexture();

	if (pTex != TNULL)
	{
		return pTex->GetHeight();
	}

	return 0;
}

// $deBlob: FUNCTION 006d5b70
void TXUIRenderer::RenderTriStripColours(TVector2* pVertices, TVector2* pUV, TUINT32* pColors, size_t numverts, TFLOAT fPosScaleX, TFLOAT fPosScaleY)
{
	if (m_bIsInScene)
	{
		auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
		auto pTransform     = m_pTransforms + m_iTransformStackPointer;

		TMatrix44 mat44;
		pTransform->Matrix44(mat44);
		pRenderContext->SetModelViewMatrix(mat44);
		pRenderContext->SetWorldViewMatrix(mat44);

		m_bIsInScene = TFALSE;
	}

	TASSERT(numverts < MAXVERTS);

	TPrimShader::Vertex* pVertex;
	auto                 pPrimShader = TPrimShader::GetSingleton();
	pPrimShader->SetBlendModeAndUnlock(1);
	pPrimShader->StartRendering(TPrimShader::PrimType_TriangleStrip);

	if (pUV != TNULL)
	{
		for (size_t i = 0; i < numverts; i++)
		{
			pVertex           = pPrimShader->GetCurrentVertex();
			pVertex->UV       = { pUV[i].x, pUV[i].y };
			pVertex->Color    = pColors[i];
			pVertex->Position = { pVertices[i].x * fPosScaleX, pVertices[i].y * fPosScaleY, 0.0f };
			pPrimShader->AddVert();
		}
	}
	else
	{
		for (size_t i = 0; i < numverts; i++)
		{
			pVertex           = pPrimShader->GetCurrentVertex();
			pVertex->UV       = TVector2::VEC_ZERO;
			pVertex->Color    = pColors[i];
			pVertex->Position = { pVertices[i].x * fPosScaleX, pVertices[i].y * fPosScaleY, 0.0f };
			pPrimShader->AddVert();
		}
	}

	pPrimShader->StopRendering();
}

TOSHI_NAMESPACE_END
