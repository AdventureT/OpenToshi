#include "pch.h"
#include "AGameStatus.h"
#include "AGameControllerHelper.h"
#include "IO/AIOEntity.h"
#include "Entities/APropGrassBomb.h"
#include "GameInterface/APlayerManager.h"
#include "Challenges/AChallengeManager.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Entities/ACivSpawner.h"
#include "Terrain2/ATerrain2.h"
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Platform/DX11/TTexture_DX11.h>
#include "Input/ABlobInputMap.h"
#include "Entities/APropTriggerHelp.h"
#include "XUI/InGame/HUD/ABlobMedalsHud.h"
#include "Entities/APickupManager.h"
#include "Challenges/AChallenge.h"
#include "Fx/AEnvironmentFX.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobAnimationComponent.h"
#include "Entities/ABlobEntity/ABlobColourMaps.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Sound/ASoundInterface.h"
#include "Sound/AMusicManager.h"
#include "GameInterface/AInGameState.h"
#include "GameInterface/AGameStateController.h"
#include "Cameras/ACamera.h"
#include "Entities/APropDoor.h"
#include "MemoryCard/EnSaveData.h"
#include "Entities/AEntityIOPacket.h"
#include "XUI/InGame/HUD/ABlobScoreFloatHud.h"
#include "XUI/InGame/HUD/ABlobScoreHud.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include "XUI/InGame/HUD/ABlobTimerHud.h"
#include "XUI/InGame/HUD/ABlobGoalHud.h"
#include "XUI/InGame/HUD/ABlobLeaderBoardHud.h"
#include "Buildings/ABuildingManager.h"
#include "Render/AXYZViewportManager.h"
#include "Render/ARenderer.h"
#include "XUI/InGame/HUD/ABlobSizeHud.h"
#include "XUI/InGame/HUD/ABlobMultiplayerHud.h"
#include "Render/Decal/ADecalManager.h"
#include "GUI/AGUIManager.h"
#include <T2Locale/T2Locale.h>

#include <cmath>

#include <Toshi/TStringManager.h>
#include <XUI/TXUIControl.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT                      AGameStatus::s_iNumAwards = 13;
AGameStatus::ScoreSummary AGameStatus::s_oScoreSummary;
// $deBlob: FUNCTION 0053c4e0
// $deBlob: FUNCTION 00858480
Toshi::TTRB AGameStatus::s_ScorePropertiesTRB;
void*       AGameStatus::s_pBlobScoringProperties;
TUINT32     AGameStatus::s_uiNumVisiblePaintedInstances;
TUINT32     AGameStatus::s_uiNumVisiblePaintableInstances;
TFLOAT      AGameStatus::s_fGlintLerp;

TSTATICASSERT( sizeof( AGameStatus::PlayerData ) == 0x4C );
TSTATICASSERT( sizeof( AGameStatus::LevelData ) == 0x814 );
TSTATICASSERT( sizeof( AGameStatus ) == 0xA44 );

// $deBlob: FUNCTION 005ba730
AGameStatus::LevelData::LevelData()
{
	// NOTE: the compiler emits the constructors of m_aPlayerData
	m_bMultiplayer = TFALSE;
	m_fTimeLimit   = 900.0f;
	m_fTimeLeft    = 900.0f;
	m_bUnk00C      = TFALSE;

	for ( TINT i = 0; i < 7; i++ )
		m_auiLevelCounts[ i ] = 0;

	m_uiUnk15C = 0;

	for ( TINT i = 0; i < 2; i++ )
		m_auiUnk160[ i ] = 0;

	m_iNumChallenges          = 0;
	m_iNumCompletedChallenges = 0;
	m_uiUnk774                = 0;
	m_iGoal                   = -1;
	m_bGoalCheat              = TFALSE;
	m_uiUnk780                = 0;
	m_iLastScoreFloatHud      = -1;

	m_pGoalHud          = TNULL;
	m_pScoreHud         = TNULL;
	m_pMessageControl   = TNULL;
	m_fEndGraydianFrame = 0.0f;
	m_fRaydianFreeFrame = 0.0f;
	m_pMedalsHud        = TNULL;
	m_fLevelTime        = 0.0f;
	m_fTargetTime       = 0.0f;

	for ( TINT i = 0; i < 4; i++ )
		m_apScoreFloatHud[ i ] = TNULL;
}

// The target times of the SPLevels in seconds
static const TFLOAT s_afSPLevelTargetTimes[ AGameStatus::SPLevel_NUMOF ] = { 480.0f, 810.0f, 1230.0f, 1080.0f, 1020.0f, 1680.0f, 1500.0f, 1020.0f, 1470.0f, 1620.0f };

// $deBlob: FUNCTION 005ba8f0
void AGameStatus::LevelData::Init( ALevelInformation::LevelProperties* a_pLevelProperties )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	switch ( pGameStatus->GetLevelMode() )
	{
		default:
			m_fTimeLimit = 15.0f;
			break;
		case 2:
			m_fTimeLimit = a_pLevelProperties->m_afTimeLimits[ 1 ];
			break;
		case 3:
			m_fTimeLimit = a_pLevelProperties->m_afTimeLimits[ 2 ];
			break;
		case -1:
		case Mode_Story:
			m_fTimeLimit = a_pLevelProperties->m_afTimeLimits[ 0 ];
			break;
	}

	if ( m_fTimeLimit != -1.0f )
		m_fTimeLimit *= 60.0f;

	m_fTimeLeft   = m_fTimeLimit;
	m_fLevelTime  = 0.0f;
	m_fTargetTime = -1.0f;

	if ( pGameStatus->GetLevelMode() != Mode_FreePaint && !m_bMultiplayer && pGameStatus->GetSPLevel() != SPLevel_Invalid )
		m_fTargetTime = s_afSPLevelTargetTimes[ pGameStatus->GetSPLevel() ];

	m_bUnk00C  = TFALSE;
	m_uiUnk15C = 0;
	TStringManager::String16Copy( m_wszUnk170, L"", 0x100 );

	for ( TINT i = 0; i < 4; i++ )
		m_aPlayerData[ i ].Reset();

	m_uiUnk774    = 0;
	m_bGoalCheat  = TFALSE;
	m_iGoal       = -1;
	m_pLevelProps = a_pLevelProperties;

	if ( m_pScoreHud != TNULL )
		m_pScoreHud->Reset();

	if ( ABlobHud::GetSingleton() != TNULL )
		ABlobHud::GetSingleton()->GetLevelTimer()->ResetWarning();

	if ( m_pMedalsHud != TNULL )
		TSTATICCAST( ABlobMedalsHud, m_pMedalsHud )->ShowNormal();

	m_iLastScoreFloatHud      = -1;
	m_iNumCompletedChallenges = 0;

	if ( m_pMessageControl != TNULL )
		m_pMessageControl->GetVisual()->PlayAnimation( L"EndNormal", L"EndNormal" );
}

// $deBlob: FUNCTION 005c02b0
AGameStatus::PlayerData::PlayerData()
{
	for ( TINT i = 0; i < 19; i++ )
		m_auiUnk00[ i ] = 0;
}

// $deBlob: FUNCTION 005c0b00
void AGameStatus::PlayerData::Reset()
{
	for ( TINT i = 0; i < 19; i++ )
		m_auiUnk00[ i ] = 0;
}

// $deBlob: FUNCTION 005bc310
void AGameStatus::LevelData::ClearCounts()
{
	m_auiLevelCounts[ 4 ]     = 0;
	m_auiLevelCounts[ 5 ]     = 0;
	m_auiLevelCounts[ 6 ]     = 0;
	m_auiUnk160[ 0 ]          = 0;
	m_auiUnk160[ 1 ]          = 0;
	m_uiUnk15C                = 0;
	m_auiLevelCounts[ 3 ]     = 0;
	m_auiLevelCounts[ 2 ]     = 0;
	m_auiLevelCounts[ 1 ]     = 0;
	m_auiLevelCounts[ 0 ]     = 0;
	m_iNumCompletedChallenges = 0;
	m_iNumChallenges          = 0;
	m_uiUnk780                = 0;
}

// $deBlob: FUNCTION 005bc3f0
AGameStatus::AGameStatus()
    : m_vecCheckPoints( GetGlobalAllocator(), 64, 0 )
{
	m_bUnk000              = TFALSE;
	m_pItMarker            = TNULL;
	m_uiUnk008             = 0;
	m_bUnk00C              = TFALSE;
	m_iCurrentLevel        = -1;
	m_iCurrentCheckpoint   = -1;
	m_iMultiPlayerGameType = -1;

	m_bLevelComplete      = TFALSE;
	m_bUnk845             = TFALSE;
	m_iLeader             = -1;
	m_iNumFreeProjections = -1;

	m_SPLevel              = -1;
	m_iSelectedWorld       = -1;
	m_iSelectedSlot        = -1;
	m_eLevelMode           = Mode_Story;
	m_uiSelectedGallery    = 0;
	m_pLevelStartGate      = TNULL;
	m_pSprintGate          = TNULL;
	m_bLevelStart          = TFALSE;
	m_uiLeaderBoardHud     = 0;
	m_uiBossGrassBomb      = 0;
	m_bPlaceFirstBuildings = TFALSE;
	m_iNumFreeProjections  = -1;

	for ( TINT i = 6; i != 0; )
		m_apNextBuildingPaintable[ --i ] = TNULL;

	if ( s_ScorePropertiesTRB.Load( "Data/Scoring/ScoreProperties.trb" ) == TTRB::ERROR_OK )
		s_pBlobScoringProperties = s_ScorePropertiesTRB.GetSymbolAddress( "BlobScoringProperties" );
	else
		s_pBlobScoringProperties = TNULL;

	m_uiUnk9F8                       = 0;
	m_uiUnk9FC                       = 0;
	s_uiNumVisiblePaintedInstances   = 0;
	s_uiNumVisiblePaintableInstances = 0;
}

// $deBlob: FUNCTION 005bc5d0
// $deBlob: FUNCTION 0056c040
AGameStatus::~AGameStatus()
{
	m_vecCheckPoints.Clear();
	s_ScorePropertiesTRB.Close();
}

// $deBlob: FUNCTION 005bc8f0
TBOOL AGameStatus::SetCurrentLevel( TINT a_iLevel )
{
	if ( a_iLevel < -1 )
		return TFALSE;

	s_uiNumVisiblePaintedInstances   = 0;
	s_uiNumVisiblePaintableInstances = 0;
	m_iCurrentLevel                  = a_iLevel;
	m_iNumFreeProjections            = -1;
	m_bUnk00C                        = TFALSE;
	return TTRUE;
}

// $deBlob: FUNCTION 005bc930
TBOOL AGameStatus::SetCurrentCheckpoint( TINT a_iCheckpoint )
{
	// NOTE: retail's check can never be true (a checkpoint below -1 is never out of range), so any value is set
	if ( a_iCheckpoint < -1 && m_vecCheckPoints.Size() <= a_iCheckpoint )
		return TFALSE;

	m_iCurrentCheckpoint = a_iCheckpoint;
	return TTRUE;
}

// Sends the level's start checkpoint (the IO entity "CheckPoint1") the input message 0x3A
// $deBlob: FUNCTION 005bc950
TBOOL AGameStatus::StartAtCheckpoint()
{
	AIOEntity* pCheckPoint = AGameControllerHelper::GetCheckPointStartEntity();

	if ( pCheckPoint == TNULL )
		return TFALSE;

	pCheckPoint->SendInputMessage( 0x3A, TNULL );
	return m_vecCheckPoints.Size() != 0;
}

// The current checkpoint is the last activated one (-1: none)
// $deBlob: FUNCTION 005bf9d0
void AGameStatus::UpdateCurrentCheckPoint()
{
	AGameStatus* pStatus = GetSingleton();
	TASSERT( TNULL != pStatus, "AGameStatus::UpdateCurrentCheckPoint() - AGameStatus not created yet\n" );

	TINT iNumCheckPoints = pStatus->m_vecCheckPoints.Size();
	TINT i               = iNumCheckPoints - 1;

	for ( ; i >= 0; i-- )
	{
		if ( pStatus->m_vecCheckPoints[ i ].m_bActivated == TTRUE )
			break;
	}

	if ( i >= -1 || iNumCheckPoints > i )
		pStatus->m_iCurrentCheckpoint = i;
}

// $deBlob: FUNCTION 005bca50
AGameStatus::CheckPoint* AGameStatus::GetCheckPoint( TINT a_iIndex )
{
	TINT i = 0;

	for ( auto it = m_vecCheckPoints.Begin(); it != m_vecCheckPoints.End(); it++, i++ )
	{
		if ( i == a_iIndex )
			return &( *it );
	}

	return TNULL;
}

// $deBlob: FUNCTION 005bfa40
AGameStatus::CheckPoint* AGameStatus::FindCheckPoint( AIOEntity* a_pEntity )
{
	for ( auto it = m_vecCheckPoints.Begin(); it != m_vecCheckPoints.End(); it++ )
	{
		if ( it->m_pEntity == a_pEntity )
			return &( *it );
	}

	return TNULL;
}

// $deBlob: FUNCTION 005bc9f0
void AGameStatus::ResetLevelReferences()
{
	m_oPointData.Clear();
	m_uiBossGrassBomb = 0;
	m_bLevelStart     = TFALSE;
	m_pLevelStartGate = TNULL;
	m_pSprintGate     = TNULL;
	m_oLevelData.ClearCounts();

	for ( TINT i = 6; i != 0; )
		m_apNextBuildingPaintable[ --i ] = TNULL;
}

// The IO action 0x38: activates the checkpoint of a_pEntity
// $deBlob: FUNCTION 005bd060
void __stdcall AGameStatus::OnCheckPointActivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	CheckPoint* pCheckPoint = GetSingleton()->FindCheckPoint( a_pEntity );

	if ( pCheckPoint != TNULL )
	{
		pCheckPoint->m_bActivated = TTRUE;

		if ( GetSingleton() != TNULL )
			UpdateCurrentCheckPoint();
	}
}

// The IO action 0x39: deactivates the checkpoint of a_pEntity
// $deBlob: FUNCTION 005bd090
void __stdcall AGameStatus::OnCheckPointDeactivate( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	CheckPoint* pCheckPoint = GetSingleton()->FindCheckPoint( a_pEntity );

	if ( pCheckPoint != TNULL )
	{
		pCheckPoint->m_bActivated = TFALSE;

		if ( GetSingleton() != TNULL )
			UpdateCurrentCheckPoint();
	}
}

// The IO action 0x3A: an input with the parameter 1 registers a_pEntity as a checkpoint (at most 64); the action's
// parameter 1 sends the input message 0x3A to its single target
// $deBlob: FUNCTION 005bd0c0
void __stdcall AGameStatus::OnCheckPointRegister( void* a_pAction, void* a_pInvoker, AIOEntity* a_pEntity, void* a_pOther )
{
	TINT* piParams = *TREINTERPRETCAST( TINT**, TSTATICCAST( TUINT8, a_pAction ) + 8 );

	if ( **TREINTERPRETCAST( TINT**, TSTATICCAST( TUINT8, a_pInvoker ) + 0x18 ) == 1 )
	{
		AGameStatus* pStatus = GetSingleton();

		if ( pStatus->m_vecCheckPoints.Size() != 64 )
		{
			CheckPoint oCheckPoint;
			oCheckPoint.m_pEntity    = a_pEntity;
			oCheckPoint.m_bActivated = TFALSE;
			pStatus->m_vecCheckPoints.PushBack( oCheckPoint );
		}
	}

	if ( piParams[ 0 ] == 1 )
	{
		AIOEntity::TargetList* pTargets = a_pEntity->FindTargetList( a_pInvoker, a_pAction );

		if ( pTargets != TNULL && pTargets->iNumTargets == 1 )
		{
			ABaseEntity* pTarget = pTargets->pTargets[ 0 ].pEntity;

			if ( pTarget != TNULL && pTarget->IsA( &TGetClass( AIOEntity ) ) == TTRUE )
				pTarget->CastTo<AIOEntity>()->SendInputMessage( 0x3A, TNULL );
		}
	}
}

// $deBlob: FUNCTION 005bd2e0
void AGameStatus::InitLevelData( ALevelInformation::LevelProperties* a_pLevelProperties )
{
	m_oLevelData.m_uiUnk774     = 0;
	m_oLevelData.m_iGoal        = -1;
	m_oLevelData.m_bGoalCheat   = TFALSE;
	m_oLevelData.m_bMultiplayer = TFALSE;

	if ( TStringManager::String8CompareNoCase( a_pLevelProperties->m_szName, "mp_", 3 ) == 0 )
		m_oLevelData.m_bMultiplayer = TTRUE;

	m_oLevelData.Init( a_pLevelProperties );

	if ( !m_oLevelData.m_bMultiplayer )
	{
		m_iMultiPlayerGameType = -1;

		if ( APlayerManager::GetSingleton() != TNULL )
			APlayerManager::GetSingleton()->SetNumPlayers( 1 );
	}
	else
	{
		m_oLevelData.m_bUnk00C = TTRUE;
		TASSERT( m_iMultiPlayerGameType != GAMETYPE_MP_NONE );
	}

	for ( TINT i = 6; i != 0; )
		m_apNextBuildingPaintable[ --i ] = TNULL;

	m_bLevelComplete = TFALSE;
	m_bUnk845        = TFALSE;
	m_iLeader        = -1;
	m_bLevelStart    = TFALSE;

	for ( TINT i = 0; i < 4; i++ )
	{
		m_aoRanks[ i ].m_iPlayer  = i;
		m_aoRanks[ i ].m_iRank    = 1;
		m_aoRanks[ i ].m_iScore   = 0;
		m_aoRanks[ i ].m_fPercent = 0.0f;
	}
}

// The levels of the SPLevels
static const TCHAR* s_aszSPLevelNames[ AGameStatus::SPLevel_NUMOF ] = {
	"LastResort", "Bridge", "Slums", "Dam", "Guggentraz", "Inkydocks", "Abyss", "Ink_Harvest", "Military_02", "Spaceship"
};

// $deBlob: FUNCTION 005bc6c0
const TCHAR* AGameStatus::GetSPLevelName( TINT a_eLevel )
{
	TASSERT( a_eLevel >= SPLevel_LastResort && a_eLevel < SPLevel_NUMOF );
	return s_aszSPLevelNames[ a_eLevel ];
}

// The sprint levels; NOTE: the same table is in ABlobLevelSelection.cpp and ABlobDebugLevelSelect.cpp
static const TCHAR* s_aszSprintLevelNames[ AGameStatus::SprintLevel_NUMOF ] = {
	"Blob_In_Venice_Sprint",
	"Top_Of_The_World_Sprint",
	"Ship2Ship_Sprint",
	"Cannons_Above_Sprint",
	"HotPlates_R_US_Sprint",
	"Lost_Highway_Sprint",
	"Inky_Academy_Sprint",
	"Mile_High_Fight_Club_Sprint",
	"Hells_Angels_Attack_Sprint",
	"Trainspotting_Sprint",
	"Miles_O_Silos_Sprint",
	"Forest_Sprint",
	"Billboard_Orama_Sprint",
	"Full_Metal_Inky_Sprint",
	"Stamps_Sprint",
	"Inkyball_Sprint",
	"Hill_Climb_Sprint",
	"Deep_Down_Sprint",
	"St_Peters_Square_Sprint",
	"Tower_Of_Babel_Sprint",
};

// $deBlob: FUNCTION 005bc720
const TCHAR* AGameStatus::GetSprintLevelName( TINT a_eSprintLevel )
{
	TASSERT( a_eSprintLevel >= 0 && a_eSprintLevel < SprintLevel_NUMOF );
	return s_aszSprintLevelNames[ a_eSprintLevel ];
}

// The length of a_szName without its extension, -1 without one
static TSIZE GetLevelNameLength( const TCHAR* a_szName )
{
	const TCHAR* szExtension = T2String8::FindString( a_szName, "." );

	if ( szExtension == TNULL )
		return TSIZE( -1 );

	return T2String8::Length( a_szName ) - T2String8::Length( szExtension );
}

// $deBlob: FUNCTION 005bc780
const TCHAR* AGameStatus::FindLevelName( const TCHAR* a_szName )
{
	for ( TINT i = 0; i < SPLevel_NUMOF; i++ )
	{
		if ( T2String8::CompareNoCase( s_aszSPLevelNames[ i ], a_szName, GetLevelNameLength( a_szName ) ) == 0 )
			return s_aszSPLevelNames[ i ];
	}

	for ( TINT i = 0; i < SprintLevel_NUMOF; i++ )
	{
		if ( T2String8::CompareNoCase( s_aszSprintLevelNames[ i ], a_szName, GetLevelNameLength( a_szName ) ) == 0 )
			return s_aszSprintLevelNames[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 005bc830
TBOOL AGameStatus::IsPaintSlot( TINT a_iSlot )
{
	return a_iSlot == 0 || a_iSlot == 1 || a_iSlot == 2 || a_iSlot == 3 || a_iSlot == 5 || a_iSlot == 9 ||
	    a_iSlot == 10 || a_iSlot == 11 || a_iSlot == 12 || a_iSlot == 16 || a_iSlot == 17;
}

// $deBlob: FUNCTION 005bc880
TBOOL AGameStatus::IsCombatSlot( TINT a_iSlot )
{
	return a_iSlot == 6 || a_iSlot == 7 || a_iSlot == 13 || a_iSlot == 15;
}

// $deBlob: FUNCTION 005bc8b0
TBOOL AGameStatus::IsRaceSlot( TINT a_iSlot )
{
	return a_iSlot == 4 || a_iSlot == 8 || a_iSlot == 14 || a_iSlot == 19;
}

// $deBlob: FUNCTION 005bc8e0
TBOOL AGameStatus::IsLandmarkSlot( TINT a_iSlot )
{
	if ( a_iSlot == 18 )
		return TTRUE;

	return TFALSE;
}

// NOTE: BlobScoringProperties isn't ported as a struct yet: 0x00 the combo time, 0x04 the factor of a repainted
// building, 0x08 the factor of a multiplayer one, 0x0C the score of a building, 0x10 the one of flag 0x40000, 0x14-0x2C
// the combo multipliers
#define SCORING_INT( OFFSET )   ( TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ ( OFFSET ) / 4 ] )
#define SCORING_FLOAT( OFFSET ) ( TREINTERPRETCAST( TFLOAT*, AGameStatus::GetBlobScoringProperties() )[ ( OFFSET ) / 4 ] )

// $deBlob: FUNCTION 005bcb80
// 0xA8/0xAC of BlobScoringProperties: the score of inking a player, and of inking the leader
void AGameStatus::OnPlayerKilled( TINT a_iKillerPlayerNumber, TINT a_iPlayerToBeKilled )
{
	TASSERT( a_iKillerPlayerNumber != a_iPlayerToBeKilled );
	TASSERT( a_iPlayerToBeKilled < APlayerManager::GetSingleton()->GetNumPlayers() );

	TBOOL bLeader = TTRUE;
	TINT  iPoints = m_oLevelData.GetPoints( a_iPlayerToBeKilled );

	for ( TINT i = 0; i < APlayerManager::GetSingleton()->GetNumPlayers(); i++ )
	{
		if ( i != a_iPlayerToBeKilled && iPoints < m_oLevelData.GetPoints( i ) )
			bLeader = TFALSE;
	}

	m_oLevelData.AddScore( a_iKillerPlayerNumber, bLeader ? SCORING_INT( 0xAC ) : SCORING_INT( 0xA8 ), -1 );
}

// $deBlob: FUNCTION 005bcd10
void AGameStatus::OnPlayerDied( TINT a_iPlayer )
{
	if ( m_iMultiPlayerGameType != GAMETYPE_MP_NONE && m_iMultiPlayerGameType != 1 )
		m_oLevelData.AddScore( a_iPlayer, SCORING_INT( 0xB0 ), -1 );
}

// $deBlob: FUNCTION 005baea0
TBOOL AGameStatus::LevelData::IsTimeUp() const
{
	return m_fTimeLeft <= 0.0f;
}

// $deBlob: FUNCTION 005baeb0
TBOOL AGameStatus::LevelData::IsWithinTargetTime() const
{
	if ( m_fTargetTime != -1.0f )
		return m_fTargetTime >= m_fLevelTime;

	return TFALSE;
}

// $deBlob: FUNCTION 005baee0
void AGameStatus::LevelData::OnBuildingPainted( ABuildingInstance* a_pBuilding, TINT a_iPlayer, const TVector4* a_pPosition, TINT a_iBonus, TBOOL a_bNoPopup )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );

	TINT iGameType = AGameStatus::GetSingleton()->GetMultiPlayerGameType();

	if ( ( !( a_pBuilding->m_uiFlags & ABuildingInstance::Flags_Painted ) && !( a_pBuilding->m_uiFlags & ABuildingInstance::Flags_PaintedOnce ) ) || iGameType != -1 )
	{
		TerrainTypes::CellInstance* pInstance = a_pBuilding->m_pTerrainInstance;

		if ( !( pInstance->m_uiFlags & 0x40000 ) && pInstance->m_szModel != TNULL && *pInstance->m_szModel != '\0' )
		{
			TINT& rPainted = m_aPlayerData[ a_iPlayer ].m_iBuildingsPainted;
			rPainted++;
			TASSERT( m_aPlayerData[ a_iPlayer ].m_iBuildingsPainted <= TINT( m_auiLevelCounts[ 0 ] ) );
			TMath::Clip( rPainted, 0, TINT( m_auiLevelCounts[ 0 ] ) );
		}
	}

	TerrainTypes::CellInstance* pInstance = a_pBuilding->m_pTerrainInstance;
	TINT                        iScore    = ( pInstance->m_uiFlags & 0x40000 ) ? SCORING_INT( 0x10 ) : SCORING_INT( 0x0C );
	TINT                        iPoints;
	TINT                        iBonus;

	if ( iGameType < 0 )
	{
		iPoints = ( a_pBuilding->m_uiFlags & ABuildingInstance::Flags_Painted ) ? TINT( TFLOAT( iScore ) * SCORING_FLOAT( 0x04 ) ) : iScore;
		iBonus  = a_iBonus;
	}
	else
	{
		a_iBonus = 0;

		// The building's own (multiplayer) score replaces it
		if ( a_pBuilding->m_pTerrainInstance != TNULL && a_pBuilding->Get_MP_Score() >= 0 )
			iScore = a_pBuilding->Get_MP_Score();

		iPoints = ( a_pBuilding->m_uiFlags & ABuildingInstance::Flags_Painted ) ? iScore : TINT( std::lrint( TFLOAT( iScore ) * SCORING_FLOAT( 0x08 ) ) );
		iBonus  = 0;
	}

	if ( a_bNoPopup )
	{
		AddPoints( a_iPlayer, iPoints + iBonus );
		return;
	}

	TINT iMultiplier = 1;

	if ( iGameType == -1 && m_pLevelProps->m_sprintLevel == 0 && m_aPlayerData[ a_iPlayer ].m_iComboState == 1 )
		iMultiplier = GetComboMultiplier( m_aPlayerData[ a_iPlayer ].m_iComboCount );

	ABlobScoreFloatHud* pFloatHud = GetScoreFloatHud( a_iPlayer );

	if ( pFloatHud != TNULL && m_pLevelProps->m_sprintLevel == 0 )
	{
		const TWCHAR* wszBonusName = L"";

		if ( a_iBonus > 0 )
			wszBonusName = T2Locale::GetSingleton()->GetString( 29 );
		else if ( a_iBonus < 0 )
			wszBonusName = T2Locale::GetSingleton()->GetString( 30 );

		pFloatHud->ShowScore( iPoints, iMultiplier, L"", *a_pPosition, wszBonusName, a_iBonus );
	}

	AddScore( a_iPlayer, iPoints, a_iBonus + iPoints * ( iMultiplier - 1 ) );

	if ( m_aPlayerData[ a_iPlayer ].m_iComboState == 1 )
		m_aPlayerData[ a_iPlayer ].m_uiComboTime = SCORING_INT( 0x00 );
}

// $deBlob: FUNCTION 005bb300
void AGameStatus::LevelData::AddKillScore( TINT a_iPlayer, const TVector4& a_rPosition, TINT a_iScore )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );

	TINT iMultiplier = 1;

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 3 && m_aPlayerData[ a_iPlayer ].m_iComboState == 2 )
		iMultiplier = GetComboMultiplier( m_aPlayerData[ a_iPlayer ].m_iComboCount );

	ABlobScoreFloatHud* pFloatHud = GetScoreFloatHud( a_iPlayer );

	if ( pFloatHud != TNULL )
		pFloatHud->ShowScore( a_iScore, iMultiplier, L"", a_rPosition, L"", 0 );

	AddScore( a_iPlayer, a_iScore, a_iScore * ( iMultiplier - 1 ) );

	if ( m_aPlayerData[ a_iPlayer ].m_iComboState == 2 )
		m_aPlayerData[ a_iPlayer ].m_uiComboTime = SCORING_INT( 0x00 );
}

// $deBlob: FUNCTION 005bb140
void AGameStatus::LevelData::OnBuildingUnpainted( ABuildingInstance* a_pBuilding, TINT a_iPlayer, TBOOL a_bScore, const TVector4* a_pPosition, TINT a_iBonus )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );

	if ( a_bScore )
	{
		PlayerData& rPlayer     = m_aPlayerData[ a_iPlayer ];
		TINT        iMultiplier = 1;

		if ( rPlayer.m_iComboState == 4 )
		{
			iMultiplier           = GetComboMultiplier( rPlayer.m_iComboCount );
			rPlayer.m_uiComboTime = SCORING_INT( 0x00 );
		}

		TINT iScore = -( ( a_pBuilding->m_pTerrainInstance->m_uiFlags & 0x40000 ) ? SCORING_INT( 0x10 ) : SCORING_INT( 0x0C ) );

		ABlobScoreFloatHud* pFloatHud = GetScoreFloatHud( a_iPlayer );

		if ( pFloatHud != TNULL )
		{
			const TWCHAR* wszName = TNULL;

			if ( iMultiplier >= 2 && iMultiplier <= 8 )
				wszName = T2Locale::GetSingleton()->GetString( 19 + ( iMultiplier - 2 ) );

			const TWCHAR* wszBonusName = L"";

			if ( a_iBonus > 0 )
				wszBonusName = T2Locale::GetSingleton()->GetString( 29 );
			else if ( a_iBonus < 0 )
				wszBonusName = T2Locale::GetSingleton()->GetString( 30 );

			pFloatHud->ShowScore( iScore, iMultiplier, wszName, *a_pPosition, wszBonusName, a_iBonus );
		}

		AddScore( a_iPlayer, iScore, 0 );
		return;
	}

	if ( !( a_pBuilding->m_pTerrainInstance->m_uiFlags & 0x40000 ) )
	{
		TINT& rPainted = m_aPlayerData[ a_iPlayer ].m_iBuildingsPainted;
		rPainted--;
		TASSERT( m_aPlayerData[ a_iPlayer ].m_iBuildingsPainted >= 0 );
		TMath::Clip( rPainted, 0, TINT( m_auiLevelCounts[ 0 ] ) );
	}
}

// A civilian freed by a_iPlayer: counts it, scores a_iScore with the combo multiplier (state 3) floating at
// a_pPosition and shows the count of the civilians on m_pMessageControl
// $deBlob: FUNCTION 005bb3d0
void AGameStatus::LevelData::OnCivFreed( TINT a_iPlayer, const TVector4* a_pPosition, TINT a_iScore )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );

	TINT iMultiplier = 1;

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() != 3 && m_aPlayerData[ a_iPlayer ].m_iComboState == 3 )
		iMultiplier = GetComboMultiplier( m_aPlayerData[ a_iPlayer ].m_iComboCount );

	ABlobScoreFloatHud* pFloatHud = GetScoreFloatHud( a_iPlayer );

	if ( pFloatHud != TNULL )
		pFloatHud->ShowScore( a_iScore, iMultiplier, L"", *a_pPosition, L"", 0 );

	AddScore( a_iPlayer, a_iScore, ( iMultiplier - 1 ) * a_iScore );

	if ( m_aPlayerData[ a_iPlayer ].m_iComboState == 3 )
		m_aPlayerData[ a_iPlayer ].m_uiComboTime = SCORING_INT( 0x00 );

	// 0x14: the civilians freed, 0x158: the civilians of the level
	m_aPlayerData[ a_iPlayer ].m_auiProgress[ 2 ]++;

	TWCHAR* wszText = TStringManager::GetTempString16();
	TStringManager::String16Format( wszText, 0x80, L"%d/%d\n%ls", m_aPlayerData[ a_iPlayer ].m_auiProgress[ 2 ], m_auiLevelCounts[ 6 ], T2Locale::GetSingleton()->GetString( 0xCC ) );

	if ( m_pMessageControl == TNULL )
		return;

	TStringManager::String16Copy( m_wszMessage, wszText, 0x100 );
	m_pMessageControl->SetPresenterText( m_wszMessage, 0, -1 );
	m_pMessageControl->SetVisible( TTRUE );
	TSTATICCAST( TXUIControl, ABlobHud::GetSingleton()->GetLifeHud() )->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );

	// Already playing between "RaydianFree" and "EndGraydian": back to "RaydianFree"
	TXUIVisual*          pVisual   = m_pMessageControl->GetVisual();
	TXUITimelineControl* pTimeline = pVisual->GetTimelineControl();

	if ( pTimeline->IsPlaying() && m_fRaydianFreeFrame <= pTimeline->GetTimeLineTime() && pTimeline->GetTimeLineTime() < m_fEndGraydianFrame )
	{
		pTimeline->SetTimeLineTime( m_fRaydianFreeFrame, TTRUE );
		return;
	}

	pVisual->PlayAnimation( L"Graydian", L"EndGraydian" );
}

// A landmark completed (a_iCount, -1 to uncount) by a_iPlayer
// $deBlob: FUNCTION 005bb900
void AGameStatus::LevelData::AddLandmarksCompleted( TINT a_iPlayer, TINT a_iCount )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );

	// 0x1C: m_iLandmarksCompleted, 0x148: m_iNumLandmarks
	TINT& rCompleted = TREINTERPRETCAST( TINT&, m_aPlayerData[ a_iPlayer ].m_auiProgress[ 4 ] );
	rCompleted += a_iCount;

	TASSERT( rCompleted >= 0 && rCompleted <= TINT( m_auiLevelCounts[ 2 ] ) );

	if ( rCompleted >= TINT( m_auiLevelCounts[ 2 ] ) )
		rCompleted = TINT( m_auiLevelCounts[ 2 ] );
}

// $deBlob: FUNCTION 005bb600
void AGameStatus::LevelData::AddCivReleased( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	m_aPlayerData[ a_iPlayer ].m_auiProgress[ 3 ]++;
}

// $deBlob: FUNCTION 005bb660
void AGameStatus::LevelData::AddScore( TINT a_iPlayer, TINT a_iScore, TINT a_iBonus )
{
	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 )
	{
		if ( m_pScoreHud != TNULL )
		{
			m_pScoreHud->AddScore( a_iScore, a_iBonus, m_aPlayerData[ a_iPlayer ].m_iComboCount == 1, SCORING_FLOAT( 0x00 ) );
		}
		else
		{
			AddPoints( a_iPlayer, a_iScore + a_iBonus );
		}

		AMusicManager::GetSingleton()->AddScore( a_iScore );
	}
	else
	{
		TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
		AddPoints( a_iPlayer, a_iScore );
	}

	APropTriggerHelp::ShowContextHelp( 0x8000 ); // the help popups

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() >= 0 )
		AGameStatus::GetSingleton()->UpdateRanks();
}

// $deBlob: FUNCTION 005bb590
void AGameStatus::LevelData::AddCivsFreed( TINT a_iPlayer, TINT a_iCount )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );

	TINT& rFreed = TREINTERPRETCAST( TINT&, m_aPlayerData[ a_iPlayer ].m_auiProgress[ 2 ] );
	rFreed += a_iCount;
	TMath::Clip( rFreed, 0, TINT( m_auiLevelCounts[ 6 ] ) );
}

// $deBlob: FUNCTION 005bb750
void AGameStatus::LevelData::AddPoints( TINT a_iPlayer, TINT a_iPoints )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( ABlobInputMap::GetSingleton()->IsDisabled() && pGameStatus->GetMultiPlayerGameType() != GAMETYPE_MP_NONE )
		return;

	TINT& rPoints = m_aPlayerData[ a_iPlayer ].m_iPoints;
	rPoints += a_iPoints;

	if ( rPoints < 0 )
		rPoints = 0;

	if ( pGameStatus->GetMultiPlayerGameType() == GAMETYPE_MP_NONE )
	{
		// The grass bombs waiting for this score (sorted by it) come up, except the boss level's
		APropGrassBomb::BombList&          rList = *TREINTERPRETCAST( APropGrassBomb::BombList*, &m_oList794 );
		APropGrassBomb::BombList::Iterator it    = rList.Begin();
		APropGrassBomb::BombList::Iterator itEnd = rList.End();

		while ( it != itEnd )
		{
			APropGrassBomb* pBomb = it->m_pOwner;
			++it;

			if ( pGameStatus->GetBossGrassBomb() != pBomb )
			{
				if ( pBomb->GetRequiredScore() > rPoints )
					break;

				pBomb->TryActivate();
			}
		}
	}

	if ( pGameStatus->GetMultiPlayerGameType() >= 0 )
		pGameStatus->UpdateRanks();
}

// $deBlob: FUNCTION 005bb9b0
void AGameStatus::LevelData::IncrementUnk28( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	m_aPlayerData[ a_iPlayer ].m_auiProgress[ 7 ]++;
}

// $deBlob: FUNCTION 005bba10
void AGameStatus::LevelData::SetUnk2C( TINT a_iPlayer, TUINT32 a_uiValue )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	m_aPlayerData[ a_iPlayer ].m_auiProgress[ 8 ] = a_uiValue;
}

// $deBlob: FUNCTION 005bba80
void AGameStatus::LevelData::SetUnk04( TINT a_iPlayer, TUINT32 a_uiValue )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	m_aPlayerData[ a_iPlayer ].m_uiUnk04 = a_uiValue;
}

// $deBlob: FUNCTION 005bbc00
TFLOAT AGameStatus::LevelData::GetLandmarksCompletedPercentage( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	return ( TFLOAT( TINT( m_aPlayerData[ a_iPlayer ].m_auiProgress[ 4 ] ) ) / TFLOAT( TINT( m_auiLevelCounts[ 2 ] ) ) ) * 100.0f;
}

// $deBlob: FUNCTION 005bc130
void AGameStatus::LevelData::IncrementUnk44( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	m_aPlayerData[ a_iPlayer ].m_auiUnk3C[ 2 ]++;
}

// The level time on the HUD (only when the ABlobHud singleton exists): m:ss on the level timer, or with the tenths
// (a_bTenths, sprint levels) on the challenge timer, the one the sprint levels show (ABlobHud hides the level timer there)
static void SetLevelTimer( TFLOAT a_fTimeLeft, TBOOL a_bTenths )
{
	if ( ABlobHud::GetSingleton() == TNULL )
		return;

	TFLOAT fSeconds = TFLOAT( fmod( TDOUBLE( a_fTimeLeft ), 60.0 ) );
	TINT   iMinutes = TINT( TFLOAT( a_fTimeLeft / 60.0 ) );
	TINT   iTenths  = -1;

	if ( a_bTenths )
		iTenths = TINT( TFLOAT( fmod( TDOUBLE( fSeconds ), 1.0 ) ) * 10.0f );

	ABlobHud*      pHud   = ABlobHud::GetSingleton();
	ABlobTimerHud* pTimer = a_bTenths ? pHud->GetChallengeTimer() : pHud->GetLevelTimer();
	pTimer->SetTime( iMinutes, TINT( fSeconds ), iTenths );
}

// The tweakables of the environment's colour blend: the change that is applied, the scale of the painted share of the
// instances in view and the count of painted instances in view that gives the full blend
static TFLOAT s_fBlendEpsilon     = 0.05f;
static TFLOAT s_fBlendViewScale   = 1.6f;
static TFLOAT s_fBlendViewPainted = 8.0f;

// $deBlob: FUNCTION 005b9f90
void AGameStatus::PlayerData::Update( TFLOAT a_fDeltaTime )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();
	LevelData&   rLevelData  = pGameStatus->m_oLevelData;

	// The combo runs out: the score HUD collects its bonus
	if ( m_iComboState != 0 )
	{
		TFLOAT& rfComboTime = *TREINTERPRETCAST( TFLOAT*, &m_uiComboTime );
		rfComboTime -= a_fDeltaTime;

		if ( rfComboTime <= 0.0f )
		{
			if ( rLevelData.m_pScoreHud != TNULL )
				rLevelData.m_pScoreHud->CollectBonus();

			m_iComboState = 0;
			m_iComboCount = 0;
		}
	}

	TINT                                eLevelMode = AGameStatus::GetSingleton()->m_eLevelMode;
	ALevelInformation::LevelProperties* pProps     = rLevelData.m_pLevelProps;
	TUINT32&                            ruiAwards  = m_auiUnk3C[ 0 ];
	TFLOAT&                             rfBlend    = *TREINTERPRETCAST( TFLOAT*, &m_auiUnk3C[ 1 ] );

	if ( AGameStatus::GetSingleton()->m_iMultiPlayerGameType != GAMETYPE_MP_NONE )
		goto FullBlend;

	if ( pProps->m_sprintLevel != 0 )
	{
		// These sprint challenges award nothing until the level is complete
		TINT iSlot = AGameStatus::GetSingleton()->m_iSelectedSlot;

		if ( ( iSlot == 4 || iSlot == 8 || iSlot == 14 || iSlot == 19 ) && !AGameStatus::GetSingleton()->m_bLevelComplete )
			goto Blend;
	}
	else if ( eLevelMode != Mode_Story && eLevelMode != -1 )
	{
		goto Blend;
	}

	{
		// The medals: the score (the player's points, or the sprint challenge's score) and the seconds left reach the
		// level's limits (m_auiMedalLimits[ 1..3 ] the scores, [ 4..6 ] the times, -1: not needed)
		TINT iTimeLeft = TINT( rLevelData.m_fTimeLeft );
		TINT iScore    = -1;

		if ( pProps->m_auiMedalLimits[ 0 ] == 0 )
		{
			iScore = m_iPoints;
		}
		else if ( pProps->m_auiMedalLimits[ 0 ] == 1 )
		{
			TASSERT( rLevelData.m_pLevelProps->m_sprintLevel );

			AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();
			AChallenge*        pChallenge        = pChallengeManager->GetCurrentChallenge();

			if ( pChallenge != TNULL && pChallenge->GetData()->m_uiNumLinkedChallenges == 0 )
				iScore = pChallenge->GetScore();
			else
				iScore = pChallengeManager->GetLastScore();
		}

		const TINT* aiLimits = TREINTERPRETCAST( const TINT*, pProps->m_auiMedalLimits );
		TBOOL       bBronze  = TFALSE;
		TBOOL       bSilver  = TFALSE;
		TBOOL       bGold    = TFALSE;
		TINT        iAward   = -1;
		TBOOL       bAwarded;

		// Gold, else silver, else bronze
		for ( TINT iMedal = 3; iMedal != 0; )
		{
			iMedal--;
			TINT  iScoreLimit = aiLimits[ 1 + iMedal ];
			TINT  iTimeLimit  = aiLimits[ 4 + iMedal ];
			TBOOL bReached;

			if ( iScoreLimit == -1 )
				bReached = iTimeLimit != -1 && iTimeLeft >= iTimeLimit;
			else if ( iTimeLimit == -1 )
				bReached = iScore >= iScoreLimit;
			else
				bReached = iScore >= iScoreLimit && iTimeLeft >= iTimeLimit;

			if ( bReached )
			{
				if ( iMedal == 2 )
					bGold = TTRUE;
				else if ( iMedal == 1 )
					bSilver = TTRUE;
				else
					bBronze = TTRUE;

				break;
			}
		}

		if ( bGold || bSilver || bBronze )
		{
			// The first medal of a sprint opens its gate
			if ( ( ruiAwards & ( EnSaveData::AWARD_Bronze | EnSaveData::AWARD_Silver | EnSaveData::AWARD_Gold ) ) == 0 && rLevelData.m_pLevelProps->m_sprintLevel != 0 && AGameStatus::GetSingleton()->m_pSprintGate != TNULL )
			{
				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_OPEN;
				oPacket.m_iData    = 0;
				AGameStatus::GetSingleton()->m_pSprintGate->HandleEntityIO( TNULL, oPacket );
			}

			// Only the best medal is kept
			if ( bGold && !( ruiAwards & EnSaveData::AWARD_Gold ) )
			{
				iAward    = 0x3B;
				ruiAwards = ( ruiAwards & ~( EnSaveData::AWARD_Bronze | EnSaveData::AWARD_Silver ) ) | EnSaveData::AWARD_Gold;
				ABlobHud::GetSingleton()->AddPendingAchievements( 4 );
			}
			else if ( bSilver && !( ruiAwards & EnSaveData::AWARD_Silver ) )
			{
				iAward    = 0x3A;
				ruiAwards = ( ruiAwards & ~( EnSaveData::AWARD_Bronze | EnSaveData::AWARD_Gold ) ) | EnSaveData::AWARD_Silver;
				ABlobHud::GetSingleton()->AddPendingAchievements( 2 );
			}
			else if ( bBronze && !( ruiAwards & EnSaveData::AWARD_Bronze ) )
			{
				iAward    = 0x39;
				ruiAwards = ( ruiAwards & ~( EnSaveData::AWARD_Silver | EnSaveData::AWARD_Gold ) ) | EnSaveData::AWARD_Bronze;
				ABlobHud::GetSingleton()->AddPendingAchievements( 1 );
			}
		}

		bAwarded = TFALSE;

		if ( iAward != -1 )
		{
			APropTriggerHelp::ShowContextHelp( 0x400 );
			bAwarded = TTRUE;
		}

		if ( rLevelData.m_pLevelProps->m_sprintLevel != 0 )
			goto Blend;

		// The time award: the level was completed within its target time
		if ( AGameStatus::GetSingleton()->m_bLevelComplete && rLevelData.m_fTargetTime != -1.0f && rLevelData.m_fLevelTime <= rLevelData.m_fTargetTime )
			ruiAwards |= EnSaveData::AWARD_Time;

		// The paint awards (only the best is kept): the share of the buildings and landmarks player 0 painted
		if ( iAward == -1 && TINT( rLevelData.m_auiLevelCounts[ 0 ] ) > 0 )
		{
			const PlayerData& rPlayer0     = rLevelData.m_aPlayerData[ 0 ];
			TFLOAT            fPercentage  = ( TFLOAT( TINT( rPlayer0.m_auiProgress[ 4 ] ) + rPlayer0.m_iBuildingsPainted ) / TFLOAT( TINT( rLevelData.m_auiLevelCounts[ 2 ] + rLevelData.m_auiLevelCounts[ 0 ] ) ) ) * 100.0f;
			TUINT32           uiPaintAward = 0;

			if ( fPercentage >= 100.0f )
			{
				if ( !( ruiAwards & EnSaveData::AWARD_Paint100 ) )
				{
					iAward       = 0x42;
					ruiAwards    = ( ruiAwards & ~( EnSaveData::AWARD_Paint25 | EnSaveData::AWARD_Paint50 | EnSaveData::AWARD_Paint75 ) ) | EnSaveData::AWARD_Paint100;
					uiPaintAward = 0x80;
				}
			}
			else if ( fPercentage >= 75.0f )
			{
				if ( !( ruiAwards & EnSaveData::AWARD_Paint75 ) )
				{
					iAward       = 0x41;
					ruiAwards    = ( ruiAwards & ~( EnSaveData::AWARD_Paint25 | EnSaveData::AWARD_Paint50 | EnSaveData::AWARD_Paint100 ) ) | EnSaveData::AWARD_Paint75;
					uiPaintAward = 0x40;
				}
			}
			else if ( fPercentage >= 50.0f )
			{
				if ( !( ruiAwards & EnSaveData::AWARD_Paint50 ) )
				{
					iAward       = 0x40;
					ruiAwards    = ( ruiAwards & ~( EnSaveData::AWARD_Paint25 | EnSaveData::AWARD_Paint75 | EnSaveData::AWARD_Paint100 ) ) | EnSaveData::AWARD_Paint50;
					uiPaintAward = 0x20;
				}
			}
			else if ( fPercentage >= 25.0f )
			{
				if ( !( ruiAwards & EnSaveData::AWARD_Paint25 ) )
				{
					iAward       = 0x3F;
					ruiAwards    = ( ruiAwards & ~( EnSaveData::AWARD_Paint50 | EnSaveData::AWARD_Paint75 | EnSaveData::AWARD_Paint100 ) ) | EnSaveData::AWARD_Paint25;
					uiPaintAward = 0x10;
				}
			}

			if ( uiPaintAward != 0 )
				ABlobHud::GetSingleton()->AddPendingAchievements( uiPaintAward );
		}

		// The awards for all of a kind: the challenges, the landmarks, Bill, Transform (one per frame)
		AChallengeManager* pChallengeManager     = AChallengeManager::GetSingleton();
		TINT               iNumChallengesInLevel = pChallengeManager->GetTotalNumChallenges() - pChallengeManager->GetChallengesData()->m_uiNumNull - pChallengeManager->GetNumSubChallenges();
		TASSERT( iNumChallengesInLevel >= 0 );

		if ( iAward == -1 )
		{
			TUINT32 uiAll = 0;

			if ( iNumChallengesInLevel > 0 && pChallengeManager->GetTotalCompletedChallenges() == iNumChallengesInLevel && !( ruiAwards & EnSaveData::AWARD_Challenge ) )
			{
				iAward = 0x3C;
				uiAll  = EnSaveData::AWARD_Challenge;
			}
			else if ( TINT( rLevelData.m_auiLevelCounts[ 2 ] ) > 0 && m_auiProgress[ 4 ] == rLevelData.m_auiLevelCounts[ 2 ] && !( ruiAwards & EnSaveData::AWARD_Landmarks ) )
			{
				iAward = 0x79;
				uiAll  = EnSaveData::AWARD_Landmarks;
			}
			else if ( TINT( rLevelData.m_auiLevelCounts[ 3 ] ) > 0 && m_auiProgress[ 5 ] == rLevelData.m_auiLevelCounts[ 3 ] && !( ruiAwards & EnSaveData::AWARD_Bill ) )
			{
				iAward = 0x7A;
				uiAll  = EnSaveData::AWARD_Bill;
			}
			else if ( TINT( rLevelData.m_auiLevelCounts[ 4 ] ) > 0 && m_auiProgress[ 6 ] == rLevelData.m_auiLevelCounts[ 4 ] && !( ruiAwards & EnSaveData::AWARD_Transform ) )
			{
				iAward = 0x7B;
				uiAll  = EnSaveData::AWARD_Transform;
			}

			if ( uiAll != 0 )
			{
				ruiAwards |= uiAll;
				ABlobHud::GetSingleton()->AddPendingAchievements( uiAll );
			}
		}

		// And the styles (the pattern pickups), the trees and the Raydians, with no award id
		APlayerManager::BlobData* pBlobData = APlayerManager::GetSingleton()->GetCurrentBlobData();
		TINT                      iPatterns = APickupManager::GetSingleton()->GetNumPatternPickups();

		if ( iAward == -1 )
		{
			TUINT32 uiAll;

			if ( iPatterns > 0 && TINT( pBlobData->m_uiUnk58 ) == iPatterns && !( ruiAwards & EnSaveData::AWARD_Styles ) )
				uiAll = EnSaveData::AWARD_Styles;
			else if ( TINT( rLevelData.m_auiLevelCounts[ 1 ] ) > 0 && m_auiProgress[ 0 ] == rLevelData.m_auiLevelCounts[ 1 ] && !( ruiAwards & EnSaveData::AWARD_Trees ) )
				uiAll = EnSaveData::AWARD_Trees;
			else if ( TINT( rLevelData.m_auiLevelCounts[ 6 ] ) > 0 && m_auiProgress[ 2 ] == rLevelData.m_auiLevelCounts[ 6 ] && !( ruiAwards & EnSaveData::AWARD_Raydian ) )
				uiAll = EnSaveData::AWARD_Raydian;
			else
				goto Blend;

			ruiAwards |= uiAll;
			ABlobHud::GetSingleton()->AddPendingAchievements( uiAll );
		}

		if ( !bAwarded )
			APropTriggerHelp::ShowContextHelp( 0x200 );
	}

Blend:
	if ( rLevelData.m_pLevelProps->m_sprintLevel != 0 )
		goto FullBlend;

	if ( eLevelMode != 2 && eLevelMode != 1 && eLevelMode != 3 && eLevelMode != Mode_Story && eLevelMode != -1 )
		return;

	{
		// The environment's colour follows the share of the level painted (the landmarks count 5 times); below 90%
		// it follows the share of the painted instances in view
		TINT iPainted               = TINT( m_auiProgress[ 4 ] * 5 + m_auiProgress[ 5 ] + m_auiProgress[ 0 ] ) + m_iBuildingsPainted;
		TINT iTotalPaintableObjects = TINT( rLevelData.m_auiLevelCounts[ 2 ] * 5 + rLevelData.m_auiLevelCounts[ 3 ] + rLevelData.m_auiLevelCounts[ 1 ] + rLevelData.m_auiLevelCounts[ 0 ] );
		TASSERT( iTotalPaintableObjects > 0 );

		TFLOAT fBlend = TFLOAT( iPainted ) / TFLOAT( iTotalPaintableObjects );

		if ( fBlend < 0.9f )
		{
			TFLOAT fInView = ( TINT( s_uiNumVisiblePaintableInstances ) > 0 ) ? TFLOAT( TINT( s_uiNumVisiblePaintedInstances ) ) / TFLOAT( TINT( s_uiNumVisiblePaintableInstances ) ) : 0.0f;
			fBlend         = s_fBlendViewScale * fInView;

			TFLOAT fMax = TFLOAT( TINT( s_uiNumVisiblePaintedInstances ) ) / s_fBlendViewPainted;
			TMath::Clip( fMax, 0.0f, 1.0f );
			TMath::Clip( fBlend, 0.0f, fMax );
		}

		if ( TFLOAT( fabs( TDOUBLE( fBlend - rfBlend ) ) ) > s_fBlendEpsilon )
		{
			rfBlend = fBlend;
			AEnvironmentFX::GetSingleton()->SetBlend( fBlend, 5.0f, TFALSE );
		}
	}

	return;

FullBlend:
	if ( rfBlend < 1.0f )
	{
		rfBlend = 1.0f;
		AEnvironmentFX::GetSingleton()->SetBlend( 1.0f, 5.0f, TFALSE );
	}
}

// $deBlob: FUNCTION 005baa80
void AGameStatus::LevelData::Update( TFLOAT a_fDeltaTime )
{
	// The glint (AGameStatus::s_fGlintLerp) eases with cos( 0..0.99 * pi/2 ) and wraps back to 0 past 1
	TFLOAT fGlintLerp = s_fGlintLerp;
	TMath::Clip( fGlintLerp, 0.0f, 0.99f );
	s_fGlintLerp = TFLOAT( cos( TDOUBLE( fGlintLerp * 1.5707964f ) ) ) * 3.0f * a_fDeltaTime + s_fGlintLerp;

	if ( s_fGlintLerp > 1.0f )
		s_fGlintLerp = 0.0f;

	// Each player: its own update (the combo timer, the medals and awards, the environment's colour), then its combo ends when the
	// blob leaves the state that keeps it (the combo states 1/4 end in states 0, 3, 6, 11; 2/3 in 3, 6, 11)
	TINT iPlayer = 0;
	if ( APlayerManager::GetSingleton()->GetNumPlayers() > 0 )
	{
		do
		{
			m_aPlayerData[ iPlayer ].Update( a_fDeltaTime );

			ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( iPlayer ) );
			PlayerData&  rData = m_aPlayerData[ iPlayer ];

			if ( pBlob != TNULL && rData.m_iComboCount > 1 )
			{
				TINT  iBlobState = TSTATICCAST( ABlobEntity, pBlob )->GetBlobState();
				TBOOL bEndCombo  = TFALSE;

				if ( rData.m_iComboState == 1 || rData.m_iComboState == 4 )
					bEndCombo = iBlobState == 0 || iBlobState == 3 || iBlobState == 6 || iBlobState == 0xB;
				else if ( rData.m_iComboState == 2 || rData.m_iComboState == 3 )
					bEndCombo = iBlobState == 3 || iBlobState == 6 || iBlobState == 0xB;

				if ( bEndCombo )
					EndCombo( iPlayer );
			}

			iPlayer++;
		} while ( iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	}

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( m_bUnk00C || pGameStatus->m_eLevelMode == Mode_FreePaint )
	{
		// Only the level start (m_bLevelStart) goes on
		if ( !pGameStatus->m_bLevelStart )
			return;
	}
	else if ( !pGameStatus->m_bLevelStart )
	{
		// The countdown of the level time: warnings at the minute and the last 5 seconds, then the HUD
		TFLOAT fTimeOld = m_fTimeLeft;
		m_fTimeLeft     = fTimeOld - a_fDeltaTime;
		m_fLevelTime += a_fDeltaTime;

		TINT iSecondsOld = TMath::FloorToInt( fTimeOld );
		TINT iSecondsNew = TMath::FloorToInt( m_fTimeLeft );
		TINT iWarning    = m_pLevelProps->m_sprintLevel == 0 ? 60 : 15;

		if ( iSecondsOld == iWarning && iSecondsOld != iSecondsNew )
			ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x2E ), FMOD_EVENT_DEFAULT, TTRUE );

		if ( TUINT32( iSecondsOld - 1 ) < 5 && iSecondsOld != iSecondsNew )
			ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x2D ), FMOD_EVENT_DEFAULT, TTRUE );

		if ( !m_bMultiplayer &&
		     ( ( fTimeOld > 60.0f && m_fTimeLeft <= 60.0f && m_pLevelProps->m_sprintLevel == 0 ) ||
		       ( m_pLevelProps->m_sprintLevel != 0 && fTimeOld > 15.0f && m_fTimeLeft <= 15.0f ) ) )
		{
			APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, 0x37D, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
		}

		if ( m_fTimeLeft < 0.0f )
			m_fTimeLeft = 0.0f;

		SetLevelTimer( m_fTimeLeft, m_pLevelProps->m_sprintLevel != 0 );
		return;
	}

	// The level time starts again from the limit unless the level start respawns
	if ( !pGameStatus->m_bLevelStartRespawn )
		m_fTimeLeft = m_fTimeLimit;

	if ( m_pLevelProps->m_sprintLevel == 0 )
	{
		SetLevelTimer( m_fTimeLeft, TFALSE );
		return;
	}

	m_fLevelTime = 0.0f;
	SetLevelTimer( m_fTimeLeft, TTRUE );
}

// $deBlob: FUNCTION 005bbd20
TINT AGameStatus::LevelData::GetComboState( TINT a_iPlayer ) const
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::MAX_PLAYERS );
	return m_aPlayerData[ a_iPlayer ].m_iComboState;
}

// $deBlob: FUNCTION 005bbd80
void AGameStatus::LevelData::IncrementComboCount( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::MAX_PLAYERS );
	m_aPlayerData[ a_iPlayer ].IncrementChain();
}

// $deBlob: FUNCTION 005bbc80
TUINT32 AGameStatus::LevelData::StartCombo( TINT a_iPlayer, TINT a_iState )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::MAX_PLAYERS );
	PlayerData& rData = m_aPlayerData[ a_iPlayer ];
	rData.StartChain( a_iState );
	return rData.m_uiComboTime;
}

// $deBlob: FUNCTION 005bbaf0
void AGameStatus::LevelData::EndCombo( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	PlayerData& rData = m_aPlayerData[ a_iPlayer ];

	if ( rData.m_iComboState == 1 && m_pScoreHud != TNULL )
		m_pScoreHud->CollectBonus();

	rData.m_iComboState = 0;
	rData.m_iComboCount = 0;
}

// $deBlob: FUNCTION 005bd440
TINT AGameStatus::LevelData::GetComboMultiplier( TINT a_iCount )
{
	if ( a_iCount < 2 )
		return 1;

	if ( a_iCount > 8 )
		return SCORING_INT( 0x2C );

	// 2-8: 0x14-0x2C
	return SCORING_INT( 0x14 + ( a_iCount - 2 ) * 4 );
}

// $deBlob: FUNCTION 005bdef0
void AGameStatus::OnBuildingPaintChanged( ABuildingInstance* a_pBuilding )
{
	if ( m_iMultiPlayerGameType != 2 )
		return;

	TINT iProj = a_pBuilding->m_iGlintIndex;

	if ( iProj != -1 )
	{
		a_pBuilding->m_iGlintIndex = -1;
		RemoveInstanceProjection( a_pBuilding->m_pTerrainInstance );
	}

	if ( iProj >= 0 && iProj != 90 )
	{
		TINT iIndex = ++m_iNumFreeProjections;

		if ( iIndex < 90 )
			m_aiFreeProjections[ iIndex ] = iProj;
	}
}

// $deBlob: FUNCTION 005bdfd0
void __stdcall AGameStatus::SetBuildingProjection( ABuildingInstance* a_pBuilding, TINT a_iProjUDataNum, TBOOL a_bUnk )
{
	TASSERT( a_iProjUDataNum >= 8 && a_iProjUDataNum < 91 );

	a_pBuilding->m_iGlintIndex = TINT16( a_iProjUDataNum );
	a_pBuilding->SetTargetColour( 0 );

	SetInstanceProjection( a_pBuilding->m_pTerrainInstance, a_iProjUDataNum, a_bUnk, 0 );
}

// $deBlob: FUNCTION 005be040
TINT __stdcall AGameStatus::RemoveBuildingProjection( ABuildingInstance* a_pBuilding )
{
	TINT iProj = a_pBuilding->m_iGlintIndex;

	if ( iProj != -1 )
	{
		a_pBuilding->m_iGlintIndex = -1;
		RemoveInstanceProjection( a_pBuilding->m_pTerrainInstance );
	}

	return iProj;
}

// The projection pass of the materials of an instance's meshes
static void AddProjectionPass( TerrainTypes::InstanceMeshes* a_pMeshes, TTexture* a_pTexture, TINT a_iProjUDataNum, TBOOL a_bCheckName )
{
	if ( a_pMeshes == TNULL )
		return;

	for ( TINT i = 0; i < TINT( a_pMeshes->m_uiNumMeshes ); i++ )
	{
		TWorldMaterialHAL* pMaterial = a_pMeshes->m_pMeshes[ i ].m_pMaterial;
		const TCHAR*       szName    = pMaterial->GetMaterialName();

		if ( a_bCheckName && szName != TNULL && ( TStringManager::String8Compare( szName, "window", 6 ) == 0 || TStringManager::String8Compare( szName, "sticker", 7 ) == 0 ) )
			continue;

		TUINT uiStage = pMaterial->GetTextureNum();
		pMaterial->SetTextureNum( uiStage + 1 );
		a_pTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );
		pMaterial->SetTexture( uiStage, a_pTexture );
		pMaterial->SetUVMode( uiStage, TWorldMaterialHAL::UVMODE_PROJECTED );
		pMaterial->SetBlendMode( 13 );
		pMaterial->SetMultiPassBlendType( uiStage, TWorldMaterialHAL::MULTIPASSBLENDTYPE_BLENDWRITEALPHA );
		pMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_0, TINT16( a_iProjUDataNum ) );
	}
}

// The projection pass off the materials of an instance's meshes
static void RemoveProjectionPass( TerrainTypes::InstanceMeshes* a_pMeshes )
{
	if ( a_pMeshes == TNULL )
		return;

	for ( TINT i = 0; i < TINT( a_pMeshes->m_uiNumMeshes ); i++ )
	{
		TWorldMaterialHAL* pMaterial = a_pMeshes->m_pMeshes[ i ].m_pMaterial;
		TUINT              uiNumTex  = pMaterial->GetTextureNum();

		if ( pMaterial->GetUserDataIndex( TWorldMaterialHAL::UData_0 ) < 8 )
			continue;

		pMaterial->SetUVMode( uiNumTex - 1, TWorldMaterialHAL::UVMODE( 0 ) );
		pMaterial->SetTexture( uiNumTex - 1, TNULL );
		pMaterial->SetUserDataIndex( TWorldMaterialHAL::UData_0, -1 );
		pMaterial->SetMultiPassBlendType( uiNumTex - 1, TWorldMaterialHAL::MULTIPASSBLENDTYPE_MODULATE );
		pMaterial->SetTextureNum( uiNumTex - 1 );

		const TCHAR* szName = pMaterial->GetMaterialName();

		if ( szName == TNULL || ( TStringManager::String8Compare( szName, "window", 6 ) != 0 && TStringManager::String8Compare( szName, "sticker", 7 ) != 0 ) )
			pMaterial->SetBlendMode( 0 );
	}
}

// The cell instance of a civilian spawner entity of a building (CellInstance+0x34's entity IDs)
static TerrainTypes::CellInstance* GetSpawnerInstance( ABaseEntity* a_pEntity )
{
	ACivSpawner* pSpawner = TSTATICCAST( ACivSpawner, a_pEntity );
	TASSERT( pSpawner->IsA( &TGetClass( ACivSpawner ) ) );

	return ATerrain2::GetInstance()->GetCellInstance( pSpawner->GetBuildingInstance() );
}

// The projection of a glint over a_fHeight (the instance's or the model's bounds): the range m_fMin..m_fMax and the
// texture matrix of the projected texture
static void AGameStatus_SetProjection( TWorldShaderHAL::ProjTexture& a_rProj, TFLOAT a_fHeight, TBOOL a_bUnk )
{
	TFLOAT fMargin = ( a_fHeight / 120.0f ) * 48.0f;

	a_rProj.m_fHeight = a_fHeight;
	a_rProj.m_fMin    = -fMargin;
	a_rProj.m_fMax    = a_fHeight * 2.0f + fMargin;

	if ( a_bUnk )
		a_rProj.m_fCurrent = a_rProj.m_fMax;

	// XMMatrixOrthographicOffCenterLH( 1.0f, -1.0f, m_fMin, fMargin, 0.0f, 1.0f)
	TFLOAT fBottom = a_rProj.m_fMin;
	TFLOAT fRecipH = 1.0f / ( fMargin - fBottom );

	TMatrix44 oOrtho;
	oOrtho.Set( -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, fRecipH + fRecipH, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, -0.5f * -0.0f, fRecipH * -( fBottom + fMargin ), 1.0f * -0.0f, 1.0f );

	// Into the texture space (scaled by 0.5, then offset by 0.5); XMMatrixMultiply( a, b ) is TMatrix44::Multiply( b, a )
	TMatrix44 oScale;
	oScale.Set( 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f );
	TMatrix44 oOffset;
	oOffset.Set( 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.5f, 0.0f, 1.0f );

	TMatrix44 oScaled;
	oScaled.Multiply( oScale, oOrtho );
	a_rProj.m_oMatrix2.Multiply( oOffset, oScaled );
}

// $deBlob: FUNCTION 005bf090
void __stdcall AGameStatus::SetInstanceProjection( TerrainTypes::CellInstance* a_pInstance, TINT a_iProjUDataNum, TBOOL a_bUnk, TINT a_iModel )
{
	TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
	TASSERT( ( pShader == TNULL ) || pShader->IsA( &TGetClass( TWorldShaderHAL ) ) );

	TWorldShaderHAL::ProjTexture& rProj = pShader->GetProjTexture( a_iProjUDataNum ); // a_iIndex < MAX_PROJ_TEXTURES_ALLOC

	TTexture* pTexture = TTextureManager::GetSingleton()->FindTexture( "building_glint.tga" );

	if ( a_iModel == 0 )
	{
		if ( TStringManager::String8Compare( "prop_doorway_mainmesh", a_pInstance->m_szModel ) != 0 )
		{
			// NOTE: retail reads the height through the instance's m_ppCentre ( [ 1 ]->y )
			AGameStatus_SetProjection( rProj, a_pInstance->m_ppCentre[ 1 ]->y, a_bUnk );
		}

		if ( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Building )
		{
			ALandmarkInstance::Links* pLinks = TSTATICCAST( ALandmarkInstance::Links, a_pInstance->m_pLinks );

			if ( pLinks != TNULL )
			{
				for ( TINT i = 0; i < pLinks->m_iNumEntities; i++ )
				{
					ABaseEntity* pEntity = ABaseEntity::FindIDEntity( pLinks->m_piEntityIDs[ i ] );

					if ( !pEntity->IsA( &TGetClass( ACivSpawner ) ) )
						continue;

					TerrainTypes::CellInstance* pSpawnerInstance                                         = GetSpawnerInstance( pEntity );
					TSTATICCAST( ABuildingInstance, pSpawnerInstance->m_pModelInstance2 )->m_iGlintIndex = TINT16( a_iProjUDataNum );
					AddProjectionPass( pSpawnerInstance->m_pSymbolData, pTexture, a_iProjUDataNum, TFALSE );
				}
			}
		}

		AddProjectionPass( a_pInstance->m_pSymbolData, pTexture, a_iProjUDataNum, TTRUE );
	}
	else
	{
		// a_iModel is an AModelInstance2 (SetTreeProjection): the radius of its model's first LOD
		AModelInstance2* pModelInstance = TREINTERPRETCAST( AModelInstance2*, TUINTPTR( a_iModel ) );
		T2ModelPtr       pModel         = pModelInstance->GetModelInstance()->GetModel();
		TFLOAT           fHeight        = pModel.GetData()->GetLOD( 0 ).GetRenderVolume().GetRadius();
		AGameStatus_SetProjection( rProj, fHeight, a_bUnk );
	}
}

// $deBlob: FUNCTION 005be500
void __stdcall AGameStatus::RemoveInstanceProjection( TerrainTypes::CellInstance* a_pInstance )
{
	RemoveProjectionPass( a_pInstance->m_pSymbolData );

	if ( ( a_pInstance->m_uiFlags & 7 ) != TerrainTypes::InstanceType_Building )
		return;

	ALandmarkInstance::Links* pLinks = TSTATICCAST( ALandmarkInstance::Links, a_pInstance->m_pLinks );

	if ( pLinks == TNULL )
		return;

	for ( TINT i = 0; i < pLinks->m_iNumEntities; i++ )
	{
		ABaseEntity* pEntity = ABaseEntity::FindIDEntity( pLinks->m_piEntityIDs[ i ] );

		if ( !pEntity->IsA( &TGetClass( ACivSpawner ) ) )
			continue;

		TerrainTypes::CellInstance* pSpawnerInstance                                         = GetSpawnerInstance( pEntity );
		TSTATICCAST( ABuildingInstance, pSpawnerInstance->m_pModelInstance2 )->m_iGlintIndex = -1;
		RemoveProjectionPass( pSpawnerInstance->m_pSymbolData );
	}
}

// $deBlob: FUNCTION 005be080
void __stdcall AGameStatus::UpdateGlint( ABuildingInstance* a_pInstance, TFLOAT a_fDeltaTime )
{
	TASSERT( a_pInstance->m_iGlintIndex != -1 );
	UpdateProjectionGlint( a_pInstance->m_iGlintIndex, a_fDeltaTime );
}

// $deBlob: FUNCTION 005bde00
void AGameStatus::UpdateNextBuildingGlintCameras()
{
	if ( m_iMultiPlayerGameType != 2 )
		return;

	AXYZViewportManager*          pViewportManager = ARenderer::GetSingleton()->GetViewportManager();
	AXYZViewportManager::VIEWPORT eViewport        = pViewportManager->GetViewportBeingRendered();
	TASSERT( AXYZViewportManager::VIEWPORT_NONE != eViewport );

	ACamera* pCamera = pViewportManager->GetCamera( eViewport );

	if ( pCamera == TNULL )
		return;

	ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetBlobOfCamera( pCamera ) );

	if ( pBlob == TNULL )
		return;

	TINT iColour = TINT( pBlob->GetColourComponent()->GetStartColour() );

	for ( TINT i = 6; i != 0; )
	{
		i--;
		if ( m_apNextBuildingPaintable[ i ] != TNULL && m_apNextBuildingPaintable[ i ]->m_iGlintIndex >= 0 )
			UpdateGlintCamera( m_apNextBuildingPaintable[ i ], pCamera, iColour, TFALSE );
	}
}

// $deBlob: FUNCTION 005be0e0
void __stdcall AGameStatus::UpdateGlintCamera( ABuildingInstance* a_pInstance, ACamera* a_pCamera, TINT a_iColour, TBOOL a_bUnk )
{
	TASSERT( a_pInstance->m_iGlintIndex >= 8 );
	TASSERT( a_pInstance->m_iGlintIndex >= 0 );

	UpdateProjectionCamera( a_pInstance->m_iGlintIndex, a_pInstance, a_pCamera, a_iColour, a_bUnk, TFALSE );
}

// $deBlob: FUNCTION 005be180
void __stdcall AGameStatus::SetTreeProjection( TerrainTypes::CellInstance* a_pInstance, TINT a_iProjUDataNum, TBOOL a_bUnk )
{
	TASSERT( a_iProjUDataNum >= 8 && a_iProjUDataNum < 91 );
	TASSERT( ( a_pInstance->m_uiFlags & 7 ) == TerrainTypes::InstanceType_Tree );

	ATreeInstance* pTree = TSTATICCAST( ATreeInstance, a_pInstance->m_pModelInstance2 );
	pTree->m_iGlintIndex = TINT16( a_iProjUDataNum );

	// NOTE: SetInstanceProjection's a_iModel is the AModelInstance2 here
	SetInstanceProjection( a_pInstance, a_iProjUDataNum, a_bUnk, TINT( TREINTERPRETCAST( TUINTPTR, pTree->GetCurrentModel() ) ) );
}

// $deBlob: FUNCTION 005be260
void __stdcall AGameStatus::UpdateTreeGlint( ATreeInstance* a_pInstance, TFLOAT a_fDeltaTime )
{
	TASSERT( a_pInstance->m_iGlintIndex != -1 );

	T2ModelPtr pModel  = a_pInstance->GetCurrentModel()->GetModelInstance()->GetModel();
	TFLOAT     fHeight = pModel->GetLOD( 0 ).GetRenderVolume().GetRadius();

	TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
	TASSERT( pShader == TNULL || pShader->IsA( &TGetClass( TWorldShaderHAL ) ) );

	TWorldShaderHAL::ProjTexture& rProj = pShader->GetProjTexture( a_pInstance->m_iGlintIndex );

	TFLOAT fMargin  = ( fHeight / 120.0f ) * 48.0f;
	rProj.m_fHeight = fHeight;
	rProj.m_fMin    = -fMargin;
	rProj.m_fMax    = fHeight * 2.0f + fMargin;

	UpdateProjectionGlint( a_pInstance->m_iGlintIndex, a_fDeltaTime );
}

// $deBlob: FUNCTION 005be450
void __stdcall AGameStatus::UpdateTreeGlintCamera( ATreeInstance* a_pInstance, ACamera* a_pCamera, TINT a_iColour, TBOOL a_bUnk )
{
	TASSERT( a_pInstance->m_iGlintIndex != -1 );
	TASSERT( a_pInstance->m_iGlintIndex >= 8 );

	UpdateProjectionCamera( a_pInstance->m_iGlintIndex, a_pInstance->GetCurrentModel(), a_pCamera, a_iColour, a_bUnk, TTRUE );
}

// $deBlob: FUNCTION 005be8a0
void __stdcall AGameStatus::UpdateProjectionGlint( TINT a_iProjUDataNum, TFLOAT a_fDeltaTime )
{
	TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
	TASSERT( pShader == TNULL || pShader->IsA( &TGetClass( TWorldShaderHAL ) ) );

	TWorldShaderHAL::ProjTexture& rProj = pShader->GetProjTexture( a_iProjUDataNum );

	// NOTE: retail lerps from the max to the min, not from the current value (a_fDeltaTime isn't used)
	TFLOAT fFrom = rProj.m_fMax;
	TFLOAT fTo   = rProj.m_fMin;
	TFLOAT fNew  = ( fTo - fFrom ) * s_fGlintLerp + fFrom;

	if ( ( fFrom < fTo && fNew > fTo ) || ( fFrom > fTo && fTo > fNew ) )
		fNew = fTo;

	rProj.m_fCurrent = fNew;
}

static TFLOAT s_fGlintCameraFOV = 60.0f; // in degrees

// The projection of the glint texture a_iProjUDataNum onto a_pInstance, seen from a camera at a_pCamera's position
// raised to the glint's height and looking along its forward direction, in the colour a_iColour (its alpha unless
// a_bAlpha); a_bTree: a tree's model (its LOD 0 bounds), else an ABuildingInstance (its terrain instance's meshes);
// $deBlob: FUNCTION 005be980
void __stdcall AGameStatus::UpdateProjectionCamera( TINT a_iProjUDataNum, AModelInstance2* a_pInstance, ACamera* a_pCamera, TINT a_iColour, TBOOL a_bAlpha, TBOOL a_bTree )
{
	if ( a_pCamera == TNULL )
		return;

	TFLOAT fModelCentreY;
	{
		T2ModelPtr pModel = a_pInstance->GetModelInstance()->GetModel();
		fModelCentreY     = pModel->GetLOD( 0 ).GetRenderVolume().GetOrigin().y;
	}

	TMatrix44 mTransform;
	a_pInstance->GetTransform( mTransform );

	TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
	TASSERT( pShader == TNULL || pShader->IsA( &TGetClass( TWorldShaderHAL ) ) );

	TWorldShaderHAL::ProjTexture& rProj = pShader->GetProjTexture( a_iProjUDataNum );
	TVector4                      vPos  = a_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );

	TBOOL  bRaise = TTRUE;
	TFLOAT fCentreY;

	if ( a_bTree )
	{
		fCentreY = fModelCentreY;
	}
	else
	{
		TerrainTypes::InstanceMeshes* pMeshes = TSTATICCAST( ABuildingInstance, a_pInstance )->GetTerrainInstance()->m_pSymbolData;

		if ( pMeshes != TNULL )
			fCentreY = pMeshes->m_vCentre.y;
		else
			bRaise = TFALSE;
	}

	if ( bRaise )
		vPos.y = ( ( fCentreY + mTransform.m_f42 ) - rProj.m_fHeight ) + rProj.m_fCurrent;

	const TMatrix44& rCamMatrix = a_pCamera->GetMatrix();
	TVector4         vTarget( rCamMatrix.m_f31 + vPos.x, vPos.y + 0.0f, rCamMatrix.m_f33 + vPos.z, vPos.w );

	ACamera oCamera;
	oCamera.GetCameraMatrix().m_mMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vPos;
	oCamera.LookAtPoint( vTarget );

	TFLOAT fFOV                         = ( s_fGlintCameraFOV / 180.0f ) * TMath::PI;
	oCamera.GetCameraMatrix().m_fFOV    = fFOV;
	oCamera.GetCameraMatrix().m_fAspect = TFLOAT( tan( fFOV * 0.5f ) );

	TMatrix44 mView = oCamera.GetMatrix();
	mView.InvertOrthonormal();

	TMatrix44 mProj;
	mProj.Multiply( mView, mTransform );

	TUINT32 uiColour = ABlobColourMaps::ms_pGlintColourMap[ a_iColour ];

	if ( !a_bAlpha )
		uiColour &= 0xFFFFFF;

	rProj.m_oMatrix1 = mProj;
	rProj.m_uiColour = uiColour;
}

// $deBlob: FUNCTION 005bed00
TINT AGameStatus::AllocateProjection()
{
	if ( m_iNumFreeProjections < 0 )
		return -1;

	TASSERT( m_iNumFreeProjections >= 0 );

	return m_aiFreeProjections[ m_iNumFreeProjections-- ];
}

// $deBlob: FUNCTION 005bed90
void AGameStatus::ClearProjections()
{
	m_iNumFreeProjections = -1;
}

// $deBlob: FUNCTION 005beda0
void AGameStatus::ResetFreeProjections()
{
	m_iNumFreeProjections = -1;

	TINT i = 0;
	do
	{
		TINT iIndex = ++m_iNumFreeProjections;

		if ( iIndex < 90 )
			m_aiFreeProjections[ iIndex ] = i;

		i++;
	}
	while ( i < 90 );
}

// $deBlob: FUNCTION 005bed60
void AGameStatus::FreeProjection( TINT a_iProjUDataNum )
{
	if ( a_iProjUDataNum != 90 )
	{
		TINT iIndex = ++m_iNumFreeProjections;

		if ( iIndex < 90 )
			m_aiFreeProjections[ iIndex ] = a_iProjUDataNum;
	}
}

// $deBlob: FUNCTION 005bd990
// $deBlob: FUNCTION 005bdf50
TBOOL AGameStatus::IsNextPaintableBuilding( const ABuildingInstance* a_pBuilding ) const
{
	for ( TINT i = 6; i != 0; )
	{
		i--;
		if ( a_pBuilding == m_apNextBuildingPaintable[ i ] )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005bd750
void AGameStatus::ResetLevel()
{
	m_bUnk00C              = TFALSE;
	m_bPlaceFirstBuildings = TFALSE;
	m_bUnk000              = TFALSE;
	m_uiUnk008             = 0;

	if ( m_pItMarker != TNULL )
		m_pItMarker->SetVisible( TFALSE );

	if ( GetLeaderBoardHud() != TNULL )
		GetLeaderBoardHud()->ResetRanks();

	TINT iGameType = m_iMultiPlayerGameType;

	if ( iGameType >= 0 )
		AGUIManager::GetSingleton()->SetCursorVisibleCount( 0 );

	if ( iGameType == 2 )
	{
		m_uiUnk9F8 = 0;
		m_uiUnk9FC = 0;
		ABuildingManager::GetSingleton()->PaintAllBuildings( 0, 0xFF424242 );
		m_bPlaceFirstBuildings = TTRUE;
		m_iNumFreeProjections  = -1;

		// The six projections of the multiplayer paint mode
		for ( TINT i = 0; i < 6; i++ )
		{
			TINT iProjUDataNum = 89 - i;
			TASSERT( iProjUDataNum >= 8 && iProjUDataNum < 90 );

			FreeProjection( iProjUDataNum );
		}

		return;
	}

	if ( iGameType != 3 )
		return;

	TINT iPlayers = APlayerManager::GetSingleton()->GetNumPlayers();
	TINT iPainted = 0;
	TASSERT( iPlayers > 0 );

	// The teams' buildings start in the colour of their players (two players: 0 and 1 share teams 1/3 and 2/4)
	switch ( iPlayers )
	{
		case 1: iPainted = ABuildingManager::GetSingleton()->PaintTeamBuildings( 0, 0, 0, 0 ); break;
		case 2: iPainted = ABuildingManager::GetSingleton()->PaintTeamBuildings( 0, 1, 0, 1 ); break;
		case 3: iPainted = ABuildingManager::GetSingleton()->PaintTeamBuildings( 0, 1, 2, -1 ); break;
		case 4: iPainted = ABuildingManager::GetSingleton()->PaintTeamBuildings( 0, 1, 2, 3 ); break;
		default: break;
	}

	TINT iPoints = ( iPainted / iPlayers ) * TREINTERPRETCAST( TINT*, GetBlobScoringProperties() )[ 0x0C / 4 ];

	for ( TINT i = iPlayers; i != 0; )
	{
		i--;
		TASSERT( 0 == m_oLevelData.GetPoints( i ) );
		m_oLevelData.AddPoints( i, iPoints );
	}
}

// $deBlob: FUNCTION 005bdf80
void AGameStatus::ClearNextPaintableProjections()
{
	if ( m_iMultiPlayerGameType < 0 )
		return;

	for ( TINT i = 6; i != 0; )
	{
		i--;
		ABuildingInstance* pBuilding = m_apNextBuildingPaintable[ i ];

		if ( pBuilding != TNULL )
		{
			RemoveBuildingProjection( pBuilding );
			m_apNextBuildingPaintable[ i ] = TNULL;
		}
	}
}

// $deBlob: FUNCTION 005bd5e0
void AGameStatus::AddBonusScore( TINT a_iPlayer, TINT a_iScore, const TVector4* a_pPosition )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::MAX_PLAYERS );

	if ( APlayerManager::GetSingleton()->GetNumPlayers() == 1 && m_oLevelData.m_pScoreHud != TNULL )
	{
		m_oLevelData.m_pScoreHud->AddScore( a_iScore, -1, m_oLevelData.m_aPlayerData[ a_iPlayer ].m_iComboCount == 1, SCORING_FLOAT( 0x00 ) );
	}
	else
	{
		m_oLevelData.AddPoints( a_iPlayer, a_iScore );
	}

	TWCHAR wszText[ 128 ];
	TStringManager::String16Format( wszText, 0x80, T2Locale::GetSingleton()->GetString( 190 ), a_iScore );

	ABlobScoreFloatHud* pFloatHud = m_oLevelData.GetScoreFloatHud( a_iPlayer );

	if ( pFloatHud != TNULL )
		pFloatHud->ShowText( wszText, *a_pPosition, TFALSE );
}

// $deBlob: FUNCTION 005bbe80
void AGameStatus::LevelData::ShowMessage( const TWCHAR* a_wszText, TINT a_iType, const TVector4* a_pPosition, TINT a_iPlayer )
{
	if ( a_iType == 2 || a_iType == 3 )
	{
		ABlobScoreFloatHud* pScoreFloat = GetScoreFloatHud( a_iPlayer );

		if ( pScoreFloat != TNULL )
			pScoreFloat->ShowText( a_wszText, *a_pPosition, a_iType == 3 );

		return;
	}

	if ( a_iType == 1 || m_pMessageControl == TNULL )
		return;

	if ( a_iType == 7 )
		TStringManager::String16Format( m_wszMessage, L"%d/%d\n%ls", m_aPlayerData[ 0 ].m_auiUnk00[ 17 ], m_auiUnk160[ 0 ], a_wszText );
	else
		TStringManager::String16Copy( m_wszMessage, a_wszText, 0x100 );

	m_pMessageControl->SetPresenterText( m_wszMessage, 0, -1 );
	m_pMessageControl->SetVisible( TTRUE );
	TSTATICCAST( TXUIControl, ABlobHud::GetSingleton()->GetLifeHud() )->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );

	TXUIVisual* pVisual = m_pMessageControl->GetVisual();

	switch ( a_iType )
	{
		case 4:
			pVisual->PlayAnimation( L"TimeBonus", L"EndTimeBonus" );
			break;
		case 5:
			pVisual->PlayAnimation( L"TimeRunningOut", L"EndTimeRunningOut" );
			break;
		case 6:
			m_pMessageControl->SetVisible( TFALSE );
			break;
		case 7:
			pVisual->PlayAnimation( L"PatternCollected", L"EndPatternCollected" );
			break;
		case 8:
			pVisual->PlayAnimation( L"ChallengeCompleted", L"EndChallengeCompleted" );
			break;
		case 9:
			pVisual->PlayAnimation( L"AchievementUnlocked", L"EndAchievementUnlocked" );
			break;
		case 10:
		{
			// Already playing between "RaydianFree" and "EndGraydian": back to "RaydianFree"
			TXUITimelineControl* pTimeline = pVisual->GetTimelineControl();

			if ( pTimeline->IsPlaying() && m_fRaydianFreeFrame <= pTimeline->GetTimeLineTime() && pTimeline->GetTimeLineTime() < m_fEndGraydianFrame )
				pTimeline->SetTimeLineTime( m_fRaydianFreeFrame, TTRUE );
			else
				pVisual->PlayAnimation( L"Graydian", L"EndGraydian" );

			break;
		}
		default:
			break;
	}
}

// $deBlob: FUNCTION 005bc260
ABlobScoreFloatHud* AGameStatus::LevelData::GetScoreFloatHud( TINT a_iPlayer )
{
	if ( AGameStatus::GetSingleton()->m_eLevelMode == Mode_FreePaint )
		return TNULL;

	TINT iIndex;

	if ( m_bMultiplayer )
	{
		iIndex = a_iPlayer;
	}
	else
	{
		TASSERT( 0 == a_iPlayer );
		TINT iStart = m_iLastScoreFloatHud;

		while ( TTRUE )
		{
			iIndex               = ( m_iLastScoreFloatHud + 1 ) % 4;
			m_iLastScoreFloatHud = iIndex;

			if ( m_apScoreFloatHud[ iIndex ] != TNULL || iIndex == iStart )
				break;
		}
	}

	return m_apScoreFloatHud[ iIndex ];
}

// $deBlob: FUNCTION 005bc190
void AGameStatus::LevelData::AddBonusCollected( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	m_aPlayerData[ a_iPlayer ].m_auiUnk3C[ 3 ]++;
}

// $deBlob: FUNCTION 005bc100
void AGameStatus::LevelData::SetGoal( TINT a_iGoal, TUINT32 a_uiUnk774 )
{
	m_uiUnk774 = a_uiUnk774;
	m_iGoal    = a_iGoal;

	if ( m_pGoalHud != TNULL )
		TSTATICCAST( ABlobGoalHud, m_pGoalHud )->SetGoalCount( a_iGoal );
}

// $deBlob: FUNCTION 005bc090
void AGameStatus::LevelData::SetMessageControl( TXUIControl* a_pControl )
{
	m_pMessageControl = a_pControl;

	if ( a_pControl != TNULL )
	{
		m_fEndGraydianFrame = a_pControl->GetVisual()->GetTimelineControl()->GetNamedFrameTime( L"EndGraydian" );
		m_fRaydianFreeFrame = a_pControl->GetVisual()->GetTimelineControl()->GetNamedFrameTime( L"RaydianFree" );
	}
	else
	{
		m_fEndGraydianFrame = 0.0f;
		m_fRaydianFreeFrame = 0.0f;
	}
}

// Starts the level start (the blob attached to the level start gate's opening or respawn animation) or ends it: the
// blob faces the way its camera looks, and after the level's start (not a respawn) the music plays again
// $deBlob: FUNCTION 005bdc30
void AGameStatus::SetLevelStart( TBOOL a_bLevelStart, TBOOL a_bRespawn )
{
	m_bLevelStart = a_bLevelStart;

	if ( a_bLevelStart )
	{
		m_bLevelStartRespawn = a_bRespawn;

		if ( m_pLevelStartGate != TNULL )
			m_pLevelStartGate->PlayLevelStart( a_bRespawn );

		m_oLevelData.m_bUnk00C = TTRUE;
		return;
	}

	ABaseEntity* pBlob = APlayerManager::GetSingleton()->GetPlayerBlob( 0 );

	if ( pBlob != TNULL )
	{
		ACamera* pCamera = APlayerManager::GetSingleton()->GetBlobCamera( pBlob );

		if ( pCamera != TNULL )
		{
			TVector4 vFacing;
			pCamera->GetFlatForward( vFacing );
			pBlob->SetFacing( vFacing );
		}
	}

	if ( !m_bLevelStartRespawn )
	{
		AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentGameState();
		TASSERT( pGameState->IsA( &TGetClass( AInGameState ) ) );
		TSTATICCAST( AInGameState, pGameState )->OnLevelStartFinished();
	}

	AMusicManager::GetSingleton()->PauseMusic( TFALSE, 0.0f );
	m_bLevelStartRespawn = TFALSE;
}

// The camera of the level start (see APropDoor::GetLevelStartCamera); ends the level start when it's over
// $deBlob: FUNCTION 005bdd30
TBOOL AGameStatus::GetLevelStartCamera( TMatrix44& a_rOutMatrix, TFLOAT& a_rfWeight )
{
	if ( m_bLevelStart )
	{
		if ( m_pLevelStartGate != TNULL && m_pLevelStartGate->GetLevelStartCamera( a_rOutMatrix, a_rfWeight ) )
			return TTRUE;

		SetLevelStart( TFALSE, TFALSE );
	}

	return TFALSE;
}

// The position of the blob during the level start
// $deBlob: FUNCTION 005bdd80
TBOOL AGameStatus::GetLevelStartBlobPosition( TVector4& a_rOutPosition )
{
	return m_bLevelStart && m_pLevelStartGate != TNULL && m_pLevelStartGate->GetBlobAttachPosition( a_rOutPosition );
}

// The time of the level start animation (the blob's animation follows it)
// $deBlob: FUNCTION 005bddb0
TFLOAT AGameStatus::GetLevelStartAnimTime()
{
	if ( !m_bLevelStart || m_pLevelStartGate == TNULL )
		return 0.0f;

	return m_pLevelStartGate->GetLevelStartAnimTime();
}

// $deBlob: FUNCTION 005bedd0
void AGameStatus::OnSprintChallengesCompleted()
{
	if ( m_pSprintGate != TNULL )
	{
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_OPEN;
		oPacket.m_iData    = 0;
		m_pSprintGate->HandleEntityIO( TNULL, oPacket );
	}
}

const AGameStatus::Award AGameStatus::s_aAwards[ 13 ] = {
	{ 0x3F, 0x245, EnSaveData::AWARD_Paint25, L"Paint25" },
	{ 0x40, 0x246, EnSaveData::AWARD_Paint50, L"Paint50" },
	{ 0x41, 0x247, EnSaveData::AWARD_Paint75, L"Paint75" },
	{ 0x42, 0x248, EnSaveData::AWARD_Paint100, L"Paint100" },
	{ 0x3C, 0x249, EnSaveData::AWARD_Challenge, L"Challenge" },
	{ 0xD0, 0x24A, EnSaveData::AWARD_Time, L"Time" },
	{ 0x3D, 0x24B, EnSaveData::AWARD_Styles, L"Styles" },
	{ 0x77, 0x24C, EnSaveData::AWARD_Trees, L"Trees" },
	{ 0x79, 0x24E, EnSaveData::AWARD_Landmarks, L"Landmarks" },
	{ 0x7A, 0x24F, EnSaveData::AWARD_Bill, L"Bill" },
	{ 0x7B, 0x250, EnSaveData::AWARD_Transform, L"Transform" },
	{ 0xAC, 0x252, EnSaveData::AWARD_Raydian, L"Raydian" },
	{ 0x12E, 0x12E, EnSaveData::AWARD_Vista, L"Vista" },
};

// $deBlob: FUNCTION 005bbb70
TFLOAT AGameStatus::LevelData::GetPaintPercentage( TINT a_iPlayer ) const
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::MAX_PLAYERS );

	const PlayerData& rPlayer = m_aPlayerData[ a_iPlayer ];
	return ( TFLOAT( TINT( rPlayer.m_auiProgress[ 4 ] ) + rPlayer.m_iBuildingsPainted ) / TFLOAT( TINT( m_auiLevelCounts[ 2 ] + m_auiLevelCounts[ 0 ] ) ) ) * 100.0f;
}

// $deBlob: FUNCTION 005b9b90
void AGameStatus::CalculateScoreSummary( ScoreSummary* a_pSummary )
{
	AGameStatus*              pGameStatus = AGameStatus::GetSingleton();
	APlayerManager::BlobData* pBlobData   = APlayerManager::GetSingleton()->GetCurrentBlobData();
	const TINT*               pProps      = TREINTERPRETCAST( const TINT*, s_pBlobScoringProperties );

	TINT iPoints                = pGameStatus->m_oLevelData.GetPoints( 0 );
	a_pSummary->m_iPoints       = iPoints;
	TINT iTimeBonus             = TINT( pGameStatus->m_oLevelData.m_fTimeLeft ) * pProps[ 0x4C / 4 ];
	a_pSummary->m_iTimeBonus    = iTimeBonus;
	a_pSummary->m_iRankBonus    = 0;
	a_pSummary->m_iRankStringID = 0x47;
	a_pSummary->m_iRescueBonus  = pProps[ 0x64 / 4 ] * TINT( pBlobData->m_uiUnk58 );

	TUINT32 uiFlags = pGameStatus->m_oLevelData.m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ];

	if ( uiFlags & EnSaveData::AWARD_Paint25 )
	{
		a_pSummary->m_iRankStringID = 0x43;
		a_pSummary->m_iRankBonus    = pProps[ 0x54 / 4 ];
	}
	else if ( uiFlags & EnSaveData::AWARD_Paint50 )
	{
		a_pSummary->m_iRankStringID = 0x44;
		a_pSummary->m_iRankBonus    = pProps[ 0x58 / 4 ];
	}
	else if ( uiFlags & EnSaveData::AWARD_Paint75 )
	{
		a_pSummary->m_iRankStringID = 0x45;
		a_pSummary->m_iRankBonus    = pProps[ 0x5C / 4 ];
	}
	else if ( uiFlags & EnSaveData::AWARD_Paint100 )
	{
		a_pSummary->m_iRankStringID = 0x46;
		a_pSummary->m_iRankBonus    = pProps[ 0x60 / 4 ];
	}

	a_pSummary->m_iTotal = iTimeBonus + a_pSummary->m_iRankBonus + iPoints + a_pSummary->m_iRescueBonus;
}

// $deBlob: FUNCTION 005bee00
void AGameStatus::SetItMarker( TXUIElement* a_pElement )
{
	m_pItMarker = a_pElement;

	if ( a_pElement != TNULL )
		a_pElement->SetVisible( TFALSE );
}

// $deBlob: FUNCTION 005bee20
void AGameStatus::SetItPlayer( TINT a_iPlayer )
{
	m_uiUnk008 = a_iPlayer;

	ABlobHud* pHud = ABlobHud::GetSingleton();

	if ( pHud != TNULL )
	{
		ABlobMultiPlayerHud* pPanel;

		switch ( a_iPlayer )
		{
			case 0: pPanel = pHud->GetPlayer1Hud(); break;
			case 1: pPanel = pHud->GetPlayer2Hud(); break;
			case 2: pPanel = pHud->GetPlayer3Hud(); break;
			case 3: pPanel = pHud->GetPlayer4Hud(); break;
			default: pPanel = TNULL; break;
		}

		if ( pPanel != TNULL )
		{
			ABlobEntity* pBlob    = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( TUINT8( m_uiUnk008 ) ) );
			TUINT32      uiColour = ABlobColourComponent::ms_auiColours[ pBlob->GetColourComponent()->GetStartColour() ];

			// The wings (the children of the marker's first child) in the colour, ARGB turned to RGBA
			T2GUIElement* pWings = m_pItMarker->ChildBegin();

			for ( T2GUIElement* pWing = pWings->ChildBegin(); pWing != TNULL; pWing = pWings->GetNextChild( pWing ) )
				pWing->SetColour( ( uiColour >> 8 ) | ( uiColour << 24 ) );

			m_pItMarker->SetAnchor( T2GUIATTACHMENT( 0 ) );
			m_pItMarker->SetPivot( T2GUIATTACHMENT( 4 ) );
			m_pItMarker->SetVisible( TTRUE );
			m_pItMarker->PlayAnimation( L"wingstart", L"wingend" );
			pPanel->AddChildTail( m_pItMarker );
		}
	}

	UpdateItMarker( 0.0f );
}

// $deBlob: FUNCTION 005bef60
void AGameStatus::UpdateItMarker( TFLOAT a_fDeltaTime )
{
	TXUIElement* pMarker = m_pItMarker;

	if ( pMarker == TNULL || !pMarker->IsVisible() )
		return;

	// NOTE: the marker is a scene, retail reads its timeline's playing flag (+0xE6)
	if ( !TSTATICCAST( TXUIScene, pMarker )->GetTimelineControl().IsPlaying() )
	{
		pMarker->SetVisible( TFALSE );
		return;
	}

	// Over the blob, relative to the marker's parent
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	ABaseEntity*    pBlob          = pPlayerManager->GetPlayerBlob( TUINT8( m_uiUnk008 ) );
	TViewport*      pViewport      = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( pPlayerManager->GetPlayerCamera( TUINT8( m_uiUnk008 ) ) );

	TVector4 vPosition = pBlob->GetTranslation();
	ARenderer::GetSingleton()->ProjectToScreen( vPosition, pViewport );
	vPosition.x = pViewport->GetX() + vPosition.x;
	vPosition.y = pViewport->GetY() + vPosition.y;
	ARenderer::GetSingleton()->ScreenToCanvas( vPosition.x, vPosition.y );

	const TVector2& rParentPos = m_pItMarker->GetParent()->GetTransform().GetPos();
	m_pItMarker->SetPosition( vPosition.x - rParentPos.x, vPosition.y - rParentPos.y );
}

// $deBlob: FUNCTION 005bcfb0
const TWCHAR* AGameStatus::GetColourMessage( TUINT32 a_uiColour )
{
	switch ( a_uiColour )
	{
		case 1: return T2Locale::GetSingleton()->GetString( 0x101 );
		case 2: return T2Locale::GetSingleton()->GetString( 0x100 );
		case 3: return T2Locale::GetSingleton()->GetString( 0x102 );
		case 4: return T2Locale::GetSingleton()->GetString( 0xFF );
		case 5: return T2Locale::GetSingleton()->GetString( 0x105 );
		case 6: return T2Locale::GetSingleton()->GetString( 0x104 );
		case 7: return T2Locale::GetSingleton()->GetString( 0x103 );
	}

	return TNULL;
}

// $deBlob: FUNCTION 005bce10
void AGameStatus::SetItBlob( ABaseEntity* a_pBlob )
{
	if ( m_bLevelComplete )
		return;

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	SetItPlayer( pPlayerManager->GetPlayerIndex( a_pBlob ) );
	ADecalManager::GetSingleton()->Reset();

	ABlobEntity* pItBlob = TSTATICCAST( ABlobEntity, a_pBlob );

	if ( ABlobHud::GetSingleton() != TNULL )
		ABlobHud::GetSingleton()->ShowMessage( GetColourMessage( pItBlob->GetColourComponent()->GetStartColour() ) );

	// The others become ink blobs and lose the crown
	for ( TINT i = pPlayerManager->GetNumPlayers(); i != 0; )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( --i ) ) );

		if ( pBlob != a_pBlob )
		{
			pBlob->BecomeInkBlob();
			AModelInstance2* pCrown = pBlob->GetAnimationComponent()->GetCrownModelInstance();

			if ( pCrown != TNULL )
				pCrown->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		}
	}

	AModelInstance2* pCrown = pItBlob->GetAnimationComponent()->GetCrownModelInstance();

	if ( pCrown != TNULL )
		pCrown->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
}

// NOTE: the same code as SetItBlob (a_pInked isn't used)
// $deBlob: FUNCTION 005bcee0
void AGameStatus::OnBlobInked( ABaseEntity* a_pInked, ABaseEntity* a_pInker )
{
	if ( m_bLevelComplete )
		return;

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	SetItPlayer( pPlayerManager->GetPlayerIndex( a_pInker ) );
	ADecalManager::GetSingleton()->Reset();

	ABlobEntity* pItBlob = TSTATICCAST( ABlobEntity, a_pInker );

	if ( ABlobHud::GetSingleton() != TNULL )
		ABlobHud::GetSingleton()->ShowMessage( GetColourMessage( pItBlob->GetColourComponent()->GetStartColour() ) );

	for ( TINT i = pPlayerManager->GetNumPlayers(); i != 0; )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( --i ) ) );

		if ( pBlob != a_pInker )
		{
			pBlob->BecomeInkBlob();
			AModelInstance2* pCrown = pBlob->GetAnimationComponent()->GetCrownModelInstance();

			if ( pCrown != TNULL )
				pCrown->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		}
	}

	AModelInstance2* pCrown = pItBlob->GetAnimationComponent()->GetCrownModelInstance();

	if ( pCrown != TNULL )
		pCrown->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
}

// $deBlob: FUNCTION 005bcd40
void AGameStatus::OnAllBlobsUnpainted()
{
	if ( m_bLevelComplete )
		return;

	if ( ABlobHud::GetSingleton() != TNULL )
		ABlobHud::GetSingleton()->ShowMessage( T2Locale::GetSingleton()->GetString( 262 ) );

	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

	for ( TINT i = pPlayerManager->GetNumPlayers(); i != 0; )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( --i ) ) );

		if ( pBlob == TNULL )
			continue;

		TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

		if ( uiColour & 0x10 )
			uiColour = 16;

		pBlob->GetSizeHud()->SetColour( ABlobColourComponent::ms_auiColours[ uiColour ], TFALSE, 0.0f );
		pBlob->GetSizeHud()->GetVisual()->PlayAnimation( L"Inkbot", L"EndInkbot" );

		AModelInstance2* pCrown = pBlob->GetAnimationComponent()->GetCrownModelInstance();

		if ( pCrown != TNULL )
			pCrown->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}
}

// $deBlob: FUNCTION 005bfca0
void AGameStatus::UpdateItMatch( TFLOAT a_fDeltaTime )
{
	TBOOL bNoBlobPainted = APlayerManager::GetSingleton()->IsNoBlobPainted();

	if ( bNoBlobPainted != m_bUnk000 )
	{
		m_bUnk000 = bNoBlobPainted;

		if ( bNoBlobPainted )
			OnAllBlobsUnpainted();
	}

	UpdateItMarker( a_fDeltaTime );
	UpdateTimedMatch();
}

// $deBlob: FUNCTION 005bfcf0
void AGameStatus::UpdateGlintMatch( TFLOAT a_fDeltaTime )
{
	APlayerManager*   pPlayerManager   = APlayerManager::GetSingleton();
	ABuildingManager* pBuildingManager = ABuildingManager::GetSingleton();

	// The centre of the blobs
	TVector4 vCentre( 0.0f, 0.0f, 0.0f, 1.0f );

	for ( TINT i = APlayerManager::MAX_PLAYERS; i != 0; )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( --i ) ) );

		if ( pBlob != TNULL )
		{
			const TVector4& rPosition = pBlob->GetBodyPosition();
			vCentre.y += rPosition.y;
			vCentre.x += rPosition.x;
			vCentre.z += rPosition.z;
		}
	}

	TASSERT( APlayerManager::GetSingleton()->GetNumPlayers() > 0 );
	vCentre.Divide( TFLOAT( pPlayerManager->GetNumPlayers() ) );

	// The first buildings once the terrain is loaded
	if ( m_bPlaceFirstBuildings )
	{
		if ( ATerrain2::GetInstance()->IsStreaming() )
			return;

		m_bPlaceFirstBuildings = TFALSE;

		for ( TINT i = 6; i != 0; )
		{
			i--;
			TASSERT( TNULL == m_apNextBuildingPaintable[ i ] );

			ABuildingInstance* pBuilding   = pBuildingManager->GetRandomPaintableBuilding( m_uiUnk9F8, vCentre, 50.0f );
			m_apNextBuildingPaintable[ i ] = pBuilding;

			if ( pBuilding != TNULL )
			{
				pBuilding->SetColour( m_uiUnk9FC, -1, TFALSE, TFALSE );
				pBuilding->SetLocalAmbientColour( ABlobColourComponent::ms_auiColours[ m_uiUnk9FC ] );

				SetBuildingProjection( pBuilding, AllocateProjection(), TTRUE );
			}
		}
	}

	// The painted ones give their projection back and are replaced
	TINT iNumLeft = 0;

	for ( TINT i = 6; i != 0; )
	{
		ABuildingInstance* pBuilding = m_apNextBuildingPaintable[ --i ];

		if ( pBuilding == TNULL )
			continue;

		if ( pBuilding->m_iMultiplayerOwner >= 0 )
		{
			TINT iProj = pBuilding->m_iGlintIndex;

			if ( iProj != -1 )
			{
				pBuilding->m_iGlintIndex = -1;
				RemoveInstanceProjection( pBuilding->m_pTerrainInstance );
			}

			if ( iProj >= 0 && iProj != 90 )
			{
				TINT iIndex = ++m_iNumFreeProjections;

				if ( iIndex < 90 )
					m_aiFreeProjections[ iIndex ] = iProj;
			}

			pBuilding                      = pBuildingManager->GetRandomPaintableBuilding( m_uiUnk9F8, vCentre, 50.0f );
			m_apNextBuildingPaintable[ i ] = pBuilding;

			if ( pBuilding != TNULL )
			{
				pBuilding->SetColour( m_uiUnk9FC, -1, TFALSE, TFALSE );
				pBuilding->SetLocalAmbientColour( ABlobColourComponent::ms_auiColours[ m_uiUnk9FC ] );
				SetBuildingProjection( pBuilding, AllocateProjection(), TTRUE );
			}
		}

		pBuilding = m_apNextBuildingPaintable[ i ];

		if ( pBuilding != TNULL )
		{
			TASSERT( pBuilding->m_iGlintIndex != -1 );
			UpdateProjectionGlint( pBuilding->m_iGlintIndex, a_fDeltaTime );
			iNumLeft++;
		}
	}

	m_iLeader = 0;

	if ( m_oLevelData.m_fTimeLimit == -1.0f || 0.0f < m_oLevelData.m_fTimeLeft )
	{
		// Every building is painted
		if ( iNumLeft != 0 )
			return;

		UpdateRanks();
		m_bLevelComplete = TTRUE;
		m_bUnk845        = TTRUE;
	}
	else
	{
		UpdateRanks();
		m_bLevelComplete = TTRUE;
		m_bUnk845        = TTRUE;

		if ( ABlobHud::GetSingleton() != TNULL )
			ABlobHud::GetSingleton()->ShowWinner( m_iLeader );
	}

	ABlobInputMap::GetSingleton()->DisableFor( -1.0f );
}

// $deBlob: FUNCTION 005bda00
void AGameStatus::UpdateRanks()
{
	APlayerManager* pPlayerManager = APlayerManager::GetSingleton();
	TINT            iTotalPoints   = 0;

	for ( TINT i = 0; i < APlayerManager::MAX_PLAYERS; i++ )
	{
		Rank& rRank      = m_aoRanks[ i ];
		rRank.m_iPlayer  = -1;
		rRank.m_iRank    = 1;
		rRank.m_iScore   = 0;
		rRank.m_fPercent = 0.0f;

		if ( pPlayerManager->GetPlayerBlob( TUINT8( i ) ) != TNULL )
		{
			TINT iPoints = m_oLevelData.GetPoints( i );
			iTotalPoints += iPoints;
			rRank.m_iPlayer = i;
			rRank.m_iScore  = iPoints;
		}
	}

	// The most points first
	for ( TINT i = 0; i < APlayerManager::MAX_PLAYERS - 1; i++ )
	{
		for ( TINT j = i + 1; j < APlayerManager::MAX_PLAYERS; j++ )
		{
			if ( m_aoRanks[ i ].m_iScore < m_aoRanks[ j ].m_iScore )
			{
				Rank oRank     = m_aoRanks[ i ];
				m_aoRanks[ i ] = m_aoRanks[ j ];
				m_aoRanks[ j ] = oRank;
			}
		}
	}

	// NOTE: retail compares only the second and third places with the ones before them, so the fourth always has the
	// third's rank
	TINT iRank = 1;

	for ( TINT i = 1; i < APlayerManager::MAX_PLAYERS - 1; i++ )
	{
		if ( m_aoRanks[ i ].m_iScore < m_aoRanks[ i - 1 ].m_iScore )
		{
			iRank++;

			for ( TINT j = i; j < APlayerManager::MAX_PLAYERS; j++ )
				m_aoRanks[ j ].m_iRank = iRank;
		}
	}

	// The leaders wear the crown (in game type 1 only once the match is over)
	if ( 0 < iTotalPoints && ( AGameStatus::GetSingleton()->m_iMultiPlayerGameType != 1 || m_bLevelComplete ) )
	{
		for ( TINT i = 0; i < APlayerManager::MAX_PLAYERS; i++ )
		{
			Rank& rRank = m_aoRanks[ i ];

			if ( rRank.m_iPlayer < 0 )
				continue;

			rRank.m_fPercent = ( TFLOAT( rRank.m_iScore ) * 100.0f ) / TFLOAT( iTotalPoints );
			TMath::Clip( rRank.m_fPercent, 0.0f, 100.0f );

			ABlobEntity*     pBlob  = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( rRank.m_iPlayer ) ) );
			AModelInstance2* pCrown = pBlob->GetAnimationComponent()->GetCrownModelInstance();

			if ( rRank.m_iRank == 1 )
			{
				if ( pCrown != TNULL )
					pCrown->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
			}
			else if ( pCrown != TNULL )
			{
				pCrown->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
			}
		}
	}

	TASSERT( m_aoRanks[ 0 ].m_iPlayer >= 0 );
	m_iLeader = m_aoRanks[ 0 ].m_iPlayer;

	if ( GetLeaderBoardHud() != TNULL )
		GetLeaderBoardHud()->UpdateRanks();
}

// $deBlob: FUNCTION 005bfbe0
void AGameStatus::UpdateTimedMatch()
{
	m_iLeader = 0;

	if ( m_oLevelData.m_fTimeLimit != -1.0f && m_oLevelData.m_fTimeLeft <= 0.0f )
	{
		m_bLevelComplete = TTRUE;
		UpdateRanks();

		if ( ABlobHud::GetSingleton() != TNULL )
			ABlobHud::GetSingleton()->ShowMessage( TNULL );

		m_bUnk845 = TTRUE;

		// The leader cheers, the others lose
		APlayerManager* pPlayerManager = APlayerManager::GetSingleton();

		for ( TINT i = 0; i < pPlayerManager->GetNumPlayers(); i++ )
		{
			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pPlayerManager->GetPlayerBlob( TUINT8( i ) ) );

			if ( i == m_iLeader )
				pBlob->SetStateMPWinner();
			else
				pBlob->SetStateMPLoser();
		}

		ABlobInputMap::GetSingleton()->DisableFor( -1.0f );
	}
}

// $deBlob: FUNCTION 005bd170
void AGameStatus::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bLevelComplete )
	{
		// The multiplayer game type 1 keeps updating after the end
		if ( m_iMultiPlayerGameType == 1 )
		{
			UpdateItMarker( a_fDeltaTime );
		}

		return;
	}

	m_oLevelData.Update( a_fDeltaTime );

	if ( m_oLevelData.m_bMultiplayer )
	{
		switch ( m_iMultiPlayerGameType )
		{
			case 0:
			case 3:
				UpdateTimedMatch();
				return;
			case 1:
				UpdateItMatch( a_fDeltaTime );
				return;
			case 2:
				UpdateGlintMatch( a_fDeltaTime );
				return;
		}

		return;
	}

	TASSERT( APlayerManager::GetSingleton()->GetNumPlayers() == 1 );

	// The time limit ran out: Blob loses (from a landmark or a boss it leaves the state first)
	if ( m_oLevelData.m_fTimeLimit != -1.0f && m_oLevelData.m_fTimeLeft <= 0.0f )
	{
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

		switch ( pBlob->GetBlobState() )
		{
			case ABlobEntity::STATE_ROLL:
			case ABlobEntity::STATE_BALLISTIC:
			case ABlobEntity::STATE_STUCKWALL:
			case ABlobEntity::STATE_HOTPLATE:
			case ABlobEntity::STATE_FLOAT:
				pBlob->OnOutOfPoints( TTRUE );
				return;
			case ABlobEntity::STATE_LMARK:
				pBlob->ExitLandmark( TFALSE );
				return;
			case ABlobEntity::STATE_BOSS:
				pBlob->ExitBoss( TFALSE );
				return;
		}
	}
}

// $deBlob: FUNCTION 005bddd0
void AGameStatus::OnGameOver()
{
	m_iLeader = 0;
	ABlobInputMap::GetSingleton()->DisableFor( -1.0f );
	m_bLevelComplete = TTRUE;
}

// $deBlob: FUNCTION 005bd4e0
void AGameStatus::OnLevelComplete()
{
	m_bLevelComplete = TTRUE;
	m_oLevelData.Update( 0.0f );

	if ( m_eLevelMode != Mode_FreePaint )
	{
		if ( m_oLevelData.m_pLevelProps->m_sprintLevel == 0 )
		{
			m_bUnk845 = TTRUE;
			m_iLeader = 0;

			AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();
			m_oLevelData.m_iNumChallenges        = pChallengeManager->GetTotalNumChallenges() - pChallengeManager->GetChallengesData()->m_uiNumNull - pChallengeManager->GetNumSubChallenges();
			TASSERT( m_oLevelData.m_iNumChallenges > 0 );
			m_oLevelData.m_iNumCompletedChallenges = pChallengeManager->GetTotalCompletedChallenges();
			goto LevelEnded;
		}

		if ( ( m_oLevelData.m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ] & ( EnSaveData::AWARD_Bronze | EnSaveData::AWARD_Silver | EnSaveData::AWARD_Gold ) ) == 0 )
		{
			APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, 0x74, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );
			goto LevelEnded;
		}
	}

	m_iLeader = 0;
	m_bUnk845 = TTRUE;

LevelEnded:
	ABlobInputMap::GetSingleton()->DisableFor( -1.0f );
}

// $deBlob: FUNCTION 005bb840
void AGameStatus::LevelData::AddGrassBombCollected( TINT a_iPlayer )
{
	TASSERT( a_iPlayer >= 0 && a_iPlayer < APlayerManager::GetSingleton()->GetNumPlayers() );
	m_aPlayerData[ a_iPlayer ].m_auiProgress[ 6 ]++;
}

// $deBlob: FUNCTION 005bc3a0
void AGameStatus::LevelData::OnBlimpPainted()
{
	TUINT32& rAchievements = m_aPlayerData[ 0 ].m_auiUnk3C[ 0 ];

	if ( !( rAchievements & EnSaveData::AWARD_Vista ) )
	{
		rAchievements |= EnSaveData::AWARD_Vista;
		ABlobHud::GetSingleton()->AddPendingAchievements( EnSaveData::AWARD_Vista );
	}
}

// $deBlob: FUNCTION 005bd6f0
void AGameStatus::SetMultiPlayerGameType( TINT a_iType )
{
	TASSERT( a_iType == GAMETYPE_MP_NONE || ( a_iType >= 0 && a_iType < GAMETYPE_MP_COUNT ) );
	m_iMultiPlayerGameType = a_iType;
}
