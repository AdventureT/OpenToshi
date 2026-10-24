#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the save/load message popup (saveloadmessage*.xur): a scene that only tints its nine grid.
//-----------------------------------------------------------------------------
class ABlobSaveLoadMessage : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobSaveLoadMessage, Toshi::TXUIScene );

public:
	ABlobSaveLoadMessage();
	~ABlobSaveLoadMessage();

	void SetMessageText( const TWCHAR* a_wszText );

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void OnCreated() override; // +0x94

private:
	Toshi::TXUIScene* m_pParentScene; // 0x130 not initialised by the ctor
};
