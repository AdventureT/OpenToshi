#include "pch.h"
#include "APropTrigger.h"
#include "AWaypointData.h"
#include "Challenges/AChallengeManager.h"
#include "AEntityLoader.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "APropTriggerLevelComplete.h"
#include "Entities/ABlobEntity/ABlobEntity.h"
#include "Terrain2/TerrainTypes.h"
#include "GameInterface/APlayerManager.h"
#include "Input/AVibrationManager.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Physics/TPhysicsMotionBase.h>

#include <stdlib.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AInvisibleEntity ) == 0x84 );
TSTATICASSERT( sizeof( APropTrigger ) == 0xC0 );

// $deBlob: FUNCTION 0079a920
// $deBlob: FUNCTION 0079a930
// $deBlob: FUNCTION 0079a9a0
// $deBlob: FUNCTION 0079aa00
// $deBlob: FUNCTION 0079aa10
// $deBlob: FUNCTION 00540d60
// $deBlob: FUNCTION 00859de0
TDEFINE_CLASS( APropTrigger );

// $deBlob: FUNCTION 0079ab70
APropTrigger::APropTrigger()
{
	m_pPhysicsData       = TNULL;
	m_fDelay             = 0.0f;
	m_fTimer             = 0.0f;
	m_uiNumWaypointSets  = 0;
	m_uiMessage          = AEntityIOPacket::MESSAGE_TRIGGER;
	m_iPaintPoints       = -1;
	m_uiTriggeredBy      = 0;
	m_pWaypointSets      = TNULL;
	m_fPulseSpeed        = 0.0f;
	m_pTargetsByDistance = TNULL;
	m_iTriggerData       = 0;
	m_uiParentTrigger    = 0;
	m_uiTriggerFlags     = 0;
	m_uiNextPulseTarget  = 0;
	m_iLife              = -1;
	SetHash( TFALSE );
}

// $deBlob: FUNCTION 0079ac60
// $deBlob: FUNCTION 0079bec0
// $deBlob: FUNCTION 0079bea7
APropTrigger::~APropTrigger()
{
	::operator delete( m_pWaypointSets );
	::operator delete( m_pTargetsByDistance );
	DestroyPhysics();
}

// The touch phantom goes into the simulation
// $deBlob: FUNCTION 0079acf0
void APropTrigger::AddToPhysics()
{
	if ( m_pPhysicsData != TNULL )
	{
		// NOTE: retail adds the body even when it's TNULL
		TPhysicsObjectContainer* pBody = m_pPhysicsData->m_pBody;

		if ( pBody == TNULL || !pBody->IsInSimulation() )
			TPhysicsManager::GetSingleton()->AddToSimulation( pBody, TFALSE );
	}
}

// The touch phantom leaves the simulation
// $deBlob: FUNCTION 0079ad30
void APropTrigger::RemoveFromPhysics()
{
	if ( m_pPhysicsData != TNULL )
	{
		TPhysicsObjectContainer* pBody = m_pPhysicsData->m_pBody;

		if ( pBody != TNULL && pBody->IsInSimulation() )
			TPhysicsManager::GetSingleton()->RemoveFromSimulation( pBody, TFALSE );
	}
}

// $deBlob: FUNCTION 0079ad70
// $deBlob: FUNCTION 0079bf9d
void APropTrigger::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );
	ATriggerUtil::ProcessParams( a_rParams );

	const AEntityLoader::EntityData& rEntityData = static_cast<const AEntityLoader::EntityData&>( a_rParams );

	TBOOL bTriggerOnce       = TFALSE;
	TBOOL bResetOnExit       = TFALSE;
	TBOOL bTriggerOnTouch    = TTRUE;
	TBOOL bTriggerFirstFrame = TFALSE;
	TBOOL bQuitChallenge     = TFALSE;
	a_rParams.GetValue( bTriggerOnce, "TriggerOnce" );
	a_rParams.GetValue( bResetOnExit, "ResetOnExit" );
	a_rParams.GetValue( bTriggerOnTouch, "TriggerOnEntityTouch" );
	a_rParams.GetValue( bTriggerFirstFrame, "TriggerFirstFrame" );
	a_rParams.GetValue( bQuitChallenge, "QuitChallenge" );

	if ( bTriggerFirstFrame )
	{
		bTriggerOnTouch = TFALSE;
		bTriggerOnce    = TTRUE;
		bResetOnExit    = TFALSE;
	}

	m_uiTriggerFlags = 0;
	if ( bTriggerOnce ) m_uiTriggerFlags = TRIGGERFLAG_TriggerOnce;
	if ( bResetOnExit ) m_uiTriggerFlags |= TRIGGERFLAG_ResetOnExit;
	if ( bTriggerFirstFrame ) m_uiTriggerFlags |= TRIGGERFLAG_TriggerFirstFrame;
	if ( bQuitChallenge ) m_uiTriggerFlags |= TRIGGERFLAG_QuitChallenge;

	a_rParams.GetValue( m_fDelay, "Delay" );

	TFLOAT fPulseSpeed;
	if ( a_rParams.GetValue( fPulseSpeed, "Pulse Speed" ) )
	{
		m_uiTriggerFlags |= TRIGGERFLAG_Pulse;
		m_fPulseSpeed        = fPulseSpeed;
		m_pTargetsByDistance = TSTATICCAST( TargetDistance, TMemalign( 16, ( m_uiNumEntityTargets + m_uiNumInstances ) * sizeof( TargetDistance ) ) );
	}

	if ( a_rParams.GetValue( m_iPaintPoints, "Paint Points Required" ) )
	{
		m_iLife = m_iPaintPoints;
		TASSERT( !bTriggerOnTouch );

		TBOOL bTargetableFromBelow = TFALSE;
		a_rParams.GetBool( bTargetableFromBelow, "Targetable From Below" );
		if ( bTargetableFromBelow ) m_uiTriggerFlags |= TRIGGERFLAG_TargetableFromBelow;

		TBOOL bUseSlamAttack = TFALSE;
		a_rParams.GetBool( bUseSlamAttack, "Use Slam Attack" );
		if ( bUseSlamAttack ) m_uiTriggerFlags |= TRIGGERFLAG_UseSlamAttack;

		SetHash( TTRUE );
		TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType )
		[1] = 17;
	}

	m_uiNumWaypointSets = TUINT8( rEntityData.GetWaypointSetTargetCount() );
	if ( m_uiNumWaypointSets != 0 )
	{
		m_pWaypointSets = TSTATICCAST( WaypointSet, TMemalign( 16, m_uiNumWaypointSets * sizeof( WaypointSet ) ) );

		for ( TINT i = 0; i < rEntityData.GetWaypointSetTargetCount(); i++ )
		{
			m_pWaypointSets[ i ].m_uiSet    = rEntityData.GetWaypointSetTarget( i, 0 );
			m_pWaypointSets[ i ].m_uiNode   = rEntityData.GetWaypointSetTarget( i, 1 );
			m_pWaypointSets[ i ].m_bToggled = 0;
		}
	}

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail leaves it uninitialised (read below when there's no "MessageID")
	TINT iMessageID = -1;
#else
	TINT iMessageID;
#endif
	if ( a_rParams.GetValue( iMessageID, "MessageID" ) == TTRUE )
		SetMessageID( iMessageID );

	TINT iChallengeResponse;
	if ( a_rParams.GetValue( iChallengeResponse, "ChallengeResponse" ) == TTRUE )
	{
		if ( iChallengeResponse == 0 )
			m_uiTriggerFlags |= TRIGGERFLAG_OnChallengeComplete;
		else if ( iChallengeResponse == 1 )
			m_uiTriggerFlags |= TRIGGERFLAG_OnChallengeStart;
		else if ( iChallengeResponse == 2 )
			m_uiTriggerFlags |= TRIGGERFLAG_OnChallengeReset;
	}

	TBOOL bPlayParticleEffect;
	if ( iMessageID == 3 && a_rParams.GetBool( bPlayParticleEffect, "PlayParticleEffect" ) )
		m_iTriggerData = bPlayParticleEffect;

	if ( bTriggerOnTouch )
		CreatePhysics( rEntityData.m_pMatrix, rEntityData.m_pBBox, TTRUE );
}

// The pulse's sort order: nearest first
// $deBlob: FUNCTION 0079be30
static int __cdecl CompareTargetDistance( const void* a_pA, const void* a_pB )
{
	TFLOAT fDiff = TSTATICCAST( const APropTrigger::TargetDistance, a_pA )->m_fDistSq - TSTATICCAST( const APropTrigger::TargetDistance, a_pB )->m_fDistSq;

	if ( fDiff < 0.0f )
		return -1;

	return 0.0f < fDiff;
}

// $deBlob: FUNCTION 0079b150
// $deBlob: FUNCTION 0079bf92
void APropTrigger::PostCreate()
{
	ATriggerUtil::PostCreate();

	if ( m_uiTriggerFlags & TRIGGERFLAG_Pulse )
	{
		TINT     i           = 0;
		TVector4 vPosition   = m_oMatrix.GetTranslation();
		TINT     iNumTargets = m_uiNumInstances + m_uiNumEntityTargets;

		for ( ; i < m_uiNumEntityTargets; i++ )
		{
			m_pTargetsByDistance[ i ].m_iIndex  = i;
			m_pTargetsByDistance[ i ].m_fDistSq = TVector4::DistanceSq( GetEntityTarget( i )->GetMatrix().GetTranslation(), vPosition );
		}

		for ( ; i < iNumTargets; i++ )
		{
			m_pTargetsByDistance[ i ].m_iIndex    = i;
			TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, GetInstanceTarget( i - m_uiNumEntityTargets ) );
			m_pTargetsByDistance[ i ].m_fDistSq   = TVector4::DistanceSq( **pInstance->m_ppCentre, vPosition );
		}

		qsort( m_pTargetsByDistance, iNumTargets, sizeof( TargetDistance ), CompareTargetDistance );
	}

	for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
	{
		ABaseEntity* pTarget = GetEntityTarget( i );

		if ( pTarget != TNULL && pTarget->IsA( &TGetClass( APropTrigger ) ) )
		{
			TSTATICCAST( APropTrigger, pTarget )->m_uiParentTrigger = TREINTERPRETCAST( TUINT32, this );

			if ( pTarget->IsA( &TGetClass( APropTriggerLevelComplete ) ) )
				m_uiTriggerFlags |= TRIGGERFLAG_TargetsLevelComplete;
		}
	}
}

// $deBlob: FUNCTION 0079b2c0
void APropTrigger::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity::Tick( a_fDeltaTime );

	if ( TUINT8( m_Flags >> 24 ) & 1 )
		return;

	if ( ( m_uiTriggerFlags & TRIGGERFLAG_TriggerFirstFrame ) && !( m_uiTriggerFlags & TRIGGERFLAG_Triggered ) )
	{
		// "TriggerFirstFrame"
		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TRIGGER;
		oPacket.m_iData    = 0;
		HandleEntityIO( TNULL, oPacket );
	}

	TUINT16 uiMask;

	if ( m_uiTriggerFlags & TRIGGERFLAG_Delayed )
	{
		// The delay
		m_fTimer -= a_fDeltaTime;

		if ( 0.0f <= m_fTimer )
			return;

		Trigger();
		uiMask = TUINT16( ~TRIGGERFLAG_Delayed );
	}
	else
	{
		if ( !( m_uiTriggerFlags & TRIGGERFLAG_Pulsing ) )
			return;

		m_fTimer += a_fDeltaTime;

		if ( !( m_uiTriggerFlags & TRIGGERFLAG_Pulse ) )
			return;

		// The pulse: triggers the targets as its radius grows past them
		AEntityIOPacket oPacket;
		TINT            iData = GetTriggerData();
		oPacket.m_eMessage    = GetMessageToSend();
		oPacket.m_iData       = iData;

		TFLOAT  fRadiusSq    = m_fPulseSpeed * m_fTimer;
		TUINT16 uiNumTargets = m_uiNumInstances + m_uiNumEntityTargets;
		fRadiusSq *= fRadiusSq;

		while ( m_uiNextPulseTarget < uiNumTargets )
		{
			TargetDistance& rTarget = m_pTargetsByDistance[ m_uiNextPulseTarget ];

			if ( fRadiusSq <= rTarget.m_fDistSq )
				break;

			if ( rTarget.m_iIndex < m_uiNumEntityTargets )
			{
				ABaseEntity* pTarget = GetEntityTarget( rTarget.m_iIndex );

				if ( !( TUINT8( pTarget->GetFlags() >> 24 ) & 1 ) )
					pTarget->HandleEntityIO( this, oPacket );
			}
			else
			{
				TriggerInstanceTarget( rTarget.m_iIndex - m_uiNumEntityTargets, TNULL );
			}

			m_uiNextPulseTarget++;
		}

		if ( m_uiNextPulseTarget < uiNumTargets )
			return;

		uiMask = TUINT16( ~TRIGGERFLAG_Pulsing );
	}

	m_uiTriggerFlags &= uiMask;
	SetTicking( TFALSE );
}

// $deBlob: FUNCTION 0079b460
void APropTrigger::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	AInvisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );

	if ( TUINT8( m_Flags >> 24 ) & 1 )
		return;

	TBOOL bPainted = TFALSE;

	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_SLAM && m_iLife >= 0 && ( TUINT8( a_pCaller->GetFlags() >> 4 ) & 1 ) )
	{
		TASSERT( a_pCaller->IsA( &TGetClass( ABlobEntity ) ) );
		ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );
		TINT         iRest = pBlob->RemovePaintPoints( m_iLife, TFALSE );
		m_iLife            = TINT16( iRest );

		if ( iRest <= 0 )
		{
			bPainted = TTRUE;

			if ( a_rPacket.m_iData == 0 )
				AVibrationManager::VibrateOnce( APlayerManager::GetSingleton()->GetPlayerIndex( pBlob ), 2 );
		}
	}

	TINT eMessage = a_rPacket.m_eMessage;

	if ( eMessage == AEntityIOPacket::MESSAGE_TOUCH )
		m_uiTriggerFlags |= TRIGGERFLAG_Touched;

	if ( bPainted || eMessage == AEntityIOPacket::MESSAGE_TOUCH || eMessage == AEntityIOPacket::MESSAGE_TRIGGER || eMessage == AEntityIOPacket::MESSAGE_TOGGLE ||
	     ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE && ( TUINT8( m_uiTriggerFlags >> 13 ) & 1 ) ) ||
	     ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGESTART && ( TUINT8( m_uiTriggerFlags >> 12 ) & 1 ) ) ||
	     ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGERESET && ( TUINT8( m_uiTriggerFlags >> 14 ) & 1 ) ) )
	{
		if ( ( !( m_uiTriggerFlags & TRIGGERFLAG_Triggered ) || !( TUINT8( m_uiTriggerFlags >> 1 ) & 1 ) ) &&
		     ( a_rPacket.m_eMessage != AEntityIOPacket::MESSAGE_TOUCH || CanObjectTrigger( a_pCaller ) ) &&
		     !( m_uiTriggerFlags & ( TRIGGERFLAG_Delayed | TRIGGERFLAG_Pulsing ) ) )
		{
			if ( 0.0f < m_fDelay )
			{
				m_fTimer = m_fDelay;
				m_uiTriggerFlags |= TRIGGERFLAG_Delayed;
				SetTicking( TTRUE );
			}
			else
			{
				m_uiTriggeredBy = TREINTERPRETCAST( TUINT32, a_pCaller );
				Trigger();
			}

			m_uiTriggerFlags |= TRIGGERFLAG_Triggered;
		}
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_UNTOUCH )
	{
		m_uiTriggerFlags &= ~TRIGGERFLAG_Touched;

		// "ResetOnExit"
		if ( TUINT8( m_uiTriggerFlags >> 0 ) & 1 )
			m_uiTriggerFlags &= ~TRIGGERFLAG_Triggered;
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_CHALLENGERESET )
	{
		SendMessageToTargets( a_pCaller, AEntityIOPacket::MESSAGE_CHALLENGERESET, 0 );
		Reset( a_pCaller );
	}
	else if ( eMessage == AEntityIOPacket::MESSAGE_SKIP && ( TUINT8( m_uiTriggerFlags >> 7 ) & 1 ) )
	{
		m_fTimer = 0.0f;
		Trigger();
		m_uiTriggerFlags &= ~TRIGGERFLAG_Delayed;
		SetTicking( TFALSE );
	}
}

// $deBlob: FUNCTION 0079b720
void APropTrigger::CreatePhysics( const TMatrix44* a_pMatrix, const TVector4* a_pBBox, TBOOL a_bAdd )
{
	TASSERT( TNULL == m_pPhysicsData );

	m_pPhysicsData                        = new PhysicsData;
	m_pPhysicsData->m_oOwnerInfo.m_eType  = 0;
	m_pPhysicsData->m_oOwnerInfo.m_pOwner = TNULL;
	m_pPhysicsData->m_pHandler            = new APropTriggerCollisionEventHandler( this );
	m_pPhysicsData->m_pMover              = TNULL;
	m_pPhysicsData->m_oOwnerInfo.m_eType  = 0;
	m_pPhysicsData->m_oOwnerInfo.m_pOwner = this;
	m_pPhysicsData->m_oMatrix             = *a_pMatrix;

	// The box's origin is its centre: moves the body down by the half height along the matrix's up axis
	TMatrix44& rMatrix = m_pPhysicsData->m_oMatrix;
	TVector4   vUp( rMatrix.m_f21, rMatrix.m_f22, rMatrix.m_f23, rMatrix.m_f24 );
	vUp.Multiply( a_pBBox->y );
	rMatrix.m_f41 -= vUp.x;
	rMatrix.m_f42 -= vUp.y;
	rMatrix.m_f43 -= vUp.z;

	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();
	m_pPhysicsData->m_pBody          = pPhysicsManager->CreateFixedBody( &m_pPhysicsData->m_oMatrix, TNULL );
	pPhysicsManager->SetBox( m_pPhysicsData->m_pBody, *a_pBBox );
	pPhysicsManager->SetListener( m_pPhysicsData->m_pBody, m_pPhysicsData->m_pHandler );
	pPhysicsManager->SetCollisionLayer( m_pPhysicsData->m_pBody, 0x200 );
	pPhysicsManager->SetCollisionGroup( m_pPhysicsData->m_pBody, 6 );

	if ( m_pPhysicsData->m_pBody )
		m_pPhysicsData->m_pBody->m_uiSurface = 0;

	pPhysicsManager->UpdateBodyTransform( m_pPhysicsData->m_pBody, TFALSE );

	if ( m_pPhysicsData->m_pBody )
		m_pPhysicsData->m_pBody->m_pUserData = &m_pPhysicsData->m_oOwnerInfo;

	if ( a_bAdd )
		pPhysicsManager->AddToSimulation( m_pPhysicsData->m_pBody, TFALSE );
}

// $deBlob: FUNCTION 0079b700
void APropTrigger::Reset( ABaseEntity* a_pEntity )
{
	m_uiTriggerFlags &= ~( TRIGGERFLAG_Delayed | TRIGGERFLAG_Triggered | TRIGGERFLAG_Touched | TRIGGERFLAG_Pulsing );
	m_iLife = m_iPaintPoints;
	AInvisibleEntity::Reset( a_pEntity );
}

// $deBlob: FUNCTION 0079ba10
void APropTrigger::SetBBox( TVector4 a_vBBox )
{
	if ( m_pPhysicsData == TNULL )
	{
		AInvisibleEntity::SetBBox( TVector4( 0.1f, 0.1f, 0.1f, 1.0f ) );
		return;
	}

	AInvisibleEntity::SetBBox( a_vBBox );
}

// $deBlob: FUNCTION 0079ba50
TBOOL APropTrigger::IsTargetable()
{
	return m_iPaintPoints >= 0 && ( m_uiTriggerFlags & ( TRIGGERFLAG_Delayed | TRIGGERFLAG_Pulsing ) ) == 0 && ( m_uiTriggerFlags & ( TRIGGERFLAG_TriggerOnce | TRIGGERFLAG_Triggered ) ) != ( TRIGGERFLAG_TriggerOnce | TRIGGERFLAG_Triggered );
}

// $deBlob: FUNCTION 0079ba90
TBOOL APropTrigger::CanObjectTrigger( TObject* a_pObject )
{
	return a_pObject->IsA( &TGetClass( ABlobEntity ) ) ? TTRUE : TFALSE;
}

// $deBlob: FUNCTION 0079bab0
void APropTrigger::Trigger()
{
	if ( !( m_uiTriggerFlags & TRIGGERFLAG_Pulse ) )
	{
		TriggerAllTargets( this );
		ToggleWaypointSets();
	}
	else if ( !( m_uiTriggerFlags & TRIGGERFLAG_Pulsing ) )
	{
		// Starts the pulse (Tick)
		SetTicking( TTRUE );
		m_fTimer = 0.0f;
		m_uiTriggerFlags |= TRIGGERFLAG_Pulsing;
		m_uiNextPulseTarget = 0;
	}

	// "QuitChallenge"
	if ( m_uiTriggerFlags & TRIGGERFLAG_QuitChallenge )
		AChallengeManager::GetSingleton()->QuitChallenge( TFALSE );
}

// $deBlob: FUNCTION 0079b950
void APropTrigger::DestroyPhysics()
{
	TPhysicsManager* pPhysicsManager = TPhysicsManager::GetSingleton();

	if ( m_pPhysicsData == TNULL )
		return;

	if ( m_pPhysicsData->m_pMover )
		pPhysicsManager->RemoveCustomMover( m_pPhysicsData->m_pMover );

	pPhysicsManager->RemoveFromSimulation( m_pPhysicsData->m_pBody, TFALSE );
	pPhysicsManager->DestroyBody( m_pPhysicsData->m_pBody );

	delete m_pPhysicsData->m_pHandler;
	m_pPhysicsData->m_pHandler = TNULL;

	delete m_pPhysicsData->m_pMover;
	m_pPhysicsData->m_pMover = TNULL;

	::operator delete( m_pPhysicsData );
	m_pPhysicsData = TNULL;
}

// $deBlob: FUNCTION 0079bb30
void APropTrigger::UntoggleNodes()
{
	for ( TINT i = 0; i < m_uiNumWaypointSets; i++ )
	{
		if ( m_pWaypointSets[ i ].m_bToggled )
		{
			AWaypointData::GetSingleton()->GetSet( m_pWaypointSets[ i ].m_uiSet ).GetNode( TINT16( m_pWaypointSets[ i ].m_uiNode ) ).ToggleFlag();
			m_pWaypointSets[ i ].m_bToggled = TFALSE;
		}
	}
}

// Sends a_eMessage to the targets and toggles the waypoint sets
// $deBlob: FUNCTION 0079bce0
void APropTrigger::SendMessageAndToggleWaypoints( AEntityIOPacket::MESSAGE a_eMessage )
{
	SendMessageToTargets( this, a_eMessage, 0 );
	ToggleWaypointSets();
}

// $deBlob: FUNCTION 0079bd10
void APropTrigger::ToggleWaypointSets()
{
	for ( TINT i = 0; i < m_uiNumWaypointSets; i++ )
	{
		if ( !m_pWaypointSets[ i ].m_bToggled )
		{
			AWaypointData::GetSingleton()->GetSet( m_pWaypointSets[ i ].m_uiSet ).GetNode( TINT16( m_pWaypointSets[ i ].m_uiNode ) ).ToggleFlag();
			m_pWaypointSets[ i ].m_bToggled = TTRUE;
		}
	}
}

// a_pOther touched the phantom: sends MESSAGE_TOUCH to the trigger and MESSAGE_TOUCHEDBY to the toucher, both with
// { the toucher's position, the direction from the trigger to it }
// $deBlob: FUNCTION 0079aa20
void APropTrigger::APropTriggerCollisionEventHandler::OnTriggerEnter( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther )
{
	APropTrigger* pTrigger = m_pTrigger;

	if ( pTrigger == TNULL )
		return;

	TUINT32 uiLayers = ( a_pOther != TNULL ) ? a_pOther->m_uiCollisionFilterInfo & 0xFFF : 0;

	if ( !( uiLayers & 6 ) )
		return;

	struct
	{
		TVector4 vPosition;
		TVector4 vDirection;
	} oContact;

	oContact.vPosition = pTrigger->GetMatrix().GetTranslation();

	AEntityIOPacket oTouch;
	oTouch.m_eMessage = AEntityIOPacket::MESSAGE_TOUCH;
	oTouch.m_iData    = TREINTERPRETCAST( TINT, &oContact );

	AEntityIOPacket oTouchedBy;
	oTouchedBy.m_eMessage = AEntityIOPacket::MESSAGE_TOUCHEDBY;
	oTouchedBy.m_iData    = TREINTERPRETCAST( TINT, &oContact );

	if ( a_pOther != TNULL && a_pOther->m_pUserData != TNULL )
	{
		ABaseEntity*    pOther    = TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pOther->m_pUserData )->m_pOwner );
		const TVector4& rOtherPos = pOther->GetMatrix().GetTranslation();

		oContact.vDirection.x = rOtherPos.x - pTrigger->GetMatrix().GetTranslation().x;
		oContact.vDirection.y = rOtherPos.y - pTrigger->GetMatrix().GetTranslation().y;
		oContact.vDirection.z = rOtherPos.z - pTrigger->GetMatrix().GetTranslation().z;
		oContact.vDirection.w = rOtherPos.w;
		oContact.vDirection.Normalise();
		oContact.vPosition = rOtherPos;

		m_pTrigger->HandleEntityIO( pOther, oTouch );
		pOther->HandleEntityIO( m_pTrigger, oTouchedBy );
	}
}

// a_pOther left the phantom: sends MESSAGE_UNTOUCH to the trigger with { the trigger's position, -Y }
// $deBlob: FUNCTION 0079ab10
void APropTrigger::APropTriggerCollisionEventHandler::OnTriggerLeave( TPhysicsObjectContainer* a_pBody, TPhysicsObjectContainer* a_pOther )
{
	APropTrigger* pTrigger = m_pTrigger;

	if ( pTrigger == TNULL || a_pOther == TNULL || a_pOther->m_pUserData == TNULL )
		return;

	struct
	{
		TVector4 vPosition;
		TVector4 vDirection;
	} oContact;

	oContact.vPosition  = pTrigger->GetMatrix().GetTranslation();
	oContact.vDirection = TVector4::VEC_NEGY;

	AEntityIOPacket oUntouch;
	oUntouch.m_eMessage = AEntityIOPacket::MESSAGE_UNTOUCH;
	oUntouch.m_iData    = TREINTERPRETCAST( TINT, &oContact );

	pTrigger->HandleEntityIO( TSTATICCAST( ABaseEntity, TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pOther->m_pUserData )->m_pOwner ), oUntouch );
}
