#include "ToshiPCH.h"
#include "TModelLoaderWorld_DX11.h"
#include "TWorldShader_DX11.h"
#include "TMaterial_DX11.h"
#include "TTexture_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TWorldMaterial* TModelLoaderWorld_DX11::sm_createdMaterials[ LOCALMATS_COUNT ];
TINT            TModelLoaderWorld_DX11::sm_createdMaterialsCount;
TINT            TModelLoaderWorld_DX11::s_iMeshLoadCount;
const TClass*   TModelLoaderWorld_DX11::s_pWorldMaterialClass;

// $deBlob: FUNCTION 006bb350
void TModelLoaderWorld_DX11::LoadWorldMeshTRB( TTMDWin::TRBWorldData* a_pWorldData, TModelLOD* a_pModelLOD )
{
	sm_createdMaterialsCount = 0;
	s_iMeshLoadCount         = 0;

	for ( TINT i = 0; i < a_pWorldData->m_iNumGroups; i++ )
	{
		TTMDWin::TRBWorldCellGroup* pGroup = a_pWorldData->m_ppGroups[ i ];

		for ( TINT k = 0; k < pGroup->m_iNumCells; k++ )
		{
			CellRef oCell;
			oCell.pCell = pGroup->m_ppCells[ k ];
			oCell.iUnk  = 0;

			LoadTreeIntersect( oCell.pCell->m_pTreeRoot, &oCell, a_pModelLOD );
		}
	}

	TASSERT( s_iMeshLoadCount == a_pModelLOD->m_iMeshCount );
}

// $deBlob: FUNCTION 006bb410
void TModelLoaderWorld_DX11::LoadTreeIntersect( void* a_pNode, CellRef* a_pCell, TModelLOD* a_pModelLOD )
{
	TWorldShader* pWorldShader = TWorldShader::GetSingleton();

	// A branch node (+0x10 the next node) has its first child at +0x14, a leaf has its mesh count there followed by
	// the indices of its meshes in the cell
	TUINT8* pNode = TSTATICCAST( TUINT8, a_pNode );

	while ( *TREINTERPRETCAST( TUINT8**, pNode + 0x10 ) != TNULL )
	{
		LoadTreeIntersect( pNode + 0x14, a_pCell, a_pModelLOD );
		pNode = *TREINTERPRETCAST( TUINT8**, pNode + 0x10 );
	}

	TINT    iNumMeshes    = *TREINTERPRETCAST( TINT*, pNode + 0x14 );
	TINT16* pMeshIndices = TREINTERPRETCAST( TINT16*, pNode + 0x18 );

	for ( TINT i = 0; i < iNumMeshes; i++ )
	{
		// The cell's mesh has the TRB world mesh at +0x10 (pCellMesh->m_pWorldMesh)
		TUINT8*        pCellMesh     = TREINTERPRETCAST( TUINT8*, a_pCell->pCell->m_ppMeshes[ pMeshIndices[ i ] ] );
		TTRBWorldMesh* pTRBWorldMesh = *TREINTERPRETCAST( TTRBWorldMesh**, pCellMesh + 0x10 );

		TWorldMesh* pMesh;

		if ( TWorldShader::s_fnCreateMesh != TNULL )
			pMesh = TWorldShader::s_fnCreateMesh( TNULL );
		else
			pMesh = pWorldShader->CreateMesh( TNULL );

		TASSERT( pTRBWorldMesh->m_uiVertexCount >= 3 );
		TASSERT( pTRBWorldMesh->m_uiIndexCount >= 3 );

		TWorldMeshHAL* pMeshHAL = TSTATICCAST( TWorldMeshHAL, pMesh );
		pMeshHAL->Create( pTRBWorldMesh );
		pTRBWorldMesh->m_pMesh = pMeshHAL;

		if ( s_pWorldMaterialClass == TNULL )
			s_pWorldMaterialClass = TClass::Find( "TWorldMaterial" );

		const TCHAR*    szMaterialName = pTRBWorldMesh->m_szMaterialName;
		TWorldMaterial* pMaterial      = TNULL;

		// The materials this asset already created
		for ( TINT k = 0; k < sm_createdMaterialsCount; k++ )
		{
			if ( T2String8::CompareNoCase( sm_createdMaterials[ k ]->GetMaterialName(), szMaterialName ) == 0 )
			{
				pMaterial = sm_createdMaterials[ k ];
				break;
			}
		}

		if ( pMaterial == TNULL )
		{
			if ( TWorldShader::s_fnCreateMaterial != TNULL )
			{
				pMaterial = TWorldShader::s_fnCreateMaterial( pTRBWorldMesh->m_szMaterialName );
			}
			else
			{
				TShader* pOwnerShader = pMesh->GetOwnerShader();
				TASSERT( ( pOwnerShader == TNULL ) || pOwnerShader->IsA( &TGetClass( TWorldShader ) ) );
				pMaterial = TSTATICCAST( TWorldShader, pOwnerShader )->CreateMaterial( pTRBWorldMesh->m_szMaterialName );
			}

			pMaterial->Create( 0 );

			TASSERT( sm_createdMaterialsCount < LOCALMATS_COUNT, "World mesh has too many materials" );
			sm_createdMaterials[ sm_createdMaterialsCount++ ] = pMaterial;

			T2Material* pT2Material = TMaterialManager::GetSingleton()->FindMaterial( pTRBWorldMesh->m_szMaterialName );
			pMaterial->SetMetaMaterial( pT2Material );

			if ( pT2Material != TNULL )
			{
				TWorldMaterialHAL* pMaterialHAL = TSTATICCAST( TWorldMaterialHAL, pMaterial );

				TINT iNumTextures = pT2Material->GetNumTextures();
				TINT iNumPasses   = pT2Material->GetNumPasses();

				for ( TUINT k = 0; k < TUINT( pT2Material->GetNumTextures() ); k++ )
				{
					pMaterial->SetTexture( k, pT2Material->GetTexture( k ) );

					if ( pT2Material->GetTexture( k )->GetFlags1() & TTexture::Flags1_Alpha )
						pMaterial->SetBlendMode( 1 );
				}

				// The passes without a texture of their own use their pass texture
				for ( TINT k = iNumTextures; k < iNumPasses; k++ )
					pMaterial->SetTexture( k, pT2Material->GetPassTexture( k )->m_pTexture );

				TINT iNumPassTextures = ( iNumPasses > iNumTextures ) ? iNumPasses - iNumTextures : 0;
				pMaterial->SetTextureNum( pT2Material->GetNumTextures() + iNumPassTextures );

				if ( pT2Material->GetNumTextures() == 2 )
				{
					pMaterialHAL->SetUVStream( 0, 0 );
					pMaterialHAL->SetUVStream( 1, 1 );
					pMaterialHAL->SetMultiPassBlendType( 0, TWorldMaterialHAL::MULTIPASSBLENDTYPE_OVERRIDE );
					pMaterialHAL->SetMultiPassBlendType( 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_MODULATE );
				}

				if ( iNumPasses > 0 )
					pMaterial->SetBlendMode( pT2Material->GetPassBlendMode( 0 ) );

				for ( TINT k = 0; k < iNumPasses; k++ )
					pMaterialHAL->SetMultiPassBlendType( k, pT2Material->GetPassTevMode( k ) );

				TShader* pShader = pMaterial->GetShader();
				TASSERT( ( pShader == TNULL ) || pShader->IsA( &TGetClass( TWorldShader ) ) );

				if ( pShader != TNULL )
				{
					TWorldShaderHAL* pWorldShaderHAL = TSTATICCAST( TWorldShaderHAL, pShader );
					TINT             iOrderTable     = pT2Material->GetOrderTableIndex();

					if ( iOrderTable < pWorldShaderHAL->GetNumOrderTables() )
						pMaterialHAL->SetOrderTable( pWorldShaderHAL->GetOrderTable( iOrderTable ), 0, TTRUE );
				}

				TWorldShader::t_MaterialCreatedCallback fnCallback = pWorldShader->GetMaterialCreatedCallback();

				if ( fnCallback != TNULL )
					fnCallback( pMaterial, pT2Material );
			}
		}

		pMesh->SetMaterial( pMaterial, TTRUE );
		a_pModelLOD->m_pMeshes[ s_iMeshLoadCount++ ] = pMesh;
	}
}

// $deBlob: FUNCTION 006bb960
TWorldMaterial* TModelLoaderWorld_DX11::FindCreatedMaterial( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < sm_createdMaterialsCount; i++ )
	{
		if ( T2String8::CompareNoCase( sm_createdMaterials[ i ]->GetMaterialName(), a_szName ) == 0 )
			return sm_createdMaterials[ i ];
	}

	return TNULL;
}

TOSHI_NAMESPACE_END
