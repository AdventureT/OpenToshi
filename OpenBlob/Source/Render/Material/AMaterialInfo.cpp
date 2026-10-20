#include "pch.h"
#include "AMaterialInfo.h"

#include <Render/TMaterial.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Platform/DX11/TMaterial_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005f76d0
AMaterialInfoPool::AMaterialInfoPool( TINT a_iSize, const TCHAR* a_szName )
{
	m_szName = a_szName;
	m_iSize  = a_iSize;
	TASSERT( m_iSize > 0 );

	// NOTE: retail allocates the array with TMemalign16
	m_pInfos = new AMaterialInfo[ m_iSize ];

	for ( TINT i = 0; i < m_iSize; i++ )
	{
		m_pInfos[ i ].m_pOwnerPool = this;
		m_lFree.PushBack( &m_pInfos[ i ] );
	}
}

// $deBlob: FUNCTION 005f77f0
// $deBlob: FUNCTION 0061d960
AMaterialInfoPool::~AMaterialInfoPool()
{
	TASSERT( m_lUsed.IsEmpty() );
	m_lUsed.Clear();
	m_lFree.Clear();

	if ( m_pInfos != TNULL )
		delete[] m_pInfos;

	m_pInfos = TNULL;
}

// $deBlob: FUNCTION 005f78c0
AMaterialInfo* AMaterialInfoPool::Alloc()
{
	TASSERT( !m_lFree.IsEmpty() );
	AMaterialInfo* pInfo = m_lFree.Head();
	pInfo->Remove();
	m_lUsed.PushBack( pInfo );

	return pInfo;
}

// $deBlob: FUNCTION 005f7920
void AMaterialInfoPool::Free( AMaterialInfo& a_rMatInfo )
{
	TASSERT( a_rMatInfo.m_pOwnerPool->m_lUsed.IsInList( &a_rMatInfo ) );
	a_rMatInfo.Remove();
	a_rMatInfo.m_pOwnerPool->m_lFree.PushBack( &a_rMatInfo );
}

// $deBlob: FUNCTION 005f7980
AMaterialInfo::AMaterialInfo()
{
	m_pOwnerPool = TNULL;
	m_uiUnk0C    = 0;
	m_pMaterial  = TNULL;

	for ( TINT i = 0; i < MAX_PASSES; i++ )
		m_apUVAnimations[ i ] = TNULL;
}

// $deBlob: FUNCTION 005f79c0
void AMaterialInfo::SwapLightMapPasses( TWorldMaterialHAL* a_pMat )
{
	TASSERT( a_pMat->GetTextureNum() > 1 );

	TTexture*                 pTexture = a_pMat->m_pTextures[ 0 ];
	TFLOAT                    fUOffset = a_pMat->m_afUOffset[ 0 ];
	TWorldMaterialHAL::UVMODE eUVMode0 = a_pMat->m_aUVModes[ 0 ];
	a_pMat->m_pTextures[ 0 ]           = a_pMat->m_pTextures[ 1 ];
	a_pMat->m_pTextures[ 1 ]           = pTexture;

	TUINT8 uiUVStream        = a_pMat->m_iUVStream[ 0 ];
	a_pMat->m_iUVStream[ 0 ] = a_pMat->m_iUVStream[ 1 ];
	a_pMat->m_iUVStream[ 1 ] = uiUVStream;

	TWorldMaterialHAL::MULTIPASSBLENDTYPE eBlendType = a_pMat->m_aMultiPassBlendTypes[ 0 ];
	a_pMat->m_aMultiPassBlendTypes[ 0 ]              = a_pMat->m_aMultiPassBlendTypes[ 1 ];
	a_pMat->m_afUOffset[ 0 ]                         = a_pMat->m_afUOffset[ 1 ];
	a_pMat->m_afUOffset[ 1 ]                         = fUOffset;

	TFLOAT fVOffset                     = a_pMat->m_afVOffset[ 0 ];
	a_pMat->m_afVOffset[ 0 ]            = a_pMat->m_afVOffset[ 1 ];
	a_pMat->m_aMultiPassBlendTypes[ 1 ] = eBlendType;
	a_pMat->m_afVOffset[ 1 ]            = fVOffset;

	a_pMat->SetUVMode( 0, a_pMat->m_aUVModes[ 1 ] );
	a_pMat->SetUVMode( 1, eUVMode0 );
}

// $deBlob: FUNCTION 005f7a80
void AMaterialInfo::CopyMaterial( TMaterial* a_pSource, TMaterial* a_pDest )
{
	TASSERT( a_pDest->GetClass() == a_pSource->GetClass() );

	a_pDest->SetTextureNum( a_pSource->GetTextureNum() );

	for ( TUINT i = 0; i < a_pSource->GetTextureNum(); i++ )
		a_pDest->SetTexture( i, a_pSource->GetTexture( i ) );

	if ( !a_pSource->IsA( &TGetClass( TWorldMaterial ) ) )
	{
		a_pDest->SetUVAnimation( a_pSource->GetUVAnimation() );
		return;
	}

	TWorldMaterialHAL* pSource = TDYNAMICCAST( TWorldMaterialHAL, a_pSource );
	TWorldMaterialHAL* pDest   = TDYNAMICCAST( TWorldMaterialHAL, a_pDest );

	// The passes: texture, UV stream, blend type and UV mode
	for ( TINT i = 0; i < 5; i++ )
	{
		pDest->m_pTextures[ i ]            = pSource->m_pTextures[ i ];
		pDest->m_iUVStream[ i ]            = pSource->m_iUVStream[ i ];
		pDest->m_aMultiPassBlendTypes[ i ] = pSource->m_aMultiPassBlendTypes[ i ];
		pDest->SetUVMode( i, pSource->m_aUVModes[ i ] );

		TASSERT( pSource->m_iUserDataIndices[ TWorldMaterialHAL::UData_0 ] == -1 );
		TASSERT( pSource->m_iUserDataIndices[ TWorldMaterialHAL::UData_1 ] == -1 );
	}

	pDest->SetMaterialName( pSource->GetMaterialName() );
	pDest->SetUVAnimation( pSource->GetUVAnimation() );
}

// $deBlob: FUNCTION 005f7de0
TBOOL AMaterialInfo::IsModelFirstMeshClass( const T2ModelPtr& a_rModel, const TClass* a_pClass )
{
	TASSERT( a_rModel && a_rModel->IsCreated() && "a_model.IsValid() && a_model->IsCreated()" );

	TModel* pModel = a_rModel.GetData();

	if ( pModel->GetLODCount() <= 0 )
		return TFALSE;

	TModelLOD& rLOD = pModel->GetLOD( 0 );
	if ( rLOD.GetMeshCount() > 0 )
		return rLOD.GetMesh( 0 )->GetClass() == a_pClass;

	return TFALSE;
}

// $deBlob: FUNCTION 0079a4f0
AUVAnimation* AMaterialInfo::GetUVAnimation( TINT a_iPass ) const
{
	TASSERT( a_iPass <= m_pMaterial->GetNumPasses() );
	return m_apUVAnimations[ a_iPass ];
}
