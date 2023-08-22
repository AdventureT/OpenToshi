#pragma once

#include "Toshi/Core/TEvent.h"
#include "Toshi/File/TFileLexerUTF8.h"

class PPropertyReaderParseError
{
	PPropertyReaderParseError()
	{

	}

};

class PPropertyReader 
	: public Toshi::TGenericClassDerived<PPropertyReader, Toshi::TObject, "PPropertyReader", TMAKEVERSION(1, 0), TTRUE>
{
public:
	PPropertyReader() : m_oErrorEmitter(this)
	{
		m_pFile = TNULL;
		m_pLexer = TNULL;
	}

	virtual bool Open(const Toshi::TCString& a_szFileName);
	virtual bool Open(const Toshi::TCString& a_szFileName, Toshi::TFile* a_pFile);

	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError>* GetParseErrorEmitter() { return &m_oErrorEmitter; }

	Toshi::TCString m_szFileName;    // 0x4
	Toshi::TFile* m_pFile;           // 0xC
	Toshi::TFileLexerUTF8* m_pLexer; // 0x10
	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError> m_oErrorEmitter;
	
};

