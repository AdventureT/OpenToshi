#pragma once
#include <Toshi/TSingleton.h>

namespace Toshi
{
class TFile;
class TTRB;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the singleton that loads the IO templates (the TRB of the level's IO data) and owns the TTRB until the next
// load.
// NOTE: retail's ctor and dtor assert on ms_pSingleton, so this is a TSingleton. The ctor and dtor carry those asserts
// here, TSingleton itself doesn't.
// NOTE: no code in retail's.text calls the ctor or Load: no call, jump or immediate reference. Only the dtor is
// referenced, from the scalar deleting wrapper. AIOEntity's setup (sub_783820) reads ms_pSingleton, so the loader that
// would create it is not in this build's code.
//-----------------------------------------------------------------------------
class AIOTemplateManager
    : public Toshi::TSingleton<AIOTemplateManager>
{
public:
	AIOTemplateManager();
	virtual ~AIOTemplateManager(); // +0x00

	// Loads the templates from the file a_szFileName (it replaces the previous TTRB)
	void Load( const TCHAR* a_szFileName );

	// Returns the IO data of the template a_szName (a file name with a 4 character extension)
	void* GetIOTemplateReference( const TCHAR* a_szName );

protected:
	Toshi::TTRB* m_pTRB; // 0x04
};                       // 0x08
