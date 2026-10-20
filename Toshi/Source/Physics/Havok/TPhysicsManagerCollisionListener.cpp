#include "ToshiPCH.h"
#include "TPhysicsManagerCollisionListener.h"
#include "TPhysicsManager.h"

#include <Physics/Dynamics/Entity/hkpEntity.h>
#include <Physics/Dynamics/Collide/hkpCollisionListener.h>
#include <Physics/Collide/Agent/hkpProcessCollisionData.h>
#include <Physics/ConstraintSolver/Constraint/Contact/hkpContactPointProperties.h>
#include <Physics/Collide/Agent/Collidable/hkpCollidable.h>
#include "TPhysicsMeshContainer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00735b80
TPhysicsManagerCollisionListener::TPhysicsManagerCollisionListener( hkpEntity* a_pEntity, TPhysicsManager* a_pManager )
{
	a_pEntity->addCollisionListener( this );
	a_pEntity->addEntityListener( this );
	m_pManager = a_pManager;
}

/// The body of a collidable (the user data of its shape)
static TPhysicsObjectContainer* GetBody( const hkpCdBody* a_pCdBody )
{
	return TREINTERPRETCAST( TPhysicsObjectContainer*, a_pCdBody->getRootCollidable()->getShape()->getUserData() );
}

// $deBlob: FUNCTION 00735bd0
void TPhysicsManagerCollisionListener::contactPointAddedCallback( hkpContactPointAddedEvent& a_rEvent )
{
	TUINT32                  uiMaterialA = m_pManager->GetCollisionMaterial( a_rEvent.m_bodyA );
	TUINT32                  uiMaterialB = m_pManager->GetCollisionMaterial( a_rEvent.m_bodyB );
	TPhysicsObjectContainer* pBodyA      = GetBody( a_rEvent.m_bodyA );
	TPhysicsObjectContainer* pBodyB      = GetBody( a_rEvent.m_bodyB );

	hkpContactPointProperties* pProperties = a_rEvent.m_contactPointProperties;
	TPhysicsContactData*       pData       = m_pManager->AllocContactData();

	if ( pData )
	{
		pData->m_uiMaterialA = uiMaterialA;
		pData->m_uiMaterialB = uiMaterialB;
		pData->m_pBodyA      = pBodyA;
		pData->m_pBodyB      = pBodyB;
		pData->m_bToi        = a_rEvent.m_type == hkpContactPointAddedEvent::TYPE_TOI;
		pProperties->setUserData( TREINTERPRETCAST( hkUlong, pData ) );
	}

	// Combine the surface parameters of the mesh materials with the other body's
	TFLOAT fFrictionA    = pBodyA->m_fFriction;
	TFLOAT fRestitutionA = pBodyA->m_fRestitution;
	TFLOAT fFrictionB    = pBodyB->m_fFriction;
	TFLOAT fRestitutionB = pBodyB->m_fRestitution;

	const TPhysicsMeshMaterial* pMaterialA = m_pManager->GetMeshMaterial( a_rEvent.m_bodyA );

	if ( pMaterialA )
	{
		fFrictionA    = pProperties->getFriction();
		fRestitutionA = pProperties->getRestitution();
		pProperties->setFriction( TFLOAT( sqrt( pMaterialA->m_fFriction * fFrictionB ) ) );
		pProperties->setRestitution( TFLOAT( sqrt( pMaterialA->m_fRestitution * fRestitutionB ) ) );
	}

	const TPhysicsMeshMaterial* pMaterialB = m_pManager->GetMeshMaterial( a_rEvent.m_bodyB );

	if ( pMaterialB )
	{
		pProperties->setFriction( TFLOAT( sqrt( pMaterialB->m_fFriction * fFrictionA ) ) );
		pProperties->setRestitution( TFLOAT( sqrt( pMaterialB->m_fRestitution * fRestitutionA ) ) );
	}

	const hkContactPoint* pPoint = a_rEvent.m_contactPoint;
	TVector4              vPosition( pPoint->getPosition()( 0 ), pPoint->getPosition()( 1 ), pPoint->getPosition()( 2 ), 1.0f );
	TVector4              vNormal( pPoint->getNormal()( 0 ), pPoint->getNormal()( 1 ), pPoint->getNormal()( 2 ), 1.0f );

	m_pManager->RecordCollisionCallback( pBodyA, pBodyB, uiMaterialA, uiMaterialB, vPosition, vNormal, TPhysicsCollisionCallback::Type_Added, 0 );
}

// $deBlob: FUNCTION 00735e50
void TPhysicsManagerCollisionListener::contactPointConfirmedCallback( hkpContactPointConfirmedEvent& a_rEvent )
{
	TPhysicsObjectContainer* pBodyA = TREINTERPRETCAST( TPhysicsObjectContainer*, a_rEvent.m_collidableA->getShape()->getUserData() );
	TPhysicsObjectContainer* pBodyB = TREINTERPRETCAST( TPhysicsObjectContainer*, a_rEvent.m_collidableB->getShape()->getUserData() );

	if ( pBodyA && pBodyB )
	{
		const hkContactPoint* pPoint = a_rEvent.m_contactPoint;
		TVector4              vPosition( pPoint->getPosition()( 0 ), pPoint->getPosition()( 1 ), pPoint->getPosition()( 2 ), 1.0f );
		TVector4              vNormal( pPoint->getNormal()( 0 ), pPoint->getNormal()( 1 ), pPoint->getNormal()( 2 ), 1.0f );

		TUINT32              uiMaterialA;
		TUINT32              uiMaterialB;
		TPhysicsContactData* pData = TREINTERPRETCAST( TPhysicsContactData*, a_rEvent.m_contactPointProperties->getUserData() );

		if ( pData == TNULL )
		{
			uiMaterialA = TPhysicsManager::GetDefaultMaterial( pBodyA );
			uiMaterialB = TPhysicsManager::GetDefaultMaterial( pBodyB );
		}
		else if ( pData->m_pBodyA == pBodyA )
		{
			uiMaterialA = pData->m_uiMaterialA;
			uiMaterialB = pData->m_uiMaterialB;
		}
		else
		{
			uiMaterialA = pData->m_uiMaterialB;
			uiMaterialB = pData->m_uiMaterialA;
		}

		m_pManager->RecordCollisionCallback( pBodyA, pBodyB, uiMaterialA, uiMaterialB, vPosition, vNormal, TPhysicsCollisionCallback::Type_Confirmed, a_rEvent.getContactPointId() );
	}
}

// $deBlob: FUNCTION 00735f40
void TPhysicsManagerCollisionListener::contactPointRemovedCallback( hkpContactPointRemovedEvent& a_rEvent )
{
	TPhysicsContactData* pData = TREINTERPRETCAST( TPhysicsContactData*, a_rEvent.m_contactPointProperties->getUserData() );

	if ( pData )
		m_pManager->FreeContactData( pData );
}

// $deBlob: FUNCTION 00735f60
void TPhysicsManagerCollisionListener::contactProcessCallback( hkpContactProcessEvent& a_rEvent )
{
	TPhysicsObjectContainer* pBodyA = TREINTERPRETCAST( TPhysicsObjectContainer*, a_rEvent.m_collidableA->getShape()->getUserData() );
	TPhysicsObjectContainer* pBodyB = TREINTERPRETCAST( TPhysicsObjectContainer*, a_rEvent.m_collidableB->getShape()->getUserData() );

	if ( pBodyA && pBodyB )
	{
		hkpProcessCollisionData* pData = a_rEvent.m_collisionData;

		for ( TINT i = 0; i < pData->getNumContactPoints(); i++ )
		{
			const hkpProcessCdPoint& rPoint = pData->m_contactPoints[ i ];
			TVector4                 vPosition( rPoint.m_contact.getPosition()( 0 ), rPoint.m_contact.getPosition()( 1 ), rPoint.m_contact.getPosition()( 2 ), 1.0f );
			TVector4                 vNormal( rPoint.m_contact.getNormal()( 0 ), rPoint.m_contact.getNormal()( 1 ), rPoint.m_contact.getNormal()( 2 ), 1.0f );

			// NOTE: unlike contactPointConfirmedCallback, the stored materials aren't swapped when the bodies are
			TUINT32              uiMaterialA;
			TUINT32              uiMaterialB;
			TPhysicsContactData* pContactData = TREINTERPRETCAST( TPhysicsContactData*, a_rEvent.m_contactPointProperties[ i ]->getUserData() );

			if ( pContactData == TNULL )
			{
				uiMaterialA = pBodyA->m_uiSurface;
				uiMaterialB = pBodyB->m_uiSurface;
			}
			else
			{
				uiMaterialA = pContactData->m_uiMaterialA;
				uiMaterialB = pContactData->m_uiMaterialB;
			}

			m_pManager->RecordCollisionCallback( pBodyA, pBodyB, uiMaterialA, uiMaterialB, vPosition, vNormal, TPhysicsCollisionCallback::Type_Process, TUINT16( rPoint.m_contactPointId ) );
		}
	}
}

// $deBlob: FUNCTION 007360a0
void TPhysicsManagerCollisionListener::entityDeletedCallback( hkpEntity* a_pEntity )
{
	a_pEntity->removeCollisionListener( this );
	delete this;
}

TOSHI_NAMESPACE_END
