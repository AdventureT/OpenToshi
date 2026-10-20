#pragma once
#include "IO/IIOMessageUpdater.h"

#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: the IO action 0x3F, scales the entity to a scale in a duration. m_fTime counts down from the duration (set
// when the action starts, with the start scale).
//-----------------------------------------------------------------------------
class IOScaleUpdater
    : public IIOMessageUpdater
{
public:
	// The parameters of the action
	struct Params
	{
		TFLOAT fDuration;  // 0x00
		TFLOAT fScaleX;    // 0x04
		TFLOAT fScaleY;    // 0x08
		TFLOAT fScaleZ;    // 0x0C
		TFLOAT fCollision; // 0x10 != 0: the collision is scaled too
	};

public:
	IOScaleUpdater() {}

	// $deBlob: FUNCTION 00786b40
	// $deBlob: FUNCTION 00786930
	virtual ~IOScaleUpdater() override {} // +0x00

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x04

	void SetStartScale( const Toshi::TVector4& a_rScale ) { m_vStartScale = a_rScale; }

private:
	Toshi::TVector4 m_vStartScale; // 0x1C
};                                 // 0x2C
