#pragma once
#include "Entities/AEntityComponent2.h"

//-----------------------------------------------------------------------------
// Purpose: the data of an entity's states: their lengths and animation blend times, from "<type>_StateData_Main" in the
// state data TRB (AEntityData).
//-----------------------------------------------------------------------------
class AEntityStateComponent
    : public AEntityComponent2
{
public:
	struct BlendTime
	{
		TINT   iNewState; // 0x00
		TFLOAT fTime;     // 0x04
	};

	struct StateData
	{
		TFLOAT     fLength;           // 0x00
		BlendTime* pBlendTimes;       // 0x04 from this state to others
		TINT       iNumBlendTimes;    // 0x08
		TFLOAT     fDefaultBlendTime; // 0x0C
		TUINT8     uiFlags;           // 0x10
	};                                // 0x14

	struct Header
	{
		StateData* pData;      // 0x00
		TINT       iDataCount; // 0x04
	};

public:
	AEntityStateComponent( ABaseEntity* a_pEntity );
	virtual ~AEntityStateComponent() override; // +0x00

	TFLOAT GetStateLength( TINT a_iState );
	TFLOAT GetStateAnimBlendTime( TINT a_iCurState, TINT a_iNewState );

	// $deBlob: FUNCTION 007c61c0
	TBOOL IsStateLooping( TINT a_iState )
	{
		Header* pHeader = m_pHeader;
		TASSERT( a_iState >= 0 && a_iState < pHeader->iDataCount );
		return pHeader->pData[ a_iState ].uiFlags & 1;
	}

	TINT GetNumStates() const { return m_pHeader->iDataCount; }

private:
	Header* m_pHeader; // 0x14
};                     // 0x18
