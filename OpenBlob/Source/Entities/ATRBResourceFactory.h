#pragma once
#include "AStack.h"

#include <Toshi/TSingleton.h>

class ATRBResource;

//-----------------------------------------------------------------------------
// Purpose: the shared TRB resources (AResourceFactory.cpp, no virtual functions), created by AEntityFramework::Startup
// (and AFXViewState, AModelViewState). Retail never adds a resource, the functions besides the constructor and
// destructor are unused.
//-----------------------------------------------------------------------------
class ATRBResourceFactory
    : public Toshi::TSingleton<ATRBResourceFactory>
{
public:
	static TCONSTEXPR_VAR TINT MAX_RESOURCES = 128;

public:
	ATRBResourceFactory();
	~ATRBResourceFactory();

	// The resource named a_szName (loaded if it isn't), TNULL if none
	ATRBResource* Imp_GetResource( const TCHAR* a_szName );

	// Drops a reference of a_pResource, the last one unloads it
	void DestroyResource( ATRBResource* a_pResource );

	// Reloads the loaded resources flagged for it
	void ReloadResources();

private:
	AStack<ATRBResource*, MAX_RESOURCES> m_oResources; // 0x00
};                                                     // 0x204
