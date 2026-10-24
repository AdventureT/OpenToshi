#include "pch.h"
#include "Cameras/ACameraManager.h"
#include "Buildings/ABuildingInstance.h"
#include "Buildings/ABuildingManager.h"
#include "ABlobNavRingComponent.h"
#include "ABlobEntity.h"
#include "ABlobRibbonTrailComponent.h"
#include "ABlobColourComponent.h"
#include "Cameras/ACamera.h"
#include "Challenges/AChallengeManager.h"
#include "Challenges/ARaceChallenge.h"
#include "Challenges/AUberChallenge.h"
#include "Challenges/APaintChallenge.h"
#include "Challenges/ALandmarkChallenge.h"
#include "Challenges/ACivilianChallenge.h"
#include "Challenges/ACombatChallenge.h"
#include "Entities/AEnemyWrapper/AEnemyWrapper.h"
#include "Entities/APropTriggerEnemyGroup/APropTriggerEnemyGroup.h"
#include "Entities/ATankSpawner/ATankSpawner.h"
#include "Entities/ACar/ACar.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ATreeInstance.h"
#include "Terrain2/TerrainTypes.h"
#include "GameController/AGameStatus.h"
#include "Entities/ACiv.h"
#include "Entities/AEntityHash.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/ANavMarkers.h"
#include "Entities/APropWaterHydrant.h"
#include "Entities/APropGrassBomb.h"
#include "Entities/ABigPaintBot/ABigPaintBot.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"

#include <XUI/TXUI.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUIVisual.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The icons of the target scene (off_986758)
static const TWCHAR* s_awszIcons[] = {
	L"rib_challenge_giver.tga",
	L"rib_unpainted.tga",
	L"rib_landmark.tga",
	L"rib_combat_challenge.tga",
	L"rib_race_challenge.tga",
	L"rib_paint_bot.tga", // off_98676C
	L"rib_water.tga",     // off_986770
	L"rib_grass_bomb.tga",
};

static TCONSTEXPR_VAR TUINT32 s_uiColourIcon0 = 0xffae280d;
static TCONSTEXPR_VAR TUINT32 s_uiColourIcon1 = 0xfff1d507;
static TCONSTEXPR_VAR TUINT32 s_uiColourIcon2 = 0xff0e697b;

static TFLOAT s_fFadeTime = 0.1f;

static TFLOAT s_fIconOffset             = 7.5f; // the radius of the ring the icons are on
static TFLOAT s_fIconScaleBig           = 0.6f;
static TFLOAT s_fIconScaleSmall         = 0.4f;
static TFLOAT s_fIconsInterval          = 1.0f;
static TFLOAT s_fWaterInterval          = 1.0f;
static TFLOAT s_fTargetInterval         = 1.0f; // between two searches of the target
static TFLOAT s_fCivilianTargetInterval = 2.0f; // the same for the civilian challenges

static TXUIScene* CreateIconScene( const TWCHAR* a_wszImage, TUINT32 a_uiColour, TBOOL a_bColour )
{
	TXUI*      pXUI   = TXUI::GetSingleton();
	TXUIScene* pScene = pXUI->CreateScene( pXUI->FindResource( "ingame.xur" ), L"NavRingIcon", TFALSE );

	if ( a_wszImage )
		pScene->SetPresenterImagePath( a_wszImage, 0 );

	if ( a_bColour )
		pScene->GetVisual()->FindChildXUIElement( L"Icon" )->SetColour( a_uiColour );

	return pScene;
}

// A paint bot icon of colour a_uiColour
// $deBlob: FUNCTION 007bf920
TXUIScene* ABlobNavRingComponent::CreatePaintBotIconScene( TUINT32 a_uiColour )
{
	return CreateIconScene( s_awszIcons[ 5 ], a_uiColour, TTRUE );
}

// $deBlob: FUNCTION 007bdfd0
ABlobNavRingComponent::ABlobNavRingComponent( ABlobEntity* a_pBlob )
{
	m_pBlob             = a_pBlob;
	m_eState            = STATE_HIDDEN;
	m_uiFlags           = 0;
	m_fAlpha            = 1e-06f;
	m_fTargetTimer      = 0.0f;
	m_pCurrentChallenge = TNULL;

	for ( TINT i = 0; i < TARRAYSIZE( m_auiChallengeTarget ); i++ )
		m_auiChallengeTarget[ i ] = 0;

	m_pTargetEntity      = TNULL;
	m_pNearestChallenge  = TNULL;
	m_pChallengeEntity   = TNULL;
	m_apIconTargets[ 0 ] = TNULL;
	m_apIconTargets[ 1 ] = TNULL;
	m_apIconTargets[ 2 ] = TNULL;
	m_pWaterTarget       = TNULL;
	m_pWaterMarker       = TNULL;
	m_iBuildingIndex     = 0;
	m_pTargetBuilding    = TNULL;
	m_apIconScenes[ 0 ]  = TNULL;
	m_apIconScenes[ 1 ]  = TNULL;
	m_apIconScenes[ 2 ]  = TNULL;
	m_apIconScenes[ 3 ]  = TNULL;
	m_apIconScenes[ 4 ]  = TNULL;
	m_iTargetIcon        = -1;
	m_iShownTargetIcon   = -1;

	m_apIconScenes[ 2 ] = CreatePaintBotIconScene( s_uiColourIcon0 );
	m_apIconScenes[ 3 ] = CreatePaintBotIconScene( s_uiColourIcon1 );
	m_apIconScenes[ 4 ] = CreatePaintBotIconScene( s_uiColourIcon2 );
	m_apIconScenes[ 0 ] = CreateIconScene( TNULL, 0, TFALSE );
	m_apIconScenes[ 1 ] = CreateIconScene( s_awszIcons[ 6 ], 0, TFALSE );
}

// $deBlob: FUNCTION 007be210
void ABlobNavRingComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_eState != STATE_HIDDEN )
	{
		ABlobRibbonTrailComponent* pTrail = m_pBlob->m_pRibbonTrail;

		pTrail->m_uiFlags |= ABlobRibbonTrailComponent::FLAGS_ARROW;
		pTrail->m_vArrowScale.x = m_fAlpha;
		pTrail->m_vArrowScale.y = ABlobEntity::GetProperties( m_pBlob->m_iSize )->m_fTrailScale * m_fAlpha;
		pTrail->m_vArrowScale.z = m_fAlpha;
		pTrail->m_vArrowScale.w = 1.0f;
	}

	if ( m_iTargetIcon != m_iShownTargetIcon )
	{
		m_apIconScenes[ 0 ]->SetPresenterImagePath( s_awszIcons[ m_iTargetIcon ], 0 );
		m_iShownTargetIcon = m_iTargetIcon;
	}

	switch ( m_eState )
	{
		case STATE_SHOWING:
			m_fAlpha += a_fDeltaTime / s_fFadeTime;

			if ( m_fAlpha > 1.0f )
			{
				m_fAlpha = 1.0f;
				m_eState = STATE_SHOWN;
			}

			// Fallthrough
		case STATE_SHOWN:
			UpdateTarget( a_fDeltaTime );
			UpdateWaterTarget( a_fDeltaTime );
			UpdateIcons( a_fDeltaTime );
			break;
		case STATE_HIDING:
			m_fAlpha -= a_fDeltaTime / s_fFadeTime;

			if ( m_fAlpha < 1e-06f )
			{
				m_fAlpha = 1e-06f;
				m_pBlob->m_pRibbonTrail->m_uiFlags &= ~ABlobRibbonTrailComponent::FLAGS_ARROW;
				m_eState = STATE_HIDDEN;
			}

			break;
	}

	if ( !( TUINT8( m_uiFlags >> 3 ) & 1 ) )
		m_apIconScenes[ 0 ]->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );

	if ( !( TUINT8( m_uiFlags >> 4 ) & 1 ) )
		m_apIconScenes[ 1 ]->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
}

// $deBlob: FUNCTION 007be3a0
void ABlobNavRingComponent::Show()
{
	if ( m_eState == STATE_HIDDEN || m_eState == STATE_HIDING )
	{
		ResetTarget();
		ResetIconTargets();
		ResetWaterTarget();
		ResetBuildingTarget();
		m_eState = STATE_SHOWING;
	}
}

// $deBlob: FUNCTION 007be460
void ABlobNavRingComponent::Hide()
{
	if ( m_eState == STATE_SHOWING || m_eState == STATE_SHOWN )
	{
		ResetTarget();
		ResetIconTargets();
		ResetWaterTarget();
		ResetBuildingTarget();
		m_eState = STATE_HIDING;
	}
}

// $deBlob: FUNCTION 007be520
void ABlobNavRingComponent::ResetTarget()
{
	m_pCurrentChallenge = TNULL;

	for ( TINT i = 0; i < TARRAYSIZE( m_auiChallengeTarget ); i++ )
		m_auiChallengeTarget[ i ] = 0;

	m_pTargetEntity     = TNULL;
	m_pNearestChallenge = TNULL;
	m_pChallengeEntity  = TNULL;
	m_fTargetTimer      = 0.0f;

	if ( m_uiFlags & FLAGS_TARGET_DISPLAY )
		m_uiFlags &= ~FLAGS_TARGET_DISPLAY;
}

// $deBlob: FUNCTION 007be570
void ABlobNavRingComponent::ResetIconTargets()
{
	m_apIconTargets[ 0 ] = TNULL;
	m_apIconTargets[ 1 ] = TNULL;
	m_apIconTargets[ 2 ] = TNULL;
	m_apIconScenes[ 3 ]->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
	m_apIconScenes[ 4 ]->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
	m_apIconScenes[ 2 ]->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
	m_uiFlags &= ~( FLAGS_ICON0_DISPLAY | FLAGS_ICON1_DISPLAY | FLAGS_ICON2_DISPLAY );
	m_fIconTimer = 0.2f;
}

// $deBlob: FUNCTION 007be5e0
void ABlobNavRingComponent::ResetWaterTarget()
{
	m_pWaterTarget = TNULL;
	m_pWaterMarker = TNULL;
	m_fWaterTimer  = 0.4f;

	if ( m_uiFlags & FLAGS_WATER_DISPLAY )
		m_uiFlags &= ~FLAGS_WATER_DISPLAY;
}

// $deBlob: FUNCTION 007be610
void ABlobNavRingComponent::ResetBuildingTarget()
{
	// The target's projection 0x5A is removed
	if ( m_pTargetBuilding != TNULL && m_pTargetBuilding->m_iGlintIndex == 0x5A )
		AGameStatus::RemoveBuildingProjection( m_pTargetBuilding );

	m_iBuildingIndex  = 0;
	m_pTargetBuilding = TNULL;
}

// $deBlob: FUNCTION 007be640
void ABlobNavRingComponent::UpdateTarget( TFLOAT a_fDeltaTime )
{
	ABlobEntity* pBlob = m_pBlob;

	// Inky: no target
	if ( TUINT8( pBlob->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 )
	{
		ResetTarget();
		return;
	}

	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetLevelMode() == AGameStatus::Mode_FreePaint )
		return;

	if ( pGameStatus->GetLevelMode() == 2 )
	{
		if ( ABuildingManager::GetSingleton()->GetNumBuildings() > 0 )
			UpdateBuildingTarget( a_fDeltaTime );

		return;
	}

	// The challenge played (the current sub challenge of an uber challenge), none for a null challenge
	AChallenge* pChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();

	if ( pChallenge != TNULL )
	{
		if ( pChallenge->GetType() == AChallenge::Type_Null )
			pChallenge = TNULL;
		else if ( pChallenge->GetType() == AChallenge::Type_Uber )
			pChallenge = TSTATICCAST( AUberChallenge, pChallenge )->GetCurrentSubChallenge();
	}

	if ( m_pCurrentChallenge != pChallenge )
		ResetTarget();

	m_pCurrentChallenge = pChallenge;

	const TVector4* pTarget;
	TVector4        vTarget;

	if ( pChallenge == TNULL )
	{
		// No challenge: the nearest marker
		if ( m_pNearestChallenge != TNULL && TSTATICCAST( AChallenge, m_pNearestChallenge )->IsCompleted() )
			m_pNearestChallenge = TNULL;

		if ( m_pChallengeEntity != TNULL && !TSTATICCAST( ABaseEntity, m_pChallengeEntity )->IsTargetable() )
			m_pChallengeEntity = TNULL;

		TFLOAT fTimer  = m_fTargetTimer;
		m_fTargetTimer = fTimer - a_fDeltaTime;

		if ( m_pNearestChallenge == TNULL || fTimer - a_fDeltaTime <= 0.0f )
		{
			m_fTargetTimer      = s_fTargetInterval;
			m_pNearestChallenge = AChallengeManager::GetSingleton()->FindNearestAvailableChallenge( pBlob->GetBodyPosition() );
			m_pChallengeEntity  = TNULL;

			// The nearest targetable active grass bomb (AGameStatus::LevelData's list 0x78C)
			APropGrassBomb::BombList& rBombs = *TREINTERPRETCAST( APropGrassBomb::BombList*, &AGameStatus::GetSingleton()->GetLevelData().m_oList78C );

			if ( !rBombs.IsEmpty() )
			{
				TFLOAT fMinDistanceSq = TMath::MAXFLOAT;

				for ( APropGrassBomb::BombList::Iterator it = rBombs.Begin(); it != rBombs.End(); it++ )
				{
					ABaseEntity* pBomb = it->m_pOwner;

					if ( pBomb->IsTargetable() )
					{
						TFLOAT fDistanceSq = TVector4::DistanceSq( pBlob->GetBodyPosition(), pBomb->GetMatrix().GetTranslation() );

						if ( fDistanceSq < fMinDistanceSq )
						{
							fMinDistanceSq     = fDistanceSq;
							m_pChallengeEntity = it->m_pOwner;
						}
					}
				}
			}
		}

		AChallenge*  pMarker = TSTATICCAST( AChallenge, m_pNearestChallenge );
		ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, m_pChallengeEntity );

		if ( pEntity == TNULL )
		{
			if ( pMarker == TNULL )
			{
				if ( ABuildingManager::GetSingleton()->GetNumBuildings() < 1 )
					return;

				UpdateBuildingTarget( a_fDeltaTime );
				return;
			}

			vTarget       = pMarker->GetData()->m_pMatrix->GetTranslation();
			m_iTargetIcon = 0;
			pTarget       = &vTarget;
		}
		else if ( pMarker == TNULL )
		{
			m_iTargetIcon = 7;
			pTarget       = &pEntity->GetMatrix().GetTranslation();
		}
		else
		{
			// The nearer of the two
			vTarget = pMarker->GetData()->m_pMatrix->GetTranslation();

			if ( TVector4::DistanceSq( pBlob->GetBodyPosition(), pEntity->GetMatrix().GetTranslation() ) <= TVector4::DistanceSq( pBlob->GetBodyPosition(), vTarget ) )
			{
				pTarget       = &pEntity->GetMatrix().GetTranslation();
				m_iTargetIcon = 7;
			}
			else
			{
				pTarget       = &vTarget;
				m_iTargetIcon = 0;
			}
		}
	}
	else
	{
		TVector4 vBlob = pBlob->GetBodyPosition(); // local_2c of the cases below
		pTarget        = TNULL;

		switch ( pChallenge->GetType() )
		{
			case AChallenge::Type_Race:
				pTarget = TSTATICCAST( ARaceChallenge, pChallenge )->GetNextCheckpoint();

				if ( pTarget != TNULL )
					m_iTargetIcon = 4;

				break;
			case AChallenge::Type_ZJump:
			case AChallenge::Type_Car:
			case AChallenge::Type_Null:
			case AChallenge::Type_Uber:
				// No target
				return;
			case AChallenge::Type_Paint:
			{
				// The nearest target not painted yet: a building or a landmark (its m_iDone is set once done) or a tree
				APaintChallenge::TypeData* pData    = TSTATICCAST( APaintChallenge::TypeData, pChallenge->GetData() );
				APaintChallenge::Target*   pCurrent = TSTATICCAST( APaintChallenge::Target, TREINTERPRETCAST( void*, m_auiChallengeTarget[ 0 ] ) );

				if ( pCurrent != TNULL )
				{
					TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, pCurrent->m_pInstance );
					TUINT32                     uiType    = pInstance->m_uiFlags & 7;

					if ( uiType != 0 )
					{
						if ( uiType <= TerrainTypes::InstanceType_Landmark )
						{
							if ( pCurrent->m_iDone != 0 )
								m_auiChallengeTarget[ 0 ] = 0;
						}
						else if ( uiType == TerrainTypes::InstanceType_Tree )
						{
							ATreeInstance* pTree = TSTATICCAST( ATreeInstance, pInstance->m_pModelInstance2 );

							if ( pTree->m_bTransformed || pTree->m_bTransforming )
								m_auiChallengeTarget[ 0 ] = 0;
						}
					}
				}

				TBOOL bNoTarget = m_auiChallengeTarget[ 0 ] == 0;
				m_fTargetTimer -= a_fDeltaTime;

				if ( bNoTarget || m_fTargetTimer <= 0.0f )
				{
					m_fTargetTimer        = s_fTargetInterval;
					TFLOAT fMinDistanceSq = TMath::MAXFLOAT;

					for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
					{
						APaintChallenge::Target*    pTarget2  = &pData->m_pTargets[ i ];
						TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, pTarget2->m_pInstance );
						TUINT32                     uiType    = pInstance->m_uiFlags & 7;

						if ( uiType == 0 )
							continue;

						if ( uiType <= TerrainTypes::InstanceType_Landmark )
						{
							// NOTE: retail reads the flags of the instance before checking it
							if ( pInstance == TNULL || pTarget2->m_iDone != 0 )
								continue;
						}
						else if ( uiType == TerrainTypes::InstanceType_Tree )
						{
							ATreeInstance* pTree = TSTATICCAST( ATreeInstance, pInstance->m_pModelInstance2 );

							if ( pTree == TNULL || pTree->m_bTransformed || pTree->m_bTransforming )
								continue;
						}
						else
						{
							continue;
						}

						TFLOAT fDistanceSq = TVector4::DistanceSq( **pInstance->m_ppCentre, vBlob );

						if ( fDistanceSq < fMinDistanceSq )
						{
							fMinDistanceSq            = fDistanceSq;
							m_auiChallengeTarget[ 0 ] = TUINT32( TREINTERPRETCAST( TUINTPTR, pTarget2 ) );
						}
					}
				}

				if ( m_auiChallengeTarget[ 0 ] != 0 )
				{
					APaintChallenge::Target* pFound = TREINTERPRETCAST( APaintChallenge::Target*, m_auiChallengeTarget[ 0 ] );
					m_iTargetIcon                   = 1;
					pTarget                         = *TSTATICCAST( TerrainTypes::CellInstance, pFound->m_pInstance )->m_ppCentre;
				}

				break;
			}
			case AChallenge::Type_Landmark:
			{
				// The nearest landmark with triggers left
				ALandmarkChallenge::TypeData* pData     = TSTATICCAST( ALandmarkChallenge::TypeData, pChallenge->GetData() );
				ALandmarkInstance**           ppCurrent = TREINTERPRETCAST( ALandmarkInstance**, m_auiChallengeTarget[ 1 ] );

				if ( ppCurrent != TNULL && *ppCurrent != TNULL && ( *ppCurrent )->GetNumTriggersLeft() == 0 )
					m_auiChallengeTarget[ 1 ] = 0;

				TBOOL bNoTarget = m_auiChallengeTarget[ 1 ] == 0;
				m_fTargetTimer -= a_fDeltaTime;

				if ( bNoTarget || m_fTargetTimer <= 0.0f )
				{
					m_fTargetTimer        = s_fTargetInterval;
					TFLOAT fMinDistanceSq = TMath::MAXFLOAT;

					for ( TUINT32 i = 0; i < pData->m_uiNumLandmarks; i++ )
					{
						ALandmarkInstance** ppLandmark = TREINTERPRETCAST( ALandmarkInstance**, &pData->m_ppLandmarks[ i ] );

						if ( *ppLandmark != TNULL && ( *ppLandmark )->GetNumTriggersLeft() > 0 )
						{
							TFLOAT fDistanceSq = TVector4::DistanceSq( **( *ppLandmark )->GetTerrainInstance()->m_ppCentre, vBlob );

							if ( fDistanceSq < fMinDistanceSq )
							{
								fMinDistanceSq            = fDistanceSq;
								m_auiChallengeTarget[ 1 ] = TUINT32( TREINTERPRETCAST( TUINTPTR, ppLandmark ) );
							}
						}
					}
				}

				if ( m_auiChallengeTarget[ 1 ] != 0 )
				{
					m_iTargetIcon = 2;
					pTarget       = *( *TREINTERPRETCAST( ALandmarkInstance**, m_auiChallengeTarget[ 1 ] ) )->GetTerrainInstance()->m_ppCentre;
				}

				break;
			}
			case AChallenge::Type_Civilian:
			{
				// The nearest entity not done yet (its m_iDone is set once done)
				ACivilianChallenge::TypeData* pData    = TSTATICCAST( ACivilianChallenge::TypeData, pChallenge->GetData() );
				ACivilianChallenge::Entry*    pCurrent = TSTATICCAST( ACivilianChallenge::Entry, m_pTargetEntity );

				if ( pCurrent != TNULL && pCurrent->m_iDone != 0 )
					m_pTargetEntity = TNULL;

				TBOOL bNoTarget = m_pTargetEntity == TNULL;
				m_fTargetTimer -= a_fDeltaTime;

				if ( bNoTarget || m_fTargetTimer <= 0.0f )
				{
					m_fTargetTimer        = s_fCivilianTargetInterval;
					TFLOAT fMinDistanceSq = TMath::MAXFLOAT;

					for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
					{
						ACivilianChallenge::Entry* pEntry = &pData->m_pEntries[ i ];

						if ( pEntry->m_iDone == 0 )
						{
							TFLOAT fDistanceSq = TVector4::DistanceSq( pEntry->m_pEntity->GetMatrix().GetTranslation(), vBlob );

							if ( fDistanceSq < fMinDistanceSq )
							{
								fMinDistanceSq  = fDistanceSq;
								m_pTargetEntity = pEntry;
							}
						}
					}
				}

				if ( m_pTargetEntity != TNULL )
				{
					m_iTargetIcon = 1;
					pTarget       = &TSTATICCAST( ACivilianChallenge::Entry, m_pTargetEntity )->m_pEntity->GetMatrix().GetTranslation();
				}

				break;
			}
			case AChallenge::Type_Combat:
			{
				// The nearest enemy left: m_auiChallengeTarget[ 2 ] is the entry, m_auiChallengeTarget[ 3 ] its AEnemyWrapper (types 0 and 1)
				ACombatChallenge::TypeData* pData    = TSTATICCAST( ACombatChallenge::TypeData, pChallenge->GetData() );
				ACombatChallenge::Entry*    pCurrent = TREINTERPRETCAST( ACombatChallenge::Entry*, m_auiChallengeTarget[ 2 ] );

				if ( pCurrent != TNULL && ( pCurrent->m_iDone != 0 || ( pCurrent->m_eType == 0 && !TREINTERPRETCAST( AEnemyWrapper*, m_auiChallengeTarget[ 3 ] )->IsAlive() ) ) )
				{
					m_auiChallengeTarget[ 2 ] = 0;
					m_auiChallengeTarget[ 3 ] = 0;
				}

				TFLOAT fTimer  = m_fTargetTimer;
				m_fTargetTimer = fTimer - a_fDeltaTime;

				if ( m_auiChallengeTarget[ 2 ] == 0 || fTimer - a_fDeltaTime <= 0.0f )
				{
					m_fTargetTimer            = s_fTargetInterval;
					m_auiChallengeTarget[ 2 ] = 0;
					m_auiChallengeTarget[ 3 ] = 0;
					TFLOAT fMinDistanceSq     = TMath::MAXFLOAT;

					for ( TUINT32 i = 0; i < pData->m_uiNumEntries; i++ )
					{
						ACombatChallenge::Entry* pEntry = &pData->m_pEntries[ i ];

						if ( pEntry->m_iDone != 0 )
							continue;

						switch ( pEntry->m_eType )
						{
							case 0:
							{
								// An enemy group: its living enemies, but the ignored LeechBots
								APropTriggerEnemyGroup* pGroup = TSTATICCAST( APropTriggerEnemyGroup, pEntry->m_pEntity );

								for ( TINT j = 0; j < pGroup->GetNumEnemies(); j++ )
								{
									AEnemyWrapper* pWrapper = pGroup->GetEntityTarget( j )->CastTo<AEnemyWrapper>();

									if ( pWrapper->IsAlive() )
									{
										if ( pWrapper->GetEnemyType() != AEnemyWrapper::ENEMYTYPE_LeechBot2 || !ALeechBot2::IsFixedColourType( pWrapper->GetCustomData() ) )
										{
											TFLOAT fDistanceSq = TVector4::DistanceSq( pWrapper->GetPosition(), vBlob );

											if ( fDistanceSq < fMinDistanceSq )
											{
												fMinDistanceSq            = fDistanceSq;
												m_auiChallengeTarget[ 2 ] = TREINTERPRETCAST( TUINT32, pEntry );
												m_auiChallengeTarget[ 3 ] = TREINTERPRETCAST( TUINT32, pWrapper );
											}
										}
									}
								}

								break;
							}
							case 1:
							{
								AEnemyWrapper* pWrapper = TSTATICCAST( AEnemyWrapper, pEntry->m_pEntity );

								if ( pWrapper->IsAlive() )
								{
									TFLOAT fDistanceSq = TVector4::DistanceSq( pWrapper->GetPosition(), vBlob );

									if ( fDistanceSq < fMinDistanceSq )
									{
										m_auiChallengeTarget[ 2 ] = TREINTERPRETCAST( TUINT32, pEntry );
										fMinDistanceSq            = fDistanceSq;
										m_auiChallengeTarget[ 3 ] = TREINTERPRETCAST( TUINT32, pWrapper );
									}
								}

								break;
							}
							case 2:
							case 3:
								// A car or a turret
								if ( pEntry->m_pEntity != TNULL )
								{
									TFLOAT fDistanceSq = TVector4::DistanceSq( pEntry->m_pEntity->GetMatrix().GetTranslation(), vBlob );

									if ( fDistanceSq < fMinDistanceSq )
									{
										fMinDistanceSq            = fDistanceSq;
										m_auiChallengeTarget[ 2 ] = TREINTERPRETCAST( TUINT32, pEntry );
										m_auiChallengeTarget[ 3 ] = 0;
									}
								}

								break;
							case 4:
							{
								// A tank spawner: its tank
								ATankSpawner* pSpawner = TSTATICCAST( ATankSpawner, pEntry->m_pEntity );

								if ( pSpawner != TNULL && pSpawner->GetTank() != TNULL )
								{
									TFLOAT fDistanceSq = TVector4::DistanceSq( pSpawner->GetTank()->GetMatrix().GetTranslation(), vBlob );

									if ( fDistanceSq < fMinDistanceSq )
									{
										fMinDistanceSq            = fDistanceSq;
										m_auiChallengeTarget[ 2 ] = TREINTERPRETCAST( TUINT32, pEntry );
										m_auiChallengeTarget[ 3 ] = 0;
									}
								}

								break;
							}
						}
					}
				}

				ACombatChallenge::Entry* pTargetEntry = TREINTERPRETCAST( ACombatChallenge::Entry*, m_auiChallengeTarget[ 2 ] );

				if ( pTargetEntry != TNULL )
				{
					m_iTargetIcon = 3;

					switch ( pTargetEntry->m_eType )
					{
						case 0:
						case 1:
							pTarget = &TREINTERPRETCAST( AEnemyWrapper*, m_auiChallengeTarget[ 3 ] )->GetPosition();
							break;
						case 2:
						case 3:
							pTarget = &pTargetEntry->m_pEntity->GetMatrix().GetTranslation();
							break;
						case 4:
							pTarget = &TSTATICCAST( ATankSpawner, pTargetEntry->m_pEntity )->GetTank()->GetMatrix().GetTranslation();
							break;
						default:
							// Shown without placing its icon
							if ( !( m_uiFlags & FLAGS_TARGET_DISPLAY ) )
							{
								m_apIconScenes[ 0 ]->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
								m_uiFlags |= FLAGS_TARGET_DISPLAY;
							}

							return;
					}
				}

				break;
			}
			default:
				return;
		}

		if ( pTarget == TNULL )
		{
			if ( m_uiFlags & FLAGS_TARGET_DISPLAY )
				m_uiFlags &= ~FLAGS_TARGET_DISPLAY;

			return;
		}
	}

	PlaceIcon( m_apIconScenes[ 0 ], *pTarget, s_fIconScaleBig );

	if ( m_uiFlags & FLAGS_TARGET_DISPLAY )
		return;

	m_apIconScenes[ 0 ]->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
	m_uiFlags |= FLAGS_TARGET_DISPLAY;
}

// The distance from a_rcPosition to the bounds of a building's instance (its centre and its extents, m_ppCentre)
static TFLOAT ABlobNavRingComponent_GetBuildingDistance( const TVector4& a_rcPosition, ABuildingInstance* a_pBuilding )
{
	TVector4** ppBounds = a_pBuilding->m_pTerrainInstance->m_ppCentre;
	return TVector4::Distance( a_rcPosition, *ppBounds[ 0 ] ) - ppBounds[ 1 ]->Magnitude();
}

// The buildings UpdateBuildingTarget looks at per update
static TINT s_iBuildingsPerUpdate = 50;

// $deBlob: FUNCTION 007bf2b0
void ABlobNavRingComponent::UpdateBuildingTarget( TFLOAT a_fDeltaTime )
{
	ABuildingInstance* pOldTarget = m_pTargetBuilding;
	TVector4           vBlob      = m_pBlob->GetBodyPosition();
	TFLOAT             fBest      = TMath::MAXFLOAT;

	// A painted target is dropped
	if ( pOldTarget != TNULL && ( pOldTarget->m_uiFlags & ABuildingInstance::Flags_Painted ) )
	{
		m_pTargetBuilding = TNULL;
		m_iBuildingIndex  = 0;

		if ( m_uiFlags & FLAGS_TARGET_DISPLAY )
			m_uiFlags &= ~FLAGS_TARGET_DISPLAY;
	}

	if ( m_pTargetBuilding != TNULL )
		fBest = ABlobNavRingComponent_GetBuildingDistance( vBlob, m_pTargetBuilding );

	ABuildingManager* pManager      = ABuildingManager::GetSingleton();
	TINT              iNumBuildings = pManager->GetNumBuildings();
	TINT              iCount        = s_iBuildingsPerUpdate - 1;

	if ( s_iBuildingsPerUpdate != 1 )
	{
		while ( TTRUE )
		{
			// All looked at: the target is shown
			if ( m_iBuildingIndex >= iNumBuildings )
			{
				if ( m_pTargetBuilding != TNULL && !( m_uiFlags & FLAGS_TARGET_DISPLAY ) )
				{
					m_apIconScenes[ 0 ]->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
					m_uiFlags |= FLAGS_TARGET_DISPLAY;
				}

				m_iBuildingIndex = 0;
			}

			TASSERT( m_iBuildingIndex < pManager->GetNumBuildings() );
			ABuildingInstance*          pBuilding = pManager->GetBuilding( m_iBuildingIndex );
			TerrainTypes::CellInstance* pInstance = pBuilding->m_pTerrainInstance;

			if ( !( pBuilding->m_uiFlags & ABuildingInstance::Flags_Painted ) && pBuilding->IsAPaintableBuilding() )
			{
				TUINT32 uiFlags = pInstance->m_uiFlags;

				if ( ( uiFlags & 0x400 ) && !( uiFlags & 0x40000 ) && pInstance->m_pModelInstance2 != TNULL )
				{
					TFLOAT fDistance = ABlobNavRingComponent_GetBuildingDistance( vBlob, pBuilding );

					if ( fBest > fDistance )
					{
						fBest             = fDistance;
						m_pTargetBuilding = pBuilding;
					}
				}
			}

			m_iBuildingIndex++;

			if ( --iCount == 0 )
				break;
		}
	}

	// The glint of the target (projection 90)
	if ( m_pTargetBuilding != pOldTarget && pOldTarget != TNULL && pOldTarget->m_iGlintIndex == 90 )
		AGameStatus::RemoveBuildingProjection( pOldTarget );

	if ( m_pTargetBuilding != TNULL && m_pTargetBuilding->m_iGlintIndex < 0 )
		AGameStatus::SetBuildingProjection( m_pTargetBuilding, 90, TTRUE );

	if ( m_pTargetBuilding != TNULL )
	{
		m_iTargetIcon = 1;
		PlaceIcon( m_apIconScenes[ 0 ], *m_pTargetBuilding->m_pTerrainInstance->m_ppCentre[ 0 ], s_fIconScaleBig );

		if ( m_pTargetBuilding->m_iGlintIndex == 90 )
		{
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
			AGameStatus::UpdateGlint( m_pTargetBuilding, a_fDeltaTime );
			AGameStatus::UpdateGlintCamera( m_pTargetBuilding, pCamera, 0, TTRUE );
		}
	}
	else if ( m_uiFlags & FLAGS_TARGET_DISPLAY )
	{
		m_uiFlags &= ~FLAGS_TARGET_DISPLAY;
	}
}

// $deBlob: FUNCTION 007befb0
void ABlobNavRingComponent::UpdateWaterTarget( TFLOAT a_fDeltaTime )
{
	ABlobEntity* pBlob = m_pBlob;

	// In the water: no water icon
	if ( TUINT8( pBlob->m_uiFlags >> 7 ) & 1 )
	{
		m_pWaterTarget = TNULL;
		m_pWaterMarker = TNULL;
		m_fWaterTimer  = 0.4f;
	}
	else
	{
		TVector4 vBlob   = pBlob->GetBodyPosition();
		TFLOAT   fBest   = TMath::MAXFLOAT;
		TVector4 vTarget = TVector4::VEC_ZERO; // X0Y0Z0W1

		if ( m_pWaterTarget != TNULL )
		{
			const TVector4& rHydrant = TSTATICCAST( ABaseEntity, m_pWaterTarget )->GetMatrix().GetTranslation();
			fBest                    = TVector4::DistanceSq( rHydrant, vBlob );
			vTarget                  = rHydrant;
		}

		m_fWaterTimer -= a_fDeltaTime;

		if ( m_fWaterTimer <= 0.0f )
		{
			// The nearest water marker
			m_fWaterTimer  = s_fWaterInterval;
			m_pWaterMarker = TNULL;

			ANavMarkers*       pMarkers = ANavMarkers::GetSingleton();
			ANavMarkers::Data* pData    = pMarkers->GetData();
			TFLOAT             fNearest = TMath::MAXFLOAT;

			for ( TINT i = 0; i < pData->m_iNumMarkers; i++ )
			{
				if ( pData->m_pMarkers[ i ].m_uiFlags & 1 )
				{
					TFLOAT fDistSq = TVector4::DistanceSq( pData->m_pMarkers[ i ].m_vPosition, vBlob );

					if ( fDistSq < fNearest )
					{
						m_pWaterMarker = &pMarkers->GetData()->m_pMarkers[ i ];
						fNearest       = fDistSq;
					}
				}

				pData = pMarkers->GetData();
			}
		}

		if ( m_pWaterMarker != TNULL && TVector4::DistanceSq( TSTATICCAST( ANavMarkers::Marker, m_pWaterMarker )->m_vPosition, vBlob ) < fBest )
		{
			// The nearest point of the water path: the segment to the nearer neighbour
			ANavMarkers::Marker* pMarker = TSTATICCAST( ANavMarkers::Marker, m_pWaterMarker );
			ANavMarkers::Marker* pNext   = pMarker->m_apLinks[ 0 ];

			if ( pNext == TNULL )
			{
				pNext = pMarker->m_apLinks[ 1 ];
			}
			else if ( pMarker->m_apLinks[ 1 ] != TNULL )
			{
				TFLOAT fDistSq0 = TVector4::DistanceSq( vBlob, pNext->m_vPosition );
				TFLOAT fDistSq1 = TVector4::DistanceSq( vBlob, TSTATICCAST( ANavMarkers::Marker, m_pWaterMarker )->m_apLinks[ 1 ]->m_vPosition );
				pMarker         = TSTATICCAST( ANavMarkers::Marker, m_pWaterMarker );
				pNext           = ( fDistSq1 <= fDistSq0 ) ? pMarker->m_apLinks[ 1 ] : pMarker->m_apLinks[ 0 ];
			}

			if ( pNext == TNULL )
			{
				vTarget = pMarker->m_vPosition;
			}
			else
			{
				TVector4 vDir( pNext->m_vPosition.x - pMarker->m_vPosition.x, pNext->m_vPosition.y - pMarker->m_vPosition.y, pNext->m_vPosition.z - pMarker->m_vPosition.z, pNext->m_vPosition.w );
				TFLOAT   fLength = vDir.NormaliseMag();
				pMarker          = TSTATICCAST( ANavMarkers::Marker, m_pWaterMarker );

				TFLOAT fAlong = ( vBlob.y - pMarker->m_vPosition.y ) * vDir.y + ( vBlob.x - pMarker->m_vPosition.x ) * vDir.x + ( vBlob.z - pMarker->m_vPosition.z ) * vDir.z;

				if ( fAlong < 0.0f )
				{
					vTarget = pMarker->m_vPosition;
				}
				else if ( fAlong <= fLength )
				{
					vTarget.Multiply( vDir, fAlong );
					pMarker = TSTATICCAST( ANavMarkers::Marker, m_pWaterMarker );
					vTarget.y += pMarker->m_vPosition.y;
					vTarget.x += pMarker->m_vPosition.x;
					vTarget.z += pMarker->m_vPosition.z;
				}
				else
				{
					vTarget = pNext->m_vPosition;
				}
			}
		}

		if ( m_pWaterTarget != TNULL || m_pWaterMarker != TNULL )
		{
			TFLOAT fScale = s_fIconScaleSmall;

			if ( ( m_pBlob->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 )
				fScale = s_fIconScaleBig;

			PlaceIcon( m_apIconScenes[ 1 ], vTarget, fScale );

			if ( TUINT8( m_uiFlags >> 4 ) & 1 )
				return;

			m_apIconScenes[ 1 ]->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
			m_uiFlags |= FLAGS_WATER_DISPLAY;
			return;
		}
	}

	if ( TUINT8( m_uiFlags >> 4 ) & 1 )
		m_uiFlags &= ~FLAGS_WATER_DISPLAY;
}

// The entities of the icons: leechbots, paint bots, civilian platforms and water hydrants
// (ABlobNavRingComponent::UpdateEntities' local ANavRingEntityFilter)
class ANavRingEntityFilter : public AEntityHashFilter
{
public:
	// $deBlob: FUNCTION 007bfdf0
	virtual TBOOL operator()( ABaseEntity* a_pEntity ) override
	{
		if ( a_pEntity->GetClass() == &TGetClass( ALeechBot2 ) && a_pEntity->IsTargetable() )
			return TTRUE;

		if ( a_pEntity->GetClass() == &TGetClass( ABigPaintBot ) && a_pEntity->IsTargetable() )
			return TTRUE;

		if ( a_pEntity->GetClass() == &TGetClass( ACiv ) && ( *( TREINTERPRETCAST( TUINT8*, a_pEntity ) + 0x1F1 ) & 4 ) == 0 )
			return TTRUE;

		return a_pEntity->GetClass() == &TGetClass( APropWaterHydrant );
	}
};

// $deBlob: FUNCTION 007bf510
void ABlobNavRingComponent::UpdateIcons( TFLOAT a_fDeltaTime )
{
	TFLOAT fTimer = m_fIconTimer;
	m_fIconTimer  = fTimer - a_fDeltaTime;

	if ( fTimer - a_fDeltaTime <= 0.0f )
	{
		// The nearest leechbot of each kind and water hydrant
		m_fIconTimer = s_fIconsInterval;

		TVector4 vBlob = m_pBlob->GetBodyPosition();

		m_apIconTargets[ 0 ] = TNULL;
		m_apIconTargets[ 1 ] = TNULL;
		m_apIconTargets[ 2 ] = TNULL;
		m_pWaterTarget       = TNULL;

		TFLOAT afBest[ 3 ] = { TMath::MAXFLOAT, TMath::MAXFLOAT, TMath::MAXFLOAT };
		TFLOAT fBestWater  = TMath::MAXFLOAT;

		ANavRingEntityFilter oFilter;
		TINT                 iNumEntities;
		ABaseEntity**        ppEntities = AEntityHash::GetSingleton()->Query( vBlob, 150.0f, &iNumEntities, &oFilter, TTRUE, 500, TFALSE, TTRUE, TFALSE );

		for ( TUINT i = 0; i < TUINT( iNumEntities ); i++ )
		{
			ABaseEntity* pEntity = ppEntities[ i ];
			TINT         iIcon   = -1;

			if ( pEntity->GetClass() == &TGetClass( ALeechBot2 ) )
			{
				TASSERT( pEntity->IsTargetable() );

				TUINT8 uiType = TUINT8( TSTATICCAST( ALeechBot2, pEntity )->GetLeechbotType() );

				if ( uiType == 3 )
					iIcon = 1;
				else if ( uiType == 4 )
					iIcon = 2;
				else if ( uiType == 5 )
					iIcon = 0;
			}
			else
			{
				if ( pEntity->GetClass() == &TGetClass( ABigPaintBot ) )
				{
					TASSERT( pEntity->IsTargetable() );

					TUINT8 uiColour = TSTATICCAST( ABigPaintBot, pEntity )->GetColour();

					if ( uiColour == 1 )
						iIcon = 1;
					else if ( uiColour == 2 )
						iIcon = 2;
					else if ( uiColour == 4 )
						iIcon = 0;
				}
				else if ( ( m_pBlob->m_uiFlags & 0x80 ) == 0 && pEntity->GetClass() == &TGetClass( APropWaterHydrant ) )
				{
					TFLOAT fDistSq = TVector4::DistanceSq( pEntity->GetMatrix().GetTranslation(), vBlob );

					if ( fDistSq < fBestWater )
					{
						m_pWaterTarget = pEntity;
						fBestWater     = fDistSq;
					}
				}
			}

			if ( iIcon != -1 )
			{
				TFLOAT fDistSq = TVector4::DistanceSq( pEntity->GetMatrix().GetTranslation(), vBlob );

				if ( fDistSq < afBest[ iIcon ] )
				{
					m_apIconTargets[ iIcon ] = pEntity;
					afBest[ iIcon ]          = fDistSq;
				}
			}
		}
	}

	// The icons (1, 2, 0: retail's order) follow their entities while they're there
	static TCONSTEXPR_VAR TINT s_aiOrder[ 3 ] = { 1, 2, 0 };

	for ( TINT j = 0; j < 3; j++ )
	{
		TINT iIcon = s_aiOrder[ j ];
		UpdateIconScene( m_apIconScenes[ iIcon + 2 ], FLAGS( FLAGS_ICON0_DISPLAY << iIcon ), &m_apIconTargets[ iIcon ] );
	}
}

// $deBlob: FUNCTION 007bf9b0
void ABlobNavRingComponent::UpdateIconScene( TXUIScene* a_pScene, FLAGS a_uiFlag, void** a_ppTarget )
{
	if ( *a_ppTarget != TNULL )
	{
		ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, *a_ppTarget );

		if ( ( ( pEntity->GetFlags() >> 0x16 ) & 1 ) == 0 )
		{
			PlaceIcon( a_pScene, pEntity->GetMatrix().GetTranslation(), s_fIconScaleSmall );

			if ( *a_ppTarget != TNULL )
			{
				if ( ( m_uiFlags & a_uiFlag ) == 0 )
				{
					a_pScene->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
					m_uiFlags |= a_uiFlag;
				}

				return;
			}
		}
		else
		{
			*a_ppTarget = TNULL;
		}
	}

	if ( m_uiFlags & a_uiFlag )
	{
		a_pScene->GetVisual()->PlayAnimation( L"Normal", L"EndNormal" );
		m_uiFlags &= ~a_uiFlag;
	}
}

// Plays "Display" on the target icon (m_apIconScenes[ 0 ]) or just drops FLAGS_TARGET_DISPLAY
// $deBlob: FUNCTION 007bfd50
void ABlobNavRingComponent::SetTargetIconDisplayed( TBOOL a_bDisplayed )
{
	if ( a_bDisplayed )
	{
		if ( ( m_uiFlags & FLAGS_TARGET_DISPLAY ) == 0 )
		{
			m_apIconScenes[ 0 ]->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
			m_uiFlags |= FLAGS_TARGET_DISPLAY;
		}
	}
	else if ( m_uiFlags & FLAGS_TARGET_DISPLAY )
	{
		m_uiFlags &= ~FLAGS_TARGET_DISPLAY;
	}
}

// The same for the water icon (m_apIconScenes[ 1 ])
// $deBlob: FUNCTION 007bfda0
void ABlobNavRingComponent::SetWaterIconDisplayed( TBOOL a_bDisplayed )
{
	if ( a_bDisplayed )
	{
		if ( ( m_uiFlags & FLAGS_WATER_DISPLAY ) == 0 )
		{
			m_apIconScenes[ 1 ]->GetVisual()->PlayAnimation( L"Display", L"EndDisplay" );
			m_uiFlags |= FLAGS_WATER_DISPLAY;
		}
	}
	else if ( m_uiFlags & FLAGS_WATER_DISPLAY )
	{
		m_uiFlags &= ~FLAGS_WATER_DISPLAY;
	}
}

// $deBlob: FUNCTION 007bfa40
void ABlobNavRingComponent::PlaceIcon( TXUIScene* a_pScene, const TVector4& a_rcTarget, TFLOAT a_fScale )
{
	// On the ring around Blob, in the direction of the target
	const TVector4& rBlob  = m_pBlob->GetBodyPosition();
	TFLOAT          fBlobX = rBlob.x;
	TFLOAT          fBlobY = rBlob.y;
	TFLOAT          fBlobZ = rBlob.z;

	TVector4 vIcon( a_rcTarget.x - fBlobX, a_rcTarget.y - fBlobY, a_rcTarget.z - fBlobZ, a_rcTarget.w );
	vIcon.NormaliseXZ();
	vIcon.Multiply( m_fAlpha * s_fIconOffset );
	vIcon.x += fBlobX;
	vIcon.w = 1.0f;
	vIcon.y += fBlobY;
	vIcon.z += fBlobZ;

	// The scale of a point 50 units in front of the camera
	TFLOAT   fScaleX = a_fScale;
	TFLOAT   fScaleY = a_fScale;
	ACamera* pCamera = m_pBlob->GetViewCamera();

	if ( pCamera != TNULL )
	{
		TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( pCamera );

		if ( pViewport != TNULL )
		{
			const TMatrix44& rCamera  = pCamera->GetMatrix();
			TFLOAT           fDepth   = rCamera.m_f32 * ( vIcon.y - rCamera.m_f42 ) + rCamera.m_f31 * ( vIcon.x - rCamera.m_f41 ) + rCamera.m_f33 * ( vIcon.z - rCamera.m_f43 );
			TFLOAT           fAspect  = pViewport->GetHeight() / pViewport->GetWidth();
			TFLOAT           fHalfFOV = pCamera->GetCameraMatrix().m_fFOV * 0.5f;
			TFLOAT           fTanX    = TFLOAT( tan( double( fHalfFOV ) ) );
			TFLOAT           fTanY    = TFLOAT( tan( double( fHalfFOV * fAspect ) ) );

			fScaleX = ( ( fTanX * 50.0f ) / ( fTanX * fDepth ) ) * a_fScale;
			fScaleY = ( ( fTanY * 50.0f ) / ( fTanY * fDepth ) ) * a_fScale;
		}
	}

	ARenderer::GetSingleton()->ProjectToCanvas( vIcon, TNULL );

	T2GUITransform& rTransform = a_pScene->GetTransform();
	rTransform.SetIdentity();
	rTransform.PreMultiply( T2GUITransform( fScaleX, fScaleY ) );

	TFLOAT fHalfScaleX = fScaleX * 0.5f;
	TFLOAT fX          = vIcon.x - a_pScene->GetWidth() * fHalfScaleX;
	TFLOAT fHeight     = a_pScene->GetHeight();
	a_pScene->SetPosition( fX, vIcon.y - fHeight * fScaleY * 0.5f );
}
