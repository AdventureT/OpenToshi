#include "pch.h"
#include "AEnemyTarget.h"
#include "AEnemy.h"
#include "AEnemyBehaviour.h"
#include "Entities/AGndMotorComponent.h"
#include "Collision/ACollisionManager2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

Toshi::T2DList<AEnemyTarget::Slot> AEnemyTarget::ms_FreeSlots;

TSTATICASSERT( sizeof( AEnemyTarget ) == 0x48 );
TSTATICASSERT( sizeof( AEnemyTarget::Slot ) == 0x18 );

const AEnemyTarget::RangeInfo AEnemyTarget::ms_aRanges[ NUM_RANGES ] = {
	{ 0.0f, 2.0f, 4 },
	{ 2.0f, 5.0f, 6 },
	{ 5.0f, 15.0f, 8 },
};

// The direction of the slots with index 0
static const TVector4 s_vSlotForward( 0.0f, 0.0f, 1.0f, 1.0f );

// $deBlob: FUNCTION 007cdb10
AEnemyTarget::AEnemyTarget( ABaseEntity* a_pTarget )
    : m_pTarget( a_pTarget )
{
	m_vUnk10       = TVector4::VEC_ZERO;
	m_pReservedBy  = TNULL;
	m_fReserveTime = 0.0f;
}

// $deBlob: FUNCTION 007cdba0
// $deBlob: FUNCTION 007cebb0
AEnemyTarget::~AEnemyTarget()
{
	// NOTE: the slots are deleted, not given back to the pool
	for ( TINT i = 0; i < NUM_SLOT_LISTS; i++ )
		m_aSlots[ i ].DeleteAll();
}

// $deBlob: FUNCTION 007ce070
void AEnemyTarget::CreateSlotPool( TINT a_iCount )
{
	for ( ; a_iCount >= 1; a_iCount-- )
		ms_FreeSlots.PushBack( new Slot() );
}

// $deBlob: FUNCTION 007ce0c0
// $deBlob: FUNCTION 005418d0
// $deBlob: FUNCTION 0085a0b0
void AEnemyTarget::DestroySlotPool()
{
	ms_FreeSlots.DeleteAll();
}

// $deBlob: FUNCTION 007cdc60
void AEnemyTarget::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pReservedBy != TNULL )
	{
		m_fReserveTime -= a_fDeltaTime;

		if ( m_fReserveTime <= 0.0f )
		{
			m_pReservedBy  = TNULL;
			m_fReserveTime = 0.0f;
		}
	}

	UpdateSlots( a_fDeltaTime );
}

// $deBlob: FUNCTION 007ce0e0
void AEnemyTarget::UpdateReservation( TFLOAT a_fDeltaTime )
{
	if ( m_pReservedBy != TNULL )
	{
		m_fReserveTime -= a_fDeltaTime;

		if ( m_fReserveTime <= 0.0f )
		{
			m_pReservedBy  = TNULL;
			m_fReserveTime = 0.0f;
		}
	}
}

// $deBlob: FUNCTION 007ce120
void AEnemyTarget::UpdateSlots( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < NUM_RANGES; i++ )
	{
		for ( auto it = m_aSlots[ i ].Begin(); it != m_aSlots[ i ].End(); it++ )
		{
			if ( it->m_pOwner != TNULL )
				it->m_fTime -= a_fDeltaTime;
		}
	}
}

// $deBlob: FUNCTION 007ce020
TBOOL AEnemyTarget::ReserveTarget( AEnemy* a_pEnemy, TFLOAT a_fTime )
{
	if ( m_pReservedBy == TNULL )
	{
		m_pReservedBy  = a_pEnemy;
		m_fReserveTime = a_fTime;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007ce050
void AEnemyTarget::UnreserveTarget( AEnemy* a_pEnemy )
{
	if ( m_pReservedBy == a_pEnemy )
	{
		m_pReservedBy  = TNULL;
		m_fReserveTime = 0.0f;
	}
}

// $deBlob: FUNCTION 007ce900
void AEnemyTarget::FreeUsedSlot( Slot* a_pSlot )
{
	if ( a_pSlot != TNULL )
	{
		a_pSlot->m_pOwner         = TNULL;
		a_pSlot->m_fTime          = 0.0f;
		a_pSlot->m_oData.m_eRange = RANGE_Invalid;
		a_pSlot->m_oData.m_iIndex = 0;
		a_pSlot->Remove();
		ms_FreeSlots.PushFront( a_pSlot );
	}
}

// $deBlob: FUNCTION 007cdd80
void AEnemyTarget::AddTargetter( AEnemy* a_pEnemy )
{
	if ( FindTargetter( a_pEnemy ) == TNULL && !ms_FreeSlots.IsEmpty() )
	{
		Slot* pSlot = ms_FreeSlots.PopFront();

		if ( pSlot != TNULL )
		{
			pSlot->m_oData.m_eRange = RANGE_Targetters;
			pSlot->m_oData.m_iIndex = 0;
			pSlot->m_pOwner         = a_pEnemy;
			pSlot->m_fTime          = 0.0f;
			m_aSlots[ RANGE_Targetters ].PushFront( pSlot );
		}
	}
}

// $deBlob: FUNCTION 007cdde0
void AEnemyTarget::RemoveTargetter( AEnemy* a_pEnemy )
{
	if ( m_pReservedBy == a_pEnemy )
	{
		m_pReservedBy  = TNULL;
		m_fReserveTime = 0.0f;
	}

	Slot* pSlot = FindTargetter( a_pEnemy );

	if ( pSlot != TNULL )
	{
		FreeUsedSlot( pSlot );

		// NOTE: retail moves the freed slot to the front of the pool a second time (no effect)
		pSlot->Remove();
		ms_FreeSlots.PushFront( pSlot );
	}
}

// $deBlob: FUNCTION 007ce1d0
AEnemyTarget::Slot* AEnemyTarget::FindTargetter( AEnemy* a_pEnemy )
{
	for ( TINT i = 0; i < NUM_SLOT_LISTS; i++ )
	{
		for ( auto it = m_aSlots[ i ].Begin(); it != m_aSlots[ i ].End(); it++ )
		{
			if ( it->m_pOwner == a_pEnemy )
				return it;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 007cdcb0
TINT AEnemyTarget::GetTargetterCount( TINT a_eType )
{
	TINT iCount = 0;

	if ( a_eType != -1 )
	{
		for ( TINT i = 0; i < NUM_SLOT_LISTS; i++ )
		{
			for ( auto it = m_aSlots[ i ].Begin(); it != m_aSlots[ i ].End(); it++ )
			{
				if ( it->m_pOwner->GetEnemyType() == a_eType )
					iCount++;
			}
		}

		return iCount;
	}

	for ( TINT i = 0; i < NUM_SLOT_LISTS; i++ )
		iCount += m_aSlots[ i ].Size();

	return iCount;
}

// $deBlob: FUNCTION 007cde50
TINT AEnemyTarget::GetSurroundPoint( AEnemy* a_pSurrounder, TFLOAT a_fOptimalDistance, TVector4& a_rvOutPoint, TFLOAT a_fTime )
{
	TASSERT( a_fOptimalDistance > 0 );
	TASSERT( a_pSurrounder );

	const TVector4& rcSurrounderPos = a_pSurrounder->GetMatrix().GetTranslation();
	const TVector4& rcTargetPos     = m_pTarget->GetMatrix().GetTranslation();

	a_rvOutPoint.x = rcSurrounderPos.x - rcTargetPos.x;
	a_rvOutPoint.y = rcSurrounderPos.y - rcTargetPos.y;
	a_rvOutPoint.z = rcSurrounderPos.z - rcTargetPos.z;
	a_rvOutPoint.w = rcSurrounderPos.w;
	a_rvOutPoint.NormaliseXZ();
	a_rvOutPoint.Multiply( a_fOptimalDistance );

	SlotData oData;
	ConvertToSlot( a_rvOutPoint, oData );

	Slot* pOptimalSlot = GetClosestFreeSlot( oData, a_pSurrounder, a_fOptimalDistance, a_rvOutPoint );

	if ( pOptimalSlot == TNULL )
		return 3;

	TASSERT( TFALSE == pOptimalSlot->IsLinked() );
	pOptimalSlot->m_fTime = a_fTime;
	m_aSlots[ pOptimalSlot->m_oData.m_eRange ].PushFront( pOptimalSlot );

	return 0;
}

// $deBlob: FUNCTION 007cdfc0
void AEnemyTarget::ReleaseSurroundPoint( AEnemy* a_pEnemy )
{
	if ( TSTATICCAST( AEnemyBehaviour, a_pEnemy->GetBehaviourComponent() )->GetTargetChar() != TNULL )
	{
		Slot* pSlot = FindTargetter( a_pEnemy );

		if ( pSlot != TNULL )
		{
			pSlot->m_oData.m_eRange = RANGE_Targetters;
			pSlot->m_oData.m_iIndex = 0;
			pSlot->m_fTime          = 0.0f;
			pSlot->Remove();
			m_aSlots[ pSlot->m_oData.m_eRange ].PushFront( pSlot );
		}
	}
}

// $deBlob: FUNCTION 007ce770
TVector4 AEnemyTarget::GetPosition() const
{
	return m_pTarget->GetMatrix().GetTranslation();
}

// $deBlob: FUNCTION 007ce790
void AEnemyTarget::ConvertToSlot( const TVector4& a_rcOffset, SlotData& a_rOutData )
{
	// NOTE: retail computes the distance twice (the first result isn't used)
	TFLOAT fDistance = a_rcOffset.MagnitudeXZ();

	a_rOutData.m_eRange = GetRangeType( fDistance );

	TINT iNumSlots      = ms_aRanges[ a_rOutData.m_eRange ].m_iNumSlots;
	a_rOutData.m_iIndex = TINT( s_vSlotForward.GetAngleBetweenVector( a_rcOffset ) / ( TMath::TWO_PI / TFLOAT( iNumSlots ) ) );
}

// $deBlob: FUNCTION 007ce520
TINT AEnemyTarget::GetRangeType( TFLOAT a_fDistance )
{
	for ( TINT i = 0; i < NUM_RANGES; i++ )
	{
		if ( ms_aRanges[ i ].m_fMin <= a_fDistance && a_fDistance <= ms_aRanges[ i ].m_fMax )
			return i;
	}

	return RANGE_Far;
}

// $deBlob: FUNCTION 007ce560
TVector4 AEnemyTarget::GetOffsetDirection( const SlotData& a_rcData )
{
	TVector4 vDirection = s_vSlotForward;
	vDirection.RotateY( ( TMath::TWO_PI / TFLOAT( ms_aRanges[ a_rcData.m_eRange ].m_iNumSlots ) ) * TFLOAT( a_rcData.m_iIndex ) );
	vDirection.NormaliseXZ();

	return vDirection;
}

// $deBlob: FUNCTION 007ce960
TINT AEnemyTarget::Mod( TINT a_iValue, TINT a_iMod )
{
	if ( a_iValue >= 0 )
		return a_iValue % a_iMod;

	return a_iMod + a_iValue % a_iMod;
}

// $deBlob: FUNCTION 007ce5c0
AEnemyTarget::Slot* AEnemyTarget::GetClosestFreeSlot( const SlotData& a_rcData, AEnemy* a_pEnemy, TFLOAT a_fOptimalDistance, TVector4& a_rvOutPoint )
{
	Slot* pSlot = FindTargetter( a_pEnemy );

	if ( pSlot != TNULL )
	{
		pSlot->m_pOwner         = TNULL;
		pSlot->m_fTime          = 0.0f;
		pSlot->m_oData.m_eRange = RANGE_Invalid;
		pSlot->m_oData.m_iIndex = 0;
		pSlot->Remove();
		ms_FreeSlots.PushFront( pSlot );
	}

	pSlot = CheckPosition( a_rcData, a_pEnemy, a_fOptimalDistance, a_rvOutPoint );

	if ( pSlot != TNULL )
		return pSlot;

	// The other slots of the ring, closest first, alternating both sides
	TINT iNumSlots = ms_aRanges[ a_rcData.m_eRange ].m_iNumSlots;
	TINT iNumTries = TINT( TFLOAT( iNumSlots ) * 0.5f + 0.5f );

	SlotData oLeft  = a_rcData;
	SlotData oRight = a_rcData;

	for ( TINT i = 0; i < iNumTries; i++ )
	{
		oLeft.m_iIndex = Mod( a_rcData.m_iIndex + 1 + i, iNumSlots );
		pSlot          = CheckPosition( oLeft, a_pEnemy, a_fOptimalDistance, a_rvOutPoint );

		if ( pSlot != TNULL )
			return pSlot;

		oRight.m_iIndex = Mod( a_rcData.m_iIndex - i - 1, iNumSlots );
		pSlot           = CheckPosition( oRight, a_pEnemy, a_fOptimalDistance, a_rvOutPoint );

		if ( pSlot != TNULL )
			return pSlot;
	}

	return TNULL;
}

// $deBlob: FUNCTION 007ce280
AEnemyTarget::Slot* AEnemyTarget::CheckPosition( const SlotData& a_rcData, AEnemy* a_pEnemy, TFLOAT a_fOptimalDistance, TVector4& a_rvOutPoint )
{
	if ( CheckSlotOwner( a_rcData, a_pEnemy ) && CheckGround( a_rcData, a_pEnemy, a_fOptimalDistance, a_rvOutPoint ) && CheckLOS( a_pEnemy, a_rvOutPoint ) )
	{
		if ( ms_FreeSlots.IsEmpty() )
			return TNULL;

		Slot* pSlot = ms_FreeSlots.PopFront();

		if ( pSlot != TNULL )
		{
			pSlot->m_oData  = a_rcData;
			pSlot->m_pOwner = a_pEnemy;
			pSlot->m_fTime  = 0.0f;
		}

		return pSlot;
	}

	return TNULL;
}

// $deBlob: FUNCTION 007ce830
TBOOL AEnemyTarget::CheckSlotOwner( const SlotData& a_rcData, AEnemy* a_pEnemy )
{
	T2DList<Slot>& rSlots = m_aSlots[ a_rcData.m_eRange ];

	for ( auto it = rSlots.Begin(); it != rSlots.End(); it++ )
	{
		if ( it->m_oData.m_eRange == a_rcData.m_eRange && it->m_oData.m_iIndex == a_rcData.m_iIndex )
			return it->m_pOwner == a_pEnemy;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 007ce310
TBOOL AEnemyTarget::CheckGround( const SlotData& a_rcData, AEnemy* a_pEnemy, TFLOAT a_fOptimalDistance, TVector4& a_rvOutPoint )
{
	static const TVector4 s_vRayDirection( 0.0f, 1.0f, 0.0f, 1.0f );

	TVector4 vDirection = s_vSlotForward;
	vDirection.RotateY( ( TMath::TWO_PI / TFLOAT( ms_aRanges[ a_rcData.m_eRange ].m_iNumSlots ) ) * TFLOAT( a_rcData.m_iIndex ) );
	vDirection.NormaliseXZ();

	TVector4 vOffset;
	vOffset.Multiply( vDirection, a_fOptimalDistance );

	const TVector4& rcTargetPos = m_pTarget->GetMatrix().GetTranslation();

	a_rvOutPoint.x = rcTargetPos.x + vOffset.x;
	a_rvOutPoint.y = rcTargetPos.y + vOffset.y;
	a_rvOutPoint.z = rcTargetPos.z + vOffset.z;
	a_rvOutPoint.w = rcTargetPos.w;
	a_rvOutPoint.y -= 0.5f;

	TFLOAT            fDistance = 1.0f;
	ACollisionObject* pObject   = ACollisionManager2::CollideRaySolidFast( a_rvOutPoint, s_vRayDirection, fDistance, a_pEnemy->GetCollisionObject(), TNULL, 0x87, TNULL );

	if ( AGndMotorComponent::IsStandableCollisionObject( pObject, TVector4::VEC_ZERO ) )
	{
		a_rvOutPoint.y += fDistance;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007ce450
TBOOL AEnemyTarget::CheckLOS( AEnemy* a_pEnemy, const TVector4& a_rcPoint )
{
	TVector4 vStart = a_rcPoint;
	vStart.y        = a_rcPoint.y - a_pEnemy->GetBBox().y * 2.0f;

	const TVector4& rcTargetPos = m_pTarget->GetMatrix().GetTranslation();

	TVector4 vDirection;
	vDirection.w = rcTargetPos.w;
	vDirection.x = rcTargetPos.x - vStart.x;
	vDirection.z = rcTargetPos.z - vStart.z;
	vDirection.y = ( rcTargetPos.y - m_pTarget->GetBBox().y ) - vStart.y;

	TFLOAT fDistance = vDirection.Magnitude();
	vDirection.Divide( fDistance );

	return ACollisionManager2::CollideRaySolidFast( vStart, vDirection, fDistance, a_pEnemy->GetCollisionObject(), TNULL, 0x83, TNULL ) == TNULL;
}
