#include "pch.h"
#include "AVisibleEntity.h"
#include "AEntityPhysicsMover.h"
#include "AFXBreakpointComponent.h"
#include "AEntityParams.h"
#include "AEntityFactory.h"
#include "AEntityModelComponent.h"
#include "AEntityPoseController.h"
#include "AEntityStateComponent.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionInstance2.h"
#include "Fx/Shadows/AShadow.h"

#include "Collision/ACollisionCommon.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/TPhysicsResponder.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <Physics/TSphereData.h>
#include <Physics/TBoxData.h>
#include <Physics/TCapsuleData.h>
#include <Physics/TCylinderData.h>
#include <Physics/Dynamics/Entity/hkpRigidBody.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007c2c80
// $deBlob: FUNCTION 007c2c90
// $deBlob: FUNCTION 007c2ca0
// $deBlob: FUNCTION 007c2cb0
// $deBlob: FUNCTION 007c2cc0
// $deBlob: FUNCTION 005417f0
// $deBlob: FUNCTION 0085a070
TDEFINE_CLASS_NORUNTIME( AVisibleEntity );

// $deBlob: FUNCTION 007c2cd0
AVisibleEntity::AVisibleEntity()
{
	m_Flags |= 0x20000;
	m_pModelComponent        = TNULL;
	m_pShadow                = TNULL;
	m_fAmbientScale          = 0.5f;
	m_pPoseController        = TNULL;
	m_pStateComponent        = TNULL;
	m_pFXBreakpointComponent = TNULL;
	m_pPhysicsData           = TNULL;
}

// $deBlob: FUNCTION 007c2d30
// $deBlob: FUNCTION 007c4240
AVisibleEntity::~AVisibleEntity()
{
	if ( !( m_Flags & FLAG_NoComponentDelete ) )
	{
		if ( m_pFXBreakpointComponent )
			delete m_pFXBreakpointComponent;

		if ( m_pPoseController )
			delete m_pPoseController;

		if ( m_pModelComponent )
			delete m_pModelComponent;

		if ( m_pStateComponent )
			delete m_pStateComponent;
	}

	DestroyPhysicsData();

	if ( m_pShadow )
		delete m_pShadow;
}

// $deBlob: FUNCTION 007c2e00
void AVisibleEntity::Create()
{
	ABaseEntity::Create();

	if ( !( m_Flags & FLAG_NoComponentDelete ) )
	{
		CreateModelComponent( TNULL );
		CreatePoseController();
		CreateStateComponent();
		CreateFXBreakpointComponent( TNULL );
		SetVisible( TTRUE );
	}
}

void AVisibleEntity::UpdateModelVisibility( TBOOL a_bVisible )
{
	AModelInstance2* pModelInstance = m_pModelComponent->GetModelInstance();

	if ( !a_bVisible || ( !( m_Flags & FLAG_HighLOD ) && !( m_Flags & FLAG_ModelAlwaysLOD ) ) )
	{
		pModelInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}
	else
	{
		pModelInstance->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
		m_pModelComponent->SetTransform( m_oMatrix );
	}
}

// $deBlob: FUNCTION 007c2e90
void AVisibleEntity::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pPoseController )
		m_pPoseController->Tick( a_fDeltaTime );

	if ( m_pModelComponent )
	{
		if ( m_Flags & FLAG_VisibleDirty )
		{
			m_Flags &= ~FLAG_VisibleDirty;
			UpdateModelVisibility( m_Flags & FLAG_Visible );
		}

		m_pModelComponent->Tick( a_fDeltaTime );
	}

	if ( m_pFXBreakpointComponent )
		m_pFXBreakpointComponent->Tick( a_fDeltaTime );

	if ( m_pStateComponent )
		m_pStateComponent->Tick( a_fDeltaTime );

	ABaseEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007c2f80
void AVisibleEntity::SetVisible( TBOOL a_bVisible )
{
	if ( a_bVisible )
		m_Flags |= FLAG_Visible;
	else
		m_Flags &= ~FLAG_Visible;

	if ( m_pModelComponent == TNULL )
		m_Flags |= FLAG_VisibleDirty;
	else
		UpdateModelVisibility( a_bVisible );

	if ( m_pFXBreakpointComponent )
		TSTATICCAST( AFXBreakpointComponent, m_pFXBreakpointComponent )->SetVisible( a_bVisible );

	if ( m_pShadow )
		m_pShadow->SetHidden( !a_bVisible );

	if ( !a_bVisible )
	{
		RemoveFromPhysics();
	}
	else if ( !( GetLOD() & FLAG_LowLOD ) )
	{
		AddToPhysics();
	}
}

// $deBlob: FUNCTION 007c3050
void AVisibleEntity::PreRenderUpdate()
{
	if ( ( m_Flags & FLAG_UpdateMatrix ) && m_pModelComponent )
		m_pModelComponent->SetTransform( m_oMatrix );

	if ( ( TUINT8( m_Flags >> 2 ) & 1 ) && m_pShadow )
		m_pShadow->Update( m_oMatrix.GetTranslation(), m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), ( m_Flags & FLAG_UpdateMatrix ) != 0 );
}

// $deBlob: FUNCTION 007c30a0
void AVisibleEntity::OnLODChanging( TINT a_iLOD )
{
	if ( m_pModelComponent == TNULL )
		return;

	AModelInstance2* pModelInstance = m_pModelComponent->GetModelInstance();

	if ( !( a_iLOD & FLAG_HighLOD ) )
	{
		if ( !( m_Flags & FLAG_ModelAlwaysLOD ) )
			pModelInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

		if ( m_pShadow )
			m_pShadow->SetHidden( TTRUE );

		if ( m_pFXBreakpointComponent )
			TSTATICCAST( AFXBreakpointComponent, m_pFXBreakpointComponent )->SetVisible( TFALSE );
	}
	else
	{
		if ( !( m_Flags & FLAG_ModelAlwaysLOD ) )
			pModelInstance->SetFlag( AModelInstance2::FLAG_Visible, ( TUINT8( m_Flags >> 2 ) & 1 ) != 0 );

		if ( TUINT8( m_Flags >> 2 ) & 1 )
		{
			m_pModelComponent->SetTransform( m_oMatrix );
			m_Flags |= FLAG_UpdateMatrix;
		}

		if ( m_pShadow )
			m_pShadow->SetHidden( TFALSE );

		if ( m_pFXBreakpointComponent )
			TSTATICCAST( AFXBreakpointComponent, m_pFXBreakpointComponent )->SetVisible( TTRUE );
	}

	if ( !( a_iLOD & FLAG_LowLOD ) )
	{
		if ( ( GetLOD() & FLAG_LowLOD ) && ( TUINT8( m_Flags >> 2 ) & 1 ) )
			AddToPhysics();

		return;
	}

	RemoveFromPhysics();
}

// $deBlob: FUNCTION 007c31a0
void AVisibleEntity::SyncBodyTransform()
{
	if ( m_pPhysicsData && m_pPhysicsData->m_pPhysicsMover )
		TSTATICCAST( AEntityPhysicsMover, m_pPhysicsData->m_pPhysicsMover )->SetBodyTransform( m_oMatrix );
}

// $deBlob: FUNCTION 007c31c0
void AVisibleEntity::Reset( ABaseEntity* a_pEntity )
{
	SyncBodyTransform();

	if ( TUINT8( m_Flags >> 24 ) & 1 )
	{
		AddToPhysics();
		SetVisible( TTRUE );
		SetTicking( TFALSE );
		m_Flags &= ~( 0x1000000 | FLAG_DontTick );
	}

	m_Flags = ( m_Flags & ~0x1C00 ) | FLAG_LOD0;
}

// $deBlob: FUNCTION 007c3210
void AVisibleEntity::AddToPhysics()
{
	if ( TUINT8( m_Flags >> 21 ) & 1 )
	{
		SyncBodyTransform();

		if ( m_pPhysicsData )
		{
			if ( m_pPhysicsData->m_pPhysicsMover )
				TPhysicsManager::GetSingleton()->AddCustomMover( m_pPhysicsData->m_pPhysicsMover );

			TPhysicsManager::GetSingleton()->AddToSimulation( m_pPhysicsData->m_pBody, TFALSE );
			TPhysicsManager::GetSingleton()->UpdateBodyTransform( m_pPhysicsData->m_pBody, TFALSE );
		}

		if ( m_pModelComponent && m_pModelComponent->GetModelInstance()->GetCollisionInstance() )
		{
			ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, m_pModelComponent->GetModelInstance()->GetCollisionInstance() );
			pCollision->AddToWorld( ACollisionInstance2::PhysicsObjectType_Standard | ACollisionInstance2::PhysicsObjectType_Custom );
			pCollision->UpdateBodyTransforms();
		}
	}
}

// $deBlob: FUNCTION 007c32a0
void AVisibleEntity::RemoveFromPhysics()
{
	if ( m_pPhysicsData )
	{
		if ( m_pPhysicsData->m_pPhysicsMover )
			TPhysicsManager::GetSingleton()->RemoveCustomMover( m_pPhysicsData->m_pPhysicsMover );

		TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pPhysicsData->m_pBody, TFALSE );
	}

	if ( m_pModelComponent && m_pModelComponent->GetModelInstance()->GetCollisionInstance() )
		TSTATICCAST( ACollisionInstance2, m_pModelComponent->GetModelInstance()->GetCollisionInstance() )->RemoveFromWorld( ACollisionInstance2::PhysicsObjectType_Standard | ACollisionInstance2::PhysicsObjectType_Custom );
}

// $deBlob: FUNCTION 007c3300
void AVisibleEntity::SetPhysicsEnabled( TBOOL a_bEnable )
{
	if ( !a_bEnable )
	{
		if ( TUINT8( m_Flags >> 21 ) & 1 )
		{
			m_Flags &= ~FLAG_PhysicsEnabled;
			RemoveFromPhysics();
		}
	}
	else if ( !( TUINT8( m_Flags >> 21 ) & 1 ) )
	{
		m_Flags |= FLAG_PhysicsEnabled;

		if ( !( GetLOD() & FLAG_LowLOD ) && ( TUINT8( m_Flags >> 2 ) & 1 ) )
			AddToPhysics();
	}
}

// $deBlob: FUNCTION 007c3370
void AVisibleEntity::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );
	a_rParams.GetValue( m_fAmbientScale, "Ambient Scale" );
}

// $deBlob: FUNCTION 007c33a0
void AVisibleEntity::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_DESPAWN && ( TUINT8( m_Flags >> 2 ) & 1 ) )
		Despawn( a_rPacket.m_iData != 0 );
}

// $deBlob: FUNCTION 007c33d0
void AVisibleEntity::Despawn( TBOOL a_bPlayEffect )
{
	if ( !( TUINT8( m_Flags >> 24 ) & 1 ) )
	{
		void* pDespawnEffect = GetDespawnEffect();

		if ( a_bPlayEffect && pDespawnEffect )
		{
			// The effect of the removal at the entity
			TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( TSTATICCAST( TParticleEffect, pDespawnEffect ) );

			if ( pEffect != TNULL )
				pEffect->GetTransform() = m_oMatrix;
		}

		RemoveFromPhysics();
		SetVisible( TFALSE );
		SetTicking( TFALSE );
		m_Flags |= 0x1000000 | FLAG_DontTick;
		AEntityFactory::GetSingleton()->DeactivateEntity( *this );
	}
}

// $deBlob: FUNCTION 007c3500
void AVisibleEntity::CreatePoseController()
{
	TASSERT( TNULL == m_pPoseController );
	m_pPoseController = new AEntityPoseController( this, TNULL );
}

// $deBlob: FUNCTION 007c35b0
void AVisibleEntity::CreateModelComponent( const TCHAR* a_szModelName )
{
	TASSERT( TNULL == m_pModelComponent );
	m_pModelComponent = new AEntityModelComponent( this, a_szModelName ? a_szModelName : GetModelName() );
	m_pModelComponent->GetModelInstance()->SetAmbientScale( m_fAmbientScale );
}

// $deBlob: FUNCTION 007c3670
void AVisibleEntity::CreateStateComponent()
{
	TASSERT( TNULL == m_pStateComponent, "TNULL==m_pEntityStateComponent" );
	m_pStateComponent = new AEntityStateComponent( this );
}

// $deBlob: FUNCTION 007c3720
void AVisibleEntity::CreateFXBreakpointComponent( const TCHAR* a_szName )
{
	TASSERT( TNULL == m_pFXBreakpointComponent );

	if ( a_szName == TNULL )
		a_szName = GetModelName();

	m_pFXBreakpointComponent = AFXBreakpointComponent::Create( a_szName, TMemory_GetGlobalHeap(), this, TNULL );
}

// $deBlob: FUNCTION 007c3790
void AVisibleEntity::CreatePhysics( const AEntityTypeData::TRBData* a_pTRBData, TShapeBaseClass** a_ppShapes, const TMatrix44* a_pMatrices, TINT a_iNumObjects, AEntityPhysicsData* a_pPhysicsData, TBOOL a_bFixed )
{
	if ( a_pPhysicsData == TNULL )
	{
		TASSERT( TNULL == m_pPhysicsData );
		m_pPhysicsData               = TSTATICCAST( AEntityPhysicsData, TMalloc( sizeof( AEntityPhysicsData ) ) );
		m_pPhysicsData->m_eOwnerType = 0;
		m_pPhysicsData->m_pOwner     = TNULL;
	}
	else
	{
		TASSERT( TNULL == m_pPhysicsData || a_pPhysicsData == m_pPhysicsData );
		m_pPhysicsData = a_pPhysicsData;
	}

	m_pPhysicsData->m_pCollisionEventHandler = TNULL;
	m_pPhysicsData->m_pPhysicsMover          = TNULL;
	m_pPhysicsData->m_eOwnerType             = 2;
	m_pPhysicsData->m_pOwner                 = this;
	m_pPhysicsData->m_oMatrix.Identity();

	if ( a_iNumObjects == 1 )
	{
		m_pPhysicsData->m_oOffset = *a_pMatrices;

		TMatrix44 matCopy = m_pPhysicsData->m_oMatrix;
		m_pPhysicsData->m_oMatrix.Multiply( matCopy, m_pPhysicsData->m_oOffset );
	}
	else
	{
		m_pPhysicsData->m_oOffset.Identity();
	}

	TPhysicsManager*         pPhysicsManager = TPhysicsManager::GetSingleton();
	TPhysicsObjectContainer* pBody;

	if ( !a_bFixed )
	{
		TINT eMotionType = TPhysicsManager::MotionType_Static;

		if ( a_pTRBData->m_eMotionType == 1 )
			eMotionType = ( a_pTRBData->m_bMotionSubType != 0 ) + 1;
		else if ( a_pTRBData->m_eMotionType == 2 )
			eMotionType = TPhysicsManager::MotionType_Dynamic;

		pBody = pPhysicsManager->CreateBody( &m_pPhysicsData->m_oMatrix, TNULL, eMotionType, TPhysicsManager::SubType_Default );
	}
	else
	{
		pBody = pPhysicsManager->CreateFixedBody( &m_pPhysicsData->m_oMatrix, TNULL );
	}

	m_pPhysicsData->m_pBody = pBody;
	pBody->m_fMass          = a_pTRBData->m_fMass;

	TASSERT( a_iNumObjects > 0 );

	for ( TINT i = 0; i < a_iNumObjects; i++ )
	{
		TShapeBaseClass* pShape = a_ppShapes[ i ];

		switch ( pShape->GetType() )
		{
			case TShapeBaseClass::Type_Sphere:
				if ( a_iNumObjects == 1 )
					pPhysicsManager->SetSphere( m_pPhysicsData->m_pBody, TSTATICCAST( TSphereData, pShape )->GetRadius() );
				else
					pPhysicsManager->AddListSphere( m_pPhysicsData->m_pBody, a_pMatrices[ i ], TSTATICCAST( TSphereData, pShape )->GetRadius() );
				break;
			case TShapeBaseClass::Type_Box:
				if ( a_iNumObjects == 1 )
					pPhysicsManager->SetBox( m_pPhysicsData->m_pBody, TSTATICCAST( TBoxData, pShape )->GetHalfExtents() );
				else
					pPhysicsManager->AddListBox( m_pPhysicsData->m_pBody, a_pMatrices[ i ], TSTATICCAST( TBoxData, pShape )->GetHalfExtents() );
				break;
			case TShapeBaseClass::Type_Capsule:
				if ( a_iNumObjects == 1 )
					pPhysicsManager->SetCapsule( m_pPhysicsData->m_pBody, TSTATICCAST( TCapsuleData, pShape )->GetHeight(), TSTATICCAST( TCapsuleData, pShape )->GetRadius() );
				else
					pPhysicsManager->AddListCapsule( m_pPhysicsData->m_pBody, a_pMatrices[ i ], TSTATICCAST( TCapsuleData, pShape )->GetHeight(), TSTATICCAST( TCapsuleData, pShape )->GetRadius() );
				break;
			case TShapeBaseClass::Type_Cylinder:
				if ( a_iNumObjects == 1 )
					pPhysicsManager->SetCylinder( m_pPhysicsData->m_pBody, TSTATICCAST( TCylinderData, pShape )->GetHeight(), TSTATICCAST( TCylinderData, pShape )->GetRadius() );
				else
					pPhysicsManager->AddListCylinder( m_pPhysicsData->m_pBody, a_pMatrices[ i ], TSTATICCAST( TCylinderData, pShape )->GetHeight(), TSTATICCAST( TCylinderData, pShape )->GetRadius() );
				break;
		}
	}

	TFLOAT fFriction    = 0.5f;
	TFLOAT fRestitution = 0.2f;
	ACollisionCommon::GetSurfaceParams( a_pTRBData->m_uiPhysicsFlags, fFriction, fRestitution );
	pBody->m_fFriction    = fFriction;
	pBody->m_fRestitution = fRestitution;

	pBody->m_fLinearDamping = a_pTRBData->m_fLinearDamping;
	if ( pBody->m_body )
		pBody->m_body->setLinearDamping( a_pTRBData->m_fLinearDamping );

	pBody->m_fAngularDamping = a_pTRBData->m_fAngularDamping;
	if ( pBody->m_body )
		pBody->m_body->setAngularDamping( a_pTRBData->m_fAngularDamping );

	pBody->m_flags &= ~0x10;

	pBody->m_fMaxLinearVelocity = a_pTRBData->m_fMaxLinearVelocity;
	if ( pBody->m_body )
		pBody->m_body->setMaxLinearVelocity( a_pTRBData->m_fMaxLinearVelocity );

	pBody->m_fMaxAngularVelocity = a_pTRBData->m_fMaxAngularVelocity;
	if ( pBody->m_body )
		pBody->m_body->setMaxAngularVelocity( a_pTRBData->m_fMaxAngularVelocity );

	TUINT32 uiLayer = 2;

	if ( a_pTRBData->m_uiPhysicsFlags & 0x2010 )
		uiLayer = 0x20;
	else if ( a_pTRBData->m_uiPhysicsFlags & 0x4400000 )
		uiLayer = 0x100;

	pPhysicsManager->SetCollisionLayer( pBody, uiLayer );

	static TCONSTEXPR_VAR TUINT32 s_auiGroups[ 8 ] = { 0x81, 2, 4, 8, 0x10, 0x20, 0x40, 0x100 };

	TUINT32 uiGroup = 0;

	for ( TINT i = 0; i < 8; i++ )
	{
		if ( a_pTRBData->m_abCollisionGroups[ i ] != 0 )
			uiGroup |= s_auiGroups[ i ];
	}

	pPhysicsManager->SetCollisionGroup( pBody, ( pBody->m_eMotionType == TPhysicsManager::MotionType_KeyframedReporting ) ? 0x200 : uiGroup );

	pBody->m_uiSurface = a_pTRBData->m_uiPhysicsFlags;
	pBody->m_pUserData = &m_pPhysicsData->m_eOwnerType;
}

// $deBlob: FUNCTION 007c3c60
void AVisibleEntity::CreatePhysics( const AEntityTypeData::TRBData* a_pTRBData, const AEntityParam::Collision* a_pCollision, AEntityPhysicsData* a_pPhysicsData, TBOOL a_bFixed )
{
	AEntityTypeData::TRBData oTRBData;
	oTRBData.m_szClassName         = a_pTRBData->m_szClassName;
	oTRBData.m_szModelName         = a_pTRBData->m_szModelName;
	oTRBData.m_fMass               = a_pTRBData->m_fMass;
	oTRBData.m_uiPhysicsFlags      = a_pCollision->m_uiPhysicsFlags;
	oTRBData.m_aUnk10[ 0 ]         = a_pCollision->m_uiUnk04;
	oTRBData.m_aUnk10[ 1 ]         = a_pCollision->m_uiUnk00;
	oTRBData.m_fLinearDamping      = a_pTRBData->m_fLinearDamping;
	oTRBData.m_fAngularDamping     = a_pTRBData->m_fAngularDamping;
	oTRBData.m_fMaxLinearVelocity  = a_pTRBData->m_fMaxLinearVelocity;
	oTRBData.m_fMaxAngularVelocity = a_pTRBData->m_fMaxAngularVelocity;

	TUINT32 uiGroups                  = a_pCollision->m_uiGroupFlags;
	oTRBData.m_abCollisionGroups[ 0 ] = ( uiGroups & 0x81 ) != 0;
	oTRBData.m_abCollisionGroups[ 1 ] = ( uiGroups & 2 ) != 0;
	oTRBData.m_abCollisionGroups[ 2 ] = ( uiGroups & 4 ) != 0;
	oTRBData.m_abCollisionGroups[ 3 ] = ( uiGroups & 8 ) != 0;
	oTRBData.m_abCollisionGroups[ 4 ] = ( uiGroups & 0x10 ) != 0;
	oTRBData.m_abCollisionGroups[ 5 ] = ( uiGroups & 0x20 ) != 0;
	oTRBData.m_abCollisionGroups[ 6 ] = ( uiGroups & 0x40 ) != 0;
	oTRBData.m_abCollisionGroups[ 7 ] = ( uiGroups & 0x100 ) != 0;

	oTRBData.m_eMotionType     = a_pTRBData->m_eMotionType;
	oTRBData.m_bMotionSubType  = a_pTRBData->m_bMotionSubType;
	oTRBData.m_szDespawnEffect = a_pTRBData->m_szDespawnEffect;

	TINT              iNumShapes = a_pCollision->m_iNumShapes;
	TShapeBaseClass** ppShapes   = TSTATICCAST( TShapeBaseClass*, TMemalign( 16, iNumShapes * sizeof( TShapeBaseClass* ) ) );
	TMatrix44*        pMatrices  = TSTATICCAST( TMatrix44, TMemalign( 16, iNumShapes * sizeof( TMatrix44 ) ) );

	for ( TINT i = 0; i < iNumShapes; i++ )
	{
		const AEntityParam::CollisionShape* pShape = a_pCollision->m_ppShapes[ i ];

		// NOTE: an unknown shape type leaves its shape and matrix uninitialised (as in retail)
		TBOOL bKnown = TTRUE;

		switch ( pShape->m_eType )
		{
			case 2:
			{
				TSphereData* pSphere = new TSphereData();
				pSphere->SetRadius( pShape->m_afValues[ 0 ] );
				ppShapes[ i ] = pSphere;
				break;
			}
			case 3:
			{
				TCapsuleData* pCapsule = new TCapsuleData();
				pCapsule->SetRadius( pShape->m_afValues[ 0 ] );
				pCapsule->SetHeight( pShape->m_afValues[ 1 ] );
				ppShapes[ i ] = pCapsule;
				break;
			}
			case 4:
			{
				TCylinderData* pCylinder = new TCylinderData();
				pCylinder->SetRadius( pShape->m_afValues[ 0 ] );
				pCylinder->SetHeight( pShape->m_afValues[ 1 ] );
				ppShapes[ i ] = pCylinder;
				break;
			}
			case 5:
			{
				TBoxData* pBox = new TBoxData();
				pBox->SetHalfExtents( TVector4( pShape->m_afValues[ 0 ], pShape->m_afValues[ 1 ], pShape->m_afValues[ 2 ], 1.0f ) );
				ppShapes[ i ] = pBox;
				break;
			}
			default:
				bKnown = TFALSE;
				break;
		}

		if ( bKnown )
			pMatrices[ i ] = *pShape->m_pMatrix;

		if ( pShape->m_eType == 3 || pShape->m_eType == 4 )
		{
			// Capsules and cylinders: the rows are rotated (z, x, y)
			TVector4* pRows = TREINTERPRETCAST( TVector4*, &pMatrices[ i ] );
			TVector4  vRow0 = pRows[ 0 ];
			TVector4  vRow1 = pRows[ 1 ];
			pRows[ 0 ]      = pRows[ 2 ];
			pRows[ 1 ]      = vRow0;
			pRows[ 2 ]      = vRow1;
		}
	}

	CreatePhysics( &oTRBData, ppShapes, pMatrices, iNumShapes, a_pPhysicsData, a_bFixed );

	for ( TINT i = 0; i < iNumShapes; i++ )
	{
		if ( ppShapes[ i ] )
			delete ppShapes[ i ];

		ppShapes[ i ] = TNULL;
	}

	::operator delete( ppShapes );
	::operator delete( pMatrices );
}

// $deBlob: FUNCTION 007c3fd0
void AVisibleEntity::SetPhysicsMover( TPhysicsMotionBase* a_pPhysicsMover )
{
	TASSERT( m_pPhysicsData );
	TASSERT( TNULL == m_pPhysicsData->m_pPhysicsMover );

	m_pPhysicsData->m_pPhysicsMover = a_pPhysicsMover;

	if ( m_pPhysicsData->m_pBody && m_pPhysicsData->m_pBody->IsInSimulation() )
		TPhysicsManager::GetSingleton()->AddCustomMover( m_pPhysicsData->m_pPhysicsMover );
}

// $deBlob: FUNCTION 007c40a0
void AVisibleEntity::SetCollisionEventHandler( TPhysicsResponder* a_pHandler )
{
	TASSERT( m_pPhysicsData );
	TASSERT( TNULL == m_pPhysicsData->m_pCollisionEventHandler );

	m_pPhysicsData->m_pCollisionEventHandler = a_pHandler;
	TPhysicsManager::GetSingleton()->SetListener( m_pPhysicsData->m_pBody, a_pHandler );
}

// $deBlob: FUNCTION 007c4150
void AVisibleEntity::DestroyPhysicsData()
{
	if ( m_pPhysicsData )
	{
		TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
		pPhysicsManager->RemoveCustomMover( m_pPhysicsData->m_pPhysicsMover );
		pPhysicsManager->RemoveFromSimulation( m_pPhysicsData->m_pBody, TFALSE );
		pPhysicsManager->DestroyBody( m_pPhysicsData->m_pBody );

		if ( m_pPhysicsData->m_pCollisionEventHandler )
			delete m_pPhysicsData->m_pCollisionEventHandler;

		m_pPhysicsData->m_pCollisionEventHandler = TNULL;

		if ( m_pPhysicsData->m_pPhysicsMover )
			delete m_pPhysicsData->m_pPhysicsMover;

		m_pPhysicsData->m_pPhysicsMover = TNULL;

		::operator delete( m_pPhysicsData );
		m_pPhysicsData = TNULL;
	}
}

// $deBlob: FUNCTION 007c3460
void AVisibleEntity::CreateShadow( TFLOAT a_fSize, TFLOAT a_fAlpha )
{
	AShadow* pShadow = new AShadow( a_fSize, 0 );
	pShadow->SetAlpha( a_fAlpha );
	m_pShadow = pShadow;
	pShadow->Init( a_fSize, 0 );
}
