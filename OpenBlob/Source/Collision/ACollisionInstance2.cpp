#include "pch.h"
#include "ACollisionInstance2.h"
#include "ACollisionCommon.h"
#include "ACollisionInstanceResponderMover.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsConstraintContainer.h>
#include <Render/TMesh.h>
#include <Render/TMaterial.h>
#include <Platform/DX11/TMaterial_DX11.h>

#include <Physics/Dynamics/Entity/hkpRigidBody.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The most collision meshes an instance makes bodies for
static TCONSTEXPR_VAR TINT MAX_PHYSICS_OBJECTS = 32;

// $deBlob: FUNCTION 005a32b0
ACollisionInstance2::ACollisionInstance2()
    : m_pModelInstance( TNULL )
    , m_pOwner( TNULL )
    , m_iNumPhysicsObjects( 0 )
    , m_pPhysicsInfos( TNULL )
    , m_uiFlags( 0 )
    , m_pConstraint( TNULL )
    , m_pResponder( TNULL )
{
}

// $deBlob: FUNCTION 005a32f0
// $deBlob: FUNCTION 00572bf0
ACollisionInstance2::~ACollisionInstance2()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( m_pConstraint )
	{
		pPhysicsManager->DestroyConstraint( m_pConstraint );
		m_pConstraint = TNULL;
	}

	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		PhysicsInfo& rInfo = m_pPhysicsInfos[ i ];

		if ( rInfo.m_pBody )
		{
			if ( rInfo.m_pInstanceMover )
			{
				pPhysicsManager->RemoveCustomMover( rInfo.m_pInstanceMover );

				if ( rInfo.m_pInstanceMover )
					delete rInfo.m_pInstanceMover;

				rInfo.m_pInstanceMover = TNULL;
			}

			pPhysicsManager->RemoveFromSimulation( rInfo.m_pBody, TFALSE );
			pPhysicsManager->DestroyBody( rInfo.m_pBody );

			if ( rInfo.m_pMeshContainer )
			{
				TPhysicsMeshContainer::Destroy( rInfo.m_pMeshContainer );
				rInfo.m_pMeshContainer = TNULL;
			}

			rInfo.m_pBody = TNULL;
			rInfo.m_mWorld.Identity();
			rInfo.m_mLocal.Identity();
		}
	}

	::operator delete( m_pPhysicsInfos );

	if ( m_pResponder )
		delete m_pResponder;

	m_pResponder = TNULL;
}

// $deBlob: FUNCTION 005a3470
void ACollisionInstance2::SetModelInstance( TModelInstance* a_pModelInstance, OwnerObject* a_pOwner )
{
	m_pModelInstance = a_pModelInstance;
	m_pOwner         = a_pOwner;
}

// Removes the scale of a body matrix whose basis vectors aren't unit length
// $deBlob: FUNCTION 005a62a0
static void RemoveScale( TMatrix44& a_rMatrix )
{
	if ( TMath::Abs( a_rMatrix.AsBasisVector4( 0 ).MagnitudeSq() - 1.0f ) <= 0.01f &&
	     TMath::Abs( a_rMatrix.AsBasisVector4( 1 ).MagnitudeSq() - 1.0f ) <= 0.01f &&
	     TMath::Abs( a_rMatrix.AsBasisVector4( 2 ).MagnitudeSq() - 1.0f ) <= 0.01f )
		return;

	TVector4 vScale;
	a_rMatrix.GetScale( vScale );
	a_rMatrix.Scale( TVector4( 1.0f / vScale.x, 1.0f / vScale.y, 1.0f / vScale.z, 1.0f ) );
}

// $deBlob: FUNCTION 005a34a0
void ACollisionInstance2::SetMatrix( const TMatrix44& a_rMatrix )
{
	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		PhysicsInfo& rInfo = m_pPhysicsInfos[ i ];

		// The mover moves the dynamic and keyframed bodies itself
		if ( rInfo.m_pInstanceMover && ( ( rInfo.m_pInstanceMover->GetFlags() & ACollisionInstanceMover::Flags_MotionStateDynamic ) || ( rInfo.m_pInstanceMover->GetFlags() & ACollisionInstanceMover::Flags_MotionStateKeyframe ) ) )
			continue;

		if ( rInfo.m_pBody )
		{
			if ( rInfo.m_pBody->m_eShapeType == 6 )
			{
				TModel* pModel = m_pModelInstance->GetModel().GetData();
				rInfo.m_mWorld.Multiply( a_rMatrix, pModel->GetCollisionData( i ).m_mTransform );
			}
			else
			{
				rInfo.m_mWorld.Multiply( a_rMatrix, rInfo.m_mLocal );
			}

			RemoveScale( rInfo.m_mWorld );
		}

		if ( rInfo.m_pInstanceMover )
		{
			rInfo.m_pInstanceMover->GetInstanceMatrix() = a_rMatrix;
		}
	}
}

// $deBlob: FUNCTION 005a3640
void ACollisionInstance2::UpdateBodyTransforms()
{
	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		PhysicsInfo& rInfo = m_pPhysicsInfos[ i ];

		if ( rInfo.m_pBody )
			TPhysicsManager::UpdateBodyTransform( rInfo.m_pBody, TFALSE );

		if ( rInfo.m_pInstanceMover )
			rInfo.m_pInstanceMover->SnapToBone();
	}
}

// $deBlob: FUNCTION 005a36a0
void ACollisionInstance2::Create( TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, TBOOL a_bPrimitives, TINT a_iNumShapeTypes, TINT* a_pShapeTypes )
{
	TModel* pModel = a_pModelInstance->GetModel().GetData();

	if ( pModel->GetCollisionDataCount() <= 0 )
		return;

	TINT iNumModels = TMath::Min( pModel->GetCollisionDataCount(), MAX_PHYSICS_OBJECTS );
	AllocatePhysicsInfos( iNumModels );

	// Is it grass (the first surface of a collision mesh)?
	TBOOL bGrass = TFALSE;

	for ( TINT i = 0; i < iNumModels; i++ )
	{
		TModelCollisionMesh& rCollisionData = pModel->GetCollisionData( i );

		TUINT32 uiLayer   = 0;
		TUINT32 uiSurface = 0;
		TUINT32 uiGroup;
		GetCollisionFilter( rCollisionData.m_pMaterials[ 0 ].m_szName, a_pOwnerInfo->m_eType, uiLayer, uiGroup, uiSurface );

		if ( uiSurface == ACollisionCommon::SURFACE_Grass )
		{
			bGrass = TTRUE;
			break;
		}
	}

	// Is it a hot plate (a material of the model)?
	TBOOL bHotplate = TFALSE;

	{
		TModelLOD& rLOD = pModel->GetLOD( 0 );

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			T2Material*  pMetaMaterial = TSTATICCAST( T2Material, rLOD.GetMeshes()[ i ]->GetMaterial()->GetMetaMaterial() );
			const TCHAR* szName        = pMetaMaterial->GetName();

			if ( szName != TNULL && TStringManager::String8Compare( szName, "hotplate" ) == 0 )
			{
				bHotplate = TTRUE;
				break;
			}
		}
	}

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	TINT             iPhysicsObject  = 0;
	TINT             iOldWorld       = pPhysicsManager->GetDefaultWorld();

	for ( TINT iModel = 0; iModel < iNumModels; iModel++ )
	{
		TModelCollisionMesh& rCollisionData = pModel->GetCollisionData( iModel );
		const TCHAR*         szMaterial     = rCollisionData.m_pMaterials[ 0 ].m_szName;
		TBOOL                bDecal         = ACollisionCommon::GetSurfaceType( szMaterial ) == ACollisionCommon::SURFACE_Decal;

		if ( m_pOwner && !( GetOwnerFlags() & BITFLAG( 17 ) ) && bDecal )
		{
			m_iNumPhysicsObjects--;
			continue;
		}

		TBOOL bStatic = rCollisionData.m_iBone == -1 && ( !m_pOwner || ( !( GetOwnerFlags() & BITFLAG( 5 ) ) && !( GetOwnerFlags() & BITFLAG( 16 ) ) ) );

		if ( !bStatic && iPhysicsObject > 0 && bDecal )
		{
			m_iNumPhysicsObjects--;
			continue;
		}

		// The decals go to the second world
		pPhysicsManager->SetDefaultWorld( bDecal ? APhysicsWorldType_Decal : APhysicsWorldType_GamePlay );

		PhysicsInfo&             rInfo = m_pPhysicsInfos[ iPhysicsObject ];
		TPhysicsObjectContainer* pBody;

		if ( bStatic )
		{
			pBody         = pPhysicsManager->CreateBody( &rInfo.m_mWorld, TNULL, TPhysicsManager::MotionType_Static, TPhysicsManager::SubType_SphereInertia );
			rInfo.m_pBody = pBody;
			m_uiFlags |= Flags_HasStatic;
		}
		else
		{
			pBody         = pPhysicsManager->CreateBody( &rInfo.m_mWorld, TNULL, TPhysicsManager::MotionType_Keyframed, TPhysicsManager::SubType_Default );
			rInfo.m_pBody = pBody;
			m_uiFlags |= Flags_HasDynamic;

			if ( !a_bPrimitives )
				a_bPrimitives = TTRUE;
		}

		TUINT32 uiLayer   = 0;
		TUINT32 uiGroup   = 0;
		TUINT32 uiSurface = 0;
		GetCollisionFilter( szMaterial, a_pOwnerInfo->m_eType, uiLayer, uiGroup, uiSurface );

		if ( iNumModels == 1 && ( !m_pOwner || ( GetOwnerFlags() & BITFLAG( 17 ) ) ) )
			uiLayer |= 8;

		if ( uiLayer & 0x81 )
		{
			if ( m_pOwner && !( GetOwnerFlags() & BITFLAG( 17 ) ) )
				uiLayer |= 0x400;
			else if ( bGrass )
				uiLayer |= 0x400;
		}

		if ( m_pOwner && !( GetOwnerFlags() & 0x80 ) )
			uiLayer &= ~0x10;

		if ( bGrass || bHotplate )
			uiLayer &= ~8;

		TFLOAT fFriction    = 1.0f;
		TFLOAT fRestitution = 0.5f;
		ACollisionCommon::GetSurfaceParams( uiSurface, fFriction, fRestitution );

		if ( a_bPrimitives )
		{
			// A primitive shape fitted to the collision mesh (a box unless the caller gives the shape types)
			TINT iShapeType = 3;

#ifdef DEBLOB_COMMUNITY_PATCH
			// NOTE: retail reads one past the end of a_pShapeTypes when iPhysicsObject == a_iNumShapeTypes
			if ( 0 < a_iNumShapeTypes && iPhysicsObject < a_iNumShapeTypes )
#else
			if ( 0 < a_iNumShapeTypes && iPhysicsObject <= a_iNumShapeTypes )
#endif
				iShapeType = a_pShapeTypes[ iPhysicsObject ];

			TFLOAT    afParams[ 4 ];
			TMatrix44 mTransform;
			GetPrimitiveShape( &rCollisionData, iShapeType, afParams, mTransform );

			switch ( iShapeType )
			{
				case 2:
				{
					TPhysicsManager::AddListSphere( rInfo.m_pBody, mTransform, afParams[ 0 ] );
				}
				break;
				case 4:
				{
					if ( afParams[ 0 ] <= 0.0f )
						TPhysicsManager::AddListSphere( rInfo.m_pBody, mTransform, afParams[ 1 ] );
					else
						TPhysicsManager::AddListCapsule( rInfo.m_pBody, mTransform, afParams[ 0 ], afParams[ 1 ] );
				}
				break;
				case 5:
				{
					TPhysicsManager::AddListCylinder( rInfo.m_pBody, mTransform, afParams[ 0 ], afParams[ 1 ] );
				}
				break;
				default:
				{
					TPhysicsManager::AddListBox( rInfo.m_pBody, mTransform, *TREINTERPRETCAST( TVector4*, afParams ) );
				}
				break;
			}

			if ( !( uiLayer & 8 ) )
				uiLayer |= 0x400;
		}
		else
		{
			TASSERT( rCollisionData.GetBoundingRadius() > 0.0f );

			TPhysicsMeshContainer* pContainer = TPhysicsMeshContainer::Create();
			pContainer->SetCollisionData( &rCollisionData );
			rInfo.m_pMeshContainer = pContainer;
			TPhysicsManager::SetMesh( pBody, pContainer );
		}

		rInfo.m_pBody->m_fMass = 1.0f;

		if ( rInfo.m_pBody )
			rInfo.m_pBody->m_fFriction = fFriction;

		if ( rInfo.m_pBody )
			rInfo.m_pBody->m_fRestitution = fRestitution;

		rInfo.m_oOwnerInfo = *a_pOwnerInfo;

		if ( rInfo.m_pBody )
			rInfo.m_pBody->m_pUserData = &rInfo.m_oOwnerInfo;

		if ( !a_bPrimitives && rCollisionData.m_iNumMaterials > 1 )
		{
			// Each surface of the mesh: its material and the union of their filters on the body
			for ( TINT iMaterial = 0; iMaterial < rCollisionData.m_iNumMaterials; iMaterial++ )
			{
				TPhysicsMeshMaterial* pMaterial = TNULL;

				if ( rInfo.m_pBody && rInfo.m_pBody->m_eShapeType == 6 )
					pMaterial = rInfo.m_pBody->m_pMeshContainer->GetMaterial( iMaterial );

				GetCollisionFilter( rCollisionData.m_pMaterials[ iMaterial ].m_szName, a_pOwnerInfo->m_eType, uiLayer, uiGroup, uiSurface );

				if ( iNumModels == 1 )
				{
					if ( !m_pOwner || ( GetOwnerFlags() & BITFLAG( 17 ) ) )
						uiLayer |= 8;

					if ( uiSurface & ( ACollisionCommon::SURFACE_ForceField | ACollisionCommon::SURFACE_ForceFieldEdge ) )
						uiLayer &= ~8;
				}

				if ( m_pOwner )
				{
					if ( !( GetOwnerFlags() & BITFLAG( 17 ) ) && ( uiLayer & 0x81 ) )
						uiLayer |= 0x400;

					if ( !( GetOwnerFlags() & 0x80 ) )
						uiLayer &= ~0x10;
				}

				if ( !pMaterial )
					uiSurface = 0;

				ACollisionCommon::GetSurfaceParams( uiSurface, fFriction, fRestitution );

				if ( pMaterial )
				{
					pMaterial->m_uiFilterInfo = ( uiGroup & 0xfff ) << 12 | ( pMaterial->m_uiFilterInfo & 0xff000000 ) | ( uiLayer & 0xfff );
					pMaterial->m_uiSurface    = uiSurface;
					pMaterial->m_fFriction    = fFriction;
					pMaterial->m_fRestitution = fRestitution;
				}

				TPhysicsObjectContainer* POC = rInfo.m_pBody;

				TUINT32 uiOldLayer = POC ? POC->m_uiCollisionFilterInfo & 0xfff : 0;
				TUINT32 uiOldGroup = POC ? ( POC->m_uiCollisionFilterInfo >> 12 ) & 0xfff : 0;

				TASSERT( POC != TNULL );
				TUINT32 uiOldSurface = POC ? POC->m_uiSurface : 0;

				pPhysicsManager->SetCollisionLayer( rInfo.m_pBody, uiOldLayer | uiLayer );
				pPhysicsManager->SetCollisionGroup( rInfo.m_pBody, uiOldGroup | uiGroup );

				if ( rInfo.m_pBody )
					rInfo.m_pBody->m_uiSurface = uiOldSurface | uiSurface;
			}
		}
		else
		{
			pPhysicsManager->SetCollisionLayer( rInfo.m_pBody, uiLayer );
			pPhysicsManager->SetCollisionGroup( rInfo.m_pBody, uiGroup );

			if ( rInfo.m_pBody )
				rInfo.m_pBody->m_uiSurface = uiSurface;
		}

		if ( ( !m_pOwner || !( GetOwnerFlags() & BITFLAG( 16 ) ) ) && rCollisionData.m_iBone != -1 )
		{
			TASSERT( rInfo.m_pInstanceMover == TNULL );

			// The body follows its bone (keyframed by the mover)
			rInfo.m_pInstanceMover = new ACollisionInstanceMover( rInfo.m_pBody, this, TINT16( iModel ) );
			rInfo.m_pInstanceMover->SetMotionState( ACollisionInstanceMover::Flags_MotionStateKeyframe );

			if ( m_pOwner && ( GetOwnerFlags() & BITFLAG( 16 ) ) )
				rInfo.m_pInstanceMover->AddFlags( ACollisionInstanceMover::Flags_NoBone );

			pPhysicsManager->AddCustomMover( rInfo.m_pInstanceMover );
		}

		iPhysicsObject++;
	}

	TASSERT( iOldWorld == APhysicsWorldType_GamePlay );
	pPhysicsManager->SetDefaultWorld( iOldWorld );
}

// $deBlob: FUNCTION 005a4260
void ACollisionInstance2::CreateConstraint( PhysicsRecord* a_pRecord, const TMatrix44* a_pMatrix )
{
	if ( a_pRecord->m_iNumConstraints <= 0 )
		return;

	ConstraintDef*           pDef     = a_pRecord->m_ppConstraints[ 0 ];
	TPhysicsObjectContainer* pBodyA   = pDef->m_pDefA->m_pBody;
	TPhysicsObjectContainer* pBodyB   = pDef->m_pDefB->m_pBody;
	TPhysicsManager*         pManager = TPhysicsManager::GetSingleton();

	TVector4 vPoint1;
	TVector4 vPoint2;

	switch ( pDef->m_iType )
	{
		case 7:
		{
			vPoint1 = *pDef->m_pvPoint1;
			TMatrix44::TransformVector( vPoint1, *a_pMatrix, vPoint1 );

			m_pConstraint = pManager->CreateConstraint( pBodyA, pBodyB, TFALSE );

			if ( !m_pConstraint->m_bInWorld )
			{
				m_pConstraint->m_eType   = TPhysicsConstraintContainer::TYPE_BALLANDSOCKET;
				m_pConstraint->m_vPivotA = vPoint1;
			}

			break;
		}
		case 8:
		{
			vPoint1 = *pDef->m_pvPoint1;
			TMatrix44::TransformVector( vPoint1, *a_pMatrix, vPoint1 );
			vPoint2 = *pDef->m_pvPoint2;
			TMatrix44::TransformVector( vPoint2, *a_pMatrix, vPoint2 );

			m_pConstraint = pManager->CreateConstraint( pBodyA, pBodyB, TFALSE );

			if ( !m_pConstraint->m_bInWorld )
			{
				m_pConstraint->m_eType   = TPhysicsConstraintContainer::TYPE_HINGE;
				m_pConstraint->m_vPivotA = vPoint1;
				m_pConstraint->m_vPivotB = vPoint2;

				// The axis points from the pivot to the second point
				m_pConstraint->m_vPivotB.x = m_pConstraint->m_vPivotB.x - m_pConstraint->m_vPivotA.x;
				m_pConstraint->m_vPivotB.y = m_pConstraint->m_vPivotB.y - m_pConstraint->m_vPivotA.y;
				m_pConstraint->m_vPivotB.z = m_pConstraint->m_vPivotB.z - m_pConstraint->m_vPivotA.z;
				m_pConstraint->m_vPivotB.Normalise();
			}

			break;
		}
		case 9:
		{
			vPoint1 = *pDef->m_pvPoint1;
			TMatrix44::TransformVector( vPoint1, *a_pMatrix, vPoint1 );
			vPoint2 = *pDef->m_pvPoint2;
			TMatrix44::TransformVector( vPoint2, *a_pMatrix, vPoint2 );

			m_pConstraint = pManager->CreateConstraint( pBodyA, pBodyB, TFALSE );
			TPhysicsManager::SetConstraintLimitedHinge( m_pConstraint, vPoint1, vPoint2, pDef->m_fMinAngle, pDef->m_fMaxAngle, pDef->m_fFriction );
			break;
		}
		case 10:
		{
			vPoint1 = *pDef->m_pvPoint1;
			TMatrix44::TransformVector( vPoint1, *a_pMatrix, vPoint1 );
			vPoint2 = *pDef->m_pvPoint2;
			TMatrix44::TransformVector( vPoint2, *a_pMatrix, vPoint2 );

			m_pConstraint = pManager->CreateConstraint( pBodyA, pBodyB, TFALSE );
			TPhysicsManager::SetConstraintStiffSpring( m_pConstraint, vPoint1, vPoint2 );

			// The bodies of a spring share a sub system id
			TINT8 iId = pManager->FindFreeSubSystemId();
			TPhysicsManager::SetSubSystemId( pBodyA, iId );
			TPhysicsManager::SetSubSystemId( pBodyB, iId );
			break;
		}
		default:
			// NOTE: the other types do nothing, not even the modifier below
			return;
	}

	if ( pDef->m_iType != 10 )
	{
		// The bodies of the constraint don't collide with each other (the same sub system id)
		TINT8 iId = pManager->FindFreeSubSystemId();
		TPhysicsManager::SetSubSystemId( pBodyA, iId );
		TPhysicsManager::SetSubSystemId( pBodyB, iId );
	}

	ConstraintModifier* pModifier = pDef->m_pModifier;

	if ( pModifier != TNULL )
	{
		if ( pModifier->m_iType == 0xe )
		{
			TPhysicsManager::SetConstraintBreakable( m_pConstraint, pModifier->m_fValue );

			if ( pModifier->m_iUnk08 != 0 && pModifier->m_pDefA != TNULL )
				TPhysicsManager::SetConstraintBodyBreakInfo( m_pConstraint, pModifier->m_pDefA->m_pBody, pModifier->m_fA1, pModifier->m_fA3, pModifier->m_fA2, pModifier->m_fA4 != 0.0f );

			if ( pModifier->m_iUnk24 != 0 && pModifier->m_pDefB != TNULL )
				TPhysicsManager::SetConstraintBodyBreakInfo( m_pConstraint, pModifier->m_pDefB->m_pBody, pModifier->m_fB1, pModifier->m_fB3, pModifier->m_fB2, pModifier->m_fB4 != 0.0f );
		}
		else if ( pModifier->m_iType == 0xd )
		{
			TPhysicsManager::SetConstraintMalleable( m_pConstraint, pModifier->m_fValue, *TREINTERPRETCAST( TFLOAT*, &pModifier->m_iUnk08 ) );
		}
	}
}

// $deBlob: FUNCTION 005a4610
void ACollisionInstance2::AddToWorld( TUINT32 a_uiTypeMask )
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( TUINT8( m_uiFlags >> 4 ) & 1 )
		return;

	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		PhysicsInfo& rInfo = m_pPhysicsInfos[ i ];

		if ( rInfo.m_pBody && ( rInfo.m_pBody->m_uiCollisionFilterInfo & 0xfff ) && ( a_uiTypeMask & GetPhysicsObjectType( i ) ) )
		{
			if ( TUINT8( m_uiFlags >> 0 ) & 1 )
				pPhysicsManager->SetMotionType( rInfo.m_pBody, TPhysicsManager::MotionType_Static );
			else if ( rInfo.m_pBody && rInfo.m_pBody->m_eMotionType == TPhysicsManager::MotionType_Dynamic )
				pPhysicsManager->SetMotionType( rInfo.m_pBody, TPhysicsManager::MotionType_Static );

			pPhysicsManager->AddToSimulation( rInfo.m_pBody, TFALSE );

			TPhysicsObjectContainer* pBody = rInfo.m_pBody;

			if ( pBody && pBody->IsInSimulation() )
			{
				// NOTE: retail activates static bodies (hkpEntity::activate in Havok 5.5) and deactivates the others
				// (hkpEntity::deactivate)
				if ( pBody->m_eMotionType == TPhysicsManager::MotionType_Static )
					pBody->m_body->activate();
				else
					pBody->m_body->deactivate();
			}

			if ( rInfo.m_pInstanceMover )
				pPhysicsManager->AddCustomMover( rInfo.m_pInstanceMover );
		}
	}

	if ( m_pConstraint )
		pPhysicsManager->AddConstraint( m_pConstraint );
}

// $deBlob: FUNCTION 005a4720
void ACollisionInstance2::RemoveFromWorld( TUINT32 a_uiTypeMask )
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		PhysicsInfo& rInfo = m_pPhysicsInfos[ i ];

		if ( rInfo.m_pBody && ( a_uiTypeMask & GetPhysicsObjectType( i ) ) )
		{
			pPhysicsManager->RemoveFromSimulation( rInfo.m_pBody, TFALSE );

			if ( rInfo.m_pInstanceMover )
				pPhysicsManager->RemoveCustomMover( rInfo.m_pInstanceMover );
		}
	}

	if ( m_pConstraint )
		pPhysicsManager->RemoveConstraint( m_pConstraint );
}

// Activates the static bodies in the simulation, deactivates the others
// $deBlob: FUNCTION 005a4870
void ACollisionInstance2::UpdateBodiesActivation( TBOOL a_bUnused )
{
	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		TPhysicsObjectContainer* pBody = m_pPhysicsInfos[ i ].m_pBody;

		if ( pBody != TNULL && pBody->IsInSimulation() )
		{
			if ( pBody->m_eMotionType == TPhysicsManager::MotionType_Static )
				pBody->m_body->activate();
			else
				pBody->m_body->deactivate();
		}
	}
}

// $deBlob: FUNCTION 005a4210
void ACollisionInstance2::CreatePhysicsObjects( TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, PhysicsRecord* a_pRecord )
{
	PhysicsDef*       pDef    = *a_pRecord->m_ppDef;
	PhysicsShapeList* pShapes = pDef->m_pShapes;

	if ( pShapes != TNULL && pShapes->m_iNumShapes > 0 )
	{
		// One custom mesh shape is a mesh body, anything else the standard body
		if ( pShapes->m_iNumShapes == 1 && pShapes->m_ppShapes[ 0 ]->m_iType == 6 )
			CreateCustomMeshBodies( a_pModelInstance, a_pOwnerInfo, pDef );
		else
			CreateStandardBody( a_pModelInstance, a_pOwnerInfo, pDef );
	}
}

// $deBlob: FUNCTION 005a49f0
void ACollisionInstance2::CreateCustomMeshBodies( TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, PhysicsDef* a_pDef )
{
	PhysicsShapeList* pShapes   = a_pDef->m_pShapes;
	PhysicsShape*     pMeshData = TNULL;

	if ( pShapes->m_iNumShapes == 1 && pShapes->m_ppShapes[ 0 ]->m_iType == 6 )
		pMeshData = pShapes->m_ppShapes[ 0 ];

	TASSERT( pMeshData != TNULL );

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	TINT             iOldWorld       = pPhysicsManager->GetDefaultWorld();

	TModel* pModel     = a_pModelInstance->GetModel().GetData();
	TINT    iNumModels = TMath::Min( pModel->GetCollisionDataCount(), MAX_PHYSICS_OBJECTS );
	AllocatePhysicsInfos( iNumModels );

	TINT iPhysicsObject = 0;

	for ( TINT iModel = 0; iModel < iNumModels; iModel++ )
	{
		TModelCollisionMesh& rCollisionData = pModel->GetCollisionData( iModel );
		TBOOL                bDecal         = ACollisionCommon::GetSurfaceType( rCollisionData.m_pMaterials[ 0 ].m_szName ) == ACollisionCommon::SURFACE_Decal;

		if ( !m_pOwner || ( TUINT8( GetOwnerFlags() >> 17 ) & 1 ) )
		{
			if ( bDecal && !( pShapes->m_uiFlags & 8 ) )
			{
				m_iNumPhysicsObjects--;
				continue;
			}
		}
		else if ( bDecal )
		{
			m_iNumPhysicsObjects--;
			continue;
		}

		// The decals go to the second world
		pPhysicsManager->SetDefaultWorld( bDecal ? APhysicsWorldType_Decal : APhysicsWorldType_GamePlay );

		PhysicsInfo& rInfo = m_pPhysicsInfos[ iPhysicsObject ];
		rInfo.m_pBody      = CreateDefBody( a_pDef, &rInfo.m_mWorld, rCollisionData.m_iBone != -1 );
		rInfo.m_oOwnerInfo = *a_pOwnerInfo;

		TPhysicsObjectContainer* pBody = rInfo.m_pBody;

		if ( pBody )
			pBody->m_pUserData = &rInfo.m_oOwnerInfo;

		if ( pBody && pBody->m_eMotionType == TPhysicsManager::MotionType_Dynamic )
			m_uiFlags |= Flags_FirstDynamic;
		else if ( pBody && ( pBody->m_eMotionType == TPhysicsManager::MotionType_Keyframed || pBody->m_eMotionType == TPhysicsManager::MotionType_KeyframedReporting ) )
			m_uiFlags |= Flags_HasDynamic;
		else
			m_uiFlags |= Flags_HasStatic;

		TASSERT( rCollisionData.GetBoundingRadius() > 0.0f );

		TPhysicsMeshContainer* pContainer = TPhysicsMeshContainer::Create();
		pContainer->SetCollisionData( &rCollisionData );
		rInfo.m_pMeshContainer = pContainer;
		TPhysicsManager::SetMesh( rInfo.m_pBody, pContainer );

		if ( rInfo.m_pBody )
			rInfo.m_pBody->m_fFriction = pShapes->m_fFriction;

		if ( rInfo.m_pBody )
			rInfo.m_pBody->m_fRestitution = pShapes->m_fRestitution;

		TUINT32 uiLayer;
		TUINT32 uiGroup;

		if ( pMeshData->m_iParam2 == 17 )
		{
			if ( T2String8::Compare( rCollisionData.m_pMaterials[ 0 ].m_szName, "render" ) == 0 )
			{
				uiLayer = 8;
				uiGroup = 0;
			}
			else
			{
				uiLayer = pShapes->m_uiFlags & ~8;
				uiGroup = pShapes->m_uiGroup;
			}
		}
		else
		{
			uiLayer = pShapes->m_uiFlags;
			uiGroup = pShapes->m_uiGroup;
		}

		if ( !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
		{
			uiLayer &= ~0x40;

			if ( ( uiLayer & 0x1a1 ) == 0 )
				uiLayer |= 0x80;
		}

		pPhysicsManager->SetCollisionLayer( rInfo.m_pBody, uiLayer );
		pPhysicsManager->SetCollisionGroup( rInfo.m_pBody, uiGroup );

		if ( rInfo.m_pBody )
			rInfo.m_pBody->m_uiSurface = pShapes->m_uiUnk14;

		if ( rCollisionData.m_iBone != -1 )
		{
			rInfo.m_pInstanceMover = new ACollisionInstanceMover( rInfo.m_pBody, this, TINT16( iModel ) );
			pPhysicsManager->AddCustomMover( rInfo.m_pInstanceMover );
		}

		// The body is activated when it's in the simulation (retail calls the Havok entity's activate, or deactivate
		// for a body that isn't static)
		if ( rInfo.m_pBody && rInfo.m_pBody->IsInSimulation() )
		{
			if ( rInfo.m_pBody->m_eMotionType == TPhysicsManager::MotionType_Static )
				rInfo.m_pBody->m_body->activate();
			else
				rInfo.m_pBody->m_body->deactivate();
		}

		iPhysicsObject++;
	}

	a_pDef->m_pBody = m_pPhysicsInfos[ 0 ].m_pBody;

	TASSERT( iOldWorld == APhysicsWorldType_GamePlay );
	pPhysicsManager->SetDefaultWorld( iOldWorld );
}

// $deBlob: FUNCTION 005a4f20
void ACollisionInstance2::CreateStandardBody( TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, PhysicsDef* a_pDef )
{
	AllocatePhysicsInfos( 1 );

	PhysicsInfo*      pInfo   = m_pPhysicsInfos;
	PhysicsShapeList* pShapes = a_pDef->m_pShapes;

	TPhysicsManager* pManager = TPhysicsManager::GetSingleton();

	pInfo->m_pBody                 = CreateDefBody( a_pDef, &pInfo->m_mWorld, TFALSE );
	TPhysicsObjectContainer* pBody = pInfo->m_pBody;

	pInfo->m_oOwnerInfo = *a_pOwnerInfo;

	if ( pBody )
		pBody->m_pUserData = &pInfo->m_oOwnerInfo;

	a_pDef->m_pBody = pBody;

	// The kind of body decides the flags of the instance
	if ( pBody && pBody->m_eMotionType == TPhysicsManager::MotionType_Dynamic )
	{
		m_uiFlags |= Flags_FirstDynamic;
	}
	else if ( pBody && ( pBody->m_eMotionType == TPhysicsManager::MotionType_Keyframed || pBody->m_eMotionType == TPhysicsManager::MotionType_KeyframedReporting ) )
	{
		m_uiFlags |= Flags_HasDynamic;
	}
	else
	{
		m_uiFlags |= Flags_HasStatic;
	}

	AddShapes( pBody, a_pDef );

	pInfo->m_mLocal = *a_pDef->m_pMatrix;

	if ( pBody )
	{
		pBody->m_fFriction    = pShapes->m_fFriction;
		pBody->m_fRestitution = pShapes->m_fRestitution;
	}

	if ( pBody )
		pBody->m_uiSurface = pShapes->m_uiUnk14;

	// The layer and the group of the body: the dynamic bodies and the ones with an owner (or without the
	// owner's flag 0x20000) keep their layer, the others get the 0x400 bit and the static bits
	TUINT32 uiLayer = pShapes->m_uiFlags | 0x400;

	if ( ( m_uiFlags & Flags_FirstDynamic ) || ( TUINT8( m_uiFlags >> 2 ) & 1 ) || ( m_pOwner && !( TUINT8( GetOwnerFlags() >> 17 ) & 1 ) ) )
		uiLayer = ( pShapes->m_uiFlags & ~0x8 ) | 0x400;

	TUINT32 uiLayerOut = uiLayer;

	if ( !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
	{
		uiLayerOut = uiLayer & ~0x40;

		if ( ( uiLayer & 0x1a1 ) == 0 )
			uiLayerOut |= 0x80;
	}

	pManager->SetCollisionLayer( pBody, uiLayerOut );
	pManager->SetCollisionGroup( pBody, pShapes->m_uiGroup );

	PhysicsMotion* pMotion = a_pDef->m_pMotion;

	// The body is dynamic when its shape is a dynamic body (retail's type 0): it gets a mover of its own
	TBOOL bDynamic  = a_pDef->m_pBodyShape != TNULL && a_pDef->m_pBodyShape->m_iType == 0;
	TBOOL bKeyframe = pBody && ( pBody->m_eMotionType == TPhysicsManager::MotionType_Keyframed || pBody->m_eMotionType == TPhysicsManager::MotionType_KeyframedReporting );
	TBOOL bUnk20    = pMotion && pMotion->m_iUnk18 != 0;

	if ( bDynamic )
	{
		ACollisionInstanceMover* pMover = new ACollisionInstanceMover( pBody, this, -1 );
		pInfo->m_pInstanceMover         = pMover;
		pMover->SetMotionState( ACollisionInstanceMover::Flags_MotionStateStatic );
		pMover->SetInverseMatrix( pInfo->m_mLocal );

		if ( pMotion && pMotion->m_iSinkAndHide != 0 )
			pMover->SetSinkAndHide( TTRUE, pMotion->m_fHideDelay );

		if ( pMotion && pMotion->m_fFallDelay > 0.0f )
			pMover->m_fFallDelay = pMotion->m_fFallDelay;

		if ( pMotion && pMotion->m_iSinkAndRemove != 0 )
			pMover->SetSinkAndRemove( TTRUE, pMotion->m_fRemoveDelay );

		pManager->AddCustomMover( pMover );

		// The responder of the body: the mover reacts when it is hit
		ACollisionInstanceResponderMover* pResponder = new ACollisionInstanceResponderMover( this );
		pResponder->SetMover( pInfo->m_pInstanceMover );
		m_pResponder = pResponder;
		pManager->SetListener( pInfo->m_pBody, pResponder );
		m_uiFlags |= Flags_StaticBodies;
		pResponder->SetUnk08( pMotion->m_iUnk00 );
	}
	else if ( bKeyframe && ( !m_pOwner || !( TUINT8( GetOwnerFlags() >> 5 ) & 1 ) ) )
	{
		ACollisionInstanceMover* pMover = new ACollisionInstanceMover( pBody, this, -1 );
		pInfo->m_pInstanceMover         = pMover;
		pMover->SetMotionState( ACollisionInstanceMover::Flags_MotionStateKeyframe );
		pMover->AddFlags( ACollisionInstanceMover::Flags_NoBone );
		pMover->SetInverseMatrix( pInfo->m_mLocal );
		pManager->AddCustomMover( pMover );
	}

	if ( pInfo->m_pInstanceMover && bUnk20 )
		pInfo->m_pInstanceMover->AddFlags( 0x20 );

	// The body is activated when it's in the simulation (retail calls the Havok entity's activate, or deactivate
	// for a body that isn't static)
	if ( pBody && pBody->IsInSimulation() )
	{
		if ( pBody->m_eMotionType == TPhysicsManager::MotionType_Static )
			pBody->m_body->activate();
		else
			pBody->m_body->deactivate();
	}
}

// Creates the static, keyframed or dynamic body of a physics definition's body shape (nullptr for the other types)
// $deBlob: FUNCTION 005a5370
TPhysicsObjectContainer* ACollisionInstance2::CreateDefBody( PhysicsDef* a_pDef, TMatrix44* a_pMatrix, TBOOL a_bKeyframed )
{
	TPhysicsManager* pManager = TPhysicsManager::GetSingleton();

	if ( a_bKeyframed )
	{
		TPhysicsObjectContainer* pBody = pManager->CreateBody( a_pMatrix, TNULL, TPhysicsManager::MotionType_Keyframed, TPhysicsManager::SubType_Default );
		pBody->m_fMass                 = 50000.0f;
		return pBody;
	}

	PhysicsShape* pShape = a_pDef->m_pBodyShape;
	TINT          iType  = TPhysicsManager::MotionType_Static;

	if ( pShape != TNULL )
	{
		if ( pShape->m_iType != 1 )
		{
			if ( pShape->m_iType == 0 )
			{
				TPhysicsObjectContainer* pBody = pManager->CreateBody( a_pMatrix, TNULL, TPhysicsManager::MotionType_Dynamic, pShape->m_iFlag == 0 );

				// NOTE: retail keeps the greater of the shape's value and 50000
				pBody->m_fMass = ( pShape->m_fParam1 > 50000.0f || pShape->m_fParam1 == 50000.0f ) ? pShape->m_fParam1 : 50000.0f;

				// The damping is copied as it is: the Havok body takes it from the container when it's added
				pBody->m_fLinearDamping  = pShape->m_fParam2;
				pBody->m_fAngularDamping = pShape->m_fParam3;

				SetMaxLinearVelocity( pBody, TFLOAT( pShape->m_iSpeed ) / 3.6f, TTRUE );
				SetMaxAngularVelocity( pBody, TFLOAT( pShape->m_iAngle ) * 0.0017453294f );
				return pBody;
			}

			return TNULL;
		}

		iType = TPhysicsManager::MotionType_Keyframed;
	}

	TPhysicsObjectContainer* pBody = pManager->CreateBody( a_pMatrix, TNULL, iType, iType );
	pBody->m_fMass                 = 50000.0f;
	return pBody;
}

// A mesh container of the collision mesh
// $deBlob: FUNCTION 005a54a0
static TPhysicsMeshContainer* CreateMeshContainer( TModelCollisionMesh& a_rCollisionData )
{
	TASSERT( a_rCollisionData.m_fBoundingRadius > 0.0f );
	TPhysicsMeshContainer* pContainer = TPhysicsMeshContainer::Create();
	pContainer->SetCollisionData( &a_rCollisionData );
	return pContainer;
}

// $deBlob: FUNCTION 005a5500
void ACollisionInstance2::AddShapes( TPhysicsObjectContainer* a_pBody, PhysicsDef* a_pDef )
{
	PhysicsShapeList* pShapes = a_pDef->m_pShapes;

	for ( TINT i = 0; i < pShapes->m_iNumShapes; i++ )
	{
		PhysicsShape* pShape = pShapes->m_ppShapes[ i ];

		const TMatrix44& rMatrix = pShape->m_pMatrix ? *pShape->m_pMatrix : TMatrix44::IDENTITY;

		switch ( pShape->m_iType )
		{
			case 2:
				TPhysicsManager::AddListSphere( a_pBody, rMatrix, pShape->m_fParam1 );
				break;

			case 3:
			case 4:
			{
				// The capsule and the cylinder take the rows of their matrix in the order 3, 1, 2, 4
				TMatrix44 mShape( rMatrix.m_f31, rMatrix.m_f32, rMatrix.m_f33, rMatrix.m_f34, rMatrix.m_f11, rMatrix.m_f12, rMatrix.m_f13, rMatrix.m_f14, rMatrix.m_f21, rMatrix.m_f22, rMatrix.m_f23, rMatrix.m_f24, rMatrix.m_f41, rMatrix.m_f42, rMatrix.m_f43, rMatrix.m_f44 );

				if ( pShape->m_iType == 3 )
					TPhysicsManager::AddListCapsule( a_pBody, mShape, pShape->m_fParam2, pShape->m_fParam1 );
				else
					TPhysicsManager::AddListCylinder( a_pBody, mShape, pShape->m_fParam2, pShape->m_fParam1 );
				break;
			}

			case 5:
				TPhysicsManager::AddListBox( a_pBody, rMatrix, TVector4( pShape->m_fParam1, pShape->m_fParam2, pShape->m_fParam3, 1.0f ) );
				break;
		}
	}
}

// $deBlob: FUNCTION 005a67a0
void ACollisionInstance2::SetMaxLinearVelocity( TPhysicsObjectContainer* a_pBody, TFLOAT a_fVelocity, TBOOL a_bUnk )
{
	TASSERT( a_pBody != TNULL );

	a_pBody->m_fMaxLinearVelocity = a_fVelocity;

	if ( !a_bUnk )
		a_pBody->m_flags |= 0x10;
	else
		a_pBody->m_flags &= ~0x10;

	// NOTE: retail writes the hkUFloat8 velocity bucket of Havok 5.5's hkpMotion (hkpRigidBody + 0x18c)
	if ( a_pBody->m_body != TNULL )
		a_pBody->m_body->setMaxLinearVelocity( a_fVelocity );
}

// $deBlob: FUNCTION 005a6730
void ACollisionInstance2::SetMaxAngularVelocity( TPhysicsObjectContainer* a_pBody, TFLOAT a_fVelocity )
{
	TASSERT( a_pBody != TNULL );

	a_pBody->m_fMaxAngularVelocity = a_fVelocity;

	// NOTE: as SetMaxLinearVelocity, retail writes the Havok 5.5 bucket (hkpRigidBody + 0x18d)
	if ( a_pBody->m_body != TNULL )
		a_pBody->m_body->setMaxAngularVelocity( a_fVelocity );
}

// $deBlob: FUNCTION 005a48d0
void ACollisionInstance2::CreateKeyframeMovers()
{
	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		PhysicsInfo& rInfo = m_pPhysicsInfos[ i ];

		if ( rInfo.m_pBody == TNULL )
			continue;

		TASSERT( rInfo.m_pInstanceMover == TNULL );

		rInfo.m_pInstanceMover = new ACollisionInstanceMover( rInfo.m_pBody, this, TINT16( i ) );
		rInfo.m_pInstanceMover->AddFlags( ACollisionInstanceMover::Flags_NoBone );
		rInfo.m_pInstanceMover->SetMotionState( ACollisionInstanceMover::Flags_MotionStateKeyframe );

		TPhysicsManager::GetSingleton()->AddCustomMover( rInfo.m_pInstanceMover );
	}
}

// $deBlob: FUNCTION 005a56e0
void ACollisionInstance2::AllocatePhysicsInfos( TINT a_iNumPhysicsObjects )
{
	m_iNumPhysicsObjects = a_iNumPhysicsObjects;
	m_pPhysicsInfos      = TSTATICCAST( PhysicsInfo, TMemalign( 16, a_iNumPhysicsObjects * sizeof( PhysicsInfo ) ) );

	if ( m_pPhysicsInfos )
	{
		for ( TINT i = a_iNumPhysicsObjects - 1; i >= 0; i-- )
		{
			m_pPhysicsInfos[ i ].m_oOwnerInfo.m_eType  = 0;
			m_pPhysicsInfos[ i ].m_oOwnerInfo.m_pOwner = TNULL;
		}
	}

	for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
	{
		PhysicsInfo& rInfo = m_pPhysicsInfos[ i ];
		rInfo.m_mWorld.Identity();
		rInfo.m_mLocal.Identity();
		rInfo.m_pBody          = TNULL;
		rInfo.m_pMeshContainer = TNULL;
		rInfo.m_pInstanceMover = TNULL;
	}
}

// $deBlob: FUNCTION 005a5790
void ACollisionInstance2::GetCollisionFilter( const TCHAR* a_szMaterial, TINT a_eOwnerType, TUINT32& a_ruiLayer, TUINT32& a_ruiGroup, TUINT32& a_ruiSurface )
{
	TUINT32 uiSurface = ACollisionCommon::GetSurfaceType( a_szMaterial );
	a_ruiSurface      = uiSurface;

	if ( uiSurface & ACollisionCommon::SURFACE_Decal )
	{
		a_ruiLayer = 8;
		a_ruiGroup = 0;
		return;
	}

	if ( ( uiSurface & ACollisionCommon::SURFACE_Water ) || ( uiSurface & ACollisionCommon::SURFACE_Ink ) )
	{
		a_ruiLayer = 0x20;
		a_ruiGroup = 0;
		return;
	}

	if ( uiSurface & ( ACollisionCommon::SURFACE_ForceField | ACollisionCommon::SURFACE_ForceFieldEdge ) )
	{
		a_ruiLayer = 0x100;
		a_ruiGroup = 0;
		return;
	}

	if ( a_eOwnerType == 3 )
	{
		a_ruiLayer = 0x80;
		a_ruiLayer |= 0x10;
		a_ruiGroup = 0x46;
		return;
	}

	a_ruiLayer = ( a_eOwnerType == 4 ) ? 1 : 0x81;
	a_ruiLayer |= 0x10;
	a_ruiGroup = 0x46;
}

// The number of materials of all the collision meshes of the model
// $deBlob: FUNCTION 005a5850
static TINT GetNumCollisionMaterials( TModelInstance* a_pModelInstance )
{
	TINT iNumMaterials = 0;

	for ( TINT i = 0; i < a_pModelInstance->GetModel()->GetCollisionDataCount(); i++ )
		iNumMaterials += a_pModelInstance->GetModel()->GetCollisionData( i ).m_iNumMaterials;

	return iNumMaterials;
}

// Looks for the first collision mesh whose first material isn't "render" (the result isn't used)
// $deBlob: FUNCTION 005a59e0
static void FindNonRenderCollisionMesh( TModelInstance* a_pModelInstance )
{
	TINT iCount = a_pModelInstance->GetModel()->GetCollisionDataCount();

	if ( iCount > 1 )
	{
		for ( TINT i = 0; i < iCount; i++ )
		{
			if ( T2String8::Compare( a_pModelInstance->GetModel()->GetCollisionData( i ).m_pMaterials[ 0 ].m_szName, "render" ) != 0 )
				break;
		}
	}
}

// $deBlob: FUNCTION 005a5b20
void ACollisionInstance2::GetPrimitiveShape( const TModelCollisionMesh* a_pCollisionData, TINT a_iShapeType, TFLOAT* a_pParams, TMatrix44& a_rTransform )
{
	const TFLOAT* pExtents = &a_pCollisionData->m_vAABBExtents.x;

	TINT iLongest  = -1;
	TINT iShortest = -1;
	TINT iMiddle   = -1;

	if ( a_iShapeType == 4 || a_iShapeType == 5 )
	{
		// The longest axis is the capsule's or cylinder's axis, the middle one gives its radius
		TFLOAT fMax = -TMath::MAXFLOAT;
		TFLOAT fMin = TMath::MAXFLOAT;

		if ( fMax < pExtents[ 0 ] )
		{
			iLongest = 0;
			fMax     = pExtents[ 0 ];
		}

		if ( pExtents[ 0 ] < fMin )
		{
			iShortest = 0;
			fMin      = pExtents[ 0 ];
		}

		if ( fMax < pExtents[ 1 ] )
		{
			iLongest = 1;
			fMax     = pExtents[ 1 ];
		}

		if ( pExtents[ 1 ] < fMin )
		{
			iShortest = 1;
			fMin      = pExtents[ 1 ];
		}

		if ( fMax < pExtents[ 2 ] )
			iLongest = 2;

		if ( pExtents[ 2 ] < fMin )
			iShortest = 2;

		if ( iLongest == 0 )
			iMiddle = ( iShortest != 1 ) ? 1 : 2;
		else if ( iLongest == 1 )
			iMiddle = ( iShortest == 0 ) ? 2 : 0;
		else
			iMiddle = ( iShortest == 0 ) ? 1 : 0;
	}

	if ( a_iShapeType == 4 || a_iShapeType == 5 )
	{
		a_pParams[ 1 ] = pExtents[ iMiddle ];
		a_pParams[ 0 ] = pExtents[ iLongest ] * 2.0f;

		if ( a_iShapeType == 4 )
		{
			// The capsule's height is between the centres of its caps
			a_pParams[ 0 ] = a_pParams[ 0 ] - a_pParams[ 1 ] * 2.0f;
			a_pParams[ 0 ] = TMath::Max( a_pParams[ 0 ], 0.0f );
		}
	}
	else
	{
		a_pParams[ 0 ] = pExtents[ 0 ];
		a_pParams[ 1 ] = pExtents[ 1 ];
		a_pParams[ 2 ] = pExtents[ 2 ];
		a_pParams[ 3 ] = pExtents[ 3 ];
	}

	a_rTransform.Identity();
	TASSERT( a_pCollisionData->m_vAABBCentre.w == 1.0f );
	a_rTransform.SetTranslation( a_pCollisionData->m_vAABBCentre );

	if ( a_iShapeType == 4 || a_iShapeType == 5 )
	{
		// Turn the shape's axis (Z) to the longest axis
		switch ( iLongest )
		{
			case 0:
			{
				TVector4 vRow1                                     = a_rTransform.AsBasisVector4( BASISVECTOR_RIGHT );
				a_rTransform.AsBasisVector4( BASISVECTOR_RIGHT )   = a_rTransform.AsBasisVector4( BASISVECTOR_UP );
				a_rTransform.AsBasisVector4( BASISVECTOR_UP )      = a_rTransform.AsBasisVector4( BASISVECTOR_FORWARD );
				a_rTransform.AsBasisVector4( BASISVECTOR_FORWARD ) = vRow1;
			}
			break;
			case 1:
			{
				TVector4 vRow1                                     = a_rTransform.AsBasisVector4( BASISVECTOR_RIGHT );
				TVector4 vRow2                                     = a_rTransform.AsBasisVector4( BASISVECTOR_UP );
				a_rTransform.AsBasisVector4( BASISVECTOR_RIGHT )   = a_rTransform.AsBasisVector4( BASISVECTOR_FORWARD );
				a_rTransform.AsBasisVector4( BASISVECTOR_UP )      = vRow1;
				a_rTransform.AsBasisVector4( BASISVECTOR_FORWARD ) = vRow2;
			}
			break;
		}
	}
}
