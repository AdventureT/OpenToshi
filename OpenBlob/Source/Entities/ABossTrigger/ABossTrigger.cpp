#include "pch.h"
#include "ABossTrigger.h"
#include "Entities/AStreetSpawner/AStreetSpawner.h"
#include "Entities/ABoss/ABoss.h"
#include "Entities/AEntityParams.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/APropPlatform.h"
#include "Entities/APropTrigger.h"
#include "Entities/APropHazard/AContainerHazard.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Buildings/ABuildingInstance.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "GameController/AGameStatus.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "Terrain2/TerrainTypes.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ABossTrigger ) == 0x98 );

// The building of an instance target of a platform, TNULL if the instance isn't a building
static ABuildingInstance* GetPlatformBuilding( APropPlatform* a_pPlatform, TINT a_iIndex )
{
	TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, a_pPlatform->GetInstanceTarget( a_iIndex ) );

	if ( pInstance != TNULL && ( pInstance->m_uiFlags & 7 ) == 1 )
		return TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 );

	return TNULL;
}

// The entity target a_iIndex if it's a platform (the exact class), else TNULL
static APropPlatform* GetPlatform( ATriggerUtil* a_pTriggerUtil, TINT a_iIndex )
{
	ABaseEntity* pTarget = a_pTriggerUtil->GetEntityTarget( a_iIndex );

	if ( pTarget != TNULL && pTarget->GetClass() == &TGetClass( APropPlatform ) )
		return TSTATICCAST( APropPlatform, pTarget );

	return TNULL;
}

// $deBlob: FUNCTION 007a5320
// $deBlob: FUNCTION 007a5330
// $deBlob: FUNCTION 007a53a0
// $deBlob: FUNCTION 007a5400
// $deBlob: FUNCTION 007a5410
// $deBlob: FUNCTION 005410e0
// $deBlob: FUNCTION 00859ec0
TDEFINE_CLASS( ABossTrigger );

// $deBlob: FUNCTION 007a5420
ABossTrigger::ABossTrigger()
{
	m_eBTState         = BTState_Down;
	m_uiBTFlags        = 0;
	m_uiRequiredColour = 1;
	m_uiBossStage      = 0;
	m_pBoss            = TNULL;

	SetHash( TFALSE );
	SetTicking( TFALSE );
	m_Flags |= FLAG_DontTick;
}

// $deBlob: FUNCTION 007a54c0
// $deBlob: FUNCTION 007a60b0
// $deBlob: FUNCTION 007a609a
ABossTrigger::~ABossTrigger()
{
}

// $deBlob: FUNCTION 007a5530
// $deBlob: FUNCTION 007a6169
void ABossTrigger::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );
	ATriggerUtil::ProcessParams( a_rParams );

	a_rParams.GetValue( m_uiBossStage, "BossStage" );
	a_rParams.GetValue( m_uiRequiredColour, "Required Colour" );
}

// $deBlob: FUNCTION 007a5580
// $deBlob: FUNCTION 007a615e
void ABossTrigger::PostCreate()
{
	ATriggerUtil::PostCreate();

	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget != TNULL && pTarget->GetClass() == &TGetClass( ABoss ) )
		{
			m_pBoss = TSTATICCAST( ABoss, pTarget );
			break;
		}
	}

	SetBuildingsLocked( TTRUE );
}

// $deBlob: FUNCTION 007a55f0
void ABossTrigger::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity::Tick( a_fDeltaTime );

	if ( m_eBTState == BTState_Up )
	{
		UpdateProjections( a_fDeltaTime );

		// Complete once every building of the platforms has the required colour
		for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
		{
			APropPlatform* pPlatform = GetPlatform( this, i );

			if ( pPlatform != TNULL && !pPlatform->AreInstancesColour( m_uiRequiredColour ) )
			{
				m_uiBTFlags &= ~BTFLAG_Completed;
				return;
			}
		}

		if ( !( m_uiBTFlags & BTFLAG_Completed ) )
		{
			m_uiBTFlags |= BTFLAG_Completed;
			TASSERT( BTState_Up == GetBTState() );
			m_uiBTFlags &= ~BTFLAG_Raising;
			m_eBTState = BTState_Moving;

			TriggerPlatforms();
			DisableProjections();
			SetBuildingsLocked( TTRUE );
			m_pBoss->OnBossTriggerCompleted();
		}
	}
	else if ( m_eBTState == BTState_Moving )
	{
		// Waits for the platforms to stop
		for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
		{
			APropPlatform* pPlatform = GetPlatform( this, i );

			if ( pPlatform != TNULL && !pPlatform->IsStopped() )
				return;
		}

		if ( m_uiBTFlags & BTFLAG_Raising )
		{
			TASSERT( !IsCompleted() );
			m_eBTState = BTState_Up;
			return;
		}

		m_eBTState = BTState_Down;

		if ( m_uiBTFlags & BTFLAG_Completed )
		{
			SetTicking( TFALSE );
			m_Flags |= FLAG_DontTick;
		}
	}
}

// $deBlob: FUNCTION 007a5830
void ABossTrigger::Reset( ABaseEntity* a_pEntity )
{
	DisableProjections();
	SetBuildingsLocked( TTRUE );
	ABaseEntity::Reset( a_pEntity );
	m_uiBTFlags = 0;
	SetTicking( TFALSE );
	m_Flags |= FLAG_DontTick;
	m_eBTState = BTState_Down;
}

void ABossTrigger::TriggerPlatforms()
{
	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		APropPlatform* pPlatform = GetPlatform( this, i );

		if ( pPlatform != TNULL )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pPlatform->HandleEntityIO( this, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007a5880
void ABossTrigger::Raise()
{
	m_Flags &= ~FLAG_DontTick;
	SetTicking( TTRUE );
	m_uiBTFlags |= BTFLAG_Raising;
	m_eBTState = BTState_Moving;

	TriggerPlatforms();
	EnableProjections();
	SetBuildingsLocked( TFALSE );
	RestoreBuildingsCollision();
}

// $deBlob: FUNCTION 007a5930
void ABossTrigger::ActivateSpawners()
{
	// The street spawners among the targets are started
	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget != TNULL && pTarget->GetClass() == &TGetClass( AStreetSpawner ) )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pTarget->HandleEntityIO( this, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007a59a0
void ABossTrigger::RaiseAgain()
{
	TASSERT( BTState_Down == GetBTState() );

	m_Flags &= ~FLAG_DontTick;
	SetTicking( TTRUE );
	m_eBTState  = BTState_Moving;
	m_uiBTFlags = ( m_uiBTFlags & ~BTFLAG_Completed ) | BTFLAG_Raising;

	TUINT32 uiGrey = ABlobColourComponent::ms_auiColours[ 0 ];

	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		APropPlatform* pPlatform = GetPlatform( this, i );

		if ( pPlatform != TNULL )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pPlatform->HandleEntityIO( this, oPacket );

			// The buildings lose their colour
			TINT iNumInstances = pPlatform->GetNumInstances();

			for ( TINT j = 0; j < iNumInstances; j++ )
			{
				ABuildingInstance* pBuilding = GetPlatformBuilding( pPlatform, j );

				if ( pBuilding != TNULL )
				{
					pBuilding->m_uiCurrentColour = 0;
					pBuilding->SetLocalAmbientColour( uiGrey );
				}
			}
		}
	}

	EnableProjections();
	SetBuildingsLocked( TFALSE );
}

// $deBlob: FUNCTION 007a5ae0
void ABossTrigger::SetHazardsActive( TBOOL a_bActive )
{
	TINT eMessage = ( !a_bActive ) + AEntityIOPacket::MESSAGE_OPEN;

	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget != TNULL && pTarget->GetClass() == &TGetClass( AContainerHazard ) )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE( eMessage );
			oPacket.m_iData    = 0;
			pTarget->HandleEntityIO( this, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007a5b60
void ABossTrigger::DespawnTargets( TBOOL a_bDespawn )
{
	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget != TNULL && pTarget->GetClass() == &TGetClass( AStreetSpawner ) )
		{
			// The street spawners stop, their enemies go too on despawn
			AStreetSpawner* pSpawner = TSTATICCAST( AStreetSpawner, pTarget );
			pSpawner->Stop();

			if ( a_bDespawn )
				pSpawner->DeactivateSpawns();

			continue;
		}

		if ( a_bDespawn && pTarget != TNULL && pTarget->GetClass() == &TGetClass( APropTrigger ) )
		{
			TASSERT( AEntityIOPacket::MESSAGE_DESPAWN == TSTATICCAST( ATriggerUtil, TSTATICCAST( APropTrigger, pTarget ) )->GetMessageToSend() );

			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
			oPacket.m_iData    = 0;
			pTarget->HandleEntityIO( this, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007a5c50
void ABossTrigger::EnableProjections()
{
	m_uiBTFlags |= BTFLAG_Projections;

	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		APropPlatform* pPlatform = GetPlatform( this, i );

		if ( pPlatform != TNULL )
		{
			TINT iNumInstances = pPlatform->GetNumInstances();

			for ( TINT j = 0; j < iNumInstances; j++ )
			{
				ABuildingInstance* pBuilding = GetPlatformBuilding( pPlatform, j );

				if ( pBuilding != TNULL )
					pBuilding->m_uiFlags |= ABuildingInstance::Flags_BossPlatform;
			}
		}
	}
}

// $deBlob: FUNCTION 007a5d00
void ABossTrigger::DisableProjections()
{
	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		APropPlatform* pPlatform = GetPlatform( this, i );

		if ( pPlatform != TNULL )
		{
			TINT iNumInstances = pPlatform->GetNumInstances();

			for ( TINT j = 0; j < iNumInstances; j++ )
			{
				ABuildingInstance* pBuilding = GetPlatformBuilding( pPlatform, j );

				if ( pBuilding != TNULL )
				{
					TINT iProjection = AGameStatus::RemoveBuildingProjection( pBuilding );

					if ( iProjection >= 0 )
						AGameStatus::GetSingleton()->FreeProjection( iProjection );

					pBuilding->m_uiFlags &= ~ABuildingInstance::Flags_BossPlatform;
				}
			}
		}
	}

	m_uiBTFlags &= ~BTFLAG_Projections;
}

// $deBlob: FUNCTION 007a5dd0
void ABossTrigger::UpdateProjections( TFLOAT a_fDeltaTime )
{
	if ( !( m_uiBTFlags & BTFLAG_Projections ) )
		return;

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		APropPlatform* pPlatform = GetPlatform( this, i );

		if ( pPlatform == TNULL )
			continue;

		TINT iNumInstances = pPlatform->GetNumInstances();

		for ( TINT j = 0; j < iNumInstances; j++ )
		{
			ABuildingInstance* pBuilding = GetPlatformBuilding( pPlatform, j );

			if ( pBuilding == TNULL || m_uiRequiredColour == pBuilding->m_uiCurrentColour || !( pBuilding->m_uiFlags & ABuildingInstance::Flags_BossPlatform ) )
				continue;

			TINT iGlint = pBuilding->m_iGlintIndex;

			if ( iGlint < 0 )
			{
				// The building gets a projection
				if ( !AJellyShader_DX11::GetSingleton()->IsPainting( pBuilding ) )
				{
					TINT iProjection = AGameStatus::GetSingleton()->AllocateProjection();

					if ( iProjection != -1 )
					{
						AGameStatus::SetBuildingProjection( pBuilding, iProjection, TTRUE );

						if ( !( pBuilding->m_uiFlags & ABuildingInstance::Flags_Inked ) )
							pBuilding->SetLocalAmbientColour( ABlobColourComponent::ms_auiColours[ pBuilding->m_uiCurrentColour ] );
					}
				}
			}
			else
			{
				AGameStatus::UpdateProjectionGlint( iGlint, a_fDeltaTime );
				// Projects the required colour onto the building
				AGameStatus::UpdateProjectionCamera( iGlint, pBuilding, pCamera, m_uiRequiredColour, TTRUE, TFALSE );
			}
		}
	}
}

// $deBlob: FUNCTION 007a5f50
void ABossTrigger::RestoreBuildingsCollision()
{
	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		APropPlatform* pPlatform = GetPlatform( this, i );

		if ( pPlatform != TNULL )
		{
			TINT iNumInstances = pPlatform->GetNumInstances();

			for ( TINT j = 0; j < iNumInstances; j++ )
			{
				ABuildingInstance* pBuilding = GetPlatformBuilding( pPlatform, j );

				if ( pBuilding != TNULL )
					pBuilding->SetCollisionLayers( TFALSE );
			}
		}
	}
}

// $deBlob: FUNCTION 007a5ff0
void ABossTrigger::SetBuildingsLocked( TBOOL a_bLocked )
{
	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		APropPlatform* pPlatform = GetPlatform( this, i );

		if ( pPlatform != TNULL )
		{
			TINT iNumInstances = pPlatform->GetNumInstances();

			for ( TINT j = 0; j < iNumInstances; j++ )
			{
				ABuildingInstance* pBuilding = GetPlatformBuilding( pPlatform, j );

				if ( pBuilding != TNULL )
				{
					if ( a_bLocked )
						pBuilding->m_uiFlags |= ABuildingInstance::Flags_NotPaintable;
					else
						pBuilding->m_uiFlags &= ~ABuildingInstance::Flags_NotPaintable;
				}
			}
		}
	}
}
