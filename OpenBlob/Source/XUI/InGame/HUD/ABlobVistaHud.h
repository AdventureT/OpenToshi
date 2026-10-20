#pragma once
#include <XUI/TXUIScene.h>

//-----------------------------------------------------------------------------
// Purpose: the vista HUD of the single player (scene "VistaHud" of vista.xur): the vista's score panel.
// It finds its elements (ids 9, 0xA, 0xC, 0xD, 0xE), shows the rank of the vista score, and registers itself in
// APlayerManager (m_pVistaHud) while it exists; its Tick runs APlayerManager::UpdateVista over the in-game scene.
//-----------------------------------------------------------------------------
class ABlobVistaHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobVistaHud, Toshi::TXUIScene );

public:
	ABlobVistaHud();
	~ABlobVistaHud();

	void TransBackFromVista();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                                            // +0x04
	virtual TBOOL Create( Toshi::TXUIResource& a_rResource, Toshi::XURXUIElementData* a_pData, TBOOL a_bCreateChildren ) override; // +0x5C
	virtual void  OnCreated() override;                                                                                            // +0x94
	virtual TBOOL HandleButton( TINT a_eButton, TINT a_iController ) override;                                                     // +0xF8

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Shows a_iScore in m_pUnk138 (locale string 0xBE)
	void SetScore( TINT a_iScore );

	// $deBlob: FUNCTION 0065bf30
	void ShowUnk13C( TBOOL a_bVisible ) { m_pUnk13C->SetVisible( a_bVisible ); }

	// $deBlob: FUNCTION 0065bf40
	void ShowUnk140( TBOOL a_bVisible ) { m_pUnk140->SetVisible( a_bVisible ); }

	// $deBlob: FUNCTION 0065bf50
	void ShowUnk130( TBOOL a_bVisible ) { m_pUnk130->SetVisible( a_bVisible ); }

private:
	Toshi::TXUIControl* m_pUnk130;        // 0x130 element 0xE (shows the rank of the vista score)
	Toshi::TXUIElement* m_pUnk134;        // 0x134 element 0xD
	Toshi::TXUIControl* m_pUnk138;        // 0x138 control 0xA (shows m_awszText)
	Toshi::TXUIElement* m_pUnk13C;        // 0x13C element 0xC (hidden by OnCreated)
	Toshi::TXUIElement* m_pUnk140;        // 0x140 element 9
	TINT                m_iUnk144;        // 0x144
	TWCHAR              m_awszText[ 32 ]; // 0x148 the score (SetScore)
};
