#pragma once
#include "IO/IIOMessageUpdater.h"

#include <Math/TVector4.h>

class ABaseEntity;

//-----------------------------------------------------------------------------
// Purpose: the IO action 0x3C, moves the entity over time: by an offset (from its position or its start position) or to
// its first target entity, in a duration or (duration -1) with an acceleration and a maximum speed. The finished
// message is sent within a distance of the end.
//-----------------------------------------------------------------------------
class IOMoveUpdater
    : public IIOMessageUpdater
{
public:
	// The parameters of the action
	struct Params
	{
		TFLOAT fDuration;        // 0x00 -1: the speed is used
		TFLOAT fOffsetX;         // 0x04 Z up
		TFLOAT fOffsetY;         // 0x08
		TFLOAT fOffsetZ;         // 0x0C
		TINT   iFromStart;       // 0x10 0: the offset is from the position, else from the start position
		TFLOAT fMaxSpeed;        // 0x14
		TFLOAT fAcceleration;    // 0x18
		TFLOAT fMessageDistance; // 0x1C
		TINT   iToTarget;        // 0x20 1: moves to the target entity
		TINT   iUnk24;           // 0x24
		TINT   iFinishedMessage; // 0x28 SendFinishedMessage
	};

public:
	IOMoveUpdater() { m_pTarget = TNULL; }

	// $deBlob: FUNCTION 00786ae0
	// $deBlob: FUNCTION 00786910
	virtual ~IOMoveUpdater() override {} // +0x00

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void Activate() override;                    // +0x08

	void SetTarget( ABaseEntity* a_pTarget ) { m_pTarget = a_pTarget; }

private:
	// Sends the finished message once within the message distance of a_rvPosition
	void CheckMessageDistance( const Params* a_pParams, const Toshi::TVector4& a_rvPosition );

private:
	Toshi::TVector4 m_vVelocity;    // 0x1C
	Toshi::TVector4 m_vStart;       // 0x2C
	Toshi::TVector4 m_vEnd;         // 0x3C
	ABaseEntity*    m_pTarget;      // 0x4C
	TBOOL           m_bMessageSent; // 0x50
	TINT            m_iUnk54;       // 0x54 not used
};                                  // 0x58
