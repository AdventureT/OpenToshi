#pragma once
#include "Entities/AVisibleEntity.h"
#include "Entities/ATriggerUtil.h"

TOSHI_NAMESPACE_START
class TAnimation;
TOSHI_NAMESPACE_END

class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: a visible entity that triggers targets (plus ATriggerUtil's 6 at 0x9C): a model ("Mesh"), a collision
// object, paintable or not.
//-----------------------------------------------------------------------------
class AProp
    : public AVisibleEntity
    , public ATriggerUtil
{
public:
	TDECLARE_CLASS( AProp, AVisibleEntity );

	enum PROPFLAG : TUINT8
	{
		PROPFLAG_Paintable = 0x1,
	};

public:
	AProp();
	virtual ~AProp() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void Create() override;                                                                   // +0x1C
	virtual void PostCreate() override;                                                               // +0x20 (also ATriggerUtil +0x08)
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C (also ATriggerUtil +0x04)
	virtual void SetPosition( const Toshi::TVector4& a_rPosition ) override;                          // +0x40
	virtual void SetMatrix( const Toshi::TMatrix44& a_rMatrix ) override;                             // +0x48
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 0076dcc0
	virtual ACollisionObject* GetCollisionObject() override { return m_pCollisionObject; } // +0x70
	virtual void              SetBBox( Toshi::TVector4 a_vBBox ) override;                 // +0x7C
	virtual void              Reset( ABaseEntity* a_pEntity ) override;                    // +0xC8
	// $deBlob: FUNCTION 00774ec0
	virtual const TCHAR* GetModelName() override { return TNULL; } // +0xEC
	// Sets the model instance's ambient colour from the painting caller's colour
	virtual void ApplyBlobColour( ABaseEntity* a_pCaller, void* a_pModelInstance ); // +0xF0

protected:
	void CreateCollisionObject();

	// The manual animations of the model
public:
	TINT16 CreateManualAnimation( const TCHAR* a_szName );
	void   SetMesh( const TCHAR* a_szMesh );
	// The manual animation m_iManualAnim if it's still running (0 < time < duration), else TNULL
	Toshi::TAnimation* GetPlayingManualAnimation();
	Toshi::TAnimation* AddManualAnimation( TINT16 a_iSequence, TFLOAT a_fTime, TFLOAT a_fBlendInSpeed, TFLOAT a_fWeight, TFLOAT a_fSpeed );
	void               KillManualAllAnimations( TFLOAT a_fBlendOutSpeed );

	TBOOL IsPainted();

	// NOTE: public, APropWindCannon checks the animations of its target
	TBOOL IsManualAnimationFinished( TINT16 a_iSequence );

protected:
	// Creates the ACollisionInstance2 of the model (owner type 2: a prop)
	void CreateModelCollision( TBOOL a_bPrimitives );
	// Restores the default local ambient colour of a_pModelInstance
	void ResetAmbientColour( AModelInstance2* a_pModelInstance );

protected:
	ACollisionObject* m_pCollisionObject; // 0xAC
	TINT16            m_iManualAnim;      // 0xB0 -1
	TUINT8            m_uiPropFlags;      // 0xB2 PROPFLAG
};
