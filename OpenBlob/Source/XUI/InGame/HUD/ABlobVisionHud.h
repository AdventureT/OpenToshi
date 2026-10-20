#pragma once
#include <XUI/TXUIScene.h>

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: the vision HUD of the single player (scene "VisionHud"): while the blob sees through its
// vision (the camera's vision mode, AGameStatus/APlayerManager's vision state) it sets the vision colour from the
// level's colour table (by the camera's index) and plays the "Normal" animation.
//-----------------------------------------------------------------------------
class ABlobVisionHud : public Toshi::TXUIScene
{
public:
	TDECLARE_CLASS( ABlobVisionHud, Toshi::TXUIScene );

public:
	ABlobVisionHud();
	~ABlobVisionHud();

	//-----------------------------------------------------------------------------
	// TXUIElement
	//-----------------------------------------------------------------------------
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void OnCreated() override;                 // +0x94

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransFrom() override;        // +0x118
	virtual void TransBackTo() override;      // +0x11C
	virtual void EndTransBackTo() override;   // +0x12C
	virtual void EndTransBackFrom() override; // +0x130

	// Set when the blob dies in the blob vision (ABlobEntity::OnOutOfPoints)
	void SetUnk130() { m_bUnk130 = TTRUE; }

private:
	void UpdateColour( ABlobEntity* a_pBlob );

	TBOOL   m_bUnk130;        // 0x130
	TUINT32 m_uiVisionColour; // 0x134 the vision colour index (0..0x10)
};
