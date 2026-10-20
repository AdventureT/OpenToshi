#pragma once
#include "Entities/ABaseEntity.h"

//-----------------------------------------------------------------------------
// Purpose: an invisible entity that plays a sound event (or switches sound emitters on or off) when it receives an
// entity IO message.
//-----------------------------------------------------------------------------
class ASoundAction
    : public ABaseEntity
{
public:
	TDECLARE_CLASS( ASoundAction, ABaseEntity );

	// The entity's sound action property (AEntityParam::TYPE_SOUNDACTION, 0xC bytes), copied by ProcessParams
	struct Data
	{
		enum FLAG : TUINT16
		{
			FLAG_PlayEvent       = 0x1,
			FLAG_DisableEmitters = 0x2,
			FLAG_EnableEmitters  = 0x4,
			FLAG_NotPositional   = 0x8,
			FLAG_Started         = 0x10, // a looping event was started (stopped by Reset)
		};

		TUINT16  m_uiFlags;      // 0x00 FLAG
		TINT16   m_iEventID;     // 0x02 ASoundMap::EVENTID
		TINT16   m_iGroupID;     // 0x04 ASoundMap::GROUPID
		TINT16   m_iNumEmitters; // 0x06
		TUINT32* m_pEmitters;    // 0x08 m_iNumEmitters entries (TMemalign16)
	};

public:
	ASoundAction();
	virtual ~ASoundAction() override; // +0x08

	virtual void ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual void Reset( ABaseEntity* a_pEntity ) override;                                            // +0xC8

private:
	void Start();
	void Play();
	void SetEmittersActive( TBOOL a_bActive );

private:
	Data* m_pData; // 0x80
};
