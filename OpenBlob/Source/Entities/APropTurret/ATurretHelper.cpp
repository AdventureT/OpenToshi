#include "pch.h"
#include "ATurretHelper.h"
#include "ARandom.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/ABaseEntity.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameInterface/APlayerManager.h"
#include "Sound/APositionalSoundManager.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundMap.h"
#include <Physics/Havok/TPhysicsManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ATurretHelper ) == 0xA8 );

// $deBlob: FUNCTION 007c8480
ATurretHelper::ATurretHelper( ABaseEntity* a_pOwner )
{
	m_vRight           = Toshi::TVector4::VEC_ZERO;
	m_vOrigin          = Toshi::TVector4::VEC_ZERO;
	m_vAim             = Toshi::TVector4( 1.0f, 0.0f, 0.0f, 1.0f );
	m_vLookAt          = Toshi::TVector4::VEC_ZERO;
	m_vFacing          = Toshi::TVector4::VEC_ZERO;
	m_pOwner           = a_pOwner;
	m_pTarget          = TNULL;
	m_eState           = 0;
	m_iCounter         = 0;
	m_iMaxBurst        = 3;
	m_uiFlags          = TURRETFLAG_Unk1;
	m_fAngleFrac       = 0.0f;
	m_fTimer60         = 0.0f;
	m_fPitchFrac       = 0.5f;
	m_fTargetPitchFrac = 0.5f;
	m_fFireTimer       = -1.0f;
	m_fBurstTimer      = -1.0f;
	m_fIdleTime        = 0.499f;
	m_fGravity         = 30.0f;
	m_fProjectileSpeed = 65.0f;
	m_fRotateSpeed     = 0.5f;
	m_fMaxVisualAngleA = 0.85f;
	m_fMaxVisualAngleB = 0.85f;
	m_fVisualRangeA    = 150.0f;
	m_fVisualRangeB    = 150.0f;
	m_fFireDelay       = 10.0f;
	m_fBurstDelay      = 1.0f;
	m_fPitchMin        = 0.0f;
	m_fPitchMax        = 1.0f;
	m_fLOSTimer        = -1.0f;
}

// $deBlob: FUNCTION 007c8580
// $deBlob: FUNCTION 00779680
ATurretHelper::~ATurretHelper()
{
	// NOTE: the retail destructor is a thunk into StopEffects, both are the same code
	StopEffects();
}

// $deBlob: FUNCTION 007c8590
TBOOL ATurretHelper::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fFireTimer > 0.0f )
		m_fFireTimer -= a_fDeltaTime;

	if ( m_fBurstTimer > 0.0f )
		m_fBurstTimer -= a_fDeltaTime;

	if ( m_fLOSTimer > 0.0f )
		m_fLOSTimer -= a_fDeltaTime;

	switch ( m_eState )
	{
		case 0:
			if ( UpdateTargeting() )
			{
				UpdateAnimation( a_fDeltaTime );
				m_eState = 1;
				return TFALSE;
			}

			if ( UpdateAnimation( a_fDeltaTime ) )
			{
				if ( m_fTimer60 <= 0.0f )
				{
					if ( ( m_uiFlags & TURRETFLAG_Unk1 ) == 0 )
					{
						m_fTimer60 = 1.0f - m_fIdleTime;
						return TFALSE;
					}

					m_fTimer60 = m_fIdleTime;
					return TFALSE;
				}

				m_fTimer60 = 0.0f;
				m_uiFlags ^= TURRETFLAG_Unk1;
				PlayPositionalSound();
				return TFALSE;
			}
			break;

		case 1:
			if ( !UpdateTargeting() )
			{
				if ( 0.5f < m_fAngleFrac || m_fAngleFrac == 0.5f )
					m_uiFlags |= TURRETFLAG_Unk1;
				else
					m_uiFlags &= ~TURRETFLAG_Unk1;

				m_fTimer60         = 0.0f;
				m_fTargetPitchFrac = 0.5f;
				m_fLOSTimer        = -1.0f;
				m_fBurstTimer      = -1.0f;
				m_eState           = 0;
				m_iCounter         = 0;
				UpdateAnimation( a_fDeltaTime );
				return TFALSE;
			}

			if ( UpdateAnimation( a_fDeltaTime ) && GetCurrentTimer() < 0.0f )
			{
				m_eState = 2;
				return TFALSE;
			}
			break;

		case 2:
			m_iCounter += 1;
			m_fBurstTimer = m_fBurstDelay;
			m_fFireTimer  = m_fFireDelay;

			if ( m_iMaxBurst <= m_iCounter )
			{
				m_fBurstTimer = -1.0f;
				m_iCounter    = 0;
			}

			if ( !UpdateTargeting() )
			{
				if ( 0.5f < m_fAngleFrac || m_fAngleFrac == 0.5f )
					m_uiFlags |= TURRETFLAG_Unk1;
				else
					m_uiFlags &= ~TURRETFLAG_Unk1;

				m_fTimer60         = 0.0f;
				m_fTargetPitchFrac = 0.5f;
				m_fLOSTimer        = -1.0f;
				m_fBurstTimer      = -1.0f;
				m_eState           = 0;
				m_iCounter         = 0;
			}
			else
			{
				m_eState = 1;
			}

			UpdateAnimation( a_fDeltaTime );
			return TTRUE;

		case 3:
			StopEffects();
			break;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c87f0
void ATurretHelper::Reset()
{
	m_uiFlags &= ~TURRETFLAG_Turning;
	m_eState           = 0;
	m_iCounter         = 0;
	m_pTarget          = TNULL;
	m_fAngleFrac       = 0.0f;
	m_fTimer60         = 0.0f;
	m_fPitchFrac       = 0.5f;
	m_fTargetPitchFrac = 0.5f;
	m_fFireTimer       = -1.0f;
	m_fBurstTimer      = -1.0f;
	m_fLOSTimer        = -1.0f;
}

// $deBlob: FUNCTION 007c8840
void ATurretHelper::Initialise( TFLOAT a_fSearchAngle, TFLOAT a_fFireDelay, TFLOAT a_fGravity, TFLOAT a_fProjectileSpeed, TFLOAT a_fRotateSpeed, TFLOAT a_fVisualRangeA, TFLOAT a_fVisualRangeB, TFLOAT a_fMaxVisualAngleA, TFLOAT a_fMaxVisualAngleB, TINT a_iMaxBurst, TFLOAT a_fBurstDelay )
{
	TASSERT( a_fMaxVisualAngleA <= a_fMaxVisualAngleB );
	TASSERT( a_fMaxVisualAngleB <= TMath::PI );

	m_fIdleTime        = a_fSearchAngle;
	m_fGravity         = a_fGravity;
	m_fProjectileSpeed = a_fProjectileSpeed;
	m_fRotateSpeed     = a_fRotateSpeed;
	m_fMaxVisualAngleA = ( a_fMaxVisualAngleA <= TMath::PI ) ? a_fMaxVisualAngleA : TMath::PI;
	m_fVisualRangeA    = a_fVisualRangeA;
	m_fVisualRangeB    = a_fVisualRangeB;
	m_fMaxVisualAngleB = ( a_fMaxVisualAngleB <= TMath::PI ) ? a_fMaxVisualAngleB : TMath::PI;
	m_fFireDelay       = a_fFireDelay;

	TASSERT( a_iMaxBurst < 0x7F );
	m_iMaxBurst   = (TINT8)a_iMaxBurst;
	m_fBurstDelay = a_fBurstDelay;
}

// $deBlob: FUNCTION 007c9740
TBOOL ATurretHelper::IsInCone( const Toshi::TVector4& a_rvOrigin, const Toshi::TVector4& a_rvConeDirection, TFLOAT a_fConeFOV, TFLOAT a_fConeRange, const Toshi::TVector4& a_rvTarget )
{
	TASSERT( a_fConeFOV < TMath::PI );
	TASSERT( a_rvConeDirection.IsNormalised() );

	Toshi::TVector4 vDiff;
	vDiff.w = a_rvTarget.w;
	vDiff.x = a_rvTarget.x - a_rvOrigin.x;
	vDiff.y = a_rvTarget.y - a_rvOrigin.y;
	vDiff.z = a_rvTarget.z - a_rvOrigin.z;

	if ( a_fConeFOV >= 1.5707964f || ( a_rvConeDirection.y * vDiff.y + a_rvConeDirection.x * vDiff.x + a_rvConeDirection.z * vDiff.z ) > 0.0f )
	{
		if ( vDiff.MagnitudeSq() <= a_fConeRange * a_fConeRange )
		{
			vDiff.Normalise();

			TFLOAT fDot = a_rvConeDirection.y * vDiff.y + a_rvConeDirection.x * vDiff.x + a_rvConeDirection.z * vDiff.z;
			Toshi::TMath::Clip( fDot, -1.0f, 1.0f );

			TFLOAT fAngle = Toshi::TMath::ACos( fDot );
			return fAngle <= a_fConeFOV;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c89c0
void ATurretHelper::SetFacing( const Toshi::TVector4& a_rvFacing )
{
	m_vFacing = a_rvFacing;
	m_vRight.CrossProduct( a_rvFacing, Toshi::TVector4::VEC_NEGY );

	TASSERT( m_vRight.y == 0.0f && m_vRight.IsNormalised() );
}

// $deBlob: FUNCTION 007c8ad0
void ATurretHelper::SetState( TINT a_eState )
{
	switch ( a_eState )
	{
		case 0:
		{
			if ( 0.5f < m_fAngleFrac || m_fAngleFrac == 0.5f )
				m_uiFlags |= TURRETFLAG_Unk1;
			else
				m_uiFlags &= ~TURRETFLAG_Unk1;

			m_fTimer60         = 0.0f;
			m_fTargetPitchFrac = 0.5f;
			m_fLOSTimer        = -1.0f;
			m_fBurstTimer      = -1.0f;
			m_iCounter         = 0;
		}
		break;
		case 3:
		{
			StopEffects();

			if ( m_pTarget != TNULL )
			{
				m_pTarget->DecTargetCount();
				m_eState  = 3;
				m_pTarget = TNULL;
				return;
			}
		}
		break;
	}

	m_eState = (TUINT8)a_eState;
}

// $deBlob: FUNCTION 007c8b50
void ATurretHelper::ReleaseTarget()
{
	StopEffects();

	if ( m_pTarget != TNULL )
	{
		m_pTarget->DecTargetCount();
		m_pTarget = TNULL;
	}
}

// $deBlob: FUNCTION 007c8bb0
TFLOAT ATurretHelper::GetCurrentTimer() const
{
	if ( m_iCounter > 0 )
	{
		return TMath::Min( m_fFireTimer, m_fBurstTimer );
	}

	return m_fFireTimer;
}

// $deBlob: FUNCTION 007c8c00
void ATurretHelper::OnFire( AProjectile* a_pProjectile )
{
	// NOTE: retail ignores both the helper and the projectile, it only forwards to the HUD
	ABlobHud::GetSingleton()->OnFire();
}

// $deBlob: FUNCTION 007c8c10
TBOOL ATurretHelper::UpdateTargeting()
{
	// NOTE: the retail code is one function with gotos; the aim path is the do/while body, which returns, and every
	// "break" goes to the miss path.
	do
	{
		TINT8 iPlayer = APlayerManager::GetSingleton()->GetClosestPlayer( m_vOrigin );
		if ( iPlayer == -1 )
			break;

		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( iPlayer ) );

		// state 11 is STATE_DEAD, state 8 is not named yet; 0x80000 is a BFLAG (0x554) not named yet
		if ( pBlob->m_eState == ABlobEntity::STATE_DEAD || pBlob->m_eState == ABlobEntity::STATE_GRASSBOMB || ( pBlob->m_uiFlags & 0x80000 ) != 0 || !( pBlob->m_fUntargetableTime <= 0.0f ) )
			break;

		Toshi::TVector4 vOrigin = m_vOrigin;
		Toshi::TVector4 vTarget = pBlob->GetBodyPosition();

		// the visual cone is the lerp between the two angles at the blob's size (0..9)
		TFLOAT fT     = TFLOAT( pBlob->m_iSize ) / 9.0f;
		TFLOAT fFOV   = Toshi::TMath::LerpTowards( m_fMaxVisualAngleA, m_fMaxVisualAngleB, fT );
		TFLOAT fRange = Toshi::TMath::LerpTowards( m_fVisualRangeA, m_fVisualRangeB, fT );

		Toshi::TVector4 vDir = m_vFacing;
		vDir.RotateY( -( m_fAngleFrac * 6.2831855f ) );

		if ( !IsInCone( vOrigin, vDir, fFOV, fRange, vTarget ) )
			break;

		// TBOOL bBlocked: no target yet means the turret is not in its aim state; the timer is checked below
		TBOOL bBlocked = m_pTarget == TNULL;

		if ( m_fLOSTimer <= 0.0f )
		{
			TPhysicsRay oRay;
			oRay.m_vFrom            = vOrigin;
			oRay.m_vTo              = vTarget;
			oRay.m_uiCollisionLayer = 0xA1;

			TPhysicsShapeContact aContacts[ 3 ];
			TINT                 iNumContacts = TPhysicsManager::GetSingleton()->CollideRayAll( oRay, aContacts, 3, TNULL );

			// blocked when a contact is not a body of this turret's owner (a body without user data counts)
			bBlocked = TFALSE;
			for ( TINT i = 0; i < iNumContacts; i++ )
			{
				TPhysicsObjectContainer* pBody = aContacts[ i ].m_pBody;
				if ( pBody == TNULL )
					break;

				ACollisionInstance2::OwnerInfo* pInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, pBody->m_pUserData );
				if ( pInfo == TNULL || pInfo->m_pOwner != m_pOwner )
				{
					bBlocked = TTRUE;
					break;
				}
			}

			// random 0.1 .. 0.15 s until the next check
			TUINT32 uiRand = ARandom::GetSingleton()->GetRandom().RandRaw();
			m_fLOSTimer    = TFLOAT( TINT( uiRand >> 1 ) ) * 4.6566129e-10f * 0.050000004f + 0.1f;
		}

		if ( bBlocked )
			break;

		// the aim is the ballistic velocity from m_vLookAt to the blob's position, with m_fProjectileSpeed speed and m_fGravity gravity
		ABlobEntity::CalculateJumpVelocityAtSpeed( m_vLookAt, pBlob->GetTranslation(), m_fProjectileSpeed, m_fGravity, m_vAim );

		Toshi::TVector4 vAim = m_vAim;
		vAim.Normalise();

		TFLOAT fYaw   = ( vAim.y + 1.0f ) * 0.5f;
		TFLOAT fDelta = m_fTargetPitchFrac - fYaw;
		if ( fDelta < 0.0f )
			fDelta = -fDelta;

		if ( fDelta <= 0.05f )
			fYaw = fYaw * 0.2f + m_fTargetPitchFrac * 0.8f;

		m_fTargetPitchFrac = fYaw;
		Toshi::TMath::Clip( m_fTargetPitchFrac, 0.0f, 1.0f );

		TFLOAT fXZ = vAim.NormaliseXZ();

		TASSERT( m_vFacing.y == 0.0f && m_vFacing.IsNormalised() );

		if ( fXZ < 0.35f || ( vAim.x * ( vTarget.x - vOrigin.x ) + vAim.y * 0.0f + vAim.z * ( vTarget.z - vOrigin.z ) ) < 0.0f )
		{
			// the blob is behind the aim: it is aimed along the XZ direction to the blob instead
			vAim.x = vTarget.x - vOrigin.x;
			vAim.y = 0.0f;
			vAim.z = vTarget.z - vOrigin.z;
			vAim.w = vTarget.w;
			vAim.NormaliseXZ();

			TASSERT( m_vFacing.y == 0.0f && m_vFacing.IsNormalised() );
		}

		TFLOAT fDot = m_vFacing.y * vAim.y + m_vFacing.x * vAim.x + m_vFacing.z * vAim.z;
		Toshi::TMath::Clip( fDot, -1.0f, 1.0f );

		TFLOAT fAngle = Toshi::TMath::ACos( fDot ) / 6.2831855f;
		if ( ( m_vRight.y * vAim.y + m_vRight.x * vAim.x + m_vRight.z * vAim.z ) > 0.0f )
			fAngle = 1.0f - fAngle;

		// m_fTimer60 (0x60) is the smoothed turning fraction; it's blended with the new angle when close, else snapped
		fDelta = m_fTimer60 - fAngle;
		if ( fDelta < 0.0f )
			fDelta = -fDelta;

		TBOOL bWrapped = fDelta > ( 1.0f - fDelta );
		if ( bWrapped )
			fDelta = 1.0f - fDelta;

		if ( fDelta <= 0.1f )
		{
			m_fTimer60 = fAngle * 0.2f + m_fTimer60 * 0.8f;
			if ( bWrapped )
			{
				m_fTimer60 += 0.5f;
				if ( m_fTimer60 > 1.0f )
					m_fTimer60 -= 1.0f;
			}
		}
		else
		{
			m_fTimer60 = fAngle;
		}

		if ( m_pTarget != pBlob )
		{
			if ( m_pTarget != TNULL )
				m_pTarget->DecTargetCount();

			m_pTarget = pBlob;
			pBlob->IncTargetCount();
		}

		return TTRUE;
	} while ( TFALSE );

	if ( m_pTarget != TNULL )
	{
		m_pTarget->DecTargetCount();
		m_pTarget = TNULL;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c92b0
TBOOL ATurretHelper::UpdateAnimation( TFLOAT a_fDeltaTime )
{
	TFLOAT fLerpAmount = ( m_fRotateSpeed * a_fDeltaTime ) / 6.2831855f;
	TASSERT( fLerpAmount < 0.5f );

	TFLOAT fPitchFrac       = m_fPitchFrac;
	TFLOAT fTargetPitchFrac = m_fTargetPitchFrac;
	TFLOAT fAngle           = m_fAngleFrac;
	TUINT8 uiFlags          = m_uiFlags & ~TURRETFLAG_Turning;

	m_uiFlags = uiFlags;

	TBOOL bRet;

	if ( fTargetPitchFrac <= fPitchFrac )
	{
		bRet = ( fPitchFrac - fTargetPitchFrac <= fLerpAmount );

		if ( bRet )
			m_fPitchFrac = fTargetPitchFrac;
		else
			m_fPitchFrac = fPitchFrac - fLerpAmount;

		if ( m_fPitchFrac <= m_fPitchMin && m_fPitchMin != m_fPitchFrac )
		{
			m_fPitchFrac = m_fPitchMin;
			bRet         = TFALSE;
			m_uiFlags    = uiFlags | TURRETFLAG_Turning;
		}
	}
	else
	{
		bRet = ( fTargetPitchFrac - fPitchFrac <= fLerpAmount );

		if ( bRet )
			m_fPitchFrac = fTargetPitchFrac;
		else
			m_fPitchFrac = fPitchFrac + fLerpAmount;

		if ( m_fPitchMax < m_fPitchFrac )
		{
			m_fPitchFrac = m_fPitchMax;
			bRet         = TFALSE;
			m_uiFlags    = uiFlags | TURRETFLAG_Turning;
		}
	}

	TFLOAT fTarget = m_fTimer60;
	TFLOAT fStep   = fLerpAmount;

	// the angle lerps towards m_fTimer60 (the idle target) the short way round
	if ( fTarget <= fAngle )
	{
		TFLOAT fDist = fAngle - fTarget;

		if ( 0.5f <= fDist )
		{
			if ( 1.0f - fDist < fStep )
			{
				m_fAngleFrac = fTarget;
				goto LAB_Done;
			}

			bRet         = TFALSE;
			fStep        = fAngle + fStep;
			m_fAngleFrac = fStep;
			if ( fStep <= 1.0f )
				goto LAB_Done;

			fStep -= 1.0f;
		}
		else
		{
			if ( fDist < fStep )
			{
				m_fAngleFrac = fTarget;
				goto LAB_Done;
			}

			fStep = fAngle - fStep;
		}
	}
	else
	{
		TFLOAT fDist = fTarget - fAngle;

		if ( 0.5f <= fDist )
		{
			if ( 1.0f - fDist < fStep )
			{
				m_fAngleFrac = fTarget;
				goto LAB_Done;
			}

			bRet         = TFALSE;
			fStep        = fAngle - fStep;
			m_fAngleFrac = fStep;
			if ( 0.0f <= fStep )
				goto LAB_Done;

			fStep += 1.0f;
		}
		else
		{
			if ( fDist < fStep )
			{
				m_fAngleFrac = fTarget;
				goto LAB_Done;
			}

			fStep = fAngle + fStep;
		}
	}

	bRet         = TFALSE;
	m_fAngleFrac = fStep;

LAB_Done:
	fStep         = 0.0f;
	TFLOAT fPitch = 0.0f;
	if ( 0.0f < a_fDeltaTime )
	{
		fPitch = ( fAngle - m_fAngleFrac ) / a_fDeltaTime;
		fStep  = ( fPitchFrac - m_fPitchFrac ) / a_fDeltaTime;
	}

	UpdateSounds( fStep, fPitch );
	return bRet;
}

// $deBlob: FUNCTION 007c9530
void ATurretHelper::UpdateSounds( TFLOAT a_fYawSpeed, TFLOAT a_fPitchSpeed )
{
	// NOTE: the owner's loop sounds are group 0x1E, events 0x140/0x144/0x145/0x148
	if ( m_pOwner != TNULL )
	{
		TBOOL bYawStill = TTRUE;
		if ( Toshi::TMath::Abs( a_fYawSpeed ) <= 0.015f )
		{
			if ( m_uiFlags & TURRETFLAG_SoundLoop148 )
			{
				APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x148 );
				m_uiFlags &= ~TURRETFLAG_SoundLoop148;
			}
		}
		else
		{
			bYawStill = TFALSE;
			if ( !( m_uiFlags & TURRETFLAG_SoundLoop148 ) )
			{
				APositionalSoundManager::GetSingleton()->AddEntitySound( m_pOwner, 0x1E, 0x148 );
				m_uiFlags |= TURRETFLAG_SoundLoop148;
			}

			TFLOAT fVolume = m_fPitchFrac;
			Toshi::TMath::Clip( fVolume, 0.25f, 0.75f );
			fVolume = (TFLOAT)( fVolume * -1.2 + 0.4 );
			APositionalSoundManager::GetSingleton()->SetPitch( m_pOwner, 0x1E, 0x148, fVolume );
		}

		if ( Toshi::TMath::Abs( a_fPitchSpeed ) <= 0.015f )
		{
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x145 );
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x144 );
			m_uiFlags &= 0xCF; // clears TURRETFLAG_SoundLoop144 and TURRETFLAG_SoundLoop145
			if ( bYawStill )
			{
				if ( m_uiFlags & TURRETFLAG_SoundLoop140 )
				{
					return;
				}

				APositionalSoundManager::GetSingleton()->AddEntitySound( m_pOwner, 0x1E, 0x140 );
				m_uiFlags |= TURRETFLAG_SoundLoop140;
				return;
			}
		}
		else if ( a_fPitchSpeed <= 0.015f )
		{
			if ( a_fPitchSpeed < -0.015f )
			{
				if ( m_uiFlags & TURRETFLAG_SoundLoop145 )
				{
					APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x145 );
					m_uiFlags &= ~TURRETFLAG_SoundLoop145;
				}

				if ( !( m_uiFlags & TURRETFLAG_SoundLoop144 ) )
				{
					APositionalSoundManager::GetSingleton()->AddEntitySound( m_pOwner, 0x1E, 0x144 );
					m_uiFlags |= TURRETFLAG_SoundLoop144;
				}
			}
		}
		else
		{
			if ( m_uiFlags & TURRETFLAG_SoundLoop144 )
			{
				APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x144 );
				m_uiFlags &= ~TURRETFLAG_SoundLoop144;
			}

			if ( !( m_uiFlags & TURRETFLAG_SoundLoop145 ) )
			{
				APositionalSoundManager::GetSingleton()->AddEntitySound( m_pOwner, 0x1E, 0x145 );
				m_uiFlags |= TURRETFLAG_SoundLoop145;
			}
		}

		if ( m_uiFlags & TURRETFLAG_SoundLoop140 )
		{
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x140 );
			m_uiFlags &= ~TURRETFLAG_SoundLoop140;
		}
	}
}

// $deBlob: FUNCTION 007c94d0
Toshi::TVector4* ATurretHelper::GetAimDirection( Toshi::TVector4* a_pOut ) const
{
	*a_pOut = m_vFacing;
	a_pOut->RotateY( -( m_fAngleFrac * 6.2831855f ) );
	return a_pOut;
}

// $deBlob: FUNCTION 007c9510
void ATurretHelper::PlayPositionalSound()
{
	if ( m_pOwner != TNULL )
	{
		// NOTE: group 0x1E and event 0x146 are the turret's loop sounds (ASoundMap ids not named yet); the
		// position is the owner's matrix translation (owner + 0x40)
		ASoundInterface::PlayEventAtPosition( (ASoundMap::GROUPID)0x1E, (ASoundMap::EVENTID)0x146, m_pOwner->GetMatrix().AsBasisVector4( Toshi::BASISVECTOR_TRANSLATION ), (FMOD_EVENT_MODE)0 );
	}
}

// $deBlob: FUNCTION 007c9970
void ATurretHelper::SetFlagUnk1( TBOOL a_bSet )
{
	if ( a_bSet )
		m_uiFlags |= TURRETFLAG_Unk1;
	else
		m_uiFlags &= ~TURRETFLAG_Unk1;
}

// $deBlob: FUNCTION 007c9990
void ATurretHelper::SetTurning( TBOOL a_bSet )
{
	if ( a_bSet )
		m_uiFlags |= TURRETFLAG_Turning;
	else
		m_uiFlags &= ~TURRETFLAG_Turning;
}

// $deBlob: FUNCTION 007c8a40
void ATurretHelper::StopEffects()
{
	if ( m_pOwner != TNULL )
	{
		if ( m_uiFlags & TURRETFLAG_SoundLoop148 )
		{
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x148 );
			m_uiFlags &= ~TURRETFLAG_SoundLoop148;
		}

		if ( m_uiFlags & TURRETFLAG_SoundLoop144 )
		{
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x144 );
			m_uiFlags &= ~TURRETFLAG_SoundLoop144;
		}

		if ( m_uiFlags & TURRETFLAG_SoundLoop145 )
		{
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x145 );
			m_uiFlags &= ~TURRETFLAG_SoundLoop145;
		}

		if ( m_uiFlags & TURRETFLAG_SoundLoop140 )
		{
			APositionalSoundManager::GetSingleton()->RemoveSound( m_pOwner, 0x1E, 0x140 );
			m_uiFlags &= ~TURRETFLAG_SoundLoop140;
		}
	}
}

// $deBlob: FUNCTION 007c99b0
const TCHAR* ATurretHelper::GetProjectileDataName( TINT a_iIndex )
{
	if ( a_iIndex == 0 )
		return "ProjectileData_Prop_Turret_Bullet_EntityData";

	if ( a_iIndex != 1 )
		return TNULL;

	return "ProjectileData_Boss_Bullet_EntityData";
}
