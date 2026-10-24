#pragma once
#include "XUI/ABlobMenu.h"

//-----------------------------------------------------------------------------
// Purpose: the music track selection of the extras: it owns the "MusicMixerScene" below the front end,
// which it destroys with itself or on TransTo when m_bDestroyMixer is set (then the back scene is "ExtrasScene");
// leaving it stops the streamed music.
//-----------------------------------------------------------------------------
class ABlobTrackSelection : public ABlobMenu
{
public:
	TDECLARE_CLASS( ABlobTrackSelection, ABlobMenu );

public:
	ABlobTrackSelection();
	~ABlobTrackSelection();

	//-----------------------------------------------------------------------------
	// TXUIScene
	//-----------------------------------------------------------------------------
	virtual void TransTo() override;   // +0x114
	virtual void TransFrom() override; // +0x118

	// Destroys the "MusicMixerScene"
	void DestroyMusicMixer();

	// The next TransTo destroys the "MusicMixerScene" and goes back to "ExtrasScene" (ABlobPopupMusicMixer)
	// $deBlob: FUNCTION 0063a820
	void SetDestroyMixer() { m_bDestroyMixer = TTRUE; }

private:
	TBOOL m_bDestroyMixer; // 0x140
};
