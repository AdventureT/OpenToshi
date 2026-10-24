#include "pch.h"
#include "Entities/APickupManager.h"
#include "AChallenge.h"
#include "AChallengeManager.h"
#include "ARevolutionary/ARevolutionary.h"
#include "Entities/ABaseEntity.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityIOPacket.h"
#include "Entities/APropTriggerHelp.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "Collision/ACollisionInstance2.h"
#include "Cameras/ACameraManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Sound/ASoundInterface.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <XUI/TXUI.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Sends a message without data to the challenge's linked entities
static void AChallenge_SendToEntities( AChallenge::Data* a_pData, AEntityIOPacket::MESSAGE a_eMessage )
{
	for ( TINT i = 0; i < a_pData->m_uiNumEntities; i++ )
	{
		ABaseEntity* pEntity = a_pData->m_ppEntities[ i ];

		if ( pEntity != TNULL )
		{
			// NOTE: retail leaves the packet's data uninitialised (only the message is set)
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = a_eMessage;
			oPacket.m_iData    = 0;
			pEntity->HandleEntityIO( TNULL, oPacket );
		}
	}
}

// $deBlob: FUNCTION 005982a0
void AChallenge::CollisionHandler::OnTriggerEnter( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther )
{
	TUINT32 uiLayers = ( a_pOther != TNULL ) ? a_pOther->m_uiCollisionFilterInfo & 0xFFF : 0;

	if ( !( uiLayers & 4 ) || a_pOther == TNULL || a_pOther->m_pUserData == TNULL )
		return;

	TASSERT( TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pOther->m_pUserData )->m_pOwner )->IsA( &TGetClass( ABlobEntity ) ) );

	AChallenge*        pChallenge = m_pChallenge;
	AChallengeManager* pManager   = AChallengeManager::GetSingleton();

	if ( !pChallenge->IsCompleted() && pChallenge->IsAvailable() && pChallenge != pManager->m_pCurrentChallenge )
	{
		AChallenge* pPrevious        = pManager->m_pNearbyChallenge;
		pManager->m_pNearbyChallenge = pChallenge;

		if ( ( ( pChallenge->m_pData->m_uiFlags & Data::Flags_HasCamera ) || ( a_pBody != TNULL && a_pBody->m_pUserData != TNULL ) ) && pPrevious != pChallenge )
			pManager->m_bShowChallengeScreen = TTRUE;
	}
}

// $deBlob: FUNCTION 00598380
void AChallenge::CollisionHandler::OnTriggerLeave( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther )
{
	TUINT32 uiLayers = ( a_pOther != TNULL ) ? a_pOther->m_uiCollisionFilterInfo & 0xFFF : 0;

	if ( ( uiLayers & 4 ) && AChallengeManager::GetSingleton()->m_pNearbyChallenge == m_pChallenge && !AChallengeManager::GetSingleton()->m_bPromptShown )
		AChallengeManager::GetSingleton()->m_pNearbyChallenge = TNULL;
}

// $deBlob: FUNCTION 005983c0
AChallenge::~AChallenge()
{
	m_pRevolutionary = TNULL;

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( m_pBody != TNULL )
	{
		pPhysicsManager->RemoveFromSimulation( m_pBody, TFALSE );
		pPhysicsManager->DestroyBody( m_pBody );
	}

	if ( m_ppBodies != TNULL )
	{
		for ( TINT i = 0; i < m_pData->m_uiNumBodies; i++ )
		{
			pPhysicsManager->RemoveFromSimulation( m_ppBodies[ i ], TFALSE );
			pPhysicsManager->DestroyBody( m_ppBodies[ i ] );
		}

		TFree( m_ppBodies );
	}

	while ( !m_oLinkedFrom.IsEmpty() )
	{
		LinkNode* pNode = m_oLinkedFrom.Back();
		pNode->Remove();
		delete pNode;
	}
}

// $deBlob: FUNCTION 005985a0
void AChallenge::Start()
{
	m_fTimeLeft = TFLOAT( m_pData->m_iTimeLimit );

	if ( m_fTimeLeft <= 0.0f || 10.0f <= m_fTimeLeft )
	{
		m_pRevolutionary->OnChallengeStart();
	}
	else
	{
		m_pRevolutionary->OnChallengeStartShort();
	}

	m_uiFlags &= Flags_Restarted;
	AChallengeManager::GetSingleton()->ResetEntities();

	if ( m_pData->m_uiFlags & Data::Flags_SubChallenge )
		APlayerManager::GetSingleton()->PlaySSCamera( 0, m_pData->m_iSSCamera );

	ANedCameraFunction& rNed = ACameraManager::GetSingleton()->GetNedCameras();

	for ( TINT i = m_pData->m_iNumCameraRegions; i != 0; )
	{
		i--;
		rNed.SetRegionEnabled( m_pData->m_pCameraRegions[ i ], TTRUE );
	}

	AChallenge_SendToEntities( m_pData, AEntityIOPacket::MESSAGE_CHALLENGESTART );

	// Keep what Blob looks like at the start for a restart
	ABlobEntity*       pBlob    = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
	AChallengeManager* pManager = AChallengeManager::GetSingleton();

	TUINT32 uiColour = pBlob->m_pColourComponent->GetCurrentColour();

	if ( uiColour & 0x10 )
		uiColour = 0x10;

	pManager->m_oRestartInfo.m_vBlobPosition = pBlob->GetMatrix().GetTranslation();
	pManager->m_oRestartInfo.m_pChallenge    = this;
	pManager->m_oRestartInfo.m_uiBlobColour  = uiColour;
	pManager->m_oRestartInfo.m_iBlobPoints   = pBlob->m_iPaintPoints;
}

// $deBlob: FUNCTION 005986e0
void AChallenge::Complete()
{
	m_uiFlags |= Flags_Completed;
	m_pRevolutionary->OnChallengeComplete();

	if ( m_eType != Type_Null && !( m_pData->m_uiFlags & Data::Flags_SubChallenge ) )
	{
		TUINT32 uiFlags = m_pData->m_uiFlags;
		TINT    iScore  = 0;

		if ( uiFlags & Data::Flags_DifficultyEasy )
			iScore = TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x68 / 4 ];
		else if ( uiFlags & Data::Flags_DifficultyMedium )
			iScore = TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x6C / 4 ];
		else if ( uiFlags & Data::Flags_DifficultyHard )
			iScore = TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x70 / 4 ];

		if ( !( m_uiFlags & Flags_Restarted ) )
			iScore = TINT( TFLOAT( iScore ) * 1.25f );

		AGameStatus::GetSingleton()->GetLevelData().AddScore( 0, iScore, -1 );
	}

	OnEnd();

	ANedCameraFunction& rNed = ACameraManager::GetSingleton()->GetNedCameras();
	TINT                i    = m_pData->m_iNumCameraRegions;

	if ( m_eType == Type_Null )
	{
		while ( i != 0 )
		{
			i--;

			// Kept enabled if one of its fixed cameras isn't a 0x200 one
			ANedCameraFunction::Region* pRegion  = rNed.FindRegion( m_pData->m_pCameraRegions[ i ] );
			TBOOL                       bEnabled = TFALSE;

			for ( TINT j = 0; j < pRegion->m_iNumFixedCams; j++ )
			{
				if ( !( pRegion->m_pFixedCams[ j ].m_iFLAGS & 0x200 ) )
				{
					bEnabled = TTRUE;
					break;
				}
			}

			rNed.SetRegionEnabled( m_pData->m_pCameraRegions[ i ], bEnabled );
		}
	}
	else
	{
		while ( i != 0 )
		{
			i--;
			rNed.SetRegionEnabled( m_pData->m_pCameraRegions[ i ], TFALSE );
		}
	}

	AChallenge_SendToEntities( m_pData, AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE );

	if ( m_eType != Type_Null && !( m_pData->m_uiFlags & Data::Flags_SubChallenge ) && AGameStatus::GetSingleton()->GetLevelProperties()->m_sprintLevel == 0 )
		APickupManager::SpawnPickupNearPlayer( 2, 0 );
}

// $deBlob: FUNCTION 00598890
void AChallenge::Fail()
{
	m_pRevolutionary->OnChallengeFail();
	Reset( TTRUE );
	m_uiFlags |= 0x16;
	OnEnd();
}

// $deBlob: FUNCTION 005988c0
void AChallenge::Quit()
{
	Reset( TTRUE );
	m_uiFlags |= 0x14;
	OnEnd();
}

// $deBlob: FUNCTION 005988e0
void AChallenge::ShowStartPrompt()
{
	m_pRevolutionary->ShowStartPrompt();
}

// $deBlob: FUNCTION 005988f0
void AChallenge::HideStartPrompt()
{
	m_pRevolutionary->HideStartPrompt();
}

// $deBlob: FUNCTION 00598910
void AChallenge::SendSkipToEntities()
{
	AChallenge_SendToEntities( m_pData, AEntityIOPacket::MESSAGE_SKIP );
}

// $deBlob: FUNCTION 00598960
void AChallenge::Update( TFLOAT a_fDeltaTime )
{
	AGameStatus*            pGameStatus = AGameStatus::GetSingleton();
	AGameStatus::LevelData& rLevelData  = pGameStatus->GetLevelData();

	if ( rLevelData.m_bUnk00C || rLevelData.m_fTimeLeft <= 0.0f )
		return;

	if ( APlayerManager::GetSingleton()->GetPlayerBlob( 0 )->IsFlagged( ABaseEntity::FLAG_Dead ) )
		return;

	if ( !( m_uiFlags & Flags_Failed ) && m_pData->m_iTimeLimit != -1 && !( m_pData->m_uiFlags & Data::Flags_SubChallenge ) )
	{
		TXUI* pXUI = TXUI::GetSingleton();

		if ( APlayerManager::GetSingleton()->GetCameraMode( 0 ) != 4 && pXUI->GetActiveScene() != TNULL && pXUI->GetActiveScene()->GetClass() == &TGetClass( ABlobHud ) &&
		     pXUI->GetTransToScene() == TNULL && pXUI->GetTransFromScene() == TNULL && !pXUI->IsInputLocked() )
		{
			TINT iBefore = TMath::FloorToInt( m_fTimeLeft );
			m_fTimeLeft -= a_fDeltaTime;
			TINT iAfter = TMath::FloorToInt( m_fTimeLeft );

			// The last 5 seconds tick
			if ( pGameStatus->GetLevelProperties()->m_sprintLevel == 0 && TUINT( iBefore - 1 ) < 5 && iBefore != iAfter )
				ASoundInterface::PlayEvent( ASoundMap::GROUPID_COMMON, ASoundMap::EVENTID( 0x2D ), FMOD_EVENT_DEFAULT, TTRUE );
		}

		if ( m_fTimeLeft <= 0.0f )
			AChallengeManager::GetSingleton()->OnChallengeTimeOut( this );
	}

	m_fHintTimer += a_fDeltaTime;

	if ( 30.0f <= m_fHintTimer && pGameStatus->GetLevelProperties()->m_sprintLevel == 0 )
	{
		TINT iStringID = GetHintStringID();

		if ( iStringID != -1 )
			APropTriggerHelp::ShowHelp( TNULL, APropTriggerHelp::HELPTYPE_Hud, iStringID, -1, TTRUE, 5.0f, 0, TNULL, TFALSE );

		m_fHintTimer = 0.0f;
	}
}

// $deBlob: FUNCTION 00598af0
TBOOL AChallenge::Create( Data* a_pData )
{
	m_pData = a_pData;

	TASSERT( m_pData->m_iGUID != -1 );
	TASSERT( TNULL == ABaseEntity::FindIDEntity( a_pData->m_iGUID ) );

	// Unit scale
	TVector4 vScale;
	m_pData->m_pMatrix->GetScale( vScale );

	TVector4 vInvScale;
	vInvScale.x = ( vScale.x <= 1e-05f ) ? 1.0f : 1.0f / vScale.x;
	vInvScale.y = ( vScale.y <= 1e-05f ) ? 1.0f : 1.0f / vScale.y;
	vInvScale.z = ( vScale.z <= 1e-05f ) ? 1.0f : 1.0f / vScale.z;
	m_pData->m_pMatrix->Scale( vInvScale );

	// The trigger sphere
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	m_pBody = pPhysicsManager->CreateFixedBody( m_pData->m_pMatrix, TNULL );
	TPhysicsManager::SetSphere( m_pBody, 10.0f );
	pPhysicsManager->SetListener( m_pBody, &m_oCollisionHandler );
	pPhysicsManager->SetCollisionLayer( m_pBody, 0x200 );
	pPhysicsManager->SetCollisionGroup( m_pBody, 4 );
	TPhysicsManager::UpdateBodyTransform( m_pBody, TFALSE );
	pPhysicsManager->AddToSimulation( m_pBody, TFALSE );

	// The extra trigger boxes
	if ( m_pData->m_uiNumBodies != 0 )
	{
		m_ppBodies = TSTATICCAST( TPhysicsObjectContainer*, TMemalign( 16, m_pData->m_uiNumBodies * sizeof( TPhysicsObjectContainer* ) ) );

		for ( TINT i = 0; i < m_pData->m_uiNumBodies; i++ )
		{
			Data::LinkedBody& rBody = m_pData->m_pBodies[ i ];

			// The box sits on its matrix
			TVector4 vHalfExtents = *rBody.m_pOffset;
			rBody.m_pMatrix->AsBasisVector4( BASISVECTOR_TRANSLATION ).y -= vHalfExtents.y;

			m_ppBodies[ i ] = pPhysicsManager->CreateFixedBody( rBody.m_pMatrix, TNULL );

			if ( m_ppBodies[ i ] != TNULL )
				m_ppBodies[ i ]->m_pUserData = &rBody;

			TPhysicsManager::SetBox( m_ppBodies[ i ], vHalfExtents );
			pPhysicsManager->SetListener( m_ppBodies[ i ], &m_oCollisionHandler );
			pPhysicsManager->SetCollisionLayer( m_ppBodies[ i ], 0x200 );
			pPhysicsManager->SetCollisionGroup( m_ppBodies[ i ], 4 );
			TPhysicsManager::UpdateBodyTransform( m_ppBodies[ i ], TFALSE );
			pPhysicsManager->AddToSimulation( m_ppBodies[ i ], TFALSE );
		}
	}

	m_oMatrix       = *m_pData->m_pMatrix;
	TMatrix44 mCopy = m_oMatrix;
	m_oMatrix.Multiply( mCopy, TMatrix44::IDENTITY );

	// The marker
	const TCHAR* pszMarker;

	switch ( m_eType )
	{
		case Type_Paint:
		case Type_Civilian:
		case Type_Car:
			pszMarker = "ARevolutionary";
			break;
		case Type_Landmark:
			pszMarker = "ARevolutionaryLandmark";
			break;
		case Type_Combat:
			pszMarker = "ARevolutionaryCombat";
			break;
		case Type_Race:
			pszMarker = "ARevolutionaryRace";
			break;
		case Type_Null:
			// NOTE: ANullChallenge's data (0x38) says which marker it shows
			switch ( *TREINTERPRETCAST( TINT*, m_pData + 1 ) )
			{
				case 0:
					pszMarker = "ARevolutionaryRace";
					break;
				case 1:
					pszMarker = "ARevolutionary";
					break;
				case 2:
					pszMarker = "ARevolutionaryLandmark";
					break;
				case 3:
					pszMarker = "ARevolutionaryCombat";
					break;
				default:
					// NOTE: retail passes a_pData itself as the name
					pszMarker = TREINTERPRETCAST( const TCHAR*, a_pData );
					break;
			}
			break;
		default:
			TASSERT( m_eType == Type_Uber || m_eType == Type_ZJump );
			pszMarker = "ARevolutionary";
			break;
	}

	ARevolutionary* pRevolutionary = TSTATICCAST( ARevolutionary, AEntityFactory::GetSingleton()->CreateEntity( pszMarker ) );
	pRevolutionary->SetPosition( m_pData->m_pMatrix->GetTranslation() );
	pRevolutionary->SetID( m_pData->m_iGUID );
	m_pRevolutionary = pRevolutionary;
	pRevolutionary->SetChallenge( this );

	// The linked entities
	for ( TINT i = 0; i < a_pData->m_uiNumEntities; i++ )
	{
		TINT iID = a_pData->m_piEntityIDs[ i ];
		TASSERT( iID != -1 );

		ABaseEntity* pEntity       = ABaseEntity::FindIDEntity( iID );
		a_pData->m_ppEntities[ i ] = pEntity;

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: temporary scaffolding: entity types that aren't ported aren't created
		if ( pEntity == TNULL )
			continue;
#endif

		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_CHALLENGEINIT;
		oPacket.m_iData    = 0;
		pEntity->HandleEntityIO( TNULL, oPacket );
	}

	ANedCameraFunction& rNed = ACameraManager::GetSingleton()->GetNedCameras();

	for ( TINT i = m_pData->m_iNumCameraRegions; i != 0; )
	{
		i--;
		rNed.SetRegionEnabled( m_pData->m_pCameraRegions[ i ], TFALSE );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00599110
void AChallenge::LinkChallenges()
{
	for ( TUINT i = 0; i < m_pData->m_uiNumLinkedChallenges; i++ )
	{
		AChallenge* pLinked                = AChallengeManager::GetSingleton()->FindChallengeByGUID( m_pData->m_piLinkedChallengeGUIDs[ i ] );
		m_pData->m_ppLinkedChallenges[ i ] = pLinked;

		// NOTE: retail doesn't check pLinked
		pLinked->AddLinkedFrom( this );
	}
}

// $deBlob: FUNCTION 00599210
void AChallenge::SetAvailable( TBOOL a_bAvailable, TBOOL a_bIgnoreLinks )
{
	if ( m_pData->m_uiFlags & Data::Flags_SubChallenge )
		return;

	if ( !a_bAvailable || IsCompleted() )
	{
		m_uiFlags &= ~Flags_Available;
		m_pRevolutionary->Refresh( TFALSE );
		return;
	}

	// Every challenge linking here must be completed first
	for ( auto it = m_oLinkedFrom.Begin(); it != m_oLinkedFrom.End(); it++ )
	{
		if ( !it->GetNodeValue()->IsCompleted() )
		{
			if ( a_bIgnoreLinks != TTRUE )
				return;

			break;
		}
	}

	m_uiFlags |= Flags_Available;
	m_pRevolutionary->Refresh( TFALSE );
}

// $deBlob: FUNCTION 00599300
void AChallenge::Reset( TBOOL a_bUnk )
{
	m_uiFlags   = 0;
	m_fTimeLeft = -1.0f;

	m_pRevolutionary->Reset( TNULL );

	ANedCameraFunction& rNed = ACameraManager::GetSingleton()->GetNedCameras();

	for ( TINT i = m_pData->m_iNumCameraRegions; i != 0; )
	{
		i--;
		rNed.SetRegionEnabled( m_pData->m_pCameraRegions[ i ], TFALSE );
	}

	AChallenge_SendToEntities( m_pData, AEntityIOPacket::MESSAGE_CHALLENGERESET );
}

// $deBlob: FUNCTION 005993a0
TBOOL AChallenge::IsLinkedTo( TINT a_iGUID ) const
{
	for ( TUINT i = 0; i < m_pData->m_uiNumLinkedChallenges; i++ )
	{
		if ( m_pData->m_piLinkedChallengeGUIDs[ i ] == a_iGUID )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005993e0
void AChallenge::AddLinkedFrom( AChallenge* a_pChallenge )
{
	for ( auto it = m_oLinkedFrom.Begin(); it != m_oLinkedFrom.End(); it++ )
	{
		if ( it->GetNodeValue() == a_pChallenge )
			return;
	}

	m_oLinkedFrom.PushBack( new LinkNode( a_pChallenge ) );
}

// $deBlob: FUNCTION 005994a0
void AChallenge::AddTime( TFLOAT a_fDeltaTime )
{
	AGameStatus* pGameStatus = AGameStatus::GetSingleton();

	if ( pGameStatus->GetLevelProperties()->m_sprintLevel != 0 )
		pGameStatus->GetLevelData().m_fTimeLeft += a_fDeltaTime;

	if ( m_pData->m_iTimeLimit != -1 )
		m_fTimeLeft += a_fDeltaTime;
}
