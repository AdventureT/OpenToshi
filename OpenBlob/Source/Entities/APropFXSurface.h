#pragma once
#include "Entities/AInvisibleEntity.h"
#include "FXComposition/AFXCompositionInstance.h"
#include <Toshi/T2DList.h>

//-----------------------------------------------------------------------------
// Purpose: a surface that plays effects on the characters that touch it: an "EnterEffectName" when a character enters
// it, an "InsideEffectName" while it's below the surface and an "ExitEffectName" when it leaves; up to 8 effect sets,
// the characters inside are nodes of a shared pool of 8.
// NOTE: retail tests the characters against a unit box at the world's origin, not the surface (see
//); names from the entity params
//-----------------------------------------------------------------------------
class APropFXSurface
    : public AInvisibleEntity
{
public:
	TDECLARE_CLASS( APropFXSurface, AInvisibleEntity );

	static TCONSTEXPR_VAR TINT NUM_SLOTS = 8;
	static TCONSTEXPR_VAR TINT NUM_NODES = 8;

	// The effects of one character (0x2BC bytes)
	struct Slot
	{
		// $deBlob: FUNCTION 007de750
		Slot() {}
		// $deBlob: FUNCTION 007de7c0
		~Slot() {}

		AFXCompositionInstance m_oEnter;  // 0x000
		AFXCompositionInstance m_oInside; // 0x0E8
		AFXCompositionInstance m_oExit;   // 0x1D0
		TBOOL                  m_bUsed;   // 0x2B8 never set by retail
	};

	// A character on a surface (0x10 bytes)
	struct Node : public Toshi::T2GenericDList::Node
	{
		// $deBlob: FUNCTION 007de770
		Node()
		{
			m_pEntity = TNULL;
			m_pSlot   = TNULL;
		}

		ABaseEntity* m_pEntity; // 0x08
		Slot*        m_pSlot;   // 0x0C
	};

public:
	APropFXSurface();
	virtual ~APropFXSurface() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 007deb70
	virtual void Reset( ABaseEntity* a_pEntity ) override {} // +0xC8 NOTE: doesn't call AInvisibleEntity::Reset

	// The node pool (AEntityFramework::Initialise/Shutdown)
	static void InitialisePool();
	static void DeinitialisePool();

private:
	// Puts back the nodes of the characters on the surface into the pool
	void ReleaseNodes();

	// The surface's matrix with the translation of a_rPosition at the surface's height
	void GetEffectMatrix( const Toshi::TVector4& a_rPosition, Toshi::TMatrix44& a_rOutMatrix );
	void GetEffectMatrix( ABaseEntity* a_pEntity, Toshi::TMatrix44& a_rOutMatrix );

	// The character is in the surface's box (always a unit box at the origin, see the class comment)
	TBOOL IsInSurface( ABaseEntity* a_pEntity );

private:
	static Node*                ms_pNodes;
	static Toshi::T2DList<Node> ms_oFreeNodes;

	Toshi::T2DList<Node> m_oNodes;              // 0x084 the characters on the surface
	Slot                 m_aSlots[ NUM_SLOTS ]; // 0x08C
	TINT                 m_iUnk166C;            // 0x166C -1
};
