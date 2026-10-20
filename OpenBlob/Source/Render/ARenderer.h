#pragma once
#include "Render/ADisplayModes_Win.h"
#include <Math/TVector2.h>
#include <Math/TQuaternion.h>
#include <Render/TRender.h>

#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>
#include <Toshi/THPTimer.h>
#include <File/TTRB.h>
#include <Render/TViewport.h>

class AXYZViewportManager;
class AModelInstance2;

namespace Toshi
{
class TRender;
class T2GUIRectangle;
} // namespace Toshi

class ACamera;
TOSHI_NAMESPACE_START
class TScene;
class TCameraObject;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the game's renderer (ARenderer_Win.cpp): creates the render interface and its display, the shaders and
// viewports, renders a frame (the world through the viewports, the GUI).
//-----------------------------------------------------------------------------
class ARenderer : public Toshi::TSingleton<ARenderer>
{
	friend class AEnvironmentFX; // sets the fog

public:
	// The index of the camera being rendered (split screen)
	static TINT GetCurrentCameraIndex() { return s_iCurrentCamera; }

	// "-windowed" size when -width/-height aren't given
	static TUINT32 s_uiDefaultWidth;
	static TUINT32 s_uiDefaultHeight;

	// "antialias" of setup.ini, FXAA when not 0
	static TINT s_iAntialiasLevel;

public:
	// The screen capture of the platform layer (TRender::CreateScreenCapture); DX11 has none.
	class ScreenCapture
	{
	public:
		virtual ~ScreenCapture()                                           = default; // +0x00
		virtual void  Create( TINT a_iUnk, TINT a_iWidth, TINT a_iHeight ) = 0;       // +0x04
		virtual void  Unk08()                                              = 0;       // +0x08
		virtual void  Start()                                              = 0;       // +0x0C
		virtual void  Unk10()                                              = 0;       // +0x10
		virtual TBOOL IsReady()                                            = 0;       // +0x14
		virtual void* GetData()                                            = 0;       // +0x18
	};

public:
	// The colour conversions (ARenderer.cpp): RGB (0-255) to HSV (H in degrees, S and V 0-1), TFALSE for a grey (the
	// outputs aren't written); HSV to an ARGB colour (alpha 0xFF) or to its components.
	static TBOOL RGBToHSV( TINT a_iR, TINT a_iG, TINT a_iB, TFLOAT* a_pH, TFLOAT* a_pS, TFLOAT* a_pV );
	static void  HSVToRGB( TFLOAT a_fH, TFLOAT a_fS, TFLOAT a_fV, TUINT32* a_pOutColour );
	static void  HSVToRGB( TFLOAT a_fH, TFLOAT a_fS, TFLOAT a_fV, TUINT32* a_pOutR, TUINT32* a_pOutG, TUINT32* a_pOutB );

	ARenderer();
	virtual ~ARenderer();

	// Creates the order tables and the render interface with its display (TFALSE if the display can't be created)
	TBOOL CreateInterface();

	// Waits for the render thread and frees the resources destroyed meanwhile (before a level loads)
	static void FlushRender();

	// Projects a world position to the screen (pixels of the main viewport) with the camera of a_pViewport (TNULL:
	// the current one), (-100, -100) behind the near plane
	void ProjectToScreen( Toshi::TVector4& a_rPosition, Toshi::TViewport* a_pViewport );

	// Projects a world position onto the main viewport (pixels) with the current camera's angles; TTRUE if it's on
	// screen
	TBOOL ConvertWorldToScreen( const Toshi::TVector4& a_rvWorld, Toshi::TVector2& a_rvScreen );

	// Like ConvertWorldToScreen, clipped to the screen minus the margins: 0 on screen, 1-8 the side or corner it's
	// clipped to (1 left, 2 right, 3 top, 4 bottom, 5/6 top left/right, 7/8 bottom left/right), 9 at the camera
	TINT ConvertWorldToScreenClipped( const Toshi::TVector4& a_rvWorld, Toshi::TVector2& a_rvScreen, TFLOAT a_fLeft, TFLOAT a_fRight, TFLOAT a_fTop, TFLOAT a_fBottom );

	// Converts screen pixels to XUI canvas coordinates
	void ScreenToCanvas( TFLOAT& a_rX, TFLOAT& a_rY );

	// Projects a world position to XUI canvas coordinates (the HUD's projection)
	void ProjectToCanvas( Toshi::TVector4& a_rPosition, Toshi::TViewport* a_pViewport );

	// Projects a world position to T2GUI coordinates (the root element's, centred)
	void ProjectToGUI( Toshi::TVector4& a_rPosition, Toshi::TViewport* a_pViewport );

	// $deBlob: FUNCTION 005ca6f0
	void SetLoadingScreen( TBOOL a_bLoadingScreen ) { m_bIsLoadingScreen = a_bLoadingScreen; }
	void SetSkipFrame( TBOOL a_bSkipFrame ) { m_bSkipFrame = a_bSkipFrame; }

	// Creates the scene, the boot assets, the shaders, the viewports and the fade rectangle. Returns TTRUE
	TBOOL Create();

	// Destroys what Create made
	void Destroy();

	void              UpdateFixed( TFLOAT a_fUnused );
	Toshi::TViewport* GetMainViewport() const;
	void              SetUnk34( TBOOL a_bUnk34 );
	TFLOAT            GetUnk34Height() const;
	void              GetMainViewportSize( Toshi::TVector2& a_rOutSize ) const;
	void*             GetCaptureData() const;
	void              ReleaseCapture();
	void              ThrowFrameEvent();

	// Renders a frame
	void Update( TFLOAT a_fDeltaTime );

	// The clear colour of the main viewport
	void SetBackgroundColour( TUINT32 a_uiRed, TUINT8 a_uiGreen, TUINT8 a_uiBlue );

	AXYZViewportManager* GetViewportManager() const { return m_pViewportManager; }

	// Projections with the camera object of the viewport m_uiCurrentViewport selects
	// The screen position of a_rvPosition in its x and y (-100, -100 when it's before the near plane)
	void ProjectToScreen( Toshi::TVector3& a_rvPosition );
	// The screen position of a_rvPosition seen from a_pCamera: TRUE if it's between the near and far planes
	TBOOL ProjectToScreen( ACamera* a_pCamera, const Toshi::TVector3& a_rvPosition, Toshi::TVector2& a_rOutScreen );
	// The screen rectangle of a sphere (radius at least 4); NOTE: the height is the width
	void GetScreenRect( const Toshi::TVector3& a_rvPosition, TFLOAT a_fRadius, TFLOAT* a_pOutX, TFLOAT* a_pOutY, TFLOAT* a_pOutWidth, TFLOAT* a_pOutHeight );
	// The ray through a screen position from a camera at a_rvPosition with the orientation a_rcRotation
	void GetScreenRay( ACamera* a_pCamera, const Toshi::TVector2& a_rcScreen, Toshi::TVector3& a_rOutOrigin, Toshi::TVector3& a_rOutDirection );
	void GetScreenRay( const Toshi::TVector3& a_rvPosition, const Toshi::TQuaternion& a_rcRotation, const Toshi::TVector2& a_rcScreen, Toshi::TVector3& a_rOutOrigin, Toshi::TVector3& a_rOutDirection );

	// Picks the depth/stencil format of a_rDisplayParams TRender has a device for (TRUE: 4, 2, 3, 0, 1; FALSE: the
	// current one, then 1, 2, 4, 3)
	static void __stdcall FindDepthStencilFormat( Toshi::TRender::DISPLAYPARAMS& a_rDisplayParams, TBOOL a_bPreferStencil );
	TFLOAT GetFrameTime() const { return m_fFrameTime; }
	void   SetFogEnabled( TBOOL a_bEnabled ) { m_bFogEnabled = a_bEnabled; }

private:
	// Creates TRenderDX11 and its display with the size and mode of the command line or setup.ini (ARenderer_Win.cpp)
	TBOOL CreateTRender();

	// Creates the system vertex and index factories of TRender (VFSYSVNDUV1, VFSKIN, VFWORLD, IFSYS)
	void CreateTRenderResources();

	// Creates the viewport of the full display (the GUI's)
	void CreateMainViewport();

	// Renders XUI, T2GUI and the fades in the main viewport
	void RenderGUI( TBOOL a_bAllowBackgroundClear );

	// Copies the matrix (with the shake), FOV, near/far and projection centre of each used viewport's ACamera (or
	// the current camera) to its TCameraObject
	void GetAppCamera();

	// Sets the background colour of every used viewport of the viewport manager
	void InitialiseViewPort();

	typedef void( __stdcall* t_MainSceneCallback )( TFLOAT a_fDeltaTime, ACamera* a_pCamera );

	// Renders a viewport of the world with a_pCameraObject: fog, ambient and lights of the render context, then
	// a_fnMainScene
	void RenderMainScene( TFLOAT a_fDeltaTime, Toshi::TViewport* a_pViewport, Toshi::TCameraObject* a_pCameraObject, ACamera* a_pCamera, t_MainSceneCallback a_fnMainScene, TBOOL a_bAllowBackgroundClear );

	// Renders the models of the loading screen in the viewport selected by the viewport manager
	void RenderLoadingScreen( TFLOAT a_fDeltaTime );

	// Starts and finishes the capture of the back buffer (m_uiCaptureState is the state, 0: none)
	void UpdateCapture( Toshi::TRender* a_pRender );
	// Requests a capture of a_iWidth x a_iHeight (a capture in progress is dropped); TFALSE in an unknown state
	TBOOL StartCapture( TINT a_iWidth, TINT a_iHeight, TBOOL a_bUnk20, TBOOL a_bWait );
	void  FinishCapture( Toshi::TRender* a_pRender );

	// Fades the instances between the camera and Blob (MainScene): saves their alpha and applies the faded one / puts
	// the saved one back
	static void __stdcall FadeCameraInstances( ACamera* a_pCamera );
	static void __stdcall RestoreCameraInstances( ACamera* a_pCamera );

	// Renders the world (models, terrain, particles...) seen by a_pCamera
	static void __stdcall MainScene( TFLOAT a_fDeltaTime, ACamera* a_pCamera );

private:
	static Toshi::TTRB     s_BootAssetsTRB;
	static Toshi::THPTimer s_FrameTimer;
	static TINT            s_iCurrentCamera;
	static TBOOL           s_bRenderGUIAfterWorld;
	static TBOOL           s_bGlow;

	static TCONSTEXPR_VAR TINT MAX_VIEWPORTS_PER_FRAME = 4;

private:
	TBOOL                  m_bUnk04;               // 0x04
	Toshi::TScene*         m_pScene;               // 0x08
	Toshi::TViewport*      m_pViewport;            // 0x0C
	AXYZViewportManager*   m_pViewportManager;     // 0x10
	TUINT32                m_uiUnk14;              // 0x14
	TFLOAT                 m_fUnk18;               // 0x18
	TUINT32                m_uiCaptureState;       // 0x1C
	TBOOL                  m_bUnk20;               // 0x20
	ScreenCapture*         m_pCapture;             // 0x24 the capture in progress
	void*                  m_pCaptureData;         // 0x28 the captured image (4 bytes per pixel)
	TINT                   m_iCaptureWidth;        // 0x2C
	TINT                   m_iCaptureHeight;       // 0x30
	TBOOL                  m_bUnk34;               // 0x34
	Toshi::TGenericEmitter m_FrameEmitter;         // 0x38 thrown at the start of each frame
	TBOOL                  m_bRenderGUI;           // 0x44
	TBOOL                  m_bUnk45;               // 0x45
	TBOOL                  m_bIsLoadingScreen;     // 0x46
	TUINT32                m_uiBackgroundColour;   // 0x48
	TFLOAT                 m_afAmbientColour[ 4 ]; // 0x4C of the viewports' render contexts
	TBOOL                  m_bCaptureWait;         // 0x5C
	TBOOL                  m_bSkipFrame;           // 0x5D the next Update only clears it
	Toshi::T2GUIRectangle* m_pRectangle;           // 0x60 covering the screen (black, alpha 0)
	TFLOAT                 m_fFrameTime;           // 0x64
	TBOOL                  m_bFogEnabled;          // 0x68
	TFLOAT                 m_afFogColour[ 4 ];     // 0x6C
	TFLOAT                 m_fFogStart;            // 0x7C at least the camera's near
	TFLOAT                 m_fFogEnd;              // 0x80 at most the camera's far
	ADisplayModes_Win      m_DisplayModes;         // 0x84
};
