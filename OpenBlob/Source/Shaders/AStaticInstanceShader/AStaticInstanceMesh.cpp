#include "pch.h"
#include "AStaticInstanceMesh.h"
#include "Render/ModelLoading/AModelLoader.h"

#include <Render/TRender.h>
#include <Render/TVertexFactoryResourceInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AStaticInstanceMesh::Instance ) == 0xA0 );
TSTATICASSERT( sizeof( AStaticInstanceMesh ) == 0x24 );

// $deBlob: FUNCTION 00603710
// $deBlob: FUNCTION 00603720
// $deBlob: FUNCTION 00603730
// $deBlob: FUNCTION 00603740
// $deBlob: FUNCTION 00603750
// $deBlob: FUNCTION 0053cf90
// $deBlob: FUNCTION 00858760
TDEFINE_CLASS_NORUNTIME( AStaticInstanceMesh );

// $deBlob: FUNCTION 00603c50
// $deBlob: FUNCTION 00603c70
AStaticInstanceMesh::Instance::~Instance()
{
	if ( m_pResource != TNULL )
	{
		TRender::GetSingleton()->DestroyResource( m_pResource );
		m_pResource = TNULL;
	}
}

// $deBlob: FUNCTION 006039a0
AStaticInstanceMesh::AStaticInstanceMesh()
{
	m_pMaterial      = TNULL;
	m_uiFlags        = 0;
	m_uiMaxVertices  = 0;
	m_uiNumInstances = 0;
	m_pVertexPool    = TNULL;
	m_pInstances     = TNULL;
	m_uiUnk20        = 0;
	m_pOwnerShader   = TNULL;
	m_State          = 0;
}

// $deBlob: FUNCTION 00603760
// $deBlob: FUNCTION 00603d10
AStaticInstanceMesh::~AStaticInstanceMesh()
{
}

// $deBlob: FUNCTION 00603840
TBOOL AStaticInstanceMesh::Validate()
{
	if ( !IsValidated() )
	{
		m_pVertexPool->Validate();
		SetFlags( State_Validated, TTRUE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00603860
void AStaticInstanceMesh::Invalidate()
{
	if ( IsValidated() )
		SetFlags( State_Validated, TFALSE );
}

// $deBlob: FUNCTION 00603770
void AStaticInstanceMesh::OnDestroy()
{
	// NOTE: retail doesn't call TMesh::OnDestroy
	AModelLoader::DestroyMaterial( m_pMaterial );
	DestroyPools();
	SetFlags( State_Created, TFALSE );
}

// $deBlob: FUNCTION 00603790
TBOOL AStaticInstanceMesh::Create( TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT a_uiNumInstances )
{
	TASSERT( TFALSE == IsCreated() );

	m_uiFlags       = a_uiFlags;
	m_uiMaxVertices = a_uiMaxVertices;

	m_uiNumInstances = TMath::UInt32ToUInt16( a_uiNumInstances );

	if ( !CreatePools() )
		return TFALSE;

	SetFlags( State_Created, TTRUE );
	return TTRUE;
}

// $deBlob: FUNCTION 00603880
TBOOL AStaticInstanceMesh::Lock( TVertexPoolResourceInterface::LockBuffer* a_pLockBuffer )
{
	TASSERT( !( m_uiFlags & FLAG_LOCKED ) );

	if ( m_pVertexPool->Lock( a_pLockBuffer ) )
	{
		m_uiFlags |= FLAG_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 006038f0
void AStaticInstanceMesh::Unlock( TUINT32 a_uiNumVertices )
{
	TASSERT( m_uiFlags & FLAG_LOCKED );

	if ( TINT16( a_uiNumVertices ) == -1 )
	{
		a_uiNumVertices = TMath::UInt32ToUInt16( m_pVertexPool->GetNumVertices() );
	}

	m_pVertexPool->Unlock( TUINT16( a_uiNumVertices ) );
	m_uiFlags &= ~FLAG_LOCKED;
}

// $deBlob: FUNCTION 006039e0
TBOOL AStaticInstanceMesh::CreatePools()
{
	TASSERT( 0 != m_uiMaxVertices );
	TASSERT( TNULL == m_pVertexPool );

	TVertexFactoryResourceInterface* pFactory = TSTATICCAST( TVertexFactoryResourceInterface, TRender::GetSingleton()->GetSystemResource( TRender::SYSRESOURCE_VFWORLD ) );
	m_pVertexPool                             = pFactory->CreatePoolResource( m_uiMaxVertices, 1 );
	m_pInstances                              = new Instance[ m_uiNumInstances ];

	return TTRUE;
}

// $deBlob: FUNCTION 00603b20
void AStaticInstanceMesh::DestroyPools()
{
	if ( m_pVertexPool != TNULL )
	{
		TRender::GetSingleton()->DestroyResource( m_pVertexPool );
		m_pVertexPool = TNULL;
	}

	if ( m_pInstances != TNULL )
		delete[] m_pInstances;

	m_pInstances    = TNULL;
	m_uiFlags       = 0;
	m_uiMaxVertices = 0;
}
