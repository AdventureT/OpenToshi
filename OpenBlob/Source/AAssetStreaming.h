#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <File/TFileStream.h>

//-----------------------------------------------------------------------------
// Purpose: a TRB loaded by the file stream thread
//-----------------------------------------------------------------------------
class ATRBLoaderJob : public Toshi::T2DList<ATRBLoaderJob>::Node
{
public:
	typedef void ( *t_OnFinished )( ATRBLoaderJob* a_pJob, void* a_pUserData );

public:
	ATRBLoaderJob();
	// $deBlob: FUNCTION 0056d0f0
	~ATRBLoaderJob() = default;

	// Sets the TRB and its file
	void Init( Toshi::TTRB* a_pTRB, const TCHAR* a_szFileName );

	// Clears the job
	void Reset();

	// Adds the stream job to the file stream thread of AAssetStreaming
	void BeginJob();

public:
	TBOOL                m_bFinished;           // 0x08
	Toshi::TTRBStreamJob m_StreamJob;           // 0x0C
	Toshi::TTRB*         m_pTRB;                // 0x9C
	TCHAR                m_szFileName[ 128 ];   // 0xA0
	t_OnFinished         m_fnOnFinished;        // 0x120
	void*                m_pOnFinishedUserData; // 0x124
};

//-----------------------------------------------------------------------------
// Purpose: loads TRBs on a file stream thread, one job at a time
//-----------------------------------------------------------------------------
class AAssetStreaming : public Toshi::TSingleton<AAssetStreaming>
{
public:
	static TCONSTEXPR_VAR TINT JOB_COUNT = 256;

public:
	AAssetStreaming();
	~AAssetStreaming();

	// Starts the next job or finishes the current one
	void Update( TFLOAT a_fDeltaTime );

	// Takes a job of the free list
	ATRBLoaderJob* GetFreeJob();

	// Gives a job back to the free list
	void ReleaseJob( ATRBLoaderJob* a_pJob );

	void  AddMainThreadJob2( ATRBLoaderJob* a_pJob );
	TBOOL HasActiveJobs();

	// Copies a_szTRB (lower case) to a_szName with the extension .txs, returns the start of the file name
	// NOTE: a member in retail that doesn't use this
	static TINT GetTXSNameFromTRB( const TCHAR* a_szTRB, TCHAR* a_szName, TINT a_iNameLength );

	Toshi::TFileStream& GetFileStream() { return m_FileStream; }

private:
	Toshi::T2DList<ATRBLoaderJob> m_Jobs;         // 0x00
	ATRBLoaderJob*                m_pCurrentJob;  // 0x08
	Toshi::TFileStream            m_FileStream;   // 0x0C
	TBOOL                         m_bPauseSounds; // 0xD8 pauses the sounds of AMovieState when the jobs are done
	Toshi::T2DList<ATRBLoaderJob> m_FreeList;     // 0xDC
};
