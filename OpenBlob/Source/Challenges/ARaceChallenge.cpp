#include "pch.h"
#include "ARaceChallenge.h"
#include "AChallengeManager.h"
#include "Collision/ACollisionInstance2.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Entities/ABlobEntity/ABlobColourComponent.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "ParticleSystem/AEnvParticles.h"
#include "Sound/ASoundInterface.h"
#include "XUI/InGame/HUD/ABlobChallengeStatusHud.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <T2Locale/T2Locale.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The first link of a_pNode to follow (Link::FLAG_Toggled), -1 if there's none
static TINT ARaceChallenge_FindNextLink( AWaypointData::Node* a_pNode )
{
	for ( TINT i = 0; i < a_pNode->uiLinkCount; i++ )
	{
		if ( a_pNode->GetLink( i ).uiFlags & AWaypointData::Link::FLAG_Toggled )
			return i;
	}

	return -1;
}

// Is a_pNode the last node of the race (no link to follow)?
static TBOOL ARaceChallenge_IsLastNode( AWaypointData::Node* a_pNode )
{
	return a_pNode->uiLinkCount == 0 || ( a_pNode->uiLinkCount == 1 && !( a_pNode->pLinks[ 0 ].uiFlags & AWaypointData::Link::FLAG_Toggled ) );
}

// $deBlob: FUNCTION 0059f0e0
ARaceChallenge::ARaceChallenge()
    : AChallenge( Type_Race )
{
	m_oRaceHandler.m_pChallenge = TNULL;
	m_pWPEffectMain             = TNULL;
	m_pWPEffectRest             = TNULL;
	m_pWPEffectFinal            = TNULL;
	m_apWPEmitters[ 0 ]         = TNULL;
	m_apWPEmitters[ 1 ]         = TNULL;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves m_pWPBody uninitialised (Create makes it only for a race with waypoints)
	m_pWPBody = TNULL;
#endif
}

// $deBlob: FUNCTION 0059f190
ARaceChallenge::~ARaceChallenge()
{
#ifdef DEBLOB_COMMUNITY_PATCH
	if ( m_pWPBody != TNULL )
#endif
	{
		TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pWPBody, TFALSE );
		TPhysicsManager::GetSingleton()->DestroyBody( m_pWPBody );
	}

	ShowWaypoints( TFALSE );

	m_pWPEffectMain  = TNULL;
	m_pWPEffectRest  = TNULL;
	m_pWPEffectFinal = TNULL;
}

// Blob touched the trigger of the current node
// $deBlob: FUNCTION 0059f020
void ARaceChallenge::CollisionHandler::OnTriggerEnter( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther )
{
	TUINT32 uiLayers = ( a_pOther != TNULL ) ? a_pOther->m_uiCollisionFilterInfo & 0xFFF : 0;

	if ( !( uiLayers & 4 ) || a_pOther == TNULL || a_pOther->m_pUserData == TNULL )
		return;

	TASSERT( TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pOther->m_pUserData )->m_pOwner )->IsA( &TGetClass( ABlobEntity ) ) );

	if ( a_pBody != TNULL && a_pBody->m_pUserData != TNULL && m_pChallenge->UpdateWaypoints( TSTATICCAST( AWaypointData::Node, a_pBody->m_pUserData ) ) )
		TPhysicsManager::GetSingleton()->RemoveFromSimulation( a_pBody, TFALSE );
}

// $deBlob: FUNCTION 0059f240
TBOOL ARaceChallenge::Create( TypeData* a_pData )
{
	if ( !AChallenge::Create( a_pData ) )
		return TFALSE;

	AWaypointData* pWaypoints = AWaypointData::GetSingleton();

	if ( a_pData->m_iWPSetID != -1 )
	{
		// NOTE: retail looks the HUD up without using it
		TXUI::GetSingleton()->FindResource( "hud.xur" );
		TStringManager::String16Format( m_wszRequiredSize, 8, L"%d", TSTATICCAST( TypeData, m_pData )->m_iRequiredSize );

		m_pWPSet                    = &pWaypoints->GetSet( a_pData->m_iWPSetID );
		m_oRaceHandler.m_pChallenge = this;

		// The trigger sphere of the current node
		m_oWPMatrix = TMatrix44::IDENTITY;
		m_pWPBody   = TPhysicsManager::GetSingleton()->CreateFixedBody( &m_oWPMatrix, TNULL );
		TPhysicsManager::GetSingleton()->SetListener( m_pWPBody, &m_oRaceHandler );
		TPhysicsManager::GetSingleton()->SetCollisionLayer( m_pWPBody, 0x200 );
		TPhysicsManager::GetSingleton()->SetCollisionGroup( m_pWPBody, 4 );
		TPhysicsManager::GetSingleton()->SetSphere( m_pWPBody, 6.0f );

		TParticleEffectManager* pEffects = TParticleSystem::GetSingleton()->GetEffectManager();

		m_pWPEffectMain = pEffects->FindEffect( "Blob_RaceMarker" );
		TASSERT( m_pWPEffectMain, "Couldn't find particle effect for race challenge: Blob_RaceMarker" );

		m_pWPEffectRest = pEffects->FindEffect( "Blob_RaceMarker_Next" );
		TASSERT( m_pWPEffectRest, "Couldn't find particle effect for race challenge: Blob_RaceMarker_Next" );

		m_pWPEffectFinal = pEffects->FindEffect( "Blob_RaceMarker_Last" );
		TASSERT( m_pWPEffectFinal, "Couldn't find particle effect for race challenge: Blob_RaceMarker_Last" );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0059f460
void ARaceChallenge::Update( TFLOAT a_fDeltaTime )
{
	AChallenge::Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 0059f480
void ARaceChallenge::Start()
{
	TSTATICCAST( ABlobChallengeStatusHud, AChallengeManager::GetSingleton()->GetHudElement() )->OnChallengeStart( this );

	// The trigger goes to the start node; NOTE: retail doesn't check m_pWPSet (a race without waypoints)
	AWaypointData::Set* pSet = m_pWPSet;
	m_pCurrentNode           = &pSet->GetNode( TINT16( pSet->GetStartNode() ) );

	if ( m_pWPBody != TNULL )
		m_pWPBody->m_pUserData = m_pCurrentNode;

	m_oWPMatrix = TMatrix44::IDENTITY;
	m_oWPMatrix.SetTranslation( m_pCurrentNode->vPosition );

	TPhysicsManager::GetSingleton()->UpdateBodyTransform( m_pWPBody, TFALSE );
	TPhysicsManager::GetSingleton()->AddToSimulation( m_pWPBody, TFALSE );

	ShowWaypoints( TTRUE );
	m_bLastNodeNext   = TFALSE;
	m_iNumCheckpoints = 0;

	AChallengeManager::GetSingleton()->SetCounter( 0, pSet->iNodeCount );
	AChallenge::Start();
}

// $deBlob: FUNCTION 0059f5d0
void ARaceChallenge::Reset( TBOOL a_bUnk )
{
	AChallenge::Reset( a_bUnk );

	ShowWaypoints( TFALSE );
	m_bLastNodeNext = TFALSE;
	m_pCurrentNode  = TNULL;

	TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pWPBody, TFALSE );
}

// $deBlob: FUNCTION 0059f610
TBOOL ARaceChallenge::UpdateWaypoints( AWaypointData::Node* a_pNode )
{
	m_iNumCheckpoints++;
	m_fHintTimer = 0.0f;
	AChallengeManager::GetSingleton()->SetCounter( m_iNumCheckpoints, m_pWPSet->iNodeCount );

	// The next node
	m_pCurrentNode = TNULL;
	TINT iLink     = ARaceChallenge_FindNextLink( a_pNode );

	if ( iLink >= 0 )
		m_pCurrentNode = a_pNode->GetLinkedNode( iLink, m_pWPSet );

	TypeData* pData = TSTATICCAST( TypeData, m_pData );
	ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 1 ), ASoundMap::EVENTID( 0x2B ), a_pNode->vPosition, 0 );

	if ( m_pCurrentNode == TNULL )
	{
		// The last node: won if Blob has the required colour and size
		ABlobEntity* pBlob   = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );
		TBOOL        bColour = TTRUE;

		if ( pData->m_uiRequiredColour != TUINT32( -1 ) )
		{
			TUINT32 uiColour = pBlob->GetColourComponent()->GetCurrentColour();

			if ( uiColour & 0x10 )
				uiColour = 0x10;

			bColour = ( uiColour == pData->m_uiRequiredColour );
		}

		TBOOL bSize = TTRUE;

		if ( pData->m_iRequiredSize != -1 )
			bSize = pBlob->GetSize() + 1 >= pData->m_iRequiredSize;

		if ( bColour && bSize )
		{
			AChallengeManager::GetSingleton()->OnChallengeCompleted( this, TFALSE );
			ShowWaypoints( TFALSE );

			TMatrix44 oEffect = TMatrix44::IDENTITY;
			oEffect.SetTranslation( a_pNode->vPosition );
			AChallengeManager::TriggerEffect( oEffect );
			return TTRUE;
		}
	}
	else
	{
		// The trigger moves to the next node
		m_oWPMatrix = TMatrix44::IDENTITY;
		m_oWPMatrix.SetTranslation( m_pCurrentNode->vPosition );

		if ( m_pWPBody != TNULL )
			m_pWPBody->m_pUserData = m_pCurrentNode;

		TPhysicsManager::GetSingleton()->UpdateBodyTransform( m_pWPBody, TFALSE );
		ShowWaypoints( TTRUE );

		TMatrix44 oEffect = TMatrix44::IDENTITY;
		oEffect.SetTranslation( a_pNode->vPosition );
		AChallengeManager::TriggerEffect( oEffect );

		// Is the node after the next one the last?
		for ( TINT i = 0; i < a_pNode->uiLinkCount; i++ )
		{
			if ( a_pNode->GetLink( i ).uiFlags & AWaypointData::Link::FLAG_Toggled )
			{
				AWaypointData::Node* pNext = a_pNode->GetLinkedNode( i, m_pWPSet );

				if ( pNext->uiLinkCount == 1 )
				{
					m_bLastNodeNext = TTRUE;
					m_vLastNode     = pNext->vPosition;
				}

				if ( m_bLastNodeNext )
					break;
			}
		}

		// The time bonus of the checkpoint
		if ( pData->m_iTimeLimit != -1 && pData->m_iTimeBonus > 0 )
		{
			AddTime( TFLOAT( pData->m_iTimeBonus ) );

			TWCHAR* wszText = TStringManager::GetTempString16();
			TStringManager::String16Format( wszText, T2Locale::GetSingleton()->GetString( 0x1B ), pData->m_iTimeBonus );

			TVector4 vPosition = a_pNode->vPosition;
			vPosition.y -= 2.0f;

			AGameStatus::GetSingleton()->GetLevelData().ShowMessage( wszText, 3, &vPosition, 0 );
			AGameStatus::GetSingleton()->GetLevelData().ShowMessage( wszText, 4, TNULL, 0 );
		}
	}

	return TFALSE;
}

// The share of the checkpoints reached (percent)
// $deBlob: FUNCTION 0059f9b0
TINT ARaceChallenge::GetScore()
{
	return TINT( ( TFLOAT( m_iNumCheckpoints ) / TFLOAT( m_pWPSet->iNodeCount ) ) * 100.0f );
}

// $deBlob: FUNCTION 0059f9e0
void ARaceChallenge::OnEnd()
{
	ShowWaypoints( TFALSE );
	TPhysicsManager::GetSingleton()->RemoveFromSimulation( m_pWPBody, TFALSE );
}

// $deBlob: FUNCTION 0059fa00
void ARaceChallenge::ShowWaypoints( TBOOL a_bShow )
{
	AEnvParticles* pEnvParticles = AEnvParticles::GetSingleton();

	if ( pEnvParticles == TNULL || m_pWPEffectMain == TNULL || m_pWPEffectRest == TNULL || m_pWPEffectFinal == TNULL )
		return;

	for ( TINT i = 0; i < 2; i++ )
	{
		if ( m_apWPEmitters[ i ] != TNULL )
		{
			pEnvParticles->RemoveEmitter( m_apWPEmitters[ i ], TTRUE );
			m_apWPEmitters[ i ] = TNULL;
		}
	}

	if ( !a_bShow )
		return;

	// The marker of the current node (the final one at the last node)
	m_apWPEmitters[ 0 ] = pEnvParticles->AddEmitter( ARaceChallenge_IsLastNode( m_pCurrentNode ) ? m_pWPEffectFinal : m_pWPEffectMain, m_oWPMatrix, 0.0f, 1 );

	if ( m_pWPSet == TNULL || m_pCurrentNode == TNULL )
		return;

	// The marker of the next node
	AWaypointData::Node* pNode = m_pCurrentNode;

	for ( TINT i = 1; i < 2; i++ )
	{
		TINT iLink = ARaceChallenge_FindNextLink( pNode );

		if ( iLink < 0 )
			return;

		pNode = pNode->GetLinkedNode( iLink, m_pWPSet );

		if ( pNode == TNULL )
			return;

		TMatrix44 oMatrix = TMatrix44::IDENTITY;
		oMatrix.SetTranslation( pNode->vPosition );

		m_apWPEmitters[ i ] = pEnvParticles->AddEmitter( ARaceChallenge_IsLastNode( pNode ) ? m_pWPEffectFinal : m_pWPEffectRest, oMatrix, 0.0f, 1 );
	}
}

// $deBlob: FUNCTION 0059fd90
TINT ARaceChallenge::GetHintStringID()
{
	return 0xA1;
}
