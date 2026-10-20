#include "pch.h"
#include "ABuildPathJobQueue.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABuildPathJobQueue ) == 0x6420 );

// $deBlob: FUNCTION 007dd840
ABuildPathJobQueue::ABuildPathJobQueue()
{
	InitFreeLists();
}

// $deBlob: FUNCTION 007dd8c0
ABuildPathJob* ABuildPathJobQueue::AddNewJob( AAINavigator* a_pNavigator, const TVector4& a_rvStart, const TVector4& a_rvGoal, TFLOAT a_fRadius, TBOOL a_bUnk, const TVector4* a_pFacing, AAINavigator::eGroundGoal a_eGroundGoal, AAINavigator::eDoubleBuffer a_eDoubleBuffer )
{
	TASSERT( !m_FreeJobs.IsEmpty(), "No more jobs available for building AI path" );

	ABuildPathJob* pJob = TSTATICCAST( ABuildPathJob, m_FreeJobs.PopBack() );

	pJob->Init( a_pNavigator, a_rvStart, a_rvGoal, a_fRadius, a_bUnk, a_pFacing, a_eGroundGoal, a_eDoubleBuffer );

	m_PendingJobs.PushBack( pJob );

	return pJob;
}

// $deBlob: FUNCTION 007dd9e0
void ABuildPathJobQueue::InitFreeLists()
{
	for ( TINT i = 0; i < MAX_JOBS; i++ )
		m_FreeJobs.PushBack( &m_aJobs[ i ] );
}

// $deBlob: FUNCTION 007ddb10
// $deBlob: FUNCTION 007dda60
ABuildPathJobQueue::~ABuildPathJobQueue()
{
	DestroyFreeLists();
}
