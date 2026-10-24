#pragma once
#include "Entities/AEntityIOPacket.h"

class ABaseEntity;
class AEntityParams;

//-----------------------------------------------------------------------------
// Purpose: the targets an entity triggers: entity targets, instances, navigation sets and camera shakes read from the
// entity params. A second base of AProp and APropTrigger.
//-----------------------------------------------------------------------------
class ATriggerUtil
{
public:
	ATriggerUtil();
	virtual ~ATriggerUtil(); // +0x00

	virtual void ProcessParams( const AEntityParams& a_rParams ); // +0x04
	virtual void PostCreate();                                    // +0x08
	virtual void TriggerAllTargets( ABaseEntity* a_pCaller );     // +0x0C
	// $deBlob: FUNCTION 0076ddd0
	virtual AEntityIOPacket::MESSAGE GetMessageToSend() { return AEntityIOPacket::MESSAGE_TRIGGER; } // +0x10
	// $deBlob: FUNCTION 0076dde0
	virtual TINT GetTriggerData() { return 0; } // +0x14

	// Returns the entity target a_iIndex (NULL if it doesn't exist)
	ABaseEntity* GetEntityTarget( TINT a_iIndex );

	// Returns the terrain instance target a_iIndex (a TerrainTypes::CellInstance)
	void* GetInstanceTarget( TINT a_iIndex );

	// Triggers the building instance target a_iIndex
	void TriggerInstanceTarget( TINT a_iIndex, void* a_pTrigger );
	// Triggers the transformation of every instance target for a_pBlob
	void TriggerInstanceTargets( ABaseEntity* a_pBlob );

	// Sends a_eMessage to every entity target, and on MESSAGE_TRIGGER also triggers the instances and camera shakes
	void SendMessageToTargets( ABaseEntity* a_pCaller, AEntityIOPacket::MESSAGE a_eMessage, TINT a_iData );

	// Starts every camera shake
	void StartCameraShakes();

	TUINT16 GetNumEntityTargets() const { return m_uiNumEntityTargets; }
	TUINT16 GetNumInstances() const { return m_uiNumInstances; }

protected:
	// The ID list: camera shakes, then nav sets, instances and entity targets
	// $deBlob: FUNCTION 0077e980
	TUINT16 GetEntityTargetID( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumEntityTargets );
		return m_pTargets[ m_uiNumInstances + ( ( m_uiNumNavSets + a_iIndex ) + m_uiNumCameraShakes ) ];
	}

	// $deBlob: FUNCTION 0077eaf0
	void SetEntityTargetID( TINT a_iIndex, TINT a_iTarget )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumEntityTargets );
		TASSERT( a_iTarget >= 0 && a_iTarget < 0xFFFF );
		m_pTargets[ m_uiNumInstances + ( ( m_uiNumNavSets + a_iIndex ) + m_uiNumCameraShakes ) ] = TUINT16( a_iTarget );
	}

	// $deBlob: FUNCTION 0077e9f0
	TUINT16 GetInstanceID( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumInstances );
		return m_pTargets[ m_uiNumNavSets + ( m_uiNumCameraShakes + a_iIndex ) ];
	}

	// $deBlob: FUNCTION 0077eb90
	void SetInstanceID( TINT a_iIndex, TINT a_iInstance )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumInstances );
		TASSERT( a_iInstance >= 0 && a_iInstance < 0xFFFF );
		m_pTargets[ m_uiNumNavSets + m_uiNumCameraShakes + a_iIndex ] = TUINT16( a_iInstance );
	}

	// $deBlob: FUNCTION 0079a440
	TUINT16 GetNavSetID( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumNavSets );
		return m_pTargets[ m_uiNumCameraShakes + a_iIndex ];
	}

	// $deBlob: FUNCTION 0077ec30
	void SetNavSet( TINT a_iIndex, TUINT a_uiNavSet )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumNavSets );
		TASSERT( a_uiNavSet >= 0 && a_uiNavSet < 0xFFFF );
		m_pTargets[ m_uiNumCameraShakes + a_iIndex ] = TUINT16( a_uiNavSet );
	}

	// $deBlob: FUNCTION 0077e920
	TINT GetCameraShakeID( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_iCameraShakeCount );
		return m_pTargets[ a_iIndex ];
	}

	// $deBlob: FUNCTION 0077ea50
	void SetCameraShakeID( TINT a_iIndex, TINT a_iCamShakeID )
	{
		TASSERT( a_iIndex >= 0 && a_iIndex < m_iCameraShakeCount );
		TASSERT( a_iCamShakeID >= 0 && a_iCamShakeID < 0xFFFF );
		m_pTargets[ a_iIndex ] = TUINT16( a_iCamShakeID );
	}

protected:
	TUINT16* m_pTargets;           // 0x04
	TUINT16  m_uiNumEntityTargets; // 0x08
	TUINT16  m_uiNumInstances;     // 0x0A
	TUINT16  m_uiNumNavSets;       // 0x0C
	union
	{
		TUINT16 m_uiNumCameraShakes; // 0x0E
		TUINT16 m_iCameraShakeCount; // 0x0E
	};
};
