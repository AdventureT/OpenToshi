#include "ToshiPCH.h"
#include "TParticleMesh.h"
#include "Render/TMaterial.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00712330
void TParticleMesh::Lock( TSysMesh* a_pMesh, TMaterial* a_pMaterial )
{
	TASSERT( m_pMesh == TNULL );
	TASSERT( a_pMesh != TNULL );
	TASSERT( a_pMaterial != TNULL );

	m_pMesh = a_pMesh;
	a_pMesh->Lock( &m_oLockBuffer );
	m_uiUsedVerts    = 0;
	m_uiUsedIndicies = 0;

	// TIndexPoolResourceInterface::LockBuffer and TVertexPoolResourceInterface::LockBuffer getters (inlined)
	TASSERT( m_oLockBuffer.oIndices.pBuffer != TNULL && "m_pLockedBuffer!=TNULL" );
	m_pIndices = m_oLockBuffer.oIndices.pBuffer;

	TASSERT( m_oLockBuffer.oVertices.uiNumStreams != 0 && "a_uiBuffer<m_uiNumStreams" );
	m_pVertices = TREINTERPRETCAST( Vertex*, m_oLockBuffer.oVertices.apStreams[ 0 ] );

	m_pMesh->SetMaterial( a_pMaterial );
}

// $deBlob: FUNCTION 00712460
void TParticleMesh::Unlock( TUINT a_uiNumVerts, TUINT a_uiNumIndices )
{
	TASSERT( a_uiNumVerts + m_uiUsedVerts <= m_pMesh->GetMaxVertices() );
	// NOTE: retail's assert adds the number of vertices to the used indices
	TASSERT( a_uiNumVerts + m_uiUsedIndicies <= m_pMesh->GetMaxIndices() );

	m_pIndices += a_uiNumIndices;
	m_pVertices += a_uiNumVerts;

	m_uiUsedVerts += TMath::UInt32ToUInt16( a_uiNumVerts );
	m_uiUsedIndicies += TMath::UInt32ToUInt16( a_uiNumIndices );

	if ( m_uiUsedVerts >= m_pMesh->GetMaxVertices() || m_uiUsedIndicies >= m_pMesh->GetMaxIndices() )
	{
		m_pMesh->Unlock( m_uiUsedVerts, m_uiUsedIndicies );
		m_pMesh->Validate();

		m_pMesh          = TNULL;
		m_uiUsedVerts    = 0;
		m_uiUsedIndicies = 0;
		m_pIndices       = TNULL;
		m_pVertices      = TNULL;
	}
}

// $deBlob: FUNCTION 007125b0
void TParticleMesh::ForceUnlock()
{
	if ( m_pMesh == TNULL )
		return;

	TASSERT( m_uiUsedVerts <= m_pMesh->GetMaxVertices() );
	TASSERT( m_uiUsedIndicies <= m_pMesh->GetMaxIndices() );

	m_pMesh->Unlock( m_uiUsedVerts, m_uiUsedIndicies );
	m_pMesh->Validate();

	m_pMesh          = TNULL;
	m_uiUsedVerts    = 0;
	m_uiUsedIndicies = 0;
	m_pIndices       = TNULL;
	m_pVertices      = TNULL;
}

TOSHI_NAMESPACE_END
