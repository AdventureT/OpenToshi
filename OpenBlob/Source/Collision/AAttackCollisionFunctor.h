#pragma once
#include "ACollisionFunctor.h"
#include "ACollisionObject.h"
#include "Entities/ABaseEntity.h"

//-----------------------------------------------------------------------------
// Purpose: a collision functor that skips everything but visible entities (8 bytes).
// NOTE: retail never creates one (dead code).
//-----------------------------------------------------------------------------
class AAttackCollisionFunctor
    : public ACollisionFunctor
{
public:
	// $deBlob: FUNCTION 005aca00
	AAttackCollisionFunctor() { m_bUnk04 = TFALSE; }

	// $deBlob: FUNCTION 005acb00
	// $deBlob: FUNCTION 005acb10
	virtual ~AAttackCollisionFunctor() override {} // +0x20

	// DOES_NOTHING (a shared "return TFALSE" stub, 005aca90)
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10

	// Skips the objects that aren't owned by a visible entity
	// $deBlob: FUNCTION 005aca10
	virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) override // +0x18
	{
		Toshi::TObject* pOwnerObject = TSTATICCAST( Toshi::TObject, a_pObject->GetOwner() );

		if ( HASANYFLAG( a_pObject->GetFlags(), ACollisionObject::FLAG_OwnerType1 | ACollisionObject::FLAG_OwnerType2 ) )
		{
			TASSERT( ( pOwnerObject == TNULL ) || pOwnerObject->IsA( &TGetClass( ABaseEntity ) ) );

			if ( !( TSTATICCAST( ABaseEntity, pOwnerObject )->GetFlags() & ABaseEntity::FLAG_Visible ) )
				return TTRUE;
		}

		return TFALSE;
	}

private:
	TBOOL m_bUnk04; // 0x04
};
