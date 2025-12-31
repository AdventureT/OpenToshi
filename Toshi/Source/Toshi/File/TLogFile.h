#pragma once
#include "Toshi/File/TFile.h"

TOSHI_NAMESPACE_START

class TLogFile : public TSingleton<TLogFile>
{
private:
	static constexpr TUINT32 cLevelMax = 30;

public:
	enum Type
	{
		Type_Info,
		Type_Warning,
		Type_Error,
		Type_NUMOF,
	};

	static constexpr const TCHAR* cTypeStrings[]{ "Info", "Warning", "Error" };

	static_assert(Type_NUMOF == sizeof(cTypeStrings) / sizeof(TCHAR*), "There are not enough/too many type strings in the array");

	enum class Error
	{
		OK,
		OpenFile,
		AlreadyCreated
	};

public:
	TLogFile();

	// $deBlob: FUNCTION 00687780
	~TLogFile()
	{
		TASSERT(TNULL == m_pFile, "TFile must be NULL");
		Close();
	}

	Error Create(const TCHAR* fileName, const TCHAR* str2, TBOOL writeExisting);
	void  Close();
	void  RecalcLevel();
	void  Print(const TCHAR* format, ...);

	void Log(Type type, const TCHAR* str1, const TCHAR* str2, const TCHAR* format, ...);

	void Down()
	{
		m_curLevel--;
		RecalcLevel();
	}

	void Up()
	{
		m_curLevel++;
		RecalcLevel();
	}

	// $deBlob: FUNCTION 00687950
	void AllowIndentation(TBOOL allow) { m_bAllowIndentation = allow; }

	// $deBlob: FUNCTION 00687940
	void SetSimpleMode(TBOOL enable) { m_bIsSimpleMode = enable; }

	// $deBlob: FUNCTION 00687b70
	TINT GetTypeCount(Type type) const { return m_typeCounts[type]; }

	// $deBlob: FUNCTION 00687b90
	TINT GetTotalLogCount() const { return m_iTotalLogCount; }

	// $deBlob: FUNCTION 00687b80
	const TCHAR* GetLevelString() const { return m_LevelString; }

private:
	TFile*  m_pFile;                  // 0x00
	TINT    m_iTotalLogCount;         // 0x04
	TBOOL   m_bIsSimpleMode;          // 0x08
	TBOOL   m_bAllowIndentation;      // 0x09
	TCHAR   m_LevelString[cLevelMax]; // 0x0A
	TUINT32 m_curLevel;               // 0x2C
	TINT    m_typeCounts[Type_NUMOF]; // 0x30
	TINT*   m_unk2;                   // 0x34
	TINT    m_unk3;                   // 0x38
};

TOSHI_NAMESPACE_END
