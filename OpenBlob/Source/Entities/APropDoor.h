#pragma once
#include "Entities/AProp.h"

class APropSwitchPressure;

//-----------------------------------------------------------------------------
// Purpose: a door or gate that opens and closes with animations on entity IO messages (APropDoor\APropDoor.cpp, plus
// ATriggerUtil's 6 at 0x9C). The level start gate (DoorType 1) and the sprint gates (DoorType 2) are registered in
// AGameStatus.
//-----------------------------------------------------------------------------
TOSHI_NAMESPACE_START
class TAnimation;
TOSHI_NAMESPACE_END

class APropDoor
    : public AProp
{
public:
	TDECLARE_CLASS( APropDoor, AProp );

	enum DOORSTATE : TINT
	{
		DOORSTATE_Opening      = 0,
		DOORSTATE_Open         = 1,
		DOORSTATE_Closing      = 2,
		DOORSTATE_WaitForClose = 3, // the close scripted sequence / delay
		DOORSTATE_Closed       = 4,
		DOORSTATE_LevelStart   = 5,
	};

	enum DOORTYPE : TUINT8
	{
		DOORTYPE_Door       = 0,
		DOORTYPE_LevelStart = 1,
		DOORTYPE_Sprint     = 2,
	};

public:
	APropDoor();
	virtual ~APropDoor() override; // +0x08

	virtual void Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C (also ATriggerUtil +0x04)
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8
	virtual void ApplyBlobColour( ABaseEntity* a_pCaller, void* a_pModelInstance ) override;          // +0xF0

	void Open();
	void Close();

	// The level start gate (DoorType 1), driven by AGameStatus
	void   PlayLevelStart( TBOOL a_bRespawn );
	TBOOL  GetLevelStartCamera( Toshi::TMatrix44& a_rOutMatrix, TFLOAT& a_rfWeight );
	TBOOL  GetBlobAttachPosition( Toshi::TVector4& a_rOutPosition );
	TFLOAT GetLevelStartAnimTime();
	// $deBlob: FUNCTION 006564e0
	DOORSTATE GetDoorState() const { return m_eState; }

public:
	// The pressure switch that opened the door (released when it closes, MESSAGE_TOUCH)
	void SetSwitch( APropSwitchPressure* a_pSwitch ) { m_iSwitch = TINT( TREINTERPRETCAST( TUINTPTR, a_pSwitch ) ); }

private:
	// The level start animation playing, if any
	Toshi::TAnimation* GetLevelStartAnimation();

private:
	TINT16    m_iOpenAnim;              // 0xB4 -1
	TINT16    m_iCloseAnim;             // 0xB6 -1
	TINT16    m_iLevelStartOpenAnim;    // 0xB8 -1
	TINT16    m_iLevelStartRespawnAnim; // 0xBA -1
	TBOOL     m_bStartOpen;             // 0xBC
	DOORTYPE  m_eDoorType;              // 0xBD
	TFLOAT    m_fBBoxScale;             // 0xC0 1
	DOORSTATE m_eState;                 // 0xC4
	TINT      m_iSwitch;                // 0xC8
	TINT      m_iCloseScriptedSequence; // 0xCC -1
	TFLOAT    m_fCloseDelay;            // 0xD0
	TFLOAT    m_fCloseTimer;            // 0xD4
};
