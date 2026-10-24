#include "pch.h"
#include "Render/Decal/ADecalManager.h"
#include "Platform/DX11/AStencilShadow_DX11.h"
#include "ARenderer.h"
#include "GameController/AGameStatus.h"
#include "Animation/AModelFactory.h"
#include "Entities/AEntityFactory.h"
#include "Render/Material/AMaterialManager.h"
#include "AApplication.h"
#include "Console/AConsoleState.h"
#include "Cameras/ABlobFollowCamera.h"

#include <Render/TOrderTable.h>
#include <Render/TAssetInit.h>
#include <GUI/T2GUI.h>
#include <GUI/T2GUIRectangle.h>
#include <XUI/TXUI.h>
#include <XUI/TXUIRenderer.h>
#include <XUI/TXUICanvas.h>
#include "Memory/AMemory.h"
#include "GUI/AGUIManager.h"
#include "GUI/AGUICursor.h"
#include "GameInterface/AGameStateController.h"
#include "Movie/AMoviePlayer.h"
#include "Render/AXYZViewportManager.h"
#include "Fx/AEnvironmentFX.h"
#include "Cameras/ACameraManager.h"
#include "Animation/AModelInstance2.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Render/AInstanceRenderer.h"
#include "Fx/Shadows/AShadow.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "GameInterface/AFrontEndMovieState.h"
#include "Shaders/ARibbonTrailShader/ARibbonTrailShader.h"
#include "Render/PaintParticleSystem/APaintParticleSystem.h"
#include "Shaders/ASpriteShader/ASpriteShaderHAL.h"
#include <Platform/DX11/TGlow_DX11.h>
#include <Platform/DX11/TFXAA_DX11.h>
#include <Render/TScene.h>
#include <TParticleSystem/TParticleSystem.h>
#include <Render/TCameraObject.h>
#include <Render/TRenderParamTable.h>
#ifdef TRENDERINTERFACE_DX11
#  include <Platform/DX11/TRender_DX11.h>
#  include <Platform/DX11/TPrimShader_DX11.h>
#  include <Platform/DX11/TWorldShader_DX11.h>
#  include <Platform/DX11/TSysShader_DX11.h>
#  include "Platform/DX11/ASkinShader_DX11.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TUINT32 ARenderer::s_uiDefaultWidth  = 1280;
TUINT32 ARenderer::s_uiDefaultHeight = 720;
TINT    ARenderer::s_iAntialiasLevel = 1;
// $deBlob: FUNCTION 0053ca60
// $deBlob: FUNCTION 00858600
Toshi::TTRB ARenderer::s_BootAssetsTRB;
// $deBlob: FUNCTION 0053ca80
// $deBlob: FUNCTION 00858610
Toshi::THPTimer ARenderer::s_FrameTimer;
TINT            ARenderer::s_iCurrentCamera;
TBOOL           ARenderer::s_bRenderGUIAfterWorld = TTRUE;
TBOOL           ARenderer::s_bGlow                = TTRUE;

// Never set (a debug switch of the original): hides the particle systems
static TBOOL s_bHideParticles = TFALSE;

// Written by the "metrics_reset" console command, never read
static TINT s_iMetrics = 0;

// The "metrics_reset" console command
// $deBlob: FUNCTION 005eea90
static void __stdcall ResetMetricsCommand( TString8* a_pArgs, TINT a_iNumArgs )
{
	s_iMetrics = 0;
}

// $deBlob: FUNCTION 005ed0e0
ARenderer::ARenderer()
    : m_FrameEmitter( this )
{

	m_bUnk04               = TFALSE;
	m_pViewport            = TNULL;
	m_pViewportManager     = TNULL;
	m_uiUnk14              = 0;
	m_fUnk18               = 360.0f;
	m_uiCaptureState       = 0;
	m_bUnk20               = TFALSE;
	m_pCapture             = TNULL;
	m_pCaptureData         = TNULL;
	m_bUnk34               = TFALSE;
	m_bRenderGUI           = TTRUE;
	m_bUnk45               = TFALSE;
	m_bIsLoadingScreen     = TFALSE;
	m_uiBackgroundColour   = 0xff000000;
	m_afAmbientColour[ 0 ] = 1.0f;
	m_afAmbientColour[ 1 ] = 1.0f;
	m_afAmbientColour[ 2 ] = 1.0f;
	m_afAmbientColour[ 3 ] = 1.0f;
	m_bCaptureWait         = TFALSE;
	m_bSkipFrame           = TFALSE;
	m_pRectangle           = TNULL;
	m_fFrameTime           = 0.0f;
	m_bFogEnabled          = TFALSE;
	m_afFogColour[ 0 ]     = 1.0f;
	m_afFogColour[ 1 ]     = 1.0f;
	m_afFogColour[ 2 ]     = 1.0f;
	m_afFogColour[ 3 ]     = 1.0f;
	m_fFogStart            = 350.0f;
	m_fFogEnd              = 400.0f;
	m_pScene               = TNULL;

	AConsoleState::RegisterCommand( "metrics_reset", ResetMetricsCommand );

	s_FrameTimer.Reset();
}

// $deBlob: FUNCTION 005ed270
// $deBlob: FUNCTION 005f0140
ARenderer::~ARenderer()
{
	AConsoleState::UnregisterCommand( ResetMetricsCommand );

	TRender* pRender = TRender::GetSingleton();
	pRender->FlushDyingResources();
	pRender->FlushDyingResources();
	pRender->FlushDyingResources();

	if ( TPrimShader* pPrimShader = TPrimShader::GetSingleton() )
		delete pPrimShader;

	pRender->Destroy();

	if ( pRender ) delete pRender;
}

// $deBlob: FUNCTION 005ed390
void ARenderer::FlushRender()
{
	TRenderDX11::Interface()->WaitForEndOfRender();
	TRenderDX11::Interface()->FlushDyingResources();
}

// $deBlob: FUNCTION 005ed3b0
TBOOL ARenderer::CreateInterface()
{
	TOrderTable::CreateStaticData( 4500, 2500 );
	return CreateTRender() == TTRUE;
}

// $deBlob: FUNCTION 005ed3d0
TBOOL ARenderer::Create()
{

	m_pScene = new TScene();

	s_BootAssetsTRB.Load( "Data\\BootAssets\\BootAssets.trb" );

	TAssetInit::InitAssets( s_BootAssetsTRB, TTRUE, TFALSE );

	new TPrimShader();

	TWorldShaderHAL* pWorldShader = new TWorldShaderHAL();
	pWorldShader->Create();

	ASkinShaderHAL* pSkinShader = new ASkinShaderHAL();
	pSkinShader->Create();

	// The sprite shader (ASpriteShaderHAL's constructor and its Create is vtable +0x1C)
	ASpriteShaderHAL* pSpriteShader = new ASpriteShaderHAL();
	pSpriteShader->Create();

	// The paint particle system (the gravity is 80.0f, Create is vtable +0x1C)
	APaintParticleSystem* pPaintParticleSystem = new APaintParticleSystem( 80.0f );
	pPaintParticleSystem->Create();

	ARibbonTrailShader* pRibbonTrailShader = new ARibbonTrailShader();
	pRibbonTrailShader->Create();

	TSysShaderHAL::OrderTablePriorities oSysPriorities;
	oSysPriorities.iPriority1 = 100;
	oSysPriorities.iPriority2 = 5220;
	oSysPriorities.iPriority3 = 5222;
	oSysPriorities.iPriority4 = 5225;
	oSysPriorities.iPriority5 = 5215;
	oSysPriorities.iPriority6 = 500;
	oSysPriorities.iPriority7 = 5219;

	TSysShaderHAL* pSysShader = new TSysShaderHAL( &oSysPriorities );
	pSysShader->Create();

	CreateMainViewport();

	m_pViewportManager = new AXYZViewportManager();

	new AInstanceRenderer();

	// The rectangle of the screen (black, transparent)
	TFLOAT fRootWidth, fRootHeight;
	T2GUI::GetSingleton()->GetRootElement()->GetDimensions( fRootWidth, fRootHeight );

	m_pRectangle = new T2GUIRectangle();
	m_pRectangle->Create( fRootWidth, fRootHeight );
	m_pRectangle->SetColour( 0xff000000 );
	m_pRectangle->SetAlpha( 0.0f );
	m_pRectangle->SetVisible( TTRUE );

	return TTRUE;
}

// $deBlob: FUNCTION 005ed6e0
void ARenderer::Update( TFLOAT a_fDeltaTime )
{
	TRender* pRender      = TRender::GetSingleton();
	TBOOL    bRenderWorld = g_oTheApp.ShouldRenderWorld();

	if ( m_bSkipFrame )
	{
		m_bSkipFrame = TFALSE;
		return;
	}

	TBOOL bRenderMovie = AMoviePlayer::GetSingleton() != TNULL && AMoviePlayer::GetSingleton()->IsMoviePlaying();
	TBOOL bRenderGUI   = TTRUE;

	TBOOL bFrameData;
	m_FrameEmitter.Throw( &bFrameData );

	pRender->Update( a_fDeltaTime );

	TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() )->UpdateWater( a_fDeltaTime );

	pRender->BeginScene();

	if ( bRenderMovie )
	{
		if ( AMoviePlayer::GetSingleton() != TNULL )
			AMoviePlayer::GetSingleton()->OnRender( a_fDeltaTime );
	}
	else if ( bRenderWorld )
	{
		GetAppCamera();
		InitialiseViewPort();
		m_pScene->Reset();
		s_iCurrentCamera = 0;

		for ( AXYZViewportManager::VIEWPORT i = AXYZViewportManager::VIEWPORT_Count - 1; i >= AXYZViewportManager::VIEWPORT_FullScreen; i-- )
		{
			if ( !m_pViewportManager->IsViewportUsed( i ) )
				continue;

			m_pViewportManager->SetViewportBeingRendered( i );
			TViewport*     pViewport     = m_pViewportManager->GetViewport( i );
			TCameraObject* pCameraObject = m_pViewportManager->GetCameraObject( i );
			ACamera*       pCamera       = m_pViewportManager->GetCamera( i );

			if ( pCamera == TNULL )
				pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

			RenderMainScene( a_fDeltaTime, pViewport, pCameraObject, pCamera, MainScene, i == AXYZViewportManager::VIEWPORT_FullScreen );

			TASSERT( s_iCurrentCamera < MAX_VIEWPORTS_PER_FRAME );
			s_iCurrentCamera++;
		}

		m_pViewportManager->SetViewportBeingRendered( AXYZViewportManager::VIEWPORT_NONE );

		if ( AInstanceRenderer::GetInstance() != TNULL )
			AInstanceRenderer::GetInstance()->ClearSubmitted();

		bRenderGUI = s_bRenderGUIAfterWorld;
	}
	else if ( m_bIsLoadingScreen == TTRUE )
	{
		GetAppCamera();
		InitialiseViewPort();
		RenderLoadingScreen( a_fDeltaTime );
	}

	if ( bRenderGUI )
		RenderGUI( TFALSE );

	{
		const TClass* pGameStateClass = AGameStateController::GetSingleton()->GetCurrentGameState()->GetClass();
		TBOOL         bIsMovieState   = pGameStateClass->IsA( &TGetClass( AFrontEndMovieState ) );
		TRenderDX11*  pRenderDX11     = TRenderDX11::Interface();

		if ( s_iAntialiasLevel != 0 && !bIsMovieState )
		{
			if ( pRenderDX11->GetCurrentDisplayParams()->uiMultisampleQualityLevel < 2 )
				pRenderDX11->GetFXAA()->Render( pRenderDX11->GetMainSRView() );
			else
				pRenderDX11->GetFXAA()->Render( pRenderDX11->GetResolveSRView() );
		}
	}

	s_FrameTimer.Update();
	m_fFrameTime = s_FrameTimer.GetDelta();

	pRender->EndScene();

	AJellyShader_DX11::GetSingleton()->Update();

	s_FrameTimer.Reset();

	UpdateCapture( pRender );
}

// Destroys what Create made: the boot assets, the scene, the rectangle, the instance renderer, the viewports and the
// shaders
// $deBlob: FUNCTION 005eda80
void ARenderer::Destroy()
{
	TAssetInit::DeinitAssets( s_BootAssetsTRB );
	s_BootAssetsTRB.Close();

	if ( m_pScene != TNULL )
		delete m_pScene;

	m_pScene = TNULL;

	if ( m_pRectangle != TNULL )
		delete m_pRectangle;

	if ( AInstanceRenderer::GetInstance() != TNULL )
		delete AInstanceRenderer::GetInstance();

	if ( m_pViewportManager != TNULL )
		delete m_pViewportManager;

	m_pViewportManager = TNULL;

	if ( m_pViewport != TNULL )
		delete m_pViewport;

	ASkinShader* pSkinShader = ASkinShader::GetSingleton();
	pSkinShader->Invalidate();
	pSkinShader->OnDestroy();
	delete pSkinShader;

	ASpriteShader* pSpriteShader = ASpriteShader::GetSingleton();
	pSpriteShader->Invalidate();
	pSpriteShader->OnDestroy();
	delete pSpriteShader;

	APaintParticleSystem* pPaintParticleSystem = APaintParticleSystem::s_Instance;
	pPaintParticleSystem->Invalidate();
	pPaintParticleSystem->OnDestroy();
	delete pPaintParticleSystem;

	ARibbonTrailShader* pRibbonTrailShader = ARibbonTrailShader::GetSingleton();
	pRibbonTrailShader->Invalidate();
	pRibbonTrailShader->OnDestroy();
	delete pRibbonTrailShader;

	TWorldShader* pWorldShader = TWorldShader::GetSingleton();
	pWorldShader->Invalidate();
	pWorldShader->OnDestroy();
	delete pWorldShader;

	TSysShader* pSysShader = TSysShader::GetSingleton();
	pSysShader->Invalidate();
	pSysShader->OnDestroy();
	delete pSysShader;
}

// An update at 30 fps (the argument is unused)
// $deBlob: FUNCTION 005edbe0
void ARenderer::UpdateFixed( TFLOAT a_fUnused )
{
	Update( 1.0f / 30.0f );
}

// $deBlob: FUNCTION 005edbf0
Toshi::TViewport* ARenderer::GetMainViewport() const
{
	return m_pViewportManager->GetCurrentViewport();
}

// $deBlob: FUNCTION 005edc00
void ARenderer::SetBackgroundColour( TUINT32 a_uiRed, TUINT8 a_uiGreen, TUINT8 a_uiBlue )
{
	if ( m_pViewport != TNULL )
		m_pViewport->SetBackgroundColor( TUINT8( a_uiRed ), a_uiGreen, a_uiBlue, 255 );

	// NOTE: without alpha (the constructor sets 0xff000000)
	m_uiBackgroundColour = a_uiBlue | ( ( a_uiGreen | ( TUINT8( a_uiRed ) << 8 ) ) << 8 );
}

// $deBlob: FUNCTION 005ef870
void ARenderer::CreateMainViewport()
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	m_pViewport = new TViewport();
	m_pViewport->SetMemoryAllocatorBlock( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) );
	m_pViewport->SetX( 0.0f );
	m_pViewport->SetY( 0.0f );
	m_pViewport->SetWidth( TFLOAT( TINT( pDisplayParams->uiWidth ) ) );
	m_pViewport->SetHeight( TFLOAT( TINT( pDisplayParams->uiHeight ) ) );
	m_pViewport->SetMinZ( 0.2f );
	m_pViewport->SetMaxZ( 1.0f );
	m_pViewport->AllowBackgroundClear( TTRUE );
	m_pViewport->AllowDepthClear( TTRUE );
	m_pViewport->Enable( TTRUE );
	m_pViewport->EnableDefaultBeginRender( TTRUE );
	m_pViewport->SetBackgroundColor( 64, 64, 64, 255 );

	TRenderDX11::Interface()->GetGlow()->SetDist( 1.5f );
	TRenderDX11::Interface()->GetGlow()->SetIntensity( 4.0f );
}

// Renders the fades of the game states, XUI, T2GUI and the GUI elements of the game
// $deBlob: FUNCTION 005efb60
void ARenderer::RenderGUI( TBOOL a_bAllowBackgroundClear )
{
	if ( m_bRenderGUI == TFALSE )
		return;

	m_pViewport->AllowBackgroundClear( a_bAllowBackgroundClear );
	m_pViewport->Begin();

	TBOOL bFadesFirst = AGameStateController::GetSingleton()->IsRenderingFadesFirst();

	if ( bFadesFirst )
	{
		T2GUI::GetSingleton()->GetRenderer()->BeginScene();
		AGameStateController::GetSingleton()->RenderFades();
		T2GUI::GetSingleton()->GetRenderer()->EndScene();
	}

	TXUI::GetSingleton()->Render();
	T2GUI::GetSingleton()->Render();

	T2GUIRenderer* pOldRenderer = T2GUI::GetSingleton()->GetRenderer();
	TXUIRenderer*  pXUIRenderer = TXUI::GetSingleton()->GetRenderer();
	T2GUI::GetSingleton()->SetRenderer( pXUIRenderer );
	pXUIRenderer->BeginScene();
	AGUIManager::GetSingleton()->GetCursor()->Render();
	pXUIRenderer->EndScene();
	T2GUI::GetSingleton()->SetRenderer( pOldRenderer );

	if ( !bFadesFirst )
	{
		T2GUI::GetSingleton()->GetRenderer()->BeginScene();
		AGameStateController::GetSingleton()->RenderFades();
		T2GUI::GetSingleton()->GetRenderer()->EndScene();
	}

	m_pViewport->End();
}

// NOTE: the screen capture functions of the platform layer are TRender::CreateScreenCapture/DestroyScreenCapture
// (TRender_DX11.cpp)

// $deBlob: FUNCTION 005ef2f0
void ARenderer::RenderLoadingScreen( TFLOAT a_fDeltaTime )
{
	TRenderDX11*    pRender          = TRenderDX11::Interface();
	TViewport*      pViewport        = m_pViewportManager->GetCurrentViewport();
	TCameraObject*  pCameraObject    = m_pViewportManager->GetCurrentCameraObject();
	TRenderContext* pOriginalContext = pRender->GetCurrentContext();

	pRender->SetCurrentRenderContext( pViewport->GetRenderContext() );
	pRender->GetTransforms().Reset();

	pViewport->AllowBackgroundClear( TTRUE );
	pViewport->Begin();

	pCameraObject->Render();
	pRender->GetTransforms().Push( pViewport->GetRenderContext()->GetWorldViewMatrix() );

	TCameraObject* pOldCameraObject = pViewport->GetRenderContext()->GetCameraObject();
	pViewport->GetRenderContext()->SetCameraObject( pCameraObject );

	// The world planes of the frustum (for the culling done by the camera's users)
	TRenderContext* pCurrentContext = TRender::GetSingleton()->GetCurrentContext();
	TPlane          aWorldPlanes[ 6 ];
	pCurrentContext->GetWorldPlanes( aWorldPlanes, 6 );
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCurrentContext->SetModelViewMatrix( pCurrentContext->GetWorldViewMatrix() );
	pCamera->SetViewPlanes( aWorldPlanes, 6 );

	pCurrentContext->SetAmbientColour( TVector4( m_afAmbientColour[ 0 ], m_afAmbientColour[ 1 ], m_afAmbientColour[ 2 ], m_afAmbientColour[ 3 ] ) );

	// A light from the upper left, white
	TMatrix44 mLightDirection                           = TMatrix44::IDENTITY;
	TMatrix44 mLightColour                              = TMatrix44::IDENTITY;
	mLightDirection.AsBasisVector4( BASISVECTOR_RIGHT ) = TVector4( 0.47f, -0.74f, 0.47f, 0.0f );
	TRender::GetSingleton()->SetLightDirectionMatrix( mLightDirection );
	mLightColour.AsBasisVector4( BASISVECTOR_RIGHT ) = TVector4( 1.0f, 1.0f, 0.73f, 0.0f );
	TRender::GetSingleton()->SetLightColourMatrix( mLightColour );

	// The UV animation clock of the materials
	AMaterialManager* pMaterialManager = AMaterialManager::GetSingleton();
	TFLOAT            fOldTime         = pMaterialManager->m_fTime;
	TINT              iFrame           = TSystemManager::GetSingleton()->GetFrameCount();
	pMaterialManager->m_fTime          = fOldTime + a_fDeltaTime;

	// NOTE: a time that stopped advancing (float precision) restarts from 0
	if ( pMaterialManager->m_fTime == fOldTime && a_fDeltaTime > 0.0f )
		pMaterialManager->m_fTime = 0.0f;

	pMaterialManager->m_iFrame = iFrame;

	// NOTE: slot +0x0C of AGameState (AInGameState renders there)
	AGameStateController::GetSingleton()->GetCurrentGameState()->HandleMenuRequestToCancel();

	if ( AModelFactory::GetSingleton() != TNULL )
		AModelFactory::GetSingleton()->RenderLoadingScreenInstances();

	pViewport->GetRenderContext()->SetCameraObject( pOldCameraObject );
	pRender->GetTransforms().Pop();

	pViewport->End();
	pRender->FlushShaders();
	pRender->SetCurrentRenderContext( pOriginalContext );
}

// $deBlob: FUNCTION 005ee9e0
TBOOL ARenderer::StartCapture( TINT a_iWidth, TINT a_iHeight, TBOOL a_bUnk20, TBOOL a_bWait )
{
	m_iCaptureWidth  = a_iWidth;
	m_iCaptureHeight = a_iHeight;
	m_bUnk20         = a_bUnk20;
	m_bCaptureWait   = a_bWait;

	if ( m_uiCaptureState != 0 )
	{
		if ( m_uiCaptureState - 1 > 3 )
			return TFALSE;

		if ( m_pCapture != TNULL )
		{
			m_pCapture->Unk08();
			TRender::GetSingleton()->DestroyScreenCapture( m_pCapture );
			m_pCapture = TNULL;
		}

		m_pCapture = TNULL;
	}

	m_uiCaptureState = 1;
	return TTRUE;
}

// $deBlob: FUNCTION 005ef570
void ARenderer::UpdateCapture( TRender* a_pRender )
{
	if ( m_uiCaptureState == 1 )
	{
		a_pRender->SetCurrentRenderContext( m_pViewportManager->GetCurrentViewport()->GetRenderContext() );

		m_pCapture = TSTATICCAST( ScreenCapture, a_pRender->CreateScreenCapture() );
		m_pCapture->Create( 0, m_iCaptureWidth, m_iCaptureHeight );
		m_pCapture->Start();
		m_uiCaptureState = 2;

		if ( m_bCaptureWait )
		{
			// NOTE: retail has no deadline either
			while ( !m_pCapture->IsReady() );

			FinishCapture( a_pRender );
			m_uiCaptureState = 1;
		}
	}
	else if ( m_uiCaptureState == 2 && m_pCapture->IsReady() )
	{
		FinishCapture( a_pRender );
	}
}

// $deBlob: FUNCTION 005efa20
void ARenderer::FinishCapture( TRender* a_pRender )
{
	m_uiCaptureState = 3;

	if ( m_pCaptureData != TNULL )
		delete[] TSTATICCAST( TUINT8, m_pCaptureData );

	TSIZE uiSize   = m_iCaptureHeight * m_iCaptureWidth * 4;
	m_pCaptureData = new TUINT8[ uiSize ];
	TUtil::MemCopy( m_pCaptureData, m_pCapture->GetData(), uiSize );
	m_pCapture->Unk08();

	a_pRender->DestroyScreenCapture( m_pCapture );
	m_pCapture = TNULL;
}

// $deBlob: FUNCTION 005ef610
void ARenderer::GetAppCamera()
{
	TASSERT( ACameraManager::IsSingletonCreated() );

	for ( AXYZViewportManager::VIEWPORT i = AXYZViewportManager::VIEWPORT_Count - 1; i >= AXYZViewportManager::VIEWPORT_FullScreen; i-- )
	{
		if ( !m_pViewportManager->IsViewportUsed( i ) )
			continue;

		ACamera* pCamera = m_pViewportManager->GetCamera( i );

		if ( pCamera == TNULL )
		{
			pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
			if ( pCamera == TNULL ) continue;
		}

		ACamera::CameraMatrix& rCamMatrix = pCamera->GetCameraMatrix();

		TMatrix44 mFinal;
		mFinal.Multiply( rCamMatrix.m_mMatrix, rCamMatrix.m_mShakeMatrix );

		TCameraObject* pCameraObject = m_pViewportManager->GetCameraObject( i );
		pCameraObject->SetProjectionCentreX( rCamMatrix.m_fProjectionCentreX );
		pCameraObject->SetProjectionCentreY( rCamMatrix.m_fProjectionCentreY );
		pCameraObject->SetFOV( rCamMatrix.m_fFOV );
		pCameraObject->GetTransformObject().SetMatrix( mFinal );
		pCameraObject->SetFar( rCamMatrix.m_fFar );
		pCameraObject->SetNear( rCamMatrix.m_fNear );
	}
}

// $deBlob: FUNCTION 005efa90
void ARenderer::InitialiseViewPort()
{
	TUINT8 uiRed   = TUINT8( m_uiBackgroundColour >> 16 );
	TUINT8 uiGreen = TUINT8( m_uiBackgroundColour >> 8 );
	TUINT8 uiBlue  = TUINT8( m_uiBackgroundColour );

	for ( AXYZViewportManager::VIEWPORT i = AXYZViewportManager::VIEWPORT_Count - 1; i >= AXYZViewportManager::VIEWPORT_FullScreen; i-- )
	{
		if ( m_pViewportManager->IsViewportUsed( i ) )
			m_pViewportManager->GetViewport( i )->SetBackgroundColor( uiRed, uiGreen, uiBlue, 255 );
	}
}

// Throws the frame event
// $deBlob: FUNCTION 005eeb80
void ARenderer::ThrowFrameEvent()
{
	// NOTE: retail passes the address of an uninitialised local
	TUINT8 uiData = 0;
	m_FrameEmitter.Throw( &uiData );
}

// $deBlob: FUNCTION 005eeba0
void ARenderer::RenderMainScene( TFLOAT a_fDeltaTime, TViewport* a_pViewport, TCameraObject* a_pCameraObject, ACamera* a_pCamera, t_MainSceneCallback a_fnMainScene, TBOOL a_bAllowBackgroundClear )
{
	a_pViewport->AllowDepthClear( TTRUE );

	TRenderDX11*    pRender          = TRenderDX11::Interface();
	TRenderContext* pRenderContext   = a_pViewport->GetRenderContext();
	TRenderContext* pOriginalContext = pRender->SetCurrentRenderContext( pRenderContext );

	pRender->GetTransforms().Reset();
	pRender->GetTransforms().Push( TMatrix44::IDENTITY );

	pRenderContext->SetFlag( TRenderContext::FLAG_FOG, m_bFogEnabled );

	if ( m_bFogEnabled )
	{
		pRenderContext->SetFogColor( TVector4( m_afFogColour[ 0 ], m_afFogColour[ 1 ], m_afFogColour[ 2 ], m_afFogColour[ 3 ] ) );

		TFLOAT fFogStart = TMath::Max( a_pCameraObject->GetNear(), m_fFogStart );
		TFLOAT fFogEnd   = TMath::Min( a_pCameraObject->GetFar(), m_fFogEnd );
		pRenderContext->SetFogDistance( fFogStart, fFogEnd );
	}

	a_pViewport->AllowBackgroundClear( a_bAllowBackgroundClear );
	a_pViewport->Begin();

	TWorldShaderHAL::ResetWaterFrame();

	a_pCameraObject->Render();
	pRender->GetTransforms().Push( a_pViewport->GetRenderContext()->GetWorldViewMatrix() );

	TCameraObject* pOldCameraObject = a_pViewport->GetRenderContext()->GetCameraObject();
	a_pViewport->GetRenderContext()->SetCameraObject( a_pCameraObject );

	// The world planes of the frustum (for the culling done by the camera's users)
	TRenderContext* pCurrentContext = TRender::GetSingleton()->GetCurrentContext();
	TPlane          aWorldPlanes[ 6 ];
	pCurrentContext->GetWorldPlanes( aWorldPlanes, 6 );
	pCurrentContext->SetModelViewMatrix( pCurrentContext->GetWorldViewMatrix() );
	a_pCamera->SetViewPlanes( aWorldPlanes, 6 );

	pCurrentContext->SetAmbientColour( TVector4( m_afAmbientColour[ 0 ], m_afAmbientColour[ 1 ], m_afAmbientColour[ 2 ], m_afAmbientColour[ 3 ] ) );

	// The directional lights of the parameter table as the light matrices (one light per row)
	TRenderParamTable* pParamTable      = TRender::GetSingleton()->GetParamTable();
	TVector4           vDirLight0Colour = pParamTable->GetParameterV4( TRenderParamTable::V4PARAM_DIRLIGHT0COLOUR );
	TVector4           vDirLight1Colour = pParamTable->GetParameterV4( TRenderParamTable::V4PARAM_DIRLIGHT1COLOUR );
	TVector4           vDirLight2Colour = pParamTable->GetParameterV4( TRenderParamTable::V4PARAM_DIRLIGHT2COLOUR );
	TVector4           vDirLight0       = pParamTable->GetParameterV4( TRenderParamTable::V4PARAM_DIRLIGHT0 );
	TVector4           vDirLight1       = pParamTable->GetParameterV4( TRenderParamTable::V4PARAM_DIRLIGHT1 );
	TVector4           vDirLight2       = pParamTable->GetParameterV4( TRenderParamTable::V4PARAM_DIRLIGHT2 );

	TMatrix44 mLightDirection = TMatrix44::IDENTITY;
	TMatrix44 mLightColour    = TMatrix44::IDENTITY;

	TASSERT( vDirLight0.w == 0.0f );
	mLightDirection.AsBasisVector4( BASISVECTOR_RIGHT ) = vDirLight0;
	TASSERT( vDirLight1.w == 0.0f );
	mLightDirection.AsBasisVector4( BASISVECTOR_UP ) = vDirLight1;
	TASSERT( vDirLight2.w == 0.0f );
	mLightDirection.AsBasisVector4( BASISVECTOR_FORWARD ) = vDirLight2;
	TRender::GetSingleton()->SetLightDirectionMatrix( mLightDirection );

	TASSERT( vDirLight0Colour.w == 0.0f );
	mLightColour.AsBasisVector4( BASISVECTOR_RIGHT ) = vDirLight0Colour;
	TASSERT( vDirLight1Colour.w == 0.0f );
	mLightColour.AsBasisVector4( BASISVECTOR_UP ) = vDirLight1Colour;
	TASSERT( vDirLight2Colour.w == 0.0f );
	mLightColour.AsBasisVector4( BASISVECTOR_FORWARD ) = vDirLight2Colour;
	TRender::GetSingleton()->SetLightColourMatrix( mLightColour );

	a_fnMainScene( a_fDeltaTime, a_pCamera );

	a_pViewport->GetRenderContext()->SetCameraObject( pOldCameraObject );
	pRender->GetTransforms().Pop();

	a_pViewport->End();
	pRender->FlushShaders();

	if ( s_bGlow )
	{
		pRender->ResolveSubresource();
		TRenderDX11::Interface()->GetGlow()->Render( TRenderDX11::Interface()->GetResolveSRView(), TNULL );
	}

	// The entity factory's callbacks
	if ( AEntityFactory::GetSingleton() != TNULL )
	{
		a_pCameraObject->Render();
		pRender->GetTransforms().Push( a_pViewport->GetRenderContext()->GetWorldViewMatrix() );
		AEntityFactory::GetSingleton()->CallCallbacks();
		pRender->GetTransforms().Pop();
	}

	pRender->FlushShaders();
	pRender->SetCurrentRenderContext( pOriginalContext );
}

// $deBlob: FUNCTION 005efdf0
void __stdcall ARenderer::FadeCameraInstances( ACamera* a_pCamera )
{
	if ( a_pCamera->GetCameraType() == ACamera::CT_BLOBFOLLOW )
	{
		ABlobFollowCamera* pCamera = TSTATICCAST( ABlobFollowCamera, a_pCamera );

		TINT              iCount;
		AModelInstance2** ppInstances = pCamera->GetAlphaInstances( iCount );

		for ( TINT i = 0; i < iCount; i++ )
		{
			if ( ppInstances[ i ] != TNULL )
			{
				pCamera->SetAlphaTarget( i, ppInstances[ i ]->GetModelInstance()->GetFade() );
				TFLOAT fAlpha = pCamera->GetAlpha( i );

				TModelInstance* pModelInstance = ppInstances[ i ]->GetModelInstance();
				pModelInstance->SetForceFade( TTRUE );
				pModelInstance->SetFade( fAlpha );
			}
		}
	}
}

// $deBlob: FUNCTION 005efec0
void __stdcall ARenderer::RestoreCameraInstances( ACamera* a_pCamera )
{
	if ( a_pCamera->GetCameraType() == ACamera::CT_BLOBFOLLOW )
	{
		ABlobFollowCamera* pCamera = TSTATICCAST( ABlobFollowCamera, a_pCamera );

		TINT              iCount;
		AModelInstance2** ppInstances = pCamera->GetAlphaInstances( iCount );

		for ( TINT i = 0; i < iCount; i++ )
		{
			if ( ppInstances[ i ] != TNULL )
			{
				TFLOAT fAlpha = pCamera->GetAlphaTarget( i );

				TModelInstance* pModelInstance = ppInstances[ i ]->GetModelInstance();
				pModelInstance->SetForceFade( TTRUE );
				pModelInstance->SetFade( fAlpha );
			}
		}
	}
}

// $deBlob: FUNCTION 005efc70
void __stdcall ARenderer::MainScene( TFLOAT a_fDeltaTime, ACamera* a_pCamera )
{
	if ( !g_oTheApp.ShouldRenderWorld() )
		return;

	AModelInstance2::ms_RenderCount = 0;

	if ( AGameStatus::GetSingleton() != TNULL )
		AGameStatus::GetSingleton()->UpdateNextBuildingGlintCameras();

	if ( AJellyShader_DX11::GetSingleton() != TNULL )
	{
		AJellyShader_DX11::GetSingleton()->PrepareConfigs( a_fDeltaTime );
		AJellyShader_DX11::GetSingleton()->RenderConfigs( a_fDeltaTime, a_pCamera );
	}

	APaintParticleSystem::s_Instance->AddRenderCall( a_pCamera );

	if ( AEntityFactory::GetSingleton() != TNULL )
		AEntityFactory::GetSingleton()->PreRenderUpdate();

	// The UV animation clock of the materials
	if ( AMaterialManager::GetSingleton() != TNULL )
	{
		AMaterialManager* pMaterialManager = AMaterialManager::GetSingleton();
		TFLOAT            fOldTime         = pMaterialManager->m_fTime;
		TFLOAT            fNewTime         = fOldTime + a_fDeltaTime;
		TINT              iFrame           = TSystemManager::GetSingleton()->GetFrameCount();

		pMaterialManager->m_fTime = fNewTime;
		// NOTE: a time that stopped advancing (float precision) restarts from 0
		if ( fNewTime == fOldTime && a_fDeltaTime > 0.0f )
			pMaterialManager->m_fTime = 0.0f;

		pMaterialManager->m_iFrame = iFrame;
	}

	if ( AModelFactory::GetSingleton() != TNULL )
	{
		FadeCameraInstances( a_pCamera );
		AModelFactory::GetSingleton()->Render();
		RestoreCameraInstances( a_pCamera );
	}

	// NOTE: slot +0x0C of AGameState (AInGameState renders there)
	AGameStateController::GetSingleton()->GetCurrentGameState()->HandleMenuRequestToCancel();

	if ( ATerrain2Interface::GetInstance() )
		ATerrain2Interface::GetInstance()->Render();

	AShadow::BatchSumbit();

	if ( AInstanceRenderer::GetInstance() != TNULL )
		AInstanceRenderer::GetInstance()->Render();

	if ( AEnvironmentFX::GetSingleton() != TNULL )
		AEnvironmentFX::GetSingleton()->Render( a_pCamera );

	if ( !s_bHideParticles && TParticleSystem::GetSingleton() != TNULL )
		TParticleSystem::GetSingleton()->Render();

	if ( ADecalManager::GetSingleton() != TNULL )
		ADecalManager::GetSingleton()->AddRenderCall( a_fDeltaTime, a_pCamera );

	if ( AStencilShadow::IsSingletonCreated() )
		AStencilShadow::GetSingleton()->Render( a_fDeltaTime, a_pCamera );
}

// $deBlob: FUNCTION 005edd30
void ARenderer::ProjectToScreen( TVector4& a_rPosition, TViewport* a_pViewport )
{
	TRenderContext* pRenderContext = m_pViewportManager->GetCurrentViewport()->GetRenderContext();

	if ( a_pViewport != TNULL )
		pRenderContext = a_pViewport->GetRenderContext();

	TVector4 vViewPos;
	TMatrix44::TransformVector( vViewPos, pRenderContext->GetWorldViewMatrix(), a_rPosition );

	const TRenderContext::PROJECTIONPARAMS& rProjParams = pRenderContext->GetProjectionParameters();

	if ( vViewPos.z < rProjParams.m_fNearClip )
	{
		a_rPosition.x = -100.0f;
		a_rPosition.y = -100.0f;
		return;
	}

	TFLOAT fY = vViewPos.y * ( 1.0f / vViewPos.z ) * rProjParams.m_Proj.y + rProjParams.m_Centre.y;

	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();
	TFLOAT                  fWidth         = m_pViewport->GetWidth();

	a_rPosition.y = ( fWidth / TFLOAT( TINT( pDisplayParams->uiWidth ) ) ) * fY;
	a_rPosition.x = ( fWidth / TFLOAT( TINT( pDisplayParams->uiWidth ) ) ) * ( vViewPos.x * ( 1.0f / vViewPos.z ) * rProjParams.m_Proj.x + rProjParams.m_Centre.x );
}

// $deBlob: FUNCTION 005ee000
TBOOL ARenderer::ConvertWorldToScreen( const TVector4& a_rvWorld, TVector2& a_rvScreen )
{
	return ConvertWorldToScreenClipped( a_rvWorld, a_rvScreen, 0.0f, 0.0f, 0.0f, 0.0f ) == 0;
}

// $deBlob: FUNCTION 005ee040
TINT ARenderer::ConvertWorldToScreenClipped( const TVector4& a_rvWorld, TVector2& a_rvScreen, TFLOAT a_fLeft, TFLOAT a_fRight, TFLOAT a_fTop, TFLOAT a_fBottom )
{
	// Updates the camera object of the main viewport
	TCameraObject*  pCameraObject    = m_pViewportManager->GetCurrentCameraObject();
	TRenderContext* pOriginalContext = TRender::GetSingleton()->GetCurrentContext();

	TRender::GetSingleton()->SetCurrentRenderContext( m_pViewport->GetRenderContext() );
	pCameraObject->Render();
	TRender::GetSingleton()->SetCurrentRenderContext( pOriginalContext );

	TFLOAT fWidth  = m_pViewport->GetRenderContext()->GetWidth();
	TFLOAT fHeight = m_pViewport->GetRenderContext()->GetHeight();

	TASSERT( ACameraManager::IsSingletonCreated() );

	TVector4 vCameraPosition = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

	const TMatrix44& rcForwardMatrix = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
	TVector4         vForward( rcForwardMatrix.m_f31, rcForwardMatrix.m_f32, rcForwardMatrix.m_f33, 1.0f );

	TVector4 vUp = ACameraManager::GetSingleton()->GetCurrentCamera()->GetUp();

	TVector4 vDirection( a_rvWorld.x - vCameraPosition.x, a_rvWorld.y - vCameraPosition.y, a_rvWorld.z - vCameraPosition.z, a_rvWorld.w );
	vDirection.Normalise();

	// At the camera
	if ( vDirection.y * vDirection.y + vDirection.x * vDirection.x + vDirection.z * vDirection.z < 0.1f * 0.1f )
	{
		a_rvScreen.x = -1.0f;
		a_rvScreen.y = -1.0f;
		return 9;
	}

	// The angles of the edges of the screen (minus the margins)
	TFLOAT fHalfFOV         = ACameraManager::GetSingleton()->GetCurrentCamera()->GetCameraMatrix().m_fFOV * 0.5f;
	TBOOL  bLeft            = TFALSE;
	TBOOL  bTop             = TFALSE;
	TFLOAT fHalfHeight      = fHeight * 0.5f;
	TFLOAT fHalfVerticalFOV = ( fHalfFOV * fHeight ) / fWidth;
	TFLOAT fHalfWidth       = fWidth * 0.5f;
	TFLOAT fLeftAngle       = ( ( fHalfWidth - a_fLeft ) * fHalfFOV ) / fHalfWidth;
	TFLOAT fRightAngle      = ( ( fHalfWidth - a_fRight ) * fHalfFOV ) / fHalfWidth;
	TFLOAT fTopAngle        = ( ( fHalfHeight - a_fTop ) * fHalfVerticalFOV ) / fHalfHeight;
	TFLOAT fBottomAngle     = ( ( fHalfHeight - a_fBottom ) * fHalfVerticalFOV ) / fHalfHeight;

	TFLOAT fUpDot = vDirection.y * vUp.y + vDirection.x * vUp.x + vDirection.z * vUp.z;

	TVector4 vVertical;
	vVertical.Multiply( vUp, fUpDot );

	TVector4 vFlat( vDirection.x - vVertical.x, vDirection.y - vVertical.y, vDirection.z - vVertical.z, vDirection.w );

	if ( vFlat.MagnitudeSq() < 0.0001f )
	{
		// Straight above or below the camera
		a_rvScreen.x = fHalfWidth;

		if ( fUpDot <= 0.0f )
		{
			a_rvScreen.y = fHeight - a_fBottom;
			return 8;
		}

		a_rvScreen.y = a_fTop;
		return 8;
	}

	vFlat.Normalise();

	// Horizontally
	TFLOAT fCos = vForward.y * vFlat.y + vForward.x * vFlat.x + vForward.z * vFlat.z;
	TMath::Clip( fCos, -1.0f, 1.0f );

	TVector4 vCross;
	vCross.CrossProduct( vFlat, vForward );

	TFLOAT fSign   = TMath::Sign( vCross.y * vUp.y + vCross.x * vUp.x + vCross.z * vUp.z );
	TFLOAT fAngleX = TMath::ACos( fCos ) * fSign;
	TBOOL  bOnScreenX;

	if ( fAngleX < -fLeftAngle || fRightAngle < fAngleX )
	{
		bOnScreenX = TFALSE;

		if ( fAngleX >= 0.0f )
			a_fLeft = fWidth - a_fRight;
		else
			bLeft = TTRUE;
	}
	else
	{
		bOnScreenX = TTRUE;
		a_fLeft    = ( ( fAngleX * fWidth ) / fHalfFOV + fWidth ) * 0.5f;
	}

	a_rvScreen.x = a_fLeft;

	// Vertically
	TFLOAT fCosY = vFlat.y * vDirection.y + vFlat.x * vDirection.x + vFlat.z * vDirection.z;
	TMath::Clip( fCosY, -1.0f, 1.0f );

	TFLOAT fSignY  = TMath::Sign( -fUpDot );
	TFLOAT fAngleY = TMath::ACos( fCosY ) * fSignY;
	TBOOL  bOnScreenY;

	if ( fAngleY < -fTopAngle || fBottomAngle < fAngleY )
	{
		bOnScreenY = TFALSE;

		if ( fAngleY >= 0.0f )
		{
			bTop   = TFALSE;
			a_fTop = fHeight - a_fBottom;
		}
		else
		{
			bTop = TTRUE;
		}
	}
	else
	{
		bOnScreenY = TTRUE;
		a_fTop     = ( ( fAngleY * fHeight ) / fHalfVerticalFOV + fHeight ) * 0.5f;
	}

	a_rvScreen.y = a_fTop;

	// 0: on screen, 1-8: the side it's clipped to (left/right, top/bottom and the corners)
	if ( bOnScreenX )
		return bOnScreenY ? 0 : ( !bTop ) + 3;

	if ( bOnScreenY )
		return ( !bLeft ) + 1;

	if ( bTop )
		return ( !bLeft ) + 5;

	return bLeft ? 7 : 8;
}

// $deBlob: FUNCTION 005ee5e0
void ARenderer::ScreenToCanvas( TFLOAT& a_rX, TFLOAT& a_rY )
{
	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();
	TXUICanvas*             pCanvas        = TXUI::GetSingleton()->GetCanvas();

	TFLOAT fWidth, fHeight;
	pCanvas->GetDimensions( fWidth, fHeight );

	a_rX -= pCanvas->GetPositionX();
	a_rY -= pCanvas->GetPositionY();
	a_rX /= TFLOAT( TINT( pDisplayParams->uiWidth ) ) / fWidth;
	a_rY /= TFLOAT( TINT( pDisplayParams->uiHeight ) ) / fHeight;
	TMath::Clip( a_rX, -5000.0f, 5000.0f );
	TMath::Clip( a_rY, -5000.0f, 5000.0f );
}

// $deBlob: FUNCTION 005ede20
void ARenderer::ProjectToGUI( TVector4& a_rPosition, TViewport* a_pViewport )
{
	ProjectToScreen( a_rPosition, a_pViewport );

	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

	TFLOAT fWidth, fHeight;
	T2GUI::GetSingleton()->GetRootElement()->GetDimensions( fWidth, fHeight );

	TFLOAT fX     = a_rPosition.x - fWidth * 0.5f;
	TFLOAT fY     = a_rPosition.y - fHeight * 0.5f;
	a_rPosition.x = fX;
	a_rPosition.y = fY;
	TFLOAT fScaleX = TFLOAT( TINT( pDisplayParams->uiWidth ) ) / fWidth;
	TFLOAT fScaleY = TFLOAT( TINT( pDisplayParams->uiHeight ) ) / fHeight;
	a_rPosition.y  = fY / fScaleY;
	a_rPosition.x  = fX / fScaleX;
}

// $deBlob: FUNCTION 005edec0
void ARenderer::ProjectToCanvas( TVector4& a_rPosition, TViewport* a_pViewport )
{
	ProjectToScreen( a_rPosition, a_pViewport );
	ScreenToCanvas( a_rPosition.x, a_rPosition.y );
}

// $deBlob: FUNCTION 005ecab0
TBOOL ARenderer::RGBToHSV( TINT a_iR, TINT a_iG, TINT a_iB, TFLOAT* a_pH, TFLOAT* a_pS, TFLOAT* a_pV )
{
	TFLOAT fMax = TMath::Max( TMath::Max( a_iR, a_iG ), a_iB ) / 255.0f;
	TFLOAT fMin = TMath::Min( TMath::Min( a_iR, a_iG ), a_iB ) / 255.0f;
	TFLOAT fR   = a_iR / 255.0f;
	TFLOAT fG   = a_iG / 255.0f;
	TFLOAT fB   = a_iB / 255.0f;

	TASSERT( fR >= 0.0f && fR <= 1.0f );
	TASSERT( fG >= 0.0f && fG <= 1.0f );
	TASSERT( fB >= 0.0f && fB <= 1.0f );

	if ( fMax == fMin )
		return TFALSE;

	if ( fMax == fR && fB <= fG )
		*a_pH = ( ( fG - fB ) / ( fMax - fMin ) ) * 60.0f;
	else if ( fMax == fR && fG < fB )
		*a_pH = ( ( fG - fB ) / ( fMax - fMin ) ) * 60.0f + 360.0f;
	else if ( fMax == fG )
		*a_pH = ( ( fB - fR ) / ( fMax - fMin ) ) * 60.0f + 120.0f;
	else if ( fMax == fB )
		*a_pH = ( ( fR - fG ) / ( fMax - fMin ) ) * 60.0f + 240.0f;

	*a_pS = 0.0f;

	if ( 0.0f < fMax )
		*a_pS = 1.0f - fMin / fMax;

	*a_pV = fMax;

	TASSERT( *a_pH >= 0.0f && *a_pH <= 360.0f );
	TASSERT( *a_pS >= 0.0f && *a_pS <= 1.0f );
	TASSERT( *a_pV >= 0.0f && *a_pV <= 1.0f );
	return TTRUE;
}

// $deBlob: FUNCTION 005ece70
void ARenderer::HSVToRGB( TFLOAT a_fH, TFLOAT a_fS, TFLOAT a_fV, TUINT32* a_pOutColour )
{
	TASSERT( a_fH >= 0.0f && a_fH <= 360.0f );
	TASSERT( a_fS >= 0.0f && a_fS <= 1.0f );
	TASSERT( a_fV >= 0.0f && a_fV <= 1.0f );

	TFLOAT fH  = a_fH / 60.0f;
	TFLOAT fHi = TFLOAT( fmod( TDOUBLE( TFLOAT( TMath::FloorToInt( fH ) ) ), 6.0 ) );
	TASSERT( (TFLOAT)( (TINT)fHi ) == fHi );

	TFLOAT fF = fH - fHi;
	TFLOAT fP = ( 1.0f - a_fS ) * a_fV;
	TFLOAT fQ = ( 1.0f - fF * a_fS ) * a_fV;
	TFLOAT fT = ( 1.0f - ( 1.0f - fF ) * a_fS ) * a_fV;
	TFLOAT fR, fG, fB;

	switch ( TINT( fHi ) )
	{
		case 0: fR = a_fV, fG = fT, fB = fP; break;
		case 1: fR = fQ, fG = a_fV, fB = fP; break;
		case 2: fR = fP, fG = a_fV, fB = fT; break;
		case 3: fR = fP, fG = fQ, fB = a_fV; break;
		case 4: fR = fT, fG = fP, fB = a_fV; break;
		case 5: fR = a_fV, fG = fP, fB = fQ; break;
		default: fR = fH, fG = fH, fB = fH; break;
	}

	*a_pOutColour = TUINT32( TINT( fB * 255.0f ) | ( ( TINT( fG * 255.0f ) | ( ( TINT( fR * 255.0f ) | 0xFF00 ) << 8 ) ) << 8 ) );
}

// $deBlob: FUNCTION 005ece00
void ARenderer::HSVToRGB( TFLOAT a_fH, TFLOAT a_fS, TFLOAT a_fV, TUINT32* a_pOutR, TUINT32* a_pOutG, TUINT32* a_pOutB )
{
	TUINT32 uiColour = 0;
	HSVToRGB( a_fH, a_fS, a_fV, &uiColour );

	*a_pOutB = uiColour & 0xFF;
	*a_pOutG = ( uiColour >> 8 ) & 0xFF;
	*a_pOutR = ( uiColour >> 16 ) & 0xFF;
}

// $deBlob: FUNCTION 005edc50
void ARenderer::ProjectToScreen( TVector3& a_rvPosition )
{
	TCameraObject*  pCameraObject = m_pViewportManager->GetCurrentCameraObject();
	TRenderContext* pOldContext   = TRender::GetSingleton()->GetCurrentContext();
	TRender::GetSingleton()->SetCurrentRenderContext( m_pViewport->GetRenderContext() );
	pCameraObject->Render();
	TRender::GetSingleton()->SetCurrentRenderContext( pOldContext );

	TRenderContext*                         pContext = m_pViewportManager->GetCurrentViewport()->GetRenderContext();
	const TRenderContext::PROJECTIONPARAMS& rParams  = pContext->GetProjectionParameters();

	TVector3 vView;
	TMatrix44::TransformVector( vView, pContext->GetWorldViewMatrix(), a_rvPosition );

	if ( vView.z < rParams.m_fNearClip )
	{
		a_rvPosition.x = -100.0f;
		a_rvPosition.y = -100.0f;
		return;
	}

	a_rvPosition.y = vView.y * ( 1.0f / vView.z ) * rParams.m_Proj.y + rParams.m_Centre.y;
	a_rvPosition.x = vView.x * ( 1.0f / vView.z ) * rParams.m_Proj.x + rParams.m_Centre.x;
}

// $deBlob: FUNCTION 005edef0
TBOOL ARenderer::ProjectToScreen( ACamera* a_pCamera, const TVector3& a_rvPosition, TVector2& a_rOutScreen )
{
	TCameraObject*  pCameraObject = m_pViewportManager->GetCurrentCameraObject();
	TRenderContext* pOldContext   = TRender::GetSingleton()->GetCurrentContext();
	TRender::GetSingleton()->SetCurrentRenderContext( m_pViewport->GetRenderContext() );
	pCameraObject->Render();
	TRender::GetSingleton()->SetCurrentRenderContext( pOldContext );

	const TRenderContext::PROJECTIONPARAMS& rParams = m_pViewportManager->GetCurrentViewport()->GetRenderContext()->GetProjectionParameters();

	TMatrix44 mView = a_pCamera->GetMatrix();
	mView.InvertOrthonormal();

	TVector3 vView;
	TMatrix44::TransformVector( vView, mView, a_rvPosition );

	a_rOutScreen.x = vView.x * ( 1.0f / vView.z ) * rParams.m_Proj.x + rParams.m_Centre.x;
	a_rOutScreen.y = vView.y * ( 1.0f / vView.z ) * rParams.m_Proj.y + rParams.m_Centre.y;

	return rParams.m_fNearClip <= vView.z && vView.z <= rParams.m_fFarClip;
}

// $deBlob: FUNCTION 005ee6d0
void ARenderer::GetScreenRect( const TVector3& a_rvPosition, TFLOAT a_fRadius, TFLOAT* a_pOutX, TFLOAT* a_pOutY, TFLOAT* a_pOutWidth, TFLOAT* a_pOutHeight )
{
	TVector3 vMax = a_rvPosition;
	TVector3 vMin = a_rvPosition;

	if ( a_fRadius < 4.0f )
		a_fRadius = 4.0f;

	TVector3 vOffset( 1.0f * a_fRadius, 0.0f * a_fRadius, 0.0f * a_fRadius );
	vOffset.RotateY( TMath::PI + 0.0f );

	vMax.x += vOffset.x;
	vMax.y += vOffset.y;
	vMax.z += vOffset.z;
	vMin.x -= vOffset.x;
	vMin.y -= vOffset.y;
	vMin.z -= vOffset.z;

	ProjectToScreen( vMax );
	ProjectToScreen( vMin );

	*a_pOutWidth  = vMin.x - vMax.x;
	*a_pOutHeight = vMin.x - vMax.x;
	*a_pOutX      = vMax.x;
	*a_pOutY      = vMax.y - *a_pOutHeight * 0.5f;
}

// $deBlob: FUNCTION 005ee820
void ARenderer::GetScreenRay( ACamera* a_pCamera, const TVector2& a_rcScreen, TVector3& a_rOutOrigin, TVector3& a_rOutDirection )
{
	TQuaternion qRotation;
	TMatrix44::MatToQuat( qRotation, a_pCamera->GetMatrix() );
	GetScreenRay( a_pCamera->GetMatrix().GetTranslation3(), qRotation, a_rcScreen, a_rOutOrigin, a_rOutDirection );
}

// $deBlob: FUNCTION 005ee860
void ARenderer::GetScreenRay( const TVector3& a_rvPosition, const TQuaternion& a_rcRotation, const TVector2& a_rcScreen, TVector3& a_rOutOrigin, TVector3& a_rOutDirection )
{
	TCameraObject*  pCameraObject = m_pViewportManager->GetCurrentCameraObject();
	TRenderContext* pOldContext   = TRender::GetSingleton()->GetCurrentContext();
	TRender::GetSingleton()->SetCurrentRenderContext( m_pViewport->GetRenderContext() );
	pCameraObject->Render();
	TRender::GetSingleton()->SetCurrentRenderContext( pOldContext );

	const TRenderContext::PROJECTIONPARAMS& rParams = m_pViewport->GetRenderContext()->GetProjectionParameters();

	TVector3 vScreen( ( a_rcScreen.x - rParams.m_Centre.x ) / rParams.m_Proj.x, ( a_rcScreen.y - rParams.m_Centre.y ) / rParams.m_Proj.y, 1.0f );

	TMatrix44 mCamera;
	mCamera.SetFromQuaternion( a_rcRotation );
	mCamera.m_f41 = a_rvPosition.x;
	mCamera.m_f42 = a_rvPosition.y;
	mCamera.m_f43 = a_rvPosition.z;

	TMatrix44::TransformVector( a_rOutDirection, mCamera, vScreen );

	a_rOutOrigin = TVector3::VEC_ZERO;
	TMatrix44::TransformVector( a_rOutOrigin, mCamera, a_rOutOrigin );

	a_rOutDirection.x -= a_rOutOrigin.x;
	a_rOutDirection.y -= a_rOutOrigin.y;
	a_rOutDirection.z -= a_rOutOrigin.z;
	a_rOutDirection.Normalize();
}

// $deBlob: FUNCTION 005ee970
void ARenderer::SetUnk34( TBOOL a_bUnk34 )
{
	m_bUnk34 = a_bUnk34;
}

// A tenth of the main viewport's height with m_bUnk34, else 0
// $deBlob: FUNCTION 005ee980
TFLOAT ARenderer::GetUnk34Height() const
{
	if ( m_bUnk34 == TTRUE )
		return m_pViewportManager->GetCurrentViewport()->GetRenderContext()->GetHeight() / 10.0f;

	return 0.0f;
}

// The width and height of the main viewport
// $deBlob: FUNCTION 005ee9b0
void ARenderer::GetMainViewportSize( Toshi::TVector2& a_rOutSize ) const
{
	TRenderContext* pRenderContext = m_pViewportManager->GetCurrentViewport()->GetRenderContext();
	a_rOutSize.x                   = pRenderContext->GetWidth();
	a_rOutSize.y                   = pRenderContext->GetHeight();
}

// The captured image (FinishCapture)
// $deBlob: FUNCTION 005eea50
void* ARenderer::GetCaptureData() const
{
	return m_pCaptureData;
}

// Frees the captured image
// $deBlob: FUNCTION 005eea60
void ARenderer::ReleaseCapture()
{
	if ( m_pCaptureData != TNULL )
	{
		delete[] TSTATICCAST( TUINT8, m_pCaptureData );
		m_pCaptureData = TNULL;
	}

	m_uiCaptureState = 0;
}

// $deBlob: FUNCTION 005eeaa0
void __stdcall ARenderer::FindDepthStencilFormat( TRender::DISPLAYPARAMS& a_rDisplayParams, TBOOL a_bPreferStencil )
{
	TRender* pRender = TRender::GetSingleton();

	if ( a_bPreferStencil == TTRUE )
	{
		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 4 );
		if ( pRender->FindDevice( a_rDisplayParams ) ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 2 );
		if ( pRender->FindDevice( a_rDisplayParams ) ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 3 );
		if ( pRender->FindDevice( a_rDisplayParams ) ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 0 );
		if ( pRender->FindDevice( a_rDisplayParams ) ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 1 );
		pRender->FindDevice( a_rDisplayParams );
	}
	else
	{
		// NOTE: retail searches with the caller's format first, then sets format 0 even when that search succeeded
		TRenderAdapter::Mode::Device* pDevice = pRender->FindDevice( a_rDisplayParams );
		a_rDisplayParams.eDepthStencilFormat  = TRender::DEPTHSTENCILFORMAT( 0 );
		if ( pDevice ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 1 );
		if ( pRender->FindDevice( a_rDisplayParams ) ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 2 );
		if ( pRender->FindDevice( a_rDisplayParams ) ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 4 );
		if ( pRender->FindDevice( a_rDisplayParams ) ) return;

		a_rDisplayParams.eDepthStencilFormat = TRender::DEPTHSTENCILFORMAT( 3 );
		pRender->FindDevice( a_rDisplayParams );
	}
}
