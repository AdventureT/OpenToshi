#pragma once
#include "Render/AMesh.h"

#include <Toshi/TSingleton.h>
#include <Render/TShader.h>
#include <Render/TMaterial.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Purpose: the debug line shader (singleton): collects coloured lines and triangles and draws them in one go
// (RenderBuffers). Abstract: ALineShaderHAL is the DX11 version (ALineShader_DX11.cpp).
// NOTE: retail never creates it: only its TClass factory references ALineShaderHAL's constructor and nothing looks the
// class up by name, so de Blob PC draws no debug lines.
//-----------------------------------------------------------------------------
class ALineShader
    : public Toshi::TShader
    , public Toshi::TSingleton<ALineShader>
{
public:
	TDECLARE_CLASS( ALineShader, Toshi::TShader );

	ALineShader();
	virtual ~ALineShader();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual TBOOL Create() OVERRIDE; // +0x1C

	// $deBlob: FUNCTION 0055f400
	virtual const TCHAR* GetShaderPrefix() OVERRIDE { return ""; } // +0x28

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName ) = 0; // +0x38
	virtual AMesh*            CreateMesh( const TCHAR* a_szName )     = 0; // +0x3C

	// Adds a line unless the shader is disabled; asserts instead of drawing when the buffer is full
	virtual void AddLineDirect( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour ); // +0x40

	// Draws the collected triangles and lines
	virtual void RenderBuffers() = 0; // +0x44

	// Adds a line (draws the collected ones first when the buffer is full)
	virtual void AddLine( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour ) = 0; // +0x48

	// Adds a triangle (draws the collected ones first when the buffer is full)
	virtual void AddTriangle( const Toshi::TVector4& a_rVertex1, const Toshi::TVector4& a_rVertex2, const Toshi::TVector4& a_rVertex3, TUINT32 a_uiColour ) = 0; // +0x4C
	virtual void AddTriangleArray( const Toshi::TVector3* a_pVertices, TUINT32 a_uiColour )                                                                 = 0; // +0x50 (three vertices)

	// Forgets the collected lines and triangles
	virtual void Reset(); // +0x54

	// $deBlob: FUNCTION 0055f4a0
	virtual void SetUnk10( TFLOAT a_fUnk ) { m_fUnk10 = a_fUnk; } // +0x58

	// $deBlob: FUNCTION 0055f410
	virtual TFLOAT GetUnk10() const { return m_fUnk10; } // +0x5C

	// The twelve edges of a box (a_rExtents: the half sizes along the axes, rotated by a_pRotation if given)
	void AddBox( const Toshi::TVector4& a_rCentre, const Toshi::TVector4& a_rExtents, TUINT32 a_uiColour, const Toshi::TMatrix44* a_pRotation );

	// The six sides of a box as twelve triangles
	void AddSolidBox( const Toshi::TVector4& a_rCentre, const Toshi::TVector4& a_rExtents, TUINT32 a_uiColour, const Toshi::TMatrix44* a_pRotation );

	// Non-virtual forwarders
	void DrawTriangle( const Toshi::TVector4& a_rVertex1, const Toshi::TVector4& a_rVertex2, const Toshi::TVector4& a_rVertex3, TUINT32 a_uiColour );
	void DrawLine( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo, TUINT32 a_uiFromColour, TUINT32 a_uiToColour );

protected:
	TBOOL  m_bEnabled; // 0x0C (TTRUE at creation)
	TFLOAT m_fUnk10;   // 0x10 (0.0f at creation)
};                     // 0x14

//-----------------------------------------------------------------------------
// Purpose: the material of the line shader (ALineShader.cpp, nothing beyond TMaterial). Never created
// (ALineShaderHAL::CreateMaterial returns TNULL).
//-----------------------------------------------------------------------------
class ALineMaterial
    : public Toshi::TMaterial
{
public:
	TDECLARE_CLASS( ALineMaterial, Toshi::TMaterial );

	ALineMaterial();
	virtual ~ALineMaterial();
}; // 0x54

//-----------------------------------------------------------------------------
// Purpose: the mesh of the line shader (ALineShader.cpp, nothing beyond AMesh). Never created
// (ALineShaderHAL::CreateMesh returns TNULL).
//-----------------------------------------------------------------------------
class ALineMesh
    : public AMesh
{
public:
	TDECLARE_CLASS( ALineMesh, AMesh );

	ALineMesh();
	virtual ~ALineMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0055f460
	virtual TBOOL Render() OVERRIDE { return TTRUE; } // +0x18
};                                                    // 0x18
