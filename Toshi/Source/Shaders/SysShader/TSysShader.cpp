#include "ToshiPCH.h"
#include "TSysShader.h"
#include "Render/TRender.h"
#include "Render/TVertexFactoryResourceInterface.h"
#include "Render/TIndexFactoryResourceInterface.h"
#include "Math/TMathInline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// TSysShader
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0070a8c0
// $deBlob: FUNCTION 0070a8d0
// $deBlob: FUNCTION 0070a8e0
// $deBlob: FUNCTION 0070a8f0
// $deBlob: FUNCTION 0070a900
// $deBlob: FUNCTION 0053fcf0
// $deBlob: FUNCTION 008595a0
TDEFINE_CLASS_NORUNTIME( TSysShader );

// $deBlob: FUNCTION 0070a970
TSysShader::TSysShader()
{
}

// $deBlob: FUNCTION 0070a910
// $deBlob: FUNCTION 0070b0a0
TSysShader::~TSysShader()
{
}

// $deBlob: FUNCTION 0070a7d0
const TCHAR* TSysShader::GetShaderPrefix()
{
	return "";
}

//-----------------------------------------------------------------------------
// TSysMaterial
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0070a9c0
// $deBlob: FUNCTION 0070a9d0
// $deBlob: FUNCTION 0070a9e0
// $deBlob: FUNCTION 0070a9f0
// $deBlob: FUNCTION 0070aa00
// $deBlob: FUNCTION 0053fc70
// $deBlob: FUNCTION 00859580
TDEFINE_CLASS_NORUNTIME( TSysMaterial );

// $deBlob: FUNCTION 0070aa30
TSysMaterial::TSysMaterial()
{
	m_eBlendMode = BLENDMODE_DEFAULT;
}

// $deBlob: FUNCTION 0070aa50
// $deBlob: FUNCTION 0070b040
TSysMaterial::~TSysMaterial()
{
}

// $deBlob: FUNCTION 0070a740
TBOOL TSysMaterial::Create( BLENDMODE a_eBlendMode )
{
	return TMaterial::Create();
}

// $deBlob: FUNCTION 0070aa10
void TSysMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	m_eBlendMode = a_eBlendMode;
}

//-----------------------------------------------------------------------------
// TSysMesh
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0070aa60
// $deBlob: FUNCTION 0070aa70
// $deBlob: FUNCTION 0070aa80
// $deBlob: FUNCTION 0070aa90
// $deBlob: FUNCTION 0070aaa0
// $deBlob: FUNCTION 0053fcb0
// $deBlob: FUNCTION 00859590
TDEFINE_CLASS_NORUNTIME( TSysMesh );

// $deBlob: FUNCTION 0070ad80
TSysMesh::TSysMesh()
{
	m_uiMaxVertices = 0;
	m_uiMaxIndices  = 0;
	m_pVertexPool   = TNULL;
	m_pIndexPool    = TNULL;
	m_bUnk20        = TFALSE;
	m_bUnk21        = TFALSE;
	m_uiFlags       = 0;
}

// $deBlob: FUNCTION 0070aab0
TBOOL TSysMesh::Validate()
{
	if ( !IsValidated() )
		m_State |= State_Validated;

	return TTRUE;
}

// $deBlob: FUNCTION 0070aad0
void TSysMesh::Invalidate()
{
	if ( m_pVertexPool != TNULL && m_pIndexPool != TNULL )
		Deallocate();

	if ( IsValidated() )
		m_State &= ~State_Validated;
}

// $deBlob: FUNCTION 0070ab00
TBOOL TSysMesh::Create( FLAGS a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices )
{
	TASSERT( TFALSE == IsCreated() );

	m_uiFlags       = a_uiFlags;
	m_uiMaxVertices = a_uiMaxVertices;
	m_uiMaxIndices  = a_uiMaxIndices;

	if ( !Allocate() )
		return TFALSE;

	m_State |= State_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 0070ab70
TBOOL TSysMesh::Lock( TLockBuffer* a_pLockBuffer )
{
	TASSERT( !( m_uiFlags & FLAGS_LOCKED ) );

	if ( TSTATICCAST( TVertexPoolResourceInterface, m_pVertexPool )->Lock( &a_pLockBuffer->oVertices ) &&
	     TSTATICCAST( TIndexPoolResourceInterface, m_pIndexPool )->Lock( &a_pLockBuffer->oIndices ) )
	{
		m_uiFlags |= FLAGS_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0070abf0
void TSysMesh::Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices )
{
	TASSERT( m_uiFlags & FLAGS_LOCKED );
	TASSERT( TUINT16( a_uiNumVertices ) >= 3 );
	TASSERT( TUINT16( a_uiNumIndices ) >= 3 );

	TVertexPoolResourceInterface* pVertexPool = TSTATICCAST( TVertexPoolResourceInterface, m_pVertexPool );
	TIndexPoolResourceInterface*  pIndexPool  = TSTATICCAST( TIndexPoolResourceInterface, m_pIndexPool );

	if ( TUINT16( a_uiNumVertices ) == TUINT16( -1 ) )
	{
		a_uiNumVertices = pVertexPool->GetNumVertices();
		TASSERT( a_uiNumVertices <= TMath::TUINT16_MAX );
	}

	if ( TUINT16( a_uiNumIndices ) == TUINT16( -1 ) )
	{
		a_uiNumIndices = pIndexPool->GetNumIndices();
		TASSERT( a_uiNumIndices <= TMath::TUINT16_MAX );
	}

	pVertexPool->Unlock( TUINT16( a_uiNumVertices ) );
	pIndexPool->Unlock( TUINT16( a_uiNumIndices ) );
	m_uiFlags &= ~FLAGS_LOCKED;
}

// $deBlob: FUNCTION 0070ad50
TResource* TSysMesh::GetVertexPool()
{
	return m_pVertexPool;
}

// $deBlob: FUNCTION 0070ad60
TResource* TSysMesh::GetIndexPool()
{
	return m_pIndexPool;
}

// $deBlob: FUNCTION 0070add0
TBOOL TSysMesh::Allocate()
{
	TASSERT( 0 != m_uiMaxVertices );
	TASSERT( 3 <= m_uiMaxIndices );
	TASSERT( TNULL == m_pVertexPool );
	TASSERT( TNULL == m_pIndexPool );

	// NOTE: TSysMeshHAL::Create doesn't call this in de Blob (its own buffers), only the base Create does
	TUINT16 uiVertexFlags = 0;

	if ( m_uiFlags & FLAGS_VERTICES_STATIC )
		uiVertexFlags = TVertexPoolResourceInterface::FLAGS_STATIC;
	else if ( m_uiFlags & FLAGS_VERTICES_DYNAMIC )
		uiVertexFlags = TVertexPoolResourceInterface::FLAGS_DYNAMIC;
	else if ( m_uiFlags & FLAGS_VERTICES_DYNAMIC_APPEND )
		uiVertexFlags = TVertexPoolResourceInterface::FLAGS_DYNAMIC_APPEND;

	TVertexFactoryResourceInterface* pVertexFactory = TSTATICCAST( TVertexFactoryResourceInterface, TRender::GetSingleton()->GetSystemResource( TRender::SYSRESOURCE_VFSYSVNDUV1 ) );
	m_pVertexPool                                   = pVertexFactory->CreatePoolResource( m_uiMaxVertices, uiVertexFlags );

	TUINT8 uiIndexFlags = 0;

	if ( m_uiFlags & FLAGS_INDICES_STATIC )
		uiIndexFlags = 1;
	else if ( m_uiFlags & FLAGS_INDICES_DYNAMIC )
		uiIndexFlags = 2;
	else if ( m_uiFlags & FLAGS_INDICES_DYNAMIC_APPEND )
		uiIndexFlags = 4;

	TIndexFactoryResourceInterface* pIndexFactory = TSTATICCAST( TIndexFactoryResourceInterface, TRender::GetSingleton()->GetSystemResource( TRender::SYSRESOURCE_IFSYS ) );
	m_pIndexPool                                  = pIndexFactory->CreatePoolResource( m_uiMaxIndices, ( ( m_uiFlags & FLAGS_UNK6 ) == 0 ) * 8 + 8 | uiIndexFlags );

	return TTRUE;
}

// $deBlob: FUNCTION 0070af50
void TSysMesh::Deallocate()
{
	if ( m_pVertexPool != TNULL )
	{
		TRender::GetSingleton()->DestroyResource( m_pVertexPool );
		m_pVertexPool = TNULL;
	}

	if ( m_pIndexPool != TNULL )
	{
		TRender::GetSingleton()->DestroyResource( m_pIndexPool );
		m_pIndexPool = TNULL;
	}

	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
	m_uiMaxIndices  = 0;
}

TOSHI_NAMESPACE_END
