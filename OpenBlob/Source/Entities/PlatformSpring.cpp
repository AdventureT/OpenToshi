#include "pch.h"
#include "PlatformSpring.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( PlatformSpring ) == 0x2C );

// $deBlob: FUNCTION 00793b80
PlatformSpring::PlatformSpring()
{
	m_fAccelDown = 0.0f;
	m_fAccelUp   = 0.0f;
	m_fDistance  = 0.0f;
	m_fAmplitude = 1.0f;
	m_fFade      = 1.0f;
	m_fFadeSpeed = 0.0f;
	m_fVelocity  = 0.0f;
	m_fOffset    = 0.0f;

	TFLOAT fRandom = ARandom::GetSingleton()->GetRandom().GetFloat();

	m_uiFlags = FLAG_Off;
	m_fWait   = 0.0f;
	m_fPhase  = fRandom * TMath::TWO_PI;
}

// $deBlob: FUNCTION 00793c20
void PlatformSpring::Init( TFLOAT a_fAccelDown, TFLOAT a_fAccelUp, TFLOAT a_fDistance, TFLOAT a_fFadeSpeed )
{
	TUINT8 uiFlags = m_uiFlags;

	m_fAccelDown = a_fAccelDown;
	m_fAccelUp   = a_fAccelUp;
	m_fDistance  = a_fDistance;
	m_fFadeSpeed = a_fFadeSpeed;

	if ( !( uiFlags & FLAG_Rising ) )
	{
		if ( uiFlags & FLAG_Off )
		{
			m_fVelocity = 0.0f;
			m_fOffset   = 0.0f;
		}
	}
	else
	{
		m_uiFlags = uiFlags & ~FLAG_Rising;
	}

	m_fWait    = 0.05f;
	m_uiFlags = ( m_uiFlags & 0xBE ) | ( FLAG_Unk4 | FLAG_Unk20 );
}

// $deBlob: FUNCTION 00793c80
TFLOAT PlatformSpring::Tick( TFLOAT a_fDeltaTime )
{
	m_fWait = TMath::Max( m_fWait - a_fDeltaTime, 0.0f );

	TUINT8 uiFlags = m_uiFlags;

	if ( ( uiFlags & ( FLAG_Oscillating | FLAG_Fading ) ) == 0 )
	{
		if ( uiFlags & FLAG_Off )
			return 0.0f;

		TFLOAT fDistance = m_fDistance;

		if ( fDistance <= 0.01f )
		{
			m_uiFlags = ( uiFlags & ~FLAG_Rising ) | FLAG_Off;
			return 0.0f;
		}

		if ( ( uiFlags & ( FLAG_Rising | FLAG_Unk4 ) ) || 0.0f < m_fWait )
		{
			uiFlags &= ~FLAG_Unk4;
		}
		else
		{
			uiFlags |= FLAG_Rising;
			m_fVelocity = 0.0f;
		}

		m_uiFlags = uiFlags;

		if ( !( uiFlags & FLAG_Rising ) )
		{
			TFLOAT fVelocity = m_fVelocity;

			if ( 9.8f <= fVelocity )
				m_fVelocity = 9.8f;
			else
				m_fVelocity = TMath::Min( m_fAccelDown * a_fDeltaTime, 9.8f - fVelocity ) + fVelocity;

			m_fOffset = a_fDeltaTime * m_fVelocity + m_fOffset;

			if ( fDistance < m_fOffset )
			{
				m_fOffset   = fDistance;
				m_uiFlags   = uiFlags | FLAG_Rising;
				m_fVelocity = 0.0f;
				return m_fOffset;
			}
		}
		else
		{
			TFLOAT fVelocity = m_fVelocity;

			if ( 4.0f <= fVelocity )
				m_fVelocity = 4.0f;
			else
				m_fVelocity = TMath::Min( m_fAccelUp * a_fDeltaTime, 4.0f - fVelocity ) + fVelocity;

			m_fOffset = m_fOffset - a_fDeltaTime * m_fVelocity;

			if ( m_fOffset < 0.0f )
			{
				m_fOffset   = 0.0f;
				m_uiFlags   = uiFlags & ~FLAG_Rising;
				m_fVelocity = 0.0f;

				if ( ( uiFlags & FLAG_Unk20 ) && ( uiFlags & FLAG_BobOnEnd ) )
				{
					m_uiFlags = ( uiFlags & ~FLAG_Rising ) | FLAG_Fading;
					m_fFade   = 1.0f;
					m_fPhase  = 0.0f;
					return m_fOffset;
				}
			}
		}

		return m_fOffset;
	}

	if ( uiFlags & FLAG_Fading )
		m_fFade = m_fFade - m_fFadeSpeed * a_fDeltaTime;

	m_fPhase = m_fAccelDown * a_fDeltaTime + m_fPhase;
	TFLOAT fSin = TFLOAT( sin( m_fPhase * TMath::TWO_PI ) );

	if ( m_fFade <= 0.0f )
	{
		m_fFade   = 0.0f;
		m_uiFlags = uiFlags & ~( FLAG_Unk20 | FLAG_Fading );
	}

	TFLOAT fOffset = m_fDistance * fSin;

	if ( m_uiFlags & FLAG_Fading )
		fOffset *= 0.3f;

	m_fOffset = fOffset * m_fFade * m_fAmplitude;
	return m_fOffset;
}
