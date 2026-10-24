#pragma once
#include "ABlobNameEntryCommon.h"

//-----------------------------------------------------------------------------
// Purpose: the Korean profile "name entry": no typing,
// the name is string 472 formatted with the profile number; "btnOK" creates the profile.
//-----------------------------------------------------------------------------
class ABlobKONameSelect : public ABlobNameEntryCommon
{
public:
	TDECLARE_CLASS( ABlobKONameSelect, ABlobMenu );

public:
	ABlobKONameSelect();
	~ABlobKONameSelect();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  OnCreated() override;                                                        // +0x94
	virtual TBOOL OnNotifyPress( Toshi::TXUIElement* a_pSource, TINT a_iController ) override; // +0xB0

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void EndTransTo() override; // +0x124

	void SetProfileNumber( TINT a_iProfileNumber );

private:
	TINT                m_iProfileNumber; // 0x478 -1
	Toshi::TXUIElement* m_pOK;            // 0x47C "btnOK"
	Toshi::TXUIElement* m_pBack;          // 0x480 "XuiBackButton1"
	Toshi::TXUIElement* m_pSaveDisplay;   // 0x484 "lblSaveDisplay"
};
