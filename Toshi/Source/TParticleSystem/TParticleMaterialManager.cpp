#include "ToshiPCH.h"
#include "TParticleMaterialManager.h"
#include "TParticleMaterial.h"
#include "TParticleSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00712090
TParticleMaterialManager::TParticleMaterialManager()
{
	m_iNumMaterials  = 0;
	m_pMaterialsRoot = TNULL;
	TUtil::MemClear( &m_oMeshMaterial, sizeof( m_oMeshMaterial ) );
	m_oMeshMaterial.m_uiVer = TUINT( -1 );
}

// $deBlob: FUNCTION 007120c0
TParticleMaterial* TParticleMaterialManager::FindMaterial( const TCHAR* a_szName ) const
{
	if ( a_szName == TNULL )
		return TNULL;

	for ( TParticleMaterial* pMaterial = m_pMaterialsRoot; pMaterial != TNULL; pMaterial = pMaterial->m_pNext )
	{
		if ( TStringManager::String8CompareNoCase( a_szName, pMaterial->m_szName ) == 0 )
			return pMaterial;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00712110
void TParticleMaterialManager::UpdateMaterials()
{
	TINT id = 0;

	for ( TParticleMaterial* pMaterial = m_pMaterialsRoot; pMaterial != TNULL; pMaterial = pMaterial->m_pNext )
	{
		pMaterial->m_uiVer = id;
		id++;
	}

	TASSERT( id == m_iNumMaterials );
}

// $deBlob: FUNCTION 00712160
void TParticleMaterialManager::AddMaterial( TParticleMaterial* a_pMaterial )
{
	TParticleSystem::GetSingleton()->SetMaterialsDirty();
	a_pMaterial->Create();
	a_pMaterial->m_uiVer = m_iNumMaterials++;

	TParticleMaterial* pRoot = m_pMaterialsRoot;
	TParticleMaterial* cur   = TNULL;

	if ( pRoot != TNULL )
	{
		TParticleMaterial* prev = TNULL;

		for ( TParticleMaterial* pIt = pRoot; ; pIt = cur->m_pNext )
		{
			cur = pIt;

			if ( a_pMaterial->m_iPriority < cur->m_iPriority )
			{
				if ( prev != TNULL )
				{
					prev->m_pNext        = a_pMaterial;
					a_pMaterial->m_pPrev = prev;
					cur->m_pPrev         = a_pMaterial;
					a_pMaterial->m_pNext = cur;
					return;
				}

				TASSERT( m_pMaterialsRoot == cur );
				m_pMaterialsRoot     = a_pMaterial;
				a_pMaterial->m_pPrev = TNULL;
				cur->m_pPrev         = a_pMaterial;
				a_pMaterial->m_pNext = cur;
				return;
			}

			prev = cur;

			if ( cur->m_pNext == TNULL )
				break;
		}
	}

	if ( pRoot == TNULL )
	{
		a_pMaterial->m_pNext = TNULL;
		a_pMaterial->m_pPrev = TNULL;
		m_pMaterialsRoot     = a_pMaterial;
		return;
	}

	TASSERT( TNULL != cur && "TNULL != prev" );
	cur->m_pNext         = a_pMaterial;
	a_pMaterial->m_pPrev = cur;
	a_pMaterial->m_pNext = TNULL;
}

// $deBlob: FUNCTION 00712270
void TParticleMaterialManager::RemoveMaterial( TParticleMaterial* a_pMaterial )
{
	for ( TParticleMaterial* pMaterial = m_pMaterialsRoot; pMaterial != TNULL; pMaterial = pMaterial->m_pNext )
	{
		if ( pMaterial == a_pMaterial )
		{
			if ( pMaterial->m_pPrev != TNULL )
				pMaterial->m_pPrev->m_pNext = pMaterial->m_pNext;

			if ( pMaterial->m_pNext != TNULL )
				pMaterial->m_pNext->m_pPrev = pMaterial->m_pPrev;

			if ( pMaterial == m_pMaterialsRoot )
				m_pMaterialsRoot = m_pMaterialsRoot->m_pNext;

			pMaterial->Destroy();
			m_iNumMaterials--;
			TASSERT( m_iNumMaterials >= 0 );
			return;
		}
	}
}

TOSHI_NAMESPACE_END
