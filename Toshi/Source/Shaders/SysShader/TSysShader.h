#pragma once
#include "Render/TShader.h"
#include "Render/TMesh.h"
#include "Render/TMaterial.h"
#include "Render/TVertexPoolResourceInterface.h"
#include "Render/TIndexPoolResourceInterface.h"
#include "Toshi/TSingleton.h"

TOSHI_NAMESPACE_START

class TResource;

//-----------------------------------------------------------------------------
// Purpose: a material of the system shader: a texture and one of the blend modes the platform half turns into
// order tables and render states.
//-----------------------------------------------------------------------------
class TSysMaterial : public TMaterial
{
public:
	TDECLARE_CLASS( TSysMaterial, TMaterial );

public:
	using BLENDMODE = TUINT32;
	enum BLENDMODE_ : BLENDMODE
	{
		BLENDMODE_DEFAULT,
		BLENDMODE_1,
		BLENDMODE_2,
		BLENDMODE_ADDITIVE, // 3: src alpha + one
		BLENDMODE_4,
		BLENDMODE_ALPHATEST, // 5: alpha test at 0.5
		BLENDMODE_6,         // 6/7: additive, in their own order tables
		BLENDMODE_7,
	};

public:
	TSysMaterial();
	~TSysMaterial();

	//-----------------------------------------------------------------------------
	// TMaterial
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 0070a730
	virtual TBOOL Create() OVERRIDE { return TMaterial::Create(); } // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode );       // +0x1C
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode ); // +0x20

	// Sets FLAGS_ALPHATEXTURE (the texture is only used for the alpha)
	// $deBlob: FUNCTION 0070aa20
	void SetAlphaTexture() { m_Flags |= FLAGS_ALPHATEXTURE; }

	// $deBlob: FUNCTION 0070a770
	BLENDMODE GetBlendMode() const { return m_eBlendMode; }

protected:
	BLENDMODE m_eBlendMode; // 0x54
}; // 0x58

//-----------------------------------------------------------------------------
// Purpose: a dynamic mesh of the system shader (position, colour, UV vertices and 16 bit indices) the caller
// locks and fills.
//-----------------------------------------------------------------------------
class TSysMesh : public TMesh
{
public:
	TDECLARE_CLASS( TSysMesh, TMesh );

public:
	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE                    = 0,
		FLAGS_VERTICES_STATIC         = BITFLAG( 0 ),
		FLAGS_VERTICES_DYNAMIC        = BITFLAG( 1 ),
		FLAGS_VERTICES_DYNAMIC_APPEND = BITFLAG( 2 ),
		FLAGS_INDICES_STATIC          = BITFLAG( 3 ),
		FLAGS_INDICES_DYNAMIC         = BITFLAG( 4 ),
		FLAGS_INDICES_DYNAMIC_APPEND  = BITFLAG( 5 ),
		FLAGS_UNK6                    = BITFLAG( 6 ), // selects the flags of the index pool
		FLAGS_LOCKED                  = 0x8000,
	};

	// What Lock returns: the vertices and the indices
	struct TLockBuffer
	{
		// $deBlob: FUNCTION 005fcef0
		TLockBuffer()
		{
			oIndices.uiOffset = 0;
			oIndices.pBuffer  = TNULL;
		}

		TVertexPoolResourceInterface::LockBuffer oVertices; // 0x00 (apStreams[ 0 ] is the vertex data)
		TIndexPoolResourceInterface::LockBuffer  oIndices;  // 0x28
	}; // 0x30

public:
	TSysMesh();

	// $deBlob: FUNCTION 0070adc0
	// $deBlob: FUNCTION 0070b070
	~TSysMesh() {}

	//-----------------------------------------------------------------------------
	// TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() OVERRIDE;   // +0x0C
	virtual void  Invalidate() OVERRIDE; // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL      Create( FLAGS a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices ); // +0x20
	virtual TBOOL      Lock( TLockBuffer* a_pLockBuffer );                                         // +0x24
	virtual void       Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices );                   // +0x28
	virtual TResource* GetVertexPool();                                                            // +0x2C
	virtual TResource* GetIndexPool();                                                             // +0x30
	virtual void       SetZBias( TINT a_iZBias ) = 0;                                              // +0x34
	// $deBlob: FUNCTION 0070a810
	virtual void       SetAlphaRef() {} // +0x38

	using TMesh::Create;

	FLAGS GetFlags() const { return m_uiFlags; }

	// $deBlob: FUNCTION 007197c0
	void SetFlags( FLAGS a_uiFlags, TBOOL a_bSet )
	{
		if ( a_bSet )
			m_uiFlags |= a_uiFlags;
		else
			m_uiFlags &= ~a_uiFlags;
	}

	TUINT16 GetMaxVertices() const { return m_uiMaxVertices; }
	TUINT16 GetMaxIndices() const { return m_uiMaxIndices; }

	// $deBlob: FUNCTION 0070ad70
	TBOOL SetUnk20( TBOOL a_bValue )
	{
		TBOOL bOld = m_bUnk20;
		m_bUnk20   = a_bValue;
		return bOld;
	}

protected:
	TBOOL Allocate();
	void  Deallocate();

protected:
	FLAGS      m_uiFlags;       // 0x10
	TUINT16    m_uiMaxVertices; // 0x14
	TUINT16    m_uiMaxIndices;  // 0x16
	TResource* m_pVertexPool;   // 0x18
	TResource* m_pIndexPool;    // 0x1C
	TBOOL      m_bUnk20;        // 0x20
	TBOOL      m_bUnk21;        // 0x21
}; // 0x24

//-----------------------------------------------------------------------------
// Purpose: the system shader: simple coloured/textured triangle strips (particles, debug meshes, the blob's trail...),
// abstract (TSysShaderHAL is the DX11 half).
//-----------------------------------------------------------------------------
class TSysShader
    : public TShader
    , public TSingleton<TSysShader>
{
public:
	TDECLARE_CLASS( TSysShader, TShader );

public:
	TSysShader();
	~TSysShader();

	//-----------------------------------------------------------------------------
	// TShader
	//-----------------------------------------------------------------------------
	virtual const TCHAR* GetShaderPrefix() OVERRIDE; // +0x28

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TSysMaterial* CreateMaterial( const TCHAR* a_szName ) = 0; // +0x38
	virtual TSysMesh*     CreateMesh( const TCHAR* a_szName )     = 0; // +0x3C
	// $deBlob: FUNCTION 0070a800
	virtual TBOOL         IsLightScattering() { return TFALSE; } // +0x40
	// $deBlob: FUNCTION 0070a820
	virtual void          SetLightScattering( TBOOL a_bEnable ) {} // +0x44
};

TOSHI_NAMESPACE_END
