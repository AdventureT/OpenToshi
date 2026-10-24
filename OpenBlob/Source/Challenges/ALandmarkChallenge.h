#pragma once
#include "AChallenge.h"


//-----------------------------------------------------------------------------
// Purpose: a challenge at a landmark
//-----------------------------------------------------------------------------
class ALandmarkChallenge
    : public AChallenge
{
public:
	// The data in the TRB (0x40 bytes)
	struct TypeData : public Data
	{
		void**  m_ppLandmarks;    // 0x38 the landmark IDs, then the landmarks (Create)
		TUINT32 m_uiNumLandmarks; // 0x3C
	};

public:
	ALandmarkChallenge();
	// The complete-object destructor (the last marker) is used by AChallengeManager::LoadChallenges' eh vector constructor and the vector deleting destructor
	// $deBlob: FUNCTION 0059cc20
	// $deBlob: FUNCTION 0059ca10
	virtual ~ALandmarkChallenge() override = default; // +0x00

	virtual void  Start() override;                                       // +0x04
	virtual void  OnLandmarkEvent( void* a_pLandmark ) override;          // +0x24
	virtual void  OnLandmarkEvent2( void* a_pLandmarkInstance ) override; // +0x28
	virtual void  Update( TFLOAT a_fDeltaTime ) override;                 // +0x30
	virtual void  Reset( TBOOL a_bUnk ) override;                         // +0x3C
	virtual TINT  GetScore() override;                                    // +0x40
	virtual TBOOL IsAlreadyCompleted() override;                          // +0x44
	virtual TBOOL Create( TypeData* a_pData );                            // +0x4C

	// $deBlob: FUNCTION 005804b0
	TINT8 GetTriggersLeft() const { return m_iTriggersLeft; }

private:
	TINT8 m_iTotalTriggers; // 0x174
	TINT8 m_iTriggersLeft;  // 0x175
	TINT8 m_iLandmarkType;  // 0x176 index of the landmark type names, -1: none
};

TSTATICASSERT( sizeof( ALandmarkChallenge ) == 0x178 );
