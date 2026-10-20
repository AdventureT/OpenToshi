#include "pch.h"
#include "ABlobMusicMixer.h"
#include "ABlobFrontEnd.h"
#include "ABlobPopupMusicMixer.h"
#include "Cameras/ACameraManager.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Movie/AMoviePlayer.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundLoadHelper.h"

#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIProgressBar.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobMusicMixer ) == 0x1C4 );

// The share of the maximum score A/Z add per second
static const TFLOAT s_fScoreSpeed = 0.5f;

// The time between two colour sounds
static const TFLOAT s_fColourSoundDelay = 0.3f;

// $deBlob: FUNCTION 00638220
// $deBlob: FUNCTION 00638230
// $deBlob: FUNCTION 006382a0
// $deBlob: FUNCTION 00638310
// $deBlob: FUNCTION 00638320
// $deBlob: FUNCTION 0053d710
// $deBlob: FUNCTION 00858940
TDEFINE_CLASS( ABlobMusicMixer );

// $deBlob: FUNCTION 00638330
ABlobMusicMixer::ABlobMusicMixer()
    : ABlobMenu( -1 )
{
	m_bStartMusic       = TTRUE;
	m_bMoviePlaying     = TFALSE;
	m_pLoadingText      = TNULL;
	m_uiUnk14C          = 0;
	m_pIntensity        = TNULL;
	m_pPopup            = TNULL;
	m_uiUnk158          = 0;
	m_uiUnk15C          = 8;
	m_fColourSoundTimer = 0.0f;
}

// $deBlob: FUNCTION 006383a0
// $deBlob: FUNCTION 00638cb7
// $deBlob: FUNCTION 00638cc0
ABlobMusicMixer::~ABlobMusicMixer()
{
	if ( m_pPopup != TNULL )
		m_pPopup->MarkForDelete();

	AMusicManager::GetSingleton()->PauseMusicNow();
	AMusicManager::GetSingleton()->UnloadMusicData();

	TASSERT( AMusicManager::IsSingletonCreated() );

	// The front end music again
	ASoundLoadHelper::GetSingleton()->UnloadSoundEventGroups();
	AMusicManager::GetSingleton()->LoadMusicData( "FrontEnd", TTRUE, TFALSE );
	AMusicManager::GetSingleton()->CreateMusicStream();
	AMusicManager::GetSingleton()->SetNoPercentage( TTRUE );
	AMusicManager::GetSingleton()->StartMusic( TTRUE );
	ASoundLoadHelper::GetSingleton()->LoadFrontEndSounds();
	ASoundLoadHelper::GetSingleton()->SetState( ASoundLoadHelper::LOADGROUP );

	// NOTE: retail waits here without a deadline until the sounds are loaded
	while ( ASoundLoadHelper::GetSingleton()->GetState() != ASoundLoadHelper::LOADED )
		ASoundLoadHelper::GetSingleton()->Update();
}

// $deBlob: FUNCTION 006387b0
void ABlobMusicMixer::Tick( TFLOAT a_fDeltaTime )
{
	TXUIScene::Tick( a_fDeltaTime );


	if ( TXUI::GetSingleton()->GetTransToScene() != TNULL || TXUI::GetSingleton()->GetTransFromScene() != TNULL || TXUI::GetSingleton()->IsInputLocked() )
		return;

	if ( !m_bMoviePlaying && !AMoviePlayer::GetSingleton()->IsMoviePlaying() )
	{
		ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
		pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_LoopMusicMixer, ABlobFrontEnd::BGMOVIE_None, TTRUE, TFALSE, TFALSE );
		m_bMoviePlaying = TTRUE;
	}

	if ( ASoundLoadHelper::GetSingleton()->GetState() != ASoundLoadHelper::LOADED )
	{
		ASoundLoadHelper::GetSingleton()->Update();
		return;
	}


	if ( m_bStartMusic )
	{
		AMusicManager::GetSingleton()->CreateMusicStream();
		AMusicManager::GetSingleton()->StartMusic( TFALSE );
		AMusicManager::GetSingleton()->AddScore( 100 );
		m_bStartMusic = TFALSE;
	}

	m_fColourSoundTimer -= a_fDeltaTime;

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );
	TFLOAT                fDirection;

	if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_A ) )
		fDirection = 1.0f;
	else if ( pKeyboard->IsDown( TInputDeviceKeyboard::KEY_Z ) )
		fDirection = -1.0f;
	else
		fDirection = 0.0f;

	if ( fDirection != 0.0f )
	{
		AMusicManager::GetSingleton()->AddScore( TMath::FloorToInt( s_fScoreSpeed * fDirection * a_fDeltaTime * TFLOAT( AMusicManager::GetSingleton()->GetMaxScore() ) ) );
		TSTATICCAST( TXUIProgressBar, m_pIntensity )->SetValue( TMath::FloorToInt( ( TFLOAT( AMusicManager::GetSingleton()->GetScore() ) / TFLOAT( AMusicManager::GetSingleton()->GetMaxScore() ) ) * 100.0f ) );
	}

	m_pLoadingText->SetVisible( TFALSE );

	// NOTE: retail ticks the scene a second time
	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 006384e0
void ABlobMusicMixer::OnCreated()
{

	ASoundLoadHelper::GetSingleton()->UnloadSoundEventGroups();
	AMusicManager::GetSingleton()->PauseMusicNow();
	AMusicManager::GetSingleton()->UnloadMusicData();
	TASSERT( AMusicManager::GetSingleton()->UsingMusicOverride() );

	const TCHAR* sOverrideTrackName = AMusicManager::GetSingleton()->GetMusicOverride();
	TASSERT( sOverrideTrackName != TNULL );

	m_wszTrackName = AMusicManager::GetTrackDisplayName( sOverrideTrackName );
	AMusicManager::GetSingleton()->LoadMusicOverride( TTRUE );

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
		m_apButtons[ i ] = TNULL;

	ASoundLoadHelper::GetSingleton()->SetState( ASoundLoadHelper::LOADGROUP );

	ABlobMenu::OnCreated();

	m_pIntensity   = FindChildXUIElement( L"MusicIntensity" );
	m_pLoadingText = FindChildXUIElement( L"LoadingText" );

	static const TWCHAR* s_awszButtons[ NUM_COLOURS ] = { L"RedSelect", L"OrangeSelect", L"YellowSelect", L"GreenSelect", L"BlueSelect", L"PurpleSelect", L"BrownSelect" };
	static const TINT    s_aiColours[ NUM_COLOURS ]   = { 4, 5, 1, 3, 2, 6, 7 };

	for ( TINT i = 0; i < NUM_COLOURS; i++ )
	{
		TINT iColour     = s_aiColours[ i ];
		m_aiColours[ i ] = iColour;
		m_apButtons[ i ] = FindChildXUIElement( s_awszButtons[ i ] );

		TXUIFigure* pFigure = TSTATICCAST( TXUIFigure, TSTATICCAST( TXUIControl, m_apButtons[ i ] )->GetVisual()->FindChildXUIElement( L"MixerButtonElement" ) );
		pFigure->SetFillColour( ABlobColourComponent::ms_auiColours[ iColour ] );
	}

	SetTrackNameText( FindChildXUIElement( L"TrackName" ), m_wszTrackName, -1 );

	TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( TXUI::GetSingleton()->FindResource( "popupmusicmixer.xur" ), L"MixerPopup", TTRUE );
	m_pPopup          = pScene->CastTo<ABlobPopupMusicMixer>();
	m_pPopup->RemoveSelf();
}

// $deBlob: FUNCTION 00638a20
TBOOL ABlobMusicMixer::OnNotifyPress( TXUIElement* a_pSource, TINT a_iController )
{
	for ( TINT i = 0; i < NUM_COLOURS; i++ )
	{
		if ( a_pSource == m_apButtons[ i ] )
			PlayColourSound( m_aiColours[ i ] );
	}

	return ABlobMenu::OnNotifyPress( a_pSource, a_iController );
}

// $deBlob: FUNCTION 00638980
TBOOL ABlobMusicMixer::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( a_eButton == XUIButton_Back )
	{
		TXUIScene* pParentScene = FindParentScene();

		if ( pParentScene != TNULL )
		{
			m_pPopup->RemoveSelf();
			pParentScene->AddChildTail( m_pPopup );
		}

		TXUI::GetSingleton()->TransitionTo( this, m_pPopup );
		AMusicManager::GetSingleton()->SetPitch( 0.1f, 0.5f, TTRUE );
	}

	return TXUIScene::HandleButton( a_eButton, a_iController );
}

// $deBlob: FUNCTION 00638b20
void ABlobMusicMixer::TransTo()
{
	AMoviePlayer::GetSingleton()->StopMovie();

	ABlobFrontEnd* pFrontEnd = TSTATICCAST( ABlobFrontEnd, FindParentScene() );
	TASSERT( -1 != m_iBackgroundMovie );
	pFrontEnd->PlayBackgroundMovie( ABlobFrontEnd::BGMOVIE_TransMusicMixer, m_iBackgroundMovie, TFALSE, TFALSE, TFALSE );

	TXUIScene::TransTo();
}

// $deBlob: FUNCTION 00638b90
void ABlobMusicMixer::PlayColourSound( TINT a_iColour )
{
	if ( m_fColourSoundTimer < 0.0f )
	{
		m_fColourSoundTimer = s_fColourSoundDelay;

		const TMatrix44& rMatrix  = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
		TVector4         vForward = rMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
		TVector4         vPos     = rMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );

		vForward.Multiply( 20.0f );
		vPos.x += vForward.x;
		vPos.y += vForward.y;
		vPos.z += vForward.z;

		AMusicManager::GetSingleton()->AddColourSyncSound( a_iColour, &vPos );
	}
}

// $deBlob: FUNCTION 00638d30
void ABlobMusicMixer::SetTrackNameText( TXUIElement* a_pElement, const TWCHAR* a_wszText, TINT a_iUnk84 )
{
	// NOTE: retail writes the text pointer (TXUIControl's m_wszText) and 0x84 directly
	TSTATICCAST( TXUIControl, a_pElement )->SetTextPointer( a_wszText );
	*TREINTERPRETCAST( TINT*, TREINTERPRETCAST( TUINT8*, a_pElement ) + 0x84 ) = a_iUnk84;
}
