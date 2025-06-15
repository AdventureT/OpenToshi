#include "ToshiPCH.h"
#include "TLogFile.h"

#ifdef TOSHI_SKU_WINDOWS
#  include "Platform/Windows/TNativeFile_Win.h"
#endif // TOSHI_SKU_WINDOWS

#include <DbgHelp.h>

TOSHI_NAMESPACE_BEGIN

TLogFile::TLogFile()
    : m_LevelString(), m_typeCounts()
{
	m_pFile             = TNULL;
	m_iTotalLogCount    = 0;
	m_bIsSimpleMode     = TFALSE;
	m_bAllowIndentation = TFALSE;
	m_curLevel          = 0;
	m_unk2              = TNULL;
	m_unk3              = 0;
}

TLogFile::Error TLogFile::Create(const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting)
{
	TASSERT(TNULL == m_pFile);

	if (m_pFile != TNULL)
	{
		return Error::AlreadyCreated;
	}

	TString8 drive;
	TString8 dir;

	TCHAR* path = TNativeFile::SplitPath(fileName, drive, dir);
	TNativeFile::DirExists(path);

	TFile::FileMode fileMode = writeExisting ? TFile::FileMode_NoBuffer | TFile::FileMode_Write : TFile::FileMode_NoBuffer | TFile::FileMode_Write | TFile::FileMode_CreateNew;

	m_pFile = TFile::Create(fileName, fileMode);

	if (m_pFile == TNULL)
	{
		return Error::OpenFile;
	}

	if (writeExisting)
	{
		m_pFile->Seek(0, TFile::TSEEK_END);
	}

	m_iTotalLogCount = 0;
	m_LevelString[0] = '\0';
	m_curLevel       = 0;

	m_pFile->CPrintf("Log created [%s]:[%s]: %s\n", fileName, str2, TUtil::GetTime());
	m_pFile->CPrintf("Compilation: %s\n", __TIMESTAMP__);

	return Error::OK;
}

void TLogFile::Close()
{
	m_curLevel = 0;
	RecalcLevel();

	for (size_t i = 0; i < 3; i++)
	{
		Print("%s count = %d\n", cTypeStrings[i], m_typeCounts[i]);
	}

	if (m_pFile != TNULL)
	{
		m_pFile->Destroy();
		m_pFile = TNULL;
	}
}

void TLogFile::RecalcLevel()
{
	m_curLevel = m_curLevel < cLevelMax - 1 ? m_curLevel : cLevelMax;
	m_curLevel = TMath::Max<TUINT32>(m_curLevel, 0);

	for (size_t i = 0; i < m_curLevel; i++)
	{
		m_LevelString[i] = '\t';
	}

	m_LevelString[m_curLevel] = '\0';
}

void TLogFile::Print(const TCHAR* format, ...)
{
	if (m_pFile != TNULL)
	{
		va_list args;
		va_start(args, format);

		TCHAR str[1024];
		T2String8::FormatV(str, sizeof(str), format, args);

		va_end(args);

		if (m_bAllowIndentation)
		{
			m_pFile->CPrintf("%s%s", m_LevelString, str);
		}
		else
		{
			m_pFile->CPrintf(str);
		}
	}
}



void TLogFile::Log(Type type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ...)
{
	if (m_pFile != TNULL)
	{
		if (m_bIsSimpleMode)
		{
			if (m_bAllowIndentation)
			{
				m_pFile->CPrintf("%d [%s]: %s", m_iTotalLogCount, cTypeStrings[type], m_LevelString);
			}
		}
		else
		{
			TCHAR strTime[9];
			_strtime(strTime);
			m_pFile->CPrintf("%d [%s] [%s]: %s: %s: %s", m_iTotalLogCount, cTypeStrings[type], strTime, str1, str2 != TNULL ? str2 : "", m_bAllowIndentation ? m_LevelString : "");
		}

		va_list args;
		va_start(args, format);
		m_pFile->VCPrintf(format, args);
		va_end(args);
		m_pFile->CPrintf("\n");
		m_typeCounts[type]++;
		m_iTotalLogCount++;
	}
}

TOSHI_NAMESPACE_END
