#include "ToshiPCH.h"
#include "TFileStream.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00686730
// $Barnyard: FUNCTION 006d7ef0
TFileStream::TFileStream()
{
}

// $deBlob: FUNCTION 00686760
// $deBlob: FUNCTION 006867f0
// $Barnyard: FUNCTION 006d7f20
// $Barnyard: FUNCTION 006d7fb0
TFileStream::~TFileStream()
{
}

// $deBlob: FUNCTION 00686790
// $Barnyard: FUNCTION 006d7f30
void TFileStream::Main()
{
	TFileStreamJob* pJob;

	while ( TTRUE )
	{
		do
		{
			m_Jobs.Pop( pJob );
			pJob->Process();
			pJob->m_bIsProcessed = TTRUE;
		} while ( pJob->m_pSemaphore == TNULL );

		pJob->m_pSemaphore->Signal();
	}
}

// $deBlob: FUNCTION 00686770
// $Barnyard: FUNCTION 006d7f70
void TFileStream::AddStream( TFileStreamJob* job )
{
	TPROFILER_SCOPE();
	job->m_bIsProcessed = TFALSE;
	m_Jobs.Push( &job );
}

//-----------------------------------------------------------------------------
// TTRBStreamJob
//-----------------------------------------------------------------------------

const TCHAR* TTRBStreamJob::GetFileName() const
{
	return m_fileName;
}

TTRB* TTRBStreamJob::GetTRB() const
{
	return m_trb;
}

// $deBlob: FUNCTION 006866f0
// $Barnyard: FUNCTION 006d7ec0
void TTRBStreamJob::Init( TTRB* trb, const TCHAR* fileName )
{
	m_trb = trb;
	T2String8::Copy( m_fileName, fileName, -1 );
}

// $deBlob: FUNCTION 00686710
// $Barnyard: FUNCTION 006d7ee0
void TTRBStreamJob::Process()
{
	TPROFILER_SCOPE();
	m_trb->Load( m_fileName );
}

// $deBlob: FUNCTION 006866d0
// $Barnyard: FUNCTION 006d7ea0
TTRBStreamJob::TTRBStreamJob()
    : TFileStreamJob( TNULL )
{
	m_trb           = TNULL;
	m_fileName[ 0 ] = '\0';
}

// $deBlob: FUNCTION 00686820
TTRBStreamJob::~TTRBStreamJob()
{
}

TOSHI_NAMESPACE_END
