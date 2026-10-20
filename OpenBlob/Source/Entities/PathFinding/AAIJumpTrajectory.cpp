#include "pch.h"
#include "AAIJumpTrajectory.h"
#include "APathfindingComponent.h"
#include "Entities/ABaseEntity.h"
#include "Entities/ACharEntity.h"
#include "Entities/AGndMotorComponent.h"
#include "Collision/ACollisionManager2.h"
#include "Console/AConsoleState.h"

#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00541930
// $deBlob: FUNCTION 0085a0e0
AConsoleVar AAIJumpTrajectory::sm_cvShowTrajectory( "ai_jump_trajectory", TFALSE ); // 0x009b8e94

static APathfindingComponent* GetPathfinding( ABaseEntity* a_pEntity )
{
	return TSTATICCAST( APathfindingComponent, a_pEntity->GetPathfinding() );
}

// $deBlob: FUNCTION 007d7ec0
AAIJumpTrajectory::AAIJumpTrajectory()
{
	// NOTE: m_fTime isn't initialised
	m_pOwner                            = TNULL;
	m_vCreateAcceleration               = TVector4::VEC_ZERO;
	m_vCreateVelocity                   = TVector4::VEC_ZERO;
	m_vCreateStart                      = TVector4::VEC_ZERO;
	m_fCreateTotalTime                  = 0.0f;
	m_fTimeScale                        = 1.0f;
	m_bActive                           = TFALSE;
	m_bUnk41                            = TFALSE;
	m_oTrajectory.m_vAcceleration       = TVector4::VEC_ZERO;
	m_oTrajectory.m_vVelocity           = TVector4::VEC_ZERO;
	m_oTrajectory.m_vStart              = TVector4::VEC_ZERO;
	m_oTrajectory.m_fTotalTime          = 0.0f;
	m_oTrajectory.m_fSecondJumpSpeed    = 0.0f;
	m_oTrajectory.m_fSecondJumpTime     = -1.0f;
	m_oTrajectory.m_fHorizontalTime     = -1.0f;
	m_oTrajectory.m_vHorizontalVelocity = TVector4::VEC_ZERO;
	m_oTrajectory.m_eJumpType           = JumpType_Auto;
}

// $deBlob: FUNCTION 007d7f70
AAIJumpTrajectory::AAIJumpTrajectory( const TVector4& a_rvA, const TVector4& a_rvStart, const TVector4& a_rvEnd, TFLOAT a_fHeight )
{
	// NOTE: m_pOwner and m_fTime aren't initialised (Create sets m_fTime)
	m_vCreateAcceleration               = TVector4::VEC_ZERO;
	m_vCreateVelocity                   = TVector4::VEC_ZERO;
	m_vCreateStart                      = TVector4::VEC_ZERO;
	m_fCreateTotalTime                  = 0.0f;
	m_fTimeScale                        = 1.0f;
	m_bActive                           = TFALSE;
	m_bUnk41                            = TFALSE;
	m_oTrajectory.m_vAcceleration       = TVector4::VEC_ZERO;
	m_oTrajectory.m_vVelocity           = TVector4::VEC_ZERO;
	m_oTrajectory.m_vStart              = TVector4::VEC_ZERO;
	m_oTrajectory.m_fTotalTime          = 0.0f;
	m_oTrajectory.m_fSecondJumpSpeed    = 0.0f;
	m_oTrajectory.m_fSecondJumpTime     = -1.0f;
	m_oTrajectory.m_fHorizontalTime     = -1.0f;
	m_oTrajectory.m_vHorizontalVelocity = TVector4::VEC_ZERO;
	m_oTrajectory.m_eJumpType           = JumpType_Auto;

	Create( a_rvA, a_rvStart, a_rvEnd, a_fHeight );
}

// $deBlob: FUNCTION 007d8040
AAIJumpTrajectory::~AAIJumpTrajectory()
{
}

// $deBlob: FUNCTION 007d8c40
void AAIJumpTrajectory::Init()
{
	AConsoleState::RegisterCVar( &sm_cvShowTrajectory );
}

// $deBlob: FUNCTION 007d8c50
void AAIJumpTrajectory::Destroy()
{
	AConsoleState::UnregisterCVar( &sm_cvShowTrajectory );
}

// $deBlob: FUNCTION 007d8390
void AAIJumpTrajectory::Create( const TVector4& a_rvA, const TVector4& a_rvStart, const TVector4& a_rvEnd, TFLOAT a_fHeight )
{
	TASSERT( TMath::Abs( a_rvA.x ) < TMath::FLOATEPSILON );
	TASSERT( TMath::Abs( a_rvA.z ) < TMath::FLOATEPSILON );

	m_fTime = 0.0f;

	TFLOAT fA = ( a_rvStart.y - a_fHeight ) * a_rvA.y;
	TFLOAT fB = ( a_rvEnd.y - a_fHeight ) * a_rvA.y;
	TASSERT( fA >= 0 );
	TASSERT( fB >= 0 );

	m_fCreateTotalTime = ( TMath::Sqrt( fB ) + TMath::Sqrt( fA ) ) * ( 1.0f / a_rvA.y );
	TASSERT( m_fCreateTotalTime > 0 );

	m_vCreateAcceleration.y = a_rvA.y;

	fA = ( a_rvStart.y - a_fHeight ) * a_rvA.y;
	TASSERT( fA >= 0 );

	m_vCreateVelocity.y = TMath::Sqrt( fA ) * -2.0f;
	m_vCreateVelocity.x = ( a_rvEnd.x - a_rvStart.x ) * ( 1.0f / m_fCreateTotalTime );
	m_vCreateVelocity.z = ( a_rvEnd.z - a_rvStart.z ) * ( 1.0f / m_fCreateTotalTime );
	m_fTime             = m_fCreateTotalTime;
	m_vCreateStart      = a_rvStart;

	// The end of the parabola must be a_rvEnd
	// NOTE: GetPosition reads m_vAcceleration, m_vVelocity, m_vStart and m_fTotalTime, which Create doesn't write: this
	// check is about another parabola
	TVector4 vPosition;
	GetPosition( vPosition, 0.0f );

	TVector4 v( vPosition.x - a_rvEnd.x, vPosition.y - a_rvEnd.y, vPosition.z - a_rvEnd.z, vPosition.w );
	TASSERT( v.Magnitude() < 0.001f );

	m_fTime   = 0.0f;
	m_bActive = TTRUE;
}

// $deBlob: FUNCTION 007d8c60
TBOOL AAIJumpTrajectory::CheckSingleJump( const TVector4& a_rvStart, const TVector4& a_rvGoal, TFLOAT a_fGravity, TFLOAT a_fJumpSpeed, TFLOAT a_fDistance, TFLOAT a_fSpeed, TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, void* a_pCollisionCache )
{
	// The top of the jump has to reach the higher end
	const TFLOAT* pHighest = &a_rvStart.y;

	if ( a_rvGoal.y > a_rvStart.y )
		pHighest = &a_rvGoal.y;

	TFLOAT fJumpHeight = ( a_fJumpSpeed * a_fJumpSpeed * 0.5f ) / a_fGravity;

	if ( *pHighest - a_rvStart.y > fJumpHeight )
		return TFALSE;

	TFLOAT fTotalTime = TMath::Sqrt( ( ( a_rvStart.y + fJumpHeight - a_rvGoal.y ) * 2.0f ) / a_fGravity ) + a_fJumpSpeed / a_fGravity;

	if ( a_fDistance / a_fSpeed > fTotalTime )
		return TFALSE;

	m_oTrajectory.m_fTotalTime = fTotalTime;

	TVector4 vDirection( a_rvGoal.x - a_rvStart.x, a_rvGoal.y - a_rvStart.y, a_rvGoal.z - a_rvStart.z, a_rvGoal.w );
	vDirection.NormaliseXZ();

	m_oTrajectory.m_vVelocity.y = -a_fJumpSpeed;

	switch ( m_oTrajectory.m_eJumpType )
	{
		case JumpType_MoveThenStop:
			m_oTrajectory.m_vVelocity.x     = vDirection.x * a_fSpeed;
			m_oTrajectory.m_vVelocity.z     = vDirection.z * a_fSpeed;
			m_oTrajectory.m_fHorizontalTime = a_fDistance / a_fSpeed;
			break;
		case JumpType_MoveThenJump:
		{
			TFLOAT fSpeed                   = a_fDistance / m_oTrajectory.m_fTotalTime;
			m_oTrajectory.m_fHorizontalTime = -1.0f;
			m_oTrajectory.m_vVelocity.z     = fSpeed * vDirection.z;
			m_oTrajectory.m_vVelocity.x     = fSpeed * vDirection.x;
			break;
		}
		case JumpType_JumpThenMove:
			m_oTrajectory.m_vVelocity.x = 0.0f;
			m_oTrajectory.m_vVelocity.z = 0.0f;
			m_oTrajectory.m_vHorizontalVelocity.Multiply( vDirection, a_fSpeed );
			m_oTrajectory.m_fHorizontalTime = m_oTrajectory.m_fTotalTime - a_fDistance / a_fSpeed;
			break;
	}

	if ( a_pObstruction != TNULL )
	{
		m_bActive = CheckFullJumpCollision( a_uiFlags, a_pObstruction, a_pCollisionCache );
		return m_bActive;
	}

	m_bActive = TTRUE;
	return m_bActive;
}

// $deBlob: FUNCTION 007d8e40
TBOOL AAIJumpTrajectory::CheckDoubleJump( const TVector4& a_rvStart, const TVector4& a_rvGoal, TFLOAT a_fGravity, TFLOAT a_fJumpSpeed, TFLOAT a_fDistance, TFLOAT a_fSpeed, TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, void* a_pCollisionCache )
{
	if ( ( GetPathfinding( m_pOwner )->GetFlags() & APathfindingComponent::FLAG_DoubleJump ) == 0 )
		return TFALSE;

	// The first jump has to reach the middle
	TVector4 vMiddle( a_rvGoal.x - a_rvStart.x, a_rvGoal.y - a_rvStart.y, a_rvGoal.z - a_rvStart.z, a_rvGoal.w );
	vMiddle.Multiply( 0.5f );
	vMiddle.x = a_rvStart.x + vMiddle.x;
	vMiddle.y = a_rvStart.y + vMiddle.y;
	vMiddle.z = a_rvStart.z + vMiddle.z;

	TFLOAT fJumpHeight = ( a_fJumpSpeed * a_fJumpSpeed * 0.5f ) / a_fGravity;

	if ( vMiddle.y - a_rvStart.y > fJumpHeight )
		return TFALSE;

	TFLOAT fJumpTime = a_fJumpSpeed / a_fGravity;

	if ( TMath::Sqrt( ( ( a_rvStart.y + fJumpHeight - vMiddle.y ) * 2.0f ) / a_fGravity ) + fJumpTime < ( a_fDistance * 0.5f ) / a_fSpeed )
		return TFALSE;

	m_oTrajectory.m_fSecondJumpSpeed = a_fJumpSpeed;
	m_oTrajectory.m_fSecondJumpTime  = fJumpTime;
	m_oTrajectory.m_fTotalTime       = TMath::Sqrt( ( ( a_rvStart.y + fJumpHeight + fJumpHeight - a_rvGoal.y ) * 2.0f ) / a_fGravity ) + fJumpTime + fJumpTime;

	TVector4 vDirection( a_rvGoal.x - a_rvStart.x, a_rvGoal.y - a_rvStart.y, a_rvGoal.z - a_rvStart.z, a_rvGoal.w );
	vDirection.NormaliseXZ();

	m_oTrajectory.m_vVelocity.y = -a_fJumpSpeed;

	switch ( m_oTrajectory.m_eJumpType )
	{
		case JumpType_MoveThenStop:
			m_oTrajectory.m_vVelocity.x     = vDirection.x * a_fSpeed;
			m_oTrajectory.m_vVelocity.z     = vDirection.z * a_fSpeed;
			m_oTrajectory.m_fHorizontalTime = a_fDistance / a_fSpeed;
			break;
		case JumpType_MoveThenJump:
		{
			// NOTE: unlike CheckSingleJump, m_fHorizontalTime is left as it is
			TFLOAT fSpeed               = a_fDistance / m_oTrajectory.m_fTotalTime;
			m_oTrajectory.m_vVelocity.z = fSpeed * vDirection.z;
			m_oTrajectory.m_vVelocity.x = fSpeed * vDirection.x;
			break;
		}
		case JumpType_JumpThenMove:
			m_oTrajectory.m_vVelocity.x = 0.0f;
			m_oTrajectory.m_vVelocity.z = 0.0f;
			m_oTrajectory.m_vHorizontalVelocity.Multiply( vDirection, a_fSpeed );
			m_oTrajectory.m_fHorizontalTime = m_oTrajectory.m_fTotalTime - a_fDistance / a_fSpeed;
			break;
	}

	if ( a_pObstruction != TNULL )
	{
		m_bActive = CheckFullJumpCollision( a_uiFlags, a_pObstruction, a_pCollisionCache );
		return m_bActive;
	}

	m_bActive = TTRUE;
	return m_bActive;
}

// $deBlob: FUNCTION 007d8050
TBOOL AAIJumpTrajectory::CheckValidJump( ABaseEntity* a_pOwner, TVector4 a_vStart, TVector4 a_vGoal, TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, JumpType a_eJumpType, TBOOL a_bCheckGoal, TBOOL a_bForce )
{
	TASSERT( a_pOwner );
	m_pOwner                  = a_pOwner;
	m_oTrajectory.m_eJumpType = JumpType_None;

	if ( !a_bForce && ( a_vGoal.z - a_vStart.z ) * ( a_vGoal.z - a_vStart.z ) + ( a_vGoal.x - a_vStart.x ) * ( a_vGoal.x - a_vStart.x ) <= 0.02f )
		return TFALSE;

	AGndMotorComponent* pMotor = GetPathfinding( a_pOwner )->GetMotor();

	// The start has to be on the ground
	if ( ( !m_pOwner->GetTranslation().IsEqual( a_vStart ) || !pMotor->IsOnGround( 0.03f ) ) && !IsValidPosition( a_vStart, m_pOwner->GetBoundingSphereRadius() * 1.1917535f ) )
		return TFALSE;

	if ( a_bCheckGoal == TTRUE && !IsValidPosition( a_vGoal, -1.0f ) )
		return TFALSE;

	// NOTE: retail gets the pathfinding component and the gravity once more without using them
	GetPathfinding( m_pOwner );
	pMotor->GetGravity();

	m_oTrajectory.m_vAcceleration.x = 0.0f;
	m_oTrajectory.m_vAcceleration.z = 0.0f;
	m_oTrajectory.m_vAcceleration.y = pMotor->GetGravity() * 0.5f;
	m_oTrajectory.m_vStart          = a_vStart;
	m_fTime                         = 0.0f;
	m_oTrajectory.m_fHorizontalTime = -1.0f;
	m_oTrajectory.m_fSecondJumpTime = -1.0f;

	// The heights above the lower end
	TFLOAT fStartHeight = a_vStart.y * -1.0f;
	TFLOAT fGoalHeight  = a_vGoal.y * -1.0f;
	TFLOAT fLowest      = ( fGoalHeight <= fStartHeight ) ? fGoalHeight : fStartHeight;
	a_vStart.y          = fStartHeight - fLowest;
	a_vGoal.y           = fGoalHeight - fLowest;

	if ( m_oTrajectory.m_eJumpType == JumpType_None )
		m_oTrajectory.m_eJumpType = ( a_eJumpType != JumpType_Auto ) ? a_eJumpType : JumpType( ( a_vStart.y < a_vGoal.y ) + JumpType_MoveThenJump );

	// NOTE: retail computes the distance and goes no further: the velocity is never set (CheckSingleJump and
	// CheckDoubleJump are unused) and a_uiFlags isn't read
	TVector4::DistanceXZ( a_vStart, a_vGoal );
	(void)a_uiFlags;

	if ( a_pObstruction != TNULL && sm_cvShowTrajectory.GetBool() )
	{
		// NOTE: nothing is drawn (Render is empty)
	}

	return TTRUE;
}

// $deBlob: FUNCTION 007d8740
TBOOL AAIJumpTrajectory::Update( TFLOAT a_fDeltaTime )
{
	TFLOAT fSecondJumpTime = m_oTrajectory.m_fSecondJumpTime;
	m_fTime                = m_fTimeScale * a_fDeltaTime + m_fTime;

	// The second jump starts from where the first one is
	if ( fSecondJumpTime != -1.0f && m_fTime >= fSecondJumpTime )
	{
		GetPosition( m_oTrajectory.m_vStart, fSecondJumpTime - m_fTime );

		TVector4 vAcceleration;
		vAcceleration.Multiply( m_oTrajectory.m_vAcceleration, 2.0f );

		TVector4 vVelocity;
		vVelocity.Multiply( vAcceleration, m_oTrajectory.m_fSecondJumpTime );

		TFLOAT fTime                = m_oTrajectory.m_fSecondJumpTime;
		m_oTrajectory.m_vVelocity.x = m_oTrajectory.m_vVelocity.x + vVelocity.x;
		m_oTrajectory.m_vVelocity.y = m_oTrajectory.m_vVelocity.y + vVelocity.y;
		m_oTrajectory.m_vVelocity.z = m_oTrajectory.m_vVelocity.z + vVelocity.z;
		m_oTrajectory.m_vVelocity.y = m_oTrajectory.m_vVelocity.y - m_oTrajectory.m_fSecondJumpSpeed;
		m_fTime                     = m_fTime - fTime;
		m_oTrajectory.m_fTotalTime  = m_oTrajectory.m_fTotalTime - fTime;

		if ( m_oTrajectory.m_fHorizontalTime != -1.0f )
			m_oTrajectory.m_fHorizontalTime = m_oTrajectory.m_fHorizontalTime - fTime;

		m_oTrajectory.m_fSecondJumpTime  = -1.0f;
		m_oTrajectory.m_fSecondJumpSpeed = 0.0f;
	}

	// The horizontal move stops or starts
	if ( m_oTrajectory.m_fHorizontalTime != -1.0f && m_fTime >= m_oTrajectory.m_fHorizontalTime )
	{
		GetPosition( m_oTrajectory.m_vStart, m_oTrajectory.m_fHorizontalTime - m_fTime );

		TVector4 vAcceleration;
		vAcceleration.Multiply( m_oTrajectory.m_vAcceleration, 2.0f );

		TVector4 vVelocity;
		vVelocity.Multiply( vAcceleration, m_oTrajectory.m_fHorizontalTime );

		JumpType eJumpType          = m_oTrajectory.m_eJumpType;
		m_oTrajectory.m_vVelocity.x = m_oTrajectory.m_vVelocity.x + vVelocity.x;
		m_oTrajectory.m_vVelocity.y = m_oTrajectory.m_vVelocity.y + vVelocity.y;
		m_oTrajectory.m_vVelocity.z = m_oTrajectory.m_vVelocity.z + vVelocity.z;

		if ( eJumpType == JumpType_MoveThenStop )
		{
			m_oTrajectory.m_vVelocity.x = 0.0f;
			m_oTrajectory.m_vVelocity.z = 0.0f;
		}
		else if ( eJumpType == JumpType_JumpThenMove )
		{
			m_oTrajectory.m_vVelocity.x = m_oTrajectory.m_vHorizontalVelocity.x;
			m_oTrajectory.m_vVelocity.z = m_oTrajectory.m_vHorizontalVelocity.z;
		}

		TFLOAT fTime               = m_oTrajectory.m_fHorizontalTime;
		m_fTime                    = m_fTime - fTime;
		m_oTrajectory.m_fTotalTime = m_oTrajectory.m_fTotalTime - fTime;

		if ( m_oTrajectory.m_fSecondJumpTime != -1.0f )
			m_oTrajectory.m_fSecondJumpTime = m_oTrajectory.m_fSecondJumpTime - fTime;

		m_oTrajectory.m_fHorizontalTime = -1.0f;
	}

	return m_oTrajectory.m_fTotalTime >= m_fTime;
}

// $deBlob: FUNCTION 007d8ac0
TVector4& AAIJumpTrajectory::GetPosition( TVector4& a_rvPosition, TFLOAT a_fTime )
{
	TFLOAT fTime = m_fTime + a_fTime;

	if ( fTime > m_oTrajectory.m_fTotalTime )
		fTime = m_oTrajectory.m_fTotalTime;

	TVector4 vVelocity;
	vVelocity.Multiply( m_oTrajectory.m_vVelocity, fTime );

	TVector4 vAcceleration;
	vAcceleration.Multiply( m_oTrajectory.m_vAcceleration, fTime * fTime );

	a_rvPosition.Set( m_oTrajectory.m_vStart.x + vAcceleration.x + vVelocity.x, m_oTrajectory.m_vStart.y + vAcceleration.y + vVelocity.y, m_oTrajectory.m_vStart.z + vVelocity.z + vAcceleration.z, vAcceleration.w );
	return a_rvPosition;
}

// $deBlob: FUNCTION 007d8b80
TVector4& AAIJumpTrajectory::GetVelocity( TVector4& a_rvVelocity )
{
	TVector4 vAcceleration;
	vAcceleration.Multiply( m_oTrajectory.m_vAcceleration, m_fTime * 2.0f );

	a_rvVelocity.Set( m_oTrajectory.m_vVelocity.x + vAcceleration.x, m_oTrajectory.m_vVelocity.y + vAcceleration.y, m_oTrajectory.m_vVelocity.z + vAcceleration.z, vAcceleration.w );
	return a_rvVelocity;
}

// $deBlob: FUNCTION 007d9110
TBOOL AAIJumpTrajectory::IsValidPosition( TVector4& a_rvPosition, TFLOAT a_fHeight )
{
	TASSERT( m_pOwner );

	TFLOAT fHeight = a_fHeight;

	if ( a_fHeight == -1.0f )
		fHeight = 1.0f;

	TFLOAT fDistance = fHeight;
	a_rvPosition.y   = a_rvPosition.y - fHeight * 0.5f;

	ACollisionObject* pHit = ACollisionManager2::CollideRaySolidFast( a_rvPosition, TVector4::VEC_POSY, fDistance, m_pOwner->GetCollisionObject(), TNULL, 135, TNULL );

	if ( !AGndMotorComponent::IsStandableCollisionObject( pHit, TVector4::VEC_ZERO ) )
		return TFALSE;

	a_rvPosition.y = a_rvPosition.y + fDistance;
	return TTRUE;
}

// $deBlob: FUNCTION 007d8370
TBOOL AAIJumpTrajectory::CheckJumpCollision( ABaseEntity* a_pOwner, TFLOAT a_fTime )
{
	m_pOwner = a_pOwner;
	return CheckJumpCollision( a_fTime );
}

// $deBlob: FUNCTION 007d8250
TBOOL AAIJumpTrajectory::CheckJumpCollision( TFLOAT a_fTime )
{
	TVector4 vPosition;
	GetPosition( vPosition, a_fTime );

	if ( TSTATICCAST( ACharEntity, m_pOwner )->CanOccupyPosition( vPosition ) )
		return TFALSE;

	m_bActive = TFALSE;
	if ( m_oTrajectory.m_fTotalTime * 0.98f < m_fTime )
		return TTRUE;

	return TFALSE;
}

// $deBlob: FUNCTION 007d91f0
TBOOL AAIJumpTrajectory::CheckFullJumpCollision( TUINT32 a_uiFlags, APathfindingComponent::Obstruction* a_pObstruction, void* a_pCollisionCache )
{
	TASSERT( a_pObstruction );
	TASSERT( a_pCollisionCache );

	// The jump is played through and restored afterwards
	const TVector4& rcHalfExtents = m_pOwner->GetBBox();

	TVector4 vSavedAcceleration       = m_oTrajectory.m_vAcceleration;
	JumpType eSavedJumpType           = m_oTrajectory.m_eJumpType;
	TVector4 vSavedVelocity           = m_oTrajectory.m_vVelocity;
	TVector4 vSavedStart              = m_oTrajectory.m_vStart;
	TFLOAT   fSavedTotalTime          = m_oTrajectory.m_fTotalTime;
	TFLOAT   fSavedSecondJumpSpeed    = m_oTrajectory.m_fSecondJumpSpeed;
	TFLOAT   fSavedSecondJumpTime     = m_oTrajectory.m_fSecondJumpTime;
	TFLOAT   fSavedHorizontalTime     = m_oTrajectory.m_fHorizontalTime;
	TVector4 vSavedHorizontalVelocity = m_oTrajectory.m_vHorizontalVelocity;

	TVector4 vPrevious;
	GetPosition( vPrevious, 0.0f );
	vPrevious.y = vPrevious.y - rcHalfExtents.y;

	while ( m_oTrajectory.m_fTotalTime >= m_fTime )
	{
		Update( 0.1f );

		TVector4 vPosition;
		GetPosition( vPosition, 0.0f );
		vPosition.y = vPosition.y - rcHalfExtents.y;

		TVector4 vDirection( vPosition.x - vPrevious.x, vPosition.y - vPrevious.y, vPosition.z - vPrevious.z, vPrevious.w );
		TFLOAT   fDistance     = vDirection.Magnitude();
		TFLOAT   fStepDistance = fDistance;

		if ( fDistance <= TMath::FLOATEPSILON )
			break;

		vDirection.Divide( fDistance );

		TUINT32 uiLayer = 135;

		if ( a_uiFlags & 2 )
			uiLayer = 131;

		if ( a_uiFlags & 4 )
			uiLayer &= ~2;

		if ( a_uiFlags & 8 )
			uiLayer &= ~0x81;

		if ( a_uiFlags & 0x20 )
			uiLayer &= ~4;

		TVector4          vNormal;
		TFLOAT            fStep;
		ACollisionObject* pHit = ACollisionManager2::GetSingleton()->CollideSweptAABB( vPrevious, rcHalfExtents, vDirection, fDistance, vNormal, 0.0f, fStep, TNULL, TNULL, uiLayer, TNULL );

		// Only landing at the end of the jump on standable ground is fine
		if ( pHit != TNULL && ( m_oTrajectory.m_fTotalTime * 0.98f > m_fTime || fStepDistance * 0.5f >= fDistance || !AGndMotorComponent::IsStandableCollisionObject( pHit, TVector4::VEC_ZERO ) ) )
		{
			a_pObstruction->pObject = pHit;
			a_pObstruction->vNormal = vNormal;
			return TFALSE;
		}

		vPrevious = vPosition;
	}

	m_fTime                             = 0.0f;
	m_oTrajectory.m_vAcceleration       = vSavedAcceleration;
	m_oTrajectory.m_vVelocity           = vSavedVelocity;
	m_oTrajectory.m_vStart              = vSavedStart;
	m_oTrajectory.m_fTotalTime          = fSavedTotalTime;
	m_oTrajectory.m_fSecondJumpSpeed    = fSavedSecondJumpSpeed;
	m_oTrajectory.m_fSecondJumpTime     = fSavedSecondJumpTime;
	m_oTrajectory.m_fHorizontalTime     = fSavedHorizontalTime;
	m_oTrajectory.m_vHorizontalVelocity = vSavedHorizontalVelocity;
	m_oTrajectory.m_eJumpType           = eSavedJumpType;

	return TTRUE;
}
