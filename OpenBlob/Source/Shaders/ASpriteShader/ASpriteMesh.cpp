#include "pch.h"
#include "ASpriteMesh.h"
#include "ASpriteShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00602aa0
// $deBlob: FUNCTION 00602ab0
// $deBlob: FUNCTION 00602ac0
// $deBlob: FUNCTION 00602ad0
// $deBlob: FUNCTION 00602ae0
// $deBlob: FUNCTION 0053ced0
// $deBlob: FUNCTION 00858730
TDEFINE_CLASS_NORUNTIME( ASpriteMesh );

// $deBlob: FUNCTION 00602d00
ASpriteMesh::ASpriteMesh()
{
	m_uiFlags       = 0;
	m_uiNumVertices = 0;
	m_uiNumIndices  = 0;
	m_pVertexPool   = TNULL;
	m_pIndexPool    = TNULL;
}

// $deBlob: FUNCTION 00602d30
// $deBlob: FUNCTION 006030c0
ASpriteMesh::~ASpriteMesh()
{
}

// $deBlob: FUNCTION 00602af0
TBOOL ASpriteMesh::Validate()
{
	if ( !IsValidated() )
	{
		TASSERT( ( TNULL != m_pVertexPool ) && ( TNULL != m_pIndexPool ) );

		if ( m_pVertexPool == TNULL || m_pIndexPool == TNULL )
			return TFALSE;

		m_pVertexPool->Validate();
		m_pIndexPool->Validate();
		SetFlags( State_Validated, TTRUE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00602b60
void ASpriteMesh::Invalidate()
{
	if ( IsValidated() )
		SetFlags( State_Validated, TFALSE );
}

// $deBlob: FUNCTION 00602b80
TBOOL ASpriteMesh::Lock( ASpriteMeshLockBuffer* a_pLockBuffer )
{
	TASSERT( !( m_uiFlags & FLAGS_LOCKED ) );

	ASpriteShader* pShader = TSTATICCAST( ASpriteShader, m_pOwnerShader );

	m_uiFlags |= FLAGS_LOCKED;

	// The mesh starts at the current vertex and index of the shader
	m_uiStartVertex = TMath::Int32ToUInt16( pShader->m_iNumVertices );
	m_uiStartIndex  = TMath::Int32ToUInt16( pShader->m_iNumIndices );

	return TTRUE;
}

// $deBlob: FUNCTION 00602c60
void ASpriteMesh::Unlock( TUINT16 a_uiNumVertices, TUINT16 a_uiNumIndices )
{
	TASSERT( m_uiFlags & FLAGS_LOCKED );

	m_uiNumVertices      = a_uiNumVertices - m_uiStartVertex;
	TUINT16 uiNumIndices = a_uiNumIndices - m_uiStartIndex;
	m_uiFlags &= ~FLAGS_LOCKED;
	m_uiNumIndices = uiNumIndices;
}
