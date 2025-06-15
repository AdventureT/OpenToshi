#pragma once
#include "TTexture_SDL.h"

TOSHI_NAMESPACE_START

class TPrimShader : public TSingleton<TPrimShader>
{
public:
	static constexpr size_t ONE_VERTEX_BUFFER_SIZE = 40000;

	struct Mesh
	{
		UINT uiStartPrim;
		UINT uiNumPrims;
	};

	struct Vertex
	{
		TVector3 Position;
		UINT     Color;
		TVector2 UV;
	};

	typedef UINT PrimType;
	enum PrimType_ : PrimType
	{
		PrimType_TriangleStrip,
		PrimType_TriangleList,
		PrimType_LineStrip,
		PrimType_LineList,
	};

	typedef UINT BlendMode;

public:
	TPrimShader();
	~TPrimShader();

	void Initialize();
	void Destroy();

	TBOOL AddMesh();
	void  AddVert();

	void Unlock();
	void StartRendering(PrimType primType);
	void StopRendering();

	void SetBlendModeAndUnlock(BlendMode blendMode)
	{
		if (m_eBlendMode != blendMode && (m_eBlendMode = blendMode, m_bIsLocked))
		{
			Unlock();
		}
	}

	void SetImageAndUnlock(TTexture* pTexture)
	{
		if (m_pImage != pTexture && (m_pImage = pTexture, m_bIsLocked))
		{
			Unlock();
		}
	}

	Mesh* GetCurrentMesh()
	{
		TASSERT(m_uiNumUsedMeshes - 1 < m_uiNumMeshes);
		return &m_pMeshes[m_uiNumUsedMeshes - 1];
	}

	Vertex* GetCurrentVertex() { return m_pCurrentVertex; }

private:
	void DrawPrim(PrimType primType, UINT numPrim, UINT startPrim);

private:
	Mesh*     m_pMeshes;
	UINT      m_uiNumMeshes;
	UINT      m_uiNumUsedMeshes;
	UINT      m_uiNumDrawnFrames;
	UINT      m_uiNumUsedVerts;
	Vertex*   m_pCurrentVertex;
	TBOOL     m_bIsLocked;
	TBOOL     m_bIsRendering;
	PrimType  m_ePrimType;
	BlendMode m_eBlendMode;
	TTexture* m_pImage;
};

TOSHI_NAMESPACE_END
