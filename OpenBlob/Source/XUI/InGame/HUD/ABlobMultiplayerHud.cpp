#include "pch.h"
#include "ABlobMultiplayerHud.h"
#include "GameInterface/APlayerManager.h"
#include "Entities/ABaseEntity.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIRenderer.h>
#include <XUI/TXUIVisual.h>
#include <Render/TViewport.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The time the player marker stays at the match start
static TFLOAT s_fPlayerMarkerTime = 3.0f;

// $deBlob: FUNCTION 00657be0
// $deBlob: FUNCTION 00657bf0
// $deBlob: FUNCTION 00657c90
// $deBlob: FUNCTION 00657d30
// $deBlob: FUNCTION 00657d40
// $deBlob: FUNCTION 0053e440
// $deBlob: FUNCTION 00858c80
TDEFINE_CLASS( ABlobMultiPlayerHud );

// $deBlob: FUNCTION 00658150
ABlobMultiPlayerHud::ABlobMultiPlayerHud()
{
	m_uiPlayer          = 0;
	m_pThreat           = TNULL;
	m_pPlayerMarker     = TNULL;
	m_fPlayerMarkerTime = 0.0f;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves these to OnCreated
	m_bThreatened = TFALSE;
	m_fThreatTime = 0.0f;
#endif
}

// $deBlob: FUNCTION 00658190
// $deBlob: FUNCTION 006581a2
// $deBlob: FUNCTION 006581b0
ABlobMultiPlayerHud::~ABlobMultiPlayerHud()
{
}

// $deBlob: FUNCTION 00657d50
void ABlobMultiPlayerHud::OnCreated()
{
	m_bThreatened   = TFALSE;
	m_fThreatTime   = 0.0f;
	m_pThreat       = FindChildXUIControl( L"Threat" );
	m_pPlayerMarker = FindChildXUIControl( L"PlayerMarker" );
}

// $deBlob: FUNCTION 00657d90
void ABlobMultiPlayerHud::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( m_uiPlayer );

	if ( !APlayerManager::GetSingleton()->IsBlobLockedOn( pBlob ) )
	{
		if ( m_bThreatened )
		{
			m_bThreatened = TFALSE;
			m_pThreat->GetVisual()->PlayAnimation( L"StopTargeted", L"EndStopTargeted" );
			m_fThreatTime = 1.0f;
		}
	}
	else if ( !m_bThreatened )
	{
		m_bThreatened = TTRUE;
		m_pThreat->GetVisual()->PlayAnimation( L"Targeted", L"EndTargeted" );
	}

	if ( 0.0f < m_fThreatTime )
		m_fThreatTime -= a_fDeltaTime;

	if ( m_bThreatened || 0.0f < m_fThreatTime || m_pPlayerMarker->GetVisual()->GetTimelineControl()->IsPlaying() || 0.0f < m_fPlayerMarkerTime )
	{
		// The blob's position on the canvas, relative to the parent of the threat and the marker
		TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( APlayerManager::GetSingleton()->GetBlobCamera( pBlob ) );
		TVector4   vPosition = pBlob->GetBasePosition();
		ARenderer::GetSingleton()->ProjectToScreen( vPosition, pViewport );
		vPosition.x = pViewport->GetX() + vPosition.x;
		vPosition.y = pViewport->GetY() + vPosition.y;
		ARenderer::GetSingleton()->ScreenToCanvas( vPosition.x, vPosition.y );

		const TVector2& rParentPos = m_pThreat->GetParent()->GetTransform().GetPos();
		vPosition.x -= rParentPos.x;
		vPosition.y -= rParentPos.y;

		if ( m_pThreat->IsVisible() )
		{
			TFLOAT fX = vPosition.x - m_pThreat->GetWidth() * 0.5f;
			TFLOAT fY = vPosition.y - m_pThreat->GetHeight() * 0.5f;
			m_pThreat->SetPosition( fX, fY );
		}

		TASSERT( m_pPlayerMarker->GetParent() == m_pThreat->GetParent() );

		if ( 0.0f < m_fPlayerMarkerTime || m_pPlayerMarker->GetVisual()->GetTimelineControl()->IsPlaying() )
		{
			TFLOAT fX = vPosition.x - m_pPlayerMarker->GetWidth() * 0.5f;
			TFLOAT fY = vPosition.y - m_pPlayerMarker->GetHeight();
			m_pPlayerMarker->SetPosition( fX, fY );
		}
	}

	if ( 0.0f < m_fPlayerMarkerTime )
	{
		m_fPlayerMarkerTime -= a_fDeltaTime;

		if ( m_fPlayerMarkerTime < 0.0f )
			m_pPlayerMarker->GetVisual()->PlayAnimation( L"Hide", L"EndHide" );
	}

	TXUIScene::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 00658090
void ABlobMultiPlayerHud::Render()
{
	// The panel is rendered inside its own viewport
	TFLOAT fWidth;
	TFLOAT fHeight;
	GetDimensions( fWidth, fHeight );

	fWidth *= 0.5f;
	fHeight *= 0.5f;

	TXUIRenderer* pRenderer = TXUI::GetSingleton()->GetRenderer();
	pRenderer->SetViewportRect( -fWidth, fHeight, fWidth, -fHeight );
	TXUIControl::Render();
	pRenderer->ResetViewportRect();
}

// $deBlob: FUNCTION 00658120
void ABlobMultiPlayerHud::ShowPlayerMarker()
{
	TXUIControl* pPlayerMarker = m_pPlayerMarker;
	m_fPlayerMarkerTime        = s_fPlayerMarkerTime;
	pPlayerMarker->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
}
