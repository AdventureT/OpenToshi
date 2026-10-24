#include "ToshiPCH.h"
#include "TWorldShader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TWorldShader::t_CreateMaterialCallback TWorldShader::s_fnCreateMaterial = TNULL;
TWorldShader::t_CreateMeshCallback     TWorldShader::s_fnCreateMesh     = TNULL;
TMemoryHeap*                           TWorldShader::s_pWorldMemHeap    = TNULL;

//-----------------------------------------------------------------------------
// TWorldMaterialManager
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00706910
TWorldMaterial* TWorldMaterialManager::FindMaterial( const TCHAR* a_szName )
{
	for ( TWorldMaterial* pMat = m_pFirstMaterial; pMat != TNULL; pMat = pMat->m_pNextMaterial )
	{
		if ( T2String8::CompareNoCase( pMat->m_szMaterialName, a_szName ) == 0 )
			return pMat;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00706950
void TWorldMaterialManager::AddMaterial( TWorldMaterial* a_pMat )
{
	a_pMat->m_pNextMaterial = m_pFirstMaterial;
	m_pFirstMaterial        = a_pMat;
}

// $deBlob: FUNCTION 00706970
void TWorldMaterialManager::RemoveMaterial( TWorldMaterial* a_pMat )
{
	TASSERT( a_pMat->GetRefCount() == 0 );

	TWorldMaterial* pPrev = TNULL;

	for ( TWorldMaterial* pMat = m_pFirstMaterial; pMat != TNULL; pMat = pMat->m_pNextMaterial )
	{
		if ( a_pMat == pMat )
		{
			if ( pPrev == TNULL )
			{
				m_pFirstMaterial     = pMat->m_pNextMaterial;
				pMat->m_pNextMaterial = TNULL;
			}
			else
			{
				pPrev->m_pNextMaterial = pMat->m_pNextMaterial;
			}

			if ( a_pMat->IsCreated() )
				a_pMat->OnDestroy();

			delete a_pMat;
			return;
		}

		pPrev = pMat;
	}

	TASSERT( !"didnt find material to remove!" );
}

//-----------------------------------------------------------------------------
// TWorldShader
//-----------------------------------------------------------------------------

// NOTE: TDEFINE_CLASS generates GetClass, CreateObject, CreateObjectInPlace (abstract: both return TNULL)
// $deBlob: FUNCTION 00706a30
// $deBlob: FUNCTION 00706a40
// $deBlob: FUNCTION 00706a50
// $deBlob: FUNCTION 00706a60
// $deBlob: FUNCTION 00706a70
// $deBlob: FUNCTION 0053fb70
// $deBlob: FUNCTION 00859540
TDEFINE_CLASS_NORUNTIME( TWorldShader );

// $deBlob: FUNCTION 00706a80
TWorldShader::TWorldShader()
{
	m_fUnk10             = 0.5f;
	m_pUnk14             = TNULL;
	m_fnMaterialCreated   = TNULL;
	m_fnMaterialDestroyed = TNULL;

	new TWorldMaterialManager();
}

// $deBlob: FUNCTION 00706b40
// $deBlob: FUNCTION 007071f0
TWorldShader::~TWorldShader()
{
	// NOTE: retail frees the manager without running its destructor
	TWorldMaterialManager* pManager = TWorldMaterialManager::GetSingleton();

	if ( pManager != TNULL )
	{
		TWorldMaterialManager::ms_pSingleton = TNULL;
		operator delete( pManager );
	}
}

//-----------------------------------------------------------------------------
// TWorldMaterial
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00706bc0
// $deBlob: FUNCTION 00706bd0
// $deBlob: FUNCTION 00706be0
// $deBlob: FUNCTION 00706bf0
// $deBlob: FUNCTION 00706c00
// $deBlob: FUNCTION 0053faf0
// $deBlob: FUNCTION 00859520
TDEFINE_CLASS_NORUNTIME( TWorldMaterial );

// $deBlob: FUNCTION 00706c30
TWorldMaterial::TWorldMaterial()
{
	m_iRefCount      = 0;
	m_pNextMaterial  = TNULL;
	m_szMaterialName = TNULL;
	m_eBlendMode     = 0;
}

// $deBlob: FUNCTION 00706c60
// $deBlob: FUNCTION 00707130
TWorldMaterial::~TWorldMaterial()
{
}

// $deBlob: FUNCTION 00705a80
TBOOL TWorldMaterial::Create( BLENDMODE a_eBlendMode )
{
	SetBlendMode( a_eBlendMode );
	return TMaterial::Create();
}

// $deBlob: FUNCTION 00706c10
void TWorldMaterial::SetBlendMode( BLENDMODE a_eBlendMode )
{
	// NOTE: TMaterial's flag 3 (FLAGS_GLOW in Barnyard) marks the materials with a blend mode other than 0
	m_Flags &= ~FLAGS_GLOW;
	m_eBlendMode = a_eBlendMode;

	if ( a_eBlendMode != 0 )
		m_Flags |= FLAGS_GLOW;
}

//-----------------------------------------------------------------------------
// TWorldMesh
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 00706c70
// $deBlob: FUNCTION 00706c80
// $deBlob: FUNCTION 00706c90
// $deBlob: FUNCTION 00706ca0
// $deBlob: FUNCTION 00706cb0
// $deBlob: FUNCTION 0053fb30
// $deBlob: FUNCTION 00859530
TDEFINE_CLASS_NORUNTIME( TWorldMesh );

// $deBlob: FUNCTION 00706f90
TWorldMesh::TWorldMesh()
{
	m_pUnk10 = TNULL;
}

// $deBlob: FUNCTION 007071c0
TWorldMesh::~TWorldMesh()
{
}

// $deBlob: FUNCTION 00706cc0
void TWorldMesh::OnDestroy()
{
	if ( GetMaterial() != TNULL )
	{
		GetMaterial()->DecRefCount();

		if ( GetMaterial()->GetRefCount() == 0 )
			TWorldMaterialManager::GetSingleton()->RemoveMaterial( GetMaterial() );
	}

	TMesh::OnDestroy();
}

// $deBlob: FUNCTION 00707290
TBOOL TWorldMesh::RenderWithCallback( t_RenderCallback a_fnCallback, void* a_pUserData )
{
	return TTRUE;
}

// $deBlob: FUNCTION 00706e10
void TWorldMesh::SetMaterial( TWorldMaterial* a_pMaterial, TBOOL a_bManaged )
{
	if ( a_pMaterial->GetRefCount() == 0 && a_bManaged )
		TWorldMaterialManager::GetSingleton()->AddMaterial( a_pMaterial );

	if ( GetMaterial() != TNULL )
	{
		GetMaterial()->DecRefCount();

		if ( GetMaterial()->GetRefCount() == 0 && a_bManaged )
			TWorldMaterialManager::GetSingleton()->RemoveMaterial( GetMaterial() );
	}

	a_pMaterial->IncRefCount();
	m_pMaterial = a_pMaterial;
}

TOSHI_NAMESPACE_END
