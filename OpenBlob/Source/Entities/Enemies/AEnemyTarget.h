#pragma once
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

class ABaseEntity;
class AEnemy;

//-----------------------------------------------------------------------------
// Purpose: a target of the enemies (Enemies\AEnemyTarget.cpp, a virtual destructor): one per player blob
// (AEnemyManager::AddTarget). The enemies attacking it are its targetters; they surround it in slots placed on three
// rings (ranges) around it, and one of them at a time can reserve it (the attacker).
//-----------------------------------------------------------------------------
class AEnemyTarget
    : public Toshi::T2DList<AEnemyTarget>::Node
{
public:
	// The rings around the target (GetRangeType) and the list of the targetters without a slot
	enum RangeType : TINT
	{
		RANGE_Invalid    = -1,
		RANGE_Near       = 0, // 0..2, 4 slots
		RANGE_Mid        = 1, // 2..5, 6 slots
		RANGE_Far        = 2, // 5..15, 8 slots
		RANGE_Targetters = 3, // the targetters that don't surround the target
		NUM_RANGES       = 3,
		NUM_SLOT_LISTS   = 4,
	};

	// A position on a ring (0x08 bytes)
	struct SlotData
	{
		TINT m_eRange; // 0x00 RangeType
		TINT m_iIndex; // 0x04 0..the ring's slot count
	};

	// A slot of the pool (0x18 bytes)
	class Slot
	    : public Toshi::T2DList<Slot>::Node
	{
	public:
		Slot()
		    : m_pOwner( TNULL ), m_fTime( 0.0f )
		{}

	public:
		AEnemy*  m_pOwner; // 0x08
		TFLOAT   m_fTime;  // 0x0C counted down while it's owned
		SlotData m_oData;  // 0x10
	};

	// A ring (0x0C bytes)
	struct RangeInfo
	{
		TFLOAT m_fMin;      // 0x00
		TFLOAT m_fMax;      // 0x04
		TINT   m_iNumSlots; // 0x08
	};

public:
	AEnemyTarget( ABaseEntity* a_pTarget );
	virtual ~AEnemyTarget(); // +0x00

	// Fills the pool of the free slots with a_iCount slots
	static void CreateSlotPool( TINT a_iCount );
	static void DestroySlotPool();

	void Tick( TFLOAT a_fDeltaTime );
	void UpdateReservation( TFLOAT a_fDeltaTime );
	void UpdateSlots( TFLOAT a_fDeltaTime );

	// Only one enemy at a time can reserve the target (for a_fTime seconds)
	TBOOL ReserveTarget( AEnemy* a_pEnemy, TFLOAT a_fTime );
	void  UnreserveTarget( AEnemy* a_pEnemy );

	void  AddTargetter( AEnemy* a_pEnemy );
	void  RemoveTargetter( AEnemy* a_pEnemy );
	Slot* FindTargetter( AEnemy* a_pEnemy );
	// The targetters of the type (AEnemyManager::EnemyType), all of them with -1
	TINT GetTargetterCount( TINT a_eType );

	// Gives a_pSurrounder the free slot closest to the direction it comes from, at a_fOptimalDistance (a_rvOutPoint):
	// 0 got one, 3 none is free
	TINT GetSurroundPoint( AEnemy* a_pSurrounder, TFLOAT a_fOptimalDistance, Toshi::TVector4& a_rvOutPoint, TFLOAT a_fTime );
	// Back to the targetters without a slot
	void ReleaseSurroundPoint( AEnemy* a_pEnemy );

	Toshi::TVector4 GetPosition() const;
	ABaseEntity*    GetTarget() const { return m_pTarget; }
	AEnemy*         GetReservedBy() const { return m_pReservedBy; }

	static void            FreeUsedSlot( Slot* a_pSlot );
	static void            ConvertToSlot( const Toshi::TVector4& a_rcOffset, SlotData& a_rOutData );
	static TINT            GetRangeType( TFLOAT a_fDistance );
	static Toshi::TVector4 GetOffsetDirection( const SlotData& a_rcData );
	static TINT            Mod( TINT a_iValue, TINT a_iMod );

private:
	Slot* GetClosestFreeSlot( const SlotData& a_rcData, AEnemy* a_pEnemy, TFLOAT a_fOptimalDistance, Toshi::TVector4& a_rvOutPoint );
	Slot* CheckPosition( const SlotData& a_rcData, AEnemy* a_pEnemy, TFLOAT a_fOptimalDistance, Toshi::TVector4& a_rvOutPoint );
	TBOOL CheckSlotOwner( const SlotData& a_rcData, AEnemy* a_pEnemy );
	TBOOL CheckGround( const SlotData& a_rcData, AEnemy* a_pEnemy, TFLOAT a_fOptimalDistance, Toshi::TVector4& a_rvOutPoint );
	TBOOL CheckLOS( AEnemy* a_pEnemy, const Toshi::TVector4& a_rcPoint );

public:
	static const RangeInfo ms_aRanges[ NUM_RANGES ];

private:
	static Toshi::T2DList<Slot> ms_FreeSlots;

private:
	ABaseEntity*         m_pTarget;                  // 0x0C the player blob
	Toshi::TVector4      m_vUnk10;                   // 0x10
	AEnemy*              m_pReservedBy;              // 0x20
	TFLOAT               m_fReserveTime;             // 0x24
	Toshi::T2DList<Slot> m_aSlots[ NUM_SLOT_LISTS ]; // 0x28 RangeType
};
