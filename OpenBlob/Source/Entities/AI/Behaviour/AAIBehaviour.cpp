#include "pch.h"
#include "AAIBehaviour.h"
#include "Entities/ABaseEntity.h"
#include "Console/AConsoleState.h"
#include "Entities/AGndMotorComponent.h"
#include "Collision/ACollisionManager2.h"
#include "Entities/ACharEntity.h"
#include "Entities/PathFinding/APathfindingComponent.h"
#include "Entities/PathFinding/AAINavigator.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005419f0
// $deBlob: FUNCTION 0085a120
static AConsoleVar s_cvBehaviourShowEnemy( "behaviour_show_enemy", TFALSE ); // 0x009b9018

// $deBlob: FUNCTION 007e1530
AAIBehaviour::AAIBehaviour( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_fScheduleTimer = 0.0f;
	m_iAttackType    = -1;
	m_fAttackedTimer = 0.0f;
	m_pSelectedEnemy = TNULL;
	m_pEnemyPool     = new EnemyPool();

	AConsoleState::RegisterCVar( &s_cvBehaviourShowEnemy );
}

// $deBlob: FUNCTION 007e15f0
// $deBlob: FUNCTION 007e26f0
AAIBehaviour::~AAIBehaviour()
{
	while ( !m_Enemies.IsEmpty() )
	{
		Enemy* pEnemy = m_Enemies.PopBack();
		m_pEnemyPool->FreeObject( pEnemy );
	}

	if ( m_pEnemyPool )
		delete m_pEnemyPool;
}

// $deBlob: FUNCTION 007e1760
void AAIBehaviour::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_fScheduleTimer > 0.0f )
		m_fScheduleTimer -= a_fDeltaTime;

	UpdateMemory( a_fDeltaTime );
}

// $deBlob: FUNCTION 007e1700
void AAIBehaviour::ResetComponent()
{
	m_bEnabled       = TTRUE;
	m_uiFlags        = 0;
	m_pSelectedEnemy = TNULL;
	m_fAttackedTimer = 0.0f;
	m_fScheduleTimer = 0.3f;
	ClearEnemyList();
}

// $deBlob: FUNCTION 007e1730
void AAIBehaviour::OnScheduleFinished( TINT a_iSchedule, TINT a_iResult, TINT a_iTask )
{
	if ( a_iResult == 1 )
		m_fScheduleTimer = GetAttackedTimeOut( a_iSchedule, a_iTask );
}

// $deBlob: FUNCTION 007e1b80
TBOOL AAIBehaviour::EntityGroundPosition( TVector4* a_pPosition, TFLOAT a_fDistance, ACollisionObject* a_pIgnore, TINT a_iUnk4 )
{
	static const TVector4 s_vDown( 0.0f, 1.0f, 0.0f, 1.0f ); // (y is down)

	ACollisionObject* pObject = ACollisionManager2::CollideRaySolidFast( *a_pPosition, s_vDown, a_fDistance, a_pIgnore, TNULL, 0x87, TNULL );

	if ( AGndMotorComponent::IsStandableCollisionObject( pObject, TVector4::VEC_ZERO ) )
	{
		a_pPosition->y += a_fDistance;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007e1cb0
void AAIBehaviour::UpdateMemory( TFLOAT a_fDeltaTime )
{
	if ( m_fAttackedTimer > 0.0f )
	{
		m_fAttackedTimer -= a_fDeltaTime;

		if ( m_fAttackedTimer <= 0.0f )
			BeenAttackedTimedout();
	}
}

// $deBlob: FUNCTION 007e1ce0
void AAIBehaviour::JustBeenAttacked( ABaseEntity* a_pAttacker )
{
	TFLOAT fTime = GetBeenAttackedTime();
	m_uiFlags |= FLAG_BeenAttacked;
	m_fAttackedTimer = fTime;
}

// The closest known enemy entity, TNULL without one
// $deBlob: FUNCTION 007e1e40
ABaseEntity* AAIBehaviour::GetClosestEnemy( TBOOL a_bCurrent )
{
	Enemy* pEnemy = GetClosestEnemyNode( a_bCurrent );
	return ( pEnemy != TNULL ) ? pEnemy->m_pEnemy : TNULL;
}

// $deBlob: FUNCTION 007e1e60
TFLOAT AAIBehaviour::CalculateEnemyThreat( Enemy* a_pEnemy, TFLOAT a_fMaxDistance, TFLOAT a_fMaxFrustration )
{
	TFLOAT fThreat = 1.0f - a_pEnemy->m_fDistanceSq / a_fMaxDistance;
	return TMath::Max( 0.0f, fThreat );
}

// $deBlob: FUNCTION 007e22d0
TBOOL AAIBehaviour::GetAttackPoint( ABaseEntity* a_pEntity, TVector4* a_pPoint, TFLOAT a_fGroundDistance )
{
	TASSERT( a_pEntity != TNULL );

	if ( a_pEntity->IsA( &TGetClass( ACharEntity ) ) )
	{
		// A character in the air: the ground under it
		AGndMotorComponent* pMotor    = TSTATICCAST( AGndMotorComponent, TSTATICCAST( ACharEntity, a_pEntity )->GetMotorComponent() );
		TBOOL               bOnGround = pMotor->IsOnGround( 0.03f );
		*a_pPoint                     = a_pEntity->GetMatrix().GetTranslation();

		if ( !bOnGround && !EntityGroundPosition( a_pPoint, a_fGroundDistance, a_pEntity->GetCollisionObject(), 0 ) )
			return TFALSE;
	}
	else
	{
		*a_pPoint = a_pEntity->GetMatrix().GetTranslation();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 007e1ea0
TBOOL AAIBehaviour::IsKnownEnemy( ABaseEntity* a_pEntity )
{
	for ( auto it = m_Enemies.Begin(); it != m_Enemies.End(); it++ )
	{
		if ( it->m_pEnemy == a_pEntity )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007e1f30
void AAIBehaviour::AddToEnemyList( ABaseEntity* a_pEntity )
{
	TASSERT( !IsKnownEnemy( a_pEntity ) );

	Enemy* pEnemy;

	if ( m_Enemies.Size() == MAX_ENEMIES )
	{
		// The last (least threatening) one is reused
		pEnemy = m_Enemies.PopBack();
	}
	else
	{
		pEnemy = m_pEnemyPool->NewObject();
	}

	pEnemy->m_pEnemy        = a_pEntity;
	pEnemy->m_bNew          = TTRUE;
	pEnemy->m_fLostTime     = 0.0f;
	pEnemy->m_fDistanceSq   = TVector4::DistanceSq( m_pEntity->GetMatrix().GetTranslation(), a_pEntity->GetMatrix().GetTranslation() );
	pEnemy->m_fThreatRating = 0.0f;
	m_Enemies.Push( pEnemy );

	AddedNewEnemy( pEnemy );
}

// $deBlob: FUNCTION 007e2220
void AAIBehaviour::ClearEnemyList()
{
	while ( !m_Enemies.IsEmpty() )
	{
		Enemy* pEnemy = m_Enemies.PopBack();
		m_pEnemyPool->FreeObject( pEnemy );
	}
}

// $deBlob: FUNCTION 007e2050
AAIBehaviour::Enemy* AAIBehaviour::FindInEnemyList( ABaseEntity* a_pEnemy )
{
	TASSERT( a_pEnemy );

	for ( auto it = m_Enemies.Begin(); it != m_Enemies.End(); it++ )
	{
		if ( it->m_pEnemy == a_pEnemy )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 007e2180
void AAIBehaviour::RemoveFromEnemyList( Enemy* a_pEnemy )
{
	TASSERT( a_pEnemy );

	a_pEnemy->Remove();
	m_pEnemyPool->FreeObject( a_pEnemy );

	// NOTE: retail reads the freed node (the pool doesn't touch m_pEnemy)
	if ( m_pSelectedEnemy == a_pEnemy->m_pEnemy )
		SetSelectedEnemy( TNULL );
}

// $deBlob: FUNCTION 007e1d10
AAIBehaviour::Enemy* AAIBehaviour::GetClosestEnemyNode( TBOOL a_bCurrent )
{
	Enemy* pClosest = TNULL;
	TFLOAT fMinDist = TMath::MAXFLOAT;

	if ( a_bCurrent == TTRUE )
	{
		for ( auto it = m_Enemies.Begin(); it != m_Enemies.End(); it++ )
		{
			TFLOAT fDistSq = TVector4::DistanceSq( m_pEntity->GetMatrix().GetTranslation(), it->m_pEnemy->GetMatrix().GetTranslation() );

			if ( fDistSq < fMinDist )
			{
				pClosest = it;
				fMinDist = fDistSq;
			}
		}

		return pClosest;
	}

	for ( auto it = m_Enemies.Begin(); it != m_Enemies.End(); it++ )
	{
		if ( it->m_fDistanceSq < fMinDist )
		{
			pClosest = it;
			fMinDist = it->m_fDistanceSq;
		}
	}

	return pClosest;
}

// $deBlob: FUNCTION 007e1960
TFLOAT AAIBehaviour::GetDistanceSqToClosestEnemy( TBOOL a_bCurrent )
{
	Enemy* pEnemy = GetClosestEnemyNode( a_bCurrent );

	if ( pEnemy != TNULL && pEnemy->m_pEnemy != TNULL )
		return TVector4::DistanceSq( m_pEntity->GetMatrix().GetTranslation(), pEnemy->m_pEnemy->GetMatrix().GetTranslation() );

	return -1.0f;
}

// $deBlob: FUNCTION 007e1880
TBOOL AAIBehaviour::WithinAttackRange( const TVector4& a_rcPosition, TFLOAT a_fRadius, TBOOL a_bOptimal, TINT a_iAttackType )
{
	TINT iAttackType = a_iAttackType;

	if ( iAttackType == -1 )
	{
		iAttackType = m_iAttackType;

		if ( iAttackType == -1 )
			return TFALSE;
	}

	TFLOAT fMin;
	TFLOAT fMax;

	if ( GetAttackRange( &fMin, &fMax, iAttackType ) )
	{
		if ( a_bOptimal )
			fMax = GetAttackRangeOptimal();

		if ( fMin != fMax || fMax != -1.0f )
		{
			TFLOAT fDistance = TVector4::Distance( m_pEntity->GetMatrix().GetTranslation(), a_rcPosition ) - a_fRadius - m_pEntity->GetBBox().z;

			if ( fDistance <= fMax && fMin <= fDistance )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007e2110
TBOOL AAIBehaviour::WithinRange( const TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fMin, TFLOAT a_fMax )
{
	if ( a_fMin == a_fMax && a_fMax == -1.0f )
		return TFALSE;

	TFLOAT fDistance = TVector4::Distance( m_pEntity->GetMatrix().GetTranslation(), a_rcPosition ) - a_fRadius - m_pEntity->GetBBox().z;
	return a_fMax >= fDistance && fDistance >= a_fMin;
}

// $deBlob: FUNCTION 007e17b0
TBOOL AAIBehaviour::GetOptimalAttackPosition( TVector4& a_rvOutPosition )
{
	ABaseEntity* pEnemy = m_pSelectedEnemy;

	if ( pEnemy == TNULL )
		return TFALSE;

	const TVector4& rcOwnerPos = m_pEntity->GetMatrix().GetTranslation();
	const TVector4& rcEnemyPos = pEnemy->GetMatrix().GetTranslation();

	TVector4 vOffset( rcOwnerPos.x - rcEnemyPos.x, rcOwnerPos.y - rcEnemyPos.y, rcOwnerPos.z - rcEnemyPos.z, rcOwnerPos.w );
	vOffset.NormaliseXZ();
	vOffset.Multiply( GetAttackRangeOptimal() );

	a_rvOutPosition.x = rcEnemyPos.x + vOffset.x;
	a_rvOutPosition.y = rcEnemyPos.y + vOffset.y;
	a_rvOutPosition.z = rcEnemyPos.z + vOffset.z;
	a_rvOutPosition.w = rcEnemyPos.w;

	return TTRUE;
}

// $deBlob: FUNCTION 007e1a00
TBOOL AAIBehaviour::FaceEntity( ABaseEntity* a_pEntity, TBOOL a_bInstant )
{
	if ( a_pEntity == TNULL )
		return TFALSE;

	const TVector4& rcEntityPos = a_pEntity->GetMatrix().GetTranslation();
	const TVector4& rcOwnerPos  = m_pEntity->GetMatrix().GetTranslation();

	TVector4 vFacing( rcEntityPos.x - rcOwnerPos.x, rcEntityPos.y - rcOwnerPos.y, rcEntityPos.z - rcOwnerPos.z, rcEntityPos.w );
	vFacing.NormaliseXZ();

	ACharEntity* pOwner = GetCharOwner();

	if ( a_bInstant )
	{
		TSTATICCAST( AGndMotorComponent, pOwner->GetMotorComponent() )->SetFacing( vFacing );
		return TTRUE;
	}

	return !TSTATICCAST( APathfindingComponent, pOwner->GetPathfinding() )->GetNavigator()->FaceDirection( vFacing );
}

// $deBlob: FUNCTION 007e1ab0
TBOOL AAIBehaviour::FaceSelectedEnemy( TBOOL a_bUnk )
{
	ABaseEntity* pEnemy = m_pSelectedEnemy;

	if ( pEnemy == TNULL )
		return TFALSE;

	const TVector4& rcEnemyPos = pEnemy->GetMatrix().GetTranslation();
	const TVector4& rcOwnerPos = m_pEntity->GetMatrix().GetTranslation();

	TVector4 vFacing( rcEnemyPos.x - rcOwnerPos.x, rcEnemyPos.y - rcOwnerPos.y, rcEnemyPos.z - rcOwnerPos.z, rcEnemyPos.w );
	vFacing.NormaliseXZ();

	// NOTE: a_bUnk isn't used
	return !TSTATICCAST( APathfindingComponent, GetCharOwner()->GetPathfinding() )->GetNavigator()->FaceDirection( vFacing );
}

// $deBlob: FUNCTION 007e1be0
TBOOL AAIBehaviour::IsSelectedEnemyNew()
{
	Enemy* pEnemyNode = FindInEnemyList( m_pSelectedEnemy );
	TASSERT( TNULL != pEnemyNode );

	return pEnemyNode->m_bNew;
}

// $deBlob: FUNCTION 007e1c30
void AAIBehaviour::ClearSelectedEnemyNew()
{
	Enemy* pEnemyNode = FindInEnemyList( m_pSelectedEnemy );
	TASSERT( TNULL != pEnemyNode );

	pEnemyNode->m_bNew = TFALSE;
}

// $deBlob: FUNCTION 007e1c80
void AAIBehaviour::SetTaunted()
{
	m_uiFlags |= FLAG_Taunted;
}

// $deBlob: FUNCTION 007e23e0
ACharEntity* AAIBehaviour::GetCharOwner()
{
	TASSERT( m_pEntity->IsA( &TGetClass( ACharEntity ) ) );
	return TSTATICCAST( ACharEntity, m_pEntity );
}
