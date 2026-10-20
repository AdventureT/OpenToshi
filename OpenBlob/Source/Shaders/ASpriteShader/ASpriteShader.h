#pragma once
#include "ASpriteMesh.h"
#include "ASpriteMaterial.h"

#include <Toshi/TSingleton.h>
#include <Toshi/TNodeList.h>
#include <Render/TShader.h>
#include <Math/TMatrix44.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: a node of the sprite shader's mesh list. Retail: TNodeList<TNodeWrapper<ASpriteMesh>>.
//-----------------------------------------------------------------------------
class ASpriteMeshNode
    : public Toshi::TNodeList<ASpriteMeshNode>::TNode
{
public:
	// $deBlob: FUNCTION 00602d60
	ASpriteMeshNode( ASpriteMesh* a_pMesh )
	    : m_pMesh( a_pMesh )
	{}

	// $deBlob: FUNCTION 00602fa0
	ASpriteMesh* GetMesh() const { return m_pMesh; }

private:
	ASpriteMesh* m_pMesh; // 0x0C
};                        // 0x10

//-----------------------------------------------------------------------------
// Purpose: one vertex of the sprite shader.
//-----------------------------------------------------------------------------
struct ASpriteVertex
{
	TFLOAT  fX;       // 0x00
	TFLOAT  fY;       // 0x04
	TFLOAT  fZ;       // 0x08
	TUINT32 uiColour; // 0x0C
	TFLOAT  fU;       // 0x10
	TFLOAT  fV;       // 0x14
};                    // 0x18

//-----------------------------------------------------------------------------
// Purpose: the 2D sprite shader (singleton). It owns a pool of 256 sprite meshes (a list of nodes, more are added when
// they run out), two materials (line and fill) and generates triangles, rectangles, strips and lines into the current
// mesh. Abstract: ASpriteShaderHAL is the DX11 version.
// NOTE: retail creates the shader but never generates anything with it (its DX11 virtuals are empty).
//-----------------------------------------------------------------------------
class ASpriteShader
    : public Toshi::TShader
    , public Toshi::TSingleton<ASpriteShader>
{
	friend class ASpriteMesh;

public:
	TDECLARE_CLASS( ASpriteShader, Toshi::TShader );

	using MeshList = Toshi::TNodeList<ASpriteMeshNode>;

	ASpriteShader();
	virtual ~ASpriteShader();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void         OnDestroy() OVERRIDE;       // +0x0C
	virtual TBOOL        Create() OVERRIDE;          // +0x1C
	virtual const TCHAR* GetShaderPrefix() OVERRIDE; // +0x28

	//-----------------------------------------------------------------------------
	// Own methods (abstract in the base, DX11 in ASpriteShaderHAL)
	// NOTE: MSVC puts overloaded virtuals in reverse declaration order: the float overloads are declared first so that
	// the TVector2 ones get the lower slots, as in retail.
	//-----------------------------------------------------------------------------
	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName )                       = 0;                                                                                                                                                          // +0x38
	virtual ASpriteMesh*      CreateMeshPool( TUINT32 a_uiNumMeshes, TUINT32* a_puiStride ) = 0;                                                                                                                                                          // +0x3C
	virtual ASpriteMesh*      CreateMesh( TINT a_iUnk )                                     = 0;                                                                                                                                                          // +0x40
	virtual void              Unknown44( TINT a_iUnk )                                      = 0;                                                                                                                                                          // +0x44
	virtual void              Unknown48( TINT a_iUnk )                                      = 0;                                                                                                                                                          // +0x48
	virtual void              Unknown4C( TINT a_iUnk )                                      = 0;                                                                                                                                                          // +0x4C
	virtual void              Unknown50( TINT a_iUnk )                                      = 0;                                                                                                                                                          // +0x50
	virtual void              Unknown54( TINT a_iUnk )                                      = 0;                                                                                                                                                          // +0x54
	virtual TUINT8            SetTransformEnabled( TINT a_iEnable );                                                                                                                                                                                       // +0x58
	virtual void              Unknown5C( TINT a_iUnk1, TINT a_iUnk2 )               = 0;                                                                                                                                                                  // +0x5C
	virtual void              Unknown60( TINT a_iUnk1, TINT a_iUnk2 )               = 0;                                                                                                                                                                  // +0x60
	virtual void              Unknown64( TINT a_iUnk )                              = 0;                                                                                                                                                                  // +0x64
	virtual void              Unknown68( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 ) = 0;                                                                                                                                                                  // +0x68
	virtual void              Unknown6C( TINT a_iUnk )                              = 0;                                                                                                                                                                  // +0x6C
	virtual void              SetMaterial( ASpriteMaterial* a_pMaterial );                                                                                                                                                                                // +0x70
	virtual void              SetColour( TUINT32 a_uiColour );                                                                                                                                                                                            // +0x74
	virtual void              Unknown78( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4 ) = 0;                                                                                                                                                    // +0x78
	virtual void              SetTransform( const Toshi::TMatrix44& a_rMatrix );                                                                                                                                                                          // +0x7C
	virtual void              RenderTriangle( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fX3, TFLOAT a_fY3, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2, TFLOAT a_fU3, TFLOAT a_fV3 );                      // +0x84
	virtual void              RenderTriangle( const Toshi::TVector2& a_rPos1, const Toshi::TVector2& a_rUV1, const Toshi::TVector2& a_rPos2, const Toshi::TVector2& a_rUV2, const Toshi::TVector2& a_rPos3, const Toshi::TVector2& a_rUV3, TFLOAT a_fZ ); // +0x80
	virtual void              RenderRectangle( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ, TFLOAT a_fU1, TFLOAT a_fV1, TFLOAT a_fU2, TFLOAT a_fV2 );                                                                             // +0x8C
	virtual void              RenderRectangle( const Toshi::TVector2& a_rPos1, const Toshi::TVector2& a_rUV1, const Toshi::TVector2& a_rPos2, const Toshi::TVector2& a_rUV2, TFLOAT a_fZ );                                                               // +0x88
	virtual void              RenderTriStrip( Toshi::TVector2* a_pPositions, Toshi::TVector2* a_pUVs, TINT a_iNumVertices );                                                                                                                              // +0x90
	virtual void              RenderLine( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ );                                                                                                                                          // +0x98
	virtual void              RenderLine( const Toshi::TVector2& a_rFrom, const Toshi::TVector2& a_rTo, TFLOAT a_fZ );                                                                                                                                    // +0x94
	virtual void              RenderOutlineRectangle( const Toshi::TVector2& a_rMin, const Toshi::TVector2& a_rMax, TFLOAT a_fZ );                                                                                                                        // +0x9C
	virtual void              RenderFilledRectangle( const Toshi::TVector2& a_rMin, const Toshi::TVector2& a_rMax, TFLOAT a_fZ );                                                                                                                         // +0xA0
	virtual void              UnknownA4( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5 ) = 0;                                                                                                                                      // +0xA4
	virtual void              UnknownA8( TINT a_iUnk1, TINT a_iUnk2 )                                           = 0;                                                                                                                                      // +0xA8
	virtual void              BeginMeshGeneration();                                                                                                                                                                                                      // +0xAC
	virtual void              EndMeshGeneration();                                                                                                                                                                                                        // +0xB0

private:
	void         RenderLineImpl( TFLOAT a_fX1, TFLOAT a_fY1, TFLOAT a_fX2, TFLOAT a_fY2, TFLOAT a_fZ );
	ASpriteMesh* AllocateMesh( TINT a_iNumVertices, TINT a_iNumIndices );
	void         AdvanceMeshList();
	void         AddMesh();
	ASpriteMesh* GetMesh();

protected:
	TUINT8                m_uiFlags;                // 0x0C (bit 0x20: the transform is enabled)
	TINT                  m_aiRenderState[ 6 ];     // 0x10 (the defaults 4, 1.0f, 4, 5, 0, 4)
	ASpriteMaterial*      m_pMaterial;              // 0x28
	TUINT32               m_uiColour;               // 0x2C (-1 at creation)
	Toshi::TVector2       m_vUnk30;                 // 0x30 (NOTE: retail copies an uninitialised TVector2 here)
	Toshi::TVector2       m_vUnk38;                 // 0x38 (NOTE: retail copies an uninitialised TVector2 here)
	Toshi::TMatrix44      m_mTransform;             // 0x40
	TBOOL                 m_bTransformChanged;      // 0x80 (SetTransform; the next mesh is started)
	MeshList              m_lMeshes;                // 0x84
	MeshList::Iterator    m_itCurrentMesh;          // 0x94
	ASpriteMeshLockBuffer m_oLockBuffer;            // 0x98
	TINT                  m_iNumIndices;            // 0xC8 (the next index of the generation)
	TINT                  m_iNumVertices;           // 0xCC (the next vertex of the generation)
	ASpriteMaterial*      m_pLineMaterial;          // 0xD0 ("LineMat")
	ASpriteMaterial*      m_pFillMaterial;          // 0xD4 ("FillMat")
	TUINT32               m_uiMeshFlags;            // 0xD8 (100 at creation)
	TUINT16               m_uiMaxVertices;          // 0xDC (0x1800)
	TUINT16               m_uiMaxIndices;           // 0xDE (0x2400)
	ASpriteMesh*          m_pInitialMeshAllocation; // 0xE0 (256 meshes of the stride of ASpriteMeshHAL)
};                                                  // 0xE4

TSTATICASSERT( sizeof( ASpriteShader ) == 0xE4 );
