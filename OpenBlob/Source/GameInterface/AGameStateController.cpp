#include "pch.h"
#include "AGameStateController.h"
#include "AInGameState.h"
#include "Console/AConsoleState.h"
#include "AApplication.h"
#include "Memory/AMemory.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "GUI/AGUIManager.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUI.h>
#include <Render/TViewport.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005c4ad0
AGameStateController::AGameStateController()
{
	m_bStatePushing = TFALSE;

	// NOTE: retail leaves m_fWidescreenTime uninitialised
	m_fWidescreenDuration    = 0.0f;
	m_bRenderFadesWhenPaused = TFALSE;
	m_bUnk13D                = TFALSE;

	for ( TINT i = NUM_FADEVIEWPORT; i != 0; )
	{
		i--;
		m_afFadeAlpha[ i ]               = 0.0f;
		m_aFadeViewPort[ i ].m_pFade     = TNULL;
		m_aFadeViewPort[ i ].m_fTime     = 0.0f;
		m_aFadeViewPort[ i ].m_fDuration = 0.0f;
		m_aFadeViewPort[ i ].m_eFadeType = 0;
		m_aFadeViewPort[ i ].m_bEnabled  = TFALSE;
	}
}

// NOTE: the destructor is TSingleton's (see TSingleton.h)
AGameStateController::~AGameStateController()
{
}

// $deBlob: FUNCTION 005c4bb0
void AGameStateController::Create()
{
	T2GUIElement*        pRoot            = T2GUI::GetSingleton()->GetRootElement();
	AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
	TFLOAT               fScreenHeight    = pViewportManager->GetViewport( AXYZViewportManager::VIEWPORT( 0 ) )->GetHeight();

	for ( TINT i = NUM_FADEVIEWPORT; i != 0; )
	{
		i--;
		TASSERT( TNULL == m_aFadeViewPort[ i ].m_pFade );

		// The fade rectangle of the viewport (black, transparent)
		TViewport* pViewport = pViewportManager->GetViewport( AXYZViewportManager::VIEWPORT( i ) );
		TFLOAT     fWidth    = pViewport->GetWidth();
		TFLOAT     fHeight   = pViewport->GetHeight();
		TFLOAT     fX        = pViewport->GetX();
		TFLOAT     fBottom   = pViewport->GetY() + fHeight;

		T2GUIRectangle* pFade        = new T2GUIRectangle();
		m_aFadeViewPort[ i ].m_pFade = pFade;
		pFade->Create( fWidth, fHeight );
		pFade->SetAnchor( T2GUIATTACHMENT_TOPLEFT );
		pFade->SetPivot( T2GUIATTACHMENT_TOPLEFT );
		pFade->SetTransform( fX, fScreenHeight - fBottom, 0.0f );
		pFade->SetColour( 0xFF000000 );
		pFade->SetAlpha( 0.0f );
		pFade->SetState( T2GUIElement::STATE_INGAME | T2GUIElement::STATE_MENU | T2GUIElement::STATE_UNK3 | T2GUIElement::STATE_SUBTITLES );
		pRoot->AddChildTail( pFade );

		m_aFadeViewPort[ i ].m_fTime     = 0.0f;
		m_aFadeViewPort[ i ].m_fDuration = 0.0f;
		m_aFadeViewPort[ i ].m_eFadeType = 0;
		m_aFadeViewPort[ i ].m_bEnabled  = TFALSE;

		TSTATICCAST( T2GUIElement, m_aFadeViewPort[ i ].m_pFade )->SetVisible( TFALSE );
	}

	// The widescreen bars (54/448 and 74/448 of the root's height) in the safe area
	TFLOAT fRootWidth;
	TFLOAT fRootHeight;
	pRoot->GetDimensions( fRootWidth, fRootHeight );

	TFLOAT fSafeHeight     = AGUIManager::GetSingleton()->GetSafeArea()->GetHeight();
	m_fWidescreenTopHeight = ( fRootHeight * 54.0f ) / 448.0f;
	TFLOAT fOffset         = ( fRootHeight - fSafeHeight ) * 0.5f;

	T2GUIRectangle* pTop = new T2GUIRectangle();
	m_pWidescreenTop     = pTop;
	pTop->Create( fRootWidth, m_fWidescreenTopHeight );
	pTop->SetAnchor( T2GUIATTACHMENT_TOPCENTER );
	pTop->SetPivot( T2GUIATTACHMENT_TOPCENTER );
	pTop->SetColour( 0xFF000000 );
	pTop->SetTransform( 0.0f, -fOffset, 0.0f );
	pTop->SetVisible( TFALSE );
	pTop->SetState( T2GUIElement::STATE_INGAME | T2GUIElement::STATE_MENU | T2GUIElement::STATE_UNK3 | T2GUIElement::STATE_SUBTITLES );

	m_fWidescreenBottomHeight = ( fRootHeight * 74.0f ) / 448.0f;

	T2GUIRectangle* pBottom = new T2GUIRectangle();
	m_pWidescreenBottom     = pBottom;
	pBottom->Create( fRootWidth, m_fWidescreenBottomHeight );
	pBottom->SetAnchor( T2GUIATTACHMENT_BOTTOMCENTER );
	pBottom->SetPivot( T2GUIATTACHMENT_BOTTOMCENTER );
	pBottom->SetColour( 0xFF000000 );
	pBottom->SetTransform( 0.0f, fOffset, 0.0f );
	pBottom->SetVisible( TFALSE );
	pBottom->SetState( T2GUIElement::STATE_INGAME | T2GUIElement::STATE_MENU | T2GUIElement::STATE_UNK3 | T2GUIElement::STATE_SUBTITLES );

	AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( pTop );
	AGUIManager::GetSingleton()->GetSafeArea()->AddChildTail( pBottom );

	PushState( new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) AGameState() );
}

// $deBlob: FUNCTION 005c5020
void AGameStateController::Destroy()
{
	TASSERT( m_oStateStack.Size() == 1, "State stack is not empty at shutdown!" );

	AGameState* pState = m_oStateStack.Back();
	m_oStateStack.PopBack();

	if ( pState->GetState() & AGameState::State_Activated )
	{
		pState->OnDeactivate();
		pState->RemoveState( AGameState::State_Activated );
	}

	if ( pState->GetState() & AGameState::State_Inserted )
	{
		pState->OnRemoval();
		pState->RemoveState( AGameState::State_Inserted );
	}

	delete pState;

	TASSERT( m_oStateStack.IsEmpty() );
	UnloadGUI();
}

// $deBlob: FUNCTION 005c5140
void AGameStateController::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fDeltaTime = TMath::Min( a_fDeltaTime, 0.1f );

	if ( GetCurrentGameState()->OnUpdate( fDeltaTime ) == AGameState::UpdateResult_Remove )
	{
		GetCurrentGameState()->RemoveSelf();
	}

#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	AConsoleState::OpenIfRequested();
#endif

	UpdateFade( fDeltaTime );
	UpdateWidescreen( fDeltaTime );
}

// $deBlob: FUNCTION 005c51c0
AGameState* AGameStateController::GetCurrentGameState()
{
	TASSERT( !m_oStateStack.IsEmpty(), "No current game state!" );
	return m_oStateStack.Back();
}

// $deBlob: FUNCTION 005c5f80
AGameState* AGameStateController::FindState( TClass* a_pClass )
{
	for ( TINT i = 0; i < m_oStateStack.Size(); i++ )
	{
		if ( m_oStateStack[ i ]->GetClass() == a_pClass )
			return m_oStateStack[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 005c5260
void AGameStateController::PushState( AGameState* a_pState )
{
	TASSERT( m_bStatePushing == TFALSE );
	m_bStatePushing = TTRUE;

	if ( m_oStateStack.Size() != 0 )
	{
		AGameState* pCurrentState = m_oStateStack.Back();

		if ( !( pCurrentState->GetState() & AGameState::State_Suspended ) )
		{
			pCurrentState->OnSuspend();
			pCurrentState->AddState( AGameState::State_Suspended );
		}

		if ( pCurrentState->GetState() & AGameState::State_Activated )
		{
			pCurrentState->OnDeactivate();
			pCurrentState->RemoveState( AGameState::State_Activated );
		}
	}

	m_oStateStack.PushBack( a_pState );

	if ( !( a_pState->GetState() & AGameState::State_Inserted ) )
	{
		a_pState->OnInsertion();
		a_pState->AddState( AGameState::State_Inserted );
	}

	if ( !( a_pState->GetState() & AGameState::State_Activated ) )
	{
		a_pState->OnActivate();
		a_pState->AddState( AGameState::State_Activated );
	}

	m_bStatePushing = TFALSE;
}

// $deBlob: FUNCTION 005c5480
void AGameStateController::PopState( AGameState* a_pState )
{
	TASSERT( m_oStateStack.Size() > 1, "No more states to pop!" );

	AGameState* pState = m_oStateStack.Back();
	TASSERT( pState == a_pState, "Top state doesn't match state being popped!" );

	m_oStateStack.PopBack();

	if ( pState->GetState() & AGameState::State_Activated )
	{
		pState->OnDeactivate();
		pState->RemoveState( AGameState::State_Activated );
	}

	if ( pState->GetState() & AGameState::State_Inserted )
	{
		pState->OnRemoval();
		pState->RemoveState( AGameState::State_Inserted );
	}

	if ( m_oStateStack.Size() == 1 )
	{
		// Only the base state is left
		g_oTheApp.Destroy();
		delete pState;
		return;
	}

	AGameState* pNewState = m_oStateStack.Back();

	if ( pNewState->GetState() & AGameState::State_Suspended )
	{
		pNewState->OnResume( pState );
		pNewState->RemoveState( AGameState::State_Suspended );
	}

	pNewState = m_oStateStack.Back();

	if ( !( pNewState->GetState() & AGameState::State_Activated ) )
	{
		pNewState->OnActivate();
		pNewState->AddState( AGameState::State_Activated );
	}

	delete pState;
}

// $deBlob: FUNCTION 005c5240
void AGameStateController::PopCurrentState()
{
	AGameState* pCurrentState = GetCurrentGameState();

	if ( m_oStateStack.Size() > 1 )
		PopState( pCurrentState );
}

// $deBlob: FUNCTION 005c5730
void AGameStateController::ResetStack()
{
	TBOOL bDeactivate = TTRUE;

	while ( m_oStateStack.Size() > 1 )
	{
		AGameState* pState = m_oStateStack.Back();
		m_oStateStack.PopBack();

		// Only the current state is active
		if ( bDeactivate && ( pState->GetState() & AGameState::State_Activated ) )
		{
			pState->OnDeactivate();
			pState->RemoveState( AGameState::State_Activated );
		}

		bDeactivate = TFALSE;

		if ( pState->GetState() & AGameState::State_Inserted )
		{
			pState->OnRemoval();
			pState->RemoveState( AGameState::State_Inserted );
		}

		delete pState;
	}

	TASSERT( m_oStateStack.Size() == 1 );
}

// $deBlob: FUNCTION 005c5830
TBOOL AGameStateController::IsFadeEnabled( TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );
	return m_aFadeViewPort[ a_iViewport ].m_bEnabled;
}

// $deBlob: FUNCTION 005c5880
void AGameStateController::EnableFade( TBOOL a_bEnable, TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );
	TASSERT( m_aFadeViewPort[ a_iViewport ].m_pFade, "GameState Overlay not created" );

	m_aFadeViewPort[ a_iViewport ].m_bEnabled = a_bEnable;
}

// $deBlob: FUNCTION 005c5910
void AGameStateController::SetFade( TFLOAT a_fAlpha, TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	// NOTE: TNULL until Create's fade rectangles are ported (AXYZViewportManager): the alpha is kept in m_afFadeAlpha
	if ( m_aFadeViewPort[ a_iViewport ].m_pFade != TNULL )
		TSTATICCAST( T2GUIElement, m_aFadeViewPort[ a_iViewport ].m_pFade )->SetAlpha( a_fAlpha );

	m_afFadeAlpha[ a_iViewport ] = a_fAlpha;

	if ( a_fAlpha == 0.0f )
	{
		m_aFadeViewPort[ a_iViewport ].m_bEnabled = TFALSE;
		return;
	}

	if ( !m_aFadeViewPort[ a_iViewport ].m_bEnabled )
		m_aFadeViewPort[ a_iViewport ].m_bEnabled = TTRUE;
}

// $deBlob: FUNCTION 005c59a0
TFLOAT AGameStateController::GetFade( TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	// NOTE: TNULL until Create's fade rectangles are ported (AXYZViewportManager): the stored alpha is returned
	if ( m_aFadeViewPort[ a_iViewport ].m_pFade == TNULL )
		return m_afFadeAlpha[ a_iViewport ];

	return TSTATICCAST( T2GUIElement, m_aFadeViewPort[ a_iViewport ].m_pFade )->GetAlpha();
}

// $deBlob: FUNCTION 005c5a00
void AGameStateController::FadeToBlack( TFLOAT a_fDuration, TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	m_aFadeViewPort[ a_iViewport ].m_eFadeType = FadeType_ToBlack;
	m_aFadeViewPort[ a_iViewport ].m_fDuration = a_fDuration;

	TFLOAT fFade                           = GetFade( a_iViewport );
	m_aFadeViewPort[ a_iViewport ].m_fTime = fFade * a_fDuration;

	SetFade( fFade, a_iViewport );
	EnableFade( TTRUE, a_iViewport );
}

// $deBlob: FUNCTION 005c5ae0
void AGameStateController::FadeFromBlack( TFLOAT a_fDuration, TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	m_aFadeViewPort[ a_iViewport ].m_eFadeType = FadeType_FromBlack;
	m_aFadeViewPort[ a_iViewport ].m_fDuration = a_fDuration;
	m_aFadeViewPort[ a_iViewport ].m_fTime     = 0.0f;

	SetFade( 1.0f, a_iViewport );
	EnableFade( TTRUE, a_iViewport );
}

// $deBlob: FUNCTION 005c5b60
void AGameStateController::SetFadeColour( TUINT32 a_uiColour, TINT a_iViewport )
{
	// NOTE: TNULL until Create's fade rectangles are ported (AXYZViewportManager); retail always has them
	if ( m_aFadeViewPort[ a_iViewport ].m_pFade == TNULL )
		return;

	TSTATICCAST( T2GUIElement, m_aFadeViewPort[ a_iViewport ].m_pFade )->SetColour( a_uiColour );
}

// $deBlob: FUNCTION 005c5b80
TBOOL AGameStateController::IsFading( TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	FadeViewport& rFade = m_aFadeViewPort[ a_iViewport ];
	return rFade.m_bEnabled && 0.0f < rFade.m_fDuration && rFade.m_fTime <= rFade.m_fDuration;
}

// $deBlob: FUNCTION 005c5c00
TBOOL AGameStateController::IsFadingToBlack( TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	return m_aFadeViewPort[ a_iViewport ].m_eFadeType == FadeType_ToBlack && IsFading( a_iViewport );
}

// $deBlob: FUNCTION 005c5c70
TBOOL AGameStateController::IsFadingFromBlack( TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	return m_aFadeViewPort[ a_iViewport ].m_eFadeType == FadeType_FromBlack && IsFading( a_iViewport );
}

// $deBlob: FUNCTION 005c5ce0
void AGameStateController::StopFade( TINT a_iViewport )
{
	TASSERT( a_iViewport >= 0 && a_iViewport < NUM_FADEVIEWPORT );

	m_aFadeViewPort[ a_iViewport ].m_eFadeType = FadeType_None;
	m_aFadeViewPort[ a_iViewport ].m_fDuration = 0.0f;
	m_aFadeViewPort[ a_iViewport ].m_fTime     = 0.0f;
}

// Deletes the fade rectangles and the widescreen bars
// $deBlob: FUNCTION 005c5e50
void AGameStateController::UnloadGUI()
{
	for ( TINT i = NUM_FADEVIEWPORT; i != 0; )
	{
		i--;
		T2GUIRectangle* pFade = TSTATICCAST( T2GUIRectangle, m_aFadeViewPort[ i ].m_pFade );

		if ( pFade != TNULL )
		{
			pFade->RemoveSelf();
			delete pFade;
			m_aFadeViewPort[ i ].m_pFade = TNULL;
		}
	}

	T2GUIRectangle* pBottom = TSTATICCAST( T2GUIRectangle, m_pWidescreenBottom );

	if ( pBottom != TNULL )
	{
		pBottom->RemoveSelf();
		delete pBottom;
		m_pWidescreenBottom = TNULL;
	}

	T2GUIRectangle* pTop = TSTATICCAST( T2GUIRectangle, m_pWidescreenTop );

	if ( pTop != TNULL )
	{
		pTop->RemoveSelf();
		delete pTop;
		m_pWidescreenTop = TNULL;
	}
}

// $deBlob: FUNCTION 005c5f30
void AGameStateController::ResetFades()
{
	for ( TINT i = NUM_FADEVIEWPORT; i != 0; )
	{
		i--;
		// NOTE: TNULL until Create's fade rectangles are ported (AXYZViewportManager)
		if ( m_aFadeViewPort[ i ].m_pFade != TNULL )
			TSTATICCAST( T2GUIElement, m_aFadeViewPort[ i ].m_pFade )->SetAlpha( 0.0f );
		m_afFadeAlpha[ i ] = 0.0f;

		m_aFadeViewPort[ i ].m_fTime     = 0.0f;
		m_aFadeViewPort[ i ].m_fDuration = 0.0f;
		m_aFadeViewPort[ i ].m_eFadeType = FadeType_None;
		m_aFadeViewPort[ i ].m_bEnabled  = TFALSE;
	}
}

// $deBlob: FUNCTION 005c6120
void AGameStateController::RenderFades()
{
	AGameState* pState = GetCurrentGameState();

	if ( !m_bRenderFadesWhenPaused && pState != TNULL && pState->IsA( &TGetClass( AInGameState ) ) && TSTATICCAST( AInGameState, pState )->IsPaused() )
		return;

	for ( TINT i = NUM_FADEVIEWPORT; i != 0; )
	{
		i--;
		FadeViewport& rFade = m_aFadeViewPort[ i ];

		// NOTE: retail has no TNULL check (the rectangles always exist); m_pFade stays TNULL until Create's viewport
		// part is ported
		if ( rFade.m_bEnabled && rFade.m_pFade != TNULL )
		{
			T2GUIElement* pFade = TSTATICCAST( T2GUIElement, rFade.m_pFade );
			pFade->PreRender();
			pFade->Render();
			pFade->PostRender();
		}
	}
}

// $deBlob: FUNCTION 005c6190
void AGameStateController::UpdateFade( TFLOAT a_fDeltaTime )
{
	AGameState* pState = GetCurrentGameState();

	// The fades stop while the pause menu is open
	if ( !m_bRenderFadesWhenPaused && pState != TNULL )
	{
		if ( pState->IsA( &TGetClass( AInGameState ) ) && TSTATICCAST( AInGameState, pState )->IsPaused() )
			return;
	}

	for ( TINT i = NUM_FADEVIEWPORT; i != 0; )
	{
		i--;
		FadeViewport& rFade = m_aFadeViewPort[ i ];

		if ( !rFade.m_bEnabled )
			continue;

		TBOOL bToBlack;

		if ( rFade.m_eFadeType == FadeType_ToBlack )
			bToBlack = TTRUE;
		else if ( rFade.m_eFadeType == FadeType_FromBlack )
			bToBlack = TFALSE;
		else
			continue;

		rFade.m_fTime += a_fDeltaTime;

		if ( rFade.m_fTime <= rFade.m_fDuration )
		{
			TFLOAT fFade = TMath::Min( rFade.m_fTime / rFade.m_fDuration, 1.0f );

			if ( !bToBlack )
				fFade = 1.0f - fFade;

			SetFade( fFade, i );
			EnableFade( TTRUE, i );
		}
		else
		{
			GetCurrentGameState()->OnFadeFinished( i, !bToBlack );
			SetFade( bToBlack ? 1.0f : 0.0f, i );
			rFade.m_eFadeType = FadeType_None;
		}
	}
}

// $deBlob: FUNCTION 005c5d50
void AGameStateController::SetWidescreen( TBOOL a_bOn, TFLOAT a_fDuration )
{
	T2GUIRectangle* pTop    = TSTATICCAST( T2GUIRectangle, m_pWidescreenTop );
	T2GUIRectangle* pBottom = TSTATICCAST( T2GUIRectangle, m_pWidescreenBottom );

	if ( a_fDuration == 0.0f )
	{
		if ( !a_bOn )
		{
			pTop->SetVisible( TFALSE );
			pBottom->SetVisible( TFALSE );
		}
		else
		{
			pTop->SetHeight( m_fWidescreenTopHeight );
			pTop->SetVisible( TTRUE );
			pBottom->SetHeight( m_fWidescreenBottomHeight );
			pBottom->SetVisible( TTRUE );
		}
	}
	else if ( !a_bOn )
	{
		// Shrinks from (almost) the full height
		pTop->SetHeight( m_fWidescreenTopHeight - 1.0f );
		pTop->SetVisible( TTRUE );
		pBottom->SetHeight( m_fWidescreenBottomHeight - 1.0f );
		pBottom->SetVisible( TTRUE );
	}
	else
	{
		// Grows from 1
		pTop->SetHeight( 1.0f );
		pTop->SetVisible( TTRUE );
		pBottom->SetHeight( 1.0f );
		pBottom->SetVisible( TTRUE );
	}

	m_bWidescreenOn       = a_bOn;
	m_fWidescreenDuration = a_fDuration;
	m_fWidescreenTime     = 0.0f;
}

// $deBlob: FUNCTION 005c62c0
void AGameStateController::UpdateWidescreen( TFLOAT a_fDeltaTime )
{
	if ( m_fWidescreenTime == m_fWidescreenDuration )
	{
		m_fWidescreenTime     = 0.0f;
		m_fWidescreenDuration = 0.0f;
	}

	if ( 0.0f < m_fWidescreenDuration )
	{
		T2GUIRectangle* pTop    = TSTATICCAST( T2GUIRectangle, m_pWidescreenTop );
		T2GUIRectangle* pBottom = TSTATICCAST( T2GUIRectangle, m_pWidescreenBottom );

		m_fWidescreenTime += a_fDeltaTime;

		if ( !m_bWidescreenOn )
		{
			// Slid out: hidden
			if ( m_fWidescreenDuration <= m_fWidescreenTime )
			{
				pTop->SetVisible( TFALSE );
				pBottom->SetVisible( TFALSE );
				m_fWidescreenTime     = 0.0f;
				m_fWidescreenDuration = 0.0f;
				return;
			}
		}
		else if ( m_fWidescreenDuration < m_fWidescreenTime )
		{
			m_fWidescreenTime = m_fWidescreenDuration;
		}

		TFLOAT fProgress = m_fWidescreenTime / m_fWidescreenDuration;
		TFLOAT fBottomHeight;

		if ( !m_bWidescreenOn )
		{
			pTop->SetHeight( m_fWidescreenTopHeight * ( 1.0f - fProgress ) );
			fBottomHeight = m_fWidescreenBottomHeight * ( 1.0f - fProgress );
		}
		else
		{
			pTop->SetHeight( m_fWidescreenTopHeight * fProgress );
			fBottomHeight = m_fWidescreenBottomHeight * fProgress;
		}

		pBottom->SetHeight( fBottomHeight );
	}
}
