#include "pch.h"
#include "ACollisionInstanceMover.h"
#include "ACollisionInstance2.h"

#include "Cameras/ACameraManager.h"
#include "Terrain2/ATerrain2Interface.h"
#include "GameInterface/APlayerManager.h"
#include "Input/AVibrationManager.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Render/TRenderContext.h>

#include <Physics/Dynamics/Entity/hkpRigidBody.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005a6a70
ACollisionInstanceMover::ACollisionInstanceMover( TPhysicsObjectContainer* a_pBody, ACollisionInstance2* a_pInstance, TINT16 a_iIndex )
    : m_iUnk08( -1 )
    , m_pBody( a_pBody )
    , m_pInstance( a_pInstance )
    , m_mInstance( TMatrix44::IDENTITY )
    , m_mInstanceInverse( TMatrix44::IDENTITY )
    , m_fTime( 0.0f )
    , m_fDynamicTime( 0.0f )
    , m_fHideDelay( 0.0f )
    , m_fFallDelay( 0.0f )
    , m_fRemoveDelay( 0.0f )
    , m_iIndex( a_iIndex )
    , m_uiFlags( Flags_ReturnToStatic | Flags_MotionStateStatic )
    , m_uiCollisionFilter( 0 )
{
	m_iWorld = 0;

	TUINT32 uiGroup     = m_pBody ? ( m_pBody->m_uiCollisionFilterInfo & 0xfff ) : 0;
	m_uiCollisionFilter = uiGroup;

	if ( m_pBody )
		m_uiCollisionFilter = ( ( ( m_pBody->m_uiCollisionFilterInfo >> 12 ) & 0xfff ) << 16 ) | uiGroup;
	else
		m_uiCollisionFilter = uiGroup;
}

// $deBlob: FUNCTION 005a6980
ACollisionInstanceMover::ACollisionInstanceMover( TINT16 a_iIndex )
    : m_iUnk08( -1 )
    , m_pBody( TNULL )
    , m_pInstance( TNULL )
    , m_mInstance( TMatrix44::IDENTITY )
    , m_mInstanceInverse( TMatrix44::IDENTITY )
    , m_fTime( 0.0f )
    , m_fDynamicTime( 0.0f )
    , m_fHideDelay( 0.0f )
    , m_fFallDelay( 0.0f )
    , m_fRemoveDelay( 0.0f )
    , m_iIndex( a_iIndex )
    , m_uiFlags( Flags_ReturnToStatic | Flags_MotionStateStatic )
    , m_uiCollisionFilter( 0 )
{
	m_iWorld = 0;
}

// $deBlob: FUNCTION 005a76a0
void ACollisionInstanceMover::SetBodyAndInstance( TPhysicsObjectContainer* a_pBody, ACollisionInstance2* a_pInstance )
{
	m_pBody     = a_pBody;
	m_pInstance = a_pInstance;

	if ( a_pInstance != TNULL )
		m_mInstance = a_pInstance->m_pModelInstance->GetTransform().GetMatrix();
}

// Activates a static body in the simulation, deactivates the others
static void UpdateActivation( TPhysicsObjectContainer* a_pBody )
{
	if ( a_pBody && a_pBody->IsInSimulation() )
	{
		// NOTE: retail activates static bodies (hkpEntity::activate in Havok 5.5) and deactivates the others
		// (hkpEntity::deactivate)
		if ( a_pBody->m_eMotionType == TPhysicsManager::MotionType_Static )
			a_pBody->m_body->activate();
		else
			a_pBody->m_body->deactivate();
	}
}

// The largest of the three scale factors (the first one on ties)
static TFLOAT GetMaxScale( const TVector3& a_rcScale )
{
	const TFLOAT* pMax = &a_rcScale.x;

	if ( *pMax <= a_rcScale.y && a_rcScale.y != *pMax )
		pMax = &a_rcScale.y;

	if ( *pMax <= a_rcScale.z && a_rcScale.z != *pMax )
		pMax = &a_rcScale.z;

	return *pMax;
}

// Raises the model by the size of its bounding sphere times a_fFactor
static void OffsetModel( TModelInstance* a_pModelInstance, TMatrix44& a_rModelMatrix, TFLOAT a_fFactor )
{
	T2ModelPtr pModel  = a_pModelInstance->GetModel();
	TFLOAT     fRadius = pModel.GetData()->GetLOD( 0 ).m_RenderVolume.GetRadius();

	a_rModelMatrix.m_f42 = fRadius * GetMaxScale( a_pModelInstance->GetTransform().GetScale() ) * a_fFactor + a_rModelMatrix.m_f42;
	a_pModelInstance->GetTransform().SetMatrix( a_rModelMatrix );
}

// $deBlob: FUNCTION 005a6bb0
void ACollisionInstanceMover::Update( TFLOAT a_fTimeStep, TPhysicsManager* a_pManager )
{
	if ( !m_pInstance || !m_pBody || !m_pBody->IsInSimulation() )
		return;

	TUINT16 uiFlags = m_uiFlags;

	if ( !( TUINT8( uiFlags >> 8 ) & 1 ) )
	{
		if ( TUINT8( uiFlags >> 7 ) & 1 )
		{
			// Keyframed: move the body to its bone (or to the instance's matrix)
			if ( a_pManager->GetNumSteps() > 0 )
				return;

			TSkeletonInstance* pSkeletonInstance = TNULL;
			m_uiFlags                            = uiFlags & ~Flags_FallPending;
			TModelInstance* pModelInstance       = m_pInstance->m_pModelInstance;
			TINT            iBone                = -1;

			if ( m_iIndex != -1 && !( TUINT8( uiFlags >> 3 ) & 1 ) )
			{
				T2ModelPtr pModel = pModelInstance->GetModel();
				iBone             = pModel.GetData()->GetCollisionData( m_iIndex ).m_iBone;
				pSkeletonInstance = pModelInstance->GetSkeletonInstance();
			}

			if ( iBone == -1 && !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
				return;

			TMatrix44 mTarget = pModelInstance->GetTransform().GetLocalMatrix();

			if ( !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
			{
				TMatrix44 mBone = TMatrix44::IDENTITY;
				pSkeletonInstance->GetBoneTransformCurrent( iBone, mBone );
				mTarget.Multiply( mBone );
			}

			TMatrix44 mInv = m_mInstanceInverse;
			mInv.Invert( mInv );
			mTarget.Multiply( mInv );

			TPhysicsManager::ApplyHardKeyFrame( m_pBody, mTarget, a_pManager->GetTimeStep() );
			return;
		}

		// Static: becomes dynamic after m_fFallDelay seconds
		if ( !( uiFlags & Flags_MotionStateStatic ) || !( TUINT8( uiFlags >> 1 ) & 1 ) )
			return;

		m_fTime += a_fTimeStep;

		if ( m_fTime <= m_fFallDelay )
			return;

		m_fTime        = 0.0f;
		m_fDynamicTime = 0.0f;
		a_pManager->SetMotionType( m_pBody, TPhysicsManager::MotionType_Dynamic );

		TPhysicsObjectContainer* pBody = m_pBody;
		m_uiCollisionFilter            = 0;
		TUINT32 uiLow                  = pBody ? ( pBody->m_uiCollisionFilterInfo & 0xfff ) : 0;
		TUINT32 uiHigh                 = pBody ? ( ( pBody->m_uiCollisionFilterInfo >> 12 ) & 0xfff ) : 0;
		m_uiCollisionFilter            = ( uiHigh << 16 ) | ( uiLow & 0xffff );

		SetMotionState( Flags_MotionStateDynamic );
		return;
	}

	// Dynamic: the model follows the body
	m_fDynamicTime += a_fTimeStep;
	m_fTime += a_fTimeStep;

	TMatrix44*      pBodyMatrix    = m_pBody->m_pMatrix;
	TModelInstance* pModelInstance = m_pInstance->m_pModelInstance;
	TMatrix44       mModel;
	mModel.Multiply( *pBodyMatrix, m_mInstanceInverse );
	TTransformObject& rTransform = pModelInstance->GetTransform();
	rTransform.SetMatrix( mModel );

	if ( TUINT8( m_uiFlags >> 0 ) & 1 )
	{
		TFLOAT fEnd = m_fHideDelay + 1.0f;

		if ( m_fTime <= fEnd )
		{
			if ( m_fHideDelay < m_fTime )
			{
				TFLOAT fLeft = fEnd - m_fTime;
				OffsetModel( pModelInstance, mModel, 1.0f - ( fLeft <= 0.0f ? 0.0f : fLeft ) );
				SetState( Flags_StateFadingOut );
			}
		}
		else if ( !IsOnScreen() )
		{
			// Off screen: put it back where the instance has it, static
			pModelInstance->m_eFlags |= TModelInstance::Flags_ForceFade;
			pModelInstance->SetFade( 1.0f );
			rTransform.SetMatrix( m_mInstance );
			pModelInstance->m_eFlags = ( pModelInstance->m_eFlags & ~TModelInstance::Flags_Visible ) | TModelInstance::Flags_Visible;

			ACollisionInstance2::OwnerObject* pOwner = m_pInstance->m_pOwner;

			if ( pOwner )
			{
				pOwner->m_uiFlags |= 0x800000;
				ATerrain2Interface::GetInstance()->SetInstanceInCellList( TREINTERPRETCAST( TerrainTypes::CellInstance*, pOwner ), TFALSE );
			}

			a_pManager->SetMotionType( m_pBody, TPhysicsManager::MotionType_Static );

			TMatrix44 mInv = m_mInstanceInverse;
			mInv.Invert( mInv );
			TMatrix44 mBody;
			mBody.Multiply( m_mInstance, mInv );
			TPhysicsManager::SetTransform( m_pBody, mBody, TFALSE );

			UpdateActivation( m_pBody );

			a_pManager->SetCollisionGroup( m_pBody, m_uiCollisionFilter >> 16 );
			a_pManager->SetCollisionLayer( m_pBody, TINT16( m_uiCollisionFilter ) );
			m_fTime        = 0.0f;
			m_uiFlags      = m_uiFlags & 0xe1fd;
			m_fDynamicTime = 0.0f;
			SetMotionState( Flags_MotionStateStatic );
			return;
		}
		else if ( !( TUINT8( m_uiFlags >> 9 ) & 1 ) )
		{
			UpdateActivation( m_pBody );

			a_pManager->SetCollisionGroup( m_pBody, 0 );
			a_pManager->SetCollisionLayer( m_pBody, 0 );
			pModelInstance->SetFade( 0.0f );
			pModelInstance->m_eFlags = ( pModelInstance->m_eFlags & ~TModelInstance::Flags_Visible ) | TModelInstance::Flags_ForceFade;

			ACollisionInstance2::OwnerObject* pOwner = m_pInstance->m_pOwner;

			if ( pOwner )
			{
				pOwner->m_uiFlags &= ~0x800000;
				ATerrain2Interface::GetInstance()->SetInstanceInCellList( TREINTERPRETCAST( TerrainTypes::CellInstance*, pOwner ), TFALSE );
			}

			SetState( Flags_StateInvisible );
		}
	}

	if ( TUINT8( m_uiFlags >> 2 ) & 1 )
	{
		TFLOAT fEnd = m_fRemoveDelay + 1.0f;

		if ( fEnd < m_fTime )
		{
			// Remove it from the world
			m_pInstance->m_uiFlags |= 0x10;
			pModelInstance->m_eFlags &= ~TModelInstance::Flags_Visible;

			ACollisionInstance2::OwnerObject* pOwner = m_pInstance->m_pOwner;

			if ( pOwner )
			{
				pOwner->m_uiFlags &= ~0x800000;
				ATerrain2Interface::GetInstance()->SetInstanceInCellList( TREINTERPRETCAST( TerrainTypes::CellInstance*, pOwner ), TFALSE );
			}

			a_pManager->SetMotionType( m_pBody, TPhysicsManager::MotionType_Static );
			m_pInstance->RemoveFromWorld( 0x60 );
			m_fTime        = 0.0f;
			m_uiFlags      = m_uiFlags & ~Flags_FallPending;
			m_fDynamicTime = 0.0f;
			SetState( Flags_StateRemoved );
			return;
		}

		if ( m_fRemoveDelay < m_fTime )
		{
			TFLOAT fLeft = fEnd - m_fTime;
			OffsetModel( pModelInstance, mModel, 1.0f - ( fLeft <= 0.0f ? 0.0f : fLeft ) );
			SetState( Flags_StateFadingOut );
		}
	}

	uiFlags = m_uiFlags;

	if ( !( uiFlags & Flags_ReturnToStatic ) || ( uiFlags & Flags_StateMask ) || ( TUINT8( uiFlags >> 5 ) & 1 ) )
		return;

	static TFLOAT s_fMaxRestAngle = 0.17453292f; // 10 degrees

	if ( m_fDynamicTime <= 1.5f )
		return;

	// Back to static once the body rests (within 1 unit and 10 degrees) where the instance has it
	TMatrix44 mInv = m_mInstanceInverse;
	mInv.Invert( mInv );
	TMatrix44 mExpected;
	mExpected.Multiply( m_mInstance, mInv );

	TVector4 vDiff;
	vDiff.x = pBodyMatrix->m_f41 - mExpected.m_f41;
	vDiff.y = pBodyMatrix->m_f42 - mExpected.m_f42;
	vDiff.z = pBodyMatrix->m_f43 - mExpected.m_f43;
	vDiff.w = pBodyMatrix->m_f44;

	if ( vDiff.MagnitudeSq() <= 1.0f )
	{
		mExpected.SetTranslation( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );
		mExpected.InvertOrthonormal();

		TMatrix44 mBody = *pBodyMatrix;
		mBody.SetTranslation( TVector4( 0.0f, 0.0f, 0.0f, 1.0f ) );

		TMatrix44 mDiff;
		mDiff.Multiply( mBody, mExpected );

		TQuaternion qDiff;
		TMatrix44::MatToQuat( qDiff, mDiff );
		qDiff /= qDiff.Magnitude();

		TFLOAT fAngle = TMath::ACos( TMath::Abs( qDiff.w ) );

		if ( fAngle + fAngle <= s_fMaxRestAngle )
		{
			a_pManager->SetMotionType( m_pBody, TPhysicsManager::MotionType_Static );
			m_fTime        = 0.0f;
			m_uiFlags      = m_uiFlags & ~Flags_FallPending;
			m_fDynamicTime = 0.0f;

			if ( m_pInstance->m_pOwner )
				ATerrain2Interface::GetInstance()->SetInstanceInCellList( TREINTERPRETCAST( TerrainTypes::CellInstance*, m_pInstance->m_pOwner ), TFALSE );

			SetMotionState( Flags_MotionStateStatic );
			return;
		}
	}

	TUINT32 uiGroup = m_pBody ? ( ( m_pBody->m_uiCollisionFilterInfo >> 12 ) & 0xfff ) : 0;

	if ( uiGroup & 0x46 )
	{
		a_pManager->SetCollisionLayer( m_pBody, 0 );
		a_pManager->SetCollisionGroup( m_pBody, uiGroup & ~0x46 );
		SetState( Flags_StateCollisionDisable );
	}
}

// $deBlob: FUNCTION 005a7680
void ACollisionInstanceMover::GetLinearVelocity( TPhysicsManager* a_pManager, TVector4& a_rvOutVelocity )
{
	a_pManager->GetLinearVelocity( m_pBody, a_rvOutVelocity );
}

// $deBlob: FUNCTION 005a7720
void ACollisionInstanceMover::SetSinkAndHide( TBOOL a_bSet, TFLOAT a_fParam )
{
	if ( a_bSet )
		m_uiFlags |= Flags_SinkAndHide;
	else
		m_uiFlags &= ~Flags_SinkAndHide;

	m_fHideDelay = a_fParam;
	m_fTime      = 0.0f;
}

// $deBlob: FUNCTION 005a7760
void ACollisionInstanceMover::SetSinkAndRemove( TBOOL a_bSet, TFLOAT a_fParam )
{
	if ( a_bSet )
		m_uiFlags |= Flags_SinkAndRemove;
	else
		m_uiFlags &= ~Flags_SinkAndRemove;

	m_fRemoveDelay = a_fParam;
	m_fTime        = 0.0f;
}

// $deBlob: FUNCTION 005a77a0
void ACollisionInstanceMover::SetInverseMatrix( const TMatrix44& a_rMatrix )
{
	m_mInstanceInverse = a_rMatrix;
	m_mInstanceInverse.Invert( m_mInstanceInverse );
}

// $deBlob: FUNCTION 005a7950
void ACollisionInstanceMover::ResetToInstance()
{
	TPhysicsManager* pManager = TPhysicsManager::GetSingleton();

	if ( !( m_uiFlags & Flags_MotionStateDynamic ) || !m_pInstance || !m_pBody )
		return;

	// The body goes back to where the instance has it
	TMatrix44 mInv = m_mInstanceInverse;
	mInv.Invert( mInv );
	TMatrix44 mBody;
	mBody.Multiply( m_mInstance, mInv );
	*m_pBody->m_pMatrix = mBody;
	TPhysicsManager::UpdateBodyTransform( m_pBody, TFALSE );

	UpdateActivation( m_pBody );

	pManager->SetCollisionGroup( m_pBody, TUINT16( m_uiCollisionFilter >> 16 ) );
	pManager->SetCollisionLayer( m_pBody, TUINT16( m_uiCollisionFilter ) );

	TModelInstance* pModelInstance = m_pInstance->m_pModelInstance;
	pModelInstance->m_eFlags |= TModelInstance::Flags_ForceFade;
	pModelInstance->SetFade( 1.0f );
	pModelInstance->GetTransform().SetMatrix( m_mInstance );
	pModelInstance->m_eFlags |= TModelInstance::Flags_Visible;

	ACollisionInstance2::OwnerObject* pOwner = m_pInstance->m_pOwner;

	if ( pOwner )
	{
		ATerrain2Interface::GetInstance()->SetInstanceInCellList( TREINTERPRETCAST( TerrainTypes::CellInstance*, pOwner ), TFALSE );
	}

	m_fTime        = 0.0f;
	m_fDynamicTime = 0.0f;
	m_uiFlags      = ( m_uiFlags & 0xe07d ) | Flags_MotionStateStatic;
}

// $deBlob: FUNCTION 005a7cf0
void ACollisionInstanceMover::SetMotionState( TUINT16 a_uiFlag )
{
	TASSERT( a_uiFlag & Flags_MotionStateStatic || a_uiFlag & Flags_MotionStateKeyframe || a_uiFlag & Flags_MotionStateDynamic );
	m_uiFlags = ( m_uiFlags & ~Flags_MotionStateMask ) | a_uiFlag;
}

// $deBlob: FUNCTION 005a7c90
void ACollisionInstanceMover::SetState( TUINT16 a_uiFlag )
{
	TASSERT( a_uiFlag & Flags_StateInvisible || a_uiFlag & Flags_StateCollisionDisable || a_uiFlag & Flags_StateRemoved || a_uiFlag & Flags_StateFadingOut );
	m_uiFlags = ( m_uiFlags & ~Flags_StateMask ) | a_uiFlag;
}

// $deBlob: FUNCTION 005a7d60
TBOOL ACollisionInstanceMover::IsOnScreen()
{
	TModelInstance* pModelInstance = m_pInstance->m_pModelInstance;
	T2ModelPtr      pModel         = pModelInstance->GetModel();
	TSphere         oVolume        = pModel.GetData()->GetLOD( 0 ).m_RenderVolume;
	const TVector3& rScale         = pModelInstance->GetTransform().GetScale();

	TVector3 vCentre( oVolume.GetOrigin().x * rScale.x, oVolume.GetOrigin().y * rScale.y, oVolume.GetOrigin().z * rScale.z );
	TFLOAT   fRadius = GetMaxScale( rScale ) * oVolume.GetRadius();

	if ( fRadius > 0.0f )
	{
		TMatrix44::TransformVector( vCentre, m_mInstance, vCentre );

		// NOTE: retail also fetches the current render context here and doesn't use it
		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

		if ( !TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), pCamera->GetViewPlanes(), 6 ) )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005a7ad0
void ACollisionInstanceMover::SnapToBone()
{
	if ( !( m_uiFlags & Flags_MotionStateDynamic ) && !( TUINT8( m_uiFlags >> 6 ) & 1 ) )
	{
		if ( !( TUINT8( m_uiFlags >> 7 ) & 1 ) )
			return;

		TSkeletonInstance* pSkeletonInstance = TNULL;
		TModelInstance*    pModelInstance    = m_pInstance->m_pModelInstance;
		TINT               iBone             = -1;

		if ( m_iIndex != -1 && !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
		{
			T2ModelPtr pModel = pModelInstance->GetModel();
			iBone             = pModel.GetData()->GetCollisionData( m_iIndex ).m_iBone;
			pSkeletonInstance = pModelInstance->GetSkeletonInstance();
		}

		if ( iBone == -1 && !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
			return;

		TMatrix44 mTarget = pModelInstance->GetTransform().GetLocalMatrix();

		if ( !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
		{
			TMatrix44 mBone = TMatrix44::IDENTITY;
			pSkeletonInstance->GetBoneTransformCurrent( iBone, mBone );
			mTarget.Multiply( mBone );
		}

		TMatrix44 mInv = m_mInstanceInverse;
		mInv.Invert( mInv );
		mTarget.Multiply( mInv );

		TPhysicsManager::SetTransform( m_pBody, mTarget, TFALSE );
		TPhysicsManager::StopBody( m_pBody );
	}

	TPhysicsManager::UpdateBodyTransform( m_pBody, TFALSE );
}

// $deBlob: FUNCTION 005a77e0
void ACollisionInstanceMover::OnHit()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( !( m_uiFlags & Flags_FallPending ) && APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetCurrentPlayerBlob();
		AVibrationManager::VibrateOnce( APlayerManager::GetSingleton()->GetPlayerIndex( pBlob ), 4 );
	}

	m_uiFlags |= Flags_FallPending;

	if ( ( m_uiFlags & Flags_MotionStateStatic ) && m_fFallDelay <= 0.0f )
	{
		// A static body hit by the player turns dynamic
		m_fTime        = 0.0f;
		m_fDynamicTime = 0.0f;
		pPhysicsManager->SetMotionType( m_pBody, TPhysicsManager::MotionType_Dynamic );

		TUINT32 uiGroup     = m_pBody ? ( m_pBody->m_uiCollisionFilterInfo & 0xfff ) : 0;
		m_uiCollisionFilter = uiGroup;

		if ( m_pBody )
			m_uiCollisionFilter = ( ( ( m_pBody->m_uiCollisionFilterInfo >> 12 ) & 0xfff ) << 16 ) | uiGroup;
		else
			m_uiCollisionFilter = uiGroup;

		m_uiFlags = ( m_uiFlags & ~( Flags_MotionStateStatic | Flags_MotionStateKeyframe ) ) | Flags_MotionStateDynamic;

		if ( m_pInstance->m_pOwner )
			ATerrain2Interface::GetInstance()->SetInstanceInCellList( TREINTERPRETCAST( TerrainTypes::CellInstance*, m_pInstance->m_pOwner ), TTRUE );
	}

	if ( TUINT8( m_uiFlags >> 8 ) & 1 )
	{
		if ( ( m_uiFlags & Flags_SinkAndHide ) && m_fTime < m_fHideDelay )
			m_fTime = 0.0f;

		if ( TUINT8( m_uiFlags >> 4 ) & 1 )
			m_fDynamicTime = 0.0f;
	}
}
