#include "pch.h"
#include "ABlobCursorTrail.h"
#include "GUI/AGUICursor.h"
#include "GUI/AGUIManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/ALoadLevelState.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"
#include "ARandom.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIFigure.h>
#include <XUI/TXUIRenderer.h>
#include <GUI/T2GUI.h>
#include <GUI/T2GUIMaterial.h>
#include <GUI/T2GUITransform.h>
#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Platform/DX11/TTexture_DX11.h>
#include <Math/TMath.h>
#include <Math/TMathInline.h>

#include <fmod_event.hpp>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABlobCursorTrail ) == 0x1A48 );

// The colours of the trail
static const TUINT32 s_auiColours[ ABlobCursorTrail::NUM_COLOURS ] = {
	0xfff1d507,
	0xff0e697b,
	0xffae280d,
	0xffa582bc,
	0xfff38701,
	0xff668527,
	0xff706741,
};

// The loading screens of the SPLevels
static const TCHAR* s_aLoadingScreenXURs[ AGameStatus::SPLevel_NUMOF ] = {
	"loadingscreen.xur",
	"loadingscreen_uptown.xur",
	"loadingscreen_downtown.xur",
	"loadingscreen_chromadam.xur",
	"loadingscreen_guggentraz.xur",
	"loadingscreen_thedocklands.xur",
	"loadingscreen_hanging_gardens.xur",
	"loadingscreen_chromacity.xur",
	"loadingscreen_ministryofink.xur",
	"loadingscreen_lakeraydia.xur",
};

// The drips and splats are removed from the ring when their point's life gets lower than this
static TCONSTEXPR_VAR TFLOAT FADE_LIFE = 1.0f;

ABlobCursorTrail* ABlobCursorTrail::sm_apCursorTrails[ MAX_CURSOR_ARRAY ];

// $deBlob: FUNCTION 00626f00
// $deBlob: FUNCTION 00626f10
// $deBlob: FUNCTION 00626f80
// $deBlob: FUNCTION 00626ff0
// $deBlob: FUNCTION 00627000
// $deBlob: FUNCTION 0053d090
// $deBlob: FUNCTION 008587a0
TDEFINE_CLASS( ABlobCursorTrail );

// $deBlob: FUNCTION 00627010
ABlobCursorTrail::ABlobCursorTrail()
{
	m_vPrevPos   = TVector2::VEC_ZERO;
	m_vLastPoint = TVector2::VEC_ZERO;

	m_iHead         = 0;
	m_iNumPoints    = 0;
	m_pVertices     = TNULL;
	m_pUVs          = TNULL;
	m_pColours      = TNULL;
	m_iNextDrip     = 0;
	m_iNextSplat    = 0;
	m_bReset        = TTRUE;
	m_bErase        = TFALSE;
	m_iIndex        = 0;
	m_iColour       = 0;
	m_fSplatTimer   = 0.0f;
	m_fFadeSpeed    = 1.0f;
	m_iNumVisible   = 0;
	m_bSoundStarted = TFALSE;
	m_fSoundParam   = 0.0f;
	m_iSpeedHead    = 0;
	m_pEvent        = TNULL;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves the points uninitialised
	memset( m_aPoints, 0, sizeof( m_aPoints ) );
#endif

	m_iMaxVertices = MAX_VERTICES;
	m_pVertices    = TSTATICCAST( TVector2, TMemoryHeap_Malloc( TXUI::ms_pXUIMemoryBlock, 4096 ) );
	m_pUVs         = TSTATICCAST( TVector2, TMemoryHeap_Malloc( TXUI::ms_pXUIMemoryBlock, m_iMaxVertices * sizeof( TVector2 ) ) );
	m_pColours     = TSTATICCAST( TUINT32, TMemoryHeap_Malloc( TXUI::ms_pXUIMemoryBlock, m_iMaxVertices * sizeof( TUINT32 ) ) );

	// NOTE: retail's MemSet is memset: the low byte of the colour fills the first 512 bytes
	memset( m_pColours, s_auiColours[ m_iColour ], m_iMaxVertices );

	TTexture* pTexture = TTextureManager::GetSingleton()->FindTexture( "cursortrail.tga" );
	m_pMaterial        = T2GUI::CreateMaterial( pTexture );

	// T2GUIMaterial's address modes
	// $deBlob: FUNCTION 00629eb0
	m_pMaterial->SetTextureAddressV( T2GUIMaterial::TEXTUREADDRESS_WRAP );
	m_pMaterial->SetTextureAddress( T2GUIMaterial::TEXTUREADDRESS_WRAP );

	m_aPoints[ 0 ].uiColour = s_auiColours[ m_iColour ];

	m_vSoundPos     = TVector2::VEC_ZERO;
	m_vSoundPrevPos = TVector2::VEC_ZERO;

	TINT iSlot = -1;

	for ( TINT i = 0; i < MAX_CURSOR_ARRAY; i++ )
	{
		if ( sm_apCursorTrails[ i ] == this )
		{
			iSlot = -2;
		}
		else if ( sm_apCursorTrails[ i ] == TNULL && iSlot == -1 )
		{
			iSlot = i;
		}
	}

	TASSERT( iSlot != -1 && "ERR: Slot for cursor trail not found. Increase MAX_CURSOR_ARRAY value!\n" );

	if ( iSlot >= 0 )
		sm_apCursorTrails[ iSlot ] = this;
}

// $deBlob: FUNCTION 00629db0
// $deBlob: FUNCTION 00627370
// $deBlob: FUNCTION 00629da4
ABlobCursorTrail::~ABlobCursorTrail()
{
	if ( m_pEvent != TNULL )
	{
		m_pEvent->stop( false );
		m_pEvent = TNULL;
	}

	TFree( m_pVertices );
	TFree( m_pUVs );
	TFree( m_pColours );

	if ( m_pMaterial != TNULL )
		T2GUI::DestroyMaterial( m_pMaterial );

	for ( TINT i = 0; i < MAX_CURSOR_ARRAY; i++ )
	{
		if ( sm_apCursorTrails[ i ] == this )
		{
			sm_apCursorTrails[ i ] = TNULL;
			break;
		}
	}
}

// $deBlob: FUNCTION 00627450
void ABlobCursorTrail::Tick( TFLOAT a_fDeltaTime )
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() || !IsVisible() )
	{
		if ( m_pEvent != TNULL )
		{
			m_pEvent->stop( false );
			m_pEvent = TNULL;
		}

		TXUIScene::Tick( a_fDeltaTime );
		return;
	}

	AGUICursor*           pCursor   = AGUIManager::GetSingleton()->GetCursor();
	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	// X changes the colour
	if ( !pKeyboard->WasDown( TInputDeviceKeyboard::KEY_X ) && pKeyboard->IsDown( TInputDeviceKeyboard::KEY_X ) && !pCursor->IsMPColour() )
	{
		if ( ++m_iColour >= NUM_COLOURS )
			m_iColour = 0;
	}

	// C erases
	m_bErase = pKeyboard->IsDown( TInputDeviceKeyboard::KEY_C );

	if ( !pCursor->IsMPColour() )
		pCursor->SetColour( s_auiColours[ m_iColour ] );

	TVector2 vCursor;
	pCursor->GetPosition( vCursor );

	T2GUITransform oInvTransform;
	GetInvScreenTransform( oInvTransform );

	TVector2 vPos;
	oInvTransform.Transform( vPos, vCursor );

	if ( m_bReset )
	{
		m_vLastPoint = vPos;
		m_vPrevPos   = vPos;
		m_bReset     = TFALSE;
	}

	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	TBOOL bTrailActive   = !m_bErase && pCursor->IsSceneVisible();
	TBOOL bParentVisible = GetParent()->IsVisible();

	if ( bTrailActive )
	{
		if ( bParentVisible )
		{
			// The paint sound follows the speed of the cursor
			if ( m_pEvent != TNULL )
			{
				FMOD::EventParameter* pSpeedParam = TNULL;
				FMOD::EventParameter* pLoadParam  = TNULL;
				m_pEvent->getParameterByIndex( 0, &pSpeedParam );
				m_pEvent->getParameter( "load", &pLoadParam );

				if ( pSpeedParam != TNULL )
				{
					TFLOAT fMin;
					TFLOAT fMax;
					pSpeedParam->getRange( &fMin, &fMax );

					m_vSoundPos      = vPos;
					TFLOAT fDistance = TVector2::Distance( m_vSoundPos, m_vSoundPrevPos );
					TFLOAT fSpeed    = fDistance / a_fDeltaTime;

					m_vSoundPrevPos = m_vSoundPos;

					m_afSpeeds[ m_iSpeedHead++ ] = fSpeed * 1.0f;

					if ( m_iSpeedHead >= NUM_SPEEDS )
						m_iSpeedHead = 0;

					TFLOAT fSum = 0.0f;

					for ( TINT i = 0; i < NUM_SPEEDS; i += 6 )
					{
						fSum = ( ( ( ( ( fSum + m_afSpeeds[ i ] ) + m_afSpeeds[ i + 1 ] ) + m_afSpeeds[ i + 2 ] ) + m_afSpeeds[ i + 3 ] ) + m_afSpeeds[ i + 4 ] ) + m_afSpeeds[ i + 5 ];
					}

					TFLOAT fAverage = TFLOAT( fSum / 12.0 );
					TMath::Clip( fAverage, fMin, fMax );
					pSpeedParam->setValue( fAverage );
				}

				if ( pLoadParam != TNULL )
				{
					TINT iPrev = m_iSpeedHead - 2;

#ifdef DEBLOB_COMMUNITY_PATCH
					if ( iPrev < 0 )
						iPrev = NUM_SPEEDS + iPrev;
#else
					// NOTE: retail subtracts: the index is 13 or 14 and reads past the speeds into m_vSoundPos
					if ( iPrev < 0 )
						iPrev = NUM_SPEEDS - iPrev;
#endif

					TINT iNext = iPrev + 1;

					if ( iNext >= NUM_SPEEDS )
						iNext = 0;

					TFLOAT fParam = TMath::LERP( m_fSoundParam, ( m_afSpeeds[ iNext ] - m_afSpeeds[ iPrev ] ) * 0.0007f + 0.5f, 7.0f * a_fDeltaTime );
					m_fSoundParam = fParam;

					TFLOAT fParamMin;
					TFLOAT fParamMax;
					pLoadParam->getRange( &fParamMin, &fParamMax );
					TMath::Clip( fParam, fParamMin, fParamMax );
					pLoadParam->setValue( fParam );
				}
			}
			else if ( ASoundLoadHelper::GetSingleton() != TNULL && ASoundLoadHelper::GetSingleton()->IsLoadingSoundsLoaded() )
			{
				m_bSoundStarted = TTRUE;
				ASoundInterface::PlayEvent( ASoundMap::GROUPID_LOADING_SOUNDS, ASoundMap::EVENTID( 283 ), FMOD_EVENT_DEFAULT, TTRUE );
				m_pEvent = ASoundInterface::GetEvent( ASoundMap::GROUPID_LOADING_SOUNDS, ASoundMap::EVENTID( 281 ), FMOD_EVENT_DEFAULT );

				m_fSoundParam = 0.5f;
				m_pEvent->start();
			}
		}
		else
		{
			// The trail is active but its parent is hidden: the sound stops
			if ( m_pEvent != TNULL )
			{
				m_pEvent->stop( false );
				m_pEvent = TNULL;
			}
		}
	}
	else
	{
		if ( bParentVisible && m_bSoundStarted && m_bErase )
		{
			if ( ASoundLoadHelper::GetSingleton()->IsLoadingSoundsLoaded() )
			{
				ASoundInterface::PlayEvent( ASoundMap::GROUPID_LOADING_SOUNDS, ASoundMap::EVENTID( 285 ), FMOD_EVENT_DEFAULT, TTRUE );
				m_bSoundStarted = TFALSE;
			}
		}

		if ( m_pEvent != TNULL )
		{
			if ( bParentVisible )
				ASoundInterface::PlayEvent( ASoundMap::GROUPID_LOADING_SOUNDS, ASoundMap::EVENTID( 284 ), FMOD_EVENT_DEFAULT, TTRUE );

			m_pEvent->stop( false );
			m_pEvent = TNULL;
		}
	}

	if ( pCursor->IsSceneVisible() )
	{
		// The cursor moved: a new point
		if ( !vPos.IsEqual( m_vPrevPos ) )
		{
			TINT iHead = m_iHead;

			if ( m_aPoints[ iHead ].fLife <= 0.0f )
				m_iNumVisible++;

			m_aPoints[ m_iHead ].vPos    = vPos;
			m_aPoints[ m_iHead ].fLife   = 10.0f;
			m_aPoints[ m_iHead ].fDistSq = ( m_vPrevPos.y - vPos.y ) * ( m_vPrevPos.y - vPos.y ) + ( m_vPrevPos.x - vPos.x ) * ( m_vPrevPos.x - vPos.x );

			TUINT32 uiColour = pCursor->IsMPColour() ? pCursor->GetColour() : s_auiColours[ m_iColour ];

			m_aPoints[ m_iHead ].uiColour = uiColour;

			if ( m_bErase )
				m_aPoints[ m_iHead ].uiColour &= 0x00FFFFFF;

			StopDripAtHead();

			if ( m_iNumPoints == 0 )
			{
				m_iHead++;
				m_iNumPoints = 1;

				if ( m_aPoints[ m_iHead ].fLife <= 0.0f )
					m_iNumVisible++;

				m_aPoints[ m_iHead ].vPos     = vPos;
				m_aPoints[ m_iHead ].fLife    = 10.0f;
				m_aPoints[ m_iHead ].uiColour = s_auiColours[ m_iColour ];

				StopDripAtHead();
			}
		}

		// Far enough from the last point: the point is kept and the trail may drip or splat
		TFLOAT fDistSq = ( m_vLastPoint.y - vPos.y ) * ( m_vLastPoint.y - vPos.y ) + ( m_vLastPoint.x - vPos.x ) * ( m_vLastPoint.x - vPos.x );

		if ( fDistSq > 400.0f )
		{
			TUINT32 uiColour = pCursor->IsMPColour() ? pCursor->GetColour() : s_auiColours[ m_iColour ];

			if ( !m_bErase && m_aPoints[ m_iHead ].fDistSq < 100.0f )
			{
				if ( rRandom.GetFloatMinMax( 0.0f, 1.0f ) < 0.1f )
				{
					TFLOAT fHalfWidth  = GetWidth() * 0.5f;
					TFLOAT fHalfHeight = GetHeight() * 0.5f;

					Effect& rDrip = m_aDrips[ m_iNextDrip ];

					rDrip.pScene->SetVisible( TTRUE );
					rDrip.pScene->SetPosition( vPos.x + fHalfWidth, vPos.y + fHalfHeight );
					rDrip.pScene->StopAnimation();
					rDrip.pScene->PlayAnimation( L"Drip", L"EndDrip" );
					rDrip.iPoint = m_iHead;

					for ( T2GUIElement* pChild = rDrip.pScene->ChildBegin(); pChild != TNULL; pChild = rDrip.pScene->GetNextChild( pChild ) )
					{
						TASSERT( TSTATICCAST( TXUIElement, pChild )->IsA( &TGetClass( TXUIFigure ) ) );
						TSTATICCAST( TXUIFigure, pChild )->SetFillColour( uiColour );
					}

					if ( ++m_iNextDrip >= NUM_DRIPS )
						m_iNextDrip = 0;
				}
			}

			m_fSplatTimer += a_fDeltaTime;

			if ( !m_bErase )
			{
				if ( rRandom.GetFloatMinMax( 0.0f, 1.0f ) < 0.05f && m_fSplatTimer >= 1.0f )
				{
					m_fSplatTimer = 0.0f;

					TFLOAT fHalfWidth  = GetWidth() * 0.5f;
					TFLOAT fHalfHeight = GetHeight() * 0.5f;

					Effect& rSplat = m_aSplats[ m_iNextSplat ];

					TFLOAT fSplatHalfWidth  = rSplat.pScene->GetWidth() * 0.5f;
					TFLOAT fSplatHalfHeight = rSplat.pScene->GetHeight() * 0.5f;

					rSplat.pScene->SetVisible( TTRUE );
					rSplat.pScene->SetPosition( ( fHalfWidth + vPos.x ) - fSplatHalfWidth, ( fHalfHeight + vPos.y ) - fSplatHalfHeight );
					rSplat.pScene->SetZRotation( rRandom.GetFloatMinMax( 0.0f, 360.0f ) );
					rSplat.pScene->PlayAnimation( L"Splat", L"EndSplat" );
					rSplat.iPoint = m_iHead;

					for ( T2GUIElement* pChild = rSplat.pScene->ChildBegin(); pChild != TNULL; pChild = rSplat.pScene->GetNextChild( pChild ) )
						pChild->SetColour( uiColour );

					if ( ++m_iNextSplat >= NUM_SPLATS )
						m_iNextSplat = 0;
				}
			}

			m_iHead++;

			if ( ++m_iNumPoints > MAX_POINTS )
				m_iNumPoints = MAX_POINTS;

			if ( m_iHead >= MAX_POINTS )
				m_iHead = 0;

			m_vLastPoint              = vPos;
			m_aPoints[ m_iHead ].vPos = vPos;
		}
	}

	m_vPrevPos = vPos;

	// The points fade out the faster the more of them are alive
	TFLOAT fVisible = TFLOAT( m_iNumVisible ) * 0.00390625f;
	TMath::Clip( fVisible, 0.0f, 1.0f );
	m_fFadeSpeed = TMath::LERP( 1.0f, 5.0f, fVisible );

	TINT iPoint = m_iHead - 1;

	if ( iPoint < 0 )
		iPoint = MAX_POINTS - 1;

	for ( TINT i = 0; i < m_iNumPoints; i++ )
	{
		TFLOAT fOldLife = m_aPoints[ iPoint ].fLife;
		TBOOL  bAlive   = fOldLife > 0.0f;

		m_aPoints[ iPoint ].fLife = fOldLife - m_fFadeSpeed * a_fDeltaTime;

		if ( m_aPoints[ iPoint ].fLife < 0.0f )
		{
			m_aPoints[ iPoint ].fLife = 0.0f;

			if ( bAlive )
			{
				if ( --m_iNumVisible < 0 )
					m_iNumVisible = 0;
			}
		}

		if ( --iPoint < 0 )
			iPoint = MAX_POINTS - 1;
	}

	TFLOAT fHeadLife = m_aPoints[ m_iHead ].fLife;

	m_aPoints[ m_iHead ].fLife = fHeadLife - m_fFadeSpeed * a_fDeltaTime;

	if ( m_aPoints[ m_iHead ].fLife < 0.0f )
	{
		m_aPoints[ m_iHead ].fLife = 0.0f;

		if ( fHeadLife > 0.0f )
		{
			if ( --m_iNumVisible < 0 )
				m_iNumVisible = 0;
		}
	}

	// The drips fade with their point
	for ( TINT i = 0; i < NUM_DRIPS; i++ )
	{
		Effect& rDrip = m_aDrips[ i ];

		if ( rDrip.pScene->GetTimelineControl().IsPlaying() )
		{
			if ( rRandom.GetFloat() < 0.05f )
				rDrip.pScene->StopAnimation();
		}

		if ( rDrip.iPoint != -1 )
		{
			TFLOAT fLife = m_aPoints[ rDrip.iPoint ].fLife;

			if ( FADE_LIFE * 2.0f <= fLife )
				rDrip.pScene->SetOpacity( 1.0f );
			else
				rDrip.pScene->SetOpacity( fLife / ( FADE_LIFE * 2.0f ) );

			if ( fLife <= 0.0f )
			{
				rDrip.iPoint = -1;
				rDrip.pScene->SetVisible( TFALSE );
			}
		}
	}

	// The splats too
	for ( TINT i = 0; i < NUM_SPLATS; i++ )
	{
		Effect& rSplat = m_aSplats[ i ];

		if ( rSplat.iPoint != -1 )
		{
			TFLOAT fLife = m_aPoints[ rSplat.iPoint ].fLife;

			if ( FADE_LIFE <= fLife )
				rSplat.pScene->SetOpacity( 1.0f );
			else
				rSplat.pScene->SetOpacity( fLife / FADE_LIFE );

			if ( fLife <= 0.0f )
			{
				rSplat.iPoint = -1;
				rSplat.pScene->SetVisible( TFALSE );
			}
		}
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00628210
void ABlobCursorTrail::Render()
{
	TXUIControl::Render();

	TXUIRenderer* pRenderer = TXUI::GetSingleton()->GetRenderer();

	if ( m_iNumPoints <= 0 )
		return;

	pRenderer->SetMaterial( m_pMaterial );

	TINT     iVertex = 0;
	TVector2 vTemp( 1.0f, 0.0f );
	TVector2 vNormal( 1.0f, 0.0f );
	TVector2 vPrevNormal( 1.0f, 0.0f );
	TVector2 vPrevCurve( 0.0f, 0.0f );
	TVector2 vVertexA( 0.0f, 0.0f );
	TVector2 vVertexB( 0.0f, 0.0f );
	TBOOL    bFirst   = TTRUE;
	TUINT32  uiColour = s_auiColours[ m_iColour ];

	// NOTE: retail wraps with 0x101 instead of 256, the oldest point is skipped
	TINT iIndex = m_iHead - m_iNumPoints;

	if ( iIndex < 0 )
		iIndex += 0x101;

	for ( TINT iLoop = 0; iLoop < m_iNumPoints; iLoop++ )
	{
		TINT iCur = iIndex;

		if ( iCur >= MAX_POINTS )
			iCur = 0;

		TVector2 vP1 = m_aPoints[ iCur ].vPos;

		iIndex = iCur + 1;

		TINT iNext = iIndex;

		if ( iNext >= MAX_POINTS )
			iNext = 0;

		if ( iCur == m_iHead )
			break;

		TINT iPrev = iCur - 1;

		if ( iPrev < 0 )
			iPrev = MAX_POINTS - 1;

		TINT iNext2 = iNext + 1;

		if ( iNext2 >= MAX_POINTS )
			iNext2 = 0;

		TVector2 vP2 = m_aPoints[ iNext ].vPos;

		TUINT32 uiPointColour = m_aPoints[ iCur ].uiColour;
		TUINT32 uiBase        = uiPointColour & 0x00FFFFFF;
		TUINT32 uiPointAlpha  = uiPointColour >> 24;

		TFLOAT fSegmentLenSq = ( vP2.y - vP1.y ) * ( vP2.y - vP1.y ) + ( vP2.x - vP1.x ) * ( vP2.x - vP1.x );

		// NOTE: GetAlpha returns in st0, retail multiplies in the x87 registers
		TINT iAlpha = TINT( TFLOAT( double( GetAlpha() ) * double( TFLOAT( uiPointAlpha ) / 255.0f ) * 255.0 ) );

		uiColour = ( TUINT32( iAlpha ) << 24 ) | uiBase;

		if ( vP1.IsEqual( vP2 ) )
			continue;

		fSegmentLenSq = fSegmentLenSq / 25.0f;
		TMath::Clip( fSegmentLenSq, 0.0f, 1.0f );

		TINT iSteps = TINT( fSegmentLenSq * 5.0f );

		if ( iSteps != 0 )
		{
			if ( iSteps <= 0 )
				continue;
		}
		else
		{
			iSteps = 1;
		}

		TFLOAT   fLife1   = m_aPoints[ iCur ].fLife;
		TFLOAT   fLife2   = m_aPoints[ iNext ].fLife;
		TFLOAT   fDistSq1 = m_aPoints[ iCur ].fDistSq;
		TFLOAT   fDistSq2 = m_aPoints[ iNext ].fDistSq;
		TVector2 vTangent1;

		if ( bFirst )
		{
			bFirst      = TFALSE;
			vPrevCurve  = vP1;
			vTangent1.x = vP2.x - vP1.x;
			vTangent1.y = vP2.y - vP1.y;
		}
		else
		{
			vTangent1.x = vP2.x - m_aPoints[ iPrev ].vPos.x;
			vTangent1.y = vP2.y - m_aPoints[ iPrev ].vPos.y;
		}

		TVector2 vTangent2;
		TINT     iLastSegment = ( m_iNumPoints >= MAX_POINTS ) ? m_iNumPoints - 2 : m_iNumPoints - 1;

		if ( iLoop == iLastSegment )
		{
			vTangent2.x = vP2.x - vP1.x;
			vTangent2.y = vP2.y - vP1.y;
		}
		else
		{
			TFLOAT fDx = vP2.x - m_aPoints[ iNext2 ].vPos.x;
			TFLOAT fDy = vP2.y - m_aPoints[ iNext2 ].vPos.y;

			if ( 25.0f <= fDy * fDy + fDx * fDx )
			{
				vTangent2.x = m_aPoints[ iNext2 ].vPos.x - vP1.x;
				vTangent2.y = m_aPoints[ iNext2 ].vPos.y - vP1.y;
			}
			else
			{
				vTangent2.x = vP2.x - vP1.x;
				vTangent2.y = vP2.y - vP1.y;
			}
		}

		TFLOAT fTan1X = vTangent1.x * 0.5f;
		TFLOAT fTan1Y = vTangent1.y * 0.5f;
		TFLOAT fTan2X = vTangent2.x * 0.5f;
		TFLOAT fTan2Y = vTangent2.y * 0.5f;

		TFLOAT fT         = 1.0f / TFLOAT( iSteps );
		TFLOAT fDistDelta = fDistSq2 - fDistSq1;

		for ( TINT k = 0; k < iSteps; k++ )
		{
			// The point on the Hermite spline between the two points
			TFLOAT fT2  = fT * fT;
			TFLOAT fT3  = fT2 * fT;
			TFLOAT fH01 = fT2 * 3.0f;
			TFLOAT fH1a = ( fT3 * 2.0f ) - fH01;
			TFLOAT fH1b = fH01 - ( fT3 * 2.0f );
			TFLOAT fH00 = fH1a + 1.0f;
			TFLOAT fH11 = fT3 - fT2;
			TFLOAT fH10 = ( fT3 - ( fT2 * 2.0f ) ) + fT;

			TVector2 vCurve;
			vCurve.x = ( ( fTan1X * fH10 ) + ( ( vP1.x * fH00 ) + ( vP2.x * fH1b ) ) ) + ( fTan2X * fH11 );
			vCurve.y = ( ( fTan1Y * fH10 ) + ( ( vP1.y * fH00 ) + ( vP2.y * fH1b ) ) ) + ( fTan2Y * fH11 );

			TFLOAT fWidthScale = 1.0f - ( ( ( TFLOAT( k ) * fDistDelta ) / TFLOAT( iSteps ) + fDistSq1 ) * 0.000625f );
			TMath::Clip( fWidthScale, 0.4f, 1.0f );

			TFLOAT fWidth;

			if ( ( iLoop != 0 || k != 0 ) && ( iLoop != m_iNumPoints - 1 || k != 4 ) )
			{
				// The normal is averaged with the previous one
				if ( vCurve.IsEqual( vPrevCurve ) )
				{
					vNormal = TVector2( 1.0f, 0.0f );
				}
				else
				{
					vNormal.x = vCurve.y - vPrevCurve.y;
					vNormal.y = -( vCurve.x - vPrevCurve.x );
					vNormal.Normalize();
				}

				vTemp.x = vPrevNormal.x + vNormal.x;
				vTemp.y = vPrevNormal.y + vNormal.y;
				vTemp.Normalize();

				fWidth = fWidthScale * 8.0f;
			}
			else
			{
				// The first and the 5th step of the last segment
				if ( vCurve.IsEqual( vPrevCurve ) )
				{
					vTemp = TVector2( 1.0f, 0.0f );
				}
				else
				{
					vTemp.x = vCurve.y - vPrevCurve.y;
					vTemp.y = -( vCurve.x - vPrevCurve.x );
					vTemp.Normalize();
				}

				vNormal = vTemp;

				if ( iLoop != 0 || k != 0 )
					fWidth = fWidthScale * 8.0f;
				else
					fWidth = 8.0f;
			}

			fT = fT + ( 1.0f / TFLOAT( iSteps ) );

			// The alpha follows the life of the points
			TFLOAT fLife = ( ( ( fLife2 - fLife1 ) * TFLOAT( k ) ) / TFLOAT( iSteps ) ) + fLife1;

			if ( iAlpha != 0 )
			{
				if ( FADE_LIFE < fLife )
					iAlpha = 255;
				else
					iAlpha = TINT( TFLOAT( fLife * 255.0f ) / FADE_LIFE );
			}

			// NOTE: GetAlpha returns in st0, retail multiplies in the x87 registers
			TFLOAT fElementAlpha = GetAlpha();
			TFLOAT fAlpha        = TFLOAT( double( fElementAlpha ) * double( TFLOAT( iAlpha ) / 255.0f ) * 255.0 );

			vVertexA.x = ( vTemp.x * fWidth ) + vPrevCurve.x;
			vVertexA.y = ( vTemp.y * fWidth ) + vPrevCurve.y;
			vVertexB.x = vPrevCurve.x - ( vTemp.x * fWidth );
			vVertexB.y = vPrevCurve.y - ( vTemp.y * fWidth );

			iAlpha   = TINT( fAlpha );
			uiColour = ( TUINT32( iAlpha ) << 24 ) | uiBase;

			m_pColours[ iVertex ]      = uiColour;
			m_pVertices[ iVertex ]     = vVertexA;
			m_pColours[ iVertex + 1 ]  = uiColour;
			m_pVertices[ iVertex + 1 ] = vVertexB;

			if ( ( ( iVertex + 1 ) / 2 ) & 1 )
			{
				m_pUVs[ iVertex ]     = TVector2( 1.0f, 0.0f );
				m_pUVs[ iVertex + 1 ] = TVector2( 1.0f, 1.0f );
			}
			else
			{
				m_pUVs[ iVertex ]     = TVector2( 0.0f, 0.0f );
				m_pUVs[ iVertex + 1 ] = TVector2( 0.0f, 1.0f );
			}

			iVertex += 2;
			vPrevCurve  = vCurve;
			vPrevNormal = vNormal;
		}

		// The buffer is nearly full: draw it and start again with the last pair
		if ( iVertex >= 500 )
		{
			m_pColours[ iVertex ]      = uiColour;
			m_pVertices[ iVertex ]     = vVertexA;
			m_pColours[ iVertex + 1 ]  = uiColour;
			m_pVertices[ iVertex + 1 ] = vVertexB;

			if ( ( ( iVertex + 1 ) / 2 ) & 1 )
			{
				m_pUVs[ iVertex ]     = TVector2( 1.0f, 0.0f );
				m_pUVs[ iVertex + 1 ] = TVector2( 1.0f, 1.0f );
			}
			else
			{
				m_pUVs[ iVertex ]     = TVector2( 0.0f, 0.0f );
				m_pUVs[ iVertex + 1 ] = TVector2( 0.0f, 1.0f );
			}

			pRenderer->RenderTriStripColours( m_pVertices, m_pUVs, m_pColours, iVertex + 2, 1.0f, 1.0f );

			iVertex = 2;

			m_pColours[ 0 ]  = uiColour;
			m_pVertices[ 0 ] = vVertexA;
			m_pColours[ 1 ]  = uiColour;
			m_pVertices[ 1 ] = vVertexB;
			m_pUVs[ 0 ]      = TVector2( 0.0f, 0.0f );
			m_pUVs[ 1 ]      = TVector2( 0.0f, 1.0f );
		}
	}

	if ( m_iNumPoints > 1 )
	{
		m_pColours[ iVertex ]      = uiColour;
		m_pVertices[ iVertex ]     = vVertexA;
		m_pColours[ iVertex + 1 ]  = uiColour;
		m_pVertices[ iVertex + 1 ] = vVertexB;

		if ( ( ( iVertex + 1 ) / 2 ) & 1 )
		{
			m_pUVs[ iVertex ]     = TVector2( 1.0f, 0.0f );
			m_pUVs[ iVertex + 1 ] = TVector2( 1.0f, 1.0f );
		}
		else
		{
			m_pUVs[ iVertex ]     = TVector2( 0.0f, 0.0f );
			m_pUVs[ iVertex + 1 ] = TVector2( 0.0f, 1.0f );
		}

		TINT iNumVertices = iVertex + 2;

		if ( iNumVertices > 2 )
		{
			pRenderer->RenderTriStripColours( m_pVertices, m_pUVs, m_pColours, iNumVertices, 1.0f, 1.0f );

			// The round cap of the head: a half disc around the middle of the last pair
			TVector2 avCap[ 6 ];

			TFLOAT fCenterX = ( ( vVertexA.x - vVertexB.x ) * 0.5f ) + vVertexB.x;
			TFLOAT fCenterY = ( ( vVertexA.y - vVertexB.y ) * 0.5f ) + vVertexB.y;

			avCap[ 0 ] = TVector2( fCenterX, fCenterY );
			avCap[ 1 ] = vVertexA;

			TVector2 vRadius( vVertexA.x - fCenterX, vVertexA.y - fCenterY );

			for ( TINT i = 0; i < 4; i++ )
			{
				vRadius.Rotate( -(TFLOAT)( 3.1415927 * 0.25 ) );
				avCap[ 2 + i ] = TVector2( vRadius.x + fCenterX, vRadius.y + fCenterY );
			}

			TUINT16 auiIndices[ 12 ];

			for ( TINT i = 0; i < 4; i++ )
			{
				auiIndices[ i * 3 ]     = 0;
				auiIndices[ i * 3 + 1 ] = TUINT16( i + 1 );
				auiIndices[ i * 3 + 2 ] = TUINT16( i + 2 );
			}

			pRenderer->SetColour( uiColour );
			pRenderer->SetMaterial( TNULL );
			pRenderer->RenderIndexedTriList( avCap, TNULL, auiIndices, 12, 2, 6, 1.0f, 1.0f );
		}
	}
}

// $deBlob: FUNCTION 00629100
void ABlobCursorTrail::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == TFALSE )
	{
		Reset();
		Hide();
	}
	else if ( a_bVisible == TTRUE )
	{
		Show();
	}
	else
	{
		Hide();
	}
}

// $deBlob: FUNCTION 00629130
void ABlobCursorTrail::Reset()
{
	m_iNumPoints  = 0;
	m_iHead       = 0;
	m_iNumVisible = 0;

	if ( m_pEvent != TNULL )
	{
		m_pEvent->stop( false );
		m_pEvent = TNULL;
	}

	m_vLastPoint = TVector2::VEC_ZERO;
	m_vPrevPos   = m_vLastPoint;

	for ( TINT i = 0; i < NUM_SPLATS; i++ )
	{
		m_aSplats[ i ].pScene->SetVisible( TFALSE );
		m_aSplats[ i ].iPoint = -1;
	}

	for ( TINT i = 0; i < NUM_DRIPS; i++ )
	{
		m_aDrips[ i ].pScene->SetVisible( TFALSE );
		m_aDrips[ i ].iPoint = -1;
	}

	// NOTE: retail's MemSet is memset: the low byte of the colour fills the first 512 bytes
	memset( m_pColours, s_auiColours[ m_iColour ], m_iMaxVertices );
	m_aPoints[ 0 ].uiColour = s_auiColours[ m_iColour ];

	m_fSplatTimer = 0.0f;
	m_bReset      = TTRUE;
}

// $deBlob: FUNCTION 00629230
void ABlobCursorTrail::CreateEffects()
{
	AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentGameState();

	TXUIResource* pResource;
	const TWCHAR* wszSplat;
	const TWCHAR* wszDrip;

	if ( pGameState->IsA( &TGetClass( AInGameState ) ) )
	{
		pResource = TXUI::GetSingleton()->FindResource( "ingame.xur" );
		wszSplat  = L"CursorTrailSplat2";
		wszDrip   = L"CursorTrailDrip2";
	}
	else if ( pGameState->IsA( &TGetClass( ALoadLevelState ) ) )
	{
		AGameStatus* pGameStatus = AGameStatus::GetSingleton();
		const TCHAR* szXUR;

		if ( pGameStatus->IsMultiplayer() )
		{
			szXUR = "loadingscreen_multi.xur";
		}
		else if ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 || pGameStatus->GetLevelMode() == AGameStatus::Mode_FreePaint )
		{
			szXUR = "loadingsprint.xur";
		}
		else if ( pGameStatus->GetSPLevel() == -1 )
		{
			szXUR = s_aLoadingScreenXURs[ 0 ];
		}
		else
		{
			szXUR = s_aLoadingScreenXURs[ pGameStatus->GetSPLevel() ];
		}

		pResource = TXUI::GetSingleton()->FindResource( szXUR );
		wszSplat  = L"CursorTrailSplat3";
		wszDrip   = L"CursorTrailDrip3";
	}
	else
	{
		pResource = TXUI::GetSingleton()->FindResource( "frontend.xur" );
		wszSplat  = L"CursorTrailSplat";
		wszDrip   = L"CursorTrailDrip";
	}

	for ( TINT i = 0; i < NUM_DRIPS; i++ )
	{
		TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( pResource, wszDrip, TFALSE );

		m_aDrips[ i ].pScene = pScene;
		pScene->SetVisible( TFALSE );
		AddChildTail( pScene );
		m_aDrips[ i ].iPoint = -1;
	}

	for ( TINT i = 0; i < NUM_SPLATS; i++ )
	{
		TXUIScene* pScene = TXUI::GetSingleton()->CreateScene( pResource, wszSplat, TFALSE );

		m_aSplats[ i ].pScene = pScene;
		pScene->SetVisible( TFALSE );
		AddChildTail( pScene );
		m_aSplats[ i ].iPoint = -1;
	}
}

// $deBlob: FUNCTION 006293b0
void ABlobCursorTrail::StopSound( TBOOL a_bPlayEnd )
{
	if ( m_pEvent != TNULL )
	{
		if ( a_bPlayEnd )
			ASoundInterface::PlayEvent( ASoundMap::GROUPID_LOADING_SOUNDS, ASoundMap::EVENTID( 284 ), FMOD_EVENT_DEFAULT, TTRUE );

		m_pEvent->stop( false );
		m_pEvent = TNULL;
	}
}

// $deBlob: FUNCTION 00629400
void ABlobCursorTrail::StopDripAtHead()
{
	for ( TINT i = 0; i < NUM_DRIPS; i++ )
	{
		if ( m_aDrips[ i ].iPoint == m_iHead )
		{
			m_aDrips[ i ].iPoint = -1;
			m_aDrips[ i ].pScene->SetVisible( TFALSE );
			return;
		}
	}
}

// $deBlob: FUNCTION 00629440
void ABlobCursorTrail::ClampFades()
{
	TXUI* pXUI = TXUI::GetSingleton();

	if ( pXUI->GetTransToScene() != TNULL || pXUI->GetTransFromScene() != TNULL || pXUI->IsInputLocked() || !IsVisible() )
		return;

	TFLOAT fMaxLife = FADE_LIFE * 0.25f;

	// NOTE: retail clamps the points 0 to m_iNumPoints, not the ones of the ring
	for ( TINT i = 0; i <= m_iNumPoints; i++ )
	{
		if ( m_aPoints[ i ].fLife >= fMaxLife )
			m_aPoints[ i ].fLife = fMaxLife;
	}

	for ( TINT i = 0; i < NUM_DRIPS; i++ )
	{
		TINT iPoint = m_aDrips[ i ].iPoint;

		if ( iPoint != -1 && m_aPoints[ iPoint ].fLife >= fMaxLife )
			m_aPoints[ iPoint ].fLife = fMaxLife;
	}

	for ( TINT i = 0; i < NUM_SPLATS; i++ )
	{
		TINT iPoint = m_aSplats[ i ].iPoint;

		if ( iPoint != -1 && m_aPoints[ iPoint ].fLife >= fMaxLife )
			m_aPoints[ iPoint ].fLife = fMaxLife;
	}
}

// ClampFades of every trail
// $deBlob: FUNCTION 00629710
void ABlobCursorTrail::ClampAllFades()
{
	for ( TINT i = 0; i < MAX_CURSOR_ARRAY; i++ )
	{
		if ( sm_apCursorTrails[ i ] != TNULL )
			sm_apCursorTrails[ i ]->ClampFades();
	}
}

// Reset of every trail
// $deBlob: FUNCTION 00629a10
void ABlobCursorTrail::ResetAll()
{
	for ( TINT i = 0; i < MAX_CURSOR_ARRAY; i++ )
	{
		if ( sm_apCursorTrails[ i ] != TNULL )
			sm_apCursorTrails[ i ]->Reset();
	}
}

// $deBlob: FUNCTION 00629b30
TVector2* ABlobCursorTrail::EvaluateHermite( TVector2* a_pOut, const TVector2* a_pPoints, TFLOAT a_fT )
{
	TFLOAT fT2  = a_fT * a_fT;
	TFLOAT fT3  = fT2 * a_fT;
	TFLOAT fH01 = fT2 * 3.0f;
	TFLOAT fA   = fT3 * 2.0f;
	TFLOAT fH11 = fT3 - fT2;
	TFLOAT fH00 = ( fA - fH01 ) + 1.0f;
	TFLOAT fH1b = fH01 - fA;
	TFLOAT fH10 = ( fT3 - ( fT2 * 2.0f ) ) + a_fT;

	a_pOut->x = ( ( ( a_pPoints[ 0 ].x * fH00 ) + ( a_pPoints[ 1 ].x * fH1b ) ) + ( a_pPoints[ 2 ].x * fH10 ) ) + ( a_pPoints[ 3 ].x * fH11 );
	a_pOut->y = ( ( ( a_pPoints[ 0 ].y * fH00 ) + ( a_pPoints[ 1 ].y * fH1b ) ) + ( a_pPoints[ 2 ].y * fH10 ) ) + ( a_pPoints[ 3 ].y * fH11 );
	return a_pOut;
}

// $deBlob: FUNCTION 00629c10
TVector2* ABlobCursorTrail::EvaluateBezier( TVector2* a_pOut, const TVector2* a_pPoints, TINT a_iDegree, TFLOAT a_fT )
{
	TFLOAT fSumX = 0.0f;
	TFLOAT fSumY = 0.0f;

	if ( a_fT == 0.0f )
	{
		*a_pOut = a_pPoints[ 0 ];
		return a_pOut;
	}

	TFLOAT fPowT = 1.0f;

	if ( a_fT >= 1.0f )
	{
		// NOTE: retail returns the 4th point whatever the degree is
		*a_pOut = a_pPoints[ 3 ];
		return a_pOut;
	}

	TFLOAT fResultX = 0.0f;
	TFLOAT fResultY = 0.0f;

	TFLOAT fOneMinusT    = 1.0f - a_fT;
	TFLOAT fPowOneMinusT = TFLOAT( pow( double( fOneMinusT ), double( TFLOAT( a_iDegree ) ) ) );

	if ( a_iDegree >= 0 )
	{
		TINT iRemaining = a_iDegree;

		for ( TINT i = 0; i <= a_iDegree; i++ )
		{
			TFLOAT fCoefficient = fPowOneMinusT * fPowT;
			fPowT               = fPowT * a_fT;
			fPowOneMinusT       = fPowOneMinusT / fOneMinusT;

			TINT iA = a_iDegree;
			TINT iB = i;
			TINT iC = iRemaining;

			// The binomial coefficient
			if ( a_iDegree >= 1 )
			{
				do
				{
					fCoefficient = fCoefficient * TFLOAT( iA );
					iA--;

					if ( iB > 1 )
					{
						fCoefficient = fCoefficient / TFLOAT( iB );
						iB--;
					}

					if ( iC > 1 )
					{
						fCoefficient = fCoefficient / TFLOAT( iC );
						iC--;
					}
				} while ( iA >= 1 );
			}

			iRemaining--;

			fSumX    = ( a_pPoints[ i ].x * fCoefficient ) + fSumX;
			fResultX = fSumX;
			fSumY    = ( a_pPoints[ i ].y * fCoefficient ) + fSumY;
			fResultY = fSumY;
		}
	}

	a_pOut->x = fResultX;
	a_pOut->y = fResultY;
	return a_pOut;
}
