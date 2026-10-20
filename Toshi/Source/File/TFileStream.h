/**
 * @file TFileStream.h
 * File stream interface for the Toshi engine
 */

#pragma once
#include "Toshi/TFifo.h"
#include "Thread/TThread.h"
#include "File/TTRB.h"

TOSHI_NAMESPACE_START

class TFileStreamJob
{
public:
	friend class TFileStream;

public:
	// $deBlob: FUNCTION 006867d0
	TFileStreamJob( TSemaphore* semaphore )
	    : m_pSemaphore( semaphore ), m_bIsProcessed( TFALSE ) {}

	// $Barnyard: FUNCTION 006d7f90
	// $Barnyard: FUNCTION 00401170
	// $deBlob: FUNCTION 0056d150
	virtual ~TFileStreamJob() = default;
	virtual void Process()    = 0;

	TBOOL IsProcessed() const { return m_bIsProcessed; }

protected:
	TSemaphore* m_pSemaphore;   // 0x04
	TBOOL       m_bIsProcessed; // 0x08
};

class TFileStream : public TThread
{
public:
	TFileStream();

	// This method will be executed by the thread
	virtual void Main() override;

	virtual ~TFileStream();

	// Adds job to the FIFO
	void AddStream( TFileStreamJob* job );

private:
	TFifo<TFileStreamJob*, 32> m_Jobs; // 0x14
};

class TTRBStreamJob : public TFileStreamJob
{
public:
	TTRBStreamJob();
	virtual ~TTRBStreamJob();

	virtual void Process();

	void Init( TTRB* trb, const TCHAR* fileName );

	TTRB*        GetTRB() const;
	const TCHAR* GetFileName() const;

public:
	TTRB* m_trb;             // 0x0C
	TCHAR m_fileName[ 128 ]; // 0x10 (64 in Barnyard; de Blob: 128, the job is 0x90 bytes in ATRBLoaderJob)
};

TOSHI_NAMESPACE_END
