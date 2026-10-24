#include "pch.h"
#include "Entities/ATRBResourceFactory.h"
#include "AFXViewState.h"
#include "AAssetStreaming.h"
#include "ACurrentLevelInformation.h"
#include "ARegionManager.h"
#include "Cameras/ACameraManager.h"
#include "FXComposition/AFXCompositionManager.h"
#include "Fx/AEnvironmentFX.h"
#include "Input/AInputManager2.h"
#include "GUI/AFontManager.h"
#include "GUI/AGUIManager.h"
#include "Render/Lighting/ALightingData.h"

#include <Toshi/TString8.h>
#include <File/TTRB.h>
#include <Sound/TSound.h>
#include <TParticleSystem/TParticleSystem.h>

#include <fmod_event.hpp>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AFXViewState ) == 0x220 );

// The buttons of the previewer
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Exit       = 0x00;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Trigger    = 0x01;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Down       = 0x02;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Up         = 0x03;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Unk4       = 0x04;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Unk5       = 0x05;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Unk12      = 0x12;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_Stop       = 0x16;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_ToggleMove = 0x17;
static TCONSTEXPR_VAR AInputMap::INPUTBUTTON BUTTON_ToggleHelp = 0x1A;

// $deBlob: FUNCTION 005c2f10
AFXViewState::AFXViewState()
{
	m_eState     = STATE_LoadLevel;
	m_bRotating  = TTRUE;
	m_bShowHelp  = TFALSE;
	m_fTime      = 0.0f;
	m_levelprops = TNULL;
	m_pTRBJob    = TNULL;
	m_TRB        = TNULL;

	// The status text, bottom left
	m_oStatusText.Create( AFontManager::sm_pDebugFont, AGUIManager::GetSingleton()->GetSafeArea()->GetWidth() );
	m_oStatusText.SetScale( 1.0f );
	m_oStatusText.SetJustify( T2GUIFont::JUSTIFY_LEFT );
	m_oStatusText.SetText( L"Movement: Rotating" );
	m_oStatusText.SetMaxChars( -1 );
	m_oStatusText.SetColour( 0xFFFFFFFF );
	m_oStatusText.SetAttachment( T2GUIATTACHMENT_BOTTOMLEFT, T2GUIATTACHMENT_BOTTOMLEFT );
	m_oStatusText.GetTransform().SetTranslation( 10.0f, -10.0f );
	AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( &m_oStatusText );

	// The help text, top left
	m_oHelpText.Create( AFontManager::sm_pDebugFont, AGUIManager::GetSingleton()->GetSafeArea()->GetWidth() );
	m_oHelpText.SetScale( 1.0f );
	m_oHelpText.SetJustify( T2GUIFont::JUSTIFY_LEFT );
	m_oHelpText.SetText( L"Press F1 for help" );
	m_oHelpText.SetMaxChars( -1 );
	m_oHelpText.SetColour( 0xFFFFFFFF );
	m_oHelpText.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oHelpText.GetTransform().SetTranslation( 0.0f, 10.0f );
	AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( &m_oHelpText );

	LoadLevel();
}

// $deBlob: FUNCTION 005c30f0
// $deBlob: FUNCTION 005c3c00
AFXViewState::~AFXViewState()
{
	// NOTE: retail only takes the status text out of the GUI
	m_oStatusText.RemoveSelf();
}

// $deBlob: FUNCTION 005c3aa0
void AFXViewState::LoadLevel()
{
	ALevelInformation* pLevelInformation = ALevelInformation::GetSingleton();

	TINT iLevelIndex = pLevelInformation->GetLevelIndex( "FxPreviewLevel" );
	TASSERT( iLevelIndex >= 0 );

	m_levelprops = pLevelInformation->GetLevelProperties( iLevelIndex );
	TASSERT( m_levelprops != TNULL );

	ACurrentLevelInformation* pCurrentLevelInformation = new ACurrentLevelInformation( m_levelprops );
	pCurrentLevelInformation->Create( TFALSE );

	m_eState = STATE_LoadLevel;
}

// $deBlob: FUNCTION 005c31a0
AGameState::UpdateResult AFXViewState::OnUpdate( TFLOAT a_fDeltaTime )
{
	switch ( m_eState )
	{
		case STATE_LoadLevel:
			if ( !ACurrentLevelInformation::GetSingleton()->IsStreaming() )
			{
				FMOD_RESULT eResult = TSound::GetSingleton()->GetEventSystem()->load( "Blob07.fev", TNULL, TNULL );
				TASSERT( eResult == FMOD_OK );

				TTRB* pRegionTRB         = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_RegionRuntimeData );
				void* pRegionRuntimeData = pRegionTRB->GetSymbolAddress( "RegionRuntimeData" );
				TTRB* pTerrainTRB        = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_Terrain );

				// NOTE: retail allocates 0x18 bytes for the 4 bytes ALightingData
				new ALightingData();
				ALightingData::GetSingleton()->Load( pTerrainTRB );

				new ARegionManager( TSTATICCAST( ARegionManager::RegionRuntimeData, pRegionRuntimeData ) );
				ARegionManager::GetSingleton()->LoadRegion( 0 );

				m_eState = STATE_LoadRegion;
			}

			// NOTE: falls through to the running state, as in retail
		default:
		{
			m_fTime += a_fDeltaTime;

			if ( m_bRotating )
			{
				TMatrix44 mTransform = TMatrix44::IDENTITY;

				TFLOAT fSin, fCos;
				TMath::SinCos( m_fTime * TMath::PI, fSin, fCos );

				TVector4& rTranslation = mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
				rTranslation.x         = fSin * 3.0f;
				rTranslation.y         = 0.0f;
				rTranslation.z         = fCos * 3.0f;
				m_oComposition.SetTransform( mTransform );
			}

			m_oComposition.Update( a_fDeltaTime );
			m_oInputHelper.Update( a_fDeltaTime );

			if ( m_oInputHelper.IsJustDown( BUTTON_Trigger ) == TTRUE )
			{
				m_oComposition.TriggerEffect( TMatrix44::IDENTITY );
			}
			else if ( m_oInputHelper.IsJustDown( BUTTON_ToggleMove ) == TTRUE )
			{
				m_fTime     = 0.0f;
				m_bRotating = !m_bRotating;
				m_oStatusText.SetText( m_bRotating ? L"Movement: Rotating" : L"Movement: Stationary" );
				m_oStatusText.SetMaxChars( -1 );
			}
			else if ( m_oInputHelper.IsJustDown( BUTTON_Stop ) == TTRUE )
			{
				m_oComposition.Stop();
			}
			else if ( m_oInputHelper.IsJustDown( BUTTON_ToggleHelp ) == TTRUE )
			{
				m_bShowHelp = !m_bShowHelp;
				m_oHelpText.SetText(
				    m_bShowHelp ? L"F1 = Show/Hide help\nSpace = Trigger FX Composition\nA = Rotate Left\nD = Rotate Right\nW = Zoom In\nS = Zoom Out\nUp Arrow = Tilt Up\nDown Arrow = Tilt Down\nLeft Arrow = Change Reference Model Previous\nRight Arrow = Change Reference Model Next\nY = Kill looping effect\nR = Rotating/Stationary movement\nESC = Exit" :
				                  L"Press F1 for help"
				);
				m_oHelpText.SetMaxChars( -1 );
			}

			// The camera moves up and down and looks at the origin
			TVector4        vCamPos       = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
			AInputManager2* pInputManager = AInputManager2::GetSingleton();

			if ( pInputManager->GetButtonDown( BUTTON_Up, pInputManager->GetControllerHandle( AInputManager2::INPUTDEVICE( 0 ) ) ) )
				vCamPos.y = a_fDeltaTime * 10.0f + vCamPos.y;
			else if ( pInputManager->GetButtonDown( BUTTON_Down, pInputManager->GetControllerHandle( AInputManager2::INPUTDEVICE( 0 ) ) ) )
				vCamPos.y = vCamPos.y - a_fDeltaTime * 10.0f;

			TVector4 vTarget( 0.0f, 0.0f, 0.0f, 1.0f );
			ACameraManager::GetSingleton()->GetCurrentCamera()->LookAtPoint( vCamPos, vTarget );

			if ( m_oInputHelper.IsJustDown( BUTTON_Exit ) == TTRUE )
				return UpdateResult_Remove;

			break;
		}
		case STATE_LoadRegion:
			if ( !ARegionManager::GetSingleton()->IsLoading() )
			{
				ARegionManager::GetSingleton()->LoadRegionSounds( 0 );
				m_eState = STATE_LoadRegionSounds;
			}
			else
			{
				ARegionManager::GetSingleton()->UpdateLoading();
			}

			break;
		case STATE_LoadRegionSounds:
			if ( ARegionManager::GetSingleton()->IsLoadingSounds() )
			{
				ARegionManager::GetSingleton()->UpdateSoundLoading();
				break;
			}

			{
				TASSERT( m_pTRBJob == 0 );
				TASSERT( m_TRB == 0 );

				TString8 strFileName( "data/Blob_FX/FXParticles.trb" );
				m_TRB     = new TTRB();
				m_pTRBJob = AAssetStreaming::GetSingleton()->GetFreeJob();
				m_pTRBJob->Init( m_TRB, strFileName.GetString() );
				AAssetStreaming::GetSingleton()->AddMainThreadJob2( m_pTRBJob );
				m_eState = STATE_LoadParticles;
			}

			break;
		case STATE_LoadParticles:
			if ( m_pTRBJob->m_bFinished )
			{
				TString8 strName( "data/Blob_FX/FXParticles" );
				TParticleSystem::GetSingleton()->LoadTRB( m_TRB, strName.GetString(), TTRUE );
				m_TRB = TNULL;
				AAssetStreaming::GetSingleton()->ReleaseJob( m_pTRBJob );
				m_pTRBJob = TNULL;

				AEnvironmentFX* pEnvironmentFX = new AEnvironmentFX();
				pEnvironmentFX->SetSkyEnabled( TFALSE );
				AEnvironmentFX::GetSingleton()->CreateSky();
				AEnvironmentFX::GetSingleton()->LoadSettings( "data/envfx/fxpreview.trb", TFALSE );
				m_eState = STATE_LoadEnvFX;
			}

			break;
		case STATE_LoadEnvFX:
			if ( AEnvironmentFX::GetSingleton()->UpdateLoading() )
			{
				TString8 strFileName( "data/" );
				strFileName += m_levelprops->m_directory;
				strFileName += "/fxdata.trb";
				AFXCompositionManager::GetSingleton()->Load( strFileName.GetString() );
				m_eState = STATE_LoadFXData;
			}

			break;
		case STATE_LoadFXData:
			if ( AFXCompositionManager::GetSingleton()->IsDoneLoading() == TTRUE )
			{
				m_oComposition.Initialize( "fxpreview" );
				m_eState = STATE_Running;
			}

			break;
	}

	return AGameState::OnUpdate( a_fDeltaTime );
}

// $deBlob: FUNCTION 005c3810
void AFXViewState::OnInsertion()
{
	new ATRBResourceFactory();

	AGameState::OnInsertion();

	ACameraManager::GetSingleton()->SetCurrentCamera( ACamera::CT_FREE, 0, TTRUE );

	TVector4 vTarget( 0.0f, 0.0f, 0.0f, 1.0f );
	TVector4 vEye( 0.0f, -5.0f, 10.0f, 1.0f );
	ACameraManager::GetSingleton()->GetCurrentCamera()->LookAtPoint( vEye, vTarget );

	AInputManager2::GetSingleton()->SetContext( AInputMap::INPUTCONTEXT_Menu );

	m_oInputHelper.AddMapping( BUTTON_Trigger, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( BUTTON_Unk4, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( BUTTON_Unk5, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( BUTTON_Unk12, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( BUTTON_Exit, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( BUTTON_ToggleMove, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( BUTTON_Stop, TFALSE, -1.0f );
	m_oInputHelper.AddMapping( BUTTON_ToggleHelp, TFALSE, -1.0f );
	m_oInputHelper.Update( 0.0f );
}

// $deBlob: FUNCTION 005c3980
void AFXViewState::OnRemoval()
{
	AInputManager2::GetSingleton()->RemoveCurrentContext();
	m_oComposition.Destroy();

	if ( ALightingData::GetSingleton() != TNULL )
		delete ALightingData::GetSingleton();

	if ( ATRBResourceFactory::GetSingleton() != TNULL )
		delete ATRBResourceFactory::GetSingleton();

	if ( AEnvironmentFX::GetSingleton() != TNULL )
		delete AEnvironmentFX::GetSingleton();

	TParticleSystem::GetSingleton()->StopAllParticles();
	TParticleSystem::GetSingleton()->Destroy();
	ARegionManager::GetSingleton()->UnloadAllRegions();

	if ( ARegionManager::GetSingleton() != TNULL )
		delete ARegionManager::GetSingleton();

	ACurrentLevelInformation::GetSingleton()->Destroy();

	if ( ACurrentLevelInformation::GetSingleton() != TNULL )
		delete ACurrentLevelInformation::GetSingleton();

	AFXCompositionManager::GetSingleton()->UnLoad();
	m_levelprops = TNULL;

	AGameState::OnRemoval();
}

// $deBlob: FUNCTION 005c3a70
void AFXViewState::OnActivate()
{
	ACameraManager::GetSingleton()->Activate();
	AGameState::OnActivate();
}

// $deBlob: FUNCTION 005c3a90
void AFXViewState::OnDeactivate()
{
	AGameState::OnDeactivate();
}
