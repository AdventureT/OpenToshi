#pragma once
#include "AChallenge.h"
#include "Entities/AWaypointData.h"

class AEnvEmitter;

namespace Toshi
{
class TParticleEffect;
}


//-----------------------------------------------------------------------------
// Purpose: a race along waypoints: a trigger sphere (m_pWPBody, radius 6) sits on the current node of the waypoint set,
// Blob touching it moves it to the next node (UpdateWaypoints, the time bonus of the type data), the race is won at the
// last node if Blob has the required colour and size. AEnvParticles shows the markers of the current and the next node
// (ShowWaypoints).
//-----------------------------------------------------------------------------
class ARaceChallenge
    : public AChallenge
{
public:
	// The data in the TRB (0x48 bytes)
	struct TypeData : public Data
	{
		TINT    m_iWPSetID;         // 0x38 -1: none
		TINT    m_iTimeBonus;       // 0x3C the seconds added at each node (with a time limit)
		TUINT32 m_uiRequiredColour; // 0x40 the colour Blob needs at the end (-1: any)
		TINT    m_iRequiredSize;    // 0x44 the size Blob needs at the end (-1: any; shown on the HUD)
	};

	// Receives the trigger events of the waypoint body
	class CollisionHandler
	    : public Toshi::TPhysicsResponder
	{
	public:
		virtual void OnTriggerEnter( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override; // +0x20
		// $deBlob: FUNCTION 0059f0d0
		virtual void OnTriggerLeave( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override {} // +0x24

	public:
		ARaceChallenge* m_pChallenge; // 0x04
	};

public:
	ARaceChallenge();
	// $deBlob: FUNCTION 0059fd30
	// $deBlob: FUNCTION 0059cd40
	virtual ~ARaceChallenge() override; // +0x00

	virtual void  Start() override;                       // +0x04
	virtual void  OnEnd() override;                       // +0x0C
	virtual void  Update( TFLOAT a_fDeltaTime ) override; // +0x30
	virtual void  Reset( TBOOL a_bUnk ) override;         // +0x3C
	virtual TINT  GetScore() override;                    // +0x40
	virtual TINT  GetHintStringID() override;             // +0x48
	virtual TBOOL Create( TypeData* a_pData );            // +0x4C

	// Blob reached a_pNode: moves the trigger to the next node, or completes the race at the last one (returns TTRUE)
	TBOOL UpdateWaypoints( AWaypointData::Node* a_pNode );

	// Removes the markers, and with a_bShow shows the ones of the current and the next node
	void ShowWaypoints( TBOOL a_bShow );

public:
	// The position the nav ring points to
	const Toshi::TVector4* GetNextCheckpoint() const { return TREINTERPRETCAST( const Toshi::TVector4*, m_pCurrentNode ); }

private:
	TWCHAR                          m_wszRequiredSize[ 8 ]; // 0x174 "%d" of m_iRequiredSize
	AWaypointData::Set*             m_pWPSet;               // 0x184
	AWaypointData::Node*            m_pCurrentNode;         // 0x188 the next checkpoint (ABlobNavRingComponent::UpdateTarget)
	CollisionHandler                m_oRaceHandler;         // 0x18C
	Toshi::TPhysicsObjectContainer* m_pWPBody;              // 0x194 its user data is m_pCurrentNode
	AEnvEmitter*                    m_apWPEmitters[ 2 ];    // 0x198 the markers of the current and the next node
	Toshi::TParticleEffect*         m_pWPEffectMain;        // 0x1A0
	Toshi::TParticleEffect*         m_pWPEffectRest;        // 0x1A4
	Toshi::TParticleEffect*         m_pWPEffectFinal;       // 0x1A8
	Toshi::TMatrix44                m_oWPMatrix;            // 0x1AC
	Toshi::TVector4                 m_vLastNode;            // 0x1EC the last node, once the next one is it
	TBOOL                           m_bLastNodeNext;        // 0x1FC
	TINT                            m_iNumCheckpoints;      // 0x200
};

TSTATICASSERT( sizeof( ARaceChallenge ) == 0x204 );
