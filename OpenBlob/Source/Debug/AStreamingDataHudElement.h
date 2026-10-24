#pragma once
#include <Toshi/TSingleton.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUITextBox.h>

class ADebugLog;

//-----------------------------------------------------------------------------
// Purpose: a debug HUD of the streaming (a T2GUIElement singleton): 18 lines of text in the safe area about the terrain
// cell queue, FMOD's memory and disc reads, the positional sounds and the music stream, updated by Update while
// enabled.
// NOTE: retail never creates it (nothing references the constructor), so it's dead code kept as retail has it.
//-----------------------------------------------------------------------------
class AStreamingDataHudElement
    : public Toshi::T2GUIElement
    , public Toshi::TSingleton<AStreamingDataHudElement>
{
public:
	static TCONSTEXPR_VAR TINT TEXT_LENGTH = 0x80;
	static TCONSTEXPR_VAR TINT NUM_LINES   = 18;

public:
	AStreamingDataHudElement();
	virtual ~AStreamingDataHudElement();

	// Creates the text boxes: the first one in the safe area, each next one below the previous one
	void CreateTextBoxes();

	// Forgets the statistics
	void Reset();

	// Updates the texts and the statistics (only hides the first text box and resets them while disabled)
	void Update( TFLOAT a_fDeltaTime );

	// The FMOD disc read callbacks (a_pUserData: the element)
	static void __stdcall OnFMODReadStart( void* a_pUserData, TINT a_iSize );
	static void __stdcall OnFMODReadEnd( void* a_pUserData );

	void OnFMODReadStart( TINT a_iSize );
	void OnFMODReadEnd();

	// Empties the log file (SFS.csv)
	void CreateLogFile();

	// $deBlob: FUNCTION 005b32a0
	void SetUnk1A90( TBOOL a_bUnk ) { m_bUnk1A90 = a_bUnk; }

private:
	TWCHAR              m_wszCurrentJob[ TEXT_LENGTH ];           // 0x003C
	Toshi::T2GUITextBox m_oCurrentJob;                            // 0x013C
	TWCHAR              m_wszFMODMemory[ TEXT_LENGTH ];           // 0x01A4
	TWCHAR              m_wszUnk2A4[ TEXT_LENGTH ];               // 0x02A4 (not used)
	Toshi::T2GUITextBox m_oFMODMemory;                            // 0x03A4
	TWCHAR              m_wszMusicRead[ TEXT_LENGTH ];            // 0x040C
	TWCHAR              m_wszTerrainTimeSinceRead[ TEXT_LENGTH ]; // 0x050C
	Toshi::T2GUITextBox m_aMusicRead[ 2 ];                        // 0x060C (the music read and the terrain time since read)
	TWCHAR              m_wszMusicPeakTime[ TEXT_LENGTH ];        // 0x06DC
	Toshi::T2GUITextBox m_oMusicPeakTime;                         // 0x07DC
	TWCHAR              m_wszTerrainQueueTimer[ TEXT_LENGTH ];    // 0x0844
	Toshi::T2GUITextBox m_oTerrainQueueTimer;                     // 0x0944
	TWCHAR              m_wszTerrainQueuePeak[ TEXT_LENGTH ];     // 0x09AC
	Toshi::T2GUITextBox m_oTerrainQueuePeak;                      // 0x0AAC
	TWCHAR              m_wszQueueSize[ TEXT_LENGTH ];            // 0x0B14
	Toshi::T2GUITextBox m_oQueueSize;                             // 0x0C14
	TWCHAR              m_wszFMODReadSize[ TEXT_LENGTH ];         // 0x0C7C
	Toshi::T2GUITextBox m_oFMODReadSize;                          // 0x0D7C
	TWCHAR              m_wszFMODReadTime[ TEXT_LENGTH ];         // 0x0DE4
	Toshi::T2GUITextBox m_oFMODReadTime;                          // 0x0EE4
	TWCHAR              m_wszNumPositionals[ TEXT_LENGTH ];       // 0x0F4C
	Toshi::T2GUITextBox m_oNumPositionals;                        // 0x104C
	TWCHAR              m_wszNumFreePositionals[ TEXT_LENGTH ];   // 0x10B4
	Toshi::T2GUITextBox m_oNumFreePositionals;                    // 0x11B4
	TWCHAR              m_wszTotalPositionals[ TEXT_LENGTH ];     // 0x121C
	Toshi::T2GUITextBox m_oTotalPositionals;                      // 0x131C
	TWCHAR              m_wszMusicBeat[ TEXT_LENGTH ];            // 0x1384
	TWCHAR              m_wszMusicPosition[ TEXT_LENGTH ];        // 0x1484
	Toshi::T2GUITextBox m_aMusic[ 2 ];                            // 0x1584 (the beat and the position)
	TWCHAR              m_wszMusicOpenState[ TEXT_LENGTH ];       // 0x1654
	Toshi::T2GUITextBox m_oMusicOpenState;                        // 0x1754
	TWCHAR              m_wszUnk17BC[ TEXT_LENGTH ];              // 0x17BC (never written)
	Toshi::T2GUITextBox m_oUnk17BC;                               // 0x18BC
	TWCHAR              m_wszDiscRead[ TEXT_LENGTH ];             // 0x1924
	Toshi::T2GUITextBox m_oDiscRead;                              // 0x1A24
	ADebugLog*          m_pLog;                                   // 0x1A8C ("SFS.csv")
	TBOOL               m_bUnk1A90;                               // 0x1A90
	TBOOL               m_bEnabled;                               // 0x1A91
	TFLOAT              m_fTerrainTimeSinceRead;                  // 0x1A94
	TFLOAT              m_fFMODTimeSinceRead;                     // 0x1A98
	TFLOAT              m_fTerrainQueueTimer;                     // 0x1A9C
	TFLOAT              m_fTerrainQueuePeak;                      // 0x1AA0
	TFLOAT              m_fFMODTimeSinceReadPeak;                 // 0x1AA4
	TINT                m_iUnk1AA8;                               // 0x1AA8
	TINT                m_iUnk1AAC;                               // 0x1AAC
	TINT                m_iQueueSize;                             // 0x1AB0
	TINT                m_iQueuePeak;                             // 0x1AB4
	TINT                m_iFMODReadSize;                          // 0x1AB8
	TINT                m_iFMODReadPeak;                          // 0x1ABC
	TINT                m_iNumFMODReads;                          // 0x1AC0
	TINT                m_iNumFMODReadsPeak;                      // 0x1AC4
	TBOOL               m_bFMODReading;                           // 0x1AC8
	TFLOAT              m_fFMODReadTime;                          // 0x1ACC
	TFLOAT              m_fFMODReadTimePeak;                      // 0x1AD0
	TFLOAT              m_fRecreateTime;                          // 0x1AD4
	TUINT16             m_uiUnk1AD8;                              // 0x1AD8
};                                                                // 0x1ADC
