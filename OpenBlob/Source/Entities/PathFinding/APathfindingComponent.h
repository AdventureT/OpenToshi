#pragma once
#include "Entities/AEntityComponent2.h"

#include <Math/TVector4.h>

class AGndMotorComponent;
class AAIMoveProbe;
class AAISteer;
class AAIPathFinder;
class AAINavigator;

//-----------------------------------------------------------------------------
// Purpose: the pathfinding of an AI character: ticks its move probe, steering, path finder and navigator, which drive
// the ground motor.
//-----------------------------------------------------------------------------
class APathfindingComponent
    : public AEntityComponent2
{
public:
	// m_uiFlags
	enum FLAG : TUINT8
	{
		FLAG_Walk        = 0x1,
		FLAG_Jump        = 0x2,
		FLAG_DoubleJump  = 0x4,
		FLAG_Fly         = 0x8,
		FLAG_Triangulate = 0x20,
		FLAG_Network     = 0x40,
	};

	// What blocks the move (set by AGndMotorComponent::UpdateCharacterMovement)
	struct Obstruction
	{
		// $deBlob: FUNCTION 007e5260
		void Reset()
		{
			pObject = TNULL;
			vNormal = Toshi::TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		}

		void*           pObject; // 0x0 ACollisionObject
		Toshi::TVector4 vNormal; // 0x4
	};

public:
	APathfindingComponent( ABaseEntity* a_pEntity, AEntityComponent2* a_pMotor );
	virtual ~APathfindingComponent() override; // +0x00

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void ResetComponent() override;            // +0x08

	TINT                GetCollisionFlags() const { return m_iCollisionFlags; }
	AGndMotorComponent* GetMotor() const { return TREINTERPRETCAST( AGndMotorComponent*, m_pMotor ); }

	void          SetFlags( TUINT8 a_uiFlags ) { m_uiFlags |= a_uiFlags; }
	TUINT8        GetFlags() const { return m_uiFlags; }
	TBOOL         AbleToFly() const { return m_uiFlags & FLAG_Fly; }
	void          SetUnk30( TINT a_iUnk ) { m_iUnk30 = a_iUnk; }
	AAIMoveProbe* GetMoveProbe() const { return m_pMoveProbe; }
	// $deBlob: FUNCTION 007d43b0
	AAISteer* GetSteer() const { return m_pSteer; }
	// $deBlob: FUNCTION 007d4380
	AAIPathFinder* GetPathFinder() const { return m_pPathFinder; }
	// $deBlob: FUNCTION 007d0c70
	AAINavigator* GetNavigator() const { return m_pNavigator; }
	Obstruction&  GetObstruction() { return m_oObstruction; }

	void SetCollisionFlags( TINT a_iFlags, TBOOL a_bSet )
	{
		if ( a_bSet ) m_iCollisionFlags |= a_iFlags;
		else m_iCollisionFlags &= ~a_iFlags;
	}

private:
	AEntityComponent2* m_pMotor;          // 0x14 AGndMotorComponent
	AAIMoveProbe*      m_pMoveProbe;      // 0x18
	AAISteer*          m_pSteer;          // 0x1C
	AAIPathFinder*     m_pPathFinder;     // 0x20
	AAINavigator*      m_pNavigator;      // 0x24
	TINT               m_iCollisionFlags; // 0x28
	TUINT8             m_uiFlags;         // 0x2C FLAG
	TINT               m_iUnk30;          // 0x30
	Obstruction        m_oObstruction;    // 0x34
};
