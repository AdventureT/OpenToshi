#include "pch.h"
#include "ABlobChallengeDifficultyHud.h"
#include "Challenges/AChallengeManager.h"
#include "GameInterface/APlayerManager.h"
#include "Render/ARenderer.h"
#include "Entities/ABaseEntity.h"

#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The squared XZ distance beyond which the stars are hidden
static const TFLOAT s_fMaxDistanceSq = 2500.0f;

// $deBlob: FUNCTION 0064dc90
// $deBlob: FUNCTION 0064dca0
// $deBlob: FUNCTION 0064dd30
// $deBlob: FUNCTION 0064ddc0
// $deBlob: FUNCTION 0064ddd0
// $deBlob: FUNCTION 0053e010
// $deBlob: FUNCTION 00858b80
TDEFINE_CLASS( ABlobChallengeDifficultyHud );

// $deBlob: FUNCTION 0064e0c0
ABlobChallengeDifficultyHud::ABlobChallengeDifficultyHud()
    : m_vPosition( TVector4::VEC_ZERO )
{
}

// $deBlob: FUNCTION 0064e0f0
// $deBlob: FUNCTION 0064e0f5
// $deBlob: FUNCTION 0064e100
ABlobChallengeDifficultyHud::~ABlobChallengeDifficultyHud()
{
}

// $deBlob: FUNCTION 0064de40
void ABlobChallengeDifficultyHud::Tick( TFLOAT a_fDeltaTime )
{
	// NOTE: retail doesn't call TXUIScene::Tick
	AChallenge* pChallenge = AChallengeManager::GetSingleton()->FindNearestChallengeToBlob( TTRUE );

	if ( pChallenge == TNULL )
	{
		SetOpacity( 0.0f );
		SetVisible( TFALSE );
		return;
	}

	TUINT32 uiFlags = pChallenge->GetData()->m_uiFlags;
	TINT    iStars;

	if ( uiFlags & 2 )
		iStars = 0;
	else if ( uiFlags & 4 )
		iStars = 1;
	else
		iStars = ( TUINT8( uiFlags ) >> 2 ) & 2;

	PlayStarsAnimation( iStars );

	TVector4 vBlobPos = APlayerManager::GetSingleton()->GetPlayerBlob( 0 )->GetMatrix().GetTranslation();
	m_vPosition       = pChallenge->GetData()->m_pMatrix->GetTranslation();

	TVector4 vToChallenge( m_vPosition.x - vBlobPos.x, m_vPosition.y - vBlobPos.y, m_vPosition.z - vBlobPos.z, m_vPosition.w );
	TFLOAT   fDistanceSq = vToChallenge.MagnitudeSqXZ();

	static TFLOAT s_fInvMaxDistanceSq = 1.0f / s_fMaxDistanceSq;

	if ( s_fMaxDistanceSq < fDistanceSq )
	{
		SetOpacity( 0.0f );
		SetVisible( TFALSE );
		return;
	}

	SetVisible( TTRUE );

	TFLOAT fTarget = TMath::Min( 1.0f - s_fInvMaxDistanceSq * fDistanceSq, 0.7f );
	SetOpacity( TMath::LERP( GetFlags_Opacity(), fTarget, a_fDeltaTime * 2.0f ) );

	m_vPosition.y -= 15.0f;
	ARenderer::GetSingleton()->ProjectToCanvas( m_vPosition, TNULL );

	TFLOAT fX = m_vPosition.x - GetWidth() * 0.5f;
	TFLOAT fY = m_vPosition.y - GetHeight() * 0.5f;
	SetPosition( fX, fY );
}

// $deBlob: FUNCTION 0064dde0
void ABlobChallengeDifficultyHud::PlayStarsAnimation( TINT a_iStars )
{
	switch ( a_iStars )
	{
		case 2:
			m_pVisual->PlayAnimation( L"Show3Stars", L"EndShow3Stars" );
			break;
		case 1:
			m_pVisual->PlayAnimation( L"Show2Stars", L"EndShow2Stars" );
			break;
		case 0:
			m_pVisual->PlayAnimation( L"Show1Star", L"EndShow1Star" );
			break;
	}
}
