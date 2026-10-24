#include "pch.h"
#include "AEnemyMelee.h"
#include "AEnemyBehaviour.h"
#include "Entities/AGndMotorComponent.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AEnemyMelee ) == 0xEC );

// $deBlob: FUNCTION 007e5000
// $deBlob: FUNCTION 007e5010
// $deBlob: FUNCTION 007e5020
// $deBlob: FUNCTION 007e5030
// $deBlob: FUNCTION 007e5040
// $deBlob: FUNCTION 00541a90
// $deBlob: FUNCTION 0085a160
TDEFINE_CLASS_NORUNTIME( AEnemyMelee );

// $deBlob: FUNCTION 007e5050
AEnemyMelee::AEnemyMelee()
{
	m_uiEnemyFlags |= EFLAG_Melee;
}

// $deBlob: FUNCTION 007e5070
// $deBlob: FUNCTION 007e5230
AEnemyMelee::~AEnemyMelee()
{
}

// $deBlob: FUNCTION 007e5080
void AEnemyMelee::OnEnterState( TINT a_iState )
{
	if ( a_iState == 0x0E )
		DoAttack( 0 );

	AEnemy::OnEnterState( a_iState );
}

// $deBlob: FUNCTION 007e50b0
void AEnemyMelee::StartCurrentStateTask()
{
	// NOTE: the recover task (0x0A) goes straight to state 0x0D, the other tasks are AEnemy's (tail jump)
	switch ( m_iCurrentTask )
	{
		case 0x0A:
			SetState( 0x0D );
			break;
		default:
			AEnemy::StartCurrentStateTask();
			break;
	}
}

// $deBlob: FUNCTION 007e50d0
TBOOL AEnemyMelee::UpdateCurrentStateTask( TFLOAT a_fDeltaTime )
{
	if ( m_iCurrentTask != 0x0A )
		return AEnemy::UpdateCurrentStateTask( a_fDeltaTime );

	// The attack (0x0D faces the enemy, then 0x0E, 0x0F)
	switch ( m_iState )
	{
		case 0x0D:
			TSTATICCAST( AEnemyBehaviour, GetBehaviourComponent() )->FaceEnemyPredictedPosition( 0.5f, TFALSE, TNULL );
		case 0x0E:
			if ( HasStateFinished() )
				SetState( m_iState + 1 );

			return TFALSE;
		case 0x0F:
			return HasStateFinished();
		default:
			return TTRUE;
	}
}

// $deBlob: FUNCTION 007e5150
void AEnemyMelee::DoAttack( TINT a_iAttack )
{
	MeleeData* pData = TSTATICCAST( MeleeData, m_pTypeData->m_pTRBData );

	// NOTE: retail passes the float m_vBBox.y * 2 as CheckMeleeCollision's 4th (unused) argument
	TFLOAT fHalfHeight2 = m_vBBox.y * 2.0f;
	TINT   iAttackType  = GetAttackType( a_iAttack );
	CheckMeleeCollision( 0.0f, pData->m_fAttackMinDot, pData->m_fAttackRangeMax, *TREINTERPRETCAST( TINT*, &fHalfHeight2 ), AEntityIOPacket::MESSAGE_ENEMYATTACK, iAttackType, pData->m_iAttackDamage, pData->m_iAttackUnk18, 0.0f );

	if ( pData->m_fLungeSpeed > 0.0f )
	{
		// The lunge forward
		TVector4 vVelocity = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
		vVelocity.Multiply( pData->m_fLungeSpeed );
		vVelocity.w = 1.0f;

		TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetVelocity( vVelocity, pData->m_fLungeTime, 0.0f, TTRUE );
	}
}
