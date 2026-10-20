#include "pch.h"
#include "AAnimationPose.h"
#include "Entities/AEntityData.h"

#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0056de30
AAnimationPoseList::AAnimationPoseList( const TCHAR* a_szName )
    : m_pTRBInfo( TNULL )
    , m_strName( a_szName )
{
	TUtil::MemClear( m_PoseHash, sizeof( m_PoseHash ) );
}

// $deBlob: FUNCTION 0056dea0
// $deBlob: FUNCTION 0056e8b0
AAnimationPoseList::~AAnimationPoseList()
{
	Unload();
}

// $deBlob: FUNCTION 0056df10
void AAnimationPoseList::Load()
{
	Unload();

	TCHAR szSymbol[ 1024 ];
	TStringManager::String8Format( szSymbol, "%s_poses_Pose", AEntityData::GetDirectoryName( m_strName.GetString() ) );
	m_pTRBInfo = TSTATICCAST( TRBInfo, AEntityData::GetSingleton()->GetPoseData( szSymbol ) );

	CreatePoseHash();
}

// $deBlob: FUNCTION 0056e060
void AAnimationPoseList::CreatePoseHash()
{
	for ( TINT i = 0; i < m_pTRBInfo->m_iNumPoses; i++ )
	{
		TASSERT( m_PoseHash[ m_pTRBInfo->m_pPoses[ i ].m_iPoseID ] == TNULL );
		TASSERT( m_pTRBInfo->m_pPoses[ i ].m_iPoseID >= 0 && m_pTRBInfo->m_pPoses[ i ].m_iPoseID < MAX_POSES );

		m_PoseHash[ m_pTRBInfo->m_pPoses[ i ].m_iPoseID ] = &m_pTRBInfo->m_pPoses[ i ];
	}
}

// $deBlob: FUNCTION 0056dfe0
void AAnimationPoseList::PatchSeqIDs( TSkeleton* a_pSkeleton )
{
	for ( TINT i = 0; i < m_pTRBInfo->m_iNumPoses; i++ )
	{
		AAnimationPose& rPose = m_pTRBInfo->m_pPoses[ i ];

		for ( TINT j = 0; j < rPose.m_iPoseComponentCount; j++ )
			rPose.m_pComponents[ j ].iSequence = a_pSkeleton->GetSequenceID( rPose.m_pComponents[ j ].szSequence, -1 );
	}
}
