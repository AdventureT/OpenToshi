#include "pch.h"
#include "AGUIMusicScene.h"
#include "Sound/AMusicManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0062d540
// $deBlob: FUNCTION 0062d550
// $deBlob: FUNCTION 0062d5d0
// $deBlob: FUNCTION 0062d650
// $deBlob: FUNCTION 0062d660
// $deBlob: FUNCTION 0053d250
// $deBlob: FUNCTION 00858810
TDEFINE_CLASS( AGUIMusicScene );

// $deBlob: FUNCTION 0062d6f0
AGUIMusicScene::AGUIMusicScene()
{
}

// $deBlob: FUNCTION 0062d670
// $deBlob: FUNCTION 0062d709
// $deBlob: FUNCTION 0062d720
AGUIMusicScene::~AGUIMusicScene()
{
	AMusicManager::GetSingleton()->UnregisterGUIScene( this );
}

// $deBlob: FUNCTION 0062d6e0
void AGUIMusicScene::OnCreated()
{
	// NOTE: retail doesn't call TXUIScene::OnCreated
	AMusicManager::GetSingleton()->RegisterGUIScene( this );
}
