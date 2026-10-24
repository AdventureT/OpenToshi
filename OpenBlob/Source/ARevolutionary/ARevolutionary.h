#pragma once
#include "Entities/AVisibleEntity.h"

class AChallenge;
class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: the marker of a challenge, a Revolutionary standing at the challenge's start; created by AChallenge::Create
// ("ARevolutionary", or a subclass picking other meshes of the model). Above it, m_pChallengeModel
// ("pickup_challengelocation.trb") bobs and spins while the challenge is available.
// States: 0 idle, 1 start prompt shown, 3 starting, 4 running, 7/8 ending
//-----------------------------------------------------------------------------
class ARevolutionary
    : public AVisibleEntity
{
public:
	TDECLARE_CLASS( ARevolutionary, AVisibleEntity );

	// The derived part of the type data ("<type>_data_EntityData" after AEntityTypeData::TRBData)
	struct RevolutionaryData
	{
		TUINT8           m_aBase[ 0x54 ]; // 0x00 AEntityTypeData::TRBData
		Toshi::TVector4* m_pBBox;         // 0x54
	};

public:
	ARevolutionary();
	virtual ~ARevolutionary() override; // +0x08

	virtual void         Tick( TFLOAT a_fDeltaTime ) override;     // +0x0C
	virtual void         OnLODChanging( TINT a_iLOD ) override;    // +0x10
	virtual void         Create() override;                        // +0x1C
	virtual void         PostCreate() override;                    // +0x20
	virtual void         Destroy() override;                       // +0x24
	virtual void         SetVisible( TBOOL a_bVisible ) override;  // +0x38
	virtual void         Reset( ABaseEntity* a_pEntity ) override; // +0xC8
	virtual void         OnEnterState( TINT a_iState ) override;   // +0xD8
	virtual const TCHAR* GetModelName() override;                  // +0xEC

	// Moves a_rfStart towards a_fDestination by at most a_fStep, the shorter way round in [a_fMin, a_fMax]
	static void MoveTowardsWrapped( TFLOAT& a_rfStart, TFLOAT a_fDestination, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep );

	// Called by AChallenge
	void  SetChallenge( AChallenge* a_pChallenge );
	void  OnChallengeStartShort();
	void  OnChallengeStart();
	void  OnChallengeComplete();
	void  OnChallengeFail();
	void  OnChallengeStarting();
	void  ShowStartPrompt();
	void  Refresh( TBOOL a_bReset );
	void  HideStartPrompt();
	TBOOL IsIdle();
	TBOOL IsStarting();
	void  HideMarker();

protected:
	void ShowMarker();
	void ShowMesh();
	void HideMesh();

protected:
	TUINT8           m_uiMesh;               // 0x9C the mesh of the model shown (ShowMesh/HideMesh)
	TUINT8           m_uiMeshA;              // 0x9D shown unless the challenge's data has flag 0x80
	TUINT8           m_uiMeshB;              // 0x9E shown if it has
	TBOOL            m_bShowMarkerNextFrame; // 0x9F
	AChallenge*      m_pChallenge;           // 0xA0
	AModelInstance2* m_pChallengeModel;      // 0xA4 "pickup_challengelocation.trb"
	TFLOAT           m_fTime;                // 0xA8 the bobbing
	TFLOAT           m_fHeight;              // 0xAC the height of m_pChallengeModel
};

TSTATICASSERT( sizeof( ARevolutionary ) == 0xB0 );
