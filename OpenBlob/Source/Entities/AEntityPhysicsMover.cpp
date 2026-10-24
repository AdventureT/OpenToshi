#include "pch.h"
#include "AEntityPhysicsMover.h"
#include "AVisibleEntity.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007c6420
AEntityPhysicsMover::AEntityPhysicsMover( AVisibleEntity* a_pEntity )
{
	m_pEntity = a_pEntity;
	m_iUnk08  = -1;
	m_iBone   = -1;
}

// $deBlob: FUNCTION 007c6770
void AEntityPhysicsMover::OnRemoved()
{
}

// Keyframes the body to the entity (its x and z axes flipped) or to a bone of its model, plus the shape's offset
// $deBlob: FUNCTION 007c6450
void AEntityPhysicsMover::Update( TFLOAT a_fTimeStep, TPhysicsManager* a_pManager )
{
	TPhysicsManager*         pPhysicsManager = TPhysicsManager::GetSingleton();
	AVisibleEntity*          pEntity         = m_pEntity;
	AEntityPhysicsData*      pPhysicsData    = pEntity->m_pPhysicsData;
	TPhysicsObjectContainer* pBody           = pPhysicsData->m_pBody;

	if ( pBody == TNULL || !pBody->IsInSimulation() || pPhysicsManager->GetNumSteps() > 0 ||
	     pBody->m_eMotionType == 3 || pBody->m_eMotionType == 0 )
		return;

	const TMatrix44& rcEntity = pEntity->GetMatrix();

	TMatrix44 matTarget;
	matTarget.m_f11 = -rcEntity.m_f11;
	matTarget.m_f12 = -rcEntity.m_f12;
	matTarget.m_f13 = -rcEntity.m_f13;
	matTarget.m_f14 = -rcEntity.m_f14;
	matTarget.m_f21 = rcEntity.m_f21;
	matTarget.m_f22 = rcEntity.m_f22;
	matTarget.m_f23 = rcEntity.m_f23;
	matTarget.m_f24 = rcEntity.m_f24;
	matTarget.m_f31 = -rcEntity.m_f31;
	matTarget.m_f32 = -rcEntity.m_f32;
	matTarget.m_f33 = -rcEntity.m_f33;
	matTarget.m_f34 = -rcEntity.m_f34;
	matTarget.m_f41 = rcEntity.m_f41;
	matTarget.m_f42 = rcEntity.m_f42;
	matTarget.m_f43 = rcEntity.m_f43;
	matTarget.m_f44 = rcEntity.m_f44;

	if ( m_iBone >= 0 )
	{
		AModelInstance2*   pModelInstance    = TSTATICCAST( AEntityModelComponent, pEntity->GetModelComponent() )->GetModelInstance();
		TSkeletonInstance* pSkeletonInstance = pModelInstance->GetModelInstance()->GetSkeletonInstance();

		if ( &pSkeletonInstance->GetBones()[ m_iBone ] == TNULL )
		{
			// NOTE: retail keyframes to the uninitialised matrix here (unreachable)
			TPhysicsManager::ApplyHardKeyFrame( pBody, matTarget, pPhysicsManager->GetTimeStep() );
			return;
		}

		TMatrix44 matBone;
		pModelInstance->GetBoneTransformLocal( 0, matBone );
		matTarget.Multiply( matBone );
		matTarget.Multiply( pSkeletonInstance->GetSkeleton()->GetBone( m_iBone )->GetTransformInv() );
	}

	matTarget.Multiply( pPhysicsData->m_oOffset );
	TPhysicsManager::ApplyHardKeyFrame( pBody, matTarget, pPhysicsManager->GetTimeStep() );
}

// $deBlob: FUNCTION 007c66b0
void AEntityPhysicsMover::SetBodyTransform( const TMatrix44& a_rcMatrix )
{
	AEntityPhysicsData*      pPhysicsData = m_pEntity->m_pPhysicsData;
	TPhysicsObjectContainer* pBody        = pPhysicsData->m_pBody;

	TMatrix44 matBody;
	matBody.Multiply( a_rcMatrix, pPhysicsData->m_oOffset );
	*pBody->m_pMatrix = matBody;

	TPhysicsManager::UpdateBodyTransform( pBody, TFALSE );

	if ( pBody->m_eMotionType != 0 )
		TPhysicsManager::StopBody( pBody );
}

// $deBlob: FUNCTION 007c67d0
void AEntityPhysicsMover::Unk0C( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c67c0
void AEntityPhysicsMover::Unk10( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c6810
void AEntityPhysicsMover::Unk14( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c6800
void AEntityPhysicsMover::Unk18( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c67b0
void AEntityPhysicsMover::Unk1C( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c67a0
void AEntityPhysicsMover::Unk20( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c67f0
void AEntityPhysicsMover::Unk24( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c67e0
void AEntityPhysicsMover::Unk28( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c6790
void AEntityPhysicsMover::Unk2C( void* a_pUnk1, void* a_pUnk2 )
{
}

// $deBlob: FUNCTION 007c6780
void AEntityPhysicsMover::Unk30()
{
}

// $deBlob: FUNCTION 007c6670
void AEntityPhysicsMover::GetLinearVelocity( TPhysicsManager* a_pManager, TVector4& a_rvOutVelocity )
{
	a_pManager->GetLinearVelocity( m_pEntity->m_pPhysicsData->m_pBody, a_rvOutVelocity );
}
