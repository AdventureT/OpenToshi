#pragma once
#include "TTexture_DX11.h"
#include "Math/TVector2.h"
#include "Math/TVector3.h"

#include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: draws 2D/3D primitives (quads, lines, strips) with a colour and an optional texture straight from a dynamic
// vertex buffer, in the current render context's model view and projection. Used by the GUI and XUI renderers and the
// debug draws. Created by ARenderer. NOTE: retail keeps the fields in a member m_Data
// (the asserts say "m_Data.m_bIsRendering") whose constructor (Initialise) and destructor (Destroy) are out of line;
// here they are TPrimShader's own
//-----------------------------------------------------------------------------
class TPrimShader
{
public:
	static TCONSTEXPR_VAR TINT ONE_VERTEX_BUFFER_SIZE = 40000;

	struct Mesh
	{
		UINT uiStartVertex; // 0x00
		UINT uiNumVerts;    // 0x04
	};

	struct Vertex
	{
		TVector3 Position; // 0x00
		TUINT32  Colour;   // 0x0C B8G8R8A8
		TVector2 UV;       // 0x10
	}; // 0x18

	typedef TUINT32 PrimType;
	enum PrimType_ : PrimType
	{
		PrimType_TriangleStrip,
		PrimType_TriangleList,
		PrimType_LineStrip,
		PrimType_LineList,
	};

	// 0: opaque, 2: additive, anything else: alpha blended
	typedef TUINT32 BlendMode;
	enum BlendMode_ : BlendMode
	{
		BlendMode_Opaque   = 0,
		BlendMode_Alpha    = 1,
		BlendMode_Additive = 2,
	};

public:
	TPrimShader();
	~TPrimShader();

	// Starts a batch of a_ePrimType primitives: binds the shaders, the image and the render states and takes a mesh
	void StartRendering( PrimType a_ePrimType );
	// Draws the vertices added since StartRendering and restores the render states
	void StopRendering();

	// Takes the next mesh and maps the vertex buffer (discarding it)
	TBOOL AddMesh();
	// Finishes the current vertex (GetCurrentVertex) and advances
	void AddVert();
	// Unmaps the vertex buffer and draws the current mesh
	void Unlock();
	// Draws the current mesh if the buffer is mapped
	void Flush();

	void SetBlendModeAndUnlock( BlendMode a_eBlendMode );
	void SetImageAndUnlock( TTexture* a_pTexture );

	Vertex*   GetCurrentVertex() const { return m_pCurrentVertex; }
	BlendMode GetBlendMode() const { return m_eBlendMode; }
	TTexture* GetImage() const { return m_pImage; }

	ID3D11VertexShader* GetVertexShader() const { return m_pVertexShader; }
	ID3D11PixelShader*  GetImagePixelShader() const { return m_pImagePixelShader; }
	ID3D11PixelShader*  GetColourPixelShader() const { return m_pColourPixelShader; }
	ID3D11InputLayout*  GetInputLayout() const { return m_pInputLayout; }

	static TPrimShader* GetSingleton() { return s_pSingleton; }

private:
	// Compiles the shaders, creates the input layout, the meshes and the vertex buffer (retail: m_Data's constructor)
	void Initialise();
	// Releases them (retail: m_Data's destructor)
	void Destroy();

	void DrawPrim( PrimType a_ePrimType, UINT a_uiNumVerts, UINT a_uiStartVertex );

private:
	static TPrimShader* s_pSingleton;

private:
	Mesh*               m_pMeshes;            // 0x00
	TINT                m_iNumMeshes;         // 0x04
	TINT                m_iNumUsedMeshes;     // 0x08
	TUINT32             m_uiFrame;            // 0x0C TRenderDX11's frame of the meshes
	TINT                m_iNumUsedVerts;      // 0x10
	Vertex*             m_pCurrentVertex;     // 0x14
	TBOOL               m_bLocked;            // 0x18
	TBOOL               m_bIsRendering;       // 0x19
	PrimType            m_ePrimType;          // 0x1C
	BlendMode           m_eBlendMode;         // 0x20
	TTexture*           m_pImage;             // 0x24
	ID3D11VertexShader* m_pVertexShader;      // 0x28
	ID3D11PixelShader*  m_pImagePixelShader;  // 0x2C
	ID3D11PixelShader*  m_pColourPixelShader; // 0x30
	ID3D11InputLayout*  m_pInputLayout;       // 0x34
	ID3D11Buffer*       m_pVertexBuffer;      // 0x38
}; // 0x3C

TOSHI_NAMESPACE_END
