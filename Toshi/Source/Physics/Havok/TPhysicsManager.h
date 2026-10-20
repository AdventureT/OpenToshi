#pragma once
#include "Toshi/TSingleton.h"
#include "Toshi/T2DList.h"
#include "Toshi/T2MemoryAllocator.h"
#include "Math/TVector4.h"
#include "Math/TMatrix44.h"
#include "ToshiTools/T2DynamicVector.h"
#include "TPhysicsObjectContainer.h"
#include "TPhysicsPhantomContainer.h"

// Havok Physics (retail: 5.5.0, we build against 6.5; see CLAUDE.md "Third-party")
#include <Common/Base/hkBase.h>
#include <Physics/Dynamics/Motion/hkpMotion.h>
#include <Physics/Collide/Agent/Collidable/hkpCollidableQualityType.h>

#define PHYSICS_MANAGER_ERROR 0.0001f

class hkpWorld;
class hkpCdBody;
class hkpCollidable;
class hkMemory;
struct hkMemoryStatistics;

TOSHI_NAMESPACE_START
class TPhysicsMeshContainer;
class TPhysicsHFContainer;
class TPhysicsManagerCollisionListener;
class TPhysicsResponder;
struct TPhysicsMeshMaterial;
class TPhysicsPhantomContainer;
TOSHI_NAMESPACE_END

class hkThreadMemory;
class hkMemoryBlockServer;
class hkJobQueue;
class hkJobThreadPool;
class hkpConstraintInstance;

TOSHI_NAMESPACE_START

class TMemoryListener;
class TPhysicsBrokenConstraintListener;
class TPhysicsConstraintContainer;
class TPhysicsMotionBase;
class TPhysicsPluginBase;

#define PHYSICS_WORLDS 2

// The init data copied by TPhysicsManager::Initialise (0x4c bytes, the first member of TPhysicsManager)
struct TPhysicsManagerInitData
{
	TFLOAT   m_fTimeStep;                     // 0x00
	TVector4 m_vBroadPhaseMin;                // 0x04
	TVector4 m_vBroadPhaseMax;                // 0x14
	TVector4 m_vGravity;                      // 0x24
	TINT     m_iUnk34;                        // 0x34
	TINT     m_iMaxCustomMovers;              // 0x38
	TINT     m_iMaxConstraints;               // 0x3c
	TINT     m_iMaxCollisionCallbacks;        // 0x40
	TINT     m_iMaxTriggerCallbacks;          // 0x44
	TINT     m_iMaxConstraintBrokenCallbacks; // 0x48
};

TSTATICASSERT( sizeof( TPhysicsManagerInitData ) == 0x4c );

// A contact found by TPhysicsManager::CollideShape (0x6c bytes)
struct TPhysicsShapeContact
{
	TVector4                 m_vPosition;       // 0x00
	TVector4                 m_vNormal;         // 0x10 the separating normal
	TPhysicsObjectContainer* m_pBody;           // 0x20 the user data of the other shape
	TUINT32                  m_uiMaterial;      // 0x24 the material flags (the triangle's or the body's default)
	TVector4                 m_vSurfaceNormal;  // 0x28 m_vNormal, or the triangle's normal on a mesh
	TVector4                 m_avTriangle[ 3 ]; // 0x38 the triangle on a mesh
	TFLOAT                   m_fDistance;       // 0x68
};

TSTATICASSERT( sizeof( TPhysicsShapeContact ) == 0x6c );

// A ray of TPhysicsManager::CollideRay (0x24 bytes on the caller's stack)
struct TPhysicsRay
{
	TVector4 m_vFrom;            // 0x00
	TVector4 m_vTo;              // 0x10
	TUINT32  m_uiCollisionLayer; // 0x20
};

// A trigger callback queued while the world steps (0xc bytes)
struct TPhysicsTriggerCallback
{
	TPhysicsObjectContainer* m_pTrigger; // 0x0
	TPhysicsObjectContainer* m_pOther;   // 0x4
	TBOOL                    m_bEnter;   // 0x8
};

// One body of a collision, as seen by one of the two bodies (0x1c bytes)
struct TPhysicsCollisionBody
{
	TPhysicsObjectContainer* m_pBody;      // 0x00
	TVector4                 m_vVelocity;  // 0x04 the linear velocity of a dynamic body (motion type 3), else 0
	TMatrix44*               m_pTransform; // 0x14 points into TPhysicsCollisionCallback::m_aTransforms
	TUINT32                  m_uiMaterial; // 0x18 the surface (TPhysicsMeshMaterial::m_uiSurface or the body's m_uiSurface)
};

// A collision as seen by one body (0x5c bytes): passed to TPhysicsResponder's collision handlers
struct TPhysicsCollisionInfo
{
	TPhysicsCollisionBody m_self;           // 0x00 the body of the responder
	TPhysicsCollisionBody m_other;          // 0x1c
	TFLOAT                m_fRelativeSpeed; // 0x38
	TVector4              m_vPosition;      // 0x3c
	TVector4              m_vNormal;        // 0x4c the contact normal (negated in body B's view)
};

// A collision callback queued while the world steps (0x13c bytes)
struct TPhysicsCollisionCallback
{
	enum Type : TUINT16
	{
		Type_Confirmed = 0x01,
		Type_Process   = 0x02,
		Type_Added     = 0x08, // dispatched at once
		Type_Disabled  = 0x10,
	};

	TPhysicsCollisionInfo m_aInfo[ 2 ];       // 0x00 [0]: body A's view, [1]: body B's view
	TUINT16               m_uiType;           // 0xb8 Type
	TUINT16               m_uiContactPointId; // 0xba
	TMatrix44             m_aTransforms[ 2 ]; // 0xbc copies of the bodies' matrices
};

TSTATICASSERT( sizeof( TPhysicsCollisionCallback ) == 0x13c );

// Stored in a contact point's user data by TPhysicsManagerCollisionListener (0x14 bytes)
struct TPhysicsContactData
{
	TUINT32                  m_uiMaterialA; // 0x00
	TUINT32                  m_uiMaterialB; // 0x04
	TPhysicsObjectContainer* m_pBodyA;      // 0x08
	TPhysicsObjectContainer* m_pBodyB;      // 0x0c
	TBOOL                    m_bUsed;       // 0x10
	TBOOL                    m_bToi;        // 0x11
};

// 800 bytes
class TPhysicsManager
    : public TSingleton<TPhysicsManager>
{
public:
	TPhysicsManager();
	~TPhysicsManager();

	static TPhysicsManager* Create();

	void Initialise( const TPhysicsManagerInitData* a_pInitData );
	void Unitialise();

	// Steps the worlds in fixed m_fTimeStep steps
	void Update( TFLOAT a_fDeltaTime );

	// Once a frame (AApplication::OnUpdate): keeps the batch statistics of the frame and, while the batch commits are
	// disabled (SetBatchCommitDisabled), commits the remove or the add batch (alternately, up to m_iBatchCommitLimit bodies) and
	// the phantom batches
	void UpdateBatches( TFLOAT a_fDeltaTime );

	// Starts/finishes the step of world a_iWorld (-1: all of them)
	void AddPlugin( TPhysicsPluginBase* a_pPlugin );
	void RemovePlugin( TPhysicsPluginBase* a_pPlugin );

	// Starts/finishes the step of world a_iWorld (-1: all of them)
	void StartStepWorld( TINT a_iWorld );
	void FinishStepWorld( TINT a_iWorld );

	// Creates a container for a body of motion type a_eMotionType (MotionType) at a_pMatrix (unit scale)
	TPhysicsObjectContainer* CreateBody( TMatrix44* a_pMatrix, void* a_pUserData, TINT a_eMotionType, TINT a_eSubType );

	// CreateBody( .., 0, 1 ) flagged PMF_FIXED (APropTrigger's touch phantom)
	TPhysicsObjectContainer* CreateFixedBody( TMatrix44* a_pMatrix, void* a_pUserData );

	// Removes the body and destroys its container (deferred while the world steps)
	void DestroyBody( TPhysicsObjectContainer* a_pBody );

	// Destroys the bodies whose destruction was deferred
	void DestroyPendingBodies();

	// Builds the Havok body of a_pBody and adds it to its world (or to the add batch while a batch operation is in
	// progress, unless a_bForce)
	void AddToSimulation( TPhysicsObjectContainer* a_pBody, TBOOL a_bForce );

	// Changes the motion type of a body created dynamic (MotionType_*), moving it to the matching active list
	void SetMotionType( TPhysicsObjectContainer* a_pBody, TUINT a_eMotionType );

	// $deBlob: FUNCTION 005612d0
	TINT SetDefaultWorld( TINT a_world )
	{
		TASSERT( a_world >= 0 );
		TASSERT( a_world < PHYSICS_WORLDS );
		TINT iOldWorld  = m_iDefaultWorld;
		m_iDefaultWorld = a_world;
		return iOldWorld;
	}

	// $deBlob: FUNCTION 005a6150
	TINT GetDefaultWorld() const { return m_iDefaultWorld; }

	// Lets bodies be added/removed directly while a batch operation is in progress, until ResumeBatchOperation
	void SuspendBatchOperation()
	{
		m_bSavedBatchOperation      = m_bBatchOperationInProgress;
		m_bBatchOperationInProgress = TFALSE;
	}

	void ResumeBatchOperation() { m_bBatchOperationInProgress = m_bSavedBatchOperation; }

	// Removes the body from its world and frees its Havok body (or puts it in the remove batch)
	void RemoveFromSimulation( TPhysicsObjectContainer* a_pBody, TBOOL a_bForce );

	// Moves the Havok body of a_pBody to its matrix (m_pMatrix) and wakes it up unless a_bNoActivate
	static void UpdateBodyTransform( TPhysicsObjectContainer* a_pBody, TBOOL a_bNoActivate );

	// Gives a_pBody a box shape with these half extents
	static void SetBox( TPhysicsObjectContainer* a_pBody, const TVector4& a_rcHalfExtents );

	// Give a_pBody a sphere/capsule/cylinder (along z)/plane shape
	static void SetSphere( TPhysicsObjectContainer* a_pBody, TFLOAT a_fRadius );
	static void SetCapsule( TPhysicsObjectContainer* a_pBody, TFLOAT a_fHeight, TFLOAT a_fRadius );
	static void SetCylinder( TPhysicsObjectContainer* a_pBody, TFLOAT a_fHeight, TFLOAT a_fRadius );
	static void SetPlane( TPhysicsObjectContainer* a_pBody, const TVector4& a_rcNormal, const TVector4& a_rcPoint );

	// Gives a_pBody the mesh shape of a_pContainer (shape type 6)
	static void SetMesh( TPhysicsObjectContainer* a_pBody, TPhysicsMeshContainer* a_pContainer );
	static void SetHeightField( TPhysicsObjectContainer* a_pBody, TPhysicsHFContainer* a_pContainer, TBOOL a_bUnk );

	// Set the low (layer) / high (system group) 12 bits of the collision filter info of a_pBody, updating the Havok
	// body's filter when it's in the simulation
	void SetCollisionLayer( TPhysicsObjectContainer* a_pBody, TUINT32 a_uiLayer );

	// Sets the responder of a_pBody's collisions, creating/destroying the collision listener of its Havok body
	void SetListener( TPhysicsObjectContainer* a_pBody, TPhysicsResponder* a_pListener );
	void SetCollisionGroup( TPhysicsObjectContainer* a_pBody, TUINT32 a_uiGroup );

	// Add a shape at a_rcTransform (unit scale) to the shape list of a_pBody
	static void AddListSphere( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, TFLOAT a_fRadius );
	static void AddListBox( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, const TVector4& a_rcHalfExtents );
	static void AddListCapsule( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, TFLOAT a_fHeight, TFLOAT a_fRadius );
	static void AddListCylinder( TPhysicsObjectContainer* a_pBody, const TMatrix44& a_rcTransform, TFLOAT a_fHeight, TFLOAT a_fRadius );

	// Starts/ends a batch operation: bodies added/removed meanwhile go to the add/remove batches
	void BeginBatchOperation();
	void EndBatchOperation();

	// a_bBatchCommitDisabled: while set, ending a batch operation doesn't commit the batches (committed when it's cleared)
	void SetBatchCommitDisabled( TBOOL a_bBatchCommitDisabled, TINT a_iBatchCommitLimit );

	// TRUE while bodies wait to be added to the world (ATerrain2::IsStreaming reads the list directly)
	TBOOL HasAddedBodies() const { return !m_addedBodies.IsEmpty(); }

	// Adds/removes a custom mover (updated before each step of the default world at the time it's added)
	void AddCustomMover( TPhysicsMotionBase* a_pMover );
	void RemoveCustomMover( TPhysicsMotionBase* a_pMover );

	// Creates a constraint container between a_pBodyA and a_pBodyB in the default world
	TPhysicsConstraintContainer* CreateConstraint( TPhysicsObjectContainer* a_pBodyA, TPhysicsObjectContainer* a_pBodyB, TBOOL a_bWantRuntime );

	// Returns the index of a_pConstraint in m_pConstraints (-1: not found)
	TINT FindConstraint( TPhysicsConstraintContainer* a_pConstraint ) const;

	// Finds an id no body uses (the top byte of the collision filter info) for the bodies of a constraint group
	TINT8 FindFreeSubSystemId();

	// Gives a_pBody the id (the top byte of its collision filter info)
	static void SetSubSystemId( TPhysicsObjectContainer* a_pBody, TINT8 a_iId );

	// The constraint becomes breakable at a_fThreshold / malleable with a_fStrength
	static void SetConstraintBreakable( TPhysicsConstraintContainer* a_pConstraint, TFLOAT a_fThreshold );
	static void SetConstraintMalleable( TPhysicsConstraintContainer* a_pConstraint, TFLOAT a_fStrength, TFLOAT a_fUnk );

	// What a broken constraint does to one of its bodies
	static void SetConstraintBodyBreakInfo( TPhysicsConstraintContainer* a_pConstraint, TPhysicsObjectContainer* a_pBody, TFLOAT a_fUnk, TFLOAT a_fAngularDamping, TFLOAT a_fLinearDamping, TUINT8 a_uiFlag );

	// Set up a constraint container as a limited hinge / a stiff spring (the points are in world space)
	static void SetConstraintLimitedHinge( TPhysicsConstraintContainer* a_pConstraint, const TVector4& a_rvPivot, const TVector4& a_rvAxisPoint, TFLOAT a_fMinAngle, TFLOAT a_fMaxAngle, TFLOAT a_fFriction );
	static void SetConstraintStiffSpring( TPhysicsConstraintContainer* a_pConstraint, const TVector4& a_rvPivotA, const TVector4& a_rvPivotB );
	// A hinge at a_rvPivot around the axis towards a_rvAxisPoint
	static void SetConstraintHinge( TPhysicsConstraintContainer* a_pConstraint, const TVector4& a_rvPivot, const TVector4& a_rvAxisPoint );

	// Steers a wheel constraint in the world (unless it's broken)
	static void SetConstraintSteeringAngle( TPhysicsConstraintContainer* a_pConstraint, TFLOAT a_fAngle );
	// The impulse the solver applied to the constraint in the last step (the sum of its solver results), 0 if it's
	// not in the world
	static TFLOAT GetConstraintImpulse( TPhysicsConstraintContainer* a_pConstraint );

	// Creates the Havok constraint of a container and adds it to its world (if both bodies are in the simulation)
	void AddConstraint( TPhysicsConstraintContainer* a_pConstraint );

	// Removes the constraint from its world and frees it
	void DestroyConstraint( TPhysicsConstraintContainer* a_pConstraint );

	// Removes the constraint from its world and releases its Havok constraint
	void RemoveConstraint( TPhysicsConstraintContainer* a_pConstraint );

	// Called by TPhysicsTriggerShape: queue a trigger callback (a_pOther entered/left the trigger body a_pTrigger)
	void OnTriggerEnter( TPhysicsObjectContainer* a_pTrigger, TPhysicsObjectContainer* a_pOther );
	void OnTriggerLeave( TPhysicsObjectContainer* a_pTrigger, TPhysicsObjectContainer* a_pOther );

	// Called by TPhysicsBrokenConstraintListener
	void OnConstraintBroken( hkpConstraintInstance* a_pConstraint, TFLOAT a_fForce );

	// The linear velocity of a body in the simulation (zero otherwise; TPhysicsManager_i_Inline.h)
	void   GetLinearVelocity( TPhysicsObjectContainer* a_pPOC, TVector4& a_rvOutVelocity ) const;
	TFLOAT GetMaxLinearVelocity( TPhysicsObjectContainer* a_pPOC ) const;

	// Set the linear/angular velocity of a body in the simulation (activating it) unless it already has it within
	// 0.001 (TPhysicsManager_i_Inline.h)
	// The centre of mass in world space (the origin without a body)
	static TVector4& GetWorldCenterOfMass( TPhysicsObjectContainer* a_pPOC, TVector4& a_rvOutCenterOfMass );
	// Keeps the centre of mass (local) and sets it on the rigid body if there is one
	static void SetCenterOfMass( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcCenterOfMass );
	static void SetLinearVelocity( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcVelocity );
	static void SetAngularVelocity( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcVelocity );

	// Creates a Havok shape of a_pShapeData (sphere, box, capsule, cylinder), TNULL for other types
	static hkpShape* CreateShape( const TShapeBaseClass* a_pShapeData );

	// Finds the contacts of a_pShapeData at a_rcMatrix in the default world (closest points within 0.001, sorted);
	// fills at most a_iMaxContacts of a_pOutContacts and returns the number of contacts found; a_pPhantom: a phantom of
	// the shape that is in the world (its cached contacts are used)
	TINT CollideShape( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact* a_pOutContacts, TINT a_iMaxContacts, TPhysicsPhantomContainer* a_pPhantom );

	// Like CollideShape, but only the closest contact (within 0.001) in a_rOutContact: TFALSE if there is none
	TBOOL CollideShapeClosest( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact& a_rOutContact, TPhysicsPhantomContainer* a_pPhantom );

	// Casts a ray in the default world (or through an AABB phantom in the simulation): the closest hit in a_rOutContact
	// (m_fDistance: the hit fraction); TFALSE if nothing was hit, the ray is too short or leaves the world
	TBOOL CollideRay( const TPhysicsRay& a_rcRay, TPhysicsShapeContact& a_rOutContact, TPhysicsPhantomContainer* a_pPhantom );

	// Debug checks of a query through a phantom: its type, its collision filter and (rays) that its box holds the ray
	// $deBlob: FUNCTION 005a9c30
	static TBOOL IsPhantomInSimulation( TPhysicsPhantomContainer* phantomContainer )
	{
		TASSERT( phantomContainer );
		return phantomContainer->IsInSimulation();
	}

	static void CheckRayPhantom( TPhysicsPhantomContainer* phantomContainer, const TPhysicsRay& a_rcRay );
	static void CheckShapePhantom( TPhysicsPhantomContainer* phantomContainer, const TShapeBaseClass& a_shape, TUINT32 a_uiCollisionLayer );

	// Casts a ray like CollideRay but returns every hit (sorted by distance, at most a_iMaxContacts in
	// a_pOutContacts); the number of hits written
	TINT CollideRayAll( const TPhysicsRay& a_rcRay, TPhysicsShapeContact* a_pOutContacts, TINT a_iMaxContacts, TPhysicsPhantomContainer* a_pPhantom );

	// The triangle (m_avTriangle, m_vSurfaceNormal) and surface (m_uiMaterial) of shape key a_uiKey of a mesh body (its
	// m_pBody must be set); TRUE only if the triangle has a material
	TBOOL GetMeshContact( const hkpCollidable* a_pRootCollidable, TUINT32 a_uiKey, TPhysicsShapeContact& a_rContact );

	// The phantoms (m_phantoms)
	TPhysicsPhantomContainer* CreateShapePhantom( const TShapeBaseClass* a_pShapeData );
	TPhysicsPhantomContainer* CreateAabbPhantom();
	void                      AddPhantomToSimulation( TPhysicsPhantomContainer* a_pContainer, const TVector4& a_rcPosition, void* a_pUnused );
	void                      RemovePhantomFromSimulation( TPhysicsPhantomContainer* a_pContainer, void* a_pUnused ); // a_pUnused is never read
	void                      DestroyPhantom( TPhysicsPhantomContainer* a_pContainer );
	void                      SetPhantomCollisionFilterInfo( TPhysicsPhantomContainer* a_pContainer, TUINT32 a_uiFilterInfo );
	// The test flags (bits 12-23 of the collision filter info) of a phantom, 0 without one
	TUINT32 GetCollisionTestFlags( TPhysicsPhantomContainer* a_pContainer ) const;
	// The collision layer (12 bits) the phantom's queries test against
	static void SetCollisionTestFlags( TPhysicsPhantomContainer* a_pContainer, TUINT32 a_uiTestFlags );
	// Sets the box of an AABB phantom (a_pBox[ 0 ]: the centre, a_pBox[ 1 ]: the half extents, grown by 1%)
	void SetAabbPhantomBox( TPhysicsPhantomContainer* a_pContainer, const TVector4* a_pBox );

	// Shape phantoms are hkpCachingShapePhantoms instead of hkpSimpleShapePhantoms (never set in retail)
	static TBOOL ms_bCachingPhantoms;
	// Use the phantoms in CollideRay/CollideShape
	static TBOOL ms_bRayPhantoms;
	static TBOOL ms_bShapePhantoms;

	// Sweeps a_pShapeData from a_rcMatrix to a_rcTo in the default world (linear cast, hits sorted, m_fDistance is the
	// fraction of the sweep); fills at most a_iMaxContacts of a_pOutContacts and returns the number filled
	TINT SweepShape( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, const TVector4& a_rcTo, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact* a_pOutContacts, TINT a_iMaxContacts, TPhysicsPhantomContainer* a_pPhantom );

	// Sweeps a_pShapeData from a_rcMatrix to a_rcTo like SweepShape but keeps only the closest hit (in a_rOutContact,
	// m_fDistance: the hit fraction); TFALSE if nothing was hit or the sweep leaves the world
	TBOOL SweepShapeClosest( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, const TVector4& a_rcTo, TUINT32 a_uiCollisionLayer, TPhysicsShapeContact& a_rOutContact, TPhysicsPhantomContainer* a_pPhantom );

	// The angular velocity of a body in the simulation (zero otherwise; TPhysicsManager_i_Inline.h)
	void GetAngularVelocity( TPhysicsObjectContainer* a_pPOC, TVector4& a_rvOutVelocity ) const;

	// Apply a force/torque to a body in the simulation for one time step (activating it); the force is passed to the
	// body's listener (TPhysicsResponder +0x14) with the centre of mass
	void ApplyForce( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcForce );
	void ApplyTorque( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcTorque );
	void SetSurfaceVelocity( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcVelocity );
	// The same with the force applied at a_rcPoint (the listener gets the force and the point)
	void ApplyPointForce( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcPoint, const TVector4& a_rcForce );
	// An impulse as a force over one time step (a_rcImpulse / m_fTimeStep)
	void ApplyImpulse( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcImpulse );
	void ApplyPointImpulse( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcPoint, const TVector4& a_rcImpulse );
	// Pushes the dynamic bodies within a_fRadius of a_rcCentre (except a_pIgnore) away from it, with an impulse
	// falling off linearly from a_fStrength
	void ApplyExplosion( TPhysicsObjectContainer* a_pIgnore, const TVector4& a_rcCentre, TFLOAT a_fStrength, TFLOAT a_fRadius );

	// The active bodies of a motion type (0: static, 3: dynamic, others: keyframed), TNULL at the end
	TPhysicsObjectContainer* GetFirstActiveBody( TINT a_eMotionType );
	TPhysicsObjectContainer* GetNextActiveBody( TPhysicsObjectContainer* a_pBody, TINT a_eMotionType );
	// The number of phantoms in the simulation
	TINT GetNumPhantomsInSimulation();
	// Whether a_pBody is in a_rList
	static TBOOL IsBodyInList( TPhysicsObjectContainer* a_pBody, T2DList<TPhysicsObjectContainer>& a_rList );

	// Whether a ray / a shape at a_rcMatrix / a swept shape is inside the broadphase (the world)
	TBOOL IsRayInWorld( const TPhysicsRay& a_rcRay );
	TBOOL IsShapeInWorld( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix );
	TBOOL IsSweptShapeInWorld( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, const TVector4& a_rcTo );

	// Whether a_pShapeData at a_rcMatrix penetrates a body in the default world (the first one found in
	// a_ppOutBody) / every body it penetrates (at most a_iMaxBodies in a_ppOutBodies, returns the number found)
	TBOOL IntersectShape( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsObjectContainer** a_ppOutBody, TPhysicsPhantomContainer* a_pPhantom );
	TINT  IntersectShapeAll( const TShapeBaseClass* a_pShapeData, const TMatrix44& a_rcMatrix, TUINT32 a_uiCollisionLayer, TPhysicsObjectContainer** a_ppOutBodies, TINT a_iMaxBodies, TPhysicsPhantomContainer* a_pPhantom );

	// The body's collision layer (m_uiCollisionFilterInfo & 0xFFF), 0 without a body
	// $deBlob: FUNCTION 005784b0
	static TUINT32 GetCollisionLayer( TPhysicsObjectContainer* a_pPOC ) { return a_pPOC ? ( a_pPOC->m_uiCollisionFilterInfo & 0xFFF ) : 0; }

	// Zeroes the body's linear and angular velocity
	static void StopBody( TPhysicsObjectContainer* a_pPOC );

	// Applies a_rcAcceleration * mass at a_rcPoint
	void ApplyForceAtPoint( TPhysicsObjectContainer* a_pPOC, const TVector4& a_rcPoint, const TVector4& a_rcAcceleration );

	// Moves a keyframed body to a_rcTarget within a_fTimeStep by setting its velocities
	// Sets the matrix of a_pPOC and updates its Havok body (TPhysicsManager_i_Inline.h)
	static void SetTransform( TPhysicsObjectContainer* a_pPOC, const TMatrix44& a_rcMatrix, TBOOL a_bNoActivate );

	static void ApplyHardKeyFrame( TPhysicsObjectContainer* a_pPOC, const TMatrix44& a_rcTarget, TFLOAT a_fTimeStep );

	TBOOL IsInitialised() const { return m_initialised; }

	// The statistics of Havok's memory (the free list memory)
	void GetMemoryStatistics( hkMemoryStatistics& a_rOutStats );
	// $deBlob: FUNCTION 005a7fe0
	TFLOAT GetTimeStep() const { return m_fTimeStep; }
	// $deBlob: FUNCTION 005a7fb0
	TINT      GetNumSteps() const { return m_iNumSteps; }

#ifdef DEBLOB_COMMUNITY_PATCH
	// The render interpolation of moving objects between the fixed steps: the steps taken since the creation and
	// the time left in the accumulator after the last Update (less than one fixed step)
	TUINT32 GetTotalSteps() const { return m_uiTotalSteps; }
	TFLOAT  GetStepRemainder() const { return m_fAliveTime; }
#endif
	hkpWorld* GetWorld( TINT a_world ) const;

	// The heap of the physics allocations (TNULL: the global heap)
	static TMemoryHeap* GetHeap();

	enum MotionType
	{
		MotionType_Static             = 0,
		MotionType_Keyframed          = 1,
		MotionType_KeyframedReporting = 2,
		MotionType_Dynamic            = 3,
	};

	// The a_eSubType of CreateBody (only used for dynamic bodies)
	enum SubType
	{
		SubType_SphereInertia = 0, // PMF_SPHERE_INERTIA
		SubType_Default       = 1,
		SubType_Unk2          = 2, // PMF_UNK2000
	};

private:
	friend class TPhysicsManagerCollisionListener;

	// The list moves of TPhysicsManager_i_Inline.h
	void DeactivateBody( TPhysicsObjectContainer* a_pBody ); // to m_bodies
	void ActivateBody( TPhysicsObjectContainer* a_pBody );   // to the active list of its motion type
	void AddBody( TPhysicsObjectContainer* a_pBody );        // to the add batch
	void RemoveBody( TPhysicsObjectContainer* a_pBody );     // to the remove batch

	// Commit the batches (EndBatchOperation): the bodies of the remove/add batch of one world (the one with the most
	// of them) leave/enter it, then the phantoms
	void CommitRemoveBatch();
	void CommitAddBatch();
	void CommitPhantomAddBatch();
	void CommitPhantomRemoveBatch();

	static hkpMotion::MotionType    GetHavokMotionType( TPhysicsObjectContainer* a_pBody, TINT a_eMotionType );
	static hkpCollidableQualityType GetHavokQualityType( TPhysicsObjectContainer* a_pBody, TINT a_eMotionType );

	// The per-step callbacks of world a_iWorld (-1: all)
	void        UpdateCustomMovers( TINT a_iWorld );
	void        StartStepPlugins( TINT a_iWorld );
	void        FinishStepPlugins( TINT a_iWorld );
	void        UpdateActiveBodies( TINT a_iWorld );
	void        UpdateActiveBody( TPhysicsObjectContainer* a_pBody ); // copies the Havok transform to the body's matrix
	void        DispatchCollisionCallbacks( TINT a_iWorld );
	static void DispatchCollisionCallback( TPhysicsCollisionCallback* a_pCallback, TINT a_iWorld );

	// The collision callbacks recorded by TPhysicsManagerCollisionListener (a_uiType: TPhysicsCollisionCallback::Type)
	void RecordCollisionCallback( TPhysicsObjectContainer* a_pBodyA, TPhysicsObjectContainer* a_pBodyB, TUINT32 a_uiMaterialA, TUINT32 a_uiMaterialB, const TVector4& a_rPosition, const TVector4& a_rNormal, TUINT16 a_uiType, TUINT16 a_uiContactPointId );
	// Keeps only the Type_Disabled bit of the recorded callbacks of a contact point
	void DisableCollisionCallbacks( TUINT16 a_uiContactPointId );

	// The surface of the shape key of a_pCdBody (the mesh material's or the body's default)
	TUINT32                     GetCollisionMaterial( const hkpCdBody* a_pCdBody );
	const TPhysicsMeshMaterial* GetMeshMaterial( const hkpCdBody* a_pCdBody );

	TPhysicsContactData* AllocContactData();
	void                 FreeContactData( TPhysicsContactData* a_pData );

	// $deBlob: FUNCTION 006254d0
	static TBOOL IsInAddBatch( TPhysicsObjectContainer* POC )
	{
		TASSERT( POC != TNULL );
		return ( POC->m_flags & TPhysicsObjectContainer::PMF_IN_ADD_BATCH ) != 0;
	}

	// $deBlob: FUNCTION 005a60f0
	static TUINT32 GetDefaultMaterial( TPhysicsObjectContainer* POC )
	{
		TASSERT( POC != TNULL );
		return POC ? POC->m_uiSurface : 0;
	}
	void DispatchConstraintBrokenCallbacks( TINT a_iWorld );
	void DispatchTriggerCallbacks( TINT a_iWorld );

	// Havok 6.5 replacements of hkpMultithreadingUtil::startStepWorld/waitForStepWorld (see Initialise)
	void StartStepWorldThread( TINT a_iWorld );
	void FinishStepWorldThread( TINT a_iWorld );

	// hkSystemMalloc/hkSystemFree
	static void* __cdecl Alloc( int a_iSize, int a_iAlign );
	static void __cdecl Free( void* a_pMem );

public:
	static TMemoryHeap* ms_pHeap;

	// $deBlob: FUNCTION 0058cb90
	hkMemoryBlockServer* GetMemoryBlockServer() const { return m_pMemoryBlockServer; }
	// The world bounds (m_vBroadPhaseMin/Max) are read by AEntityFactory::ProcessStaleEntity
	const TPhysicsManagerInitData& GetInitData() const { return m_initData; }

private:
	TPhysicsManagerInitData           m_initData;        // 0x00
	T2MemoryAllocator                 m_MemoryAllocator; // 0x58
	void*                             m_pStackBuffer;    // 0x54
	hkMemory*                         m_pFreeListMemory;
	hkThreadMemory*                   m_pThreadMemory;
	void*                             m_pUnk60;                     // 0x60
	hkMemoryBlockServer*              m_pMemoryBlockServer;         // 0x64
	TMemoryListener*                  m_pMemoryListener;            // 0x68
	hkpWorld*                         m_apWorlds[ PHYSICS_WORLDS ]; // 0x6c
	TPhysicsBrokenConstraintListener* m_pBrokenConstraintListener;  // 0x74
	TINT                              m_iDefaultWorld;              // 0x78 (the world of new bodies)
	TFLOAT                            m_fAliveTime;                 // 0x7c

	T2DList<TPhysicsObjectContainer>  m_bodies;               // 0x80
	T2DList<TPhysicsObjectContainer>  m_activeBodiesStatic;   // 0x88
	T2DList<TPhysicsObjectContainer>  m_activeBodiesKeyframe; // 0x90
	T2DList<TPhysicsObjectContainer>  m_activeBodiesDynamic;  // 0x98
	T2DList<TPhysicsObjectContainer>  m_addedBodies;          // 0xa0
	T2DList<TPhysicsObjectContainer>  m_removedBodies;        // 0xa8
	TUINT32                           m_auiBatchStats[ 20 ];  // 0xb0
	T2DList<TPhysicsObjectContainer>  m_destroyBodies;        // 0x100
	T2DList<TPhysicsPhantomContainer> m_phantoms;             // 0x108
	T2DList<TPhysicsPhantomContainer> m_PhantomAddBatch;      // 0x110 the phantoms waiting for CommitPhantomAddBatch
	T2DList<TPhysicsPhantomContainer> m_PhantomRemoveBatch;   // 0x118 the phantoms waiting for CommitPhantomRemoveBatch

	TPhysicsCollisionCallback*            m_pCollisionCallbacks;           // 0x120
	TINT                                  m_iNumCollisionCallbacks;        // 0x124
	TINT                                  m_iMaxNumCollisionCallbacksUsed; // 0x128
	TPhysicsContactData*                  m_pContactData;                  // 0x12c pool of m_iNumContactData
	TPhysicsContactData*                  m_pFreeContactData;              // 0x130 the first free entry
	TINT                                  m_iNumContactData;               // 0x134
	TINT                                  m_iNumContactDataAllocs;         // 0x138
	TINT                                  m_iNumContactDataFrees;          // 0x13c
	T2DynamicVector<TPhysicsMotionBase*>* m_pCustomMovers;                 // 0x140
	TINT                                  m_iNumCustomMovers;              // 0x144

	TINT                                           m_iNumStepListeners;                       // 0x148
	TPhysicsPluginBase*                            m_apStepListeners[ 100 ];                  // 0x14c
	TINT                                           m_iLastSubSystemId;                        // 0x2dc
	T2DynamicVector<TPhysicsConstraintContainer*>* m_pConstraints;                            // 0x2e0
	void*                                          m_pConstraintBrokenCallbacks;              // 0x2e4 (constraint, force)
	TINT                                           m_iNumConstraintBrokenCallbacks;           // 0x2e8
	TINT                                           m_iMaxNumConstraintBrokenCallbacksUsed;    // 0x2ec
	TPhysicsTriggerCallback*                       m_pTriggerCallbacks;                       // 0x2f0
	TINT                                           m_iNumTriggerCallbacks;                    // 0x2f4
	TINT                                           m_iMaxNumTriggerCallbacksUsed;             // 0x2f8
	TBOOL                                          m_initialised;                             // 0x2fc
	TFLOAT                                         m_fTimeStep;                               // 0x300
	TBOOL                                          m_bInSimulation;                           // 0x304
	TBOOL                                          m_bBatchOperationInProgress;               // 0x305
	TINT                                           m_iNumSteps;                               // 0x308
	TBOOL                                          m_bBatchCommitDisabled;                    // 0x30c
	TBOOL                                          m_bSavedBatchOperation;                    // 0x30d (SuspendBatchOperation)
	TINT                                           m_iBatchCommitLimit;                       // 0x310
	void*                                          m_apMultithreadingUtils[ PHYSICS_WORLDS ]; // 0x314 (hkpMultithreadingUtil, Havok 5.5 only)

	// NOTE: Havok 6.5's replacement of retail's hkpMultithreadingUtil (one worker thread per world): a job queue
	// and a one-thread hkCpuJobThreadPool per world (not in retail's layout)
	hkJobQueue*      m_apJobQueues[ PHYSICS_WORLDS ];
	hkJobThreadPool* m_apThreadPools[ PHYSICS_WORLDS ];
	TBOOL            m_abStepping[ PHYSICS_WORLDS ]; // 0x31c

#ifdef DEBLOB_COMMUNITY_PATCH
	TUINT32 m_uiTotalSteps; // the fixed steps taken since the creation (GetTotalSteps)
#endif
};

TOSHI_NAMESPACE_END
