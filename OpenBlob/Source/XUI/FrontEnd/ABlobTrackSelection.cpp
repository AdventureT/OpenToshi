#include "pch.h"
#include "ABlobTrackSelection.h"
#include "ABlobFrontEnd.h"
#include "Sound/AStreamingSoundPlayer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobTrackSelection ) == 0x144 );

// $deBlob: FUNCTION 0063bf90
// $deBlob: FUNCTION 0063bfa0
// $deBlob: FUNCTION 0063c030
// $deBlob: FUNCTION 0063c0b0
// $deBlob: FUNCTION 0063c0c0
// $deBlob: FUNCTION 0053d850
// $deBlob: FUNCTION 00858990
TDEFINE_CLASS( ABlobTrackSelection );

// $deBlob: FUNCTION 0063c0d0
ABlobTrackSelection::ABlobTrackSelection()
    : ABlobMenu( -1 )
{
	m_bDestroyMixer = TFALSE;
}

// $deBlob: FUNCTION 0063c100
// $deBlob: FUNCTION 0063c217
// $deBlob: FUNCTION 0063c220
ABlobTrackSelection::~ABlobTrackSelection()
{
	TXUIScene* pMixer = FindFrontEndScene( L"MusicMixerScene" );

	if ( pMixer != TNULL )
		delete pMixer;
}

// $deBlob: FUNCTION 0063c180
void ABlobTrackSelection::TransTo()
{
	if ( m_bDestroyMixer )
	{
		m_bDestroyMixer = TFALSE;

		TXUIScene* pMixer = FindFrontEndScene( L"MusicMixerScene" );

		if ( pMixer != TNULL )
			delete pMixer;

		m_pPrevScene = FindFrontEndScene( L"ExtrasScene" );
	}

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_LoopMusicMixer, ABlobFrontEnd::BGMOVIE_None, TTRUE, TFALSE, TFALSE );
	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 0063c1e0
void ABlobTrackSelection::TransFrom()
{
	AStreamingSoundPlayer::GetSingleton()->StopAll();
	TXUIScene::TransFrom();
}

// $deBlob: FUNCTION 0063c200
void ABlobTrackSelection::DestroyMusicMixer()
{
	TXUIScene* pMixer = FindFrontEndScene( L"MusicMixerScene" );

	if ( pMixer != TNULL )
		delete pMixer;
}
