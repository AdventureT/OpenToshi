#include "pch.h"
#include "AAnimationPoseFactory.h"
#include "AAnimationPose.h"
#include "AModel2.h"

#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0056e110
AAnimationPoseFactory::AAnimationPoseFactory()
{
}

// $deBlob: FUNCTION 0056e160
// $deBlob: FUNCTION 0056e880
AAnimationPoseFactory::~AAnimationPoseFactory()
{
	for ( TINT i = 0; i < m_lPoseList.Size(); i++ )
	{
		// NOTE: retail unloads the list before checking it for TNULL
		m_lPoseList[ i ]->pPoseList->Unload();
		delete m_lPoseList[ i ]->pPoseList;
		delete m_lPoseList[ i ];
	}

	m_lPoseList.Reset();
}

// $deBlob: FUNCTION 0056e320
AAnimationPoseList* AAnimationPoseFactory::GetPoseList( const TCHAR* a_szName, AModel2* a_pModel )
{
	for ( TINT i = 0; i < m_lPoseList.Size(); i++ )
	{
		if ( m_lPoseList[ i ]->pPoseList->GetName().CompareNoCase( a_szName ) == 0 )
		{
			TASSERT( a_pModel == m_lPoseList[ i ]->pModel );
			return m_lPoseList[ i ]->pPoseList;
		}
	}

	PoseList* pPoseList  = new PoseList;
	pPoseList->pPoseList = new AAnimationPoseList( a_szName );
	pPoseList->pPoseList->Load();

	if ( a_pModel->GetModelPtr().GetID() == T2ResourcePtr::IDINVALID )
	{
		pPoseList->bSeqIDsPatched = TFALSE;
	}
	else
	{
		T2ModelPtr pModel = a_pModel->GetModelPtr();
		pPoseList->pPoseList->PatchSeqIDs( TSTATICCAST( TModel, pModel.GetData() )->GetSkeleton() );
		pPoseList->bSeqIDsPatched = TTRUE;
	}

	pPoseList->pModel = a_pModel;
	m_lPoseList.Push( pPoseList );

	return pPoseList->pPoseList;
}

// $deBlob: FUNCTION 0056e560
void AAnimationPoseFactory::Update()
{
	for ( TINT i = 0; i < m_lPoseList.Size(); i++ )
	{
		if ( !m_lPoseList[ i ]->pModel->IsLoaded() )
		{
			m_lPoseList[ i ]->bSeqIDsPatched = TFALSE;
		}
		else if ( !m_lPoseList[ i ]->bSeqIDsPatched )
		{
			T2ModelPtr pModel = m_lPoseList[ i ]->pModel->GetModelPtr();
			m_lPoseList[ i ]->pPoseList->PatchSeqIDs( TSTATICCAST( TModel, pModel.GetData() )->GetSkeleton() );
			m_lPoseList[ i ]->bSeqIDsPatched = TTRUE;
		}
	}
}
