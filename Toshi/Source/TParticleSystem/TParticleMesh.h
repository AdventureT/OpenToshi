#pragma once
#include "Shaders/SysShader/TSysShader.h"
#include "Math/TVector4.h"
#include "Math/TMathInline.h"


TOSHI_NAMESPACE_START

class TMaterial;

// A locked TSysMesh the particles of one material are written to (0x40 bytes), one per particle material in
// TParticleSystem::m_pLockedMeshes
class TParticleMesh
{
public:
	// A vertex of the system shader (0x18 bytes)
	struct Vertex
	{
		TFLOAT  x, y, z; // 0x00
		TUINT32 uiColour; // 0x0C
		TFLOAT  u, v;     // 0x10
	};

public:
	TParticleMesh()
	{
		TUtil::MemClear( this, sizeof( *this ) );
	}

	void Lock( TSysMesh* a_pMesh, TMaterial* a_pMaterial );
	void Unlock( TUINT a_uiNumVerts, TUINT a_uiNumIndices );
	void ForceUnlock();

	TBOOL     IsLocked() const { return m_pMesh != TNULL; }
	// $deBlob: FUNCTION 006032d0
	TSysMesh* GetMesh() const { return m_pMesh; }

	TUINT16 GetNumUsedVerts() const { return m_uiUsedVerts; }
	TUINT16 GetNumUsedIndices() const { return m_uiUsedIndicies; }

	// $deBlob: FUNCTION 00710230
	TUINT GetMaxVertices() const
	{
		TASSERT( TNULL != m_pMesh );
		return m_pMesh->GetMaxVertices() - m_uiUsedVerts;
	}

	// $deBlob: FUNCTION 00718a50
	void SetIndex( TUINT16 a_uiIndex, TUINT16 a_uiIndexVal )
	{
		TASSERT( IsLocked() );
		TASSERT( a_uiIndex + m_uiUsedIndicies <= m_pMesh->GetMaxIndices() );
		TASSERT( a_uiIndexVal + m_uiUsedVerts <= m_pMesh->GetMaxVertices() );
		m_pIndices[ a_uiIndex ] = m_uiUsedVerts + a_uiIndexVal;
	}

	// $deBlob: FUNCTION 00712bd0
	void SetPosition( TUINT a_uiVertIndex, const TVector4& a_rPosition, const TVector4& a_rOffset )
	{
		TASSERT( IsLocked() );
		TASSERT( a_uiVertIndex + m_uiUsedVerts <= m_pMesh->GetMaxVertices() );
		m_pVertices[ a_uiVertIndex ].x = a_rPosition.x + a_rOffset.x;
		m_pVertices[ a_uiVertIndex ].y = a_rPosition.y + a_rOffset.y;
		m_pVertices[ a_uiVertIndex ].z = a_rPosition.z + a_rOffset.z;
	}

	// $deBlob: FUNCTION 00712ae0
	void SetColour( TUINT a_uiVertIndex, TUINT32 a_uiColour )
	{
		TASSERT( IsLocked() );
		TASSERT( a_uiVertIndex + m_uiUsedVerts <= m_pMesh->GetMaxVertices() );
		m_pVertices[ a_uiVertIndex ].uiColour = a_uiColour;
	}

	// $deBlob: FUNCTION 007197e0
	void SetUV( TUINT a_uiVertIndex, const TFLOAT* a_pUV )
	{
		TASSERT( IsLocked() );
		TASSERT( a_uiVertIndex + m_uiUsedVerts <= m_pMesh->GetMaxVertices() );
		m_pVertices[ a_uiVertIndex ].u = a_pUV[ 0 ];
		m_pVertices[ a_uiVertIndex ].v = a_pUV[ 1 ];
	}

	// $deBlob: FUNCTION 00712c90
	void SetUV( TUINT a_uiVertIndex, TFLOAT a_fU, TFLOAT a_fV )
	{
		TASSERT( IsLocked() );
		TASSERT( a_uiVertIndex + m_uiUsedVerts <= m_pMesh->GetMaxVertices() );
		m_pVertices[ a_uiVertIndex ].u = a_fU;
		m_pVertices[ a_uiVertIndex ].v = a_fV;
	}

	// $deBlob: FUNCTION 007189f0
	void SetMeshFlags( TSysMesh::FLAGS a_uiFlags, TBOOL a_bSet )
	{
		TASSERT( IsLocked() );
		m_pMesh->SetFlags( a_uiFlags, a_bSet );
	}

	Vertex*   GetVertices() const { return m_pVertices; }
	TUINT16*  GetIndices() const { return m_pIndices; }

public:
	TSysMesh*              m_pMesh;           // 0x00
	TUINT16                m_uiUsedVerts;     // 0x04
	TUINT16                m_uiUsedIndicies;  // 0x06
	TSysMesh::TLockBuffer  m_oLockBuffer;     // 0x08
	TUINT16*               m_pIndices;        // 0x38
	Vertex*                m_pVertices;       // 0x3C
};

TSTATICASSERT( sizeof( TParticleMesh ) == 0x40 );

TOSHI_NAMESPACE_END
