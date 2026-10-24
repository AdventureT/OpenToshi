#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Math/TMatrix44.h>
#include "Entities/APickup.h"

TOSHI_NAMESPACE_START
class TParticleEffect;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the list of the pickups in the level (Pickups\APickupManager.cpp): their spinning rotations, the collection
// test against the players and the pickup particle effects. Created by AAppInitState.
//-----------------------------------------------------------------------------
class APickupManager
    : public Toshi::TSingleton<APickupManager>
{
public:
	static TCONSTEXPR_VAR TINT ROTATION_COUNT = 4;

	enum PICKUPEFFECT
	{
		PICKUPEFFECT_Life,
		PICKUPEFFECT_Pattern,
		PICKUPEFFECT_Time,
		PICKUPEFFECT_Energy,
		PICKUPEFFECT_TagA,
		PICKUPEFFECT_TagB,
		PICKUPEFFECT_TagC,
		PICKUPEFFECT_TagD,
		PICKUPEFFECT_TagE,
		PICKUPEFFECT_TagF,
		PICKUPEFFECT_TagG,
		PICKUPEFFECT_TagH,
		PICKUPEFFECT_COUNT
	};

public:
	APickupManager();
	virtual ~APickupManager(); // +0x00

	void AddPickup( APickup& a_rPickup );
	void RemovePickup( APickup& a_rPickup );

	// Spins the rotations and lets the players collect the pickups they touch
	void UpdateRotations( TFLOAT a_fDeltaTime );

	void                    GetPickupRotation( Toshi::TMatrix44& a_rOutMatrix, TINT a_RotationIndex, TBOOL a_bReversed );
	const Toshi::TMatrix44* GetPickupRotation( TINT a_RotationIndex, TBOOL a_bReversed );

	// Asserts that the level's pickups are gone (AInGameState::OnRemoval)
	void CheckEmpty();

	// $deBlob: FUNCTION 005c0910
	TINT GetNumPatternPickups() const { return m_iNumPatternPickups; }

	// Takes a pickup of type a_eType from the pool and spawns it (APickup::Spawn); TNULL outside the story and free
	// paint modes
	static APickup* CreatePickup( TINT a_eType, const Toshi::TVector4& a_rPosition, TFLOAT a_fHiddenTime, TFLOAT a_fLifeTime );
	static APickup* SpawnPickupNearPlayer( TINT a_eType, TINT a_iPlayer );

	static void LoadEffects();
	static void ClearEffects();
	static void StartEffect( TINT a_iEffect, const Toshi::TVector4& a_rPosition );

private:
	static Toshi::TParticleEffect* m_pParticleEffects[ PICKUPEFFECT_COUNT ];
	static const TCHAR*            ms_aszEffectNames[ PICKUPEFFECT_COUNT ];

private:
	Toshi::TMatrix44                  m_aRotations[ ROTATION_COUNT ];         // 0x004
	Toshi::TMatrix44                  m_aReversedRotations[ ROTATION_COUNT ]; // 0x104
	Toshi::T2DList<APickup::ListNode> m_lPickupList;                          // 0x204
	TINT                              m_iNumPatternPickups;                   // 0x20C pickups of type 3
	TINT                              m_iNumTagPickups;                       // 0x210 pickups of type 1
};
