#pragma once

//#include "Toshi/Core/TEvent.h"
#include "Toshi/File/TFileLexerUTF8.h"
#include <Toshi/Plugins/PPropertyParser/PProperties.h>

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

	static TBOOL OnLexerParseError(PPropertyReader* a_this, Toshi::TFileLexerUTF8* a_lexer, Toshi::TFileLexerUTF8::ParseError* a_error);

	virtual bool Open(const Toshi::TCString& a_szFileName);
	virtual bool Open(const Toshi::TCString& a_szFileName, Toshi::TFile* a_pFile);

	TBOOL LoadPropertyBlock(PProperties& a_rProps);
	void SetLoadComments(TBOOL a_bLoadComments) { m_bLoadComments = a_bLoadComments; }

	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError>* GetParseErrorEmitter() { return &m_oErrorEmitter; }

	Toshi::TCString m_szFileName;    // 0x4
	Toshi::TFile* m_pFile;           // 0xC
	Toshi::TFileLexerUTF8* m_pLexer; // 0x10
	TBOOL m_bLoadComments;           // 0x24
	TBOOL m_bAssertOnError;          // 0x25
	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError> m_oErrorEmitter;
	Toshi::TListener<PPropertyReader, Toshi::TFileLexerUTF8, Toshi::TFileLexerUTF8::ParseError> m_oErrorListener;
};

