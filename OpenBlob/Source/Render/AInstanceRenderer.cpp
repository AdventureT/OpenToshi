#include "pch.h"
#include "AInstanceRenderer.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Platform/DX11/ASkinShader_DX11.h"

#include <Render/TModel.h>
#include <Render/TMesh.h>
#include <Platform/DX11/TWorldShader_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AInstanceRenderer* AInstanceRenderer::ms_Instance;

TSTATICASSERT( sizeof( AInstanceRenderer::Model ) == 0x4C );
TSTATICASSERT( sizeof( AInstanceRenderer::Batch ) == 0x14 );
TSTATICASSERT( sizeof( AInstanceRenderer ) == 0xA1C );

// $deBlob: FUNCTION 005ebd70
AInstanceRenderer::AInstanceRenderer()
{
	for ( TINT i = 0; i < MAX_MODELS; i++ )
	{
		m_Models[ i ].m_pInstance      = TNULL;
		m_Models[ i ].registerCount    = 0;
		m_Models[ i ].m_uiNumSubmitted = 0;
		m_Models[ i ].m_uiCapacity     = 0;
		m_Models[ i ].m_pBatch         = TNULL;
	}

	m_iNumModels = 0;

	for ( TINT i = 0; i < NUM_BATCHES; i++ )
		m_aBatches[ i ].m_bUsed = TFALSE;

	TASSERT( 0 == ms_Instance );
	ms_Instance = this;

	for ( TINT i = 0; i < NUM_SMALLBATCHES; i++ )
	{
		m_aBatches[ i ].m_ppRotations = TSTATICCAST( const TMatrix44*, TMemalign( 16, 8 * sizeof( void* ) ) );
		m_aBatches[ i ].m_pAlphas     = TSTATICCAST( TFLOAT, TMemalign( 16, 8 * sizeof( TFLOAT ) ) );
		m_aBatches[ i ].m_pColours    = TSTATICCAST( TUINT32, TMemalign( 16, 8 * sizeof( TUINT32 ) ) );
		m_aBatches[ i ].m_ppPositions = TSTATICCAST( const TVector4*, TMemalign( 16, 8 * sizeof( void* ) ) );
		m_aBatches[ i ].m_bBig        = TFALSE;
	}

	for ( TINT i = NUM_SMALLBATCHES; i < NUM_BATCHES; i++ )
	{
		m_aBatches[ i ].m_ppRotations = TSTATICCAST( const TMatrix44*, TMemalign( 16, 64 * sizeof( void* ) ) );
		m_aBatches[ i ].m_pAlphas     = TSTATICCAST( TFLOAT, TMemalign( 16, 64 * sizeof( TFLOAT ) ) );
		m_aBatches[ i ].m_pColours    = TSTATICCAST( TUINT32, TMemalign( 16, 64 * sizeof( TUINT32 ) ) );
		m_aBatches[ i ].m_ppPositions = TSTATICCAST( const TVector4*, TMemalign( 16, 64 * sizeof( void* ) ) );
		m_aBatches[ i ].m_bBig        = TTRUE;
	}
}

// $deBlob: FUNCTION 005ebe80
// $deBlob: FUNCTION 005f0110
AInstanceRenderer::~AInstanceRenderer()
{
	TASSERT( this == ms_Instance );
	ms_Instance = TNULL;

	for ( TINT i = 0; i < m_iNumModels; i++ )
		TASSERT( m_Models[ i ].registerCount == 0 );

	for ( TINT i = 0; i < NUM_BATCHES; i++ )
	{
		TFree( m_aBatches[ i ].m_ppRotations );
		TFree( m_aBatches[ i ].m_pAlphas );
		TFree( m_aBatches[ i ].m_ppPositions );
		TFree( m_aBatches[ i ].m_pColours );
	}
}

// $deBlob: FUNCTION 005ebf50
AModelInstance2* AInstanceRenderer::FindInstance( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < m_iNumModels; i++ )
	{
		if ( m_Models[ i ].registerCount > 0 && TStringManager::String8CompareNoCase( a_szName, m_Models[ i ].m_szName ) == 0 )
			return m_Models[ i ].m_pInstance;
	}

	return TNULL;
}

AInstanceRenderer::Batch* AInstanceRenderer::AllocBatch( TINT a_iFirst, TINT a_iLast )
{
	for ( TINT i = a_iFirst; i < a_iLast; i++ )
	{
		if ( !m_aBatches[ i ].m_bUsed )
		{
			m_aBatches[ i ].m_bUsed = TTRUE;
			return &m_aBatches[ i ];
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 005ec9b0
AInstanceRenderer::Batch* AInstanceRenderer::AllocBatch( TINT a_iType )
{
	if ( a_iType == 0 )
		return AllocBatch( 0, NUM_SMALLBATCHES );

	if ( a_iType == 1 )
		return AllocBatch( NUM_SMALLBATCHES, NUM_BATCHES );

	return TNULL;
}

void AInstanceRenderer::FreeBatch( Batch* a_pBatch, TINT a_iCount )
{
	for ( TINT i = 0; i < a_iCount; i++ )
	{
		if ( a_pBatch == &m_aBatches[ i ] )
		{
			m_aBatches[ i ].m_bUsed = TFALSE;
			return;
		}
	}
}

// $deBlob: FUNCTION 005ebfb0
TINT8 AInstanceRenderer::RegisterModel( const TCHAR* a_szName )
{
	TINT8 iFreeSlot = m_iNumModels;

	for ( TINT i = 0; i < m_iNumModels; i++ )
	{
		if ( m_Models[ i ].registerCount == 0 )
		{
			if ( i < iFreeSlot )
				iFreeSlot = TINT8( i );
		}
		else if ( TStringManager::String8CompareNoCase( a_szName, m_Models[ i ].m_szName ) == 0 )
		{
			m_Models[ i ].registerCount++;
			return TINT8( i );
		}
	}

	TASSERT( iFreeSlot < MAX_MODELS );
	TASSERT( TStringManager::String8Length( a_szName ) < MAX_NAME_LEN );

	Model& rModel      = m_Models[ iFreeSlot ];
	rModel.m_pInstance = AModelFactory::GetSingleton()->CreateInstance( a_szName, TTRUE, TTRUE );
	rModel.m_pInstance->SetFlag( AModelInstance2::FLAG_Renderable, TFALSE );
	TStringManager::String8Copy( rModel.m_szName, a_szName );
	rModel.m_uiCapacity  = 8;
	rModel.m_pBatch      = AllocBatch( 0, NUM_SMALLBATCHES );
	rModel.registerCount = 1;

	if ( iFreeSlot == m_iNumModels )
		m_iNumModels++;

	return iFreeSlot;
}

// $deBlob: FUNCTION 005ec150
void AInstanceRenderer::UnregisterModel( TINT8 a_Index )
{
	TASSERT( a_Index >= 0 && a_Index < MAX_MODELS );

	Model* gfx = &m_Models[ a_Index ];
	TASSERT( gfx->registerCount > 0 );

	if ( --gfx->registerCount != 0 )
		return;

	AModelFactory::GetSingleton()->DestroyInstance( gfx->m_pInstance );
	gfx->m_uiCapacity = 0;
	FreeBatch( gfx->m_pBatch, NUM_BATCHES );
	gfx->m_pBatch         = TNULL;
	gfx->m_uiNumSubmitted = 0;

	m_iNumModels = 0;

	for ( TINT i = 0; i < MAX_MODELS; i++ )
	{
		if ( m_Models[ i ].registerCount > 0 )
			m_iNumModels = TINT8( i + 1 );
	}
}

// $deBlob: FUNCTION 005ec250
void AInstanceRenderer::SubmitForBatchRender( TINT8 a_GFXIndex, const TMatrix44* a_pRotation, const TVector4* a_pPosition, TFLOAT a_fAlpha, TUINT32 a_uiColour )
{
	TASSERT( a_GFXIndex >= 0 && a_GFXIndex < MAX_MODELS );
	Model& rModel = m_Models[ a_GFXIndex ];

	if ( rModel.m_uiCapacity <= rModel.m_uiNumSubmitted )
	{
		// Full: move to a big batch (unless it already is one)
		if ( rModel.m_pBatch->m_bBig == TTRUE )
			return;

		Batch* pBatch = AllocBatch( NUM_SMALLBATCHES, NUM_BATCHES );

		for ( TINT i = 0; i < rModel.m_uiNumSubmitted; i++ )
		{
			pBatch->m_ppRotations[ i ] = rModel.m_pBatch->m_ppRotations[ i ];
			pBatch->m_ppPositions[ i ] = rModel.m_pBatch->m_ppPositions[ i ];
			pBatch->m_pAlphas[ i ]     = rModel.m_pBatch->m_pAlphas[ i ];
			pBatch->m_pColours[ i ]    = rModel.m_pBatch->m_pColours[ i ];
		}

		FreeBatch( rModel.m_pBatch, NUM_BATCHES );
		rModel.m_pBatch     = pBatch;
		rModel.m_uiCapacity = 64;
	}

	rModel.m_pBatch->m_ppRotations[ rModel.m_uiNumSubmitted ] = a_pRotation;
	rModel.m_pBatch->m_ppPositions[ rModel.m_uiNumSubmitted ] = a_pPosition;
	rModel.m_pBatch->m_pAlphas[ rModel.m_uiNumSubmitted ]     = a_fAlpha;
	rModel.m_pBatch->m_pColours[ rModel.m_uiNumSubmitted ]    = a_uiColour;
	rModel.m_uiNumSubmitted++;
}

// $deBlob: FUNCTION 005ec3b0
void AInstanceRenderer::Render()
{
	for ( TINT i = 0; i < m_iNumModels; i++ )
	{
		Model& rModel = m_Models[ i ];

		if ( rModel.m_uiNumSubmitted == 0 )
			continue;

		AModelInstance2* pInstance     = rModel.m_pInstance;
		Batch*           pBatch        = rModel.m_pBatch;
		TINT             iCount        = rModel.m_uiNumSubmitted;
		const TMatrix44* pLastRotation = TNULL;
		TMatrix44        oTransform;

		pInstance->UseLocalAmbientColour( TTRUE );

		for ( TINT j = 0; j < iCount; j++ )
		{
			const TMatrix44* pRotation = pBatch->m_ppRotations[ j ];

			if ( pLastRotation != pRotation )
			{
				oTransform.SetBasisVector4( BASISVECTOR_RIGHT, pRotation->AsBasisVector4( BASISVECTOR_RIGHT ) );
				oTransform.SetBasisVector4( BASISVECTOR_UP, pBatch->m_ppRotations[ j ]->AsBasisVector4( BASISVECTOR_UP ) );
				oTransform.SetBasisVector4( BASISVECTOR_FORWARD, pBatch->m_ppRotations[ j ]->AsBasisVector4( BASISVECTOR_FORWARD ) );
			}

			pLastRotation = pBatch->m_ppRotations[ j ];
			oTransform.SetBasisVector4( BASISVECTOR_TRANSLATION, *pBatch->m_ppPositions[ j ] );

			pInstance->SetTransform( oTransform );
			TModelInstance* pModelInstance = pInstance->GetModelInstance();
			pModelInstance->SetForceFade( TTRUE );
			pModelInstance->SetFade( pBatch->m_pAlphas[ j ] );
			pInstance->SetLocalAmbientColour( pBatch->m_pColours[ j ] );
			pInstance->Render( pInstance->GetModelInstance() );
		}
	}
}

// Render's loop over the copies of one model as a function
// $deBlob: FUNCTION 005ec7f0
void RenderBatch( AModelInstance2* a_pInstance, AInstanceRenderer::Batch* a_pBatch, TINT a_iCount )
{
	const TMatrix44* pLastRotation = TNULL;
	TMatrix44        oTransform;

	a_pInstance->UseLocalAmbientColour( TTRUE );

	for ( TINT j = 0; j < a_iCount; j++ )
	{
		const TMatrix44* pRotation = a_pBatch->m_ppRotations[ j ];

		if ( pLastRotation != pRotation )
		{
			oTransform.SetBasisVector4( BASISVECTOR_RIGHT, pRotation->AsBasisVector4( BASISVECTOR_RIGHT ) );
			oTransform.SetBasisVector4( BASISVECTOR_UP, a_pBatch->m_ppRotations[ j ]->AsBasisVector4( BASISVECTOR_UP ) );
			oTransform.SetBasisVector4( BASISVECTOR_FORWARD, a_pBatch->m_ppRotations[ j ]->AsBasisVector4( BASISVECTOR_FORWARD ) );
		}

		pLastRotation = a_pBatch->m_ppRotations[ j ];
		oTransform.SetBasisVector4( BASISVECTOR_TRANSLATION, *a_pBatch->m_ppPositions[ j ] );

		a_pInstance->SetTransform( oTransform );
		TFLOAT          fAlpha         = a_pBatch->m_pAlphas[ j ];
		TModelInstance* pModelInstance = a_pInstance->GetModelInstance();
		pModelInstance->SetForceFade( TTRUE );
		pModelInstance->SetFade( fAlpha );
		a_pInstance->SetLocalAmbientColour( a_pBatch->m_pColours[ j ] );
		a_pInstance->Render( a_pInstance->GetModelInstance() );
	}
}

// $deBlob: FUNCTION 005ec5c0
void AInstanceRenderer::ClearSubmitted()
{
	for ( TINT i = 0; i < m_iNumModels; i++ )
		m_Models[ i ].m_uiNumSubmitted = 0;
}

// $deBlob: FUNCTION 005ec5f0
void AInstanceRenderer::SetupMaterials( TINT8 a_Index )
{
	T2ModelPtr pModel   = m_Models[ a_Index ].m_pInstance->GetModelInstance()->GetModel();
	TINT       iNumLODs = pModel->GetLODCount();

	for ( TINT i = 0; i < iNumLODs; i++ )
	{
		TModelLOD& rLOD = pModel->GetLOD( i );

		for ( TINT j = 0; j < rLOD.m_iMeshCount; j++ )
		{
			TMaterial* pMaterial = rLOD.m_pMeshes[ j ]->GetMaterial();

			if ( pMaterial->IsA( &TGetClass( TWorldMaterialHAL ) ) )
				TDYNAMICCAST( TWorldMaterialHAL, pMaterial )->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_SHADOW );

			if ( pMaterial->IsA( &TGetClass( ASkinMaterialHAL ) ) )
				TDYNAMICCAST( ASkinMaterialHAL, pMaterial )->SetBlendMode( ASkinMaterial::BLENDMODE( 1 ), 0 );
		}
	}
}
