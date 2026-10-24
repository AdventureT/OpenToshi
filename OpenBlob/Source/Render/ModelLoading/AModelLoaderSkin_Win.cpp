#include "pch.h"
#include "AModelLoader.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Platform/DX11/ASkinShader_DX11.h"

#include <Platform/DX11/TRender_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005f9b10
void AModelLoader::LoadSkinMeshTRB( TModel* a_pModel, TINT a_iLODIndex, TModelLOD* a_pModelLOD, TTMDWin::TRBLODHeader* a_pLODHeader )
{
	ASkinShader* pShader    = ASkinShader::GetSingleton();
	TINT         iMeshCount = a_pLODHeader->m_pInfo->m_iMeshCount1 + a_pLODHeader->m_pInfo->m_iMeshCount2;
	TASSERT( iMeshCount == a_pLODHeader->m_uiMeshCount );

	for ( TINT iMeshIndex = 0; iMeshIndex < iMeshCount; iMeshIndex++ )
	{
		TTMDWin::TRBSkinMesh* pTRBSkinMesh = a_pLODHeader->m_ppMeshes[ iMeshIndex ];
		ASkinMeshHAL*         pSkinMesh    = TSTATICCAST( ASkinMeshHAL, pShader->CreateMesh( TNULL ) );

		TASSERT( pTRBSkinMesh->m_uiVertexCount >= 3 );
		TASSERT( pTRBSkinMesh->m_uiIndexCount >= 3 );
		TASSERT( pTRBSkinMesh->m_uiSubMeshCount > 0 );
		TASSERT( pTRBSkinMesh->m_uiSubMeshCount <= TMath::TUINT16_MAX );
		TASSERT( pTRBSkinMesh->m_uiVertexCount <= TMath::TUINT16_MAX );

		pSkinMesh->Create( pTRBSkinMesh->m_uiFlags, TUINT16( pTRBSkinMesh->m_uiVertexCount ), TUINT16( pTRBSkinMesh->m_uiSubMeshCount ) );

		if ( ms_pSkinMaterialClass == TNULL )
			ms_pSkinMaterialClass = TClass::Find( "ASkinMaterial", TNULL );

		TMaterial* pMaterial = TNULL;
		GetSingleton()->GetMaterial( pShader, pTRBSkinMesh->m_szMaterialName, pMaterial );
		pSkinMesh->SetMaterial( pMaterial );
		a_pModelLOD->m_pMeshes[ iMeshIndex ] = pSkinMesh;

		for ( TUINT32 sm = 0; sm < pTRBSkinMesh->m_uiSubMeshCount; sm++ )
		{
			ASkinSubMesh*            pSubMesh    = pSkinMesh->GetSubMesh( sm );
			TTMDWin::TRBSkinSubMesh* pTRBSubMesh = &pTRBSkinMesh->m_pSubMeshes[ sm ];

			TUtil::MemCopy( pSubMesh->aBoneRemap, pTRBSubMesh->m_pBones, pTRBSubMesh->m_uiBoneCount * sizeof( TINT ) );

			// NOTE: retail creates each sub mesh's index buffer with the index count of the whole mesh
			pSubMesh->CreateIndexBuffer( TUINT32( pSkinMesh ), pTRBSkinMesh->m_uiIndexCount, pTRBSubMesh->m_pIndices );
			pSubMesh->uiNumBones = pTRBSubMesh->m_uiBoneCount;

			TASSERT( pTRBSubMesh->m_uiNumVertices2 <= TMath::TUINT16_MAX );
			pSubMesh->uiNumVertices = TUINT16( pTRBSubMesh->m_uiNumVertices2 );
			memcpy( pSubMesh->afUnk08, pTRBSubMesh->m_aUnk, sizeof( pSubMesh->afUnk08 ) );

			if ( pTRBSubMesh->m_uiVertexCount != 0 )
			{
				// Only the first sub mesh has the vertices (shared by all)
				TASSERT( sm == 0 );

				D3D11_BOX oBox;
				oBox.left   = 0;
				oBox.top    = 0;
				oBox.front  = 0;
				oBox.right  = pTRBSubMesh->m_uiVertexCount * ASkinMesh::VERTEX_SIZE;
				oBox.bottom = 1;
				oBox.back   = 1;

				TRenderDX11::Interface()->GetDeviceContext()->UpdateSubresource( pSkinMesh->GetVertexPool(), 0, &oBox, pTRBSubMesh->m_pVertices, 0, 0 );
			}
		}
	}
}
