#include "ToshiPCH.h"
#include "TXUI.h"
#include "TXUICanvas.h"
#include "TXUIRenderer.h"
#include "TXUIAudio.h"
#include "TXUIInput.h"
#include "TXUIScene.h"
#include "TXUIVisual.h"
#include "TXUIShapeCache.h"
#include "TXUIStringTable.h"
#include "TXUITimeline.h"
#include "TXUIControl.h"
#include "TXUIFigure.h"
#include "TXUIImage.h"
#include "TXUIImagePresenter.h"
#include "TXUIText.h"
#include "TXUITextPresenter.h"
#include "TXUINineGrid.h"
#include "TXUIGroup.h"
#include "TXUIBackButton.h"
#include "TXUIButton.h"
#include "TXUICheckBox.h"
#include "TXUICommonList.h"
#include "TXUIEditBox.h"
#include "TXUILabel.h"
#include "TXUIList.h"
#include "TXUIMsgBoxButton.h"
#include "TXUINavButton.h"
#include "TXUIProgressBar.h"
#include "TXUIRadioButton.h"
#include "TXUIRadioGroup.h"
#include "TXUIScrollEnd.h"
#include "TXUISliderBar.h"
#include "TXUISound.h"
#include "TXUITabScene.h"
#include "TXUIVideo.h"
#include "TXUIMessageBox.h"

#include "GUI/T2GUI.h"
#include "GUI/T2GUIFontManager.h"
#include "Render/TRender.h"
#include "Render/TAssetInit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// Sizes of the core classes
TSTATICASSERT( sizeof( T2GUITransform ) == 0x10 );
TSTATICASSERT( sizeof( T2GUIElement ) == 0x3C );
TSTATICASSERT( sizeof( T2GUIContext ) == 0x3C );
TSTATICASSERT( sizeof( T2GUI ) == 0xC );
TSTATICASSERT( sizeof( TXUI ) == 0x188 );
TSTATICASSERT( sizeof( TXUIResourceTRB ) == 0x18 );
TSTATICASSERT( sizeof( TXUIResource ) == 0xD4 );
TSTATICASSERT( sizeof( TXUIStringTable ) == 0x14 );
TSTATICASSERT( sizeof( TXUIRenderer ) == 0x30 );
TSTATICASSERT( sizeof( TXUIInput ) == 0x28 );
TSTATICASSERT( sizeof( XURXUIObjectData ) == 0x1C );
TSTATICASSERT( sizeof( XURXUIElementData ) == 0x30 );
TSTATICASSERT( sizeof( XURXUIControlData ) == 0x44 );
TSTATICASSERT( sizeof( XURXUISceneData ) == 0x54 );
TSTATICASSERT( sizeof( XURXUICanvasData ) == 0x30 );
TSTATICASSERT( sizeof( XURXUINamedFrameData ) == 0x8 );
TSTATICASSERT( sizeof( XURXUIKeyframeData ) == 0xC );
TSTATICASSERT( sizeof( XURXUITimelineData ) == 0x1C );
TSTATICASSERT( sizeof( TXUITimeline ) == 0x3C );
TSTATICASSERT( sizeof( TXUITimelineControl ) == 0x50 );
TSTATICASSERT( sizeof( TXUIElement ) == 0x64 );
TSTATICASSERT( sizeof( TXUIControl ) == 0x94 );
TSTATICASSERT( sizeof( TXUIScene ) == 0x130 );
TSTATICASSERT( sizeof( TXUICanvas ) == 0xB4 );

TMemoryHeap* TXUI::ms_pXUIMemoryBlock    = TNULL;
TMemoryHeap* TXUI::ms_pXUITRBMemoryBlock = TNULL;

//-----------------------------------------------------------------------------
// TXUIResourceTRB
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006bc880
void TXUIResourceTRB::Init()
{
	if ( m_pResource == TNULL )
	{
		m_pResource = new ( TXUI::ms_pXUIMemoryBlock ) TXUIResource();

		// Every .xur gets its own pile, sized by a first pass over the data
		TUINT32      uiPileSize = m_pResource->CalculatePileSize( m_pData );
		TMemoryHeap* pOldBlock  = TXUI::ms_pXUIMemoryBlock;

		TXUI::ms_pXUIMemoryBlock = TMemory_CreateSubHeap( TXUI::ms_pXUIMemoryBlock, uiPileSize, TMEMORYHEAP_ALLOCASPILE, "xui pile" );
		m_pResource->SetMemoryHeap( TXUI::ms_pXUIMemoryBlock );

		TBOOL bRes = m_pResource->Load( m_pData );
		TASSERT( TTRUE == bRes );

		TXUI::ms_pXUIMemoryBlock = pOldBlock;
		TXUI::GetSingleton()->AddResource( this );
	}
}

// $deBlob: FUNCTION 006bc950
void TXUIResourceTRB::Deinit()
{
	if ( m_pResource )
		delete m_pResource;

	TXUI::GetSingleton()->RemoveResource( this );
}

//-----------------------------------------------------------------------------
// TXUI
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 006bc9b0
TXUI::TXUI()
{
	m_pSkin1Resource     = TNULL;
	m_pSkin2Resource     = TNULL;
	m_pDefaultFont       = TNULL;
	m_sDefaultFont       = TNULL;
	m_pCanvas            = TNULL;
	m_pActiveScene       = TNULL;
	m_pTransToScene      = TNULL;
	m_pTransFromScene    = TNULL;
	m_pContext           = TNULL;
	m_pRenderer          = TNULL;
	m_pHeadTRBResource   = TNULL;
	m_pInput             = TNULL;
	m_iDefaultController = 0;
	m_pXUIAudio          = TNULL;
	m_cbCreateMovie      = TNULL;
	m_bPopup             = TFALSE;
	m_bInputLocked       = TFALSE;
	m_pCapturing        = TNULL;

	m_oFontTRB.SetMemoryFunctions( AssetTRBAllocator, AssetTRBDeallocator, TNULL );
	m_oSkin1TRB.SetMemoryFunctions( AssetTRBAllocator, AssetTRBDeallocator, TNULL );
	m_oSkin2TRB.SetMemoryFunctions( AssetTRBAllocator, AssetTRBDeallocator, TNULL );

	m_pContext = new ( ms_pXUIMemoryBlock ) T2GUIContext();

	TRender::DISPLAYPARAMS* pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();
	m_pContext->GetRootElement()->SetDimensions( TFLOAT( TINT( pDisplayParams->uiWidth ) ), TFLOAT( TINT( pDisplayParams->uiHeight ) ) );

	m_pCanvas = new ( ms_pXUIMemoryBlock ) TXUICanvas();
	m_pCanvas->SetDimensions( TFLOAT( TINT( pDisplayParams->uiWidth ) ), TFLOAT( TINT( pDisplayParams->uiHeight ) ) );
	m_pContext->GetRootElement()->AddChildTail( m_pCanvas );

	m_pRenderer = new ( ms_pXUIMemoryBlock ) TXUIRenderer();
	m_pInput    = new ( ms_pXUIMemoryBlock ) TXUIInput();

	T2String8::Format( m_szXUIBasePath, sizeof( m_szXUIBasePath ), "" );

	// NOTE: TXUIShapeCache is a singleton, the pointer isn't kept
	new ( ms_pXUIMemoryBlock ) TXUIShapeCache();
}

// $deBlob: FUNCTION 006bcc50
// $deBlob: FUNCTION 0056c370
TXUI::~TXUI()
{
	m_pDefaultFont = TNULL;
	m_sDefaultFont = TNULL;

	FlushSceneCache();

	if ( m_pInput )
		delete m_pInput;

	if ( m_pXUIAudio )
		delete m_pXUIAudio;

	if ( m_pRenderer )
		delete m_pRenderer;

	if ( m_pCanvas )
		delete m_pCanvas;

	if ( m_pContext )
		delete m_pContext;

	if ( TXUIShapeCache* pShapeCache = TXUIShapeCache::GetSingleton() )
		delete pShapeCache;

	if ( m_pSkin1Resource )
	{
		TAssetInit::DeinitAssets( m_oSkin1TRB );
		m_oSkin1TRB.Close();
	}

	if ( m_pSkin2Resource )
	{
		TAssetInit::DeinitAssets( m_oSkin2TRB );
		m_oSkin2TRB.Close();
	}

	TAssetInit::DeinitAssets( m_oFontTRB );
	m_oFontTRB.Close();
}

// $deBlob: FUNCTION 006bcda0
void TXUI::SetXUIBasePath( const TCHAR* a_szBasePath )
{
	TASSERT( T2String8::Length( a_szBasePath ) < 128 );
	T2String8::Copy( m_szXUIBasePath, a_szBasePath, -1 );
}

// $deBlob: FUNCTION 006bce00
TXUIResource* TXUI::FindResource( const TCHAR* a_szName )
{
	TASSERT( T2String8::IsLowerCase( a_szName ) );

	for ( TXUIResourceTRB* pResourceTRB = m_pHeadTRBResource; pResourceTRB != TNULL; pResourceTRB = pResourceTRB->m_pNext )
	{
		if ( TStringManager::String8CompareNoCase( pResourceTRB->m_szFileName, a_szName, -1 ) == 0 )
			return pResourceTRB->m_pResource;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006bce90
TXUIResource* TXUI::LoadXUR( TBOOL a_bLoadStringTables, const TCHAR* a_szFileName, TBOOL a_bTRB, TTRB* a_pTRB )
{
	if ( a_szFileName == TNULL )
		return TNULL;

	TXUIResource* pResource = new ( ms_pXUIMemoryBlock ) TXUIResource();

	TCHAR        szStringTablePath[ 256 ];
	TCHAR        szFilePath[ 256 ];
	const TCHAR* pStringTablePath = TNULL;

	if ( a_bTRB )
	{
		if ( a_bLoadStringTables )
		{
			T2String8::Format( szStringTablePath, "%s/StringTables/%s/%s.trb", m_szXUIBasePath, GetXUILocaleDir(), a_szFileName );
			pStringTablePath = szStringTablePath;
		}

		T2String8::Format( szFilePath, "Data/XUI/%s.trb", a_szFileName );
	}
	else
	{
		if ( a_bLoadStringTables )
		{
			T2String8::Format( szStringTablePath, "%s/StringTables/%s/%s.xus", m_szXUIBasePath, GetXUILocaleDir(), a_szFileName );
			pStringTablePath = szStringTablePath;
		}

		T2String8::Format( szFilePath, "Data/XUI/%s.xur", a_szFileName );
	}

	if ( !pResource->Load( a_bLoadStringTables, szFilePath, pStringTablePath, a_bTRB, a_pTRB ) )
	{
		if ( pResource )
			delete pResource;

		return TNULL;
	}

	return pResource;
}

// $deBlob: FUNCTION 006bd000
void TXUI::SetSkin1( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName )
{
	if ( m_pSkin1Resource )
	{
		TAssetInit::DeinitAssets( m_oSkin1TRB );
		m_oSkin1TRB.Close();
	}

	TTRB::ERROR error = m_oSkin1TRB.Load( a_szTRBFileName );
	TASSERT( error == TTRB::ERROR_OK );

	TAssetInit::InitAssets( m_oSkin1TRB, TTRUE, TFALSE );
	m_pSkin1Resource = FindResource( a_szXURName );
}

// $deBlob: FUNCTION 006bd090
void TXUI::SetSkin2( const TCHAR* a_szTRBFileName, const TCHAR* a_szXURName )
{
	if ( m_pSkin2Resource )
	{
		TAssetInit::DeinitAssets( m_oSkin2TRB );
		m_oSkin2TRB.Close();
	}

	TTRB::ERROR error = m_oSkin2TRB.Load( a_szTRBFileName );
	TASSERT( error == TTRB::ERROR_OK );

	TAssetInit::InitAssets( m_oSkin2TRB, TTRUE, TFALSE );
	m_pSkin2Resource = FindResource( a_szXURName );
}

// $deBlob: FUNCTION 006bd120
void TXUI::UnloadSkin2()
{
	if ( m_pSkin2Resource )
	{
		TAssetInit::DeinitAssets( m_oSkin2TRB );
		m_oSkin2TRB.Close();
		m_pSkin2Resource = TNULL;
	}
}

// $deBlob: FUNCTION 006bd150
void TXUI::SetDefaultFont( const TCHAR* a_szTRBFileName, const TCHAR* a_szFontName )
{
	TASSERT( TNULL == m_pDefaultFont && TNULL == m_sDefaultFont );
	m_sDefaultFont = a_szFontName;

	TString8 strFontName( a_szFontName );
	TINT     iIndex = strFontName.FindReverse( '_', -1 );
	TASSERT( iIndex >= 0 );

	TStringManager::String8Copy( m_szFontName, m_sDefaultFont, iIndex );
	m_szFontName[ iIndex ] = '\0';

	TTRB::ERROR error = m_oFontTRB.Load( a_szTRBFileName );
	TASSERT( error == TTRB::ERROR_OK );

	TAssetInit::InitAssets( m_oFontTRB, TTRUE, TFALSE );
}

// $deBlob: FUNCTION 006bd470
T2GUIFont* TXUI::GetDefaultFont()
{
	return T2GUIFontManager::GetFont( m_sDefaultFont );
}

// $deBlob: FUNCTION 006bd270
const TCHAR* TXUI::GetXUILocaleDir()
{
	return "";
}

// $deBlob: FUNCTION 006bd280
TXUIScene* TXUI::CreateScene( TXUIResource* a_pResource, const TWCHAR* a_wszName, TBOOL a_bCache )
{
	TXUIScene* pScene = FindCachedScene( a_wszName );

	if ( pScene == TNULL )
	{
		pScene = a_pResource->CreateScene( a_wszName );

		if ( pScene == TNULL )
			return TNULL;
	}

	m_pCanvas->AddScene( pScene );

	if ( a_bCache == TTRUE )
		pScene->SetCached();

	return pScene;
}

// $deBlob: FUNCTION 006bd2d0
TXUIScene* TXUI::CreateScene( TXUIResource* a_pResource, TUINT32 a_uiIndex )
{
	TXUIScene* pScene = a_pResource->CreateScene( a_uiIndex );

	if ( pScene == TNULL )
		return TNULL;

	m_pCanvas->AddScene( pScene );
	return pScene;
}

// $deBlob: FUNCTION 006bd310
TXUIVisual* TXUI::CreateVisual( const TWCHAR* a_wszName, TBOOL a_bSkin2 )
{
	TXUIResource* pSkin = a_bSkin2 ? m_pSkin2Resource : m_pSkin1Resource;

	if ( pSkin != TNULL )
		return pSkin->CreateVisual( a_wszName );

	return TNULL;
}

// $deBlob: FUNCTION 006bd340
void TXUI::Tick( TFLOAT a_fDeltaTime )
{
	if ( T2GUI::GetSingleton() != TNULL )
	{
		T2GUIRenderer* pOldRenderer = T2GUI::GetSingleton()->GetRenderer();
		T2GUI::GetSingleton()->SetRenderer( m_pRenderer );
		m_pContext->Tick( a_fDeltaTime );
		T2GUI::GetSingleton()->SetRenderer( pOldRenderer );
	}
}

// $deBlob: FUNCTION 006bd3a0
TBOOL TXUI::TickInput( TFLOAT a_fDeltaTime )
{
	if ( m_pTransToScene == TNULL && m_pTransFromScene == TNULL && m_bInputLocked == TFALSE )
		return m_pInput->Tick( a_fDeltaTime );

	return TFALSE;
}

// $deBlob: FUNCTION 006bd3e0
void TXUI::Render()
{
	if ( T2GUI::GetSingleton() != TNULL )
	{
		T2GUIRenderer* pOldRenderer = T2GUI::GetSingleton()->GetRenderer();
		T2GUI::GetSingleton()->SetRenderer( m_pRenderer );
		m_pRenderer->BeginScene();
		m_pContext->Render();
		m_pRenderer->EndScene();
		T2GUI::GetSingleton()->SetRenderer( pOldRenderer );
	}
}

// $deBlob: FUNCTION 006bd440
void TXUI::SetCanvasDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_pCanvas->SetDimensions( a_fWidth, a_fHeight );
}

// $deBlob: FUNCTION 006bd480
void TXUI::SetFocussedScene( TXUIScene* a_pScene )
{
	m_pActiveScene = a_pScene;
	m_pInput->ResetRepeat();
}

// $deBlob: FUNCTION 006bd4a0
void TXUI::EndTransTo()
{
	m_pTransToScene = TNULL;
}

// $deBlob: FUNCTION 006bd4b0
void TXUI::EndTransBackTo()
{
	m_pTransToScene = TNULL;
}

// $deBlob: FUNCTION 006bd4c0
void TXUI::EndTransFrom()
{
	m_pTransFromScene = TNULL;
}

// $deBlob: FUNCTION 006bd4d0
void TXUI::EndTransBackFrom()
{
	m_pTransFromScene = TNULL;
}

// $deBlob: FUNCTION 006bd4e0
void TXUI::TransitionTo( TXUIScene* a_pFrom, TXUIScene* a_pTo )
{
	if ( a_pFrom != TNULL || ( a_pFrom = m_pActiveScene ) != TNULL )
		a_pFrom->TransFrom();

	m_pTransToScene = a_pTo;
	a_pTo->SetPrevScene( a_pFrom );
	m_pTransToScene->TransTo();

	m_pInput->ResetRepeat();
	m_pCapturing = TNULL;
}

// $deBlob: FUNCTION 006bd5c0
void TXUI::TransitionToPopup( TXUIScene* a_pFrom, TXUIScene* a_pTo )
{
	m_bPopup = TTRUE;

	if ( a_pFrom != TNULL || ( a_pFrom = m_pActiveScene ) != TNULL )
	{
		a_pFrom->SetFocus( TFALSE, TFALSE, TTRUE );
		a_pFrom->EndTransFrom();
	}

	m_pTransToScene = a_pTo;
	a_pTo->TransTo();
	m_pTransToScene->SetPrevScene( a_pFrom );

	m_pInput->ResetRepeat();
	m_pCapturing = TNULL;
}

// $deBlob: FUNCTION 006bd540
void TXUI::TransitionBack( TXUIScene* a_pScene )
{
	if ( m_bPopup == TFALSE )
	{
		TXUIScene* pPrevScene = a_pScene->GetPrevScene();
		m_pTransFromScene     = a_pScene;

		if ( pPrevScene == TNULL )
		{
			a_pScene->TransBackFrom();
		}
		else
		{
			a_pScene->TransBackFrom();
			pPrevScene = a_pScene->GetPrevScene();
			pPrevScene->TransBackTo();
			m_pTransToScene = pPrevScene;
		}
	}
	else
	{
		RemovePopup( a_pScene );
	}

	m_pInput->ResetRepeat();
	m_pCapturing = TNULL;
}

// $deBlob: FUNCTION 006bd640
void TXUI::RemovePopup( TXUIScene* a_pScene )
{
	TASSERT( TTRUE == m_bPopup );
	m_bPopup = TFALSE;

	if ( a_pScene->GetPrevScene() != TNULL )
	{
		m_pTransFromScene = a_pScene;
		a_pScene->TransBackFrom();

		TXUIScene* pPrevScene = a_pScene->GetPrevScene();
		m_pTransToScene       = pPrevScene;
		pPrevScene->EndTransBackTo();
		pPrevScene->SetFocus( TTRUE, TFALSE, TFALSE );
		return;
	}

	delete a_pScene;
}

// Returns the focused control of the scene for the controller, going down the focused scenes
static TXUIControl* TXUI_GetDeepestFocus( TXUIScene* a_pScene, TINT a_iController )
{
	TXUIControl* pFocus = a_pScene->GetFocus( a_iController );

	while ( pFocus != TNULL )
	{
		if ( !pFocus->IsA( &TGetClass( TXUIScene ) ) || TSTATICCAST( TXUIScene, pFocus )->GetFocus( a_iController ) == TNULL )
			break;

		pFocus = TSTATICCAST( TXUIScene, pFocus )->GetFocus( a_iController );
	}

	return pFocus;
}

// Returns TTRUE if the control takes input from the controller
static TBOOL TXUI_IsInputTarget( TXUIControl* a_pControl, TINT a_iController, TINT a_iDefaultController )
{
	if ( a_pControl->IsInputIgnored() || !a_pControl->IsVisible() || !a_pControl->IsEnabled() )
		return TFALSE;

	if ( a_iController == a_iDefaultController )
		return TTRUE;

	TXUIScene* pScene = a_pControl->FindParentScene();
	return pScene != TNULL && pScene->AcceptsAnyController();
}

// $deBlob: FUNCTION 006bd6e0
TBOOL TXUI::HandleButton( TINT a_eButton, TINT a_iController )
{
	if ( m_pTransToScene != TNULL || m_pTransFromScene != TNULL || m_bInputLocked != TFALSE )
		return TFALSE;

	TBOOL bHandled = TFALSE;

	if ( m_pCapturing != TNULL )
		return m_pCapturing->HandleButton( a_eButton, a_iController );

	if ( m_pActiveScene != TNULL && m_pActiveScene->IsEnabled() == TTRUE && m_pActiveScene->IsVisible() )
	{
		if ( m_pActiveScene->GetFocus( a_iController ) == TNULL )
			return m_pActiveScene->HandleButton( a_eButton, a_iController );

		TXUIControl* pControl           = TXUI_GetDeepestFocus( m_pActiveScene, a_iController );
		TINT         iDefaultController = GetSingleton()->m_iDefaultController;

		while ( pControl != TNULL )
		{
			if ( TXUI_IsInputTarget( pControl, a_iController, iDefaultController ) )
			{
				bHandled = pControl->HandleButton( a_eButton, a_iController );

				if ( bHandled )
				{
					if ( a_eButton == TXUIElement::XUIButton_Accept && pControl == GetSingleton()->m_pInput->GetRepeatElement() )
						GetSingleton()->m_pInput->PlayClickAnimation( a_iController );

					break;
				}
			}

			pControl = TSTATICCAST( TXUIControl, pControl->FindParentControl() );
		}
	}

	return bHandled;
}

// $deBlob: FUNCTION 006bd890
TBOOL TXUI::HandleButtonRelease( TINT a_eButton, TINT a_iController )
{
	if ( m_pTransToScene != TNULL || m_pTransFromScene != TNULL || m_bInputLocked != TFALSE )
		return TFALSE;

	if ( m_pCapturing != TNULL )
		return m_pCapturing->HandleButtonRelease( a_eButton, a_iController );

	TBOOL bHandled = TFALSE;

	if ( m_pActiveScene != TNULL && m_pActiveScene->IsEnabled() == TTRUE && m_pActiveScene->IsVisible() )
	{
		TXUIControl* pControl           = TXUI_GetDeepestFocus( m_pActiveScene, a_iController );
		TINT         iDefaultController = GetSingleton()->m_iDefaultController;

		while ( pControl != TNULL )
		{
			if ( TXUI_IsInputTarget( pControl, a_iController, iDefaultController ) )
			{
				bHandled = pControl->HandleButtonRelease( a_eButton, a_iController );
				if ( bHandled ) return bHandled;
			}

			pControl = TSTATICCAST( TXUIControl, pControl->FindParentControl() );
		}
	}

	return bHandled;
}

// $deBlob: FUNCTION 006bd9e0
TBOOL TXUI::HandleAnalog( TINT a_iParam1, TINT a_iParam2, TINT a_iController )
{
	if ( m_pTransToScene != TNULL || m_pTransFromScene != TNULL || m_bInputLocked != TFALSE )
		return TFALSE;

	if ( m_pCapturing != TNULL )
		return m_pCapturing->HandleAnalog( a_iParam1, a_iParam2, a_iController );

	TBOOL bHandled = TFALSE;

	if ( m_pActiveScene != TNULL && m_pActiveScene->IsEnabled() == TTRUE && m_pActiveScene->IsVisible() )
	{
		TXUIControl* pControl           = TXUI_GetDeepestFocus( m_pActiveScene, a_iController );
		TINT         iDefaultController = GetSingleton()->m_iDefaultController;

		while ( pControl != TNULL )
		{
			if ( TXUI_IsInputTarget( pControl, a_iController, iDefaultController ) )
			{
				bHandled = pControl->HandleAnalog( a_iParam1, a_iParam2, a_iController );
				if ( bHandled ) return bHandled;
			}

			pControl = TSTATICCAST( TXUIControl, pControl->FindParentControl() );
		}
	}

	return bHandled;
}

// $deBlob: FUNCTION 006bdb30
void TXUI::Reset()
{
	FlushSceneCache();

	T2GUIElement* pChild = m_pCanvas->ChildBegin();

	while ( pChild != TNULL )
	{
		pChild->RemoveSelf();
		delete pChild;

		pChild = m_pCanvas->ChildBegin();
	}

	m_pActiveScene    = TNULL;
	m_pTransToScene   = TNULL;
	m_pTransFromScene = TNULL;
	m_bPopup          = TFALSE;
	m_bInputLocked    = TFALSE;
	m_pInput->ResetRepeat();
	m_pCapturing = TNULL;
}

// $deBlob: FUNCTION 006bdbe0
TXUIMovie* TXUI::CreateMovie()
{
	if ( m_cbCreateMovie != TNULL )
		return m_cbCreateMovie();

	return TNULL;
}

// $deBlob: FUNCTION 006bdbf0
void TXUI::SetDefaultController( TINT a_iController )
{
	m_pInput->SetEnabled( TFALSE );
	m_iDefaultController = a_iController;
	m_pInput->SetEnabled( TTRUE );
}

// $deBlob: FUNCTION 006bdc20
void TXUI::FlushSceneCache()
{
	while ( !m_oSceneCache.IsEmpty() )
	{
		SceneNode* pNode = m_oSceneCache.Back();
		pNode->Remove();

		if ( pNode->m_pScene )
			delete pNode->m_pScene;

		delete pNode;
	}
}

// $deBlob: FUNCTION 006bdcd0
TXUIScene* TXUI::FindCachedScene( const TWCHAR* a_wszName )
{
	for ( auto it = m_oSceneCache.Begin(); it != m_oSceneCache.End(); it++ )
	{
		if ( TStringManager::String16Compare( it->m_pScene->GetID(), a_wszName, -1 ) == 0 )
			return it->m_pScene;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006bdd50
void TXUI::AddResource( TXUIResourceTRB* a_pResourceTRB )
{
	a_pResourceTRB->m_pNext = m_pHeadTRBResource;
	a_pResourceTRB->m_pPrev = TNULL;

	if ( m_pHeadTRBResource )
		m_pHeadTRBResource->m_pPrev = a_pResourceTRB;

	m_pHeadTRBResource = a_pResourceTRB;
}

// $deBlob: FUNCTION 006bdd80
void TXUI::RemoveResource( TXUIResourceTRB* a_pResourceTRB )
{
	if ( a_pResourceTRB->m_pNext )
		a_pResourceTRB->m_pNext->m_pPrev = a_pResourceTRB->m_pPrev;

	if ( a_pResourceTRB->m_pPrev )
		a_pResourceTRB->m_pPrev->m_pNext = a_pResourceTRB->m_pNext;

	if ( a_pResourceTRB == m_pHeadTRBResource )
		m_pHeadTRBResource = m_pHeadTRBResource->m_pNext;
}

// $deBlob: FUNCTION 006bddc0
void* __stdcall TXUI::AssetTRBAllocator( TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
	return TMemalign( a_iAlignment, a_uiSize, ms_pXUITRBMemoryBlock );
}

// $deBlob: FUNCTION 006bdde0
void __stdcall TXUI::AssetTRBDeallocator( TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iAlignment, TUINT32 a_uiUnk, void* a_pUserData )
{
	TMemoryHeap_Free( ms_pXUITRBMemoryBlock, a_pMem );
}

// Takes the TClass by value: the copy is made on the stack and destroyed (~TClass) on return, like retail
static void TXUI_RefClass( TClass a_oClass )
{
}

// NOTE: makes sure the linker keeps every XUI class: copies each TClass on the stack and destroys the copy.
// Nothing calls it.
// $deBlob: FUNCTION 006bde00
void TXUI::RefXUIClasses()
{
	TXUI_RefClass( TGetClass( TXUICanvas ) );
	TXUI_RefClass( TGetClass( TXUIFigure ) );
	TXUI_RefClass( TGetClass( TXUIImage ) );
	TXUI_RefClass( TGetClass( TXUIImagePresenter ) );
	TXUI_RefClass( TGetClass( TXUIText ) );
	TXUI_RefClass( TGetClass( TXUITextPresenter ) );
	TXUI_RefClass( TGetClass( TXUINineGrid ) );
	TXUI_RefClass( TGetClass( TXUIGroup ) );
	TXUI_RefClass( TGetClass( TXUIVisual ) );
	TXUI_RefClass( TGetClass( TXUIBackButton ) );
	TXUI_RefClass( TGetClass( TXUIButton ) );
	TXUI_RefClass( TGetClass( TXUICheckBox ) );
	TXUI_RefClass( TGetClass( TXUICommonList ) );
	TXUI_RefClass( TGetClass( TXUIControl ) );
	TXUI_RefClass( TGetClass( TXUIEditBox ) );
	TXUI_RefClass( TGetClass( TXUILabel ) );
	TXUI_RefClass( TGetClass( TXUIList ) );
	TXUI_RefClass( TGetClass( TXUIMsgBoxButton ) );
	TXUI_RefClass( TGetClass( TXUINavButton ) );
	TXUI_RefClass( TGetClass( TXUIProgressBar ) );
	TXUI_RefClass( TGetClass( TXUIRadioButton ) );
	TXUI_RefClass( TGetClass( TXUIRadioGroup ) );
	TXUI_RefClass( TGetClass( TXUIScene ) );
	TXUI_RefClass( TGetClass( TXUIScrollEnd ) );
	TXUI_RefClass( TGetClass( TXUISliderBar ) );
	TXUI_RefClass( TGetClass( TXUISound ) );
	TXUI_RefClass( TGetClass( TXUITabScene ) );
	TXUI_RefClass( TGetClass( TXUIVideo ) );
	TXUI_RefClass( TGetClass( TXUIMessageBox ) );
}

TOSHI_NAMESPACE_END
