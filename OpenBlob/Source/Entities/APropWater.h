#pragma once
#include "Entities/AProp.h"

#include <Sound/TSound.h>
#include <Platform/DX11/TWorldShader_DX11.h>

class AEntityModelComponent;

//-----------------------------------------------------------------------------
// Purpose: a body of water or ink that fills and drains when triggered (plus ATriggerUtil's 6 at 0x9C).
// NOTE: names from the entity params ("AllowFilling", "FillTime", "Mesh2"...)
//-----------------------------------------------------------------------------
class APropWater
    : public AProp
{
public:
	TDECLARE_CLASS( APropWater, AProp );

	enum WATERFLAG : TUINT16
	{
		WATERFLAG_AllowFilling     = 0x1,
		WATERFLAG_AllowDraining    = 0x2,
		WATERFLAG_Full             = 0x4, // not "Empty"
		WATERFLAG_ConvertToInk     = 0x8,
		WATERFLAG_ConvertFromInk   = 0x10,
		WATERFLAG_Ink              = 0x20, // "Ink": starts as ink
		WATERFLAG_IsInk            = 0x40, // the ink model (m_pMesh2) is the current one
		WATERFLAG_InkEmitter       = 0x80, // the ink sound emitter is registered
		WATERFLAG_WaterEmitter     = 0x100,
		WATERFLAG_EmptyWhenDrained = 0x200,
		WATERFLAG_StateMask        = 0xC00,  // STATE
		WATERFLAG_TransitionMask   = 0x3000, // TRANSITION
	};

	// ( m_uiWaterFlags & WATERFLAG_StateMask ) >> 10
	enum STATE
	{
		STATE_Full     = 0,
		STATE_Filling  = 1,
		STATE_Draining = 2,
		STATE_Empty    = 3,
	};

	// ( m_uiWaterFlags & WATERFLAG_TransitionMask ) >> 12: the water/ink cross fade
	enum TRANSITION
	{
		TRANSITION_ToInk   = 0,
		TRANSITION_ToWater = 1,
		TRANSITION_None    = 2,
	};

public:
	APropWater();
	virtual ~APropWater() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void OnLODChanging( TINT a_iLOD ) override;                                               // +0x10
	virtual void PostCreate() override;                                                               // +0x20
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void AddToPhysics() override;                                                             // +0x88
	virtual void RemoveFromPhysics() override;                                                        // +0x8C
	// $deBlob: FUNCTION 007a1840
	virtual TBOOL IsInteractable() override { return TTRUE; } // +0x94
	// $deBlob: FUNCTION 007a0320
	virtual TFLOAT GetInteractionRadius() const override { return m_vBBox.MagnitudeXZ(); } // +0x9C
	virtual void   Reset( ABaseEntity* a_pEntity ) override;                               // +0xC8
	virtual void   Despawn( TBOOL a_bPlayEffect ) override;                                // +0xE4

private:
	// Moves the collision bodies of the current model to a_rBodyMatrix and the entity to a_rEntityMatrix
	void SetWaterMatrices( const Toshi::TMatrix44& a_rBodyMatrix, const Toshi::TMatrix44& a_rEntityMatrix );
	// Swaps to the ink (a_bInk) or water model, cross fading through the transition materials when a_bTransition
	void SetInk( TBOOL a_bInk, TBOOL a_bTransition );
	// Triggered: converts water/ink and starts to fill or drain
	void Trigger();
	// Enables (a_bEnable) or disables the nav sets of the trigger
	void EnableNavSets( TBOOL a_bEnable );
	// The ambient sound emitter of the water or the ink
	void AddSoundEmitter();
	void RemoveSoundEmitter();
	// The fill/drain loop, pitched by the progress
	void UpdateSound();

	// The transition slot of a_uiOwner (a free one claimed for it when there's none: a_rbNew)
	static Toshi::TWorldShaderHAL::TransitionRef* GetTransition( TUINT32 a_uiOwner, TINT& a_riIndex, TBOOL& a_rbNew );

	static TBOOL ms_bTransitionMaterialsCreated;
	static TBOOL ms_bTransitionsCleared;

private:
	AEntityModelComponent* m_pMesh2;          // 0xB4 "Mesh2", the ink model (swapped with m_pModelComponent)
	FMOD::Event*           m_pSound;          // 0xB8 the fill/drain loop
	Toshi::TMatrix44       m_mBodyFull;       // 0xBC the collision bodies' matrix when full
	Toshi::TMatrix44       m_mEntityFull;     // 0xFC the entity's matrix when full
	TFLOAT                 m_fBodyEmptyY;     // 0x13C the bodies' y when empty (y points down)
	TFLOAT                 m_fEntityEmptyY;   // 0x140 the entity's y when empty ( full y + "WaterDepth" )
	TFLOAT                 m_fTime;           // 0x144 of the fill or drain
	TFLOAT                 m_fFillTime;       // 0x148 "FillTime"
	TFLOAT                 m_fDrainTime;      // 0x14C "DrainTime"
	TFLOAT                 m_fWaterDepth;     // 0x150 "WaterDepth"
	TFLOAT                 m_fDelayOnTrigger; // 0x154 "DelayOnTrigger"
	TFLOAT                 m_fDelay;          // 0x158
	TUINT16                m_uiWaterFlags;    // 0x15C WATERFLAG
	void*                  m_pTriggerCaller;  // 0x160 also the key of the TWorldShaderHAL transition slot
};
