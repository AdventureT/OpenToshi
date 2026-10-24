#pragma once
#include "Entities/AVisibleEntity.h"

class ACollisionObject;

//-----------------------------------------------------------------------------
// Purpose: the big paint bot: a paint source standing in the level. Blob touching it takes its "Colour" (0: a random
// one) and 30 paint points, then it pops (the LeechBot destroy effect of its colour) and comes back once no player is
// near (Tick).
//-----------------------------------------------------------------------------
class ABigPaintBot
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( ABigPaintBot, AVisibleEntity );

	// m_iState
	enum STATE
	{
		STATE_Idle    = 0,
		STATE_Popped  = 1, // waits until no player is near
		STATE_Respawn = 2,
	};

public:
	ABigPaintBot();
	virtual ~ABigPaintBot() override; // +0x08

	virtual void              Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void              Create() override;                                                                   // +0x1C
	virtual void              PostCreate() override;                                                               // +0x20
	virtual void              Destroy() override;                                                                  // +0x24
	virtual void              ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void              HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void              PreRenderUpdate() override;                                                          // +0x6C
	virtual ACollisionObject* GetCollisionObject() override;                                                       // +0x70
	virtual TBOOL             IsTargetable() override;                                                             // +0xA0
	virtual TINT              GetTargetColour() override;                                                          // +0xB4 the colour
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual const TCHAR*      GetModelName() override;                                                             // +0xEC

	// Sets the colour (the model's colour and the LOD distance type, the high byte of m_iCollisionGroupAndType)
	void   SetColour( TUINT8 a_uiColour );
	TUINT8 GetColour() const { return m_uiColour; }

	// Pops: the destroy effect of the colour, hidden until it respawns (STATE_Popped)
	void Pop();

private:
	void CreateCollisionObject();

private:
	static TBOOL sm_bSkinDone; // the sphere map of the skin is set

	TUINT8            m_uiColour;         // 0x9C "Colour" (0: random)
	ACollisionObject* m_pCollisionObject; // 0xA0
};
