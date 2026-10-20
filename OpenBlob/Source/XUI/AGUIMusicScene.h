#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: a scene that registers itself with AMusicManager when it's created (AMusicManager::RegisterGUIScene) and
// unregisters on destruction.
//-----------------------------------------------------------------------------
class AGUIMusicScene : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( AGUIMusicScene, Toshi::TXUIScene );

public:
	AGUIMusicScene();
	~AGUIMusicScene();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94
};
