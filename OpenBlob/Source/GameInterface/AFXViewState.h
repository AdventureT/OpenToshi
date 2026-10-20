#pragma once
#include "AGameState.h"
#include "ALevelInformation.h"
#include "FXComposition/AFXCompositionInstance.h"
#include "Input/AInputHelper.h"

#include <GUI/T2GUITextBox.h>

class ATRBLoaderJob;

namespace Toshi
{
class TTRB;
}

//-----------------------------------------------------------------------------
// Purpose: the FX composition previewer: loads the "FxPreviewLevel" level's data, the FX particles, the fxpreview
// environment settings and the level's fxdata.trb, then plays the "fxpreview" composition, rotating it around the
// origin, with keys to trigger, stop it and move the camera. Pushed by AFrontEndMovieState instead of the frontend when
// g_oTheApp.m_bFXView is set (a command line switch). Uses AGameState's TClass.
//-----------------------------------------------------------------------------
class AFXViewState : public AGameState
{
public:
	// The loading steps (m_eState)
	enum STATE
	{
		STATE_LoadLevel,
		STATE_LoadRegion,
		STATE_LoadRegionSounds,
		STATE_LoadParticles,
		STATE_LoadEnvFX,
		STATE_LoadFXData,
		STATE_Running,
	};

public:
	AFXViewState();
	~AFXViewState();

	virtual UpdateResult OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void         OnInsertion() override;
	virtual void         OnRemoval() override;
	virtual void         OnActivate() override;
	virtual void         OnDeactivate() override;

private:
	void LoadLevel();

private:
	TINT                                m_eState;       // 0x2C STATE
	AFXCompositionInstance              m_oComposition; // 0x30
	AInputHelper                        m_oInputHelper; // 0x118
	TBOOL                               m_bRotating;    // 0x13C
	TBOOL                               m_bShowHelp;    // 0x13D
	TFLOAT                              m_fTime;        // 0x140
	Toshi::T2GUITextBox                 m_oStatusText;  // 0x144
	Toshi::T2GUITextBox                 m_oHelpText;    // 0x1AC
	ALevelInformation::LevelProperties* m_levelprops;   // 0x214
	ATRBLoaderJob*                      m_pTRBJob;      // 0x218
	Toshi::TTRB*                        m_TRB;          // 0x21C
};
