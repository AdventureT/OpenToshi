#include "ToshiPCH.h"
#include "TModel_DX11.h"
#include "TMaterial_DX11.h"
#include "Shaders/SkinShader/TSkinShader.h"
#include "Render/TRender.h"
#include "Render/TVertexPoolResourceInterface.h"
#include "Render/TIndexPoolResourceInterface.h"
#include "Render/TIndexFactoryResourceInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TModelHAL::t_TRBLoadCallback2 TModelHAL::sm_pTRBLoadCallback2 = LoadSkinModel;
const TClass*                 TModelHAL::s_pSkinMaterial;

TSTATICASSERT( sizeof( TModelHAL ) == 0xC0 );

// $deBlob: FUNCTION 006b5830
TBOOL TModel::Create( const TCHAR* a_szName, TBOOL a_bLoadImmediately )
{
	TASSERT( TFALSE == IsCreated() );
	m_eFlags |= Flags_Created;

	TASSERT( TFALSE == a_bLoadImmediately );

	CreateResource( a_szName );
	m_szName = a_szName;

	return TTRUE;
}

// $deBlob: FUNCTION 006b58c0
void TModel::Delete()
{
	Unload();
	delete this;
}

// $deBlob: FUNCTION 006b58e0
void TModel::Unload()
{
	if ( m_pSkeleton != TNULL )
	{
		m_pSkeleton->GetKeyLibraryInstance().Destroy();
		m_pSkeleton = TNULL;
	}

	for ( TINT i = 0; i < m_iLODCount; i++ )
	{
		TModelLOD& rLOD = GetLOD( i );

		if ( rLOD.m_pMeshes != TNULL )
		{
			for ( TINT k = 0; k < rLOD.m_iMeshCount; k++ )
			{
				if ( rLOD.m_pMeshes[ k ] != TNULL )
				{
					// The engine's skin meshes own their materials (the game's are released by AModelLoader)
					if ( rLOD.m_pMeshes[ k ]->IsA( &TGetClass( TSkinMesh ) ) )
					{
						TSkinMesh* pSkinMesh = TSTATICCAST( TSkinMesh, rLOD.m_pMeshes[ k ] );

						if ( pSkinMesh->GetMaterial() != TNULL )
							pSkinMesh->GetMaterial()->Destroy();

						if ( pSkinMesh->GetMaterial2() != TNULL )
							pSkinMesh->GetMaterial2()->Destroy();
					}

					rLOD.m_pMeshes[ k ]->DestroyResource();
					rLOD.m_pMeshes[ k ] = TNULL;
				}
			}

			TFree( rLOD.m_pMeshes );
			rLOD.m_pMeshes    = TNULL;
			rLOD.m_iMeshCount = 0;
		}
	}

	UnloadTRB( TTRUE );
}

// $deBlob: FUNCTION 006b5a00
TModelHAL::TModelHAL()
{
}

// $deBlob: FUNCTION 006b5a10
TModelHAL::~TModelHAL()
{
}

// $deBlob: FUNCTION 006b5a20
TBOOL TModelHAL::CreateTRB( const TCHAR* a_szFileName, TBOOL a_bLoadImmediately )
{
	TASSERT( TFALSE == IsCreated() );
	m_eFlags |= Flags_Created;

	TASSERT( TTRUE == a_bLoadImmediately );

	if ( a_bLoadImmediately == TTRUE && !LoadTRB( a_szFileName ) )
	{
		m_eFlags &= ~Flags_Created;
		return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006b5ac0
TBOOL TModelHAL::LoadTRB( const TCHAR* a_szFileName )
{
	TASSERT( TTRUE == IsCreated() );

	if ( !( m_eFlags & Flags_TrbLoaded ) )
	{
		if ( !LoadTRBFile( a_szFileName ) )
			return TFALSE;

		m_eFlags |= Flags_TrbLoaded;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006b5b20
TBOOL TModelHAL::Create( TTMDWin::TTRBWinHeader* a_pTMDHeader )
{
	CreateResource( a_pTMDHeader->m_pTXSModelName );
	m_pTMDHeader = a_pTMDHeader;
	m_szName     = a_pTMDHeader->m_pTXSModelName;

	CreateSkeleton( a_pTMDHeader->m_pSkeletonHeader, a_pTMDHeader->m_pSkeleton, TTRUE );
	CreateCollision( m_pTMDHeader->m_pModelCollision );

	m_eFlags |= Flags_Created | Flags_TrbLoaded;
	return sm_pTRBLoadCallback2( this, m_pTMDHeader );
}

// $deBlob: FUNCTION 006b5b80
TBOOL __stdcall TModelHAL::LoadSkinModel( TModelHAL* a_pModel, TTMDWin::TTRBWinHeader* a_pHeader )
{
	a_pModel->m_iLODCount = a_pHeader->m_uiLODCount;
	a_pModel->m_fUnknown  = a_pHeader->m_fUnknown;

	for ( TUINT i = 0; i < a_pHeader->m_uiLODCount; i++ )
	{
		TModelLOD*             pLOD    = &a_pModel->m_LODs[ i ];
		TTMDWin::TRBLODHeader* pTRBLOD = a_pHeader->m_ppLODs[ i ];
		TTMDWin::TRBLODInfo*   pInfo   = pTRBLOD->m_pInfo;

		pLOD->m_iMeshCount   = pInfo->m_iMeshCount1 + pInfo->m_iMeshCount2;
		pLOD->m_pMeshes      = TSTATICCAST( TMesh*, TMalloc( pLOD->m_iMeshCount * sizeof( TMesh* ) ) );
		pLOD->m_RenderVolume = pInfo->m_RenderVolume;

		if ( pInfo->m_eShader != TTMDBase::SHADERTYPE_SKIN )
			return TFALSE;

		LoadSkinLOD( a_pModel, i, pLOD, pTRBLOD );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006b5c00
void __stdcall TModelHAL::LoadSkinLOD( TModelHAL* a_pModel, TINT a_iLODIndex, TModelLOD* a_pLOD, TTMDWin::TRBLODHeader* a_pTRBLOD )
{
	TINT iMeshCount = a_pTRBLOD->m_pInfo->m_iMeshCount1 + a_pTRBLOD->m_pInfo->m_iMeshCount2;
	TASSERT( iMeshCount == a_pTRBLOD->m_uiMeshCount );

	// NOTE: de Blob never creates a TSkinShader (its skin meshes are loaded by the game's AModelLoader)
	TSkinShader* pShader = TSkinShader::GetSingleton();

	for ( TINT iMeshIndex = 0; iMeshIndex < iMeshCount; iMeshIndex++ )
	{
		TTMDWin::TRBSkinMesh* pTRBSkinMesh = a_pTRBLOD->m_ppMeshes[ iMeshIndex ];
		TSkinMesh*            pMesh        = TSTATICCAST( TSkinMesh, pShader->CreateMesh( TNULL ) );

		TASSERT( pTRBSkinMesh->m_uiVertexCount >= 3 );
		TASSERT( pTRBSkinMesh->m_uiIndexCount >= 3 );
		TASSERT( pTRBSkinMesh->m_uiSubMeshCount > 0 );
		TASSERT( pTRBSkinMesh->m_uiSubMeshCount <= TMath::TUINT16_MAX );
		TASSERT( pTRBSkinMesh->m_uiVertexCount <= TMath::TUINT16_MAX );

		pMesh->Create( pTRBSkinMesh->m_uiFlags, TUINT16( pTRBSkinMesh->m_uiVertexCount ), TUINT16( pTRBSkinMesh->m_uiSubMeshCount ) );

		if ( s_pSkinMaterial == TNULL )
			s_pSkinMaterial = TClass::Find( "TSkinMaterial", TNULL );

		TMaterial* pMaterial = pShader->CreateMaterial( pTRBSkinMesh->m_szMaterialName );
		T2String8::ToLowerCase( pTRBSkinMesh->m_szMaterialName );
		T2Material* pMetaMat = TMaterialManager::GetSingleton()->FindMaterial( pTRBSkinMesh->m_szMaterialName );

		if ( pMetaMat != TNULL && pMetaMat->GetNumTextures() != 0 )
		{
			pMaterial->SetTexture( 0, pMetaMat->GetTexture( 0 ) );
			pMaterial->SetTextureNum( 1 );
		}

		pMesh->SetMaterial( pMaterial );
		pMesh->SetMaterial2( TNULL );
		a_pLOD->m_pMeshes[ iMeshIndex ] = pMesh;

		for ( TUINT32 sm = 0; sm < pTRBSkinMesh->m_uiSubMeshCount; sm++ )
		{
			TSkinSubMesh*            pSubMesh    = pMesh->GetSubMesh( sm );
			TTMDWin::TRBSkinSubMesh* pTRBSubMesh = &pTRBSkinMesh->m_pSubMeshes[ sm ];

			TUtil::MemCopy( pSubMesh->aBoneRemap, pTRBSubMesh->m_pBones, pTRBSubMesh->m_uiBoneCount * sizeof( TINT ) );

			if ( pSubMesh->pIndexPool == TNULL )
			{
				TIndexFactoryResourceInterface* pFactory = TRender::GetSingleton()->GetSystemResource<TIndexFactoryResourceInterface>( TRender::SYSRESOURCE_IFSYS );
				pSubMesh->pIndexPool                     = pFactory->CreatePoolResource( TUINT16( pTRBSkinMesh->m_uiIndexCount ), 9 );
			}

			pSubMesh->uiNumBones = pTRBSubMesh->m_uiBoneCount;
			TASSERT( pTRBSubMesh->m_uiNumVertices2 <= TMath::TUINT16_MAX );
			pSubMesh->uiNumVertices = TUINT16( pTRBSubMesh->m_uiNumVertices2 );
			TUtil::MemCopy( pSubMesh->afUnk08, pTRBSubMesh->m_aUnk, sizeof( pSubMesh->afUnk08 ) );

			if ( pTRBSubMesh->m_uiVertexCount != 0 )
			{
				TVertexPoolResourceInterface::LockBuffer oLockBuffer;

				if ( pMesh->Lock( &oLockBuffer ) == TTRUE )
				{
					TASSERT( oLockBuffer.uiNumStreams > 0 );
					TUtil::MemCopy( oLockBuffer.apStreams[ 0 ], pTRBSubMesh->m_pVertices, pTRBSubMesh->m_uiVertexCount * 0x2C );
					TASSERT( pTRBSubMesh->m_uiVertexCount <= TMath::TUINT16_MAX );
					pMesh->Unlock( TUINT16( pTRBSubMesh->m_uiVertexCount ) );
				}
			}

			if ( pTRBSubMesh->m_uiIndexCount != 0 )
			{
				TIndexPoolResourceInterface*             pIndexPool = TSTATICCAST( TIndexPoolResourceInterface, pSubMesh->pIndexPool );
				TIndexPoolResourceInterface::LockBuffer oLockBuffer;
				oLockBuffer.uiOffset = 0;
				oLockBuffer.pBuffer  = TNULL;

				pIndexPool->Lock( &oLockBuffer );
				TASSERT( oLockBuffer.pBuffer != TNULL );
				TUtil::MemCopy( oLockBuffer.pBuffer, pTRBSubMesh->m_pIndices, pTRBSubMesh->m_uiIndexCount * sizeof( TUINT16 ) );
				TASSERT( pTRBSubMesh->m_uiIndexCount <= TMath::TUINT16_MAX );
				pIndexPool->Unlock( TUINT16( pTRBSubMesh->m_uiIndexCount ) );
			}
		}
	}
}

TOSHI_NAMESPACE_END
