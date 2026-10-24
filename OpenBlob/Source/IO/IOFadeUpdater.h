#pragma once
#include "IO/IIOMessageUpdater.h"

//-----------------------------------------------------------------------------
// Purpose: the IO action 0x3E, fades the entity's model to an alpha in a duration. m_fTime counts down from the
// duration (set when the action starts, with the start alpha).
//-----------------------------------------------------------------------------
class IOFadeUpdater
    : public IIOMessageUpdater
{
public:
	// The parameters of the action
	struct Params
	{
		TFLOAT fDuration; // 0x00
		TFLOAT fAlpha;    // 0x04
	};

public:
	IOFadeUpdater() {}

	// $deBlob: FUNCTION 00786ab0
	// $deBlob: FUNCTION 00786900
	virtual ~IOFadeUpdater() override {} // +0x00

	virtual void Update( TFLOAT a_fDeltaTime ) override; // +0x04

	void SetStartAlpha( TFLOAT a_fAlpha ) { m_fStartAlpha = a_fAlpha; }

private:
	TFLOAT m_fStartAlpha; // 0x1C
};                        // 0x20
