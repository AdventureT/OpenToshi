#pragma once
#include "Collision/ACollisionFunctor.h"

#include <Toshi/T2Vector.h>
#include <Math/TVector4.h>

class ABaseEntity;
class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: moves an entity's bounding box through the collision world, sliding along what it hits.
// de Blob keeps up to 5 touches inline
//-----------------------------------------------------------------------------
class AMoveHelper
{
public:
	// An object touched by a move (0x14 bytes)
	struct ObjectTouched
	{
		ACollisionObject* pObject; // 0x00
		Toshi::TVector4   vNormal; // 0x04
	};

	static TCONSTEXPR_VAR TINT MAX_TOUCHES = 5;
	using TouchList                        = Toshi::T2Vector<ObjectTouched, MAX_TOUCHES>;

	// The functor of PerformClampedMove's sweep (0xC bytes)
	class MoveHelperFunctor
	    : public ACollisionFunctor
	{
	public:
		MoveHelperFunctor( TouchList* a_pTouches, ACollisionObject* a_pIgnore )
		    : m_pTouches( a_pTouches ), m_pIgnore( a_pIgnore )
		{}

		virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override;  // +0x10
		virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) override; // +0x18
		// $deBlob: FUNCTION 007ea1a0
		// $deBlob: FUNCTION 007ea180
		virtual ~MoveHelperFunctor() override = default; // +0x20

	private:
		TouchList*        m_pTouches; // 0x4
		ACollisionObject* m_pIgnore;  // 0x8
	};

	// The functor of SlideMove's sweeps (0x10 bytes)
	class SlideCollisionFunctor
	    : public ACollisionFunctor
	{
	public:
		SlideCollisionFunctor( TouchList* a_pTouches, ACollisionObject* a_pIgnore )
		    : m_pTouches( a_pTouches ), m_pIgnore( a_pIgnore ), m_fBottom( 0.0f )
		{}

		virtual TBOOL PreFilterSweptAABBPoly( const Toshi::TVector4& a_rNormal, const Toshi::TVector4* a_pTriangle ) override; // +0x00
		virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override;                                               // +0x10
		virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) override;                                              // +0x18
		// $deBlob: FUNCTION 007ea1d0
		// $deBlob: FUNCTION 007ea190
		virtual ~SlideCollisionFunctor() override = default; // +0x20

		void SetBottom( TFLOAT a_fBottom ) { m_fBottom = a_fBottom; }

	private:
		TouchList*        m_pTouches; // 0x4
		ACollisionObject* m_pIgnore;  // 0x8
		TFLOAT            m_fBottom;  // 0xC the bottom of the box (y is down)
	};

public:
	// Moves a_pEntity's box by a_rcMove (stepping up onto low obstacles while on the ground, then sliding);
	// a_rvOutPosition is the new position, returns the move that was done; a_bProject: the slides are projected onto
	// the plane of a_rcProjectNormal; a_bNoCollision: moves without any collision test
	static Toshi::TVector4 PerformClampedMove( ABaseEntity* a_pEntity, const Toshi::TVector4& a_rcMove, Toshi::TVector4& a_rvOutPosition, TouchList* a_pTouches, TBOOL a_bProject, const Toshi::TVector4& a_rcProjectNormal, ACollisionObject* a_pIgnore, TBOOL a_bNoCollision, void* a_pPhantom );

	// Adds a_pObject to a_pTouches (if not in it yet and there's room)
	static void AddTouchedObject( TouchList* a_pTouches, ACollisionObject* a_pObject, const Toshi::TVector4& a_rcNormal );

private:
	static TBOOL SlideMove( ABaseEntity* a_pEntity, const Toshi::TVector4& a_rcStart, Toshi::TVector4& a_rvMove, Toshi::TVector4& a_rvOutPosition, TouchList* a_pTouches, TBOOL a_bProject, const Toshi::TVector4& a_rcProjectNormal, ACollisionObject* a_pIgnore, void* a_pPhantom );
};
