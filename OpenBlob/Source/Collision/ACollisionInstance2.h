#pragma once
#include <Toshi/T2DList.h>
#include <Math/TMatrix44.h>
#include <Render/TModel.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/Havok/TPhysicsMeshContainer.h>
#include <Physics/TPhysicsMotionBase.h>
#include "ACollisionInstanceMover.h"

class ACollisionInstanceResponder;

TOSHI_NAMESPACE_START
class TPhysicsConstraintContainer;
TOSHI_NAMESPACE_END

// The physics worlds of the game's bodies (TPhysicsManager::SetDefaultWorld)
enum APhysicsWorldType
{
	APhysicsWorldType_GamePlay = 0,
	APhysicsWorldType_Decal    = 1,
};

//-----------------------------------------------------------------------------
// Purpose: the physics bodies of a model instance's collision meshes: one TPhysicsObjectContainer per collision mesh of
// the model (at most 32), a static mesh body (or a dynamic one built from primitive shapes), with the collision filter
// and surface of its materials. Used by the terrain cells and the instances/entities with collision.
//-----------------------------------------------------------------------------
class ACollisionInstance2
    : public Toshi::T2DList<ACollisionInstance2>::Node
{
public:
	enum Flags : TUINT32
	{
		Flags_StaticBodies = BITFLAG( 0 ), // the bodies are made static when added to the world
		Flags_HasStatic    = BITFLAG( 1 ), // a static body was created
		Flags_HasDynamic   = BITFLAG( 2 ), // a dynamic body was created
		Flags_FirstDynamic = BITFLAG( 3 ), // set when the first body is a dynamic one (motion type 3)
		Flags_NotInWorld   = BITFLAG( 4 ), // AddToWorld does nothing
	};

	// The type of a physics object (GetPhysicsObjectType, the mask of AddToWorld/RemoveFromWorld)
	enum PhysicsObjectType : TUINT32
	{
		PhysicsObjectType_Standard = 0x20,
		PhysicsObjectType_Custom   = 0x40, // TPhysicsCustomMeshShape mesh
	};

	// Who owns the bodies (copied to each PhysicsInfo, the user data of the bodies)
	struct OwnerInfo
	{
		TINT  m_eType;  // 0x00 4: a terrain cell
		void* m_pOwner; // 0x04 TerrainTypes::CellData for a cell
	};

	// The owner object of the instance (m_pOwner); NOTE: type unknown, only its flags are read
	struct OwnerObject
	{
		void*   m_pUnk00;  // 0x00
		TUINT32 m_uiFlags; // 0x04
	};

	// A body (0x94 bytes)
	struct PhysicsInfo
	{
		Toshi::TMatrix44                m_mWorld;         // 0x00 the matrix of the body
		Toshi::TMatrix44                m_mLocal;         // 0x40
		Toshi::TPhysicsObjectContainer* m_pBody;          // 0x80
		Toshi::TPhysicsMeshContainer*   m_pMeshContainer; // 0x84
		OwnerInfo                       m_oOwnerInfo;     // 0x88 the user data of the body
		ACollisionInstanceMover*        m_pInstanceMover; // 0x90 (bodies attached to a bone)
	};

	// One shape or body description of an instance's physics definition (retail's element of PhysicsDef::m_pShapes, or
	// PhysicsDef::m_pBodyShape). The type 2..5 shapes are the list shapes of AddShapes
	struct PhysicsShape
	{
		TINT              m_iType;   // 0x00 0 dynamic body, 1 keyframed body, 2 sphere, 3 capsule, 4 cylinder, 5 box
		Toshi::TMatrix44* m_pMatrix; // 0x04 the shape's transform (nullptr: identity)
		TFLOAT            m_fParam1; // 0x08 radius (sphere, capsule, cylinder), half extent x (box), mass (body)
		union
		{
			TFLOAT m_fParam2; // 0x0c height (capsule, cylinder), half extent y (box), linear damping (body)
			TINT   m_iParam2; // 0x0c the custom mesh (type 6): 17 selects the layer of the "render" mesh
		};
		TFLOAT m_fParam3; // 0x10 half extent z (box), angular damping (body)
		TINT   m_iSpeed;  // 0x14 speed in km/h (body)
		TINT   m_iAngle;  // 0x18 turn rate, a tenth of a degree (body)
		TINT   m_iFlag;   // 0x1c (body) 0 selects subtype 1 of the body
	};

	// The shapes of a physics definition (retail's first word of PhysicsDef)
	struct PhysicsShapeList
	{
		TFLOAT         m_fRestitution; // 0x00
		TFLOAT         m_fFriction;    // 0x04
		TUINT32        m_uiUnk08;      // 0x08
		TUINT32        m_uiFlags;      // 0x0c the collision layer (ANDed with 0x400, 0x80...)
		TUINT32        m_uiGroup;      // 0x10 the collision group
		TUINT32        m_uiUnk14;      // 0x14 copied to the body's m_uiSurface
		TINT           m_iNumShapes;   // 0x18
		PhysicsShape** m_ppShapes;     // 0x1c
	};

	// The motion of an instance's dynamic body (retail's PhysicsDef::m_pMotion)
	struct PhysicsMotion
	{
		TINT   m_iUnk00;         // 0x00 stored in the responder
		TFLOAT m_fFallDelay;     // 0x04 mover's m_fFallDelay when > 0
		TINT   m_iSinkAndHide;   // 0x08 nonzero: SetSinkAndHide
		TFLOAT m_fHideDelay;     // 0x0c the parameter of SetSinkAndHide
		TINT   m_iSinkAndRemove; // 0x10 nonzero: SetSinkAndRemove
		TFLOAT m_fRemoveDelay;   // 0x14 the parameter of SetSinkAndRemove
		TINT   m_iUnk18;         // 0x18 nonzero: the mover gets flag 0x20
	};

	// The physics definition of a cell instance (retail's P, the record the m_pUnk28 of a CellInstance points to)
	struct PhysicsDef
	{
		PhysicsShapeList*               m_pShapes;    // 0x00
		void*                           m_pUnk04;     // 0x04 the same for every definition seen so far; NOTE: type unknown
		PhysicsShape*                   m_pBodyShape; // 0x08 the body's description (nullptr: static)
		Toshi::TMatrix44*               m_pMatrix;    // 0x0c the instance's matrix (copied to the body's local matrix)
		TINT                            m_iUnk10;     // 0x10
		TINT                            m_iUnk14;     // 0x14
		Toshi::TPhysicsObjectContainer* m_pBody;      // 0x18 the body created (written by CreateStandardBody)
		PhysicsMotion*                  m_pMotion;    // 0x1c
	};

	// What a constraint does when it breaks / how stiff it is (retail's element of ConstraintDef::m_pModifier)
	struct ConstraintModifier
	{
		TINT        m_iType;  // 0x00 0xD: malleable (m_fValue the strength), 0xE: breakable (m_fValue the threshold)
		TFLOAT      m_fValue; // 0x04
		TINT        m_iUnk08; // 0x08 nonzero (with m_pDefA): body A changes when it breaks
		PhysicsDef* m_pDefA;  // 0x0C
		TINT        m_iUnk10; // 0x10
		TFLOAT      m_fA1;    // 0x14 > 0: it changes
		TFLOAT      m_fA2;    // 0x18
		TFLOAT      m_fA3;    // 0x1C
		TFLOAT      m_fA4;    // 0x20 nonzero: flag
		TINT        m_iUnk24; // 0x24 nonzero (with m_pDefB): body B changes when it breaks
		PhysicsDef* m_pDefB;  // 0x28
		TINT        m_iUnk2C; // 0x2C
		TFLOAT      m_fB1;    // 0x30
		TFLOAT      m_fB2;    // 0x34
		TFLOAT      m_fB3;    // 0x38
		TFLOAT      m_fB4;    // 0x3C
	};

	// A constraint between the bodies of two physics definitions (retail's element of PhysicsRecord::m_ppConstraints)
	struct ConstraintDef
	{
		TINT                m_iType;     // 0x00 7: ball and socket, 8: hinge, 9: limited hinge, 10: stiff spring
		TINT                m_iUnk04;    // 0x04
		TINT                m_iUnk08;    // 0x08
		ConstraintModifier* m_pModifier; // 0x0C
		PhysicsDef*         m_pDefA;     // 0x10
		TINT                m_iUnk14;    // 0x14
		PhysicsDef*         m_pDefB;     // 0x18
		TINT                m_iUnk1C;    // 0x1C
		Toshi::TVector4*    m_pvPoint1;  // 0x20 the pivot (in the instance's space)
		Toshi::TVector4*    m_pvPoint2;  // 0x24 the second pivot or a point on the axis
		TFLOAT              m_fMaxAngle; // 0x28
		TFLOAT              m_fMinAngle; // 0x2C
		TFLOAT              m_fFriction; // 0x30
	};

	// The physics record of a cell instance (retail's m_pUnk28 of a CellInstance)
	struct PhysicsRecord
	{
		TUINT32         m_uiUnk00;         // 0x00
		TINT            m_iNumConstraints; // 0x04
		PhysicsDef**    m_ppDef;           // 0x08 points to the definition
		ConstraintDef** m_ppConstraints;   // 0x0C
	};

public:
	ACollisionInstance2();

	~ACollisionInstance2();

	void SetModelInstance( Toshi::TModelInstance* a_pModelInstance, OwnerObject* a_pOwner );

	void SetMatrix( const Toshi::TMatrix44& a_rMatrix );

	void Create( Toshi::TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, TBOOL a_bPrimitives, TINT a_iNumShapeTypes, TINT* a_pShapeTypes );

	void AddToWorld( TUINT32 a_uiTypeMask );

	void RemoveFromWorld( TUINT32 a_uiTypeMask );
	void UpdateBodiesActivation( TBOOL a_bUnused );

	// The bodies get a keyframed mover each (the bodies of an animated instance follow their bones)
	void CreateKeyframeMovers();

	// Moves the Havok bodies to their matrices (and the bone-attached ones to their bones).
	void UpdateBodyTransforms();

	// Creates the bodies of a physics definition (the instance's m_pUnk28): a custom mesh body or the standard body
	// (dispatched by the type of its one shape).
	void CreatePhysicsObjects( Toshi::TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, PhysicsRecord* a_pRecord );

	// Creates the constraint of the record between the bodies of its two physics definitions (a_rMatrix: the matrix of
	// the instance, the constraint's points are in its space)
	void CreateConstraint( PhysicsRecord* a_pRecord, const Toshi::TMatrix44* a_pMatrix );

	// $deBlob: FUNCTION 005a5f40
	PhysicsObjectType GetPhysicsObjectType( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_iNumPhysicsObjects );
		Toshi::TPhysicsMeshContainer* pContainer = m_pPhysicsInfos[ a_iIndex ].m_pMeshContainer;
		return ( pContainer && pContainer->IsCustomMesh() == TTRUE ) ? PhysicsObjectType_Custom : PhysicsObjectType_Standard;
	}

	// Is a body of these types still waiting in the physics manager's add batch?
	// $deBlob: FUNCTION 00625520
	TBOOL IsBeingAdded( TUINT32 a_uiTypeMask ) const
	{
		for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
		{
			if ( ( a_uiTypeMask & GetPhysicsObjectType( i ) ) && m_pPhysicsInfos[ i ].m_pBody &&
			     ( m_pPhysicsInfos[ i ].m_pBody->m_flags & Toshi::TPhysicsObjectContainer::PMF_IN_ADD_BATCH ) )
				return TTRUE;
		}

		return TFALSE;
	}

	// Are all its bodies of these types that collide with something (a collision layer) in the simulation?
	// $deBlob: FUNCTION 0077d010
	TBOOL IsInSimulation( TUINT32 a_uiTypeMask ) const
	{
		for ( TINT i = 0; i < m_iNumPhysicsObjects; i++ )
		{
			if ( a_uiTypeMask & GetPhysicsObjectType( i ) )
			{
				Toshi::TPhysicsObjectContainer* pBody = m_pPhysicsInfos[ i ].m_pBody;

				if ( pBody && ( pBody->m_uiCollisionFilterInfo & 0xfff ) && !( pBody->m_flags & Toshi::TPhysicsObjectContainer::PMF_IN_SIMULATION ) )
					return TFALSE;
			}
		}

		return TTRUE;
	}

	TINT GetNumPhysicsObjects() const { return m_iNumPhysicsObjects; }
	void ClearFlags( TUINT32 a_uiFlags ) { m_uiFlags &= ~a_uiFlags; }
	// $deBlob: FUNCTION 005a6820
	void         SetModelInstance( Toshi::TModelInstance* a_pModelInstance ) { m_pModelInstance = a_pModelInstance; }
	PhysicsInfo& GetPhysicsInfo( TINT a_iIndex ) { return m_pPhysicsInfos[ a_iIndex ]; }

private:
	friend class ACollisionInstanceMover;

	void AllocatePhysicsInfos( TINT a_iNumPhysicsObjects );

	// The custom mesh body of a physics definition whose one shape is a custom mesh (type 6)
	void CreateCustomMeshBodies( Toshi::TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, PhysicsDef* a_pDef );

	// The standard body of a physics definition (one body with its list shapes, plus its mover)
	void CreateStandardBody( Toshi::TModelInstance* a_pModelInstance, const OwnerInfo* a_pOwnerInfo, PhysicsDef* a_pDef );

	// The body of a physics definition's body shape: a static, keyframed or dynamic body (nullptr for other types)
	static Toshi::TPhysicsObjectContainer* CreateDefBody( PhysicsDef* a_pDef, Toshi::TMatrix44* a_pMatrix, TBOOL a_bKeyframed );

	// Adds the list shapes of a physics definition to a body (sphere, box, capsule, cylinder)
	static void AddShapes( Toshi::TPhysicsObjectContainer* a_pBody, PhysicsDef* a_pDef );

	// The maximum linear and angular velocities of a body (retail's TPhysicsObjectContainer flags 0x10 on a_bUnk)
	static void SetMaxLinearVelocity( Toshi::TPhysicsObjectContainer* a_pBody, TFLOAT a_fVelocity, TBOOL a_bUnk );
	static void SetMaxAngularVelocity( Toshi::TPhysicsObjectContainer* a_pBody, TFLOAT a_fVelocity );

	// The collision filter layer/group and the surface of a collision material for an owner type
	void GetCollisionFilter( const TCHAR* a_szMaterial, TINT a_eOwnerType, TUINT32& a_ruiLayer, TUINT32& a_ruiGroup, TUINT32& a_ruiSurface );

	// The primitive shape (a_iShapeType 2: sphere, 3: box, 4: capsule, 5: cylinder) fitted to a collision mesh's
	// bounding box: a_pParams gets the half extents (sphere: [0] radius) or the height and radius, a_rTransform the
	// shape's transform in the model
	void GetPrimitiveShape( const Toshi::TModelCollisionMesh* a_pCollisionData, TINT a_iShapeType, TFLOAT* a_pParams, Toshi::TMatrix44& a_rTransform );

	TUINT32 GetOwnerFlags() const { return m_pOwner->m_uiFlags; }

private:
	Toshi::TModelInstance*              m_pModelInstance;     // 0x08
	OwnerObject*                        m_pOwner;             // 0x0c
	TINT                                m_iNumPhysicsObjects; // 0x10
	PhysicsInfo*                        m_pPhysicsInfos;      // 0x14
	TUINT32                             m_uiFlags;            // 0x18 Flags
	Toshi::TPhysicsConstraintContainer* m_pConstraint;        // 0x1c
	ACollisionInstanceResponder*        m_pResponder;         // 0x20 the responder of the body (deleted by the dtor)
};
