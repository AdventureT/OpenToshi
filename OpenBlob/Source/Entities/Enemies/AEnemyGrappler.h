#pragma once
#include "AEnemy.h"

namespace Toshi
{
class TPhysicsPhantomContainer;
}

//-----------------------------------------------------------------------------
// Purpose: an enemy that grapples the player's blob (Enemies\AEnemyGrappler.cpp, abstract: +0x228 is pure): up to four
// grapplers hold on to the bones of a blob (sm_PlayerBones) until it shakes them off.
//-----------------------------------------------------------------------------
class AEnemyGrappler
    : public AEnemy
{
public:
	TDECLARE_CLASS( AEnemyGrappler, AEnemy );

	static TCONSTEXPR_VAR TINT8 ENEMYGRAPPLER_INVALID_PLAYERINDEX = -1;
	static TCONSTEXPR_VAR TINT  ENEMYGRAPPLER_MAX_GRAPPLERS       = 4;
	static TCONSTEXPR_VAR TINT8 ENEMYGRAPPLER_INVALID_BONE        = -1;

	// m_eGrapplerState
	enum AEnemyGrapplerState : TINT8
	{
		GRAPPLERSTATE_None      = 0,
		GRAPPLERSTATE_Attacking = 1,
		GRAPPLERSTATE_Grappling = 2,
		GRAPPLERSTATE_Released  = 3,
	};

	// The grapplers on the blob of a player (0x18 bytes)
	struct PlayerBones
	{
		// NOTE: retail zeroes sm_PlayerBones in its dynamic initializer (marked on sm_PlayerBones), so the struct has a
		// constructor that clears every member
		PlayerBones()
		    : m_pPlayer( TNULL ), m_apGrapplers{}, m_fGrappleTime( 0.0f )
		{}

		ABaseEntity*    m_pPlayer;                                    // 0x00
		AEnemyGrappler* m_apGrapplers[ ENEMYGRAPPLER_MAX_GRAPPLERS ]; // 0x04
		TFLOAT          m_fGrappleTime;                               // 0x14
	};

	// The type data of a grappler
	struct GrapplerData
	    : public AEnemy::EnemyData
	{
		TUINT8 m_aUnkDC[ 0x08 ];      // 0xDC
		TFLOAT m_fDrainRate;              // 0xE4 (ALeechBot2::Tick: / 10, the drain)
		TFLOAT m_fAttackSpeed;        // 0xE8 the speed of the leap at the blob
		TFLOAT m_fAttackGravityScale; // 0xEC
		TINT   m_iStruggleCount;      // 0xF0 the hits (MESSAGE_GRAPPLESTRUGGLE) it takes to let go (read as a short by SetGrapplerState)
	};

public:
	AEnemyGrappler();
	virtual ~AEnemyGrappler() override; // +0x08

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void  Create() override;                                                                   // +0x1C
	virtual void  Destroy() override;                                                                  // +0x24
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void  OnLeaveState( TINT a_iState ) override;                                              // +0xD4
	virtual void  OnEnterState( TINT a_iState ) override;                                              // +0xD8
	virtual void  StartCurrentStateTask() override;                                                    // +0x154
	virtual TBOOL UpdateCurrentStateTask( TFLOAT a_fDeltaTime ) override;                              // +0x158
	virtual void  InitIdleState() override;                                                            // +0x15C
	virtual void  SetActive( TBOOL a_bActive, TBOOL a_bUnk ) override;                                 // +0x19C

	virtual void OnBlobDrained() = 0; // +0x228
	// $deBlob: FUNCTION 007b7380
	virtual void StartAttack() { SetGrapplerState( GRAPPLERSTATE_Attacking ); } // +0x22C
	// $deBlob: FUNCTION 007b7390
	virtual TBOOL IsGrappleState() { TINT iState = m_iState; return iState >= 0xD && iState <= 0x12 ? TTRUE : TFALSE; } // +0x230
	virtual void  ReleaseGrapple();                                        // +0x234

	void SetGrapplerState( AEnemyGrapplerState a_eState );

	// Grabs a_pPlayer where it was hit (a_rcNormal: the contact normal) if it has a free bone
	TBOOL PlayerGrappled( ABaseEntity* a_pPlayer, const Toshi::TVector4& a_rcNormal );
	// Takes the first free bone of a_pPlayer's (sm_PlayerBones), the normal and tangent in the body's space
	TBOOL FindFreeBone( ABaseEntity* a_pPlayer, const Toshi::TVector4& a_rcNormal );
	// Gives the bone back, the grapplers behind it move up
	void ReleaseBone();

	GrapplerData* GetGrapplerData() { return TSTATICCAST( GrapplerData, GetEnemyData() ); }

	AEnemyGrapplerState GetGrapplerState() const { return m_eGrapplerState; }

	static void AddPlayer( ABaseEntity* a_pPlayer );
	static void ClearPlayers();
	// The blob shakes: sends MESSAGE_GRAPPLESTRUGGLE (struggle) to the first grappler holding a_pPlayer
	static void PlayerStruggle( ABaseEntity* a_pPlayer );
	// Sends MESSAGE_GRAPPLERELEASE (let go) to the grapplers holding a_pPlayer
	static void ReleasePlayer( ABaseEntity* a_pPlayer );

	// The grapplers holding a_pPlayer let go (ReleaseGrapple)
	static void ReleaseAllGrapples( ABaseEntity* a_pPlayer );
	// A grappler holds a_pPlayer
	static TBOOL IsPlayerGrappled( ABaseEntity* a_pPlayer );

	// Moves a_rMatrix (in the grapple's space) into a_pPlayer's body space for its a_iIndex-th grappler: rotated by
	// its normal/tangent and pushed out to the bounding sphere along the normal (ABlobEntity::CreatePhysics' leech
	// bot capsules)
	static void GetBoneMatrix( Toshi::TMatrix44& a_rMatrix, ABaseEntity* a_pPlayer, TINT a_iIndex );

	// The share (0-100) of the hits it takes to free a_pPlayer from its grapplers that are done, out of the most
	// grapplers attached at once (ABlobShakeHud)
	static TINT GetStruggleProgress( ABaseEntity* a_pPlayer );

protected:
	static PlayerBones sm_PlayerBones[ 4 ];

	// The transform on the grappled blob's body: m_vPlayerTangent, m_vPlayerNormal and their cross product, at the
	// body's surface along the normal
	void GetGrappleTransform( Toshi::TMatrix44& a_rOutMatrix );

	AEnemyGrapplerState              m_eGrapplerState;        // 0xEC
	TINT8                            m_iPlayerIndex;          // 0xED sm_PlayerBones
	TINT16                           m_iHitsLeft;             // 0xEE the hits left before it lets go (GrapplerData::m_iStruggleCount)
	ABaseEntity*                     m_pPlayer;               // 0xF0 the grappled blob
	Toshi::TVector4                  m_vPlayerNormal;         // 0xF4 in the blob body's space
	Toshi::TVector4                  m_vPlayerTangent;        // 0x104 in the blob body's space
	Toshi::TPhysicsPhantomContainer* m_pLiquidRayCastHelper;  // 0x114 the phantom of the liquid ray cast
	TINT8                            m_iBoneIndex;            // 0x118 -1 (ENEMYGRAPPLER_INVALID_BONE)
	TUINT8                           m_uiSavedCollisionGroup; // 0x119 m_iCollisionGroupAndType while grappling
	TUINT8                           m_uiSavedGroupMask;      // 0x11A the collision object's 0x11 while grappling
};
