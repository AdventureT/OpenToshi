#pragma once
#include "Entities/AInvisibleEntity.h"
#include "Entities/ATriggerUtil.h"
#include "Collision/ACollisionInstance2.h"
#include <Physics/TPhysicsResponder.h>

TOSHI_NAMESPACE_START
class TPhysicsMotionBase;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: an invisible trigger volume that triggers its targets (plus ATriggerUtil's 6 at 0x84).
//-----------------------------------------------------------------------------
class APropTrigger
    : public AInvisibleEntity
    , public ATriggerUtil
{
public:
	TDECLARE_CLASS( APropTrigger, AInvisibleEntity );

	// m_uiTriggerFlags
	enum TRIGGERFLAG : TUINT16
	{
		TRIGGERFLAG_ResetOnExit          = 0x1,
		TRIGGERFLAG_TriggerOnce          = 0x2,
		TRIGGERFLAG_Pulse                = 0x4,
		TRIGGERFLAG_TargetableFromBelow  = 0x10,
		TRIGGERFLAG_UseSlamAttack        = 0x20,
		TRIGGERFLAG_TriggerFirstFrame    = 0x40,
		TRIGGERFLAG_Delayed              = 0x80,
		TRIGGERFLAG_Triggered            = 0x100,
		TRIGGERFLAG_Touched              = 0x200,
		TRIGGERFLAG_Pulsing              = 0x400,
		TRIGGERFLAG_TargetsLevelComplete = 0x800,
		TRIGGERFLAG_OnChallengeStart     = 0x1000,
		TRIGGERFLAG_OnChallengeComplete  = 0x2000,
		TRIGGERFLAG_OnChallengeReset     = 0x4000,
		TRIGGERFLAG_QuitChallenge        = 0x8000,
	};

public:
	// Receives the touch phantom's trigger events and sends them to the trigger
	class APropTriggerCollisionEventHandler
	    : public Toshi::TPhysicsResponder
	{
	public:
		APropTriggerCollisionEventHandler( APropTrigger* a_pTrigger )
		    : m_pTrigger( a_pTrigger ) {}

		// $deBlob: FUNCTION 0079bf70
		// $deBlob: FUNCTION 0079bea0
		virtual ~APropTriggerCollisionEventHandler() override = default;

		virtual void OnTriggerEnter( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override; // +0x20
		virtual void OnTriggerLeave( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override; // +0x24

	public:
		APropTrigger* m_pTrigger; // 0x04
	};

	// The touch phantom (0x94 bytes)
	struct PhysicsData
	{
		Toshi::TMatrix44                   m_oMatrix;        // 0x00 the body's matrix (the bottom of the box)
		TUINT8                             m_aUnk40[ 0x40 ]; // 0x40
		ACollisionInstance2::OwnerInfo     m_oOwnerInfo;     // 0x80 the body's user data { 0, this }
		Toshi::TPhysicsObjectContainer*    m_pBody;          // 0x88
		APropTriggerCollisionEventHandler* m_pHandler;       // 0x8C
		Toshi::TPhysicsMotionBase*         m_pMover;         // 0x90 (never set by APropTrigger itself)
	};

	// (index, squared distance) of a target, sorted by distance for the pulse
	struct TargetDistance
	{
		TINT   m_iIndex;  // 0x00 entity targets first, then instances
		TFLOAT m_fDistSq; // 0x04
	};

	// A waypoint node whose flag the trigger toggles (0xC bytes)
	struct WaypointSet
	{
		TUINT32 m_uiSet;    // 0x00
		TUINT32 m_uiNode;   // 0x04
		TUINT8  m_bToggled; // 0x08
	};

public:
	APropTrigger();
	virtual ~APropTrigger() override; // +0x08

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void  PostCreate() override;                                                               // +0x20 (also ATriggerUtil +0x08)
	virtual void  ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C (also ATriggerUtil +0x04)
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void  SetBBox( Toshi::TVector4 a_vBBox ) override;                                         // +0x7C
	virtual void  AddToPhysics() override;                                                             // +0x88
	virtual void  RemoveFromPhysics() override;                                                        // +0x8C
	virtual TBOOL IsTargetable() override;                                                             // +0xA0
	// $deBlob: FUNCTION 007626f0
	virtual TBOOL CanBeTargetedFromBelow() override { return ( m_uiTriggerFlags >> 4 ) & 1; } // +0xA4
	virtual void  Reset( ABaseEntity* a_pEntity ) override;                                   // +0xC8
	// Can the object set the trigger off (Blob)
	virtual TBOOL CanObjectTrigger( Toshi::TObject* a_pObject ); // +0xE0
	virtual void  Trigger();                                     // +0xE4
	virtual void  UntoggleNodes();                               // +0xE8 untoggles the waypoint nodes

	// The message sent to the targets for the "MessageID" param (0-4; others keep it)
	// $deBlob: FUNCTION 0079bc50
	void SetMessageID( TINT a_iMessageID )
	{
		switch ( a_iMessageID )
		{
			case 0: m_uiMessage = AEntityIOPacket::MESSAGE_TRIGGER; break;
			case 1: m_uiMessage = AEntityIOPacket::MESSAGE_STOP; break;
			case 2: m_uiMessage = AEntityIOPacket::MESSAGE_ENTER; break;
			case 3: m_uiMessage = AEntityIOPacket::MESSAGE_DESPAWN; break;
			case 4: m_uiMessage = AEntityIOPacket::MESSAGE_RESET; break;
		}
	}

	// One of the targets is an APropTriggerLevelComplete (PostCreate)
	TBOOL  TargetsLevelComplete() const { return ( m_uiTriggerFlags & TRIGGERFLAG_TargetsLevelComplete ) != 0; }
	TINT16 GetPaintPoints() const { return m_iPaintPoints; }
	// $deBlob: FUNCTION 0079bfb0
	void SetParentTrigger( TUINT32 a_uiValue ) { m_uiParentTrigger = a_uiValue; }

protected:
	// Creates the touch phantom: a fixed box body of half extents a_pBBox, a_bAdd: adds it to the simulation
	void CreatePhysics( const Toshi::TMatrix44* a_pMatrix, const Toshi::TVector4* a_pBBox, TBOOL a_bAdd );
	void DestroyPhysics();
	// Toggles the waypoint nodes that aren't toggled yet
	void ToggleWaypointSets();
	void SendMessageAndToggleWaypoints( AEntityIOPacket::MESSAGE a_eMessage );

	// ATriggerUtil
	// $deBlob: FUNCTION 00762740
	virtual AEntityIOPacket::MESSAGE GetMessageToSend() override { return AEntityIOPacket::MESSAGE( m_uiMessage ); } // +0x10
	// $deBlob: FUNCTION 00762750
	virtual TINT GetTriggerData() override { return m_iTriggerData; } // +0x14

protected:
	TFLOAT          m_fDelay;             // 0x94 "Delay"
	TFLOAT          m_fTimer;             // 0x98 the delay countdown, the pulse time
	TUINT16         m_uiTriggerFlags;     // 0x9C
	TUINT16         m_uiNextPulseTarget;  // 0x9E the next target of the pulse
	TUINT8          m_uiNumWaypointSets;  // 0xA0 the entity data's waypoint set targets
	TUINT8          m_uiMessage;          // 0xA1 1
	TINT16          m_iPaintPoints;       // 0xA2 -1, "Paint Points Required"
	TUINT32         m_uiTriggeredBy;      // 0xA4 the ABaseEntity that triggered it (without delay)
	WaypointSet*    m_pWaypointSets;      // 0xA8
	PhysicsData*    m_pPhysicsData;       // 0xAC the touch phantom
	TFLOAT          m_fPulseSpeed;        // 0xB0 "Pulse Speed"
	TargetDistance* m_pTargetsByDistance; // 0xB4 (flag 4)
	TINT            m_iTriggerData;       // 0xB8
	TUINT32         m_uiParentTrigger;    // 0xBC the APropTrigger that targets this one (PostCreate)
};
