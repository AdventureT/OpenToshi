#pragma once
#include <Toshi/TSingleton.h>
#include <Math/TVector2.h>
#include <Math/TVector3.h>

#include <d3d11.h>

namespace Toshi
{
class TTexture;
}

//-----------------------------------------------------------------------------
// Purpose: the shader of the decals: one dynamic vertex buffer used as a ring of triangles (position, colour, uv),
// drawn alpha blended with the texture of the last decal added and an alpha test. Owned by ADecalManager.
//-----------------------------------------------------------------------------
class ADecalShader
    : public Toshi::TSingleton<ADecalShader>
{
public:
	// The vertex of the decals (0x18 bytes)
	struct Vertex
	{
		Toshi::TVector3 vPosition; // 0x00
		TUINT32         uiColour;  // 0x0C B8G8R8A8
		Toshi::TVector2 vUV;       // 0x10
	};

	// A ring vertex buffer (0x18 bytes)
	struct Mesh
	{
		ID3D11Buffer* m_pVertexBuffer;      // 0x00
		TUINT         m_uiCurVertex;        // 0x04 the next vertex written (wraps to 0)
		TUINT         m_uiVertexBufferSize; // 0x08 in vertices
		TINT          m_iNumVertices;       // 0x0C
		TUINT32       m_uiUnk10;            // 0x10 (always 0)
		TUINT8        m_uiUnk14;            // 0x14 (never set)
		TUINT8        m_uiFlags;            // 0x15 1: valid (ValidateMesh)
	};

	enum MeshFlags : TUINT8
	{
		MeshFlags_Valid = 1,
	};

	static TCONSTEXPR_VAR TINT INT_NUM_VERTS_PER_PRIMITIVE = 3;

public:
	ADecalShader( TINT a_iNumVertices );
	~ADecalShader();

	void Create( TINT a_iNumVertices );

	Mesh* CreateMesh( TINT a_iNumVerts, TUINT32 a_uiUnk );
	// NOTE: doesn't use this (called on the singleton, also when it's destroyed already)
	static void DestroyMesh( Mesh* a_pMesh );
	void        ValidateMesh( Mesh* a_pMesh );

	// The mesh functions (__fastcall, the mesh in ecx)
	static void    ResetMesh( Mesh* a_pMesh );
	static Vertex* LockDiscard( Mesh* a_pMesh );
	static Vertex* Lock( Mesh* a_pMesh );
	static void    Unlock( Mesh* a_pMesh );

	void PreRender();
	void Render( Mesh* a_pMesh );
	void PostRender();

	static TBOOL IsValid( Mesh* a_pMesh ) { return a_pMesh->m_uiFlags & MeshFlags_Valid; }

	// $deBlob: FUNCTION 005f7590
	void SetVertexShader( ID3D11VertexShader* a_pVertexShader ) { m_pVertexShader = a_pVertexShader; }
	// $deBlob: FUNCTION 005f7570
	void SetTexture( Toshi::TTexture* a_pTexture ) { m_pTexture = a_pTexture; }

private:
	Toshi::TTexture*    m_pTexture;      // 0x00
	TINT                m_iNumVertices;  // 0x04
	ID3D11VertexShader* m_pVertexShader; // 0x08
	ID3D11PixelShader*  m_pPixelShader;  // 0x0C
	ID3D11InputLayout*  m_pInputLayout;  // 0x10
};
