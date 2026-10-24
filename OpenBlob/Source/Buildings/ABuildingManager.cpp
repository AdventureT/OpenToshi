#include "pch.h"
#include "ABuildingManager.h"
#include "Cameras/ACameraManager.h"
#include "ARandom.h"
#include "Platform/DX11/AJellyShader_DX11.h"
#include "ALandmarkInstance.h"
#include "AGrassInstance.h"
#include "ATreeInstance.h"
#include "AToggleVisInstance.h"
#include "ATextureSwapInstance.h"
#include "Entities/ACivSpawner.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/ASpawnedCivManager.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "ACurrentLevelInformation.h"
#include "Terrain2/ATerrain2.h"
#include "Entities/APropTriggerHelp.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Fx/AParticleEffectHelper.h"
#include "Sound/ASoundInterface.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <TParticleSystem/TParticleEffect.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleSystem.h>

// Note: Should be the last include!
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The delay before a building of a jelly block takes the painted colour
static TFLOAT s_fJellyDelay = 0.25f;

// The locale strings of the block types
static const TINT s_aiBlockTypeStrings[ PaintableBlock::NUMOF ] = { 0x6C, 0x6D, 0x6E, 0x6F, 0x70, 0x71, 0x72, 0x73 };

// $deBlob: FUNCTION 00578870
ABuildingManager::ABuildingManager()
{
	m_iMaxBuildings            = 0;
	m_iMaxLandmarks            = 0;
	m_iNumBuildings            = 0;
	m_iNumLandmarks            = 0;
	m_aInstancePool            = TNULL;
	m_apLandmarks              = TNULL;
	m_bBlobOnLandmark          = TFALSE;
	m_apPaintableBlocks        = TNULL;
	m_pBlockParticleEffects    = TNULL;
	m_iNumBlockParticleEffects = 0;
}

// $deBlob: FUNCTION 00578990
// $deBlob: FUNCTION 0056be60
ABuildingManager::~ABuildingManager()
{
	TASSERT( TTRUE == m_oActiveInstances.IsEmpty() );
	m_oFreeJellyInstances.Clear();
	m_oJellyInstances.Clear();
}

// $deBlob: FUNCTION 00578ac0
void ABuildingManager::Init( TINT a_iMaxBuildings, TINT a_iMaxLandmarks )
{
	m_iMaxBuildings = a_iMaxBuildings;
	m_iMaxLandmarks = a_iMaxLandmarks;
	m_iNumBuildings = 0;
	m_iNumLandmarks = 0;
	m_aInstancePool = new ABuildingInstance*[ a_iMaxBuildings ];
	m_apLandmarks   = new ABuildingInstance*[ a_iMaxLandmarks ];

	ALandmarkInstance::FindEffects();
	ATreeInstance::CreateEffects();

	m_oFreeJellyInstances.Clear();
	m_oJellyInstances.Clear();

	for ( TINT i = 0; i < NUM_JELLY_INSTANCES; i++ )
		m_oFreeJellyInstances.PushBack( &m_aJellyInstances[ i ] );

	m_pUpdateInstances = new UpdateInstance[ NUM_UPDATE_INSTANCES ];

	for ( TINT i = 0; i < NUM_UPDATE_INSTANCES; i++ )
		m_FreeUpdateInstanceList.PushFront( &m_pUpdateInstances[ i ] );
}

// $deBlob: FUNCTION 00578bf0
void ABuildingManager::Deinit()
{
	ATreeInstance::DestroyEffects();

	m_FreeUpdateInstanceList.Clear();
	m_oUsedUpdateInstances.Clear();

	if ( m_pUpdateInstances != TNULL )
		delete[] m_pUpdateInstances;

	m_oActiveInstances.Clear();

	delete[] m_aInstancePool;
	m_aInstancePool = TNULL;
	delete[] m_apLandmarks;
	m_apLandmarks = TNULL;

	m_iMaxBuildings     = 0;
	m_iMaxLandmarks     = 0;
	m_iNumBuildings     = 0;
	m_iNumLandmarks     = 0;
	m_apPaintableBlocks = TNULL; // NOTE: in the level's data (PaintableBlockData.trb), not freed

	for ( TINT i = 0; i < m_iNumBlockParticleEffects; i++ )
	{
		if ( m_pBlockParticleEffects[ i ] != TNULL )
			delete m_pBlockParticleEffects[ i ];
	}

	TFree( m_pBlockParticleEffects );
	m_pBlockParticleEffects    = TNULL;
	m_iNumBlockParticleEffects = 0;
}

// $deBlob: FUNCTION 00578cc0
void ABuildingManager::TriggerTransformation( TerrainTypes::CellInstance* a_pInstance, ABaseEntity* a_pBlob )
{
	if ( a_pBlob == TNULL || !a_pBlob->IsA( &TGetClass( ABlobEntity ) ) )
		a_pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( 0 );

	TASSERT( a_pBlob != TNULL, "Tried to trigger a building transformation but couldn't find any player!  Do you have a challenge reset trigger that is sending a trigger message?" );

	// The paint callback of the instance's type (index type & 7, 0 for the model and ZJump types)
	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pBlob );

	switch ( a_pInstance->m_uiFlags & 7 )
	{
		case TerrainTypes::InstanceType_Building: ABuildingInstance::OnTable97b978( a_pInstance, pBlob ); break;
		case TerrainTypes::InstanceType_Landmark: ALandmarkInstance::OnTable97b978( a_pInstance, pBlob ); break;
		case TerrainTypes::InstanceType_Tree: ATreeInstance::OnPaintRandom( a_pInstance, pBlob ); break;
		case TerrainTypes::InstanceType_Grass: AGrassInstance::OnPaint( a_pInstance, pBlob ); break;
		case TerrainTypes::InstanceType_TextureSwap: ATextureSwapInstance::OnPaint( a_pInstance, pBlob ); break;
		case TerrainTypes::InstanceType_ToggleVis: AToggleVisInstance::OnPaint( a_pInstance, pBlob ); break;
		default: break;
	}
}

// $deBlob: FUNCTION 00578d40
void ABuildingManager::TriggerLinks( TerrainTypes::CellInstance* a_pInstance, AEntityIOPacket::MESSAGE a_eMessage )
{
	ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetCurrentPlayerBlob();

	// A building painted by a player (m_iMultiplayerOwner >= 0) passes its owner's blob
	if ( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Building )
	{
		TINT8 iOwner = TSTATICCAST( ABuildingInstance, a_pInstance->m_pModelInstance2 )->m_iMultiplayerOwner;

		if ( iOwner >= 0 )
			pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iOwner ) );
	}

	ALandmarkInstance::Links* pLinks = TSTATICCAST( ALandmarkInstance::Links, a_pInstance->m_pLinks );

	if ( pLinks != TNULL )
	{
		for ( TINT i = 0; i < pLinks->m_iNumInstances; i++ )
			TriggerTransformation( pLinks->m_ppInstances[ i ], pBlob );

		for ( TINT i = 0; i < pLinks->m_iNumEntities; i++ )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = a_eMessage;
			oPacket.m_iData    = 0;

			ABaseEntity* pEntity = ABaseEntity::FindIDEntity( pLinks->m_piEntityIDs[ i ] );

			if ( pEntity != TNULL )
				pEntity->HandleEntityIO( TNULL, oPacket );
		}
	}
}

// $deBlob: FUNCTION 00578e30
void ABuildingManager::AddBuilding( ABuildingInstance* a_pBuildingInst )
{
	if ( ( a_pBuildingInst->GetTerrainInstance()->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark )
	{
		TASSERT( m_iNumLandmarks < m_iMaxLandmarks );
		m_apLandmarks[ m_iNumLandmarks++ ] = a_pBuildingInst;
	}
	else
	{
		TASSERT( m_iNumBuildings < m_iMaxBuildings );
		m_aInstancePool[ m_iNumBuildings++ ] = a_pBuildingInst;
	}

	m_oActiveInstances.PushBack( &a_pBuildingInst->GetNode() );
}

// $deBlob: FUNCTION 00578f00
void ABuildingManager::RemoveBuilding( ABuildingInstance* a_pBuildingInst )
{
	TASSERT( !a_pBuildingInst->m_oNode.IsLinked() );

	TerrainTypes::CellInstance* pInstance = a_pBuildingInst->GetTerrainInstance();

	if ( pInstance == TNULL )
		return;

	TINT                iCount;
	ABuildingInstance** ppPool;

	if ( ( pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark )
	{
		iCount = m_iNumLandmarks;
		ppPool = m_apLandmarks;
	}
	else
	{
		iCount = m_iNumBuildings;
		ppPool = m_aInstancePool;
	}

	for ( TINT i = 0; i < iCount; i++ )
	{
		if ( ppPool[ i ] == a_pBuildingInst )
		{
			ppPool[ i ] = TNULL;
			return;
		}
	}
}

// $deBlob: FUNCTION 00578fb0
void ABuildingManager::AddActiveInstance( ABuildingInstance* a_pBuildingInst )
{
	if ( !a_pBuildingInst->GetNode().IsLinked() )
		m_oActiveInstances.PushBack( &a_pBuildingInst->GetNode() );
}

// $deBlob: FUNCTION 00578fd0
void ABuildingManager::AddTickInstance( TerrainTypes::CellInstance* a_pInstance )
{
	// NOTE: retail's assert names this list m_oFreeJellyInstances
	TASSERT( !m_FreeUpdateInstanceList.IsEmpty(), "Ran out of free update instances!\n\t\t\t\t\t\t\t\t\t\t\t\t\tToo much stuff is going on at once, if this is necessary please ask a programmer to increase this limit" );


	UpdateInstance* pTick = m_FreeUpdateInstanceList.PopFront();
	pTick->m_fTime        = 0.0f;
	pTick->m_iInstance    = TINT16( a_pInstance - ATerrain2::GetInstance()->GetInstances() );
	m_oUsedUpdateInstances.PushBack( pTick );
}

// $deBlob: FUNCTION 00579080
void ABuildingManager::Update( TFLOAT a_fDeltaTime )
{
	// The active instances stay while their UpdateImp asks for another frame (ABuildingInstance::Flags_Update)
	for ( auto it = m_oActiveInstances.Begin(); it != m_oActiveInstances.End(); )
	{
		ABuildingInstance* pBuilding = ( it++ )->GetNodeValue();

		pBuilding->m_uiFlags &= ~ABuildingInstance::Flags_Update;
		pBuilding->UpdateImp( a_fDeltaTime );

		if ( !( pBuilding->m_uiFlags & ABuildingInstance::Flags_Update ) )
			pBuilding->GetNode().Remove();
	}

	// The first queued colour change of a jelly block (OnBlockBuildingPainted) runs once its delay is over
	if ( m_oJellyInstances.Size() > 0 )
	{
		JellyInstance* pJelly = m_oJellyInstances.Begin().Get();

		if ( !pJelly->m_bDone )
		{
			if ( pJelly->m_fDelay <= 0.0f )
			{
				ABuildingInstance* pBuilding = pJelly->m_pBuilding;
				AGameStatus::GetSingleton()->GetLevelData().OnBuildingPainted( pBuilding, 0, &pBuilding->GetTerrainInstance()->m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION ), 0, TTRUE );

				pJelly->m_bDone = TTRUE;
				pBuilding->SetColour( pJelly->m_uiColour, pJelly->m_iPlayer, TTRUE, TTRUE );

				m_oJellyInstances.PopFront();
				m_oFreeJellyInstances.PushBack( pJelly );
			}
			else
			{
				pJelly->m_fDelay -= a_fDeltaTime;
			}
		}
	}

	// The ticked cell instances (AddTickInstance): back to the free list once their tick returns TRUE
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	for ( auto it = m_oUsedUpdateInstances.Begin(); it != m_oUsedUpdateInstances.End(); )
	{
		UpdateInstance* pTick = it.Get();
		it++;

		TerrainTypes::CellInstance* pInstance = pTerrain->GetCellInstance( pTick->m_iInstance );
		pTick->m_fTime += a_fDeltaTime;

		// NOTE: retail calls the tick callback of the type from a table: trees (3) and type 6
		TBOOL bDone;

		switch ( pInstance->m_uiFlags & 7 )
		{
			case TerrainTypes::InstanceType_Tree:
				bDone = ATreeInstance::OnTick( pInstance, a_fDeltaTime, pTick->m_fTime );
				break;
			case TerrainTypes::InstanceType_ToggleVis:
				bDone = AToggleVisInstance::OnTick( pInstance, a_fDeltaTime, pTick->m_fTime );
				break;
			default:
				TASSERT( TFALSE && "m_aTickFns[pInstance->GetType()]" );
				bDone = TFALSE;
				break;
		}

		if ( bDone )
		{
			pTick->Remove();
			m_FreeUpdateInstanceList.PushFront( pTick );
		}
	}
}

// The paintable block a_iIndex, TNULL if there's none
// $deBlob: FUNCTION 005792e0
PaintableBlock* ABuildingManager::GetPaintableBlock( TINT a_iIndex )
{
	if ( m_apPaintableBlocks != TNULL && a_iIndex < m_apPaintableBlocks->m_iNumBlocks )
		return &m_apPaintableBlocks->m_pBlocks[ a_iIndex ];

	return TNULL;
}

// $deBlob: FUNCTION 00579310
void ABuildingManager::Reset()
{
	for ( TINT i = 0; i < m_iNumBuildings; i++ )
		m_aInstancePool[ i ]->Reset();

	for ( TINT i = 0; i < m_iNumLandmarks; i++ )
		m_apLandmarks[ i ]->Reset();

	m_bBlobOnLandmark = TFALSE;

	for ( TINT i = 0; i < m_apPaintableBlocks->m_iNumBlocks; i++ )
	{
		PaintableBlock& rBlock = m_apPaintableBlocks->m_pBlocks[ i ];
		rBlock.m_uiFlags &= ~PaintableBlock::FlagMask_Completed;
		rBlock.m_uiFlags &= ~PaintableBlock::FlagMask_Dispersed;
		rBlock.SetNumColours( 0 );
	}

	m_oFreeJellyInstances.Clear();
	m_oJellyInstances.Clear();

	for ( TINT i = 0; i < NUM_JELLY_INSTANCES; i++ )
		m_oFreeJellyInstances.PushBack( &m_aJellyInstances[ i ] );

	m_oUsedUpdateInstances.Clear();
	m_FreeUpdateInstanceList.Clear();

	for ( TINT i = 0; i < NUM_UPDATE_INSTANCES; i++ )
		m_FreeUpdateInstanceList.PushBack( &m_pUpdateInstances[ i ] );
}

// $deBlob: FUNCTION 00579680
ABuildingInstance* ABuildingManager::GetBuildingInstance( TINT a_iInstance )
{
	return TSTATICCAST( ABuildingInstance, ATerrain2::GetInstance()->GetCellInstance( a_iInstance )->m_pModelInstance2 );
}

// $deBlob: FUNCTION 005796e0
ABuildingInstance* __stdcall ABuildingManager::GetLandmark( TINT a_iInstance )
{
	ABuildingInstance* pBuilding = TSTATICCAST( ABuildingInstance, ATerrain2::GetInstance()->GetCellInstance( a_iInstance )->m_pModelInstance2 );
	TASSERT( ( pBuilding->m_pTerrainInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark );

	return pBuilding;
}

// $deBlob: FUNCTION 00579400
ABuildingInstance* ABuildingManager::GetRandomPaintableBuilding( TUINT32 a_uiColour, const TVector4& a_rPosition, TFLOAT a_fRadius )
{
	TASSERT( m_iNumBuildings >= 0 );

	AGameStatus* pGameStatus  = AGameStatus::GetSingleton();
	TFLOAT       fRadiusSq    = a_fRadius * a_fRadius;
	TINT         iHighestRank = 0;
	TINT         iNumHighest  = 0;
	TINT         iNumInRadius = 0;

	// The best rank, its buildings and the ones of them within the radius
	for ( TINT i = m_iNumBuildings; i != 0; )
	{
		ABuildingInstance* pBuilding = m_aInstancePool[ --i ];

		if ( a_uiColour == pBuilding->m_uiCurrentColour && pBuilding->IsAPaintableBuilding() && 0 < pBuilding->Get_MP_BuildingRank() &&
		     ( pBuilding->Get_MP_BuildingRank() <= iHighestRank || iHighestRank == 0 ) && !pGameStatus->IsNextPaintableBuilding( pBuilding ) )
		{
			if ( iHighestRank == 0 || pBuilding->Get_MP_BuildingRank() < iHighestRank )
			{
				iHighestRank = pBuilding->Get_MP_BuildingRank();
				iNumHighest  = 0;
				iNumInRadius = 0;
			}

			iNumHighest++;

			TVector4 vPosition;
			pBuilding->GetPosition( vPosition );
			TASSERT( pBuilding->Get_MP_BuildingRank() == iHighestRank );

			if ( TVector4::DistanceSq( a_rPosition, vPosition ) < fRadiusSq )
				iNumInRadius++;
		}
	}

	if ( iNumHighest == 0 )
		return TNULL;

	// One of them at random (of the ones within the radius if there are such)
	TINT iCount = iNumInRadius;

	if ( iNumInRadius < 1 )
	{
		iCount = iNumHighest;
		TASSERT( iNumHighest > 0 );
	}

	TINT iPick  = ARandom::GetSingleton()->GetRandom().GetInt( iCount );
	TINT iIndex = 0;

	for ( TINT i = m_iNumBuildings; i != 0; )
	{
		ABuildingInstance* pBuilding = m_aInstancePool[ --i ];

		if ( a_uiColour == pBuilding->m_uiCurrentColour && pBuilding->IsAPaintableBuilding() && TFLOAT( pBuilding->Get_MP_BuildingRank() ) == TFLOAT( iHighestRank ) &&
		     !pGameStatus->IsNextPaintableBuilding( pBuilding ) )
		{
			if ( 0 < iNumInRadius )
			{
				TVector4 vPosition;
				pBuilding->GetPosition( vPosition );

				if ( fRadiusSq <= TVector4::DistanceSq( a_rPosition, vPosition ) )
					continue;
			}

			if ( iIndex == iPick )
				return pBuilding;

			iIndex++;
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00579780
void ABuildingManager::SetLandmarkTargetScenesVisible( TBOOL a_bVisible )
{
	for ( TINT i = 0; i < m_iNumLandmarks; i++ )
		TSTATICCAST( ALandmarkInstance, m_apLandmarks[ i ] )->SetTargetScenesVisible( a_bVisible );
}

// $deBlob: FUNCTION 005797b0
TINT ABuildingManager::PaintTeamBuildings( TINT a_iPlayer1, TINT a_iPlayer2, TINT a_iPlayer3, TINT a_iPlayer4 )
{
	TINT iPlayer  = a_iPlayer1;
	TINT iPainted = 0;

	for ( TINT i = 0; i < m_iNumBuildings; i++ )
	{
		ABuildingInstance* pBuilding = m_aInstancePool[ i ];
		TINT               iTeam     = pBuilding->GetTeam();

		if ( !pBuilding->IsAPaintableBuilding() || iTeam <= 0 )
			continue;

		// NOTE: a team above 4 keeps the player of the previous building
		switch ( iTeam )
		{
			case 1: iPlayer = a_iPlayer1; break;
			case 2: iPlayer = a_iPlayer2; break;
			case 3: iPlayer = a_iPlayer3; break;
			case 4: iPlayer = a_iPlayer4; break;
			default: break;
		}

		if ( iPlayer < 0 )
			continue;

		iPainted++;

		TUINT32 uiColour = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iPlayer ) ) )->GetColourComponent()->GetUnk20();

		pBuilding->m_uiFlags           = ( pBuilding->m_uiFlags & ~ABuildingInstance::Flags_Inked ) | ABuildingInstance::Flags_Painted;
		pBuilding->m_iMultiplayerOwner = TINT8( iPlayer );
		pBuilding->SetLocalAmbientColour( ABlobColourComponent::ms_auiColours[ uiColour ] );
		pBuilding->m_uiCurrentColour = TUINT8( uiColour );

		TVector4              vExtents = *pBuilding->m_pTerrainInstance->m_ppCentre[ 1 ];
		const PatternSetting* pSetting = FindPatternSetting( GetPatternSettings(), pBuilding->m_pTerrainInstance->m_szModel );

		pBuilding->m_uiUnkC6 = TUINT8( APlayerManager::GetSingleton()->CyclePattern( iPlayer ) );
		pBuilding->m_fUnkBC  = vExtents.x / pSetting->m_fSizeX;
		pBuilding->m_fUnkC0  = vExtents.y / pSetting->m_fSizeY;
	}

	return iPainted;
}

// $deBlob: FUNCTION 00579910
void ABuildingManager::PaintAllBuildings( TINT a_iColour )
{
	PaintAllBuildings( TUINT8( a_iColour ), ABlobColourComponent::ms_auiColours[ a_iColour ] );
}

// $deBlob: FUNCTION 00579930
void ABuildingManager::PaintAllBuildings( TUINT8 a_uiColour, TUINT32 a_uiARGB )
{
	for ( TINT i = 0; i < m_iNumBuildings; i++ )
	{
		ABuildingInstance* pBuilding = m_aInstancePool[ i ];

		if ( !pBuilding->IsAPaintableBuilding() )
			continue;

		pBuilding->m_iMultiplayerOwner = -1;
		pBuilding->m_uiFlags           = ( pBuilding->m_uiFlags & ~ABuildingInstance::Flags_Inked ) | ABuildingInstance::Flags_Painted;
		pBuilding->SetLocalAmbientColour( a_uiARGB );
		pBuilding->m_uiCurrentColour = a_uiColour;

		for ( TINT j = 0; j < ABuildingInstance::MAXBLOCKS; j++ )
		{
			PaintableBlock* pBlock = TSTATICCAST( PaintableBlock, pBuilding->m_apBlocks[ j ] );

			if ( pBlock != TNULL )
			{
				OnBlockBuildingPainted( pBuilding, -1, *pBlock );

				if ( pBlock->GetType() == 7 )
					RemoveJellyInstance( pBuilding );
			}
		}
	}
}

// $deBlob: FUNCTION 00579ba0
void ABuildingManager::SetupPaintableBlocks()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();

	TASSERT( m_apPaintableBlocks == TNULL );
	m_apPaintableBlocks = TREINTERPRETCAST( PaintableBlockList*, ACurrentLevelInformation::GetSingleton()->Stream( ACurrentLevelInformation::DataBlock_PaintableBlockData )->GetSymbolAddress( "PaintableBlocks" ) );
	TASSERT( m_pBlockParticleEffects == TNULL );
	TASSERT( m_iNumBlockParticleEffects == 0 );

	for ( TINT iBlock = 0; iBlock < m_apPaintableBlocks->m_iNumBlocks; iBlock++ )
	{
		PaintableBlock* pBlock = &m_apPaintableBlocks->m_pBlocks[ iBlock ];

		// The most instances of a block (the number of graphic effects to make)
		if ( m_iNumBlockParticleEffects < pBlock->m_uiNumInstances )
			m_iNumBlockParticleEffects = pBlock->m_uiNumInstances;

		pBlock->m_vPosition = Toshi::TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

		// The instances that aren't type 7 (the ZJumps) become their buildings, which take the block and the centre of
		// their terrain instance into the block's position
		for ( TUINT16 iInstance = 0; iInstance < pBlock->m_uiNumInstances; iInstance++ )
		{
			TerrainTypes::CellInstance* pCell = pTerrain->GetCellInstance( pBlock->m_paiInstances[ iInstance ] );

			if ( ( pCell->m_uiFlags & 7 ) == 7 )
				continue;

			pBlock->m_paiInstances[ iInstance ] = TREINTERPRETCAST( TINT, pCell->m_pModelInstance2 );

			ABuildingInstance* pBuilding = TREINTERPRETCAST( ABuildingInstance*, pBlock->m_paiInstances[ iInstance ] );
			Toshi::TVector4*   pCentre   = *pBuilding->GetTerrainInstance()->m_ppCentre;

			pBlock->m_vPosition.x += pCentre->x;
			pBlock->m_vPosition.y += pCentre->y;
			pBlock->m_vPosition.z += pCentre->z;

			pBuilding->SetBlock( pBlock );
		}

		if ( pBlock->m_uiNumInstances != 0 )
			pBlock->m_vPosition.Divide( TFLOAT( pBlock->m_uiNumInstances ) );

		// The entities of the block: each one is a spawner (ACivSpawner) whose block is this one. The civilians of the
		// spawners (their linked splines, at least 8 per spawner) are added to the pool of the civilians and to the level
		TINT iNumSplines = 0;

		for ( TUINT16 iEntity = 0; iEntity < pBlock->m_uiNumEntities; iEntity++ )
		{
			ABaseEntity* pEntity = ABaseEntity::FindIDEntity( pBlock->m_pauiEntities[ iEntity ] );

			TASSERT( pEntity->IsA( &TGetClass( ACivSpawner ) ) );

			ACivSpawner* pSpawner = TSTATICCAST( ACivSpawner, pEntity );
			pSpawner->SetBlock( pBlock );

			if ( pSpawner->HasLinkedSplines() )
			{
				iNumSplines += pSpawner->GetNumLinkedSplines();
			}
		}

		// The civilians of the block: 8 per spawner in the block (the spawner count sets 1, 2 or 3 of them) or the
		// linked splines when there are more of them
		if ( pBlock->m_uiNumEntities != 0 && ( pBlock->m_uiFlags & 0xF000000 ) != 0x7000000 )
		{
			TINT iNumCivs;

			if ( pBlock->m_uiNumEntities < 9 )
				iNumCivs = 2 - ( pBlock->m_uiNumEntities < 5 );
			else
				iNumCivs = 3;

			iNumCivs *= 8;

			ASpawnedCivManager::GetSingleton()->m_iNumCivs += iNumCivs > iNumSplines ? iNumCivs : iNumSplines;
			AGameStatus::GetSingleton()->GetLevelData().AddCivilians( iNumCivs );
		}
	}

	m_pBlockGraphicEffect   = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_Block_Graphic" );
	m_pBlockParticleEffects = TREINTERPRETCAST( Toshi::TParticleEffect**, TMemalign( 16, m_iNumBlockParticleEffects * sizeof( Toshi::TParticleEffect* ) ) );

	for ( TINT i = 0; i < m_iNumBlockParticleEffects; i++ )
		m_pBlockParticleEffects[ i ] = m_pBlockGraphicEffect->Copy();
}

// $deBlob: FUNCTION 00579fd0
void ABuildingManager::OnBlockBuildingPainted( ABuildingInstance* a_pBuilding, TINT a_iPlayer, PaintableBlock& a_rBlock )
{
	if ( a_rBlock.GetType() == 7 )
	{
		// A jelly block: its other buildings take the colour of the painted one after a delay (Update)
		for ( TINT i = 0; i < a_rBlock.m_uiNumInstances; i++ )
		{
			ABuildingInstance* pBuilding = TREINTERPRETCAST( ABuildingInstance*, a_rBlock.m_paiInstances[ i ] );

			if ( pBuilding == a_pBuilding )
				continue;

			// Already queued: only its colour changes
			TBOOL bQueued = TFALSE;

			for ( auto it = m_oJellyInstances.Begin(); it != m_oJellyInstances.End(); it++ )
			{
				if ( it->m_pBuilding == pBuilding )
				{
					if ( it->m_uiColour != a_pBuilding->m_uiCurrentColour )
						it->m_uiColour = a_pBuilding->m_uiCurrentColour;

					bQueued = TTRUE;
					break;
				}
			}

			if ( bQueued )
				continue;

			TASSERT( !m_oFreeJellyInstances.IsEmpty() );
			JellyInstance* pJelly = m_oFreeJellyInstances.PopBack();
			pJelly->m_iPlayer     = a_iPlayer;
			pJelly->m_pBuilding   = pBuilding;
			pJelly->m_uiColour    = a_pBuilding->m_uiCurrentColour;
			pJelly->m_bDone       = TFALSE;
			pJelly->m_fDelay      = s_fJellyDelay;
			m_oJellyInstances.PushBack( pJelly );
		}

		return;
	}

	// The block completes once none of its buildings is white or inked
	TBOOL bCompleted = TFALSE;

	if ( !( a_rBlock.m_uiFlags & PaintableBlock::FlagMask_Completed ) && a_rBlock.m_uiNumInstances != 0 )
	{
		TINT i;

		for ( i = 0; i < a_rBlock.m_uiNumInstances; i++ )
		{
			TUINT8 uiBuildingColour = TREINTERPRETCAST( ABuildingInstance*, a_rBlock.m_paiInstances[ i ] )->m_uiCurrentColour;

			if ( uiBuildingColour == ABlobColourComponent::COLOUR_BLACK || uiBuildingColour == ABlobColourComponent::COLOUR_WHITE )
				break;
		}

		if ( i == a_rBlock.m_uiNumInstances )
		{
			bCompleted = TTRUE;
			a_rBlock.m_uiFlags |= PaintableBlock::FlagMask_Completed;
		}
	}

	// The colours of a completed block (while it has fewer than its buildings, at most 15): how many different ones
	// and the one with the fewest buildings (the first slot on a tie), given to the doorways of its spawners
	static const TINT    s_aiColourSlots[ 7 ]  = { 0, 1, 5, 2, 4, 3, 6 }; // the slot of the colours 1 to 7
	static const TUINT32 s_auiSlotColours[ 7 ] = { 1, 2, 4, 6, 5, 3, 7 }; // the colour of the slots

	TUINT32 uiColour    = 1;
	TINT    iNumColours = 0;

	if ( a_rBlock.m_uiFlags & PaintableBlock::FlagMask_Completed )
	{
		TINT iMaxColours = ( a_rBlock.m_uiNumInstances < 15 ) ? a_rBlock.m_uiNumInstances : 15;

		if ( a_rBlock.GetNumColours() < iMaxColours )
		{
			// NOTE: retail clears 15 counters, only 7 are used
			TUINT32 auiCounts[ 7 ] = {};

			for ( TINT i = 0; i < a_rBlock.m_uiNumInstances; i++ )
			{
				TUINT32 uiBuildingColour = TREINTERPRETCAST( ABuildingInstance*, a_rBlock.m_paiInstances[ i ] )->m_uiCurrentColour;

				if ( uiBuildingColour - 1 > 6 )
					continue;

				TINT iSlot = s_aiColourSlots[ uiBuildingColour - 1 ];

				if ( auiCounts[ iSlot ] == 0 )
					iNumColours++;

				auiCounts[ iSlot ]++;
			}

			// NOTE: retail picks the colour with the fewest buildings (unsigned <), not the most
			TINT iSlot = 0;

			for ( TINT i = 1; i < 7; i++ )
			{
				if ( auiCounts[ i ] != 0 && ( auiCounts[ iSlot ] == 0 || auiCounts[ i ] < auiCounts[ iSlot ] ) )
					iSlot = i;
			}

			uiColour = s_auiSlotColours[ iSlot ];
		}
	}

	T2Locale* pLocale = T2Locale::GetSingleton();

	if ( !bCompleted )
	{
		// A completed block repainted in more colours than before: the multicolour bonus
		if ( ( a_rBlock.m_uiFlags & PaintableBlock::FlagMask_Completed ) && iNumColours > a_rBlock.GetNumColours() )
		{
			TINT iBonus = a_rBlock.m_uiNumInstances * TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x0C / 4 ];
			TASSERT( (TUINT8)( a_rBlock.GetType() ) < PaintableBlock::NUMOF );

			const TWCHAR* wszFormat = pLocale->GetString( 0x69 );
			const TWCHAR* wszColour = pLocale->GetString( 0x6A + ( iNumColours != 1 ) );
			const TWCHAR* wszType   = pLocale->GetString( s_aiBlockTypeStrings[ a_rBlock.GetType() ] );
			TStringManager::String16Format( m_wszBlockMessage, wszFormat, wszType, iNumColours, wszColour, iBonus );

			AGameStatus::GetSingleton()->GetLevelData().AddScore( 0, 0, iBonus );
			a_rBlock.SetNumColours( iNumColours );
			APropTriggerHelp::ShowContextHelp( 0x80 );
		}

		return;
	}

	// The block was just completed: the score (its buildings times its colours) and the banner.
	// NOTE: retail doesn't store iNumColours in the block here, so the next repaint of one of its buildings gives the
	// multicolour bonus for the colours it already had
	TINT iScore = a_rBlock.m_uiNumInstances * TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x0C / 4 ] * iNumColours;
	TASSERT( (TUINT8)( a_rBlock.GetType() ) < PaintableBlock::NUMOF );

	const TWCHAR* wszFormat = pLocale->GetString( 0x68 );
	const TWCHAR* wszColour = pLocale->GetString( 0x6A + ( iNumColours != 1 ) );
	const TWCHAR* wszType   = pLocale->GetString( s_aiBlockTypeStrings[ a_rBlock.GetType() ] );
	TStringManager::String16Format( m_wszBlockMessage, wszFormat, wszType, iNumColours, wszColour, iScore );

	AGameStatus::GetSingleton()->GetLevelData().AddScore( 0, iScore, -1 );
	APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, 0x68, -1, TTRUE, 4.0f, 0, m_wszBlockMessage, TFALSE );

	// The entity of the block
	if ( a_rBlock.GetEntityID() >= 0 )
	{
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
		oPacket.m_iData    = 0;

		ABaseEntity::FindIDEntity( a_rBlock.GetEntityID() )->HandleEntityIO( TNULL, oPacket );
	}

	ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x1C ), FMOD_EVENT_DEFAULT, TTRUE );

	// The block graphic on each building, sized by its extents
	TParticleEffectManager* pEffectManager = TParticleSystem::GetSingleton()->GetEffectManager();

	for ( TINT i = 0; i < a_rBlock.m_uiNumInstances; i++ )
	{
		ABuildingInstance* pBuilding = TREINTERPRETCAST( ABuildingInstance*, a_rBlock.m_paiInstances[ i ] );
		TVector4**         ppBounds  = pBuilding->GetTerrainInstance()->m_ppCentre; // [ 0 ] the centre, [ 1 ] the extents

		// NOTE: retail doubles a copy of the extents and then scales the effect by the extents themselves
		TVector4 vSize = *ppBounds[ 1 ];
		vSize.Multiply( 2.0f );

		AParticleEffectHelper::ScalePositionInit( m_pBlockParticleEffects[ i ], m_pBlockGraphicEffect, ppBounds[ 1 ] );
		TParticleEffectInstance* pInstance = pEffectManager->StartEffect( m_pBlockParticleEffects[ i ] );

		if ( pInstance != TNULL )
		{
			TMatrix44 oMatrix;
			oMatrix.Identity();
			oMatrix.m_f41 = ppBounds[ 0 ]->x;
			oMatrix.m_f42 = ppBounds[ 0 ]->y;
			oMatrix.m_f43 = ppBounds[ 0 ]->z;

			pInstance->GetTransform() = oMatrix;
		}
	}

	// The civilians: the doorways of its spawners take the block's colour and the nearest 1 to 3 spawners (by the
	// number of spawners) spawn theirs
	if ( a_rBlock.m_uiNumEntities != 0 )
	{
		ABlobHud::GetSingleton()->QueueUnk25CBanner();

		TINT iNumSpawners;

		if ( a_rBlock.m_uiNumEntities < 9 )
			iNumSpawners = 2 - ( a_rBlock.m_uiNumEntities < 5 );
		else
			iNumSpawners = 3;

		TFLOAT       afDistances[ 3 ] = { TMath::MAXFLOAT, TMath::MAXFLOAT, TMath::MAXFLOAT };
		ACivSpawner* apSpawners[ 3 ]  = { TNULL, TNULL, TNULL };

		TASSERT( 1 == APlayerManager::GetSingleton()->GetNumPlayers() );
		TVector4 vBlobPosition = APlayerManager::GetSingleton()->GetCurrentPlayerBlob()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

		for ( TINT i = 0; i < a_rBlock.m_uiNumEntities; i++ )
		{
			ACivSpawner* pSpawner = TSTATICCAST( ACivSpawner, ABaseEntity::FindIDEntity( a_rBlock.m_pauiEntities[ i ] ) );
			pSpawner->PaintDoorway( uiColour, TTRUE );
			pSpawner->PickSwapTexture();

			// Keeps the nearest ones, sorted
			TFLOAT fDistance = TVector4::DistanceSq( vBlobPosition, pSpawner->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) );

			for ( TINT j = 0; j < iNumSpawners; j++ )
			{
				if ( afDistances[ j ] > fDistance )
				{
					TFLOAT       fSwapDistance = afDistances[ j ];
					ACivSpawner* pSwapSpawner  = apSpawners[ j ];
					afDistances[ j ]           = fDistance;
					apSpawners[ j ]            = pSpawner;
					fDistance                  = fSwapDistance;
					pSpawner                   = pSwapSpawner;
				}
			}
		}

		for ( TINT i = 0; i < iNumSpawners; i++ )
			apSpawners[ i ]->StartSpawning();

		APropTriggerHelp::ShowContextHelp( 0x40 );
	}

	// The SS camera of the block (a fly-by)
	if ( a_rBlock.m_iSSCamera != -1 && AGameStatus::GetSingleton()->GetLevelMode() != AGameStatus::Mode_FreePaint )
		APlayerManager::GetSingleton()->PlaySSCamera( 0, a_rBlock.m_iSSCamera );
}

// Clears the completed and dispersed flags and the colour count of every block
// $deBlob: FUNCTION 0057ab10
void ABuildingManager::ResetPaintableBlocks()
{
	for ( TINT i = 0; i < m_apPaintableBlocks->m_iNumBlocks; i++ )
	{
		m_apPaintableBlocks->m_pBlocks[ i ].m_uiFlags &= ~PaintableBlock::FlagMask_Completed;
		m_apPaintableBlocks->m_pBlocks[ i ].m_uiFlags &= ~PaintableBlock::FlagMask_Dispersed;
		m_apPaintableBlocks->m_pBlocks[ i ].SetNumColours( 0 );
	}
}

// $deBlob: FUNCTION 0057ab50
void ABuildingManager::RemoveJellyInstance( ABuildingInstance* a_pBuilding )
{
	for ( auto it = m_oJellyInstances.Begin(); it != m_oJellyInstances.End(); it++ )
	{
		if ( it->m_pBuilding == a_pBuilding )
		{
			JellyInstance* pJelly = it.Get();
			m_oJellyInstances.Erase( it );
			m_oFreeJellyInstances.PushBack( pJelly );
			return;
		}
	}
}

// $deBlob: FUNCTION 0057a8b0
void ABuildingManager::DisperseCivs( PaintableBlock& a_rBlock, ACivSpawner* a_pSpawner )
{
	TASSERT( a_rBlock.m_uiFlags & PaintableBlock::FlagMask_Completed );
	TASSERT( !( a_rBlock.m_uiFlags & PaintableBlock::FlagMask_Dispersed ) );

	// The civilians left in the spawners; nothing happens while one of them isn't at the low LOD or is spawning
	TINT  iCivsLeft     = 0;
	TBOOL bLinkedSpline = TFALSE;

	for ( TINT i = 0; i < a_rBlock.m_uiNumEntities; i++ )
	{
		ACivSpawner* pSpawner = TSTATICCAST( ACivSpawner, ABaseEntity::FindIDEntity( a_rBlock.m_pauiEntities[ i ] ) );

		if ( pSpawner != a_pSpawner && pSpawner->GetLOD() != ABaseEntity::FLAG_LowLOD )
			return;

		TBOOL bLinked = pSpawner->HasLinkedSplines();

		if ( !bLinked && pSpawner->GetLinkedSplines() != TNULL )
			return;

		if ( ( pSpawner->GetActiveCount() != 0 || pSpawner->GetState() != 0 ) && !( pSpawner->GetSpawnerFlags() & 4 ) )
			return;

		bLinkedSpline |= bLinked;
		iCivsLeft += pSpawner->GetActiveCount();
	}

	// From the last spawner on, each takes its share of the civilians left
	for ( TINT i = a_rBlock.m_uiNumEntities; i > 0; i-- )
	{
		ACivSpawner* pSpawner = TSTATICCAST( ACivSpawner, ABaseEntity::FindIDEntity( a_rBlock.m_pauiEntities[ i - 1 ] ) );

		TINT iNumCivs = TMath::CeilToInt( TFLOAT( iCivsLeft ) / TFLOAT( i ) );
		iCivsLeft -= iNumCivs;
		pSpawner->SetNumCivs( iNumCivs, bLinkedSpline );
	}

	TASSERT( 0 == iCivsLeft );
	a_rBlock.m_uiFlags |= PaintableBlock::FlagMask_Dispersed;
}

// $deBlob: FUNCTION 0057abd0
void ABuildingManager::UncountLandmarkCivs()
{
	ATerrain2* pTerrain = ATerrain2::GetInstance();
	TASSERT( TNULL != pTerrain );

	TINT iNumInstances = pTerrain->GetNumInstances();

	for ( TINT i = 0; i < iNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = pTerrain->GetCellInstance( i );

		if ( ( pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Landmark )
			TSTATICCAST( ALandmarkInstance, pInstance->m_pModelInstance2 )->UncountCivs();
	}
}

// $deBlob: FUNCTION 00751e20
void ABuildingManager::OnInstanceTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition )
{
	switch ( a_pInstance->m_uiFlags & 7 )
	{
		case TerrainTypes::InstanceType_Building:
			ABuildingInstance::OnBlobTouched( a_pInstance, a_pBlob, a_pPosition );
			break;
		case TerrainTypes::InstanceType_Landmark:
			ALandmarkInstance::OnBlobTouched( a_pInstance, a_pBlob, a_pPosition );
			break;
		case TerrainTypes::InstanceType_Tree:
			ATreeInstance::OnBlobTouched( a_pInstance, a_pBlob, a_pPosition );
			break;
		case TerrainTypes::InstanceType_TextureSwap:
			ATextureSwapInstance::OnBlobTouched( a_pInstance, a_pBlob, a_pPosition );
			break;
		case TerrainTypes::InstanceType_ToggleVis:
			AToggleVisInstance::OnBlobTouched( a_pInstance, a_pBlob, a_pPosition );
			break;
	}
}

// $deBlob: FUNCTION 00579a20
void ABuildingManager::TogglePulseAllBuildings()
{
	static TBOOL s_bPulse = TFALSE;
	s_bPulse              = !s_bPulse;

	for ( TINT i = m_iNumBuildings; i != 0; )
	{
		i--;

		if ( !m_aInstancePool[ i ]->IsAPaintableBuilding() )
			continue;

		if ( s_bPulse )
			m_aInstancePool[ i ]->StartPulse( ARandom::GetSingleton()->GetRandom().GetFloat() * 2.0f );
		else
			m_aInstancePool[ i ]->StartScaleBack();
	}
}

// $deBlob: FUNCTION 00579ac0
TBOOL ABuildingManager::GetNearestCivBlockPosition( TVector4& a_rvOutPosition )
{
	// NOTE: retail leaves the camera position uninitialised without a camera manager
#ifdef DEBLOB_COMMUNITY_PATCH
	TVector4 vCameraPosition = TVector4::VEC_ZERO;
#else
	TVector4 vCameraPosition;
#endif

	if ( ACameraManager::GetSingleton() != TNULL )
	{
		ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

		if ( pCamera == TNULL )
			return TFALSE;

		vCameraPosition = pCamera->GetMatrix().GetTranslation();
	}

	PaintableBlock* pNearest     = TNULL;
	TFLOAT          fNearestDist = 0.0f;

	for ( TINT i = 0; i < m_apPaintableBlocks->m_iNumBlocks; i++ )
	{
		PaintableBlock* pBlock = &m_apPaintableBlocks->m_pBlocks[ i ];

		if ( pBlock->m_uiNumEntities != 0 && pBlock->GetType() != 7 && !( pBlock->m_uiFlags & PaintableBlock::FlagMask_Completed ) )
		{
			TFLOAT fDist = TVector4::DistanceSq( vCameraPosition, pBlock->m_vPosition );

			if ( pNearest == TNULL || fDist < fNearestDist )
			{
				pNearest     = pBlock;
				fNearestDist = fDist;
			}
		}
	}

	if ( pNearest == TNULL )
		return TFALSE;

	a_rvOutPosition = pNearest->m_vPosition;
	return TTRUE;
}
