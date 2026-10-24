#include "ToshiPCH.h"
#include "TPhysicsObjectContainer.h"
#include "TPhysicsManager.h"
#include "Physics/TShapeBaseClass.h"
#include "Physics/TCSGNode.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

TSTATICASSERT( sizeof( TPhysicsObjectContainer ) == 124 );

// $deBlob: FUNCTION 007351f0
TPhysicsObjectContainer::TPhysicsObjectContainer()
{
	m_pMatrix               = TNULL;
	m_pUserData             = TNULL;
	m_fRadius               = 0.0f;
	m_fMass                 = 1.0f;
	m_fMaxLinearVelocity    = 200.0f;
	m_fMaxAngularVelocity   = 200.0f;
	m_flags                 = 0;
	m_eShapeType            = TShapeBaseClass::Type_None;
	m_iWorld                = 0;
	m_uiCollisionFilterInfo = 0;
	m_eMotionType           = 0;
	m_eCreationMotionType   = 0;
	m_uiSurface             = 0;
	m_geom                  = TNULL;
	m_body                  = TNULL;
	m_pSurfaceVelocityUtil  = TNULL;
	m_pShapeData            = TNULL;
	m_vCenterOfMass         = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_fRestitution          = 0.1f;
	m_fUnk44                = 0.001f;
	m_fFriction             = 0.5f;
	m_fLinearDamping        = 0.1f;
	m_fAngularDamping       = 0.2f;
	m_pListener             = TNULL;
	m_pCollisionListener    = TNULL;
	m_pMoveCallback         = TNULL;
}

// $deBlob: FUNCTION 007351a0
// $deBlob: FUNCTION 00735480
TPhysicsObjectContainer::~TPhysicsObjectContainer()
{
	ReleaseShapeData( m_eShapeType, m_pShapeData );
}

// $deBlob: FUNCTION 00735280
TPhysicsObjectContainer* TPhysicsObjectContainer::Create()
{
	return new ( TMalloc( sizeof( TPhysicsObjectContainer ), TPhysicsManager::GetHeap() ) ) TPhysicsObjectContainer();
}

// $deBlob: FUNCTION 007352c0
void TPhysicsObjectContainer::Destroy( TPhysicsObjectContainer* a_pContainer )
{
	if ( a_pContainer )
		delete a_pContainer;
}

// $deBlob: FUNCTION 00735320
void TPhysicsObjectContainer::ReleaseShapeData( TINT a_eShapeType, TShapeBaseClass* a_pShapeData )
{
	switch ( a_eShapeType )
	{
		case TShapeBaseClass::Type_Plane:
		case TShapeBaseClass::Type_Sphere:
		case TShapeBaseClass::Type_Box:
		case TShapeBaseClass::Type_Capsule:
		case TShapeBaseClass::Type_Cylinder:
			a_pShapeData->Release();
			break;
		case TShapeBaseClass::Type_List:
		{
			TShapeList* pList = reinterpret_cast<TShapeList*>( a_pShapeData );

			for ( TINT i = 0; i < pList->Size(); i++ )
			{
				TCSGNode* pNode = ( *pList )[ i ];
				ReleaseShapeData( pNode->GetShapeData()->GetType(), pNode->GetShapeData() );
				pNode->Release();
			}

			for ( TINT i = 0; i < pList->Size(); i++ )
				( *pList )[ i ] = TNULL;

			pList->Clear();
			TFree( pList );
			break;
		}
	}
}

// $deBlob: FUNCTION 007353e0
TShapeList* TPhysicsObjectContainer::GetShapeList()
{
	if ( !m_pShapeList )
		m_pShapeList = new ( TMalloc( sizeof( TShapeList ), TPhysicsManager::GetHeap() ) ) TShapeList();

	return m_pShapeList;
}

TOSHI_NAMESPACE_END
