#include "pch.h"
#include "ATriggerUtil.h"
#include "Buildings/ABuildingManager.h"
#include "Cameras/ACameraManager.h"
#include "AEntityParams.h"
#include "AEntityLoader.h"
#include "AEntityIOPacket.h"
#include "ABaseEntity.h"
#include "Terrain2/ATerrain2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0077e1a0
ATriggerUtil::ATriggerUtil()
{
	m_pTargets           = TNULL;
	m_uiNumEntityTargets = 0;
	m_uiNumInstances     = 0;
	m_uiNumNavSets       = 0;
	m_uiNumCameraShakes  = 0;
}

// $deBlob: FUNCTION 0077e1c0
// $deBlob: FUNCTION 0077e8f0
ATriggerUtil::~ATriggerUtil()
{
	::operator delete( m_pTargets );
}

// NOTE: the params are always an AEntityLoader::EntityData (the target list follows the property list)
// $deBlob: FUNCTION 0077e1d0
void ATriggerUtil::ProcessParams( const AEntityParams& a_rParams )
{
	const AEntityLoader::EntityData& rEntityData = static_cast<const AEntityLoader::EntityData&>( a_rParams );

	TASSERT( rEntityData.GetTargetCount() < TMath::MAXWCHAR );
	m_uiNumEntityTargets = TUINT16( rEntityData.GetTargetCount() );

	AEntityParams::IDList* instances = TNULL;
	if ( a_rParams.GetInstances( instances ) )
	{
		TASSERT( instances->m_iCount < TMath::MAXWCHAR );
		m_uiNumInstances = TUINT16( instances->m_iCount );
	}

	AEntityParams::IDList* pNavSets = TNULL;
	if ( a_rParams.GetNavSets( pNavSets ) )
	{
		TASSERT( pNavSets->m_iCount < TMath::MAXWCHAR );
		m_uiNumNavSets = TUINT16( pNavSets->m_iCount );
	}

	AEntityParams::IDList* pCamList = TNULL;
	if ( a_rParams.GetCameraShakes( pCamList ) )
	{
		TASSERT( pCamList->m_iCount < TMath::MAXWCHAR );
		m_uiNumCameraShakes = TUINT16( pCamList->m_iCount );
	}

	TUINT16 uiTotal = TUINT16( m_uiNumNavSets + m_uiNumEntityTargets + m_uiNumCameraShakes + m_uiNumInstances );
	if ( uiTotal != 0 )
		m_pTargets = TSTATICCAST( TUINT16, TMemalign( 16, uiTotal * sizeof( TUINT16 ) ) );

	if ( m_uiNumEntityTargets != 0 )
	{
		for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
			SetEntityTargetID( i, rEntityData.GetTarget( i ) );
	}

	if ( m_uiNumInstances != 0 )
	{
		for ( TINT i = 0; i < m_uiNumInstances; i++ )
			SetInstanceID( i, instances->m_pIDs[ i ] );
	}

	if ( m_uiNumNavSets != 0 )
	{
		for ( TINT i = 0; i < m_uiNumNavSets; i++ )
			SetNavSet( i, TUINT( pNavSets->m_pIDs[ i ] ) );
	}

	if ( m_uiNumCameraShakes != 0 )
	{
		for ( TINT i = 0; i < m_uiNumCameraShakes; i++ )
			SetCameraShakeID( i, pCamList->m_pIDs[ i ] );
	}
}

// Removes the entity targets that don't exist
// $deBlob: FUNCTION 0077e4e0
void ATriggerUtil::PostCreate()
{
	TINT i = 0;
	while ( i < m_uiNumEntityTargets )
	{
		if ( GetEntityTarget( i ) == TNULL )
		{
			for ( TINT j = i + 1; j < m_uiNumEntityTargets; j++ )
				SetEntityTargetID( j - 1, GetEntityTargetID( j ) );

			m_uiNumEntityTargets--;
		}
		else
		{
			i++;
		}
	}
}

// $deBlob: FUNCTION 0077e5a0
ABaseEntity* ATriggerUtil::GetEntityTarget( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumEntityTargets );
	return ABaseEntity::FindIDEntity( GetEntityTargetID( a_iIndex ) );
}

// $deBlob: FUNCTION 0077e650
void* ATriggerUtil::GetInstanceTarget( TINT a_iIndex )
{
	TASSERT( a_iIndex >= 0 && a_iIndex < m_uiNumInstances );
	return ATerrain2::GetInstance()->GetCellInstance( GetInstanceID( a_iIndex ) );
}

// $deBlob: FUNCTION 0077e820
void ATriggerUtil::TriggerInstanceTargets( ABaseEntity* a_pBlob )
{
	for ( TINT i = 0; i < m_uiNumInstances; i++ )
		ABuildingManager::TriggerTransformation( TREINTERPRETCAST( TerrainTypes::CellInstance*, GetInstanceTarget( i ) ), a_pBlob );
}

// $deBlob: FUNCTION 0077e860
void ATriggerUtil::TriggerInstanceTarget( TINT a_iIndex, void* a_pTrigger )
{
	ABuildingManager::TriggerTransformation( TREINTERPRETCAST( TerrainTypes::CellInstance*, GetInstanceTarget( a_iIndex ) ), TREINTERPRETCAST( ABaseEntity*, a_pTrigger ) );
}

// $deBlob: FUNCTION 0077e730
void ATriggerUtil::TriggerAllTargets( ABaseEntity* a_pCaller )
{
	TINT iData = GetTriggerData();
	SendMessageToTargets( a_pCaller, GetMessageToSend(), iData );
}

// $deBlob: FUNCTION 0077e760
void ATriggerUtil::SendMessageToTargets( ABaseEntity* a_pCaller, AEntityIOPacket::MESSAGE a_eMessage, TINT a_iData )
{
	TASSERT( AEntityIOPacket::MESSAGE_NONE != a_eMessage );

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = a_eMessage;
	oPacket.m_iData    = a_iData;

	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget && !pTarget->IsDespawned() )
			pTarget->HandleEntityIO( a_pCaller, oPacket );
	}

	if ( a_eMessage == AEntityIOPacket::MESSAGE_TRIGGER )
	{
		for ( TINT i = 0; i < m_uiNumInstances; i++ )
		{
			ABuildingManager::TriggerTransformation( TREINTERPRETCAST( TerrainTypes::CellInstance*, GetInstanceTarget( i ) ), TNULL );
		}

		StartCameraShakes();
	}
}

// $deBlob: FUNCTION 0077e880
void ATriggerUtil::StartCameraShakes()
{
	TUINT uiIndex = m_uiNumCameraShakes;
	while ( uiIndex != 0 )
	{
		uiIndex--;
		TINT iCamShakeID = GetCameraShakeID( uiIndex );
		ACameraManager::GetSingleton()->GetShakeFunction().StartShake( iCamShakeID );
	}
}
