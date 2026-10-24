#include "pch.h"
#include "AAssetStreaming.h"
#include "Sound/AMusicManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0056c960
AAssetStreaming::AAssetStreaming()
{
	m_pCurrentJob  = TNULL;
	m_bPauseSounds = TFALSE;

	m_FileStream.Create( 0, TThread::THREAD_PRIORITY_ABOVE_NORMAL, 0 );

	for ( TINT i = JOB_COUNT; i != 0; i-- )
		m_FreeList.PushBack( new ATRBLoaderJob() );
}

// $deBlob: FUNCTION 0056ca70
// $deBlob: FUNCTION 0056be30
AAssetStreaming::~AAssetStreaming()
{
	m_FileStream.Destroy();

	TASSERT( m_FreeList.Size() == JOB_COUNT );

	while ( !m_FreeList.IsEmpty() )
	{
		ATRBLoaderJob* pJob = m_FreeList.Begin();
		pJob->Remove();
		delete pJob;
	}
}

// $deBlob: FUNCTION 0056cb70
void AAssetStreaming::Update( TFLOAT a_fDeltaTime )
{
	if ( m_pCurrentJob == TNULL )
	{
		if ( !m_Jobs.IsEmpty() )
		{
			ATRBLoaderJob* pJob = m_Jobs.Begin();
			pJob->Remove();
			m_pCurrentJob = pJob;

			TASSERT( !m_pCurrentJob->IsLinked() );
			m_pCurrentJob->BeginJob();
			TASSERT( !m_pCurrentJob->IsLinked() );
		}
	}
	else if ( m_pCurrentJob->m_StreamJob.IsProcessed() == TTRUE )
	{
		if ( m_pCurrentJob->m_fnOnFinished )
			m_pCurrentJob->m_fnOnFinished( m_pCurrentJob, m_pCurrentJob->m_pOnFinishedUserData );

		m_pCurrentJob->m_bFinished = TTRUE;
		m_pCurrentJob              = TNULL;

		if ( m_Jobs.IsEmpty() && AMusicManager::GetSingleton() != TNULL && m_bPauseSounds == TTRUE )
		{
			AMusicManager::GetSingleton()->PauseMusic( TTRUE, 0.0f );
			m_bPauseSounds = TFALSE;
			return;
		}
	}
	else
	{
		TASSERT( !m_pCurrentJob->IsLinked() );
	}
}

// $deBlob: FUNCTION 0056ccb0
void AAssetStreaming::AddMainThreadJob2( ATRBLoaderJob* a_pJob )
{
	TASSERT( m_pCurrentJob != a_pJob );
	m_Jobs.PushBack( a_pJob );
}

// $deBlob: FUNCTION 0056cd00
TBOOL AAssetStreaming::HasActiveJobs()
{
	return !m_Jobs.IsEmpty() || m_pCurrentJob != TNULL;
}

// $deBlob: FUNCTION 0056cd10
ATRBLoaderJob* AAssetStreaming::GetFreeJob()
{
	TASSERT( !m_FreeList.IsEmpty(), "Job free list is empty!" );

	ATRBLoaderJob* pJob = m_FreeList.Tail();
	pJob->Remove();
	pJob->Reset();

	TASSERT( !pJob->IsLinked() );
	TASSERT( m_pCurrentJob != pJob );

	return pJob;
}

// $deBlob: FUNCTION 0056ce00
void AAssetStreaming::ReleaseJob( ATRBLoaderJob* a_pJob )
{
	a_pJob->m_pTRB            = TNULL;
	a_pJob->m_szFileName[ 0 ] = '\0';
	a_pJob->Reset();

	TASSERT( m_FreeList.Size() < JOB_COUNT );
	m_FreeList.PushBack( a_pJob );
}

// $deBlob: FUNCTION 0056ce80
TINT AAssetStreaming::GetTXSNameFromTRB( const TCHAR* a_szTRB, TCHAR* a_szName, TINT a_iNameLength )
{
	TASSERT( TStringManager::String8Length( a_szTRB ) < a_iNameLength );

	TStringManager::String8Copy( a_szName, a_szTRB, -1 );
	TStringManager::String8ToLowerCase( a_szName );

	TINT iStrLen = TStringManager::String8Length( a_szName );
	TASSERT( iStrLen > 0 );

	TINT i = iStrLen - 1;

	for ( ; i >= 0; i-- )
	{
		if ( a_szName[ i ] == '.' ) break;
	}

	if ( i >= 0 )
	{
		if ( i + 4 < a_iNameLength )
			TStringManager::String8Copy( a_szName + i + 1, "txs", -1 );
	}
	else if ( a_iNameLength - iStrLen > 5 )
	{
		TStringManager::String8Copy( a_szName + iStrLen, ".txs", -1 );
	}

	for ( i = iStrLen - 1; i >= 0; i-- )
	{
		if ( a_szName[ i ] == '\\' || a_szName[ i ] == '/' )
			return i + 1;
	}

	return 0;
}

// $deBlob: FUNCTION 0056d380
ATRBLoaderJob::ATRBLoaderJob()
    : m_bFinished( TFALSE )
{
	m_pTRB                = TNULL;
	m_fnOnFinished        = TNULL;
	m_pOnFinishedUserData = TNULL;
	m_szFileName[ 0 ]     = '\0';
}

// $deBlob: FUNCTION 0056d400
void ATRBLoaderJob::Init( TTRB* a_pTRB, const TCHAR* a_szFileName )
{
	TStringManager::String8Copy( m_szFileName, a_szFileName, sizeof( m_szFileName ) );
	m_pTRB = a_pTRB;

	TASSERT( TStringManager::String8Length( a_szFileName ) > 0 );
	m_bFinished = TFALSE;
}

// $deBlob: FUNCTION 0056d470
void ATRBLoaderJob::Reset()
{
	m_fnOnFinished        = TNULL;
	m_pOnFinishedUserData = TNULL;
	m_pTRB                = TNULL;
	m_szFileName[ 0 ]     = '\0';
	m_bFinished           = TFALSE;

	// NOTE: retail also clears the processed flag of the stream job (protected in the engine, AddStream clears it)
	m_StreamJob.m_trb           = TNULL;
	m_StreamJob.m_fileName[ 0 ] = '\0';
}

// $deBlob: FUNCTION 0056d4b0
void ATRBLoaderJob::BeginJob()
{
	m_StreamJob.Init( m_pTRB, m_szFileName );
	AAssetStreaming::GetSingleton()->GetFileStream().AddStream( &m_StreamJob );
}
