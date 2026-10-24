#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the vistas menu of a single player level: a button per vista camera of the level
// (ANedCameraFunction::FindVistaCam 0-3) showing the rank of its
// vista score; pressing one starts the vista (APlayerManager::UpdateVista), the nav button goes on to the score menu
// (or ends the level in free paint).
//-----------------------------------------------------------------------------
class ABlobVistasMenu : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobVistasMenu, ABlobMenu );

	static TCONSTEXPR_VAR TINT MAX_VISTAS = 4;

public:
	ABlobVistasMenu();
	~ABlobVistasMenu();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                 // +0x94
	virtual TBOOL OnNotifyPress( TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransTo() override; // +0x124

private:
	Toshi::TXUIControl* m_apButtons[ MAX_VISTAS ]; // 0x140 "XuiButton1"-"XuiButton4"
	Toshi::TXUIControl* m_pNavButton;              // 0x150 "XuiNavButton1" (in-game only)
	Toshi::TXUIControl* m_pLevelName;              // 0x154 "LevelName2"
};
