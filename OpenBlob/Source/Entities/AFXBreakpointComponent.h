#pragma once
#include "Entities/AEntityComponent2.h"

class AModelInstance2;
class AFXCompositionInstance;

#include <Math/TMatrix44.h>
#include <Core/TMemory.h>

//-----------------------------------------------------------------------------
// Purpose: plays the FX compositions of a model's animation breakpoints. The breakpoints come from
// data/entities/FXBreakPoints.trb (AEntityData, symbol "Main"), one entry per model file.
//-----------------------------------------------------------------------------
class AFXBreakpointComponent
    : public AEntityComponent2
{
public:
	// A breakpoint (0x14 bytes)
	struct FX
	{
		const TCHAR* m_szComposition; // 0x00 AFXCompositionInstance::Initialize
		TINT16       m_iAnimationID;  // 0x04 its group's
		TINT16       m_iIndex;        // 0x06 in m_pCompositions
		TFLOAT       m_fTime;         // 0x08
		TINT         m_iBone;         // 0x0C -1: the owner's/model's transform
		TUINT16      m_uiFlags;       // 0x10 1: bone attached, 2, 4: identity matrix, 8
		TINT16       m_iVariant;      // 0x12
	};

	// The breakpoints of an animation (0x10 bytes)
	struct Group
	{
		TINT         m_iNumFX;      // 0x00
		FX*          m_pFX;         // 0x04
		TINT         animationID;   // 0x08 resolved by Create
		const TCHAR* m_szAnimation; // 0x0C
	};

	// The groups of a model file (0xC bytes)
	struct Groups
	{
		TINT         m_iNumGroups; // 0x00
		Group*       groups;       // 0x04
		const TCHAR* m_szModel;    // 0x08 file name without the directory
	};

	// The "Main" symbol
	struct Data
	{
		TINT    m_iNumModels; // 0x00
		Groups* m_pModels;    // 0x04
	};

	static TCONSTEXPR_VAR TINT MAX_ACTIVE_FX   = 5;
	static TCONSTEXPR_VAR TINT MAX_ATTACHED_FX = 16;

public:
	AFXBreakpointComponent( ABaseEntity* a_pOwner );
	~AFXBreakpointComponent();

	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04
	virtual void ResetComponent() override;            // +0x08
	virtual void SetVisible( TBOOL a_bVisible );       // +0x14

	// Creates the component of a model (a_szModelName may have a directory) if FXBreakPoints.trb has breakpoints
	// for it; a_pModelInstance is used when there's no owner
	static AFXBreakpointComponent* Create( const TCHAR* a_szModelName, TMemoryHeap* a_pHeap, ABaseEntity* a_pOwner, AModelInstance2* a_pModelInstance );

private:
	AModelInstance2* GetModelInstance();
	void             UpdateBoneAttachedFX();
	void             TriggerBreakpoint( FX* a_pFX );
	void             ComputeDecalMatrix( Toshi::TMatrix44& a_rMatrix );

private:
	struct AttachedFX
	{
		FX*                     m_pFX;          // 0x00
		AFXCompositionInstance* m_pComposition; // 0x04
	};

	Data*                   m_pData;                        // 0x14
	AFXCompositionInstance* m_pCompositions;                // 0x18 [ m_iNumCompositions ], one per FX (FX::m_iIndex)
	FX*                     m_apActiveFX[ MAX_ACTIVE_FX ];  // 0x1C
	AttachedFX              m_aAttached[ MAX_ATTACHED_FX ]; // 0x30
	TINT                    m_iNumCompositions;             // 0xB0
	Groups*                 m_pGroups;                      // 0xB4
	AModelInstance2*        m_pModelInstance;               // 0xB8
};                                                          // 0xBC
