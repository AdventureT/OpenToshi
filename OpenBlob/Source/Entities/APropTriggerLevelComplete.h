#pragma once
#include "Entities/APropTrigger.h"

class AProp;
class APropDoor;

TOSHI_NAMESPACE_START
class TParticleEffect;
class TParticleEffectInstance;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: the level exit: the blob enters the exit door and the level is completed
// (APropTriggerLevelComplete\APropTriggerLevelComplete.cpp, plus ATriggerUtil's 6 at 0x84).
//-----------------------------------------------------------------------------
class APropTriggerLevelComplete
    : public APropTrigger
{
public:
	TDECLARE_CLASS( APropTriggerLevelComplete, APropTrigger );

	enum STATE : TINT
	{
		STATE_Waiting   = 0,
		STATE_Ready     = 1,
		STATE_Entering  = 2,
		STATE_Completed = 3,
	};

public:
	APropTriggerLevelComplete();
	virtual ~APropTriggerLevelComplete() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void PostCreate() override;                                                               // +0x20 (also ATriggerUtil +0x08)
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C (also ATriggerUtil +0x04)
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	// $deBlob: FUNCTION 0076ec90
	virtual TBOOL IsTargetable() override { return m_eState == STATE_Ready; } // +0xA0
	// $deBlob: FUNCTION 0076ecf0
	virtual Toshi::TVector4 GetTargetOffset() override { return Toshi::TVector4( 0.0f, -1.0f, 0.0f, 1.0f ); } // +0xAC
	virtual void            Reset( ABaseEntity* a_pEntity ) override;                                         // +0xC8
	// $deBlob: FUNCTION 0076ec80
	virtual TBOOL CanObjectTrigger( Toshi::TObject* a_pObject ) override { return TFALSE; } // +0xE0
	// $deBlob: FUNCTION 0076ed20
	virtual void Trigger() override {} // +0xE4

	// Out: the world matrix of the door's camera bone; returns m_fUnk120
	TFLOAT GetDoorCameraMatrix( Toshi::TMatrix44& a_rOutMatrix );
	TFLOAT GetDoorCameraAnimTime();
	// Out: the world matrix of the door's blob bone
	void GetDoorBlobMatrix( Toshi::TMatrix44& a_rOutMatrix );

private:
	void SetState( STATE a_eState );

private:
	Toshi::TMatrix44                m_mGrateMatrix;      // 0xC0 the grate's initial matrix
	STATE                           m_eState;            // 0x100
	APropDoor*                      m_pExitDoor;         // 0x104
	AProp*                          m_pGrate;            // 0x108
	ABaseEntity*                    m_pBlob;             // 0x10C
	TINT16                          m_iGrateOpenAnimID;  // 0x110 "LevelEnd_Alt"
	TINT16                          m_iGrateCloseAnimID; // 0x112 "Idle"
	TINT16                          m_iDoorCamAnimID;    // 0x114 "LevelEnd_Camera_Alt"
	TINT16                          m_iDoorCamBoneID;    // 0x116 "camera_loc"
	TINT16                          m_iDoorBlobBoneID;   // 0x118 "blobattach_loc"
	TFLOAT                          m_fUnk11C;           // 0x11C
	TFLOAT                          m_fUnk120;           // 0x120
	Toshi::TParticleEffect*         m_pExitPoolEffect;   // 0x124 "Blob_ExitPool"
	Toshi::TParticleEffectInstance* m_pExitPool;         // 0x128
};
