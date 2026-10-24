#pragma once
#include "ALineShader.h"

#include <d3d11.h>

//-----------------------------------------------------------------------------
// Purpose: the DX11 line shader: two dynamic vertex buffers of 64000 vertices (lines and triangles, position + colour)
// filled by the Add* methods and drawn by RenderBuffers.
// NOTE: retail's HAL classes are in the Toshi namespace; ours are global like the other game shaders. Never created in
// retail (see ALineShader.h).
//-----------------------------------------------------------------------------
class ALineShaderHAL
    : public ALineShader
{
public:
	TDECLARE_CLASS( ALineShaderHAL, ALineShader );

	static TCONSTEXPR_VAR TUINT32 MAX_VERTICES = 64000;

	struct Vertex
	{
		Vertex() {}

		TFLOAT  fX;       // 0x00
		TFLOAT  fY;       // 0x04
		TFLOAT  fZ;       // 0x08
		TUINT32 uiColour; // 0x0C
	};                    // 0x10

public:
	ALineShaderHAL();
	virtual ~ALineShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy() OVERRIDE;                                     // +0x0C
	virtual void  StartFlush() OVERRIDE;                                    // +0x14
	virtual void  EndFlush() OVERRIDE;                                      // +0x18
	virtual TBOOL Create() OVERRIDE;                                        // +0x1C
	virtual TBOOL Validate() OVERRIDE;                                      // +0x20
	virtual void  Invalidate() OVERRIDE;                                    // +0x24
	virtual TBOOL TryInvalidate() OVERRIDE;                                 // +0x2C
	virtual TBOOL TryValidate() OVERRIDE;                                   // +0x30
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) OVERRIDE; // +0x34

	//-----------------------------------------------------------------------------
	// ALineShader
	//-----------------------------------------------------------------------------
	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName ) OVERRIDE;                                                                                                    // +0x38
	virtual AMesh*            CreateMesh( const TCHAR* a_szName ) OVERRIDE;                                                                                                        // +0x3C
	virtual void              AddLineDirect( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour ) OVERRIDE;                // +0x40
	virtual void              RenderBuffers() OVERRIDE;                                                                                                                            // +0x44
	virtual void              AddLine( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour ) OVERRIDE;                      // +0x48
	virtual void              AddTriangle( const Toshi::TVector4& a_rVertex1, const Toshi::TVector4& a_rVertex2, const Toshi::TVector4& a_rVertex3, TUINT32 a_uiColour ) OVERRIDE; // +0x4C
	virtual void              AddTriangleArray( const Toshi::TVector3* a_pVertices, TUINT32 a_uiColour ) OVERRIDE;                                                                 // +0x50
	virtual void              Reset() OVERRIDE;                                                                                                                                    // +0x54

private:
	ID3D11Buffer*       m_pLineBuffer;        // 0x14
	ID3D11Buffer*       m_pTriangleBuffer;    // 0x18
	TUINT32             m_uiNumLineVerts;     // 0x1C
	TUINT32             m_uiNumTriangleVerts; // 0x20
	ID3D11VertexShader* m_pVertexShader;      // 0x24
	ID3D11PixelShader*  m_pPixelShader;       // 0x28
	ID3D11InputLayout*  m_pInputLayout;       // 0x2C
};                                            // 0x30

//-----------------------------------------------------------------------------
// Purpose: the DX11 line material (ALineShader_DX11.cpp, nothing beyond ALineMaterial).
//-----------------------------------------------------------------------------
class ALineMaterialHAL
    : public ALineMaterial
{
public:
	TDECLARE_CLASS( ALineMaterialHAL, ALineMaterial );

	ALineMaterialHAL();
	virtual ~ALineMaterialHAL();
}; // 0x54

//-----------------------------------------------------------------------------
// Purpose: the DX11 line mesh (ALineShader_DX11.cpp, nothing beyond ALineMesh).
//-----------------------------------------------------------------------------
class ALineMeshHAL
    : public ALineMesh
{
public:
	TDECLARE_CLASS( ALineMeshHAL, ALineMesh );

	ALineMeshHAL();
	virtual ~ALineMeshHAL();
}; // 0x18
