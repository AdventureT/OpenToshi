#pragma once
#include "Entities/ABaseEntity.h"
#include "Entities/ATriggerUtil.h"

#include <Toshi/T2DList.h>
#include "Fx/Shadows/AShadow.h"
#include "Collision/ACollisionFunctor.h"

class ACollisionObject;

class APickup;

//-----------------------------------------------------------------------------
// Purpose: the collision functor of a dropping pickup (8 bytes): the entity that dropped it (m_pIgnoredEntity) is
// ignored.
//-----------------------------------------------------------------------------
struct APickupCollisionFunctor
    : public ACollisionFunctor
{
	APickupCollisionFunctor( APickup* a_pPickup )
	    : m_pPickup( a_pPickup )
	{}

	// $deBlob: FUNCTION 007b9ef0
	// $deBlob: FUNCTION 007b9e20
	virtual ~APickupCollisionFunctor() override = default; // +0x20

	// $deBlob: FUNCTION 007b9f60
	virtual TBOOL HitNonSolidObject( ACollisionObject* a_pObject ) override { return TFALSE; } // +0x10
	virtual TBOOL HitIgnorableObject( ACollisionObject* a_pObject ) override;                  // +0x18

	APickup* m_pPickup; // 0x04
};

//-----------------------------------------------------------------------------
// Purpose: the base of the collectable pickups (plus ATriggerUtil's 6 at 0x80): a mesh drawn by AInstanceRenderer, a
// shadow, a collision object, a life time (flashing before it goes), a respawn time and a drop flight; kept in
// APickupManager's list.
//-----------------------------------------------------------------------------
class APickup
    : public ABaseEntity
    , public ATriggerUtil
{
public:
	TDECLARE_CLASS( APickup, ABaseEntity );

	enum PICKUPSTATE : TUINT8
	{
		PICKUPSTATE_Idle       = 0,
		PICKUPSTATE_Collected  = 1,
		PICKUPSTATE_Dropping   = 2,
		PICKUPSTATE_Fading     = 3,
		PICKUPSTATE_Respawning = 4,
		PICKUPSTATE_Flashing   = 5,
		PICKUPSTATE_Hidden     = 6,
	};

	enum PICKUPFLAG : TUINT8
	{
		PICKUPFLAG_NoMesh      = 0x10, // ProcessParams doesn't read "Mesh"/"Mesh2"
		PICKUPFLAG_DropTimeout = 0x20, // the drop flight ends after m_fDropTime
	};

	// APickupManager's list node (GetListNode)
	struct ListNode : public Toshi::T2GenericDList::Node
	{
		APickup* m_pOwner; // 0x08
	};

	using Callback = void ( * )( APickup* a_pPickup, void* a_pUserData );

public:
	APickup( TUINT8 a_uiPickupType, TINT a_iUnk2, TFLOAT a_fUnk3, TBOOL a_bUnk4 );
	virtual ~APickup() override; // +0x08

	virtual void              Tick( TFLOAT a_fDeltaTime ) override;                                // +0x0C
	virtual void              OnLODChanging( TINT a_iLOD ) override;                               // +0x10
	virtual void              Create() override;                                                   // +0x1C
	virtual void              PostCreate() override;                                               // +0x20 (also ATriggerUtil +0x08)
	virtual void              OnDataLoad() override;                                               // +0x28
	virtual void              ProcessParams( const AEntityParams& a_rParams ) override;            // +0x2C (also ATriggerUtil +0x04)
	virtual void              InitPooled( TINT a_eType ) override;                                 // +0x30
	virtual void              SetVisible( TBOOL a_bVisible ) override;                             // +0x38
	virtual void              SetPosition( const Toshi::TVector4& a_rPosition ) override;          // +0x40
	virtual void*             GetModelComponent() override;                                        // +0x4C
	virtual void              PreRenderUpdate() override;                                          // +0x6C
	virtual ACollisionObject* GetCollisionObject() override;                                       // +0x70
	virtual void              SetBBox( Toshi::TVector4 a_vBBox ) override;                         // +0x7C
	virtual Toshi::TVector4   GetBasePosition() const override;                                    // +0x80
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                            // +0xC8
	virtual void              OnPoolRelease() override;                                            // +0xD0
	virtual void              OnCollected( ABaseEntity* a_pCollector );                            // +0xDC
	virtual void              OnDropFinished();                                                    // +0xE0
	virtual void              Show();                                                              // +0xE4
	virtual void              Hide();                                                              // +0xE8
	virtual void              Drop( ABaseEntity* a_pDropper, const Toshi::TVector4& a_rVelocity ); // +0xEC
	virtual void              UpdateDrop( TFLOAT a_fDeltaTime );                                   // +0xF0

	// Puts the pooled pickup at a_rPosition and adds it to APickupManager: a_fHiddenTime > 0 starts it in
	// PICKUPSTATE_Hidden (hidden) for that long, otherwise it's shown; a_fLifeTime > 0: it disappears after that long
	void Spawn( const Toshi::TVector4& a_rPosition, TFLOAT a_fHiddenTime, TFLOAT a_fLifeTime );

	// Drops the pickup with a_rVelocity, the flight ends after a_fDropTime (<= 0: APickupData's drop time)
	void DropTimed( const Toshi::TVector4& a_rVelocity, TFLOAT a_fDropTime, ABaseEntity* a_pIgnored );

	// Throws the pickup from a_rFrom so that it lands on a_rTo (a_fAngle: the launch angle, outside [0, PI/2]: half
	// way between the elevation and straight up); nothing when they're closer than 0.25
	// a_iUnk isn't used
	void Fly( const Toshi::TVector4& a_rFrom, const Toshi::TVector4& a_rTo, TFLOAT a_fAngle, TINT a_iUnk );

	// $deBlob: FUNCTION 0075c640
	void SetPooled( void* a_pPooled ) { m_pPooled = a_pPooled; }

	void SetCallback( Callback a_fnCallback, void* a_pUserData )
	{
		m_fnCallback        = a_fnCallback;
		m_pCallbackUserData = a_pUserData;
	}

	// $deBlob: FUNCTION 0075c5b0
	ListNode& GetListNode() { return m_oListNode; }
	// $deBlob: FUNCTION 0075c5c0
	TUINT8 GetPickupType() const { return m_uiPickupType; }
	// Creates the collision object (if needed) and shows the pickup
	void CreateCollision();
	// Hides the pickup and waits to respawn (m_fRespawnTime > 0) or removes it (RemoveFromPhysics)
	void Despawn();

	PICKUPSTATE GetPickupState() const { return m_eState; } // 0x157
	// $deBlob: FUNCTION 007b9f30
	ABaseEntity* GetIgnoredEntity() const { return m_pIgnoredEntity; }

private:
	void UpdateLifeTime( TFLOAT a_fDeltaTime );
	void Render();

private:
	static TINT   ms_iNextRotationIndex; // 0-3
	static TFLOAT ms_fUnk9866D4;         // a squared distance (OnDataLoad)
	static TFLOAT ms_fShadowSize;

protected:
	Toshi::TVector4   m_vVelocity;         // 0x90
	Toshi::TVector4   m_vSpawnPosition;    // 0xA0
	AShadow           m_oShadow;           // 0xB0
	TFLOAT            m_fRespawnTime;      // 0x110 -1
	TFLOAT            m_fRespawnTimer;     // 0x114
	TFLOAT            m_fLifeTime;         // 0x118 -1
	TFLOAT            m_fAlpha;            // 0x11C 1
	ListNode          m_oListNode;         // 0x120
	ABaseEntity*      m_pIgnoredEntity;    // 0x12C ignored by the drop's collision query
	ACollisionObject* m_pCollision;        // 0x130
	TUINT32           m_uiUnk134;          // 0x134
	TUINT32           m_uiUnk138;          // 0x138
	TFLOAT            m_fDropTime;         // 0x13C
	TFLOAT            m_fDropDuration;     // 0x140
	TFLOAT            m_fHiddenTime;       // 0x144 PICKUPSTATE_Hidden timer
	void*             m_pPooled;           // 0x148
	Callback          m_fnCallback;        // 0x14C called once by Hide or OnPoolRelease
	void*             m_pCallbackUserData; // 0x150
	TINT8             m_iMesh;             // 0x154 AInstanceRenderer mesh, -1
	TINT8             m_iMesh2;            // 0x155 -1
	TUINT8            m_uiPickupType;      // 0x156
	PICKUPSTATE       m_eState;            // 0x157
	TUINT8            m_uiRotationIndex;   // 0x158 0-3
	TUINT8            m_uiPickupFlags;     // 0x159 PICKUPFLAG
};
