#pragma once
#include "Toshi/T2DList.h"
#include "Toshi/T2Vector.h"
#include "Math/TVector4.h"
#include "Math/TMatrix44.h"

class hkpShape;
class hkpRigidBody;
class hkpSurfaceVelocityUtil;

TOSHI_NAMESPACE_START

class TShapeBaseClass;
class TCSGNode;
class TPhysicsMeshContainer;
class TPhysicsHFContainer;
class TPhysicsResponder;

// The nodes of a shape list (shape type 7, 0x24 bytes)
using TShapeList = T2Vector<TCSGNode*, 8>;

// A physics body: the shape description, the body parameters and the Havok body built from them by
// TPhysicsManager::AddBody (124 bytes in retail)
class TPhysicsObjectContainer
    : public T2DList<TPhysicsObjectContainer>::Node
{
public:
	enum Flags : TUINT16
	{
		PMF_POSITION_ONLY   = 0x0001, // only the translation is copied from the Havok body
		PMF_KEYFRAMED       = 0x0002,
		PMF_STATIC          = 0x0004,
		PMF_SPHERE_INERTIA  = 0x0008,
		PMF_MOVED           = 0x0020, // set when the body was active in a step
		PMF_UNK40           = 0x0040, // set by TPhysicsManager::SetHeightField
		PMF_FIXED           = 0x0080,
		PMF_IN_ADD_BATCH    = 0x0100,
		PMF_IN_REMOVE_BATCH = 0x0200,
		PMF_DELETE_PENDING  = 0x0400,
		PMF_UNK800          = 0x0800,
		PMF_IN_SIMULATION   = 0x1000,
		PMF_UNK2000         = 0x2000,
	};

public:
	TPhysicsObjectContainer();
	~TPhysicsObjectContainer();

	// Allocates/frees a container on the physics heap
	static TPhysicsObjectContainer* Create();
	static void                     Destroy( TPhysicsObjectContainer* a_pContainer );

	// Releases the shape data of type a_eShapeType
	void ReleaseShapeData( TINT a_eShapeType, TShapeBaseClass* a_pShapeData );

	// Returns the shape list (shape type 7), creating it if needed
	TShapeList* GetShapeList();

	TBOOL IsInSimulation() const { return m_flags & PMF_IN_SIMULATION; }

	// TNULL is not in the simulation
	// $deBlob: FUNCTION 00580590
	static TBOOL IsInSimulation( const TPhysicsObjectContainer* a_pBody ) { return a_pBody != TNULL && ( a_pBody->m_flags & PMF_IN_SIMULATION ) != 0; }

public:
	TMatrix44*         m_pMatrix;               // 0x08
	void*              m_pUserData;             // 0x0c
	TVector4           m_vCenterOfMass;         // 0x10
	TUINT16            m_flags;                 // 0x20
	TINT16             m_eShapeType;            // 0x22 (TShapeBaseClass::Type)
	TINT               m_iWorld;                // 0x24
	TFLOAT             m_fRadius;               // 0x28
	TFLOAT             m_fMass;                 // 0x2c
	TFLOAT             m_fMaxLinearVelocity;    // 0x30
	TFLOAT             m_fMaxAngularVelocity;   // 0x34
	TUINT32            m_uiCollisionFilterInfo; // 0x38
	TUINT16            m_eMotionType;           // 0x3c
	TUINT16            m_eCreationMotionType;   // 0x3e
	TUINT32            m_uiSurface;             // 0x40
	TFLOAT             m_fUnk44;                // 0x44
	TFLOAT             m_fRestitution;          // 0x48
	TFLOAT             m_fFriction;             // 0x4c
	TFLOAT             m_fLinearDamping;        // 0x50
	TFLOAT             m_fAngularDamping;       // 0x54
	TPhysicsResponder* m_pListener;             // 0x58
	void*              m_pCollisionListener;    // 0x5c
	void*              m_pMoveCallback;         // 0x60
	union
	{
		TShapeBaseClass*       m_pShapeData;     // 0x64
		TShapeList*            m_pShapeList;     // 0x64 (shape type 7)
		TPhysicsMeshContainer* m_pMeshContainer; // 0x64 (shape type 6)
		TPhysicsHFContainer*   m_pHFContainer;   // 0x64 (shape type 8)
	};
	hkpShape*               m_geom;                 // 0x68
	hkpRigidBody*           m_body;                 // 0x6c
	hkpSurfaceVelocityUtil* m_pSurfaceVelocityUtil; // 0x70 TPhysicsManager::SetSurfaceVelocity (conveyors)
	TUINT32                 m_aUnk74[ 2 ];          // 0x74
};

TOSHI_NAMESPACE_END
