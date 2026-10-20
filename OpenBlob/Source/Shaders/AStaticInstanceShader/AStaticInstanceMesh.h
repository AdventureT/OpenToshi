#pragma once
#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>

//-----------------------------------------------------------------------------
// Purpose: the mesh of AStaticInstanceShader (can't be created at runtime): a vertex pool of m_uiMaxVertices vertices
// and m_uiNumInstances instances (a render resource each).
//-----------------------------------------------------------------------------
class AStaticInstanceMesh : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( AStaticInstanceMesh, Toshi::TMesh );

	// An instance of the mesh (0xA0 bytes)
	struct Instance
	{
		// $deBlob: FUNCTION 00603bd0
		Instance()
		{
			m_uiUnk00   = 0;
			m_uiUnk04   = 0;
			m_pResource = TNULL;
			m_uiUnk24   = 0;
		}

		~Instance();

		TUINT32           m_uiUnk00;        // 0x00
		TUINT32           m_uiUnk04;        // 0x04
		Toshi::TResource* m_pResource;      // 0x08
		TUINT32           m_auiUnk0C[ 6 ];  // 0x0C
		TUINT32           m_uiUnk24;        // 0x24
		TUINT32           m_auiUnk28[ 30 ]; // 0x28
	};

public:
	AStaticInstanceMesh();
	~AStaticInstanceMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() override;   // +0x0C
	virtual void  Invalidate() override; // +0x10
	virtual void  OnDestroy() override;  // +0x1C

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT a_uiNumInstances ); // +0x20
	virtual TBOOL Lock( Toshi::TVertexPoolResourceInterface::LockBuffer* a_pLockBuffer );       // +0x24
	virtual void  Unlock( TUINT32 a_uiNumVertices );                                            // +0x28
	// $deBlob: FUNCTION 00603990
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool() { return m_pVertexPool; } // +0x2C

private:
	TBOOL CreatePools();
	void  DestroyPools();

private:
	TUINT32                              m_uiFlags;        // 0x10 FLAGS_LOCKED
	TUINT16                              m_uiMaxVertices;  // 0x14
	TUINT16                              m_uiNumInstances; // 0x16
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;    // 0x18
	Instance*                            m_pInstances;     // 0x1C m_uiNumInstances
	TUINT32                              m_uiUnk20;        // 0x20
};
