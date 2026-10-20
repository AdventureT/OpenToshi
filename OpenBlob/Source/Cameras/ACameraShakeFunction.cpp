#include "pch.h"
#include "ACameraShakeFunction.h"
#include "ACurrentLevelInformation.h"
#include "ACamera.h"
#include "ARandom.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameInterface/APlayerManager.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ACameraShakeFunction::ShakeData* ACameraShakeFunction::ms_pShakeTypes;

// $deBlob: FUNCTION 0058f500
void ACameraShakeFunction::LoadShakeTypes()
{
	TTRB* pTRB     = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_CameraShakeCommon );
	ms_pShakeTypes = TSTATICCAST( ShakeData, pTRB->GetSymbolAddress( "CommonCameraShake" ) );
}

// $deBlob: FUNCTION 0058f520
void ACameraShakeFunction::UnloadShakeTypes()
{
	ms_pShakeTypes = TNULL;
}

// $deBlob: FUNCTION 0058f530
ACameraShakeFunction::ShakeData* ACameraShakeFunction::GetShakeType( TINT a_etype )
{
	TASSERT( a_etype >= 0 && a_etype < SHAKE_CUSTOM_TOTAL );
	return &ms_pShakeTypes[ a_etype ];
}

// $deBlob: FUNCTION 0058f580
void ACameraShakeFunction::LoadLevelShakes()
{
	m_pExtraShakes = TNULL;
	TTRB* pTRB     = ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_CameraShake );
	m_pExtraShakes = TSTATICCAST( ShakeArray, pTRB->GetSymbolAddress( "CameraShakeHeader" ) );
	ResetShakes();
}

// $deBlob: FUNCTION 0058f5d0
void ACameraShakeFunction::ResetShakes()
{
	m_iNumShakes = 0;
	m_iNextID    = 0;
	m_bShaking   = TFALSE;

	if ( m_pExtraShakes == TNULL )
		return;

	for ( TINT i = m_pExtraShakes->m_iNumShakes; i != 0; )
	{
		i--;
		// Bit 0: enabled from the start
		if ( m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag & 1 )
		{
			m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag |= SF_ENABLED;
			m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag &= ~SF_UNK40;

			if ( m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag & ( SF_LOOPING_EFFECT_PULSE | SF_LOOPING_EFFECT_CONSTANT ) )
				m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag |= SF_UNK40;
		}
		else
		{
			m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag &= ~SF_ENABLED;
			m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag &= ~SF_UNK40;
		}

		m_pExtraShakes->m_pShakes[ i ].m_fCurrentStrength = 0.0f;
		m_pExtraShakes->m_pShakes[ i ].m_fElapsedTime     = 0.0f;
	}
}

// $deBlob: FUNCTION 0058fbd0
TINT ACameraShakeFunction::AddShake( TINT a_eType, const Toshi::TVector4& a_rPosition, TFLOAT a_fStrength, TINT a_iPlayer )
{
	TASSERT( a_eType >= 0 && a_eType < SHAKE_CUSTOM_TOTAL );

	if ( m_iNumShakes >= MAX_SHAKES )
		return -1;

	ShakeData& rShake = m_aShakes[ m_iNumShakes++ ];
	rShake.m_iID      = m_iNextID++;

	// NOTE: the copy of the preset overwrites the ID just given (retail returns the preset's m_iID)
	rShake = ms_pShakeTypes[ a_eType ];

	TUINT32 uiFlags           = rShake.m_iStatusFlag;
	rShake.m_fCurrentStrength = 0.0f;
	rShake.m_fElapsedTime     = 0.0f;
	rShake.m_iStatusFlag      = uiFlags | SF_ENABLED;
	rShake.m_iPlayerEffected  = a_iPlayer;
	rShake.m_vPosition        = a_rPosition;
	rShake.m_fShake_Strength  = rShake.m_fShake_Strength * a_fStrength;

	if ( uiFlags & ( SF_LOOPING_EFFECT_PULSE | SF_LOOPING_EFFECT_CONSTANT ) )
		rShake.m_iStatusFlag = uiFlags | SF_ENABLED | SF_UNK40;

	return rShake.m_iID;
}

// $deBlob: FUNCTION 0058ebc0
void ACameraShakeFunction::UpdateShake( TFLOAT a_fDeltaTime, ShakeData& a_ShakeData )
{
	TASSERT( a_ShakeData.m_iStatusFlag & SF_ENABLED );

	TFLOAT fTime               = a_ShakeData.m_fElapsedTime + a_fDeltaTime;
	TFLOAT fDelay              = a_ShakeData.m_fDelay;
	a_ShakeData.m_fElapsedTime = fTime;

	if ( fTime < fDelay )
		return;

	TUINT32 uiFlags = a_ShakeData.m_iStatusFlag;
	TFLOAT  fLength = fDelay + a_ShakeData.m_fStart_Time + a_ShakeData.m_fHold_Time + a_ShakeData.m_fEnd_Time + a_ShakeData.m_fUnk30;

	if ( ( uiFlags & SF_UNK40 ) == 0 )
	{
		// Over
		if ( fLength < fTime )
		{
			a_ShakeData.m_iStatusFlag = uiFlags & ~SF_ENABLED;
			return;
		}
	}
	else if ( ( uiFlags & SF_LOOPING_EFFECT_PULSE ) != 0 && 0.0f < fTime - fLength )
	{
		// Start the pulse again
		a_ShakeData.m_fElapsedTime = fDelay + ( fTime - fLength );
	}

	TFLOAT fTimeBalance = a_ShakeData.m_fElapsedTime - fDelay;
	TASSERT( fTimeBalance >= 0.0f );

	if ( fTimeBalance < a_ShakeData.m_fStart_Time )
	{
		// The ramp up
		TASSERT( a_ShakeData.m_fStart_Time > 0.0f );
		TFLOAT fRatio = 1.0f - ( a_ShakeData.m_fStart_Time - fTimeBalance ) / a_ShakeData.m_fStart_Time;
		TASSERT( fRatio >= 0.0f && fRatio <= 1.0f );

		a_ShakeData.m_fCurrentStrength = a_ShakeData.m_fShake_Strength * fRatio;

		if ( a_ShakeData.m_iStatusFlag & SF_SQUARE_START )
			a_ShakeData.m_fCurrentStrength = a_ShakeData.m_fCurrentStrength * fRatio;

		TASSERT( a_ShakeData.m_fCurrentStrength <= a_ShakeData.m_fShake_Strength && a_ShakeData.m_fCurrentStrength >= 0.0f );
	}
	else
	{
		if ( ( uiFlags & SF_UNK40 ) == 0 || ( a_ShakeData.m_iStatusFlag & SF_LOOPING_EFFECT_CONSTANT ) == 0 )
		{
			fTimeBalance -= a_ShakeData.m_fStart_Time;
		}
		else
		{
			// Constant: stay at the start of the hold
			a_ShakeData.m_fElapsedTime = a_ShakeData.m_fDelay + a_ShakeData.m_fStart_Time;
			fTimeBalance               = 0.0f;
		}

		TASSERT( fTimeBalance >= 0.0f );

		if ( fTimeBalance < a_ShakeData.m_fHold_Time )
		{
			a_ShakeData.m_fCurrentStrength = a_ShakeData.m_fShake_Strength;
		}
		else
		{
			fTimeBalance -= a_ShakeData.m_fHold_Time;

			if ( a_ShakeData.m_fEnd_Time <= fTimeBalance )
			{
				a_ShakeData.m_fCurrentStrength = 0.0f;
			}
			else
			{
				// The ramp down
				TASSERT( a_ShakeData.m_fEnd_Time > 0.0f );
				TFLOAT fRatio = ( a_ShakeData.m_fEnd_Time - fTimeBalance ) / a_ShakeData.m_fEnd_Time;
				TASSERT( fRatio >= 0.0f && fRatio <= 1.0f );

				a_ShakeData.m_fCurrentStrength = a_ShakeData.m_fShake_Strength * fRatio;

				if ( a_ShakeData.m_iStatusFlag & SF_SQUARE_END )
					a_ShakeData.m_fCurrentStrength = a_ShakeData.m_fCurrentStrength * fRatio;

				TASSERT( a_ShakeData.m_fCurrentStrength <= a_ShakeData.m_fShake_Strength && a_ShakeData.m_fCurrentStrength >= 0.0f );
			}
		}
	}

	if ( 0.0f < a_ShakeData.m_fCurrentStrength )
		m_bShaking = TTRUE;
}

// $deBlob: FUNCTION 0058f690
void ACameraShakeFunction::Update( TFLOAT a_fDeltaTime )
{
	m_bShaking = TFALSE;

	// Drop the finished shakes
	for ( TINT i = 0; i < m_iNumShakes; )
	{
		if ( m_aShakes[ i ].m_iStatusFlag & SF_ENABLED )
		{
			i++;
			continue;
		}

		for ( TINT j = i; j < m_iNumShakes - 1; j++ )
			m_aShakes[ j ] = m_aShakes[ j + 1 ];

		m_iNumShakes--;
	}

	for ( TINT i = m_iNumShakes - 1; i >= 0; i-- )
		UpdateShake( a_fDeltaTime, m_aShakes[ i ] );

	if ( m_pExtraShakes )
	{
		for ( TINT i = m_pExtraShakes->m_iNumShakes; i != 0; )
		{
			i--;
			if ( m_pExtraShakes->m_pShakes[ i ].m_iStatusFlag & SF_ENABLED )
				UpdateShake( a_fDeltaTime, m_pExtraShakes->m_pShakes[ i ] );
		}
	}
}

// $deBlob: FUNCTION 0058ef80
TFLOAT ACameraShakeFunction::GetShakeStrength( const ShakeData& a_ShakeData, ACamera* a_pCamera, TFLOAT a_fMax )
{
	if ( a_ShakeData.m_fCurrentStrength <= a_fMax || ( a_ShakeData.m_iStatusFlag & SF_ENABLED ) == 0 )
		return 0.0f;

	if ( a_ShakeData.m_iPlayerEffected >= 0 )
	{
		TASSERT( a_ShakeData.m_iPlayerEffected >= 0 && a_ShakeData.m_iPlayerEffected < APlayerManager::MAX_PLAYERS );

		if ( APlayerManager::GetSingleton()->GetPlayerCamera( TUINT8( a_ShakeData.m_iPlayerEffected ) ) != a_pCamera )
			return 0.0f;
	}

	TVector4 vCamPos  = a_pCamera->GetMatrix().GetTranslation();
	TVector4 vLookDir = a_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );

	TFLOAT fStrength = 0.0f;

	if ( a_ShakeData.m_fRadius == -1.0f )
	{
		// Everywhere
		fStrength = a_ShakeData.m_fCurrentStrength;

		if ( 0.0f < a_ShakeData.m_fLookAtDirEffect && 0.0f < fStrength )
		{
			TASSERT( a_ShakeData.m_fLookAtDirEffect <= 1.0f );

			TVector4 vToShake = a_ShakeData.m_vPosition - vCamPos;
			vToShake.w        = a_ShakeData.m_vPosition.w;
			vToShake.Normalise();

			TFLOAT fEffect = a_ShakeData.m_fLookAtDirEffect;
			TFLOAT fDot    = vLookDir.y * vToShake.y + vLookDir.x * vToShake.x + vLookDir.z * vToShake.z;
			TMath::Clip( fDot, 0.0f, 1.0f );
			fStrength = fDot * fEffect * fStrength + ( fStrength - fEffect * fStrength );
		}
	}
	else
	{
		TVector4 vToShake = a_ShakeData.m_vPosition - vCamPos;
		vToShake.w        = a_ShakeData.m_vPosition.w;

		TFLOAT              fDistSq  = 0.0f;
		ACamera::CameraType eCamType = a_pCamera->GetCameraType();
		ABaseEntity*        pBlob    = TNULL;

		if ( ( a_ShakeData.m_iStatusFlag & SF_FROM_BLOB ) && eCamType != 4 && eCamType != 2 )
		{
			TASSERT( APlayerManager::IsSingletonCreated() );
			pBlob = APlayerManager::GetSingleton()->GetBlobOfCamera( a_pCamera );
		}

		if ( pBlob != TNULL )
			fDistSq = TVector4::DistanceSq( TSTATICCAST( ABlobEntity, pBlob )->GetBodyPosition(), a_ShakeData.m_vPosition );
		else
			fDistSq = vToShake.MagnitudeSq();

		TFLOAT fRadius = a_ShakeData.m_fRadius;

		if ( fDistSq < ( a_ShakeData.m_fFallOffDistance + fRadius ) * ( a_ShakeData.m_fFallOffDistance + fRadius ) )
		{
			fStrength = a_ShakeData.m_fCurrentStrength;

			if ( fRadius * fRadius < fDistSq )
			{
				TFLOAT fDist = TMath::Sqrt( fDistSq );
				TASSERT( a_ShakeData.m_fFallOffDistance > 0.0f );
				TFLOAT fScale = 1.0f - ( fDist - a_ShakeData.m_fRadius ) / a_ShakeData.m_fFallOffDistance;
				TASSERT( fScale >= 0.0f && fScale <= 1.0f );
				fStrength = fScale * fStrength;
			}

			if ( 0.0f < a_ShakeData.m_fLookAtDirEffect && 0.0f < fStrength )
			{
				TASSERT( a_ShakeData.m_fLookAtDirEffect >= 0.0f && a_ShakeData.m_fLookAtDirEffect <= 1.0f );

				// NOTE: retail uses the direction to the shake without normalising it here
				TFLOAT fEffect = a_ShakeData.m_fLookAtDirEffect;
				TFLOAT fDot    = vToShake.x * vLookDir.x + vToShake.y * vLookDir.y + vToShake.z * vLookDir.z;
				TMath::Clip( fDot, 0.0f, 1.0f );
				fStrength = fDot * fEffect * fStrength + ( fStrength - fEffect * fStrength );
			}
		}
	}

	TASSERT( fStrength >= 0.0f && fStrength <= a_ShakeData.m_fCurrentStrength );
	return fStrength;
}

// $deBlob: FUNCTION 0058f470
ACameraShakeFunction::ShakeData* ACameraShakeFunction::FindPlacedShake( TINT a_iID )
{
	if ( m_pExtraShakes == TNULL )
		return TNULL;

	for ( TINT i = m_pExtraShakes->m_iNumShakes; i != 0; )
	{
		i--;
		if ( m_pExtraShakes->m_pShakes[ i ].m_iID == a_iID )
			return &m_pExtraShakes->m_pShakes[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 0058fb40
TINT ACameraShakeFunction::AddShake( const ShakeData& a_rShake )
{
	if ( m_iNumShakes >= MAX_SHAKES )
		return -1;

	ShakeData& rShake = m_aShakes[ m_iNumShakes++ ];
	rShake            = a_rShake;
	rShake.m_iID      = m_iNextID++;

	TUINT32 uiFlags           = rShake.m_iStatusFlag | SF_ENABLED;
	rShake.m_fElapsedTime     = 0.0f;
	rShake.m_iStatusFlag      = uiFlags;
	rShake.m_fCurrentStrength = 0.0f;

	if ( uiFlags & ( SF_LOOPING_EFFECT_PULSE | SF_LOOPING_EFFECT_CONSTANT ) )
		rShake.m_iStatusFlag = uiFlags | SF_UNK40;

	return rShake.m_iID;
}

// $deBlob: FUNCTION 0058fd10
void ACameraShakeFunction::DisableShake( TINT a_iID )
{
	for ( TINT i = m_iNumShakes; i != 0; )
	{
		i--;
		if ( m_aShakes[ i ].m_iID == a_iID )
		{
			m_aShakes[ i ].m_iStatusFlag &= ~SF_ENABLED;
			return;
		}
	}
}

// $deBlob: FUNCTION 0058fcd0
void ACameraShakeFunction::StopShake( TINT a_iID )
{
	for ( TINT i = m_iNumShakes; i != 0; )
	{
		i--;
		if ( m_aShakes[ i ].m_iID == a_iID )
		{
			m_aShakes[ i ].m_iStatusFlag &= ~SF_UNK40;
			return;
		}
	}
}

// $deBlob: FUNCTION 0058fd50
void ACameraShakeFunction::StartShake( TINT a_iID )
{
	if ( m_pExtraShakes == TNULL )
		return;

	for ( TINT i = m_pExtraShakes->m_iNumShakes; i != 0; )
	{
		i--;
		ShakeData* pShake = &m_pExtraShakes->m_pShakes[ i ];

		if ( pShake->m_iID == a_iID )
		{
			TUINT32 uiFlags = pShake->m_iStatusFlag;

			if ( ( uiFlags & SF_ENABLED ) == 0 )
			{
				pShake->m_iStatusFlag = uiFlags | SF_ENABLED;

				if ( ( uiFlags & ( SF_LOOPING_EFFECT_PULSE | SF_LOOPING_EFFECT_CONSTANT ) ) == 0 )
					uiFlags = ( uiFlags & ~SF_UNK40 ) | SF_ENABLED;
				else
					uiFlags = uiFlags | SF_ENABLED | SF_UNK40;

				pShake->m_iStatusFlag      = uiFlags;
				pShake->m_fCurrentStrength = 0.0f;
				pShake->m_fElapsedTime     = 0.0f;
			}

			return;
		}
	}
}

// $deBlob: FUNCTION 0058fdc0
void ACameraShakeFunction::StopLoopingShake( TINT a_iID )
{
	if ( m_pExtraShakes == TNULL )
		return;

	for ( TINT i = m_pExtraShakes->m_iNumShakes - 1; i >= 0; i-- )
	{
		ShakeData* pShake = &m_pExtraShakes->m_pShakes[ i ];

		if ( pShake->m_iID == a_iID )
		{
			TASSERT( ( pShake->m_iStatusFlag & SF_LOOPING_EFFECT_PULSE ) || ( pShake->m_iStatusFlag & SF_LOOPING_EFFECT_CONSTANT ) );
			pShake->m_iStatusFlag &= ~SF_UNK40;
			pShake->m_fCurrentStrength = 0.0f;
			pShake->m_fElapsedTime     = 0.0f;
			return;
		}
	}
}

// $deBlob: FUNCTION 0058fe50
void ACameraShakeFunction::StopExtraShake( TINT a_iID )
{
	if ( m_pExtraShakes == TNULL )
		return;

	for ( TINT i = m_pExtraShakes->m_iNumShakes; i != 0; )
	{
		i--;
		ShakeData* pShake = &m_pExtraShakes->m_pShakes[ i ];

		if ( pShake->m_iID == a_iID )
		{
			pShake->m_iStatusFlag &= ~( SF_ENABLED | SF_UNK40 );
			pShake->m_fCurrentStrength = 0.0f;
			pShake->m_fElapsedTime     = 0.0f;
			return;
		}
	}
}

// $deBlob: FUNCTION 0058feb0
void ACameraShakeFunction::ApplyShakes( ACamera* a_pCamera )
{
	a_pCamera->GetShakeMatrix() = TMatrix44::IDENTITY;

	if ( !m_bShaking )
		return;

	TFLOAT fMax = 0.0f;

	for ( TINT i = m_iNumShakes; i != 0; )
	{
		i--;
		TFLOAT fStrength = GetShakeStrength( m_aShakes[ i ], a_pCamera, fMax );

		if ( fStrength > fMax )
			fMax = fStrength;
	}

	if ( m_pExtraShakes != TNULL )
	{
		for ( TINT i = m_pExtraShakes->m_iNumShakes; i != 0; )
		{
			i--;
			TFLOAT fStrength = GetShakeStrength( m_pExtraShakes->m_pShakes[ i ], a_pCamera, fMax );

			if ( fStrength > fMax )
				fMax = fStrength;
		}
	}

	if ( fMax > 0.0f )
	{
		TRandom& rRandom = ARandom::GetSingleton()->GetRandom();
		TFLOAT   fYaw    = ( rRandom.GetFloat() * 2.0f - 1.0f ) * fMax * 0.2f;
		TFLOAT   fPitch  = ( rRandom.GetFloat() * 2.0f - 1.0f ) * fMax * 0.2f;
		TFLOAT   fRoll   = ( rRandom.GetFloat() * 2.0f - 1.0f ) * fMax * 0.2f;

		TMatrix44& rShake = a_pCamera->GetShakeMatrix();
		rShake.RotateX( fPitch );
		rShake.RotateZ( fRoll );
		rShake.RotateY( -fYaw );
	}
}
