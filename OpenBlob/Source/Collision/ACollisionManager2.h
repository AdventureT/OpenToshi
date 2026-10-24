#pragma once
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>
#include <Physics/Havok/TPhysicsManager.h>

class ACollisionObject;
class ACollisionFunctor;

//-----------------------------------------------------------------------------
// Purpose: the game's collision queries on top of the Havok world (1 byte, created by AAppInitState).
//-----------------------------------------------------------------------------
class ACollisionManager2
    : public Toshi::TSingleton<ACollisionManager2>
{
public:
	static TCONSTEXPR_VAR TINT MAX_SWEEP_CONTACTS = 8;

public:
	ACollisionManager2()  = default;
	~ACollisionManager2() = default;

	// Sweeps a box (a_rcHalfExtents) from a_rcPosition along a_rcDirection for a_rfDistance; returns the collision
	// object hit first (TNULL: none), a_rfDistance is shortened to the hit, a_rvOutNormal is its normal and
	// a_rfOutStep the height to step up onto it (-1: can't, -2: nothing hit; see GetStepHeight)
	ACollisionObject* CollideSweptAABB( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcHalfExtents, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rvOutNormal, TFLOAT a_fStepHeight, TFLOAT& a_rfOutStep, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer, void* a_pPhantom );

	// Casts a ray from a_rcStart along a_rcDirection for a_rfDistance and returns the first collision object hit
	// that isn't a_pIgnore or rejected by a_pFunctor (TNULL: none); a_rfDistance is shortened to the hit,
	// a_pOutMaterial gets its surface
	// The first object (not a_pIgnore, not refused by a_pFunctor) overlapping the box a_rcCenter +- a_rcHalfExtents;
	// a_rvOutNormal: the contact's surface normal
	static ACollisionObject* CollideAABB( const Toshi::TVector4& a_rcCenter, const Toshi::TVector4& a_rcHalfExtents, Toshi::TVector4& a_rvOutNormal, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer );

	// Sweeps a sphere of a_fRadius like CollideSweptAABB (no phantom; a_rfOutStep is only written for a mesh hit
	// with a_fStepHeight > 0, -1: too high); a_rvOutNormal is the contact's separating normal
	static ACollisionObject* CollideSweptSphere( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rvOutNormal, TFLOAT a_fStepHeight, TFLOAT& a_rfOutStep, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer );
	// Casts a ray like CollideRaySolidFast, through a_pPhantom while it's in the world; the functor can refuse a hit
	// (HitIgnorableObject, PreFilterSweptAABBPoly with the mesh triangle) and is told of the accepted one
	// (AABBSweptHitPoly); a_rvOutNormal is the hit's separating normal
	static ACollisionObject* CollideRay( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, Toshi::TVector4& a_rvOutNormal, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer, void* a_pPhantom );

	static ACollisionObject* CollideRaySolidFast( const Toshi::TVector4& a_rcStart, const Toshi::TVector4& a_rcDirection, TFLOAT& a_rfDistance, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer, TUINT32* a_pOutMaterial );

private:
	static TFLOAT* GetStepHeight( const Toshi::TPhysicsShapeContact& a_rcContact, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcHalfExtents, TFLOAT a_fStepHeight, TFLOAT* a_pOutStep );

	// Dead helpers
	// The collision object of a body's owner
	static ACollisionObject* GetCollisionObject( Toshi::TPhysicsObjectContainer* a_pBody );
	// Casts a ray of *a_pfLength up from a_pcPosition: the first hit not on a_pIgnore in a_pOutPosition/a_pOutNormal
	static void CollideRayUp( const TFLOAT* a_pfLength, ACollisionObject* a_pIgnore, const Toshi::TVector4* a_pcPosition, Toshi::TVector4* a_pOutPosition, Toshi::TVector4* a_pOutNormal );
	// The first contact of a sphere (TSphereData's default radius) at a_rcPosition not on a_pIgnore nor refused by
	// the functor: its distance and normal
	static void CollideSphere( const Toshi::TVector4& a_rcPosition, TFLOAT* a_pOutDistance, Toshi::TVector4* a_pOutNormal, ACollisionObject* a_pIgnore, ACollisionFunctor* a_pFunctor, TUINT32 a_uiCollisionLayer );
	// The step height of a sphere onto the mesh triangle of a contact (-1: too high)
	static void GetSphereStepHeight( const Toshi::TPhysicsShapeContact& a_rcContact, const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius, TFLOAT a_fStepHeight, TFLOAT* a_pOutStep );
	// The step height from a_rcPosition onto a triangle (-1: too high or too low)
	static void GetTriangleStepHeight( const Toshi::TVector4* const* a_ppTriangle, const Toshi::TVector4& a_rcPosition, TFLOAT a_fStepHeight, TFLOAT* a_pOutStep );
	// The point of the segment a_rcFrom-a_rcTo closest to a_rcPoint in a_pOutPoint, TFALSE if it's off the segment
	static TBOOL GetClosestPointOnSegment( const Toshi::TVector4& a_rcPoint, const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo, Toshi::TVector4* a_pOutPoint );
	// The slope from a_rcTo up to a_rcFrom (height over the XZ distance, 0 if steeper than 45 degrees)
	static TFLOAT GetSlope( const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcTo );
};
