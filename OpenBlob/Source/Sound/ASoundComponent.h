#pragma once
#include "IO/AIOEntityComponent.h"
#include "Sound/ASoundMap.h"

namespace FMOD
{
class Event;
}

//-----------------------------------------------------------------------------
// Purpose: the sound of an IO entity, its component of type 0 (Sound\ASoundComponent.cpp): an FMOD event of the sound
// map, started by the action 0x1E and stopped by the action 0x40; Tick starts it when the IO data sets it playing
// (state 1) and no event was started yet.
//-----------------------------------------------------------------------------
class ASoundComponent
    : public AIOEntityComponent
{
public:
	static TCONSTEXPR_VAR TINT TYPE = 0;

	// The actions of the component
	static TCONSTEXPR_VAR TINT ACTION_Play = 0x1E;
	static TCONSTEXPR_VAR TINT ACTION_Stop = 0x40;

	// The IO data of the component (SetIOEntityData)
	struct Data
	{
		ASoundMap::GROUPID eGroupID; // 0x00
		ASoundMap::EVENTID eEventID; // 0x04
		TINT               iState;   // 0x08 1: playing
	};

	struct IOEntityData
	{
		TINT  iType; // 0x00 TYPE
		Data* pData; // 0x04
	};

public:
	ASoundComponent( AIOEntity* a_pOwner );
	virtual ~ASoundComponent() override; // +0x00

	// Tick: starts the event while it's set to play (m_iState 1) and none was started
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x08

	// SetIOEntityData: a_pUnk1 is the component's IOEntityData
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) override; // +0x0C

	// ExecuteAction: ACTION_Play and ACTION_Stop (only while the component is enabled, m_uiFlags bit 0)
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ) override; // +0x10

	void SetPlaying( TBOOL a_bPlaying );

	// Starts the event
	void Play();

private:
	FMOD::Event*       m_pEvent;   // 0x14
	ASoundMap::GROUPID m_eGroupID; // 0x18
	ASoundMap::EVENTID m_eEventID; // 0x1C
	TINT               m_iState;   // 0x20 1: playing
};                                 // 0x24
