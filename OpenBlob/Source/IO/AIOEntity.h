#pragma once
#include "Entities/ABaseEntity.h"
#include "IO/AIOEntityComponent.h"
#include "IO/IIOMessageUpdater.h"
#include "FXComposition/AFXCompositionInstance.h"

class AFXBreakpointComponent;
class IOCallbacks;

namespace Toshi
{
class TTRB;
}

//-----------------------------------------------------------------------------
// Purpose: the IO entity, the base of the entities that run the level's IO. Its IO components are AIOEntityComponent,
// its timers are the lists and arrays at +0xD4..+0xE0 (the ActionDelay and InputCooldown structs).
// NOTE: the layout is from the retail ctor, dtor, Create, PostCreate and Tick.
//-----------------------------------------------------------------------------
class AIOEntity
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( AIOEntity, ABaseEntity );

	// A delayed action of the IO data (0x14 bytes): one per action with a delay (SetupDelaysAndCooldowns)
	struct ActionDelay
	{
		// $deBlob: FUNCTION 007866a0
		ActionDelay()
		{
			m_fTimer  = 0.0f;
			m_pAction = TNULL;
			m_pInput  = TNULL;
			m_pOther  = TNULL;
		}

		// $deBlob: FUNCTION 007868d0
		// $deBlob: FUNCTION 00786960
		virtual ~ActionDelay() {} // +0x00

		TFLOAT m_fTimer;  // 0x04
		void*  m_pAction; // 0x08 the object
		void*  m_pInput;  // 0x0C its args
		void*  m_pOther;  // 0x10
	};

	// The cooldown of an input of the IO data (0xc bytes): one per input with a cooldown
	struct InputCooldown
	{
		// $deBlob: FUNCTION 007868b0
		InputCooldown()
		{
			m_pInput = TNULL;
			m_fTimer = 0.0f;
		}

		// $deBlob: FUNCTION 00786950
		// $deBlob: FUNCTION 007869c0
		virtual ~InputCooldown() {} // +0x00

		void*  m_pInput; // 0x04
		TFLOAT m_fTimer; // 0x08
	};

	// An FX composition of the IO data (0xfc bytes, m_pF8List): one per FX id (SetupFXCompositions)
	struct FXComposition
	{
		FXComposition()
		{
			m_iID             = 0;
			m_szName          = TNULL;
			m_bFollowsMatrix  = TFALSE;
			m_pFollowedEntity = TNULL;
			m_pNext           = TNULL;
		}

		TINT                   m_iID;             // 0x00
		const TCHAR*           m_szName;          // 0x04
		AFXCompositionInstance m_oInstance;       // 0x08
		TBOOL                  m_bFollowsMatrix;  // 0xF0 the instance follows m_pFollowedEntity's matrix
		void*                  m_pFollowedEntity; // 0xF4 has a TMatrix44 at +0x10 (an entity's matrix)
		FXComposition*         m_pNext;           // 0xF8
	};

	// A target of a target list (8 bytes): the entity ID from the data, the entity PostCreate looks up
	struct Target
	{
		Target()
		{
			iID     = -1;
			pEntity = TNULL;
		}

		TINT         iID;     // 0x00
		ABaseEntity* pEntity; // 0x04
	};

	// A target list of the entity (0x10 bytes, m_pC4Items), copied from the data's TargetData by SetupTargets
	struct TargetList
	{
		TINT    iUnk00;      // 0x00
		TINT    iUnk04;      // 0x04
		Target* pTargets;    // 0x08
		TINT    iNumTargets; // 0x0C
	};

	// The data's target list (0x10 bytes): the entity IDs are TINTs
	struct TargetData
	{
		TINT  iUnk00;  // 0x00
		TINT  iUnk04;  // 0x04
		TINT* pIDs;    // 0x08
		TINT  iNumIDs; // 0x0C
	};

	// The data of an IO component (8 bytes): its type (AIOMeshComponent::TYPE...) and its own data, given to the
	// component's HandleEntityIO (SetIOEntityData)
	struct ComponentData
	{
		TINT  iType; // 0x00
		void* pData; // 0x04
	};

	// The data of an IO entity (SetIOEntityData)
	struct IOEntityData
	{
		TINT                   iID;            // 0x00
		const TCHAR*           szName;         // 0x04
		const Toshi::TVector4* pPosition;      // 0x08
		const Toshi::TVector4* pRotation;      // 0x0C a quaternion
		const TCHAR*           szTemplate;     // 0x10 the IO template (AIOTemplateManager::GetIOTemplateReference), TNULL for none
		TargetData*            pTargets;       // 0x14
		TUINT16                uiNumTargets;   // 0x18
		ComponentData*         pComponents;    // 0x1C
		TINT                   iNumComponents; // 0x20
	};

	// An action of an input of the IO data (0xC bytes): run at once, or after its delay (ActionDelay)
	// NOTE: the IO data isn't typed in the older code of this class (raw offsets)
	struct IOAction
	{
		TINT   iType;   // 0x00 the callback (sm_aIOCallbacks) or a component's action
		TFLOAT fDelay;  // 0x04 > 0: delayed
		void*  pParams; // 0x08
	};

	// An input of the IO data (0x1C bytes): the message that runs its actions
	struct IOInput
	{
		TINT      iMessage;        // 0x00 the message id (HandleInputMessage)
		TINT      iNumActions;     // 0x04
		IOAction* pActions;        // 0x08
		TFLOAT    fCooldown;       // 0x0C > 0: the input is ignored for that time once it ran (InputCooldown)
		TINT      iCondition;      // 0x10 1: m_uiFlags' bits 0..3 must match iConditionValue
		TINT      iConditionValue; // 0x14 0..14
		TINT*     pParams;         // 0x18 the messages 0x0, 0x14..0x16 and 0x30 read it
	};

	// The parameters of the messages 0 (the IO's start) and 0x30 (Reset), the input's pParams
	struct StartParams
	{
		TINT   iHitPoints; // 0x00 read as a TINT16
		TINT   iCounter;   // 0x04 read as a TINT16
		TFLOAT fHeartBeat; // 0x08
		TINT   bVisible;   // 0x0C
		TINT   bActive;    // 0x10
		TINT   iRed;       // 0x14 the colour (message 0 only)
		TINT   iGreen;     // 0x18
		TINT   iBlue;      // 0x1C
	};

	// IO entity flags (m_uiFlags)
	// emitted in AIOMeshComponent.cpp
	// $deBlob: FUNCTION 007cd260
	TBOOL IsIOFlagSet( TUINT32 a_uiFlag ) const
	{
		if ( m_uiFlags & a_uiFlag )
			return TTRUE;

		return TFALSE;
	}

	const Toshi::TMatrix44& GetLocalMatrix() const { return m_oLocalMatrix; }

public:
	friend class IOCallbacks;
	friend class AGameStatus;

	AIOEntity();
	virtual ~AIOEntity() override; // +0x08

	virtual void              Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void              OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void              Create() override;                                                                   // +0x1C
	virtual void              PostCreate() override;                                                               // +0x20
	virtual void              SetVisible( TBOOL a_bVisible ) override;                                             // +0x38
	virtual void              SetPosition( const Toshi::TVector4& a_rPosition ) override;                          // +0x40
	virtual void              SetMatrix( const Toshi::TMatrix44& a_rMatrix ) override;                             // +0x48
	virtual void*             GetModelComponent() override;                                                        // +0x4C AEntityModelComponent
	virtual void              HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual Toshi::TVector4   GetReactionForce( void* a_pUnk1, void* a_pUnk2 ) override;                           // +0x60
	virtual Toshi::TVector4   GetReactionRotation( void* a_pUnk ) override;                                        // +0x64
	virtual void              PreRenderUpdate() override;                                                          // +0x6C
	virtual ACollisionObject* GetCollisionObject() override;                                                       // +0x70
	virtual Toshi::TVector4   GetBasePosition() const override;                                                    // +0x80

	// $deBlob: FUNCTION 00786c40
	virtual TBOOL IsInteractable() override { return ( m_uiFlags & 0x80 ) != 0; } // +0x94

	// $deBlob: FUNCTION 00786c60
	virtual TBOOL IsTargetable() override { return ( m_uiFlags & 0x40 ) != 0; } // +0xA0

	virtual void Reset( ABaseEntity* a_pEntity ) override; // +0xC8
	virtual void SetActive( TBOOL a_bActive ) override;    // +0xCC

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	// $deBlob: FUNCTION 00786bd0
	virtual AFXBreakpointComponent* GetFXBreakpointComponent() { return m_pD0Component; } // +0xDC

	// $deBlob: FUNCTION 00786be0
	TINT GetHitPoints() const { return m_iHitPoints; }
	// The callbacks of the IO actions, by action type (0x13C bytes), filled by InitialiseClass; a type without one is
	// executed by the components (ExecuteActionInvoker)
	using t_IOCallback                         = void( __stdcall* )( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );
	static TCONSTEXPR_VAR TINT NUM_IOCALLBACKS = 0x13C / sizeof( t_IOCallback );
	static t_IOCallback        sm_aIOCallbacks[ NUM_IOCALLBACKS ];

	// Sets the entity up from its data: ID, name, IO template, components, matrix, targets and the FX breakpoints of
	// its model
	void SetIOEntityData( const IOEntityData& a_rData, Toshi::TTRB* a_pTRB );

	// Sends a message to the IO: the components' ExecuteActionInvoker (+0x10) when the entity has no IO data, otherwise
	// the IO data's items with the same id. a_pMessage is the message id (its first int)
	void HandleInputMessage( TINT* a_pMessage, ABaseEntity* a_pOther );

	// Sends the IO the message a_iMessage (no parameters)
	void SendInputMessage( TINT a_iMessage, ABaseEntity* a_pOther );

	// The target list of the action a_pAction of the input a_pInput (the IO data's), TNULL if none
	TargetList* FindTargetList( void* a_pInput, void* a_pAction );

	// The target a_iIndex of the action a_pAction of the input a_pInput, TNULL if none
	ABaseEntity* GetMessageTarget( void* a_pInput, void* a_pAction, TINT a_iIndex );

	// Sets the IO counter and sends the input messages 0x14, 0x16 and 0x15 (the counter conditions)
	void SetCounter( TINT a_iCounter );

	// Triggers the FX compositions with the id a_iID at a_pAt's matrix
	void FXSpawn( TINT a_iID, ABaseEntity* a_pAt );

	// Stops the FX compositions with the id a_iID
	void FXStop( TINT a_iID, TBOOL a_bInstant );

	// The first component of the type a_iType in m_pFCList
	AIOEntityComponent* FindComponent( TINT a_iType );
	// Sets or clears bits of m_uiFlags, and tests them
	void  SetIOFlags( TUINT32 a_uiFlags, TBOOL a_bSet );
	TBOOL HasIOFlags( TUINT32 a_uiFlags ) const;
	// Pushes a_pUpdater to the front of m_p100List
	void AddMessageUpdater( IIOMessageUpdater* a_pUpdater );

	// The IO actions that run over time (an updater) or at once (a duration of 0)
	void Rotate( IOAction* a_pAction );
	void Move( IOAction* a_pAction, IOInput* a_pInput );
	TNOINLINE void StopRotations();
	void StartShake( IOAction* a_pAction, ABaseEntity* a_pShaker );
	void FadeMesh( IOAction* a_pAction );
	void Scale( IOAction* a_pAction );

	// The alpha of the model (its forced fade)
	void SetAlpha( TFLOAT a_fAlpha );

	// The scale of the model and the bounding box (a_bScaleCollision: grown to the scaled collision too)
	void SetScale( const Toshi::TVector4& a_rScale, TBOOL a_bScaleCollision );

	// Plays the animation of the action a_pAction (the anim IDs, m_pCCList)
	void PlayAnim( IOAction* a_pAction );

	// Hides the entity, takes it out of the hash and calls its RemoveFromPhysics (the IO action 0x18)
	void Destroy();

private:
	// Whether the flags 0x1/0x2/0x4/0x8 of m_uiFlags of the combination a_iCombination are all set (0-3: one flag,
	// 4-14: the combinations of two to four)
	TBOOL AreFlagsSet( TINT a_iCombination ) const;

	TBOOL CanExecuteActionInvoker( IOInput* a_pInput, ABaseEntity* a_pOther );
	void  ExecuteActionInvoker( IOInput* a_pInput, ABaseEntity* a_pOther );
	void  ApplyStartParams( const StartParams* a_pParams );
	void  ApplyResetParams( const StartParams* a_pParams );

	// Grows the bounding box to the box of the collision (its matrix a_rMatrix, scaled by a_rScale)
	void GrowBBox( const Toshi::TMatrix44& a_rMatrix, const Toshi::TVector4& a_rScale );

private:
	void SetupProjectilePools();
	void SetupAnimIDs();
	void SetupDelaysAndCooldowns();
	void SetupAttackableInteractFlags();
	void SetupFXCompositions();
	void SetupIOEntityComponents( const IOEntityData& a_rData, Toshi::TTRB* a_pTRB );
	void SetupTargets( const IOEntityData& a_rData );
	void SetupUpdateOverTimeMessages();

protected:
	Toshi::TMatrix44        m_oLocalMatrix;    // 0x80 identity at construction
	void*                   m_pIOData;         // 0xC0 Create runs the setup helpers only when this is set
	TargetList*             m_pC4Items;        // 0xC4 the target lists (SetupTargets)
	TUINT16                 m_uiC4Count;       // 0xC8
	TUINT16                 m_uiPadC8;         // 0xCA
	void*                   m_pCCList;         // 0xCC list of the anim IDs (0xc bytes: the ID (TINT16), the action, the next)
	AFXBreakpointComponent* m_pD0Component;    // 0xD0 the FX breakpoints of the entity's model (SetIOEntityData)
	TINT                    m_iD4Count;        // 0xD4 count of the 20-byte timer items at 0xD8
	ActionDelay*            m_pD8Items;        // 0xD8 20-byte items: a timer (+4), the object (+8) and its args (+12, +16)
	TINT                    m_iDCCount;        // 0xDC count of the 12-byte timer items at 0xE0
	InputCooldown*          m_pE0Items;        // 0xE0 12-byte items: a timer (+8)
	TUINT32                 m_uiFlags;         // 0xE4 0x10 ticks the timers at 0xDC/0xE0, 0x100 is set at construction
	TUINT32                 m_uiColour;        // 0xE8 0xFFFFFF00 at construction, set by the message 0
	TFLOAT                  m_fHeartBeat;      // 0xEC the interval of the heart beat message (0x32), 0: none
	TFLOAT                  m_fHeartBeatTimer; // 0xF0
	TINT16                  m_iHitPoints;      // 0xF4 the damage messages lower it, 0x2E at 0
	TINT16                  m_iCounter;        // 0xF6 the IO counter (SetCounter)
	FXComposition*          m_pF8List;         // 0xF8 list of FXComposition (next at +248)
	AIOEntityComponent*     m_pFCList;         // 0xFC list of the components (next at +12, type at +4)
	IIOMessageUpdater*      m_p100List;        // 0x100 the updaters (next at +0x18), the active ones are ticked
	ABaseEntity*            m_pLastTarget;     // 0x104 the target the last action picked (target mode 4)
};                                             // 0x108

//-----------------------------------------------------------------------------
// Purpose: the callbacks of the IO actions (AIOEntity::sm_aIOCallbacks, set by AIOEntity::InitialiseClass), their type
// in brackets. The target mode (most actions' first parameter): 0 the entity, 1 the action's target list, 2 the other
// entity of the message, 4 the entity's last target.
// AIOEntity.o has them before AIOEntity's functions, de Blob's are after them
//-----------------------------------------------------------------------------
class IOCallbacks
{
public:
	static void __stdcall OnShake( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );          // 0x27
	static void __stdcall OnActivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );       // 0x2A
	static void __stdcall OnDeactivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );     // 0x2B
	static void __stdcall OnDamage( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );         // 0x26
	static void __stdcall OnSetVisible( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );     // 0x19
	static void __stdcall OnScreenFadeOut( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );  // 0x1F
	static void __stdcall OnScreenFadeIn( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );   // 0x20
	static void __stdcall RelayMessage( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );     // 0x0C..0x0F, 0x4A..0x4D
	static void __stdcall OnSetHeartBeat( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );   // 0x31
	static void __stdcall OnReset( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );          // 0x2F
	static void __stdcall OnSetHitPoints( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );   // 0x2D
	static void __stdcall OnEndLevel( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );       // 0x24
	static void __stdcall OnTeleportCaller( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther ); // 0x23
	static void __stdcall OnAutosave( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );       // 0x22
	static void __stdcall OnPlayAnim( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );       // 0x1B
	static void __stdcall OnDestroy( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );        // 0x18
	static void __stdcall OnSetFlag( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );        // 0x17
	static void __stdcall OnModifyCounter( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );  // 0x13
	static void __stdcall OnSetCounter( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );     // 0x12
	static void __stdcall OnKnockBack( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );      // 0x35
	static void __stdcall OnBounceCaller( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );   // 0x37
	static void __stdcall OnFXSpawn( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );        // 0x1C
	static void __stdcall OnFXStop( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );         // 0x1D
	static void __stdcall OnRotate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );         // 0x3B
	static void __stdcall OnMove( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );           // 0x3C
	static void __stdcall OnSetModelFlag( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );   // 0x3D
	static void __stdcall OnFadeMesh( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );       // 0x3E
	static void __stdcall OnScaleEntity( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );    // 0x3F
	static void __stdcall OnFireProjectile( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther ); // 0x41
	static void __stdcall OnSpawnObject( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );    // 0x43
	static void __stdcall OnTriggerTargets( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther ); // 0x44..0x47
	static void __stdcall OnRotationStop( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther );   // 0x48

	// 0x29: empty in retail
	// $deBlob: FUNCTION 007ca000
	static void __stdcall OnFindNewTarget( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther ) {}
};
