#pragma once
#include <Toshi/TManagedPointer.h>
#include <Toshi/TString8.h>
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: a TRB file kept by ATRBResourceFactory (AResourceFactory.cpp): <path>/<name><extension>, its "Main" symbol
// is the default data. Abstract: retail has no derived class (no vtable is emitted) and nothing creates one.
//-----------------------------------------------------------------------------
class ATRBResource
    : public Toshi::TRefCounted
{
public:
	virtual ~ATRBResource()             = default; // +0x00
	virtual const TCHAR* GetExtension() = 0;       // +0x04 appended to the file name

	// (Re)loads the TRB and finds its "Main" symbol
	void Reload();

	// The symbol a_szSymbol of the TRB, the "Main" data if it has none, TNULL if the TRB isn't loaded
	void* GetTRBData( const TCHAR* a_szSymbol );

	// $deBlob: FUNCTION 0078d430
	const Toshi::TString8& GetName() const { return m_strName; }
	// $deBlob: FUNCTION 0078d420
	const Toshi::TString8& GetPath() const { return m_strPath; }
	TBOOL                  IsLoaded() const { return m_bLoaded; }
	// $deBlob: FUNCTION 0078d3d0
	TBOOL ShouldReload() const { return m_bReload; }

	// The last reference is gone: the TRB is closed (ATRBResourceFactory::DestroyResource)
	void Unload()
	{
		m_oTRB.Close();
		m_bLoaded = TFALSE;
		m_pMain   = TNULL;
	}

private:
	Toshi::TString8 m_strName; // 0x08
	Toshi::TString8 m_strPath; // 0x14 the folder
	Toshi::TTRB     m_oTRB;    // 0x20
	TBOOL           m_bLoaded; // 0x34
	void*           m_pMain;   // 0x38 the "Main" symbol
	TBOOL           m_bReload; // 0x3C ATRBResourceFactory::ReloadResources reloads it
};                             // 0x40
