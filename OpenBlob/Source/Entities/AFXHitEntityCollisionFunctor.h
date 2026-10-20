#pragma once
#include "Collision/ACollisionFunctor.h"
#include "Collision/ACollisionObject.h"
#include "Entities/AProp.h"

#include <Toshi/TObject.h>

//-----------------------------------------------------------------------------
// Purpose: the functor of AFXBreakpointComponent::ComputeDecalMatrix's ray (4 bytes): only props are hit.
//-----------------------------------------------------------------------------
class AFXHitEntityCollisionFunctor
    : public ACollisionFunctor
{
public:
	// DOES_NOTHING (shared stub: returns TFALSE)
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10

	// Skips everything that isn't a prop
	// $deBlob: FUNCTION 0076c200
	virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) override // +0x18
	{
		return TSTATICCAST( Toshi::TObject, a_pObject->GetOwner() )->IsA( &TGetClass( AProp ) ) ? TFALSE : TTRUE;
	}

	// $deBlob: FUNCTION 0076c1b0
	// $deBlob: FUNCTION 0076c0a0
	virtual ~AFXHitEntityCollisionFunctor() override = default; // +0x20
};
