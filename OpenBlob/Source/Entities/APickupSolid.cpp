#include "pch.h"
#include "APickupSolid.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APickupSolid ) == 0x16C );

// $deBlob: FUNCTION 007e4890
// $deBlob: FUNCTION 007e48a0
// $deBlob: FUNCTION 007e48b0
// $deBlob: FUNCTION 007e48c0
// $deBlob: FUNCTION 007e48d0
// $deBlob: FUNCTION 00541a50
// $deBlob: FUNCTION 0085a150
TDEFINE_CLASS_NORUNTIME( APickupSolid );

// $deBlob: FUNCTION 007e48e0
APickupSolid::APickupSolid( TUINT8 a_uiPickupType, TINT a_iUnk2, TFLOAT a_fUnk3 )
    : APickup( a_uiPickupType, a_iUnk2, a_fUnk3, TFALSE )
{
	m_uiPickupFlags |= SOLIDFLAG_Opaque;
	m_fDyingTime  = 3.0f;
	m_fDyingTimer = 0.0f;
	m_fFlashTimer = 0.0f;
	m_uiFlashes   = 5;
}

// $deBlob: FUNCTION 007e4b60
// $deBlob: FUNCTION 007e4b75
// $deBlob: FUNCTION 007e4b80
APickupSolid::~APickupSolid()
{
}

// $deBlob: FUNCTION 007e4950
void APickupSolid::Tick( TFLOAT a_fDeltaTime )
{
	TASSERT( IsVisible() );
	APickup::Tick( a_fDeltaTime );

	if ( m_uiPickupFlags & SOLIDFLAG_Dying )
	{
		m_fDyingTimer -= a_fDeltaTime;

		if ( m_fDyingTimer <= 0.0f )
		{
			if ( m_eState == PICKUPSTATE_Idle )
				m_eState = PICKUPSTATE_Fading;
			else if ( m_eState == PICKUPSTATE_Fading )
				UpdateFlashes( a_fDeltaTime );
		}
	}
}

// $deBlob: FUNCTION 007e4a00
void APickupSolid::StartDying()
{
	m_uiPickupFlags |= SOLIDFLAG_Dying;

	TFLOAT fDyingTime = GetDyingTime();
	m_fDyingTime      = fDyingTime;
	m_fDyingTimer     = fDyingTime;
	m_uiFlashes       = TUINT8( GetNumFlashes() );
	m_fFlashTimer     = 0.1f;
}

// $deBlob: FUNCTION 007e4a50
void APickupSolid::Show()
{
	m_fDyingTime    = 3.0f;
	m_fDyingTimer   = 0.0f;
	m_uiPickupFlags = ( m_uiPickupFlags & ~SOLIDFLAG_Dying ) | SOLIDFLAG_Opaque;
	m_fFlashTimer   = 0.0f;
	m_uiFlashes     = 5;
	SetAlpha( 1.0f );

	APickup::Show();
}

// $deBlob: FUNCTION 007e4aa0
void APickupSolid::UpdateFlashes( TFLOAT a_fDeltaTime )
{
	if ( m_uiFlashes != 0 )
	{
		m_fFlashTimer -= a_fDeltaTime;

		if ( !( m_uiPickupFlags & SOLIDFLAG_Opaque ) )
		{
			SetAlpha( 0.1f );

			if ( m_fFlashTimer <= 0.0f )
			{
				m_uiPickupFlags |= SOLIDFLAG_Opaque;
				m_uiFlashes--;
				m_fFlashTimer = 0.1f;
			}
		}
		else
		{
			SetAlpha( 1.0f );

			if ( m_fFlashTimer <= 0.0f )
			{
				m_uiPickupFlags &= ~SOLIDFLAG_Opaque;
				m_fFlashTimer = 0.1f;
			}
		}
	}
	else
	{
		SetVisible( TFALSE );
		SetHash( TFALSE );
		RemoveFromPhysics();
		Hide();
	}
}

// $deBlob: FUNCTION 007e4c10
void APickupSolid::SetAlpha( TFLOAT a_fAlpha )
{
	m_fAlpha = a_fAlpha;
}

// $deBlob: FUNCTION 007e4c30
TVector4 APickupSolid::GetRandomVelocity( TFLOAT a_fScale, TINT a_iUnused1, TFLOAT a_fZ, TINT a_iUnused2, TFLOAT a_fSpeed, TFLOAT a_fUp, TINT a_iMinDivisor, TINT a_iMaxDivisor )
{
	TRandom& rRandom = ARandom::GetSingleton()->GetRandom();

	TVector4 vVelocity( 0.0f, 0.0f, 0.0f, 1.0f );
	vVelocity.x = rRandom.GetFloatMinMax( a_fSpeed / TFLOAT( a_iMaxDivisor ), a_fSpeed / TFLOAT( a_iMinDivisor ) ) * a_fScale;
	vVelocity.y = rRandom.GetFloatMinMax( -a_fUp, -( a_fUp / TFLOAT( a_iMinDivisor ) ) );

	// NOTE: retail draws a third number and doesn't use it
	rRandom.RandRaw();

	vVelocity.z = a_fZ;
	return vVelocity;
}
