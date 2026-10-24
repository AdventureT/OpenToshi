#pragma once
#include <Render/TShader.h>
#include <Render/TMesh.h>
#include <Render/TMaterial.h>
#include <Render/TOrderTable.h>
#include <Math/TVector2.h>
#include <Math/TVector3.h>

#include <d3d11.h>

class ARibbonTrailShader;

//-----------------------------------------------------------------------------
// Purpose: the material of the ribbon trails (TClass "ARibbonTrailMaterial"): one texture, alpha blended, registered in
// the shader's order table.
//-----------------------------------------------------------------------------
class ARibbonTrailMaterial
    : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( ARibbonTrailMaterial, Toshi::TMaterial );

public:
	ARibbonTrailMaterial();
	~ARibbonTrailMaterial();

	//-----------------------------------------------------------------------------
	// Toshi::TMaterial
	//-----------------------------------------------------------------------------
	virtual void PreRender() OVERRIDE;  // +0x14
	virtual void PostRender() OVERRIDE; // +0x18

	using Toshi::TMaterial::Create;

	void SetOrderTable( Toshi::TOrderTable* a_pOrderTable );

private:
	Toshi::TOrderTable* m_pOrderTable; // 0x54
};                                     // 0x58

//-----------------------------------------------------------------------------
// Purpose: a ribbon trail mesh (TClass "ARibbonTrailMesh"): a triangle strip drawn from system memory
// (ARibbonTrailShader::Render).
//-----------------------------------------------------------------------------
class ARibbonTrailMesh
    : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( ARibbonTrailMesh, Toshi::TMesh );

	struct Vertex
	{
		Vertex()
		    : uiColour( 0 ) {}

		Toshi::TVector3 vPosition; // 0x00
		TUINT32         uiColour;  // 0x0C
		Toshi::TVector2 vUV;       // 0x10
	};                             // 0x18

public:
	ARibbonTrailMesh();

	// NOTE: the plain destructors of the three mesh classes are byte-identical (TMesh's vtable store, jmp ~TMesh) and
	// unreferenced
	// $deBlob: FUNCTION 005ffe50
	// $deBlob: FUNCTION 005ffdf0
	~ARibbonTrailMesh() {}

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TUINT16* GetIndices()  = 0; // +0x20
	virtual Vertex*  GetVertices() = 0; // +0x24

	// $deBlob: FUNCTION 005fff40
	TUINT GetNumVertices() const { return m_uiNumVertices; }
	// $deBlob: FUNCTION 005fff30
	TUINT GetNumIndices() const { return m_uiNumIndices; }
	// $deBlob: FUNCTION 005fff90
	TBOOL IsAlphaUpdate() const { return m_bAlphaUpdate; }

	// $deBlob: FUNCTION 007bdfc0
	void SetAlphaUpdate( TBOOL a_bAlphaUpdate ) { m_bAlphaUpdate = a_bAlphaUpdate; }

protected:
	TBOOL   m_bAlphaUpdate;  // 0x10 passed to TRenderDX11::SetAlphaUpdate
	TUINT16 m_uiNumVertices; // 0x12
	TUINT16 m_uiNumIndices;  // 0x14
};                           // 0x18

//-----------------------------------------------------------------------------
// Purpose: a ribbon trail mesh made once from given vertices (TClass "ARibbonTrailMeshStatic")
//-----------------------------------------------------------------------------
class ARibbonTrailMeshStatic
    : public ARibbonTrailMesh
{
public:
	TDECLARE_CLASS( ARibbonTrailMeshStatic, ARibbonTrailMesh );

public:
	ARibbonTrailMeshStatic();

	// $deBlob: FUNCTION 005ffeb0
	// $deBlob: FUNCTION 005ffe10
	~ARibbonTrailMeshStatic() {}

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual void OnDestroy() OVERRIDE; // +0x1C

	//-----------------------------------------------------------------------------
	// ARibbonTrailMesh
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 005fff20
	virtual TUINT16* GetIndices() OVERRIDE { return m_pIndices; } // +0x20
	// $deBlob: FUNCTION 005fff70
	virtual Vertex* GetVertices() OVERRIDE { return m_pVertices; } // +0x24

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// Copies the vertices (a_pColour: TNULL or 0xFFFFFFFF keeps the colours 0); every 3rd TUINT16 of a_pIndices is
	// an index
	virtual TBOOL Create( TUINT16 a_uiNumVertices, TUINT16 a_uiNumIndices, const Toshi::TVector3* a_pPositions, const TUINT32* a_pColour, const Toshi::TVector2* a_pUVs, const TUINT16* a_pIndices ); // +0x28

	void SetColour( TUINT32 a_uiColour );

private:
	TUINT16* m_pIndices;  // 0x18
	Vertex*  m_pVertices; // 0x1C
	TUINT32  m_uiColour;  // 0x20
};                        // 0x24

//-----------------------------------------------------------------------------
// Purpose: a ribbon trail mesh rebuilt every frame (TClass "ARibbonTrailMeshDynamic"): the vertices and indices are
// written to a ring buffer (GetIndex/GetVertex), Swap copies them to the buffers the shader draws.
//-----------------------------------------------------------------------------
class ARibbonTrailMeshDynamic
    : public ARibbonTrailMesh
{
public:
	TDECLARE_CLASS( ARibbonTrailMeshDynamic, ARibbonTrailMesh );

public:
	ARibbonTrailMeshDynamic();

	// $deBlob: FUNCTION 005ffe80
	// $deBlob: FUNCTION 005ffe00
	~ARibbonTrailMeshDynamic() {}

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual void OnDestroy() OVERRIDE; // +0x1C

	//-----------------------------------------------------------------------------
	// ARibbonTrailMesh
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 005fff10
	virtual TUINT16* GetIndices() OVERRIDE { return m_pIndices; } // +0x20
	// $deBlob: FUNCTION 005fff60
	virtual Vertex* GetVertices() OVERRIDE { return m_pVertices; } // +0x24

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// a_uiMaxSegments: 2 vertices and 2 indices each
	virtual TBOOL Create( TUINT16 a_uiMaxSegments ); // +0x28

	// Copies the ring buffers to the buffers the shader draws
	void Swap();

	// $deBlob: FUNCTION 007bdeb0
	TUINT16& GetIndex( TUINT16 a_uiIndex )
	{
		TASSERT( a_uiIndex < m_uiMaxIndices );
		return m_pWriteIndices[ a_uiIndex ];
	}

	// $deBlob: FUNCTION 007bdf40
	Vertex& GetVertex( TUINT16 a_uiIndex )
	{
		TASSERT( a_uiIndex < m_uiMaxVerts );
		return m_pWriteVertices[ a_uiIndex ];
	}

	// $deBlob: FUNCTION 007bdf20
	TUINT GetMaxVerts() const { return m_uiMaxVerts; }
	// $deBlob: FUNCTION 007bdf10
	TUINT GetMaxIndices() const { return m_uiMaxIndices; }

private:
	TUINT16* m_pIndexData;     // 0x18 2 * m_uiMaxIndices
	Vertex*  m_pVertexData;    // 0x1C 3 * m_uiMaxVerts
	TUINT16* m_pWriteIndices;  // 0x20 the second half of m_pIndexData
	Vertex*  m_pWriteVertices; // 0x24 the third part of m_pVertexData
	TUINT16* m_pIndices;       // 0x28 the drawn indices
	Vertex*  m_pVertices;      // 0x2C the drawn vertices (alternates between the first two parts)
	TUINT16  m_uiMaxVerts;     // 0x30
	TUINT16  m_uiMaxIndices;   // 0x32

public:
	TUINT16 m_uiCurVert;         // 0x34 the next vertex to write
	TUINT16 m_uiCurIndex;        // 0x36 the next index to write
	TUINT16 m_uiNumWriteVerts;   // 0x38
	TUINT16 m_uiNumWriteIndices; // 0x3A the indices Swap copies (the last ones written)
};                               // 0x3C

//-----------------------------------------------------------------------------
// Purpose: the shader of the ribbon trails (TClass "ARibbonTrailShader", created by ARenderer::Create): one order table
// (priority 5500) and a position/colour/UV shader pair.
//-----------------------------------------------------------------------------
class ARibbonTrailShader
    : public Toshi::TShader
{
public:
	TDECLARE_CLASS( ARibbonTrailShader, Toshi::TShader );

public:
	ARibbonTrailShader();
	~ARibbonTrailShader();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  StartFlush() OVERRIDE; // +0x14
	virtual void  EndFlush() OVERRIDE;   // +0x18
	virtual TBOOL Create() OVERRIDE;     // +0x1C
	// $deBlob: FUNCTION 005fff50
	virtual const TCHAR* GetShaderPrefix() OVERRIDE { return ""; }                 // +0x28
	virtual void         Render( Toshi::TRenderPacket* a_pRenderPacket ) OVERRIDE; // +0x34

	Toshi::TOrderTable* GetOrderTable() { return &m_oOrderTable; }

	// $deBlob: FUNCTION 005fff80
	TBOOL IsReady() const { return m_bReady; }

	static ARibbonTrailShader* GetSingleton() { return ms_pSingleton; }

private:
	static ARibbonTrailShader* ms_pSingleton;

private:
	Toshi::TOrderTable  m_oOrderTable;   // 0x0C
	TBOOL               m_bReady;        // 0x34 set by Create
	ID3D11VertexShader* m_pVertexShader; // 0x38
	ID3D11PixelShader*  m_pPixelShader;  // 0x3C
	ID3D11InputLayout*  m_pInputLayout;  // 0x40
};                                       // 0x44
