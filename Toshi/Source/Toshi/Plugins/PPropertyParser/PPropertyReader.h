#pragma once

//#include "Toshi/Core/TEvent.h"
#include "Toshi/File/TFileLexerUTF8.h"
#include <Toshi/Plugins/PPropertyParser/PProperties.h>

class PPropertyReaderParseError
{
public:
	PPropertyReaderParseError() = default;

	~PPropertyReaderParseError() = delete;

	PPropertyReaderParseError(const PPropertyReaderParseError& cpy)
	{
		m_Token.assign(cpy.m_Token);
		m_iUnk = cpy.m_iUnk;
		m_iUnk2 = cpy.m_iUnk2;
	}

private:
	Toshi::TFileLexerUTF8::Token m_Token; // 0x0
	int m_iUnk;                           // 0xC
	int m_iUnk2;                          // 0x10

};

class PPropertyReader 
	: public Toshi::TGenericClassDerived<PPropertyReader, Toshi::TObject, "PPropertyReader", TMAKEVERSION(1, 0), TTRUE>
{
public:
	PPropertyReader() : m_oErrorEmitter(this)
	{
		m_pFile = TNULL;
		m_pLexer = TNULL;
		m_bLoadComments = TFALSE;
		m_bAssertOnError = TTRUE;
	}

	static TBOOL OnLexerParseError(PPropertyReader* a_this, Toshi::TFileLexerUTF8* a_lexer, Toshi::TFileLexerUTF8::ParseError* a_error);

	virtual bool Open(const Toshi::TCString& a_szFileName);
	virtual bool Open(const Toshi::TCString& a_szFileName, Toshi::TFile* a_pFile);
	void Close();

	TBOOL LoadPropertyBlock(PProperties& a_rProps);
	TBOOL LoadProperty(PProperties* a_pProps);

	void SetLoadComments(TBOOL a_bLoadComments) { m_bLoadComments = a_bLoadComments; }
	void SetAssertOnError(TBOOL a_bAssertOnError) { m_bAssertOnError = a_bAssertOnError; }

	Toshi::TFileLexerUTF8* GetLexer() { return m_pLexer; }
	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError>* GetParseErrorEmitter() { return &m_oErrorEmitter; }

	Toshi::TCString m_szFileName;                                                                                 // 0x4
	Toshi::TFile* m_pFile;                                                                                        // 0xC
	Toshi::TFileLexerUTF8* m_pLexer;                                                                              // 0x10
	TBOOL m_bLoadComments;                                                                                        // 0x24
	TBOOL m_bAssertOnError;                                                                                       // 0x25
	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError> m_oErrorEmitter;                                  // 0x38
	Toshi::TListener<PPropertyReader, Toshi::TFileLexerUTF8, Toshi::TFileLexerUTF8::ParseError> m_oErrorListener; // 0x44
};

