#include "pch.h"
#include "ACharEntity.h"
#include "AGndMotorComponent.h"
#include "AGndMotorManager.h"
#include "PathFinding/APathfindingComponent.h"
#include "AEntityComponent2.h"
#include "AEntityData.h"
#include "AEntityIOPacket.h"
#include "AAttackMessage.h"
#include "AEntityHash.h"
#include "ABlobEntity/ABlobEntity.h"
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include "Collision/ACollisionObject.h"
#include "Collision/ACollisionManager2.h"
#include "AEntityModelComponent.h"
#include "Console/AConsoleState.h"
#include "Animation/AModelInstance2.h"
#include "Sound/ASoundBreakpointComponent.h"
#include <Physics/TSurfaceType.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void* ACharEntity::ms_pEntityData;

TSTATICASSERT( sizeof( ACharEntity ) == 0xBC );

// $deBlob: FUNCTION 005418b0
// $deBlob: FUNCTION 0085a0a0
static AConsoleVar s_cvCharShowMelee( "char_show_melee", TFALSE ); // 0x009b8be0

// $deBlob: FUNCTION 007c6ba0
// $deBlob: FUNCTION 007c6bb0
// $deBlob: FUNCTION 007c6bc0
// $deBlob: FUNCTION 007c6bd0
// $deBlob: FUNCTION 007c6be0
// $deBlob: FUNCTION 00541870
// $deBlob: FUNCTION 0085a090
TDEFINE_CLASS_NORUNTIME( ACharEntity );

// $deBlob: FUNCTION 007c6bf0
ACharEntity::ACharEntity()
{
	m_fTurnSpeed          = 0.0f;
	m_pMotorComponent     = TNULL;
	m_pCollisionObject    = TNULL;
	m_pBehaviourComponent = TNULL;
	m_pScheduleComponent  = TNULL;
	m_pPathfinding        = TNULL;
	m_pUnkB4              = TNULL;
	m_uiTurnFlags         = 0;
	m_iCurrentTask        = AITASK_INVALID;

	AConsoleState::RegisterCVar( &s_cvCharShowMelee );
}

// $deBlob: FUNCTION 007c6ca0
// $deBlob: FUNCTION 007c7d40
ACharEntity::~ACharEntity()
{
	if ( m_pCollisionObject )
		delete m_pCollisionObject;

	if ( !( m_Flags & FLAG_NoComponentDelete ) )
	{
		if ( m_pScheduleComponent )
			delete m_pScheduleComponent;

		if ( m_pBehaviourComponent )
			delete m_pBehaviourComponent;

		if ( m_pPathfinding )
			delete m_pPathfinding;

		if ( m_pMotorComponent )
			delete m_pMotorComponent;
	}
}

// $deBlob: FUNCTION 007c6d50
void ACharEntity::Tick( TFLOAT a_fDeltaTime )
{
	UpdateComponents( a_fDeltaTime );
	AVisibleEntity::Tick( a_fDeltaTime );

	if ( UpdateCurrentStateTask( a_fDeltaTime ) == TTRUE && m_iCurrentTask != AITASK_INVALID )
		m_iCurrentTask = AITASK_INVALID;

	// The footstep sounds play the ground's surface type
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();

	if ( pModelInstance->GetSoundBreakpointComponent() != TNULL )
	{
		const TCollisionSurface* pSurface = TSTATICCAST( AGndMotorComponent, m_pMotorComponent )->GetSurface();
		TSTATICCAST( ASoundBreakpointComponent, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetSoundBreakpointComponent() )->SetSurfaceType( TUINT16( pSurface->m_iType ) );
	}
}

// $deBlob: FUNCTION 007c6de0
void ACharEntity::OnLODChanging( TINT a_iLOD )
{
	if ( !( TUINT8( m_Flags >> 8 ) & 1 ) )
	{
		if ( a_iLOD == FLAG_HighLOD )
		{
			AGndMotorManager::GetSingleton()->RegisterMotorComponent( TSTATICCAST( AGndMotorComponent, GetMotorComponent() ) );
			AVisibleEntity::OnLODChanging( FLAG_HighLOD );
			return;
		}

		if ( GetLOD() == FLAG_HighLOD )
		{
			AGndMotorManager::GetSingleton()->UnregisterMotorComponent( TSTATICCAST( AGndMotorComponent, GetMotorComponent() ) );
		}
	}

	AVisibleEntity::OnLODChanging( a_iLOD );
}

// $deBlob: FUNCTION 007c6e60
void ACharEntity::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_UNK31 )
		m_pUnkB4 = a_pCaller;

	if ( m_pBehaviourComponent )
		m_pBehaviourComponent->HandleEntityIO( a_pCaller, a_rPacket );
}

// The melee hit: every blob (with its body in the simulation) within a_fRadius of the point in front of the character
// (m_vBBox.z + a_fHeight forward) and in front of its facing turned by a_fAngle (dot >= a_fMinDot) gets message
// a_eMessage with an AAttackMessage( a_iAttackType ). Returns the number of blobs hit.
// NOTE: a_iUnk4 isn't used
// $deBlob: FUNCTION 007c6e90
TINT ACharEntity::CheckMeleeCollision( TFLOAT a_fAngle, TFLOAT a_fMinDot, TFLOAT a_fRadius, TINT a_iUnk4, AEntityIOPacket::MESSAGE a_eMessage, TINT a_iAttackType, TINT a_iDamage, TINT a_iUnk8, TFLOAT a_fHeight )
{
	TVector4 vOffset;
	vOffset.Multiply( m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), m_vBBox.z + a_fHeight );

	const TVector4& rPosition = m_oMatrix.GetTranslation();
	TVector4        vCentre( rPosition.x + vOffset.x, rPosition.y + vOffset.y, rPosition.z + vOffset.z, rPosition.w );

	TINT          iCount     = 0;
	ABaseEntity** ppEntities = AEntityHash::GetSingleton()->Query( vCentre, a_fRadius, &iCount, TNULL, TFALSE, 500, TFALSE, TTRUE, TFALSE );

	TVector4 vFacing = m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vFacing.RotateY( a_fAngle );

	TINT iNumHits = 0;

	for ( TUINT i = 0; i < TUINT( iCount ); i++ )
	{
		ABaseEntity* pEntity = ppEntities[ i ];

		if ( pEntity == this || !pEntity->IsA( &TGetClass( ABlobEntity ) ) )
			continue;

		TPhysicsObjectContainer* pBody = *TREINTERPRETCAST( TPhysicsObjectContainer**, TREINTERPRETCAST( TUINT8*, pEntity ) + 0x3E8 ); // ABlobEntity::m_pBody

		if ( pBody == TNULL || !pBody->IsInSimulation() )
			continue;

		const TVector4& rTarget = pEntity->GetMatrix().GetTranslation();
		TVector4        vDir( rTarget.x - vCentre.x, rTarget.y - vCentre.y, rTarget.z - vCentre.z, rTarget.w );
		TFLOAT          fDistance = vDir.NormaliseMag();

		if ( fDistance - pEntity->GetBoundingSphereRadius() <= a_fRadius && a_fMinDot <= vFacing.z * vDir.z + vFacing.x * vDir.x )
		{
			iNumHits++;

			TVector4 vHit;
			vHit.Multiply( vDir, fDistance - pEntity->GetBoundingSphereRadius() );
			vHit = TVector4( vHit.x + vCentre.x, vHit.y + vCentre.y, vHit.z + vCentre.z, vCentre.w );

			AAttackMessage oMessage( AAttackMessage::ATTACKTYPE( a_iAttackType ), vHit, a_iDamage, a_iUnk8, vDir, TFALSE );
			AdjustAttackMessage( pEntity, &oMessage );

			AEntityIOPacket oPacket;
			oPacket.m_eMessage = a_eMessage;
			oPacket.m_iData    = TREINTERPRETCAST( TINT, &oMessage );
			pEntity->HandleEntityIO( this, oPacket );
		}
	}

	return iNumHits;
}

// $deBlob: FUNCTION 007c70d0
void* ACharEntity::GetMotorComponent()
{
	// NOTE: APathfindingComponent (unported) keeps the motor it was created with at 0x14
	return *TREINTERPRETCAST( void**, TREINTERPRETCAST( TUINT8*, m_pPathfinding ) + 0x14 );
}

// $deBlob: FUNCTION 007c70e0
TVector4 ACharEntity::GetEyePosition() const
{
	const TVector4& rPosition = m_oMatrix.GetTranslation();
	return TVector4( rPosition.x, rPosition.y - m_vBBox.y * 2.0f, rPosition.z, 1.0f );
}

// $deBlob: FUNCTION 007c7120
TVector4 ACharEntity::GetReactionForce( void* a_pVelocity, void* a_pEntity )
{
	ABaseEntity*    pOther    = TSTATICCAST( ABaseEntity, a_pEntity );
	const TVector4& rOtherPos = pOther->GetMatrix().GetTranslation();
	const TVector4& rPos      = m_oMatrix.GetTranslation();

	TVector4 vDir( rOtherPos.x - rPos.x, rOtherPos.y - rPos.y, rOtherPos.z - rPos.z, rOtherPos.w );
	vDir.NormaliseXZ();

	TVector4 vForce;
	vForce.Multiply( vDir, 4.5f );
	return vForce;
}

// $deBlob: FUNCTION 007c7190
void ACharEntity::StartStatesForTask( TINT a_iTask )
{
	TASSERT( AITASK_INVALID != a_iTask, "Cannot start an INVALID task\n" );
	TASSERT( a_iTask < 0x7F );

	m_iCurrentTask = TINT8( a_iTask );
	StartCurrentStateTask();
}

// $deBlob: FUNCTION 007c7220
void ACharEntity::StartInvalidTask()
{
	m_iCurrentTask = AITASK_INVALID;
	InitIdleState();
	TSTATICCAST( AEntityComponent2, GetPoseController() )->Tick( 0.0f );
}

// $deBlob: FUNCTION 007c7250
void ACharEntity::HandleTaskFailed( TINT a_iTask )
{
	InitIdleState();
	m_iCurrentTask = AITASK_INVALID;
}

// $deBlob: FUNCTION 007c7270
void ACharEntity::HandleTaskFinished( TINT a_iTask, TBOOL a_bNoAdvance )
{
	// NOTE: a TBOOL of AAISchedule (unported) at 0x18
	if ( *( TREINTERPRETCAST( TUINT8*, GetScheduleComponent() ) + 0x18 ) == 0 )
	{
		InitIdleState();
		m_iCurrentTask = AITASK_INVALID;
	}
}

// $deBlob: FUNCTION 007c72a0
void ACharEntity::SetPosition( const TVector4& a_rPosition )
{
	m_Flags |= FLAG_UpdateMatrix;
	m_oMatrix.GetTranslation() = a_rPosition;
}

// $deBlob: FUNCTION 007c72c0
TBOOL ACharEntity::CanOccupyPosition( const TVector4& a_rPosition )
{
	// A unit box standing on a_rPosition
	static const TVector4 s_vHalfExtents( 1.0f, 1.0f, 1.0f, 1.0f );

	TVector4 vCenter = a_rPosition;

	// NOTE: retail gets the collision object twice without using it
	GetCollisionObject();
	vCenter.y -= s_vHalfExtents.y;
	GetCollisionObject();

	TVector4 vNormal;
	return ACollisionManager2::CollideAABB( vCenter, s_vHalfExtents, vNormal, GetCollisionObject(), TNULL, 6 ) == TNULL;
}

// $deBlob: FUNCTION 007c7330
void ACharEntity::InitialiseStatic()
{
	ms_pEntityData = AEntityData::GetSingleton()->GetEntityData( "GlobalData_ACharEntityData_EntityData" );
}

// $deBlob: FUNCTION 007c7350
void ACharEntity::DeinitialiseStatic()
{
	ms_pEntityData = TNULL;
}

// $deBlob: FUNCTION 007c7360
void ACharEntity::Create()
{
	AVisibleEntity::Create();
	AddComponents();

	m_iLife = TINT16( GetMaxLife() );
	m_Flags &= ~FLAG_Dead;

	m_fBoundingSphereRadius = m_vBBox.Magnitude();

	GetCollisionObject()->SetFlags( ACollisionObject::FLAG( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty ) );

	if ( !( TUINT8( m_Flags >> 19 ) & 1 ) )
	{
		TSTATICCAST( APathfindingComponent, GetPathfinding() )->SetFlags( APathfindingComponent::FLAG_Walk );

		TFLOAT fSpeed = GetRunSpeed();
		TSTATICCAST( AGndMotorComponent, GetMotorComponent() )->SetSpeed( fSpeed );

		if ( TUINT8( m_Flags >> 8 ) & 1 )
		{
			AGndMotorManager::GetSingleton()->RegisterMotorComponent( TSTATICCAST( AGndMotorComponent, GetMotorComponent() ) );
		}
	}
}

// $deBlob: FUNCTION 007c7420
void ACharEntity::AddComponents()
{
	m_pCollisionObject = new ACollisionObject( this, 1, 0xFF );
	m_pCollisionObject->SetGroupMask( 0xFB );

	if ( !( TUINT8( m_Flags >> 19 ) & 1 ) )
	{
		m_pMotorComponent = new AGndMotorComponent( this );
		CreatePathfindingComponent();
		CreateBehaviourComponent();
		CreateScheduleComponent();
		InitialiseComponents();
	}
}

// $deBlob: FUNCTION 007c7500
void ACharEntity::ForceEndStatesForTask()
{
	m_iCurrentTask = AITASK_INVALID;
	InitIdleState();
	TSTATICCAST( AEntityComponent2, GetPoseController() )->Tick( 0.0f );
	TSTATICCAST( AEntityComponent2, GetMotorComponent() )->ResetComponent();
	TSTATICCAST( AEntityComponent2, GetPathfinding() )->ResetComponent();

	if ( GetBehaviourComponent() )
		TSTATICCAST( AEntityComponent2, GetBehaviourComponent() )->ResetComponent();

	if ( GetScheduleComponent() )
		TSTATICCAST( AEntityComponent2, GetScheduleComponent() )->ResetComponent();
}

// $deBlob: FUNCTION 007c7590
void ACharEntity::InitialiseComponents()
{
	ForceEndStatesForTask();

	TSTATICCAST( AEntityComponent2, GetMotorComponent() )->SetEnabled( TTRUE );
	TSTATICCAST( AEntityComponent2, GetPathfinding() )->SetEnabled( TTRUE );

	if ( GetBehaviourComponent() )
		TSTATICCAST( AEntityComponent2, GetBehaviourComponent() )->SetEnabled( TTRUE );

	if ( GetScheduleComponent() )
		TSTATICCAST( AEntityComponent2, GetScheduleComponent() )->SetEnabled( TTRUE );
}

// $deBlob: FUNCTION 007c75f0
void ACharEntity::CreateMotorComponent()
{
	m_pMotorComponent = new AGndMotorComponent( this );
}

// $deBlob: FUNCTION 007c7670
void ACharEntity::CreatePathfindingComponent()
{
	TASSERT( m_pMotorComponent, "Motor component must be created before the pathfinding" );
	m_pPathfinding = new APathfindingComponent( this, m_pMotorComponent );
}

// $deBlob: FUNCTION 007c7730
void ACharEntity::UpdateComponents( TFLOAT a_fDeltaTime )
{
	TVector4     vOldPosition = m_oMatrix.GetTranslation();
	ABaseEntity* pOldUnkB4    = m_pUnkB4;
	m_pUnkB4                  = TNULL;

	if ( !( m_Flags & FLAG_NoComponentDelete ) && m_pMotorComponent->IsEnabled() == TTRUE )
		m_pMotorComponent->Tick( a_fDeltaTime );

	if ( TVector4::DistanceSq( vOldPosition, m_oMatrix.GetTranslation() ) < TMath::TFLOAT_EPSILON && pOldUnkB4 != TNULL )
		m_pUnkB4 = pOldUnkB4;

	if ( !( m_Flags & FLAG_NoComponentDelete ) )
	{
		if ( m_pPathfinding->IsEnabled() == TTRUE )
			m_pPathfinding->Tick( a_fDeltaTime );

		if ( m_pBehaviourComponent && m_pBehaviourComponent->IsEnabled() == TTRUE )
			m_pBehaviourComponent->Tick( a_fDeltaTime );

		if ( m_pScheduleComponent && m_pScheduleComponent->IsEnabled() == TTRUE )
			m_pScheduleComponent->Tick( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 007c7840
void ACharEntity::UpdateFacing( TFLOAT a_fDeltaTime )
{
	const TurnSettings* pSettings = GetTurnSettings();
	TVector4            vFacing   = GetDesiredFacing();
	TurnToFace( a_fDeltaTime, vFacing, *pSettings );
}

// $deBlob: FUNCTION 007c7880
TVector4 ACharEntity::GetDesiredFacing()
{
	// NOTE: the desired facing of AGndMotorComponent (unported) at 0x5C
	return *TREINTERPRETCAST( TVector4*, TREINTERPRETCAST( TUINT8*, m_pMotorComponent ) + 0x5C );
}

// $deBlob: FUNCTION 007c78a0
const ACharEntity::TurnSettings* ACharEntity::GetTurnSettings()
{
	static TurnSettings sc_defaultTurnSettings = { 0.05f, TMath::TWO_PI, 0.1f, 18.0f, 18.0f };
	return &sc_defaultTurnSettings;
}

// $deBlob: FUNCTION 007c78f0
void ACharEntity::TurnToFace( TFLOAT a_fDeltaTime, const TVector4& a_rvTarget, const TurnSettings& a_rSettings )
{
	TASSERT( a_rvTarget.IsNormalised() );

	TMatrix44 oMatrix = m_oMatrix;
	TVector4  vFacing = oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	TFLOAT    fAngle  = TMath::AngleBetweenXZ( vFacing, a_rvTarget );

	// The turn speed accelerates while far from the target, decelerates once close
	if ( TMath::Abs( fAngle ) <= 0.08f || a_rSettings.m_fMaxTurnSpeed <= m_fTurnSpeed )
	{
		if ( TMath::Abs( fAngle ) <= 0.3f && a_rSettings.m_fMinTurnSpeed < m_fTurnSpeed )
			m_fTurnSpeed -= a_rSettings.m_fDeceleration * a_fDeltaTime;
	}
	else
	{
		m_fTurnSpeed += a_rSettings.m_fAcceleration * a_fDeltaTime;
	}

	if ( m_fTurnSpeed < a_rSettings.m_fMinTurnSpeed )
		m_fTurnSpeed = a_rSettings.m_fMinTurnSpeed;

	if ( a_rSettings.m_fMaxTurnSpeed < m_fTurnSpeed )
		m_fTurnSpeed = a_rSettings.m_fMaxTurnSpeed;

	if ( a_rSettings.m_fMinAngle < fAngle )
	{
		SetTurningDirection( TURNFLAG_Right );

		TFLOAT fStep = m_fTurnSpeed * TMath::PI * a_fDeltaTime;

		if ( fAngle < fStep )
			fStep = fAngle;

		oMatrix.RotateY( fStep );
		SetMatrix( oMatrix );
		return;
	}

	if ( fAngle < -a_rSettings.m_fMinAngle )
	{
		SetTurningDirection( TURNFLAG_Left );

		TFLOAT fStep = -( m_fTurnSpeed * TMath::PI * a_fDeltaTime );

		if ( fStep < fAngle )
			fStep = fAngle;

		oMatrix.RotateY( fStep );
		SetMatrix( oMatrix );
		return;
	}

	SetTurningDirection( 0 );
	m_fTurnSpeed = 0.0f;
}

// $deBlob: FUNCTION 007c7b00
void ACharEntity::StartCurrentStateTask()
{
	switch ( m_iCurrentTask )
	{
		case 0:
		case 1:
		case 2:
		case 7:
			InitMoveState();
			return;
		case 4:
			InitRecoilState();
			return;
		case 5:
		case 8:
		case 0xB:
		case 0xC:
		case 0xD:
			InitIdleState();
			return;
		case 6:
			InitAlarmState();
			return;
		case 0xA:
			InitAttackState();
			return;
		case 0xE:
			InitFidgetState();
			return;
		case 0xF:
			InitBlockState();
			return;
		case 0x10:
			InitDeathState();
			return;
	}
}

// $deBlob: FUNCTION 007c7ba0
void ACharEntity::UpdateStatesForTask( TFLOAT a_fDeltaTime )
{
	if ( UpdateCurrentStateTask( a_fDeltaTime ) == TTRUE && m_iCurrentTask != AITASK_INVALID )
		m_iCurrentTask = AITASK_INVALID;
}

// $deBlob: FUNCTION 007c7be0
TBOOL ACharEntity::UpdateCurrentStateTask( TFLOAT a_fDeltaTime )
{
	switch ( m_iCurrentTask )
	{
		case 0:
		case 1:
		case 2:
		case 7:
			return UpdateMoveState( a_fDeltaTime );
		case 4:
			return UpdateRecoilState( a_fDeltaTime );
		case 5:
		case 8:
		case 0xB:
		case 0xC:
		case 0xD:
			return UpdateIdleState( a_fDeltaTime );
		case 6:
			return UpdateAlarmState( a_fDeltaTime );
		case 0xA:
			return UpdateAttackState( a_fDeltaTime );
		case 0xE:
			return UpdateFidgetState( a_fDeltaTime );
		case 0xF:
			return UpdateBlockState( a_fDeltaTime );
		case 0x10:
			return UpdateDeathState( a_fDeltaTime );
		default:
			return TTRUE;
	}
}
