#pragma once
#include <Render/AMesh.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

//-----------------------------------------------------------------------------
// Purpose: the lock of the sprite shader's current mesh (ASpriteShader + 0x98): the locked vertex stream and index
// buffer the sprites are written to.
//-----------------------------------------------------------------------------
struct ASpriteMeshLockBuffer
{
	// $deBlob: FUNCTION 00602dd0
	ASpriteMeshLockBuffer()
	{
		oIndices.pBuffer  = TNULL;
		oIndices.uiOffset = 0;
	}

	Toshi::TVertexPoolResourceInterface::LockBuffer oVertices; // 0x00
	Toshi::TIndexPoolResourceInterface::LockBuffer  oIndices;  // 0x28
};                                                             // 0x30

TSTATICASSERT( sizeof( ASpriteMeshLockBuffer ) == 0x30 );

//-----------------------------------------------------------------------------
// Purpose: a sprite mesh: one of the 256 meshes of the sprite shader's pool. The vertices and the indices come from the
// shader's pools (Lock takes the next range, Unlock closes it).
//-----------------------------------------------------------------------------
class ASpriteMesh
    : public AMesh
{
	friend class ASpriteShaderHAL; // sets the owner shader of the meshes of the pool
	friend class ASpriteShader;    // reads the number of indices after Unlock

public:
	TDECLARE_CLASS( ASpriteMesh, AMesh );

	enum FLAGS : TUINT32
	{
		FLAGS_LOCKED = 0x8000,
	};

	ASpriteMesh();
	virtual ~ASpriteMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() OVERRIDE;   // +0x0C
	virtual void  Invalidate() OVERRIDE; // +0x10

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices ) = 0; // +0x20
	virtual TBOOL Lock( ASpriteMeshLockBuffer* a_pLockBuffer );                                     // +0x24
	virtual void  Unlock( TUINT16 a_uiNumVertices, TUINT16 a_uiNumIndices );                        // +0x28

	// $deBlob: FUNCTION 00602cc0
	Toshi::TVertexPoolResourceInterface* GetVertexPool() const { return m_pVertexPool; }
	// $deBlob: FUNCTION 00602cd0
	Toshi::TIndexPoolResourceInterface* GetIndexPool() const { return m_pIndexPool; }
	// $deBlob: FUNCTION 00602ce0
	void SetVertexPool( Toshi::TVertexPoolResourceInterface* a_pVertexPool ) { m_pVertexPool = a_pVertexPool; }
	// $deBlob: FUNCTION 00602cf0
	void SetIndexPool( Toshi::TIndexPoolResourceInterface* a_pIndexPool ) { m_pIndexPool = a_pIndexPool; }

protected:
	TUINT32                              m_uiFlags;       // 0x18 (FLAGS_LOCKED)
	TUINT16                              m_uiNumVertices; // 0x1C
	TUINT16                              m_uiNumIndices;  // 0x1E
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;   // 0x20
	Toshi::TIndexPoolResourceInterface*  m_pIndexPool;    // 0x24
	TUINT16                              m_uiStartVertex; // 0x28
	TUINT16                              m_uiStartIndex;  // 0x2A
};                                                        // 0x2C

TSTATICASSERT( sizeof( ASpriteMesh ) == 0x2C );
