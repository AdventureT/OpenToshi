#include "pch.h"
#include "AStreamingDataHudElement.h"
#include "ADebugLog.h"
#include "GUI/AGUIManager.h"
#include "GUI/AFontManager.h"
#include "Sound/APositionalSoundManager.h"
#include "Sound/AMusicManager.h"
#include "Terrain2/ATerrain2.h"

#include <Toshi/TStringManager.h>
#include <Math/TMathInline.h>

#include <fmod.hpp>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AStreamingDataHudElement ) == 0x1ADC );

// The size of FMOD's memory the usage is shown against (12 MB)
static TINT s_iFMODMemorySize = 0xC00000;

// $deBlob: FUNCTION 005b2570
AStreamingDataHudElement::AStreamingDataHudElement()
{
	m_bEnabled = TFALSE;
	m_pLog     = new ADebugLog( "SFS", "csv", -1 );
	Reset();
}

// $deBlob: FUNCTION 005b2750
// $deBlob: FUNCTION 005b3350
AStreamingDataHudElement::~AStreamingDataHudElement()
{
	// NOTE: retail unlinks every text box but the last one (m_oDiscRead) and never deletes m_pLog
	T2GUITextBox* apTextBoxes[] = {
		&m_oCurrentJob,
		&m_oFMODMemory,
		&m_aMusicRead[ 0 ],
		&m_oMusicPeakTime,
		&m_aMusicRead[ 1 ],
		&m_oTerrainQueueTimer,
		&m_oTerrainQueuePeak,
		&m_oQueueSize,
		&m_oFMODReadSize,
		&m_oFMODReadTime,
		&m_oNumPositionals,
		&m_oNumFreePositionals,
		&m_oTotalPositionals,
		&m_aMusic[ 0 ],
		&m_aMusic[ 1 ],
		&m_oMusicOpenState,
		&m_oUnk17BC,
	};

	for ( TINT i = 0; i < TARRAYSIZE( apTextBoxes ); i++ )
		apTextBoxes[ i ]->RemoveSelf();
}

// $deBlob: FUNCTION 005b29d0
void AStreamingDataHudElement::CreateTextBoxes()
{
	const TWCHAR* apTexts[ NUM_LINES ] = {
		m_wszCurrentJob,
		m_wszFMODMemory,
		m_wszMusicRead,
		m_wszMusicPeakTime,
		m_wszTerrainTimeSinceRead,
		m_wszTerrainQueueTimer,
		m_wszTerrainQueuePeak,
		m_wszQueueSize,
		m_wszFMODReadSize,
		m_wszFMODReadTime,
		m_wszNumPositionals,
		m_wszNumFreePositionals,
		m_wszTotalPositionals,
		m_wszMusicBeat,
		m_wszMusicPosition,
		m_wszMusicOpenState,
		m_wszUnk17BC,
		m_wszDiscRead,
	};

	T2GUITextBox* apTextBoxes[ NUM_LINES ] = {
		&m_oCurrentJob,
		&m_oFMODMemory,
		&m_aMusicRead[ 0 ],
		&m_oMusicPeakTime,
		&m_aMusicRead[ 1 ],
		&m_oTerrainQueueTimer,
		&m_oTerrainQueuePeak,
		&m_oQueueSize,
		&m_oFMODReadSize,
		&m_oFMODReadTime,
		&m_oNumPositionals,
		&m_oNumFreePositionals,
		&m_oTotalPositionals,
		&m_aMusic[ 0 ],
		&m_aMusic[ 1 ],
		&m_oMusicOpenState,
		&m_oUnk17BC,
		&m_oDiscRead,
	};

	for ( TINT i = 0; i < NUM_LINES; i++ )
	{
		T2GUITextBox* pTextBox = apTextBoxes[ i ];

		pTextBox->Create( AFontManager::sm_pMsgFont, 500.0f );
		pTextBox->SetJustify( T2GUIFont::JUSTIFY_CENTRE );
		pTextBox->SetColour( 0xFFFFFF00 );
		pTextBox->SetDimensions( 600.0f, 21.0f, TTRUE );
		pTextBox->SetVisible( TTRUE );
		pTextBox->SetText( apTexts[ i ] );
		pTextBox->SetMaxChars( -1 );

		if ( i == 0 )
		{
			T2GUIElement* pSafeArea = AGUIManager::GetSingleton()->GetSafeArea();
			pSafeArea->SetVisible( TTRUE );
			pTextBox->SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_TOPCENTER );
			pSafeArea->AddChildHead( pTextBox );
		}
		else
		{
			pTextBox->SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
			apTextBoxes[ i - 1 ]->AddChildTail( pTextBox );
		}
	}
}

// $deBlob: FUNCTION 005b2c00
void AStreamingDataHudElement::Reset()
{
	m_fTerrainTimeSinceRead  = 0.0f;
	m_fFMODTimeSinceRead     = 0.0f;
	m_fTerrainQueueTimer     = 0.0f;
	m_fTerrainQueuePeak      = 0.0f;
	m_uiUnk1AD8              = 0;
	m_fFMODTimeSinceReadPeak = 0.0f;
	m_iUnk1AA8               = 0;
	m_iUnk1AAC               = 0;
	m_iQueueSize             = 0;
	m_iQueuePeak             = 0;
	m_iFMODReadSize          = 0;
	m_iFMODReadPeak          = 0;
	m_bFMODReading           = TFALSE;
	m_fFMODReadTimePeak      = 0.0f;
	m_fFMODReadTime          = 0.0f;
	m_iNumFMODReads          = 0;
	m_iNumFMODReadsPeak      = 0;
	m_bUnk1A90               = TFALSE;
	m_fRecreateTime          = 0.0f;
}

// $deBlob: FUNCTION 005b2cc0
void AStreamingDataHudElement::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnabled )
	{
		m_oCurrentJob.SetVisible( TFALSE );
		Reset();
		return;
	}

	m_oCurrentJob.SetVisible( TTRUE );
	TStringManager::String16Format( m_wszCurrentJob, TEXT_LENGTH, L"Current job not set" );

	// FMOD's memory
	int         iCurrentAlloced = 0;
	int         iMaxAlloced     = 0;
	FMOD_RESULT eResult         = FMOD_Memory_GetStats( &iCurrentAlloced, &iMaxAlloced, true );
	TINT        iPercent        = TMath::CeilToInt( ( TFLOAT( iCurrentAlloced ) / TFLOAT( s_iFMODMemorySize ) ) * 100.0f );

	if ( eResult != FMOD_OK )
		TStringManager::String16Format( m_wszFMODMemory, TEXT_LENGTH, L"FMOD MEMORY ERROR" );
	else
		TStringManager::String16Format( m_wszFMODMemory, TEXT_LENGTH, L"Percent %d, Current: %d Peek: %d Max %d", iPercent, iCurrentAlloced, iMaxAlloced, s_iFMODMemorySize );

	// The terrain and FMOD's disc reads
	TStringManager::String16Format( m_wszMusicRead, TEXT_LENGTH, L"Music READ Not Def for windows" );
	TStringManager::String16Format( m_wszMusicPeakTime, TEXT_LENGTH, L"Music PEAK TIME Not Def for windows" );
	TStringManager::String16Format( m_wszTerrainTimeSinceRead, TEXT_LENGTH, L"Terrain Time Since Read %f", m_fTerrainTimeSinceRead );
	TStringManager::String16Format( m_wszTerrainQueueTimer, TEXT_LENGTH, L"Terrain Process Queue Timer %f", m_fTerrainQueueTimer );
	TStringManager::String16Format( m_wszTerrainQueuePeak, TEXT_LENGTH, L"Terrain Process Queue Peak %f", m_fTerrainQueuePeak );
	TStringManager::String16Format( m_wszQueueSize, TEXT_LENGTH, L"Queue size %d queue Peak %d", m_iQueueSize, m_iQueuePeak );
	TStringManager::String16Format( m_wszFMODReadSize, TEXT_LENGTH, L"FMOD read Size %d Peak %d", m_iFMODReadSize, m_iFMODReadPeak );
	TStringManager::String16Format( m_wszFMODReadTime, TEXT_LENGTH, L"FMOD read time %f Peak %f", m_fFMODReadTime, m_fFMODReadTimePeak );

	m_fRecreateTime += a_fDeltaTime;

	// The positional sounds
	TINT iNumSounds = APositionalSoundManager::GetSingleton()->GetNumSounds();
	TINT iNumFree   = APositionalSoundManager::GetSingleton()->GetNumFree();
	TStringManager::String16Format( m_wszNumPositionals, TEXT_LENGTH, L"Num Positional sounds %d", iNumSounds );
	TStringManager::String16Format( m_wszNumFreePositionals, TEXT_LENGTH, L"Num Free Positional sounds %d", iNumFree );
	TStringManager::String16Format( m_wszTotalPositionals, TEXT_LENGTH, L"Add %d, Total 256", iNumFree + iNumSounds );

	// The music stream
	AMusicManager* pMusicManager = AMusicManager::GetSingleton();
	TINT           iNumBeats     = pMusicManager->GetNumBeats();
	TINT           iNumBars      = pMusicManager->GetNumBars();
	TBOOL          bPitchKept    = pMusicManager->IsPitchChangeKept();

	TINT  iBeat, iBar;
	TUINT uiNextBeat, uiNextBar, uiPosition;
	TBOOL bWaitingForBeat, bWaitingForBar;
	TBOOL bStarving        = TFALSE;
	TUINT uiStreamedAmount = 0;
	TINT  iOpenState       = -1;
	TINT  iResult          = -1;
	pMusicManager->GetDebugInfo( &iBeat, &iBar, &uiNextBeat, &uiNextBar, &uiPosition, &bWaitingForBeat, &bWaitingForBar, &bStarving, &uiStreamedAmount, &iOpenState, &iResult );

	TStringManager::String16Format( m_wszMusicBeat, TEXT_LENGTH, L"Beat %d/%d  Bar %d/%d Next BeatPos %u Next Bar Pos %u", iBeat + 1, iNumBeats, iBar + 1, iNumBars, uiNextBeat, uiNextBar );
	TStringManager::String16Format( m_wszMusicPosition, TEXT_LENGTH, L"Pos %u, bModifiedPitch =%d bGotBeatPos=%d  bGotBarPos=%d", uiPosition, bPitchKept, bWaitingForBeat, bWaitingForBar );

	if ( ATerrain2::GetInstance() != TNULL )
		m_iQueueSize = ATerrain2::GetInstance()->GetNumLoadingCells();
	else
		m_iQueueSize = 0;

	if ( m_iNumFMODReads > m_iNumFMODReadsPeak )
		m_iNumFMODReadsPeak = m_iNumFMODReads;

	m_iNumFMODReads = 0;

	if ( !m_bFMODReading )
		TStringManager::String16Format( m_wszDiscRead, L"FMOD - NOT READING DISC" );
	else
		TStringManager::String16Format( m_wszDiscRead, L"FMOD - PERFORMING DISC READ" );

	const TWCHAR* wszOpenState;

	switch ( iOpenState )
	{
		case 0:
			wszOpenState = L"OpenState = READY!";
			break;
		case 1:
			wszOpenState = L"OpenState = LOADING!";
			break;
		case 2:
			wszOpenState = L"OpenState = ERROR!";
			break;
		case 3:
			wszOpenState = L"OpenState = CONNECTING!";
			break;
		case 4:
			wszOpenState = L"OpenState = BUFFERING!";
			break;
		default:
			wszOpenState = L"OpenState = UNKNOWN!";
			break;
	}

	// NOTE: retail formats the text with itself as an argument (the result of vswprintf is undefined then)
	TStringManager::String16Format( m_wszMusicOpenState, wszOpenState );

	if ( bStarving )
		TStringManager::String16Format( m_wszMusicOpenState, L"%ls !STARVING!", m_wszMusicOpenState );

	TStringManager::String16Format( m_wszMusicOpenState, TEXT_LENGTH, L"%ls streamed amount = %u Re-Create %f", m_wszMusicOpenState, uiStreamedAmount, m_fRecreateTime );

	// The statistics
	if ( m_bFMODReading )
	{
		m_fFMODReadTime += a_fDeltaTime;

		if ( m_fFMODReadTime > m_fFMODReadTimePeak )
			m_fFMODReadTimePeak = m_fFMODReadTime;
	}
	else
	{
		m_fFMODReadTime = 0.0f;
	}

	if ( m_iQueueSize > m_iQueuePeak )
		m_iQueuePeak = m_iQueueSize;

	if ( m_iQueueSize != 0 )
	{
		m_fTerrainTimeSinceRead = 0.0f;
		m_fTerrainQueueTimer += a_fDeltaTime;
	}
	else
	{
		m_fTerrainQueueTimer = 0.0f;
		m_fTerrainTimeSinceRead += a_fDeltaTime;
	}

	m_fFMODTimeSinceRead += a_fDeltaTime;

	if ( m_fTerrainQueueTimer > m_fTerrainQueuePeak )
		m_fTerrainQueuePeak = m_fTerrainQueueTimer;

	if ( m_fFMODTimeSinceRead > m_fFMODTimeSinceReadPeak )
		m_fFMODTimeSinceReadPeak = m_fFMODTimeSinceRead;
}

// $deBlob: FUNCTION 005b31f0
void __stdcall AStreamingDataHudElement::OnFMODReadStart( void* a_pUserData, TINT a_iSize )
{
	AStreamingDataHudElement* pThis = TSTATICCAST( AStreamingDataHudElement, a_pUserData );

	pThis->m_fFMODTimeSinceRead = 0.0f;
	pThis->m_bFMODReading       = TTRUE;
	pThis->m_iFMODReadSize      = a_iSize;

	if ( a_iSize > pThis->m_iFMODReadPeak )
		pThis->m_iFMODReadPeak = a_iSize;
}

// $deBlob: FUNCTION 005b3230
void __stdcall AStreamingDataHudElement::OnFMODReadEnd( void* a_pUserData )
{
	AStreamingDataHudElement* pThis = TSTATICCAST( AStreamingDataHudElement, a_pUserData );

	pThis->m_iNumFMODReads++;
	pThis->m_bFMODReading = TFALSE;
}

// $deBlob: FUNCTION 005b3250
void AStreamingDataHudElement::OnFMODReadStart( TINT a_iSize )
{
	m_fFMODTimeSinceRead = 0.0f;
	m_bFMODReading       = TTRUE;
	m_iFMODReadSize      = a_iSize;

	if ( a_iSize > m_iFMODReadPeak )
		m_iFMODReadPeak = a_iSize;
}

// $deBlob: FUNCTION 005b3280
void AStreamingDataHudElement::OnFMODReadEnd()
{
	m_iNumFMODReads++;
	m_bFMODReading = TFALSE;
}

// $deBlob: FUNCTION 005b3290
void AStreamingDataHudElement::CreateLogFile()
{
	m_pLog->CreateLogFile();
}
