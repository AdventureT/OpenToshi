#pragma once
#include "AChallenge.h"


//-----------------------------------------------------------------------------
// Purpose: a chain of sub challenges
//-----------------------------------------------------------------------------
class AUberChallenge
    : public AChallenge
{
public:
	// The data in the TRB (0x3C bytes)
	struct TypeData : public Data
	{
		union
		{
			TINT        m_iSubChallengeGUID; // 0x38 the first sub challenge in the TRB...
			AChallenge* m_pSubChallenge;     // 0x38 ...and the challenge (LinkChallenges)
		};
	};

public:
	AUberChallenge();
	// The complete-object destructor (the last marker) is used by AChallengeManager::LoadChallenges' eh vector constructor and the vector deleting destructor
	// $deBlob: FUNCTION 0059cda0
	// $deBlob: FUNCTION 0059ca30
	virtual ~AUberChallenge() override = default; // +0x00

	virtual void Start() override;                          // +0x04
	virtual void OnPaintEvent( void* a_pBuilding ) override;     // +0x10
	virtual void OnZJumpEvent( void* a_pSet ) override;     // +0x14
	virtual void OnCombatEvent( void* a_pEntityID ) override;    // +0x18
	virtual void OnCivilianEvent( void* a_pEntityID ) override;  // +0x1C
	virtual void OnCarEvent( void* a_pEntityID ) override;       // +0x20
	virtual void OnLandmarkEvent( void* a_pLandmark ) override;  // +0x24
	virtual void OnLandmarkEvent2( void* a_pLandmarkInstance ) override; // +0x28
	virtual void OnPaintEvent2( void* a_pTree ) override;    // +0x2C
	virtual void Update( TFLOAT a_fDeltaTime ) override;    // +0x30
	virtual void LinkChallenges() override;                 // +0x38
	virtual void Reset( TBOOL a_bUnk ) override;            // +0x3C

	// Links the daisy chain of sub challenges
	void LinkSubChallenges();

	// The current sub challenge was completed/timed out
	void OnSubChallengeCompleted( AChallenge* a_pSubChallenge );
	void OnSubChallengeTimeOut( AChallenge* a_pSubChallenge );

	AChallenge* GetCurrentSubChallenge() const { return m_pCurrSubChallenge; }
	void        IncrementLandmarksCompleted() { m_uiNumLandmarksCompleted++; }

private:
	AChallenge* m_pCurrSubChallenge;       // 0x174
	TUINT32     m_uiNumLandmarksCompleted; // 0x178
};

TSTATICASSERT( sizeof( AUberChallenge ) == 0x17C );
