#pragma once

#include <Toshi/File/TFileStream.h>
#include "AAssetStreaming.h"

using namespace Toshi;

class ATRBLoaderJob : public AMainThreadJob2
{
public:
	// $deBlob: FUNCTION 0056d380
	ATRBLoaderJob()
	{
	}

	virtual ~ATRBLoaderJob() = default;

	virtual JOBTYPE GetJobType()
	{
		return JOBTYPE::TRB;
	}

	// $deBlob: FUNCTION 0056d400
	virtual void Init(TTRB* trb, const char* a_szFilename)
	{
		m_StreamJob.Init(trb, a_szFilename);
		TASSERT(TStringManager::String8Length(a_szFilename) > 0);
	}

	// $deBlob: FUNCTION 0056d4b0
	virtual void BeginJob()
	{
		m_StreamJob.Init(m_StreamJob.m_trb, m_StreamJob.m_fileName);
		AAssetStreaming::GetSingleton()->GetFileStream().AddStream(&m_StreamJob);
	}

	virtual TBOOL RunJob()
	{
		return m_IsRunning;
	}
};
