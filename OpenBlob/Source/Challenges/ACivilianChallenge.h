#pragma once
#include "AChallenge.h"


//-----------------------------------------------------------------------------
// Purpose: a challenge with civilians
//-----------------------------------------------------------------------------
class ACivilianChallenge
    : public AChallenge
{
public:
	// One linked entity (0xC bytes)
	struct Entry
	{
		union
		{
			TINT m_iID; // 0x00 in the TRB...
		};
		ABaseEntity* m_pEntity; // 0x04 ...and the entity (Create)
		TINT         m_iDone;   // 0x08
	};

	// The data in the TRB (0x40 bytes)
	struct TypeData : public Data
	{
		Entry*  m_pEntries;     // 0x38
		TUINT32 m_uiNumEntries; // 0x3C
	};

public:
	ACivilianChallenge();
	// The complete-object destructor (the last marker) is used by AChallengeManager::LoadChallenges' eh vector constructor and the vector deleting destructor
	// $deBlob: FUNCTION 0059cb60
	// $deBlob: FUNCTION 0059c9f0
	virtual ~ACivilianChallenge() override = default; // +0x00

	virtual void  Start() override;                              // +0x04
	virtual void  OnCivilianEvent( void* a_pEntityID ) override; // +0x1C
	virtual void  Update( TFLOAT a_fDeltaTime ) override;        // +0x30
	virtual void  Reset( TBOOL a_bUnk ) override;                // +0x3C
	virtual TINT  GetScore() override;                           // +0x40
	virtual TBOOL IsAlreadyCompleted() override;                 // +0x44
	virtual TBOOL Create( TypeData* a_pData );                   // +0x4C

private:
};

TSTATICASSERT( sizeof( ACivilianChallenge ) == 0x174 );
