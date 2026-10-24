#pragma once
#include <Toshi/T2DList.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>

class ACollisionFunctor;

//-----------------------------------------------------------------------------
// Purpose: the collision proxy of an entity without collision instance of its own: its owner's bounding box/sphere,
// flagged dirty when the owner moves or resizes. Created by AInvisibleEntity, AProp, ACivSpawner...
// (ABaseEntity::GetCollisionObject).
//-----------------------------------------------------------------------------
class ACollisionObject
    : public Toshi::T2DList<ACollisionObject>::Node
{
public:
	using FLAG = TUINT8;
	enum FLAG_ : FLAG
	{
		FLAG_Created     = BITFLAG( 0 ), // set by the owners once created
		FLAG_AABBDirty   = BITFLAG( 1 ), // the owner's bounding box changed (UpdateAABBExtents)
		FLAG_SphereDirty = BITFLAG( 2 ), // the owner moved (UpdateBoundingSphere)
		FLAG_OwnerType0  = BITFLAG( 3 ),
		FLAG_OwnerType1  = BITFLAG( 4 ),
		FLAG_OwnerType2  = BITFLAG( 5 ),
	};

public:
	ACollisionObject( void* a_pOwner, TINT a_eOwnerType, TUINT8 a_uiGroupMask );
	virtual ~ACollisionObject();

	// NOTE: retail asserts on a vector made of the matrix's m_f44 with x, y and z zeroed (TVector4::MagnitudeSq only
	// uses x, y and z), so the assert can never fail
	// $deBlob: FUNCTION 00774f10
	void SetTransformDirty( const Toshi::TMatrix44& a_rmRHS )
	{
		TASSERT( Toshi::TVector4( 0.0f, 0.0f, 0.0f, a_rmRHS.m_f44 ).MagnitudeSq() <= Toshi::TMath::FLOATEPSILON );
		m_uiFlags |= FLAG_SphereDirty;
	}

	// NOTE: empty in the PC build (recomputes the owner's box); called when FLAG_AABBDirty is set
	// $deBlob: FUNCTION 005a9da0
	void UpdateAABBExtents() {}

	// NOTE: always TFALSE in the PC build (tests the sphere against the owner's box); a_rcSphere is the centre (xyz)
	// and the radius (w)
	// $deBlob: FUNCTION 005a9d70
	TBOOL CollideSphere( const Toshi::TVector4& a_rcSphere, TFLOAT& a_rfDistance, Toshi::TVector4& a_rvOutNormal, TUINT32 a_eGroupFlags, ACollisionFunctor* a_pFunctor ) { return TFALSE; }

	void SetFlags( FLAG a_uiFlags ) { m_uiFlags |= a_uiFlags; }
	// $deBlob: FUNCTION 00774f00
	void SetGroupMask( TUINT8 a_uiGroupMask ) { m_uiGroupMask = a_uiGroupMask; }
	// $deBlob: FUNCTION 007b88e0
	TUINT8 GetGroupMask() const { return m_uiGroupMask; }
	FLAG   GetFlags() const { return m_uiFlags; }
	void*  GetOwner() const { return m_pOwner; }

	// Does the sphere (xyz centre, w radius) touch the box?
	static TBOOL SphereIntersectsAABB( const Toshi::TVector4& a_rvSphere, const Toshi::TVector4& a_rvAABBCentre, const Toshi::TVector4& a_rvAABBHalfExtents );

private:
	void*  m_pOwner;      // 0x0C
	FLAG   m_uiFlags;     // 0x10
	TUINT8 m_uiGroupMask; // 0x11
};                        // 0x14
