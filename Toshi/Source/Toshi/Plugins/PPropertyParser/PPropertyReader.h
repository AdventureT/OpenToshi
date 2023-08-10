#pragma once

#include "Toshi/Core/TEvent.h"

class PPropertyReaderParseError
{


};

class PPropertyReader 
	: public Toshi::TGenericClassDerived<PPropertyReader, Toshi::TObject, "PPropertyReader", TMAKEVERSION(1, 0), TTRUE>
{
public:
	PPropertyReader()
	{

	}

	virtual bool Open(const Toshi::TCString& a_szFileName);
	virtual bool Open(const Toshi::TCString& a_szFileName, Toshi::TFile* a_pFile);

	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError>* GetParseErrorEmitter() { return &m_oErrorEmitter; }


	Toshi::TEmitter<PPropertyReader, PPropertyReaderParseError> m_oErrorEmitter;
};

