#pragma once
#include "IO/AIOEntityComponent.h"
#include "IO/ANEDKeyframeInterpolation.h"

namespace Toshi
{
class TTRB;
}

//-----------------------------------------------------------------------------
// Purpose: the key frame animation of an IO entity, its component of type 99 (AIOKeyframeComponent.cpp): the tracks of
// its TRB symbol move entities (SetMatrix) along their keys while it plays; the actions 0x44 (play), 0x45 (pause), 0x46
// (resume) and 0x47 (stop) control it.
//-----------------------------------------------------------------------------
class AIOKeyframeComponent
    : public AIOEntityComponent
{
public:
	static TCONSTEXPR_VAR TINT KEYFRAME_COMPONENT = 99;

	enum ACTION : TINT
	{
		ACTION_Play   = 0x44,
		ACTION_Pause  = 0x45,
		ACTION_Resume = 0x46,
		ACTION_Stop   = 0x47,
	};

	enum STATE : TINT
	{
		STATE_Stopped = 0,
		STATE_Playing = 1,
		STATE_Paused  = 2,
	};

	using PLAYFLAGS = TUINT8;
	enum PLAYFLAGS_ : PLAYFLAGS
	{
		PLAYFLAGS_Reverse  = 0x1,
		PLAYFLAGS_PingPong = 0x2, // reverses at the end
		PLAYFLAGS_Loop     = 0x4,
		PLAYFLAGS_Counted  = 0x8, // m_iLoopCount
	};

	// The parameters of ACTION_Play
	struct PlayParams
	{
		TINT iLoop;    // 0x00
		TINT iRestart; // 0x04 mode 3 restarts from the start (end when reversed)
		TINT iCount;   // 0x08 < 1: loops forever
		TINT iMode;    // 0x0C 1: reverse, 2: ping pong, 3: toggles the direction
	};

	struct IOEntityData
	{
		TINT          componentType; // 0x00 KEYFRAME_COMPONENT
		const TCHAR** pData;         // 0x04 the TRB symbol of the tracks (ANEDKeyframeInterpolation::TrackSet)
	};

public:
	AIOKeyframeComponent( AIOEntity* a_pOwner );
	virtual ~AIOKeyframeComponent() override; // +0x00

	// PostCreate: finds the entities of the tracks, the owner ticks
	virtual void Unk04() override; // +0x04

	// Tick: moves the entities of the tracks to the current time and advances it
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x08

	// SetIOEntityData: a_pUnk1 is the component's IOEntityData, a_pUnk2 the TRB
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) override; // +0x0C

	// ExecuteAction: the ACTIONs
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ) override; // +0x10

	// Starts playing with the parameters of ACTION_Play
	void Play( const PlayParams* a_pParams );

	// Moves a_pEntity to the keys of a_pKeyList at a_fTime
	static void TickKeyList( ANEDKeyframeInterpolation::KeyList* a_pKeyList, TFLOAT a_fTime, ABaseEntity* a_pEntity );

private:
	ANEDKeyframeInterpolation::TrackSet* m_pTrackSet;   // 0x14
	STATE                                m_eState;      // 0x18
	PLAYFLAGS                            m_uiPlayFlags; // 0x1C
	TINT16                               m_iLoopCount;  // 0x1E
};                                                      // 0x20
