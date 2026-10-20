#pragma once
#include "ASpriteShader.h"

#include <d3d11.h>

//-----------------------------------------------------------------------------
// Purpose: the DX11 version of ASpriteShader. Creates the LineMat and FillMat materials and the mesh pool; its drawing
// virtuals are empty in retail.
//-----------------------------------------------------------------------------
class ASpriteShaderHAL
    : public ASpriteShader
{
public:
	TDECLARE_CLASS( ASpriteShaderHAL, ASpriteShader );

	ASpriteShaderHAL();
	virtual ~ASpriteShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy() OVERRIDE;                                     // +0x0C
	virtual void  Flush() OVERRIDE;                                         // +0x10
	virtual void  StartFlush() OVERRIDE;                                    // +0x14
	virtual void  EndFlush() OVERRIDE;                                      // +0x18
	virtual TBOOL Create() OVERRIDE;                                        // +0x1C
	virtual TBOOL Validate() OVERRIDE;                                      // +0x20
	virtual void  Invalidate() OVERRIDE;                                    // +0x24
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) OVERRIDE; // +0x34

	//-----------------------------------------------------------------------------
	// ASpriteShader
	//-----------------------------------------------------------------------------
	virtual Toshi::TMaterial* CreateMaterial( const TCHAR* a_szName ) OVERRIDE;                                           // +0x38
	virtual ASpriteMesh*      CreateMeshPool( TUINT32 a_uiNumMeshes, TUINT32* a_puiStride ) OVERRIDE;                     // +0x3C
	virtual ASpriteMesh*      CreateMesh( TINT a_iUnk ) OVERRIDE;                                                         // +0x40
	virtual void              Unknown44( TINT a_iUnk ) OVERRIDE;                                                          // +0x44
	virtual void              Unknown48( TINT a_iUnk ) OVERRIDE;                                                          // +0x48
	virtual void              Unknown4C( TINT a_iUnk ) OVERRIDE;                                                          // +0x4C
	virtual void              Unknown50( TINT a_iUnk ) OVERRIDE;                                                          // +0x50
	virtual void              Unknown54( TINT a_iUnk ) OVERRIDE;                                                          // +0x54
	virtual void              Unknown5C( TINT a_iUnk1, TINT a_iUnk2 ) OVERRIDE;                                           // +0x5C
	virtual void              Unknown60( TINT a_iUnk1, TINT a_iUnk2 ) OVERRIDE;                                           // +0x60
	virtual void              Unknown64( TINT a_iUnk ) OVERRIDE;                                                          // +0x64
	virtual void              Unknown68( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 ) OVERRIDE;                             // +0x68
	virtual void              Unknown6C( TINT a_iUnk ) OVERRIDE;                                                          // +0x6C
	virtual void              SetMaterial( ASpriteMaterial* a_pMaterial ) OVERRIDE;                                       // +0x70
	virtual void              Unknown78( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4 ) OVERRIDE;               // +0x78
	virtual void              UnknownA4( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5 ) OVERRIDE; // +0xA4
	virtual void              UnknownA8( TINT a_iUnk1, TINT a_iUnk2 ) OVERRIDE;                                           // +0xA8
	virtual void              BeginMeshGeneration() OVERRIDE;                                                             // +0xAC
	virtual void              EndMeshGeneration() OVERRIDE;                                                               // +0xB0
};
