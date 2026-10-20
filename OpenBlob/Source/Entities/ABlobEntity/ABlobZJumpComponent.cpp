#include "pch.h"
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include "Entities/ABoss/ABoss.h"
#include "ABlobZJumpComponent.h"
#include "ABlobSoundComponent.h"
#include "ABlobAnimationComponent.h"
#include "ABlobEntity.h"
#include "Entities/AZJumpData.h"
#include "Entities/APropSwitchPressure.h"
#include "Entities/APropTrigger.h"
#include "Buildings/ALandmarkInstance.h"
#include "Buildings/ABuildingManager.h"
#include "ABlobColourComponent.h"
#include "Entities/APropPlatform.h"
#include "Entities/APropWaterHydrant.h"
#include "Entities/ACar/ACar.h"
#include "Entities/ALeechbot2/ALeechBot2.h"
#include "Entities/ABigPaintBot/ABigPaintBot.h"
#include "Entities/AInky_Baton/AInky_Baton.h"
#include "Entities/AInky_Baton_Elite/AInky_Baton_Elite.h"
#include "Entities/AEntityHash.h"
#include "Entities/AVisibleEntity.h"
#include "Entities/AEntityIOPacket.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Challenges/AChallengeManager.h"
#include "GameController/AGameStatus.h"
#include "GameInterface/APlayerManager.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include "Terrain2/TerrainTypes.h"
#include "XUI/InGame/HUD/ABlobHud.h"
#include <Physics/Havok/TPhysicsManager.h>
#include <Render/TRender.h>
#include <Toshi/TStringManager.h>
#include <XUI/TXUI.h>
#include <XUI/TXUICanvas.h>
#include <XUI/TXUITimelineControl.h>

#include <GUI/T2GUI.h>
#include <XUI/TXUIControl.h>
#include <XUI/TXUIVisual.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING


// $deBlob: FUNCTION 00763e20
TBOOL ABlobZJumpComponent::PriorityTarget::GetPosition( TVector4& a_rOutPosition ) const
{
	if ( m_eType == PriorityTarget::TARGET_LMARK )
	{
		if ( m_pTarget != TNULL && m_iIndex >= 0 )
		{
			TREINTERPRETCAST( ALandmarkInstance*, m_pTarget )->GetTriggerPosition( m_iIndex, a_rOutPosition );
			return TTRUE;
		}
	}
	else if ( m_eType == PriorityTarget::TARGET_ENTITY )
	{
		ABaseEntity* pTarget = TREINTERPRETCAST( ABaseEntity*, m_pTarget );

		if ( pTarget != TNULL && ( !pTarget->IsPooled() || pTarget->GetPooledID() == m_iIndex ) )
		{
			a_rOutPosition = pTarget->GetTargetOffset();

			TVector4 vBase = pTarget->GetBasePosition();
			a_rOutPosition.x += vBase.x;
			a_rOutPosition.y += vBase.y;
			a_rOutPosition.z += vBase.z;
			return TTRUE;
		}
	}
	else if ( m_eType == PriorityTarget::TARGET_INSTANCE && m_pTarget != TNULL )
	{
		// The target's +0x10 is a pointer to a pointer to the position
		TVector4* pPosition = *TREINTERPRETCAST( TVector4**, *TREINTERPRETCAST( TUINT8**, TREINTERPRETCAST( TUINT8*, m_pTarget ) + 0x10 ) );
		a_rOutPosition      = *pPosition;
		return TTRUE;
	}

	a_rOutPosition.Set( 0.0f, 0.0f, 0.0f, 1.0f );
	return TFALSE;
}

// $deBlob: FUNCTION 00763fb0
ABlobZJumpComponent::ABlobZJumpComponent( ABlobEntity* a_pOwner )
{
	m_pOwner                = a_pOwner;
	m_eState                = STATE_NONE;
	m_pMaterial             = TNULL;
	m_uiUnk10               = 0;
	m_pScreenEffectInstance = TNULL;
	m_pPoint                = TNULL;
	m_pCurrentNode          = TNULL;
	m_pTargetNode           = TNULL;
	m_pStandNode            = TNULL;
	m_pUnk2C                = TNULL;
	m_pLastPoint            = TNULL;
	m_fSearchTimer          = 0.0f;
	m_fKeepTargetsTimer     = 0.0f;
	m_fHoldTime             = 0.0f;
	m_pCurrentPriorityLock  = TNULL;
	m_iUnk64                = 0;
	m_pLockElement          = TNULL;
	m_uiUnk6C               = 0;
	m_pEnergyFill           = TNULL;
	m_pOutline              = TNULL;
	m_pCircleAbove          = TNULL;
	m_pGreyFill             = TNULL;
	m_pCannotLock           = TNULL;
	m_pCannotTarget         = TNULL;
	m_pPout                 = TNULL;
	m_pPassiveFill          = TNULL;

	for ( TINT i = 0; i < TARRAYSIZE( m_apPaintMeters ); i++ )
		m_apPaintMeters[ i ] = TNULL;

	m_pScore      = TNULL;
	m_pScoreWhite = TNULL;
	m_bUnk120     = TFALSE;
	m_vUnk124     = TVector4::VEC_ZERO;
	m_vUnk124.w   = 1.0f;
	m_vUnk134     = m_vUnk124;
	m_vUnk144     = m_vUnk124;
	m_fUnk154     = 0.0f;

	m_fFlightTime         = 0.0f;
	m_fLockElementOffsetX = 0.0f;
	m_aUnk160[ 0 ]        = 0;
	m_aUnk160[ 1 ]        = 0;

	m_wszLockText[ 0 ]  = L'\0';
	m_pRightArrowColour = TNULL;
	m_pLeftArrowColour  = TNULL;

	m_pMaterial   = T2GUI::CreateMaterial( T2GUI::GetTexture( "zjumpoffscreen.tga" ) );
	m_pTargets    = new PriorityTarget[ NUM_TARGETS ];
	m_pRectangles = new T2GUIRectangle[ NUM_TARGETS ];

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		T2GUIRectangle* pRectangle = &m_pRectangles[ i ];

		pRectangle->Create( 16.0f, 16.0f );
		pRectangle->SetMaterial( m_pMaterial );
		pRectangle->SetVisible( TFALSE );
		pRectangle->SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
		pRectangle->FlipU();
		T2GUI::GetSingleton()->GetRootElement()->AddChildTail( pRectangle );

		m_pTargets[ i ].m_eType      = -1;
		m_pTargets[ i ].m_pRectangle = pRectangle;
		m_lFreeTargets.PushBack( &m_pTargets[ i ] );
	}

	m_pGroups       = new PriorityGroup[ NUM_GROUPS ];
	m_pScreenEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_Env_ZJump_Screen" );
}

// $deBlob: FUNCTION 00764330
// $deBlob: FUNCTION 00751450
ABlobZJumpComponent::~ABlobZJumpComponent()
{
	m_lFreeTargets.Clear();
	m_lTargets.Clear();
	m_lGroups.Clear();

	for ( TINT i = 0; i < NUM_GROUPS; i++ )
		m_pGroups[ i ].m_lLockList.Clear();

	delete[] m_pGroups;

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
		m_pRectangles[ i ].RemoveSelf();

	delete[] m_pRectangles;
	delete[] m_pTargets;

	T2GUI::DestroyMaterial( m_pMaterial );

	if ( m_pScreenEffectInstance )
	{
		m_pScreenEffectInstance->StopInstant();
		m_pScreenEffectInstance = TNULL;
	}
}

// Stops the hold timeline of the HUD and starts the Z-jump fail effect at Blob (states 3 and 4, the hold ran out)
static void ZJumpFailed( ABlobEntity* a_pOwner )
{
	ABlobHud::GetSingleton()->GetHoldTimeline()->SetVisible( TFALSE );

	if ( ABlobEntity::ms_pZJumpFailEffect != TNULL )
	{
		TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( ABlobEntity::ms_pZJumpFailEffect );

		if ( pInstance != TNULL )
		{
			TASSERT( a_pOwner->GetBodyPosition().w == 1.0f );
			pInstance->GetTransform().SetTranslation( a_pOwner->GetBodyPosition() );
		}
	}
}

// The position of the target of a_rTarget's entity: GetTargetOffset + GetBasePosition
static void GetEntityTargetPosition( ABaseEntity* a_pEntity, TVector4& a_rOutPosition )
{
	a_rOutPosition = a_pEntity->GetTargetOffset();

	TVector4 vBase = a_pEntity->GetBasePosition();
	a_rOutPosition.x += vBase.x;
	a_rOutPosition.y += vBase.y;
	a_rOutPosition.z += vBase.z;
}

// The Z-jump node of a Z-jump instance (zjump.m_iSet, zjump.m_iNode)
static AZJumpData::Node* GetInstanceNode( TerrainTypes::CellInstance* a_pInstance )
{
	TASSERT( a_pInstance->zjump.m_iSet >= 0 && a_pInstance->zjump.m_iNode >= 0 );
	return &AZJumpData::GetSingleton()->GetSet( a_pInstance->zjump.m_iSet )->m_pNodes[ a_pInstance->zjump.m_iNode ];
}

// a_pNode is m_pStandNode or one of its links, in m_pStandNode's set (the ZPRIORITY 0x25 targets)
static TBOOL IsNextNode( AZJumpData::Node* a_pNode, AZJumpData::Node* a_pFrom )
{
	return a_pFrom != TNULL && a_pNode != TNULL && a_pNode->m_pSet == a_pFrom->m_pSet &&
	    ( a_pNode == a_pFrom || a_pNode == a_pFrom->m_apLinks[ 0 ] || a_pNode == a_pFrom->m_apLinks[ 1 ] );
}

// $deBlob: FUNCTION 007645c0
void ABlobZJumpComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( !AZJumpData::GetSingleton() )
		return;

	if ( !AGameStatus::GetSingleton()->IsMultiplayer() )
		ABlobHud::GetSingleton()->GetHoldTimeline()->SetVisible( TFALSE );

	if ( m_fKeepTargetsTimer > 0.0f )
		m_fKeepTargetsTimer -= a_fDeltaTime;

	if ( m_pStandNode != TNULL && ( m_eState == STATE_LOCK || m_eState == STATE_NONE ) )
	{
		ABlobEntity* pOwner  = m_pOwner;
		TFLOAT       fRadius = pOwner->GetBoundingSphereRadius();

		if ( pOwner->GetBodyPosition().y - fRadius > m_pStandNode->GetPosition().y )
		{
			m_pStandNode = TNULL;
			m_pUnk2C     = TNULL;
		}
	}

	if ( m_pLastPoint != TNULL )
	{
		ABlobEntity* pOwner = m_pOwner;

		if ( TVector4::Distance( m_pLastPoint->m_pNode->GetPosition(), pOwner->GetBodyPosition() ) - pOwner->GetBoundingSphereRadius() > 3.0f )
			m_pLastPoint = TNULL;
	}

	// The target the lock element aims at (UpdateAim)
	TVector4 vAim;

	switch ( m_eState )
	{
		case STATE_POINT:
		{
			ABlobEntity* pOwner = m_pOwner;

			if ( 3.0f > TVector4::Distance( m_pPoint->m_pNode->GetPosition(), pOwner->GetBodyPosition() ) - pOwner->GetBoundingSphereRadius() )
			{
				vAim = m_pTargetNode->GetPosition();
				break;
			}

			// Blob left the point
			AZJumpData::GetSingleton()->SetPointEffect( AZJumpData::GetSingleton()->GetPointIndex( m_pPoint ), TTRUE );
			m_pPoint       = TNULL;
			m_pCurrentNode = TNULL;
			m_pTargetNode  = TNULL;
			HideLockElement();

			if ( m_lTargets.IsEmpty() )
			{
				PruneGroups();
				FindNodeTargets();
				FindEntityTargets();
				FindLandmarkTargets();
				m_fSearchTimer = 0.3f;
			}

			SelectLock();

			if ( m_pCurrentPriorityLock != TNULL && m_pCurrentPriorityLock->m_eType == PriorityTarget::TARGET_LMARK )
				SetState( STATE_LOCKLMARK );
			else
				SetState( STATE_LOCK );

			return;
		}
		case STATE_NODEFLIGHT:
			m_fUnk154 += a_fDeltaTime;

			if ( m_pTargetNode == TNULL )
				return;

			vAim = m_pTargetNode->GetPosition();
			break;
		case STATE_AIM:
			if ( m_pCurrentNode == TNULL )
				return;

			m_fHoldTime -= a_fDeltaTime;
			UpdateHoldTimeline();

			if ( m_fHoldTime < 0.0f )
			{
				ZJumpFailed( m_pOwner );
				m_pOwner->GetSoundComponent()->PlayZJumpLand();
				m_pOwner->GetAnimationComponent()->SetState( ABlobAnimationComponent::STATE_ZJUMPFAIL );
				SetState( STATE_TIMEOUT );
				m_pOwner->InkBounce();
			}

			return;
		case STATE_AIMNODE:
			if ( m_pCurrentNode == TNULL )
				return;

			m_fHoldTime -= a_fDeltaTime;
			UpdateHoldTimeline();

			if ( m_fHoldTime < 0.0f )
			{
				ZJumpFailed( m_pOwner );
				m_pOwner->GetSoundComponent()->PlayZJumpLand();
				SetState( STATE_TIMEOUT );
				m_pOwner->InkBounce();
				HideLockElement();
				return;
			}

			vAim = m_pTargetNode->GetPosition();
			break;
		case STATE_ATTACKSLAM:
		case STATE_SETEND:
		case STATE_TIMEOUT:
			if ( m_pOwner->GetBlobState() == ABlobEntity::STATE_FLOAT )
				SetState( STATE_NONE );

			return;
		case STATE_LOCK:
		case STATE_LOCKLMARK:
		{
			AZJumpData::Point* pPoint = FindPoint();

			if ( pPoint != TNULL )
			{
				// A Z-jump point in reach: the lock goes to it (state 1)
				m_pPoint       = pPoint;
				m_bUnk120      = pPoint->m_iUnk08 == 1;
				m_pCurrentNode = pPoint->m_pNode;
				TASSERT( pPoint->m_pSet->m_iNodeCount > 1, "Every Z-Jump set must have at least 2 nodes!" );
				m_pTargetNode = pPoint->m_pNode->m_apLinks[ 0 ];
				m_iUnk64      = 0;

				while ( !m_lTargets.IsEmpty() )
					FreeTarget( *m_lTargets.PopFront() );

				HideGroupMarkers();

				if ( IsPointWithoutEntities( m_pPoint ) == TTRUE )
				{
					m_pOwner->GetSoundComponent()->StartZJumpPointLoop();
					m_pOwner->GetSoundComponent()->PlayLockAim();
				}

				SetState( STATE_POINT );
				UpdateLockElement( TFALSE );
				return;
			}

			TINT ePrevLockType = -1;
			m_fSearchTimer -= a_fDeltaTime;

			if ( m_pCurrentPriorityLock != TNULL )
				ePrevLockType = m_pCurrentPriorityLock->m_ePriorityType;

			TBOOL bRegroup = !m_lGroups.IsEmpty();

			if ( bRegroup )
			{
				PruneGroups();
				bRegroup = !m_lGroups.IsEmpty();
			}

			if ( bRegroup )
			{
				// Updates the priority types and distances of the grouped targets and sorts them again
				T2DList<PriorityGroup> lGroups;
				lGroups.Splice( lGroups.End(), m_lGroups );

				while ( !lGroups.IsEmpty() )
				{
					PriorityGroup*           pGroup = lGroups.PopBack();
					T2DList<PriorityTarget>& rLocks = pGroup->m_lLockList;

					if ( rLocks.IsEmpty() )
						continue;

					T2DList<PriorityTarget> lLocks;
					lLocks.Splice( lLocks.End(), rLocks );

					while ( !lLocks.IsEmpty() )
					{
						PriorityTarget* pTarget       = lLocks.PopBack();
						TINT            ePrevPriority = pTarget->m_ePriorityType;

						if ( ePrevPriority == 1 || ePrevPriority == 0x25 )
						{
							if ( pTarget->m_eType == PriorityTarget::TARGET_ENTITY )
							{
								APropPlatform* pPlatform = TSTATICCAST( APropPlatform, pTarget->m_pTarget );
								TASSERT( pPlatform == TNULL || pPlatform->IsA( &TGetClass( APropPlatform ) ) );

								pTarget->m_ePriorityType = IsNextNode( pPlatform->GetZJumpNode(), m_pStandNode ) ? 0x25 : 1;
							}
							else if ( pTarget->m_eType == PriorityTarget::TARGET_INSTANCE )
							{
								TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, pTarget->m_pTarget );
								pTarget->m_ePriorityType              = -1;

								if ( ( pInstance->m_uiFlags & 7 ) == 7 && IsNextNode( GetInstanceNode( pInstance ), m_pStandNode ) )
									pTarget->m_ePriorityType = 0x25;
							}
						}
						else if ( ePrevPriority == 0x12 || ePrevPriority == 0x13 )
						{
							// A landmark trigger: 0x13 when Blob can paint it, else 0x12
							TASSERT( pTarget->m_eType == PriorityTarget::TARGET_LMARK );

							TUINT32 uiBlobColour = m_pOwner->m_pColourComponent->GetCurrentColour();
							TUINT32 uiColour     = pTarget->m_uiFlags & 0xF;
							TBOOL   bCanPaint;

							if ( ( pTarget->m_uiFlags & 0x10 ) == 0 || uiColour == 0 )
								bCanPaint = ( uiBlobColour & 0x10 ) == 0 && uiBlobColour != 0;
							else
								bCanPaint = uiColour == ( ( uiBlobColour & 0x10 ) ? 0x10 : uiBlobColour ) || ( TUINT8( m_pOwner->m_uiFlags >> 0x15 ) & 1 ) != 0;

							pTarget->m_ePriorityType = bCanPaint ? 0x13 : 0x12;
						}
						else if ( pTarget->m_eType == PriorityTarget::TARGET_ENTITY )
						{
							pTarget->m_ePriorityType = TINT8( TSTATICCAST( ABaseEntity, pTarget->m_pTarget )->GetCollisionGroupAndType() >> 8 );
						}

						TINT ePriorityType   = pTarget->m_ePriorityType;
						pTarget->m_iPriority = -1;

						if ( ePriorityType != -1 )
							pTarget->m_iPriority = GetPriority( ePriorityType, m_pOwner );

						if ( pTarget->m_iPriority == -1 || ( pTarget->m_ePriorityType != ePrevPriority && !IsStillLockable( *pTarget ) ) )
						{
							if ( m_pCurrentPriorityLock == pTarget )
								m_pCurrentPriorityLock = TNULL;

							FreeTarget( *pTarget );
							continue;
						}

						UpdateTargetFlags( *pTarget );

						const TVector4& rBlobPosition = m_pOwner->GetBodyPosition();

						if ( pTarget->m_eType == PriorityTarget::TARGET_ENTITY )
						{
							if ( pTarget->m_ePriorityType != 0x25 )
							{
								pTarget->m_fDistance = TVector4::DistanceSq( rBlobPosition, TSTATICCAST( ABaseEntity, pTarget->m_pTarget )->GetMatrix().GetTranslation() );
							}
							else
							{
								APropPlatform* pPlatform = TSTATICCAST( APropPlatform, pTarget->m_pTarget );
								TASSERT( pPlatform == TNULL || pPlatform->IsA( &TGetClass( APropPlatform ) ) );

								pTarget->m_fDistance = GetNodeWeight( pPlatform->GetZJumpNode() );
							}
						}
						else if ( pTarget->m_eType == PriorityTarget::TARGET_LMARK )
						{
							TVector4 vTrigger;
							TSTATICCAST( ALandmarkInstance, pTarget->m_pTarget )->GetTriggerPosition( pTarget->m_iIndex, vTrigger );
							pTarget->m_fDistance = TVector4::DistanceSq( rBlobPosition, vTrigger );
						}
						else if ( pTarget->m_eType == PriorityTarget::TARGET_INSTANCE )
						{
							TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, pTarget->m_pTarget );

							if ( pTarget->m_ePriorityType != 0x25 || ( pInstance->m_uiFlags & 7 ) != 7 )
								pTarget->m_fDistance = TVector4::DistanceSq( rBlobPosition, pInstance->m_pMatrix->GetTranslation() );
							else
								pTarget->m_fDistance = GetNodeWeight( GetInstanceNode( pInstance ) );
						}

						InsertGroupTarget( m_pGroups[ pTarget->m_ePriorityType ].m_lLockList, pTarget );
					}

					if ( !rLocks.IsEmpty() )
						InsertGroup( m_lGroups, pGroup );
				}
			}

			for ( TINT i = 0; i < NUM_GROUPS; i++ )
			{
				PriorityGroup* pGroup = &m_pGroups[ i ];

				if ( !pGroup->m_lLockList.IsEmpty() && !pGroup->IsLinked() )
					InsertGroup( m_lGroups, pGroup );
			}

			if ( m_lTargets.IsEmpty() )
			{
				TBOOL bChecked = TFALSE;

				if ( !m_lGroups.IsEmpty() )
				{
					// Checks the view of one grouped target per frame (round robin over m_pTargets)
					for ( TINT i = m_iUnk64 + 1; i != m_iUnk64 && i < NUM_TARGETS; i++ )
					{
						PriorityTarget* pTarget = &m_pTargets[ i ];

						if ( pTarget->m_eType != -1 && ( pTarget->m_uiFlags & PriorityTarget::FLAGS_UNSORTED ) == 0 )
						{
							if ( !IsTargetVisible( *pTarget ) )
							{
								if ( pTarget == m_pCurrentPriorityLock )
									m_pCurrentPriorityLock = TNULL;

								pTarget->Remove();
								FreeTarget( *pTarget );
							}

							m_iUnk64 = i;
							bChecked = TTRUE;
							break;
						}
					}
				}

				if ( !bChecked && m_fSearchTimer <= 0.0f )
				{
					m_iUnk64 = 0;
					FindEntityTargets();
					FindLandmarkTargets();
					m_fSearchTimer = 0.3f;
				}
			}
			else
			{
				// Sorts one new target per frame into its group
				PriorityTarget* pTarget = m_lTargets.PopFront();

				if ( !IsStillLockable( *pTarget ) || !IsTargetVisible( *pTarget ) )
				{
					FreeTarget( *pTarget );
				}
				else
				{
					pTarget->m_uiFlags &= ~PriorityTarget::FLAGS_UNSORTED;

					PriorityGroup* pGroup = &m_pGroups[ pTarget->m_ePriorityType ];
					InsertGroupTarget( pGroup->m_lLockList, pTarget );

					if ( pGroup->IsLinked() )
						pGroup->Remove();

					InsertGroup( m_lGroups, pGroup );
				}
			}

			TASSERT( !m_pCurrentPriorityLock || IsStillLockable( *m_pCurrentPriorityLock ) );

			// The lock goes to the best target of its old priority type, else of the best group
			if ( ePrevLockType != -1 && ( m_pCurrentPriorityLock == TNULL || ( m_pCurrentPriorityLock->m_uiFlags & PriorityTarget::FLAGS_LOCKED ) == 0 ) )
			{
				PriorityTarget*          pNewLock = TNULL;
				T2DList<PriorityTarget>& rLocks   = m_pGroups[ ePrevLockType ].m_lLockList;

				if ( rLocks.IsEmpty() || ( pNewLock = rLocks.Head(), pNewLock == TNULL ) || m_pCurrentPriorityLock == TNULL || m_pCurrentPriorityLock->m_ePriorityType != ePrevLockType )
				{
					while ( !m_lGroups.IsEmpty() )
					{
						PriorityGroup* pGroup = m_lGroups.Head();

						if ( !pGroup->m_lLockList.IsEmpty() )
						{
							pNewLock = pGroup->m_lLockList.Head();
							break;
						}

						pGroup->Remove();
					}
				}

				if ( pNewLock != TNULL && m_pCurrentPriorityLock != pNewLock )
				{
					m_pOwner->GetSoundComponent()->PlayLockOn();
					m_pCurrentPriorityLock = pNewLock;
					pNewLock->m_pRectangle->SetVisible( TFALSE );
					HideLockElement();
					ShowLock( *m_pCurrentPriorityLock );
				}
			}

			if ( m_pCurrentPriorityLock == TNULL )
				SelectLock();

			if ( m_pCurrentPriorityLock == TNULL )
			{
				SetState( STATE_LOCK );
				return;
			}

			TASSERT( IsStillLockable( *m_pCurrentPriorityLock ) );

			switch ( m_pCurrentPriorityLock->m_eType )
			{
				case 0:
					SetState( STATE_LOCK );
					GetEntityTargetPosition( TSTATICCAST( ABaseEntity, m_pCurrentPriorityLock->m_pTarget ), vAim );
					break;
				case 1:
					SetState( STATE_LOCKLMARK );
					TSTATICCAST( ALandmarkInstance, m_pCurrentPriorityLock->m_pTarget )->GetTriggerPosition( m_pCurrentPriorityLock->m_iIndex, vAim );
					break;
				case 2:
					SetState( STATE_LOCK );
					vAim = TSTATICCAST( TerrainTypes::CellInstance, m_pCurrentPriorityLock->m_pTarget )->m_pMatrix->GetTranslation();
					break;
				default:
					return;
			}

			break;
		}
		case STATE_ATTACKFLIGHT:
		case STATE_SWITCHSLAM:
		{
			m_fUnk154 += a_fDeltaTime;

			if ( m_pCurrentPriorityLock == TNULL )
				return;

			if ( !IsStillAttackable( *m_pCurrentPriorityLock ) )
			{
				m_pCurrentPriorityLock->Remove();
				FreeTarget( *m_pCurrentPriorityLock );
				m_pCurrentPriorityLock = TNULL;
				EndAttack( TFALSE );
				return;
			}

			if ( m_pCurrentPriorityLock->m_eType == PriorityTarget::TARGET_ENTITY )
			{
				GetEntityTargetPosition( TSTATICCAST( ABaseEntity, m_pCurrentPriorityLock->m_pTarget ), vAim );
			}
			else if ( m_pCurrentPriorityLock->m_eType == PriorityTarget::TARGET_INSTANCE )
			{
				vAim = TSTATICCAST( TerrainTypes::CellInstance, m_pCurrentPriorityLock->m_pTarget )->m_pMatrix->GetTranslation();
			}
			else
			{
				// NOTE: retail aims at its uninitialised stack vector here (a landmark lock in states 9/12)
				vAim = TVector4::VEC_ZERO;
			}

			break;
		}
		case STATE_LMARKFLIGHT:
			m_fUnk154 += a_fDeltaTime;

			if ( m_pCurrentPriorityLock == TNULL )
				return;

			TSTATICCAST( ALandmarkInstance, m_pCurrentPriorityLock->m_pTarget )->GetTriggerPosition( m_pCurrentPriorityLock->m_iIndex, vAim );
			break;
		default:
			return;
	}

	UpdateAim( a_fDeltaTime, vAim );
}

// $deBlob: FUNCTION 00765410
void ABlobZJumpComponent::Reset()
{
	if ( TXUIControl* pLockElement = TSTATICCAST( TXUIControl, m_pLockElement ) )
	{
		pLockElement->SetVisible( TFALSE );
		pLockElement->GetVisual()->StopAnimation();
	}

	m_eState               = STATE_NONE;
	m_uiUnk10              = 0;
	m_fSearchTimer         = 0.0f;
	m_fHoldTime            = 0.0f;
	m_fKeepTargetsTimer    = 0.0f;
	m_pPoint               = TNULL;
	m_pCurrentNode         = TNULL;
	m_pTargetNode          = TNULL;
	m_pStandNode           = TNULL;
	m_pUnk2C               = TNULL;
	m_pLastPoint           = TNULL;
	m_pCurrentPriorityLock = TNULL;
	m_iUnk64               = 0;

	if ( !m_lTargets.IsEmpty() )
		m_lFreeTargets.Splice( m_lFreeTargets.End(), m_lTargets );

	for ( TINT i = 0; i < NUM_GROUPS; i++ )
	{
		if ( !m_pGroups[ i ].m_lLockList.IsEmpty() )
			m_lFreeTargets.Splice( m_lFreeTargets.End(), m_pGroups[ i ].m_lLockList );
	}

	for ( TINT i = 0; i < NUM_TARGETS; i++ )
	{
		m_pTargets[ i ].m_uiFlags = 0;
		m_pTargets[ i ].m_eType   = -1;
	}

	m_vUnk124     = TVector4::VEC_ZERO;
	m_vUnk124.w   = 1.0f;
	m_vUnk134     = m_vUnk124;
	m_fUnk154     = 0.0f;
	m_fFlightTime = 0.0f;
	m_vUnk144     = m_vUnk124;

	if ( m_pScreenEffectInstance )
	{
		m_pScreenEffectInstance->StopInstant();
		m_pScreenEffectInstance = TNULL;
	}
}

// $deBlob: FUNCTION 00763f10
TBOOL ABlobZJumpComponent::PriorityTarget::GetHeight( TFLOAT* a_pfOutHeight, TFLOAT a_fRadius ) const
{
	if ( m_eType == PriorityTarget::TARGET_ENTITY )
	{
		ABaseEntity* pTarget = TREINTERPRETCAST( ABaseEntity*, m_pTarget );

		if ( pTarget != TNULL && ( !pTarget->IsPooled() || pTarget->GetPooledID() == m_iIndex ) )
		{
			*a_pfOutHeight = ABlobZJumpComponent::GetEntityHeight( pTarget, a_fRadius );
			return TTRUE;
		}
	}
	else if ( m_eType == PriorityTarget::TARGET_LMARK )
	{
		if ( m_pTarget != TNULL && m_iIndex > -1 )
		{
			*a_pfOutHeight = 0.0f;
			return TTRUE;
		}
	}
	else if ( m_eType == PriorityTarget::TARGET_INSTANCE && m_pTarget != TNULL )
	{
		*a_pfOutHeight = ABlobEntity::GetCommonProperties()->m_afUnk38[ 6 ] + a_fRadius; // 0x50
		return TTRUE;
	}

	*a_pfOutHeight = 0.0f;
	return TFALSE;
}

// $deBlob: FUNCTION 00765570
TVector4& ABlobZJumpComponent::GetTargetPosition( TVector4& a_rOutPosition ) const
{
	if ( m_pTargetNode == TNULL )
	{
		if ( m_pCurrentPriorityLock != TNULL )
		{
			m_pCurrentPriorityLock->GetPosition( a_rOutPosition );
			return a_rOutPosition;
		}

		a_rOutPosition = m_pOwner->GetBodyPosition();
		return a_rOutPosition;
	}

	// The node Blob jumps to: an instance's or an entity's position
	if ( m_pTargetNode->m_eType == AZJumpData::NodeType_Instance )
	{
		a_rOutPosition = m_pTargetNode->GetPosition();
		return a_rOutPosition;
	}

	if ( m_pTargetNode->m_eType == AZJumpData::NodeType_Entity )
	{
		a_rOutPosition = TSTATICCAST( ABaseEntity, m_pTargetNode->m_pInstance )->GetMatrix().GetTranslation();
		return a_rOutPosition;
	}

	return a_rOutPosition;
}

// $deBlob: FUNCTION 00765640
TFLOAT ABlobZJumpComponent::GetEntityHeight( ABaseEntity* a_pTarget, TFLOAT a_fRadius )
{
	if ( a_pTarget->IsA( &TGetClass( APropSwitchPressure ) ) && *TREINTERPRETCAST( TFLOAT*, TREINTERPRETCAST( TUINT8*, a_pTarget ) + 0x24 ) <= 0.8f )
		return 0.0f;

	if ( a_pTarget->IsA( &TGetClass( APropTrigger ) ) )
	{
		if ( ( *TREINTERPRETCAST( TUINT8*, TREINTERPRETCAST( TUINT8*, a_pTarget ) + 0x9C ) & 0x20 ) == 0 )
			return 0.0f;
	}
	else if ( a_pTarget->GetClass() == &TGetClass( ABoss ) )
	{
		return 0.0f;
	}

	TFLOAT fHeight = a_pTarget->GetAttackSlamHeight();

	if ( fHeight <= 0.0f )
		fHeight = ABlobEntity::GetCommonProperties()->m_afUnk38[ 6 ]; // 0x50

	return ( *TREINTERPRETCAST( TFLOAT*, TREINTERPRETCAST( TUINT8*, a_pTarget ) + 0x54 ) * 2.0f + a_fRadius ) + fHeight;
}

// $deBlob: FUNCTION 007655e0
TFLOAT ABlobZJumpComponent::GetTargetHeight() const
{
	ABlobEntity* pOwner = m_pOwner;

	if ( m_pTargetNode == TNULL )
	{
		if ( m_pCurrentPriorityLock != TNULL )
		{
			TFLOAT fHeight = 0.0f;
			m_pCurrentPriorityLock->GetHeight( &fHeight, pOwner->GetBoundingSphereRadius() );
			return fHeight;
		}

		return 0.0f;
	}

	// m_pTargetNode is the landmark or instance point: its type is at +0, the entity at +0xC
	TUINT32* puPoint = TREINTERPRETCAST( TUINT32*, m_pTargetNode );

	if ( puPoint[ 0 ] == 2 )
		return GetEntityHeight( TREINTERPRETCAST( ABaseEntity*, puPoint[ 3 ] ), pOwner->GetBoundingSphereRadius() );

	return 0.0f;
}

// $deBlob: FUNCTION 007656f0
TBOOL ABlobZJumpComponent::CanJumpToLandmark() const
{
	PriorityTarget* pLock = m_pCurrentPriorityLock;

	if ( ( pLock->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE ) != 0 || pLock->m_eType != PriorityTarget::TARGET_LMARK ||
	     ( TSTATICCAST( ALandmarkInstance, pLock->m_pTarget )->GetBuildingFlags() & ALandmarkInstance::Flags_TriggersHidden ) != 0 ||
	     TUINT( m_pOwner->m_iPaintPoints ) < 1 )
		return TFALSE;

	TUINT32 uiColour = m_pOwner->m_pColourComponent->GetCurrentColour();

	if ( ( uiColour & 0x10 ) == 0 && uiColour != 0 && uiColour < 0x10 )
	{
		TUINT32 uiTriggerColour = TSTATICCAST( ALandmarkInstance, pLock->m_pTarget )->GetTriggerColour( pLock->m_iIndex );

		if ( uiTriggerColour == 0 || uiTriggerColour == uiColour || ( TUINT8( m_pOwner->m_uiFlags >> 0x15 ) & 1 ) != 0 )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00765760
TBOOL ABlobZJumpComponent::HasValidPriorityLock() const
{
	PriorityTarget* pLock = m_pCurrentPriorityLock;

	return pLock != TNULL && ( pLock->m_uiFlags & 0x20 ) == 0 && TUINT32( pLock->m_eType ) <= 2 && pLock->m_pTarget != TNULL;
}

// $deBlob: FUNCTION 00765a90
TBOOL ABlobZJumpComponent::OnBlobJump( TFLOAT a_fFlightTime )
{
	TBOOL bAim = TFALSE;

	switch ( m_eState )
	{
		case STATE_POINT:
			if ( m_pTargetNode->m_eType == AZJumpData::NodeType_Entity )
			{
				// The point's next node is an entity: the attack goes at it (state 9) as a target of priority 0x25
				m_pStandNode = m_pCurrentNode;

				PriorityTarget* pTarget  = AllocTarget( 0.0f );
				ABaseEntity*    pEntity  = TSTATICCAST( ABaseEntity, m_pTargetNode->m_pInstance );
				pTarget->m_eType         = PriorityTarget::TARGET_ENTITY;
				pTarget->m_pTarget       = pEntity;
				pTarget->m_iIndex        = pEntity->IsPooled() ? pEntity->GetPooledID() : -1;
				pTarget->m_ePriorityType = 0x25;
				pTarget->m_iPriority     = GetPriority( 0x25, m_pOwner );
				pTarget->m_uiFlags       = PriorityTarget::FLAGS_LOCKED | PriorityTarget::FLAGS_UNK100;
				pTarget->m_fDistance     = 0.0f;

				PriorityGroup* pGroup = &m_pGroups[ 0x25 ];
				InsertGroupTarget( pGroup->m_lLockList, pTarget );

				if ( !pGroup->IsLinked() )
					InsertGroup( m_lGroups, pGroup );

				m_pCurrentPriorityLock = pTarget;
				m_pCurrentNode         = TNULL;
				m_pTargetNode          = TNULL;
				SetState( STATE_ATTACKFLIGHT );
				bAim = TTRUE;
				break;
			}

			// Not at an entity: the Z-jump to the next node
			TFALLTHROUGH;
		case STATE_AIMNODE:
			SetState( STATE_NODEFLIGHT );
			break;
		case STATE_AIM:
			m_pOwner->GetSoundComponent()->PlayZJumpLand();
			SetState( STATE_NONE );
			break;
		case STATE_LOCK:
		case STATE_LOCKLMARK:
		{
			PriorityTarget* pLock = m_pCurrentPriorityLock;

			if ( pLock == TNULL || ( pLock->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE ) != 0 )
				return TFALSE;

			HideGroupMarkers();

			TBOOL bAttack = TFALSE;

			if ( pLock->m_eType == PriorityTarget::TARGET_ENTITY )
			{
				ABaseEntity* pTarget = TSTATICCAST( ABaseEntity, pLock->m_pTarget );

				if ( pTarget->IsA( &TGetClass( APropSwitchPressure ) ) )
				{
					// A pressure switch that is still up: Blob slams it (state 12)
					if ( 0.8f < *TREINTERPRETCAST( TFLOAT*, TREINTERPRETCAST( TUINT8*, pTarget ) + 0x24 ) )
						bAttack = TTRUE;
					else
						SetState( STATE_SWITCHSLAM );
				}
				else
				{
					APropTrigger* pTrigger = TSTATICCAST( APropTrigger, pTarget );

					if ( !pTarget->IsA( &TGetClass( APropTrigger ) ) )
					{
						bAttack = TTRUE;
					}
					else
					{
						TASSERT( pTrigger == TNULL || pTrigger->IsA( &TGetClass( APropTrigger ) ) );

						if ( ( ( *( TREINTERPRETCAST( TUINT8*, pTrigger ) + 0x9C ) >> 5 ) & 1 ) != 0 )
							bAttack = TTRUE;
						else
							SetState( STATE_SWITCHSLAM );
					}
				}
			}
			else if ( pLock->m_eType == PriorityTarget::TARGET_LMARK )
			{
				SetState( STATE_LMARKFLIGHT );
				break;
			}
			else if ( pLock->m_eType == PriorityTarget::TARGET_INSTANCE )
			{
				bAttack = TTRUE;
			}
			else
			{
				break;
			}

			if ( bAttack )
			{
				SetState( STATE_ATTACKFLIGHT );
				m_pOwner->ClearZPlayerAttack();
			}

			bAim = TTRUE;
			break;
		}
	}

	if ( bAim )
	{
		// The point the attack aims at, its start and Blob's position
		GetTargetPosition( m_vUnk124 );
		m_vUnk124.y = m_vUnk124.y - GetTargetHeight();
		m_vUnk134   = m_vUnk124;
		m_vUnk144   = m_pOwner->GetBodyPosition();
	}

	m_fFlightTime = a_fFlightTime;
	m_fUnk154     = 0.0f;
	return TTRUE;
}

// $deBlob: FUNCTION 00766920
void ABlobZJumpComponent::OnColourChanged()
{
	if ( m_eState == STATE_POINT || m_eState == STATE_LOCK || m_eState == STATE_LOCKLMARK )
		UpdateLockElement( TTRUE );
}

// $deBlob: FUNCTION 007669a0
TBOOL ABlobZJumpComponent::IsLockedOnLevelCompleteTrigger() const
{
	if ( m_pCurrentPriorityLock != TNULL && m_pCurrentPriorityLock->m_eType == PriorityTarget::TARGET_ENTITY )
	{
		ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, m_pCurrentPriorityLock->m_pTarget );

		if ( pEntity->GetClass() == &TGetClass( APropTrigger ) )
		{
			APropTrigger* pTrigger = TSTATICCAST( APropTrigger, pEntity );

			if ( pTrigger->GetPaintPoints() != -1 && pTrigger->TargetsLevelComplete() )
				return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00766a20
void ABlobZJumpComponent::SetState( TINT a_eState )
{
	// Leaving the Z-jump of a point (states 1-4): its effect comes back
	if ( ( STATE_NONE < m_eState && m_eState < STATE_ATTACKSLAM ) && ( a_eState < STATE_POINT || STATE_AIMNODE < a_eState ) && m_pPoint != TNULL )
	{
		TINT iIndex = AZJumpData::GetSingleton()->GetPointIndex( m_pPoint );
		TASSERT( -1 < iIndex );
		AZJumpData::GetSingleton()->SetPointEffect( iIndex, TTRUE );
	}

	if ( a_eState == STATE_POINT || a_eState == STATE_AIM )
	{
		// The screen effect at the next node's instance (its rows 1 and 2 swapped)
		if ( m_pTargetNode->m_eType == AZJumpData::NodeType_Instance )
		{
			if ( m_pScreenEffectInstance == TNULL )
				m_pScreenEffectInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pScreenEffect );

			const TMatrix44& rInstance = **TREINTERPRETCAST( TMatrix44**, m_pTargetNode->m_pInstance );

			TASSERT( rInstance.m_f14 == 0.0f );
			TASSERT( rInstance.m_f34 == 0.0f );
			TASSERT( rInstance.m_f24 == 0.0f );

			TVector4 vRow0 = rInstance.AsBasisVector4( BASISVECTOR_RIGHT );
			TVector4 vRow2 = rInstance.AsBasisVector4( BASISVECTOR_FORWARD );
			TVector4 vRow1 = rInstance.AsBasisVector4( BASISVECTOR_UP );
			TVector4 vPos  = m_pTargetNode->GetPosition();

			TMatrix44& rEffect = m_pScreenEffectInstance->GetTransform();
			rEffect.SetBasisVector4( BASISVECTOR_RIGHT, vRow0 );
			rEffect.SetBasisVector4( BASISVECTOR_UP, vRow2 );
			rEffect.SetBasisVector4( BASISVECTOR_FORWARD, vRow1 );
			rEffect.SetBasisVector4( BASISVECTOR_TRANSLATION, vPos );
		}
	}
	else if ( a_eState != STATE_AIMNODE && m_pScreenEffectInstance )
	{
		m_pScreenEffectInstance->StopInstant();
		m_pScreenEffectInstance = TNULL;
	}

	switch ( a_eState )
	{
		case STATE_NONE:
			m_pPoint       = TNULL;
			m_pCurrentNode = TNULL;
			m_pTargetNode  = TNULL;

			if ( TXUIControl* pLockElement = TSTATICCAST( TXUIControl, m_pLockElement ) )
			{
				pLockElement->SetVisible( TFALSE );
				pLockElement->GetVisual()->StopAnimation();
			}

			break;
		case STATE_POINT:
			TASSERT( m_pPoint );
			AZJumpData::GetSingleton()->SetPointEffect( AZJumpData::GetSingleton()->GetPointIndex( m_pPoint ), TFALSE );
			break;
		case STATE_SETEND:
		case STATE_TIMEOUT:
			m_pCurrentNode = TNULL;
			m_pTargetNode  = TNULL;
			break;
		case STATE_ATTACKFLIGHT:
			m_pPoint = TNULL;
			break;
	}

	m_eState = a_eState;
}

// $deBlob: FUNCTION 007699f0
void ABlobZJumpComponent::FreeTarget( PriorityTarget& a_rTarget )
{
	TASSERT( !a_rTarget.IsLinked() );
	a_rTarget.m_uiFlags = 0;
	a_rTarget.m_eType   = -1;
	a_rTarget.m_pRectangle->SetVisible( TFALSE );
	m_lFreeTargets.PushBack( &a_rTarget );
}

// $deBlob: FUNCTION 0076a6b0
void ABlobZJumpComponent::FreeTargets()
{
	while ( !m_lTargets.IsEmpty() )
		FreeTarget( *m_lTargets.PopFront() );
}

// $deBlob: FUNCTION 00769a60
void ABlobZJumpComponent::FreeGroups()
{
	while ( !m_lGroups.IsEmpty() )
	{
		PriorityGroup* pGroup = m_lGroups.PopFront();

		while ( !pGroup->m_lLockList.IsEmpty() )
			FreeTarget( *pGroup->m_lLockList.PopFront() );
	}
}

// $deBlob: FUNCTION 00769ba0
void ABlobZJumpComponent::HideGroupMarkers()
{
	for ( TINT i = 0; i < NUM_GROUPS; i++ )
	{
		T2DList<PriorityTarget>& rList = m_pGroups[ i ].m_lLockList;

		if ( !rList.IsEmpty() )
		{
			for ( auto it = rList.Begin(); it != rList.End(); it++ )
				it->m_pRectangle->SetVisible( TFALSE );
		}
	}
}

// $deBlob: FUNCTION 007664c0
void ABlobZJumpComponent::OnInkBounce()
{
	if ( m_eState == STATE_AIM || m_eState == STATE_AIMNODE )
	{
		m_pOwner->GetSoundComponent()->PlayZJumpLand();
		SetState( STATE_NONE );
		ABlobHud::GetSingleton()->GetHoldTimeline()->SetVisible( TFALSE );

		if ( TXUIControl* pLockElement = TSTATICCAST( TXUIControl, m_pLockElement ) )
		{
			pLockElement->SetVisible( TFALSE );
			pLockElement->GetVisual()->StopAnimation();
		}
	}
}

// $deBlob: FUNCTION 00765920
void ABlobZJumpComponent::Cancel()
{
	switch ( m_eState )
	{
		case STATE_AIMNODE:
			SetState( STATE_AIM );

			if ( TXUIControl* pLockElement = TSTATICCAST( TXUIControl, m_pLockElement ) )
			{
				pLockElement->SetVisible( TFALSE );
				pLockElement->GetVisual()->StopAnimation();
			}

			HideGroupMarkers();
			return;
		case STATE_LOCK:
		case STATE_LOCKLMARK:
			m_iUnk64 = 0;
			FreeTargets();

			if ( TXUIControl* pLockElement = TSTATICCAST( TXUIControl, m_pLockElement ) )
			{
				pLockElement->SetVisible( TFALSE );
				pLockElement->GetVisual()->StopAnimation();
			}

			m_fKeepTargetsTimer = 0.5f;
			TFALLTHROUGH;
		case STATE_POINT:
			SetState( STATE_NONE );
			m_pOwner->GetSoundComponent()->StopZJumpEvent();
			HideGroupMarkers();
			return;
		default:
			HideGroupMarkers();
			return;
	}
}

// $deBlob: FUNCTION 00766300
void ABlobZJumpComponent::End()
{
	Cancel();

	// NOTE: m_eState is the one Cancel left
	switch ( m_eState )
	{
		case STATE_LOCK:
		case STATE_LOCKLMARK:
			FreeTargets();
			FreeGroups();
			TFALLTHROUGH;
		case STATE_NODEFLIGHT:
		case STATE_AIM:
			SetState( STATE_NONE );
			return;
		case STATE_ATTACKFLIGHT:
			SetState( STATE_NONE );
			m_pOwner->ClearZPlayerAttack();
			return;
		default:
			return;
	}
}

// $deBlob: FUNCTION 00766940
void ABlobZJumpComponent::OnSizeChanged()
{
	if ( m_eState == STATE_POINT || m_eState == STATE_LOCK || m_eState == STATE_LOCKLMARK )
		UpdateLockElement( TFALSE );
}

static void UnpackARGB( TUINT32 a_uiColour, TUINT32* a_pOutR, TUINT32* a_pOutG, TUINT32* a_pOutB, TUINT32* a_pOutA )
{
	*a_pOutB = a_uiColour & 0xFF;
	*a_pOutG = ( a_uiColour >> 8 ) & 0xFF;
	*a_pOutR = ( a_uiColour >> 16 ) & 0xFF;
	*a_pOutA = a_uiColour >> 24;
}

// The trigger colours whose number is shown white (the dark colours)
static TBOOL IsDarkColour( TUINT32 a_uiColour )
{
	return a_uiColour == 7 || a_uiColour == 0x10 || a_uiColour == 4 || a_uiColour == 2 || a_uiColour == 3;
}

// The frame of m_pEnergyFill for a_iPaint paint points (1, 100 and 200 points are named frames)
static TFLOAT GetEnergyFrame( TINT a_iPaint, TFLOAT a_f1Point, TFLOAT a_f100Point, TFLOAT a_f200Point )
{
	if ( a_iPaint == 1 )
		return a_f1Point;

	if ( a_iPaint < 101 )
		return ( a_f100Point - a_f1Point ) * ( TFLOAT( a_iPaint ) / 100.0f ) + a_f1Point;

	return ( a_f200Point - a_f100Point ) * ( TFLOAT( a_iPaint - 100 ) / 100.0f ) + a_f100Point;
}

// $deBlob: FUNCTION 00766d20
void ABlobZJumpComponent::UpdateLockElement( TBOOL a_bNoTimeline )
{
	// The channels of the lock colour (retail keeps them in statics)
	static TUINT32 s_uiR;
	static TUINT32 s_uiG;
	static TUINT32 s_uiB;
	static TUINT32 s_uiA;

	m_wszLockText[ 0 ] = L'\0';

	const TWCHAR* wszStart = L"Lock";
	const TWCHAR* wszEnd   = L"EndLock";

	if ( m_pTargetNode != TNULL )
	{
		wszStart = L"LockZJump";
		wszEnd   = L"EndLockZJump";
	}
	else if ( PriorityTarget* pLock = m_pCurrentPriorityLock )
	{
		if ( pLock->m_eType == PriorityTarget::TARGET_ENTITY )
		{
			ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, pLock->m_pTarget );

			if ( pEntity != TNULL )
			{
				TBOOL bLeechbot = pEntity->IsA( &TGetClass( ALeechBot2 ) );

				if ( !bLeechbot ||
				     ( !ALeechBot2::IsFixedColourType( TUINT8( TSTATICCAST( ALeechBot2, pEntity )->GetLeechbotType() ) ) && TSTATICCAST( ALeechBot2, pEntity )->GetTankFill() < 1.0f ) )
				{
					TBOOL bBigPaintBot = pEntity->IsA( &TGetClass( ABigPaintBot ) );

					if ( bBigPaintBot )
					{
						// A big paint bot: the arrows in its colour (single player) or Blob's start colour
						wszStart = L"LockLeechbot";
						wszEnd   = L"EndLockLeechbot";

						TUINT32 uiColour;

						if ( TUINT32( AGameStatus::GetSingleton()->GetMultiPlayerGameType() ) > 3 )
							m_pOwner->m_pColourComponent->GetMixedColour( uiColour, *( TREINTERPRETCAST( TUINT8*, pEntity ) + 0x9C ) );
						else
							uiColour = ABlobColourComponent::ms_auiColours[ m_pOwner->m_pColourComponent->GetStartColour() ];

						m_pLeftArrowColour->SetColour( uiColour );
						m_pRightArrowColour->SetColour( uiColour );
					}
					else if ( pEntity->IsA( &TGetClass( ACar ) ) && TSTATICCAST( ACar, pEntity )->GetCarType() == 7 )
					{
						// A paint tanker: the arrows in its paint
						TUINT32 uiColour;

						switch ( TSTATICCAST( ACar, pEntity )->GetTankerStage() )
						{
							case 0:
								m_pOwner->m_pColourComponent->GetMixedColour( uiColour, 4 );
								break;
							case 1:
								m_pOwner->m_pColourComponent->GetMixedColour( uiColour, 2 );
								break;
							case 2:
								m_pOwner->m_pColourComponent->GetMixedColour( uiColour, 1 );
								break;
							default:
								uiColour = 0;
								break;
						}

						m_pLeftArrowColour->SetColour( uiColour );
						m_pRightArrowColour->SetColour( uiColour );
						wszStart = L"LockLeechbot";
						wszEnd   = L"EndLockLeechbot";
					}
					else if ( pEntity->GetMaxLife() == 0 )
					{
						// Nothing to paint: the passive lock in Blob's colour
						TUINT32 uiColour = m_pOwner->m_pColourComponent->GetCurrentColour();

						if ( uiColour & 0x10 )
							uiColour = 0x10;

						m_pPassiveFill->SetColour( ABlobColourComponent::ms_auiColours[ uiColour ] );
						wszStart = L"PassiveLock";
						wszEnd   = L"EndPassiveLock";
					}
					else
					{
						// The paint the entity needs, crossed when Blob can't Z-jump at it
						TUINT32 uiARGB      = 0xFFB6B6AB;
						TBOOL   bDark       = TFALSE;
						TBOOL   bCrossed    = ( TUINT8( m_pCurrentPriorityLock->m_uiFlags >> 5 ) & 1 ) != 0; // FLAGS_UNLOCKABLE
						TBOOL   bCheckCross = TTRUE;

						if ( !pEntity->IsFlagged( ABaseEntity::FLAG_Player ) )
						{
							if ( m_pOwner->m_iPaintPoints == 0 && ( m_pCurrentPriorityLock->m_uiFlags & PriorityTarget::FLAGS_UNK100 ) == 0 )
								bCrossed = TTRUE;

							if ( pEntity->GetClass() == &TGetClass( AInky_Baton_Elite ) )
							{
								// An elite baton: its colour, crossed unless Blob has it
								TUINT32 uiColour = TSTATICCAST( AInky_Baton, pEntity )->GetBatonType();
								TASSERT( uiColour != ABlobColourComponent::COLOUR_WHITE );

								uiARGB = ABlobColourComponent::ms_auiColours[ uiColour ];

								if ( IsDarkColour( uiColour ) )
									bDark = TTRUE;

								TUINT32 uiBlobColour = m_pOwner->m_pColourComponent->GetCurrentColour();

								if ( uiBlobColour & 0x10 )
									uiBlobColour = 0x10;

								if ( TSTATICCAST( AInky_Baton, pEntity )->GetBatonType() == uiBlobColour )
									bCrossed = TFALSE;
								else
									bCrossed = ( TUINT8( m_pOwner->m_uiFlags >> 0x15 ) & 1 ) == 0;

								m_pCannotLock->SetColour( uiARGB );
							}
							else
							{
								TUINT32 uiColour = m_pOwner->m_pColourComponent->GetCurrentColour();

								if ( uiColour & 0x10 )
								{
									uiColour = 0x10;
									bDark    = TTRUE;
								}
								else if ( IsDarkColour( uiColour ) )
								{
									bDark = TTRUE;
								}

								uiARGB = ABlobColourComponent::ms_auiColours[ uiColour ];
							}
						}
						else
						{
							// Another Blob: crossed when Blob can't ink it
							if ( !m_pOwner->CanInkBlob( TSTATICCAST( ABlobEntity, pEntity ) ) )
							{
								bCrossed    = TTRUE;
								bCheckCross = TFALSE;
							}
						}

						if ( bCheckCross && !bCrossed )
							m_pCannotTarget->PlayAnimation( L"CrossOff", L"CrossOff" );
						else
							m_pCannotTarget->PlayAnimation( L"CrossOnLoop", L"EndCrossOnLoop" );

						if ( pEntity->GetMaxLife() == 1 )
						{
							TStringManager::String16Format( m_wszScoreText, TARRAYSIZE( m_wszScoreText ), L"%d", pEntity->GetMaxLife() );
							TSTATICCAST( TXUIVisual, m_pEnergyFill )->GetTimelineControl()->SetTimeLineTime( m_fEnergy1Point, TTRUE );
						}
						else
						{
							// The paint it needs (m_iLife)
							TINT16 iPaint = pEntity->GetLife();
							TFLOAT fFrame;

							if ( iPaint < 101 )
								fFrame = ( m_fEnergy100Point - m_fEnergy1Point ) * ( TFLOAT( TINT( iPaint ) ) / 100.0f ) + m_fEnergy1Point;
							else
								fFrame = ( TFLOAT( iPaint - 100 ) / 100.0f ) * ( m_fEnergy200Point - m_fEnergy100Point ) + m_fEnergy100Point;

							TSTATICCAST( TXUIVisual, m_pEnergyFill )->GetTimelineControl()->SetTimeLineTime( fFrame, TTRUE );
							TStringManager::String16Format( m_wszScoreText, TARRAYSIZE( m_wszScoreText ), L"%d", TINT( pEntity->GetLife() ) );
						}

						for ( TINT i = 0; i < TARRAYSIZE( m_apPaintMeters ); i++ )
						{
							TXUIElement* pMeter = m_apPaintMeters[ i ];

							if ( pMeter->IsVisible() )
								pMeter->SetColour( uiARGB );
						}

						m_pCircleAbove->SetColour( uiARGB );

						if ( !bDark )
						{
							m_pScore->SetText( m_wszScoreText, -1 );
							m_pScore->SetVisible( TTRUE );
						}
						else
						{
							m_pScore->SetVisible( TFALSE );
							m_pScoreWhite->SetText( m_wszScoreText, -1 );
						}

						m_pScoreWhite->SetVisible( bDark );
					}
				}
				else
				{
					// A full leechbot: the arrows in its paint (single player) or Blob's start colour
					wszStart = L"LockLeechbot";
					wszEnd   = L"EndLockLeechbot";

					TUINT32 uiColour;

					if ( TUINT32( AGameStatus::GetSingleton()->GetMultiPlayerGameType() ) < 4 )
						uiColour = ABlobColourComponent::ms_auiColours[ m_pOwner->m_pColourComponent->GetStartColour() ];
					else
						m_pOwner->m_pColourComponent->GetMixedColour( uiColour, TSTATICCAST( ALeechBot2, pEntity )->GetPaintColour() );

					m_pLeftArrowColour->SetColour( uiColour );
					m_pRightArrowColour->SetColour( uiColour );
				}
			}
		}
		else if ( pLock->m_eType == PriorityTarget::TARGET_LMARK )
		{
			// A landmark trigger: the paint it still needs, in its colour (or Blob's), crossed when Blob can't paint it
			ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pLock->m_pTarget );
			TUINT32            uiColour  = pLandmark->GetTriggerColour( pLock->m_iIndex );

			if ( uiColour == 0 )
			{
				uiColour = m_pOwner->m_pColourComponent->GetCurrentColour();

				if ( uiColour & 0x10 )
					uiColour = 0x10;
			}

			UnpackARGB( ABlobColourComponent::ms_auiColours[ uiColour ], &s_uiR, &s_uiG, &s_uiB, &s_uiA );
			TUINT32 uiARGB = ( ( ( ( s_uiA << 8 ) | s_uiR ) << 8 ) | s_uiG ) << 8 | s_uiB;

			if ( ( pLandmark->GetBuildingFlags() & ALandmarkInstance::Flags_TriggersHidden ) == 0 )
			{
				TUINT32      uiTriggerColour = pLandmark->GetTriggerColour( m_pCurrentPriorityLock->m_iIndex );
				TBOOL        bCanPaint       = TFALSE;
				ABlobEntity* pOwner          = m_pOwner;

				if ( pOwner->m_iPaintPoints != 0 )
				{
					TUINT32 uiBlobColour = pOwner->m_pColourComponent->GetCurrentColour();

					if ( uiBlobColour & 0x10 )
						uiBlobColour = 0x10;

					if ( ( TUINT8( pOwner->m_uiFlags >> 0x15 ) & 1 ) == 0 )
					{
						if ( uiTriggerColour == 0 )
						{
							if ( uiBlobColour - 1 < 0xF )
								bCanPaint = TTRUE;
						}
						else
						{
							bCanPaint = uiBlobColour == uiTriggerColour;
						}
					}
					else
					{
						bCanPaint = TTRUE;
					}
				}

				TINT iPaintLeft = pLandmark->GetTriggerPaintLeft( m_pCurrentPriorityLock->m_iIndex );
				TSTATICCAST( TXUIVisual, m_pEnergyFill )->GetTimelineControl()->SetTimeLineTime( GetEnergyFrame( iPaintLeft, m_fEnergy1Point, m_fEnergy100Point, m_fEnergy200Point ), TTRUE );

				for ( TINT i = 0; i < TARRAYSIZE( m_apPaintMeters ); i++ )
				{
					TXUIElement* pMeter = m_apPaintMeters[ i ];

					if ( pMeter->IsVisible() )
						pMeter->SetColour( uiARGB );
				}

				m_pCircleAbove->SetColour( uiARGB );
				TStringManager::String16Format( m_wszScoreText, TARRAYSIZE( m_wszScoreText ), L"%d", iPaintLeft );

				TXUIControl* pScore;

				if ( IsDarkColour( uiColour ) )
				{
					m_pScore->SetVisible( TFALSE );
					m_pScoreWhite->SetText( m_wszScoreText, -1 );
					pScore = m_pScoreWhite;
				}
				else
				{
					m_pScoreWhite->SetVisible( TFALSE );
					m_pScore->SetText( m_wszScoreText, -1 );
					pScore = m_pScore;
				}

				pScore->SetVisible( TTRUE );

				if ( !bCanPaint )
					m_pCannotTarget->PlayAnimation( L"CrossOnLoop", L"EndCrossOnLoop" );
				else
					m_pCannotTarget->PlayAnimation( L"CrossOff", L"CrossOff" );
			}
			else
			{
				m_pCannotLock->SetColour( uiARGB );
				wszStart = L"CannotTarget";
				wszEnd   = L"EndCannotTarget";
			}
		}
		else if ( pLock->m_eType == PriorityTarget::TARGET_INSTANCE )
		{
			m_pCannotTarget->PlayAnimation( L"CrossOff", L"CrossOff" );

			TUINT32 uiColour = m_pOwner->m_pColourComponent->GetCurrentColour();

			if ( uiColour & 0x10 )
				uiColour = 0x10;

			m_pPassiveFill->SetColour( ABlobColourComponent::ms_auiColours[ uiColour ] );
			wszStart = L"PassiveLock";
			wszEnd   = L"EndPassiveLock";
		}
	}


	if ( m_pTargetNode == TNULL && m_pCurrentPriorityLock == TNULL )
	{
		if ( m_pLockElement != TNULL )
			m_pLockElement->SetVisible( TFALSE );
	}
	else if ( m_pLockElement != TNULL )
	{
		m_pLockElement->SetVisible( TTRUE );
		m_pLockElement->SetPresenterText( m_wszLockText, 0, -1 );

		if ( !a_bNoTimeline )
			m_pLockElement->GetVisual()->PlayAnimation( wszStart, wszEnd );
	}
}

// $deBlob: FUNCTION 007669f0
void ABlobZJumpComponent::OnEntityRemoved( ABaseEntity* a_pEntity )
{
	if ( m_pCurrentPriorityLock && m_pCurrentPriorityLock->m_eType == PriorityTarget::TARGET_ENTITY && m_pCurrentPriorityLock->m_pTarget == a_pEntity )
		m_pCurrentPriorityLock = TNULL;
}

// $deBlob: FUNCTION 00766420
void ABlobZJumpComponent::OnRecoil()
{
	if ( m_eState == STATE_ATTACKSLAM || m_eState == STATE_ATTACKFLIGHT )
	{
		// After an attack on a Z-jump instance, its point isn't found again until Blob leaves it (m_pLastPoint)
		PriorityTarget* pLock = m_pCurrentPriorityLock;

		if ( pLock != TNULL && pLock->m_eType == PriorityTarget::TARGET_INSTANCE && pLock->m_ePriorityType == 0x25 )
		{
			TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, pLock->m_pTarget );

			if ( ( pInstance->m_uiFlags & 7 ) == 7 && -1 < pInstance->zjump.m_iSet && -1 < pInstance->zjump.m_iNode )
			{
				AZJumpData::Set* pSet = AZJumpData::GetSingleton()->GetSet( pInstance->zjump.m_iSet );
				m_pLastPoint          = pSet->FindPoint( &AZJumpData::GetSingleton()->GetSet( pInstance->zjump.m_iSet )->m_pNodes[ pInstance->zjump.m_iNode ] );
			}
		}

		SetState( STATE_NONE );
	}
}

// $deBlob: FUNCTION 00766520
void ABlobZJumpComponent::EndCharge()
{
	if ( m_eState == STATE_NODEFLIGHT || m_eState == STATE_AIMNODE )
	{
		m_pOwner->m_pSoundComponent->PlayZJumpLand();
		SetState( STATE_NONE );
		ABlobHud::GetSingleton()->GetHoldTimeline()->SetVisible( TFALSE );

		if ( TXUIControl* pLockElement = TSTATICCAST( TXUIControl, m_pLockElement ) )
		{
			pLockElement->SetVisible( TFALSE );
			pLockElement->GetVisual()->StopAnimation();
		}
	}

	TASSERT( m_eState != STATE_ATTACKFLIGHT );
	TASSERT( m_eState != STATE_ATTACKSLAM );
}

// $deBlob: FUNCTION 007665e0
void ABlobZJumpComponent::SetContactBody( Toshi::TPhysicsObjectContainer* a_pBody )
{
	m_uiUnk10 = a_pBody ? TUINT32( a_pBody->m_pUserData ) : 0;
}

// $deBlob: FUNCTION 00763d20
TBOOL ABlobZJumpComponent::PriorityTarget::CanPaint( ABlobEntity* a_pBlob ) const
{
	if ( m_uiFlags & FLAGS_PAINTABLE )
	{
		TUINT32 uiColour = m_uiFlags & 0xF;

		if ( uiColour == 0 )
		{
			// Any colour but the ink
			TUINT32 uiBlobColour = a_pBlob->m_pColourComponent->GetCurrentColour();

			if ( ( TUINT8( uiBlobColour >> 4 ) & 1 ) || uiBlobColour == 0 )
				return TFALSE;
		}
		else if ( ( TUINT8( a_pBlob->m_uiFlags >> 0x15 ) & 1 ) == 0 )
		{
			TUINT32 uiBlobColour = a_pBlob->m_pColourComponent->GetCurrentColour();

			if ( uiBlobColour & 0x10 )
				uiBlobColour = 0x10;

			if ( uiColour != uiBlobColour )
				return TFALSE;
		}
	}

	return TTRUE;
}

// The position of the target itself (an entity's matrix, a landmark trigger, an instance's matrix)
// $deBlob: FUNCTION 00763d80
TBOOL ABlobZJumpComponent::PriorityTarget::GetBasePosition( TVector4& a_rOutPosition ) const
{
	if ( m_eType == TARGET_ENTITY )
	{
		ABaseEntity* pTarget = TSTATICCAST( ABaseEntity, m_pTarget );

		if ( pTarget != TNULL && ( !pTarget->IsPooled() || pTarget->GetPooledID() == m_iIndex ) )
		{
			a_rOutPosition = pTarget->GetMatrix().GetTranslation();
			return TTRUE;
		}
	}
	else if ( m_eType == TARGET_LMARK )
	{
		if ( m_pTarget != TNULL && m_iIndex > -1 )
		{
			TSTATICCAST( ALandmarkInstance, m_pTarget )->GetTriggerPosition( m_iIndex, a_rOutPosition );
			return TTRUE;
		}
	}
	else if ( m_eType == TARGET_INSTANCE && m_pTarget != TNULL )
	{
		a_rOutPosition = TSTATICCAST( TerrainTypes::CellInstance, m_pTarget )->m_pMatrix->GetTranslation();
		return TTRUE;
	}

	a_rOutPosition = TVector4::VEC_ZERO; // X0Y0Z0W1
	return TFALSE;
}

// $deBlob: FUNCTION 007657a0
void ABlobZJumpComponent::StartLock()
{
	if ( !AZJumpData::GetSingleton() || m_pOwner->m_bInBlobVision != 0 )
		return;

	m_fSearchTimer = 0.0f;

	if ( m_eState == STATE_NONE )
	{
		AZJumpData::Point* pPoint = FindPoint();

		if ( pPoint != TNULL )
		{
			m_pPoint       = pPoint;
			m_pCurrentNode = pPoint->m_pNode;
			m_bUnk120      = pPoint->m_iUnk08 == 1;
			TASSERT( pPoint->m_pSet->m_iNodeCount > 1, "Every Z-Jump set must have at least 2 nodes!" );
			m_pTargetNode = pPoint->m_pNode->m_apLinks[ 0 ];

			if ( IsPointWithoutEntities( m_pPoint ) == TTRUE )
			{
				m_pOwner->GetSoundComponent()->OnZJumpPoint();
				m_pOwner->GetSoundComponent()->StartZJumpPointLoop();
				SetState( STATE_POINT );
				return;
			}

			m_pOwner->GetSoundComponent()->PlayLockOn();
			SetState( STATE_POINT );
			return;
		}

		if ( m_fKeepTargetsTimer <= 0.0f )
		{
			m_pCurrentPriorityLock = TNULL;
			FreeTargets();
			FreeGroups();
		}

		if ( m_lTargets.IsEmpty() )
		{
			PruneGroups();
			FindNodeTargets();
			FindEntityTargets();
			FindLandmarkTargets();
			m_fSearchTimer = 0.3f;
		}

		SelectLock();
		SetState( ( m_pCurrentPriorityLock == TNULL || m_pCurrentPriorityLock->m_eType != PriorityTarget::TARGET_LMARK ) ? STATE_LOCK : STATE_LOCKLMARK );
		m_bLockFound = m_pCurrentPriorityLock != TNULL;
	}
	else if ( m_eState == STATE_AIM && m_pTargetNode != TNULL )
	{
		m_pOwner->GetSoundComponent()->PlayLockAim();
		SetState( STATE_AIMNODE );
	}
}

// $deBlob: FUNCTION 00765d10
void ABlobZJumpComponent::OnLanded()
{
	if ( !AZJumpData::GetSingleton() )
		return;

	switch ( m_eState )
	{
		case STATE_NODEFLIGHT:
		{
			// The node Blob jumped to
			AZJumpData::Node* pNode  = m_pTargetNode;
			TBOOL             bEnded = TFALSE;
			TBOOL             bCheck = TTRUE;
			m_pTargetNode            = TNULL;

			ABlobEntity* pOwner    = m_pOwner;
			TFLOAT       fDistance = TVector4::Distance( pNode->GetPosition(), pOwner->GetBodyPosition() ) - pOwner->GetBoundingSphereRadius();

			if ( 3.0f > fDistance )
			{
				m_pLastPoint = pNode->m_pSet->FindPoint( pNode );

				if ( m_pLastPoint != TNULL )
				{
					// Blob is at a point: the Z-jump ends there
					bEnded = TTRUE;
					bCheck = TFALSE;
				}
				else
				{
					// The end of the set: no link that doesn't lead back
					bEnded = TTRUE;
					bCheck = TFALSE;

					for ( TINT i = 0; i < 2; i++ )
					{
						if ( pNode->m_apLinks[ i ] != TNULL && pNode->m_apLinks[ i ] != m_pCurrentNode )
						{
							bEnded = TFALSE;
							bCheck = TTRUE;
							break;
						}
					}
				}
			}

			// Blob is on the instance of the node: the Z-jump goes on to the next node
			TINT* pContact = TREINTERPRETCAST( TINT*, m_uiUnk10 ); // the contact body's user data { type, object }

			if ( bCheck && pContact != TNULL && pContact[ 0 ] == 3 && pContact[ 1 ] != 0 )
			{
				TerrainTypes::CellInstance* pInstance = TREINTERPRETCAST( TerrainTypes::CellInstance*, pContact[ 1 ] );

				if ( ( pInstance->m_uiFlags & 7 ) == 7 )
				{
					AZJumpData::Node* pInstanceNode = GetInstanceNode( pInstance );
					ACameraManager::GetSingleton()->GetShakeFunction().AddShake( 5, m_pOwner->GetBodyPosition(), 1.0f, -1 );

					if ( pInstanceNode == pNode )
					{
						// The instance under the node's instance (a ray along its Z axis)
						const TMatrix44& rMatrix = *pInstance->m_pMatrix;

						TPhysicsRay oRay;
						oRay.m_vFrom.Multiply( rMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), 3.0f );
						oRay.m_vFrom.x += rMatrix.m_f41;
						oRay.m_vFrom.y += rMatrix.m_f42;
						oRay.m_vFrom.z += rMatrix.m_f43;
						oRay.m_vTo.Multiply( rMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), -3.0f );
						oRay.m_vTo.x += rMatrix.m_f41;
						oRay.m_vTo.y += rMatrix.m_f42;
						oRay.m_vTo.z += rMatrix.m_f43;
						oRay.m_uiCollisionLayer = 0x81;

						TPhysicsShapeContact aContacts[ 4 ];
						TINT                 iNumContacts = TPhysicsManager::GetSingleton()->CollideRayAll( oRay, aContacts, 4, TNULL );

						for ( TINT i = 0; i < iNumContacts; i++ )
						{
							TPhysicsObjectContainer* pBody = aContacts[ i ].m_pBody;

							if ( pBody != TNULL && pBody->m_pUserData != TNULL )
							{
								TINT* pUserData = TSTATICCAST( TINT, pBody->m_pUserData );

								if ( pUserData[ 0 ] == 3 && TREINTERPRETCAST( TerrainTypes::CellInstance*, pUserData[ 1 ] ) != pInstance )
								{
									// NOTE: retail passes the contact itself, its first member is the position
									ABuildingManager::OnInstanceTouched( TREINTERPRETCAST( TerrainTypes::CellInstance*, pUserData[ 1 ] ), m_pOwner, &aContacts[ i ].m_vPosition );
									break;
								}
							}
						}

						AZJumpData::Node* pPrevNode = m_pCurrentNode;
						m_pCurrentNode              = pNode;

						for ( TINT i = 0; i < 2; i++ )
						{
							if ( pNode->m_apLinks[ i ] != pPrevNode )
							{
								m_pTargetNode = pNode->m_apLinks[ i ];
								break;
							}
						}

						if ( m_pTargetNode == TNULL )
						{
							bEnded       = TTRUE;
							m_pLastPoint = pNode->m_pSet->FindPoint( pNode );
						}
					}
				}
			}

			if ( m_pTargetNode == TNULL )
			{
				if ( bEnded )
				{
					AChallengeManager::GetSingleton()->OnZJumpEvent( m_pPoint->m_pSet );
					m_pOwner->GetSoundComponent()->PlayZJumpEnd();
					SetState( STATE_SETEND );
				}
				else
				{
					m_pOwner->GetSoundComponent()->PlayZJumpLand();
					SetState( STATE_NONE );
				}

				m_pCurrentNode         = TNULL;
				m_pPoint               = TNULL;
				m_pCurrentPriorityLock = TNULL;
			}
			else
			{
				// On to the next node: Blob holds on (state 3) for the hold time
				m_pOwner->GetSoundComponent()->OnZJumpPoint();
				m_pOwner->GetSoundComponent()->PlayZJumpNode();
				SetState( STATE_AIM );
				m_fHoldTime = ABlobEntity::GetCommonProperties()->m_afUnk38[ 0 ];
			}

			HideLockElement();
			return;
		}
		case STATE_AIM:
		case STATE_AIMNODE:
		case STATE_LOCK:
		case STATE_LOCKLMARK:
			return;
		case STATE_ATTACKSLAM:
		case STATE_SETEND:
		case STATE_TIMEOUT:
			if ( m_pOwner->m_oMotion.m_uiContactFlags & 0x20 )
			{
				m_pCurrentPriorityLock = TNULL;
				SetState( STATE_NONE );
			}

			return;
		case STATE_ATTACKFLIGHT:
		{
			ABlobEntity* pOwner = m_pOwner;

			if ( pOwner->m_oMotion.HasWallContactFlag( 0x800000 ) )
			{
				m_pCurrentPriorityLock = TNULL;
				TPhysicsManager::GetSingleton()->StopBody( pOwner->m_pBody );
				SetState( STATE_NONE );
				return;
			}

			PriorityTarget* pLock = m_pCurrentPriorityLock;

			if ( pLock == TNULL )
			{
				pOwner->PressCameraButton();
				SetState( STATE_NONE );
				return;
			}

			if ( pLock->m_eType != PriorityTarget::TARGET_ENTITY )
				return;

			// NOTE: retail reads the target's flags before its null check
			ABaseEntity* pTarget = TSTATICCAST( ABaseEntity, pLock->m_pTarget );

			if ( pTarget->IsFlagged( ABaseEntity::FLAG_Player ) )
			{
				m_pCurrentPriorityLock = TNULL;
				pOwner->ClearZPlayerAttack();
				m_pOwner->PressCameraButton();
				SetState( STATE_NONE );
				return;
			}

			if ( pTarget == TNULL )
			{
				m_pCurrentPriorityLock = TNULL;
				SetState( STATE_NONE );
				m_pOwner->PressCameraButton();
				return;
			}

			// Blob dropped below the target: the attack goes on to the slam (state 5)
			ABlobEntity::Properties& rProperties = ABlobEntity::ms_pBlobProperties[ pOwner->m_iSize ];

			if ( pTarget->GetMatrix().m_f42 - pTarget->GetBBox().y > pOwner->GetBodyPosition().y + rProperties.m_fRadius &&
			     TVector4::DistanceXZ( pOwner->GetBodyPosition(), pTarget->GetMatrix().GetTranslation() ) < rProperties.m_fSlamRadius )
			{
				m_pCurrentPriorityLock = TNULL;
				SetState( STATE_ATTACKSLAM );
			}

			return;
		}
		case STATE_LMARKFLIGHT:
		{
			ABlobEntity* pOwner = m_pOwner;

			if ( pOwner->m_oMotion.HasWallContactFlag( 0x800000 ) )
			{
				m_pCurrentPriorityLock = TNULL;
				TPhysicsManager::GetSingleton()->StopBody( pOwner->m_pBody );
				SetState( STATE_NONE );
			}

			return;
		}
		case STATE_SWITCHSLAM:
		{
			if ( m_pCurrentPriorityLock != TNULL && m_pCurrentPriorityLock->m_pTarget != TNULL &&
			     TSTATICCAST( ABaseEntity, m_pCurrentPriorityLock->m_pTarget )->IsA( &TGetClass( APropTrigger ) ) )
			{
				APropTrigger* pTrigger = TSTATICCAST( APropTrigger, m_pCurrentPriorityLock->m_pTarget );
				TASSERT( pTrigger == TNULL || pTrigger->IsA( &TGetClass( APropTrigger ) ) );
				TFLOAT fRadius = m_pOwner->GetBoundingSphereRadius();

				if ( fRadius * fRadius > TVector4::DistanceSq( pTrigger->GetMatrix().GetTranslation(), m_pOwner->GetBodyPosition() ) )
				{
					AEntityIOPacket oPacket;
					oPacket.m_eMessage = AEntityIOPacket::MESSAGE_SLAM;
					oPacket.m_iData    = 0;
					pTrigger->HandleEntityIO( m_pOwner, oPacket );
				}
			}

			m_pCurrentPriorityLock = TNULL;
			SetState( STATE_NONE );
			return;
		}
		default:
			return;
	}
}

// $deBlob: FUNCTION 00766600
void ABlobZJumpComponent::SetLockElement( TXUIControl* a_pElement )
{
	m_pLockElement = a_pElement;
	a_pElement->SetVisible( TFALSE );

	TXUIVisual* pVisual = m_pLockElement->GetVisual();

	if ( pVisual == TNULL )
		return;

	m_pEnergyFill = pVisual->FindChildXUIElement( L"EnergyFill" );
	TSTATICCAST( TXUIVisual, m_pEnergyFill )->GetTimelineControl()->ResetAnimation();

	m_pOutline = m_pLockElement->GetVisual()->FindChildXUIElement( L"outline" );
	m_pOutline->SetColour( 0xFFBCBAA5 );
	m_pCircleAbove = m_pEnergyFill->FindChildXUIElement( L"circleabove" );
	m_pGreyFill    = m_pLockElement->GetVisual()->FindChildXUIElement( L"GreyFill" );
	m_pGreyFill->SetColour( 0xFF7A786B );
	m_pCannotLock   = m_pLockElement->GetVisual()->FindChildXUIElement( L"cannotlock" );
	m_pCannotTarget = m_pLockElement->GetVisual()->FindChildXUIElement( L"CannotTarget" );
	m_pPout         = m_pLockElement->GetVisual()->FindChildXUIElement( L"pout" );
	m_pPout->SetColour( 0xFF383F38 );
	m_pPassiveFill = m_pLockElement->GetVisual()->FindChildXUIElement( L"PassiveFill" );

	TWCHAR* wszName = TStringManager::GetTempString16();

	for ( TINT i = 0; i < TINT( TARRAYSIZE( m_apPaintMeters ) ); )
	{
		i++;
		TStringManager::String16Format( wszName, L"PaintMeter%d", i );
		m_apPaintMeters[ i - 1 ] = m_pEnergyFill->FindChildXUIElement( wszName );
	}

	for ( TINT i = 0; i < 8; )
	{
		i++;
		TStringManager::String16Format( wszName, L"trailoutline%d", i );
		m_pEnergyFill->FindChildXUIElement( wszName )->SetColour( 0xFF383F38 );
	}

	TXUIElement* pZJump = m_pLockElement->GetVisual()->FindChildXUIElement( L"Zjump" );
	pZJump->FindChildXUIElement( L"zjump1" )->SetColour( 0xFF4CD096 );
	pZJump->FindChildXUIElement( L"zjump2" )->SetColour( 0xFF4CD096 );
	pZJump->FindChildXUIElement( L"zjump3" )->SetColour( 0xFF4CD096 );
	pZJump->FindChildXUIElement( L"zjump4" )->SetColour( 0xFF4CD096 );

	TXUITimelineControl* pTimeline = TSTATICCAST( TXUIVisual, m_pEnergyFill )->GetTimelineControl();
	m_fEnergy1Point                = pTimeline->GetNamedFrameTime( L"1Point" );
	m_fEnergy100Point              = TSTATICCAST( TXUIVisual, m_pEnergyFill )->GetTimelineControl()->GetNamedFrameTime( L"100Point" );
	m_fEnergy200Point              = TSTATICCAST( TXUIVisual, m_pEnergyFill )->GetTimelineControl()->GetNamedFrameTime( L"200Point" );

	m_pScore      = m_pEnergyFill->FindChildXUIControl( L"Score" );
	m_pScoreWhite = m_pEnergyFill->FindChildXUIControl( L"ScoreWhite" );
	m_pScoreWhite->SetVisible( TFALSE );

	m_pLeftArrowColour  = m_pLockElement->GetVisual()->FindChildXUIElement( L"leftarrowcolour" );
	m_pRightArrowColour = m_pLockElement->GetVisual()->FindChildXUIElement( L"rightarrowcolour" );
}

// $deBlob: FUNCTION 00766370
void ABlobZJumpComponent::Deactivate()
{
	SetState( STATE_NONE );
	m_pOwner->ClearZPlayerAttack();

	if ( TXUIControl* pLockElement = TSTATICCAST( TXUIControl, m_pLockElement ) )
	{
		pLockElement->SetVisible( TFALSE );
		pLockElement->GetVisual()->StopAnimation();
	}
}

// $deBlob: FUNCTION 007663a0
void ABlobZJumpComponent::EndAttack( TBOOL a_bHit )
{
	if ( m_eState == STATE_ATTACKFLIGHT )
	{
		if ( !a_bHit )
		{
			SetState( STATE_NONE );
			m_pOwner->PressCameraButton();
			m_pOwner->ClearZPlayerAttack();
		}
		else
		{
			SetState( STATE_ATTACKSLAM );
			m_pOwner->ClearZPlayerAttack();
		}
	}
	else if ( m_eState == STATE_LMARKFLIGHT )
	{
		SetState( STATE_NONE );

		if ( !a_bHit )
			TPhysicsManager::GetSingleton()->StopBody( m_pOwner->m_pBody );
	}

	HideLockElement();
}

// $deBlob: FUNCTION 00766870
AZJumpData::Point* ABlobZJumpComponent::FindPoint() const
{
	AZJumpData* pData = AZJumpData::GetSingleton();

	if ( pData == TNULL || m_pOwner->GetNumLeechbotsAttached() > 0 )
		return TNULL;

	// NOTE: retail tests all of Blob's flags (0x554) against 0
	if ( m_pOwner->m_uiFlags != 0 )
	{
		for ( TINT i = 0; i < pData->GetPointCount(); i++ )
		{
			AZJumpData::Point* pPoint = pData->GetPoint( i );

			if ( pPoint != m_pLastPoint )
			{
				ABlobEntity* pOwner = m_pOwner;

				if ( TVector4::Distance( pPoint->m_pNode->GetPosition(), pOwner->GetMatrix().GetTranslation() ) - m_pOwner->GetBoundingSphereRadius() < 2.0f )
					return pPoint;
			}
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00767670
void ABlobZJumpComponent::UpdateHoldTimeline()
{
	ABlobHud*    pHud      = ABlobHud::GetSingleton();
	TXUIElement* pTimeline = pHud->GetHoldTimeline();
	pTimeline->SetVisible( TTRUE );

	TFLOAT fProgress = 1.0f - m_fHoldTime / ABlobEntity::GetCommonProperties()->m_afUnk38[ 0 ];
	TMath::Clip( fProgress, 0.0f, 1.0f );
	fProgress = ABlobHud::GetSingleton()->GetHoldTimelineStopTime() * fProgress;
	TSTATICCAST( TXUIVisual, pTimeline )->GetTimelineControl()->SetTimeLineTime( fProgress, TTRUE );

	TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( APlayerManager::GetSingleton()->GetBlobCamera( m_pOwner ) );

	TVector4 vPosition = m_pCurrentNode->GetPosition();
	vPosition.y -= 2.0f;
	ARenderer::GetSingleton()->ProjectToCanvas( vPosition, pViewport );

	if ( TRender::GetSingleton()->GetAspectRatio() != TRender::ASPECT_RATIO_16_9 )
		vPosition.x += TXUI::GetSingleton()->GetCanvas()->GetPositionX();

	TFLOAT fX = vPosition.x - pTimeline->GetWidth() * 0.5f;
	TFLOAT fY = vPosition.y - pTimeline->GetHeight() * 0.5f;
	pTimeline->SetPosition( fX, fY );
}

// $deBlob: FUNCTION 007677e0
void ABlobZJumpComponent::UpdateAim( TFLOAT a_fDeltaTime, const TVector4& a_rcTarget )
{
	TVector4   vCanvas   = a_rcTarget;
	TViewport* pViewport = ARenderer::GetSingleton()->GetViewportManager()->GetCameraTViewport( m_pOwner->GetViewCamera() );
	TVector4   vMarker   = a_rcTarget;

	if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == AGameStatus::GAMETYPE_MP_NONE )
	{
		ARenderer::GetSingleton()->ProjectToGUI( vMarker, pViewport );
		vMarker.x += pViewport->GetX();
		vMarker.y += pViewport->GetY();
		ARenderer::GetSingleton()->ProjectToCanvas( vCanvas, pViewport );
		vCanvas.x += m_fLockElementOffsetX;
	}
	else
	{
		ARenderer::GetSingleton()->ProjectToScreen( vCanvas, pViewport );
		vCanvas.x += pViewport->GetX();
		vCanvas.y += pViewport->GetY();
		ARenderer::GetSingleton()->ScreenToCanvas( vCanvas.x, vCanvas.y );

		// The split screen: relative to the parent of the lock element
		const TVector2& rParentPos = m_pLockElement->GetParent()->GetTransform().GetPos();
		vCanvas.x -= rParentPos.x;
		vCanvas.y -= rParentPos.y;
	}


	if ( TXUIControl* pLockElement = m_pLockElement )
	{
		TFLOAT fX = vCanvas.x - pLockElement->GetWidth() * 0.5f;
		TFLOAT fY = vCanvas.y - m_pLockElement->GetHeight() * 0.5f;
		pLockElement->SetPosition( fX, fY );

		if ( !m_pLockElement->IsVisible() )
			UpdateLockElement( TFALSE );
	}

	if ( m_eState != STATE_LOCK && m_eState != STATE_LOCKLMARK )
		return;

	// The off-screen markers of the other targets
	for ( auto itGroup = m_lGroups.Begin(); itGroup != m_lGroups.End(); )
	{
		PriorityGroup* pGroup      = itGroup;
		auto           itNextGroup = itGroup.Next();

		for ( auto it = pGroup->m_lLockList.Begin(); it != pGroup->m_lLockList.End(); )
		{
			PriorityTarget* pTarget = it;
			it                      = it.Next();

			if ( m_pCurrentPriorityLock == pTarget )
				continue;

			if ( !IsStillLockable( *pTarget ) )
			{
				pTarget->Remove();
				FreeTarget( *pTarget );
				continue;
			}

			if ( pTarget->m_eType == PriorityTarget::TARGET_ENTITY )
			{
				ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, pTarget->m_pTarget );
				vMarker              = pEntity->GetMatrix().GetTranslation();
				vMarker.y            = pEntity->GetMatrix().m_f42 - TSTATICCAST( ABaseEntity, pTarget->m_pTarget )->GetBBox().y * 2.0f;
			}
			else if ( pTarget->m_eType == PriorityTarget::TARGET_LMARK )
			{
				TSTATICCAST( ALandmarkInstance, pTarget->m_pTarget )->GetTriggerPosition( pTarget->m_iIndex, vMarker );
			}
			else if ( pTarget->m_eType == PriorityTarget::TARGET_INSTANCE )
			{
				vMarker = TSTATICCAST( TerrainTypes::CellInstance, pTarget->m_pTarget )->m_pMatrix->GetTranslation();
			}

			ARenderer::GetSingleton()->ProjectToGUI( vMarker, pViewport );
			vMarker.x += pViewport->GetX();
			vMarker.y += pViewport->GetY();
			pTarget->m_pRectangle->SetTransform( vMarker.x, -( vMarker.y - 8.0f ), 1.5707964f );
			pTarget->m_pRectangle->SetVisible( TTRUE );
		}

		if ( pGroup->m_lLockList.IsEmpty() )
			pGroup->Remove();

		itGroup = itNextGroup;
	}
}

// $deBlob: FUNCTION 00767c30
void ABlobZJumpComponent::ShowLock( PriorityTarget& a_rLock )
{
	TVector4 vPosition;

	switch ( a_rLock.m_eType )
	{
		case PriorityTarget::TARGET_ENTITY:
			HideLockElement();
			GetEntityTargetPosition( TSTATICCAST( ABaseEntity, a_rLock.m_pTarget ), vPosition );
			break;
		case PriorityTarget::TARGET_LMARK:
			TSTATICCAST( ALandmarkInstance, a_rLock.m_pTarget )->GetTriggerPosition( a_rLock.m_iIndex, vPosition );
			HideLockElement();
			break;
		case PriorityTarget::TARGET_INSTANCE:
			HideLockElement();
			UpdateAim( 0.0f, TSTATICCAST( TerrainTypes::CellInstance, a_rLock.m_pTarget )->m_pMatrix->GetTranslation() );
			return;
		default:
			return;
	}

	UpdateAim( 0.0f, vPosition );
}

// $deBlob: FUNCTION 00767d20
void ABlobZJumpComponent::UpdateTargetFlags( PriorityTarget& a_rTarget )
{
	a_rTarget.m_uiFlags &= ~PriorityTarget::FLAGS_UNLOCKABLE;

	ABlobEntity* pOwner  = m_pOwner;
	TUINT32      uiFlags = a_rTarget.m_uiFlags;

	if ( ( pOwner->m_iPaintPoints != 0 || ( uiFlags & PriorityTarget::FLAGS_UNK100 ) != 0 ) && a_rTarget.CanPaint( pOwner ) )
	{
		if ( ( ( pOwner->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) == 0 )
		{
			// Another Blob: only one Blob can ink
			if ( a_rTarget.m_ePriorityType == 0x24 )
			{
				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_rTarget.m_pTarget );
				TASSERT( pBlob == TNULL || pBlob->IsA( &TGetClass( ABlobEntity ) ) );

				if ( !pOwner->CanInkBlob( pBlob ) )
					a_rTarget.m_uiFlags |= PriorityTarget::FLAGS_UNLOCKABLE;
			}

			return;
		}

		// An inked Blob can't Z-jump at the ZPRIORITY 0x20-0x22 targets
		if ( TUINT32( a_rTarget.m_ePriorityType - 0x20 ) >= 3 )
			return;
	}

	a_rTarget.m_uiFlags = uiFlags | PriorityTarget::FLAGS_UNLOCKABLE;
}

// $deBlob: FUNCTION 00767db0
TBOOL ABlobZJumpComponent::IsStillLockable( PriorityTarget& a_rTarget )
{
	ABlobEntity::CommonProperties* pProperties = ABlobEntity::GetCommonProperties();

	switch ( a_rTarget.m_eType )
	{
		case PriorityTarget::TARGET_ENTITY:
		{
			ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, a_rTarget.m_pTarget );

			if ( pEntity == m_pOwner || ( ( pEntity->GetFlags() >> 0x16 ) & 1 ) != 0 || !pEntity->IsTargetable() )
				return TFALSE;

			if ( 0 < a_rTarget.m_iIndex && pEntity->GetPooledID() != a_rTarget.m_iIndex )
				return TFALSE;

			TINT ePriorityType = TINT8( TSTATICCAST( ABaseEntity, a_rTarget.m_pTarget )->GetCollisionGroupAndType() >> 8 );

			if ( ePriorityType == -1 )
				return TFALSE;

			if ( ePriorityType == 5 )
			{
				if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 )
				{
					ABaseEntity* pPlayer = APlayerManager::GetSingleton()->GetPaintedBlob();

					if ( pPlayer != TNULL && pPlayer != m_pOwner )
						return TFALSE;
				}
			}
			else if ( ePriorityType == 0x24 )
			{
				TASSERT( pEntity->IsFlagged( ABaseEntity::FLAG_Player ), "pCurEntity->IsPlayer()" );

				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pEntity );
				TASSERT( pBlob == TNULL || pBlob->IsA( &TGetClass( ABlobEntity ) ) );

				if ( !m_pOwner->CanInkBlob( pBlob ) )
					return TFALSE;
			}

			if ( GetPriority( ePriorityType, m_pOwner ) == -1 )
				return TFALSE;

			if ( ( ( m_pOwner->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) == 0 &&
			     TSTATICCAST( ABaseEntity, a_rTarget.m_pTarget )->IsA( &TGetClass( APropWaterHydrant ) ) )
				return TFALSE;

			ABlobEntity*    pOwner          = m_pOwner;
			ABaseEntity*    pTarget         = TSTATICCAST( ABaseEntity, a_rTarget.m_pTarget );
			const TVector4& rTargetPosition = pTarget->GetMatrix().GetTranslation();
			TVector4        vDelta( pOwner->GetBodyPosition().x - rTargetPosition.x, pOwner->GetBodyPosition().y - rTargetPosition.y, pOwner->GetBodyPosition().z - rTargetPosition.z, pOwner->GetBodyPosition().w );

			if ( a_rTarget.m_ePriorityType != 0x25 && vDelta.y > pOwner->GetBoundingSphereRadius() && !pTarget->CanBeTargetedFromBelow() )
				return TFALSE;

			TFLOAT fDistance = vDelta.NormaliseMag();
			pTarget          = TSTATICCAST( ABaseEntity, a_rTarget.m_pTarget );

			// The boss is always in reach
			ABaseEntity* pAlwaysInReach = ABoss::GetSingleton();

			if ( pAlwaysInReach == pTarget || fDistance < pProperties->m_afUnk38[ 1 ] )
				return m_pOwner->GetViewCamera()->IsInViewCone( pTarget->GetMatrix().GetTranslation(), pTarget->GetBoundingSphereRadius(), 200.0f );

			return TFALSE;
		}
		case PriorityTarget::TARGET_LMARK:
		{
			ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, a_rTarget.m_pTarget );

			if ( pLandmark == TNULL || a_rTarget.m_iIndex < 0 || !pLandmark->IsTriggerUnpainted( a_rTarget.m_iIndex ) )
				return TFALSE;

			TMatrix44 oTrigger;
			pLandmark->GetTriggerMatrix( a_rTarget.m_iIndex, oTrigger );

			// In front of the trigger, not too far below Blob
			const TVector4& rBlobPosition = m_pOwner->GetBodyPosition();
			const TVector4& rTrigger      = oTrigger.GetTranslation();

			if ( rBlobPosition.y - pProperties->m_afUnk38[ 2 ] < rTrigger.y &&
			     0.0f < oTrigger.m_f12 * ( rBlobPosition.y - rTrigger.y ) + oTrigger.m_f11 * ( rBlobPosition.x - rTrigger.x ) + oTrigger.m_f13 * ( rBlobPosition.z - rTrigger.z ) )
			{
				if ( pProperties->m_afUnk38[ 1 ] * 1.21f * pProperties->m_afUnk38[ 1 ] <= TVector4::DistanceSq( rTrigger, rBlobPosition ) )
					return TFALSE;

				return m_pOwner->GetViewCamera()->IsInViewCone( rTrigger, 200.0f );
			}

			return TFALSE;
		}
		case PriorityTarget::TARGET_INSTANCE:
		{
			TVector4 vPosition = TSTATICCAST( TerrainTypes::CellInstance, a_rTarget.m_pTarget )->m_pMatrix->GetTranslation();

			if ( pProperties->m_afUnk38[ 1 ] * 1.21f * pProperties->m_afUnk38[ 1 ] <= TVector4::DistanceSq( vPosition, m_pOwner->GetBodyPosition() ) )
				return TFALSE;

			return m_pOwner->GetViewCamera()->IsInViewCone( vPosition, 200.0f );
		}
		default:
			return TFALSE;
	}
}

// $deBlob: FUNCTION 00768140
TBOOL ABlobZJumpComponent::IsStillAttackable( PriorityTarget& a_rTarget ) const
{
	switch ( a_rTarget.m_eType )
	{
		case PriorityTarget::TARGET_ENTITY:
		{
			ABaseEntity* pEntity = TSTATICCAST( ABaseEntity, a_rTarget.m_pTarget );
			TASSERT( pEntity != m_pOwner, "pCurEntity!=&m_rBlob" );

			if ( ( ( pEntity->GetFlags() >> 0x16 ) & 1 ) == 0 && pEntity->IsTargetable() )
			{
				if ( 0 < a_rTarget.m_iIndex && pEntity->GetPooledID() != a_rTarget.m_iIndex )
					return TFALSE;

				TINT ePriorityType = TINT8( TSTATICCAST( ABaseEntity, a_rTarget.m_pTarget )->GetCollisionGroupAndType() >> 8 );

				if ( ePriorityType != -1 )
					return GetPriority( ePriorityType, m_pOwner ) != -1;
			}

			return TFALSE;
		}
		case PriorityTarget::TARGET_LMARK:
			if ( a_rTarget.m_pTarget != TNULL && -1 < a_rTarget.m_iIndex )
				return TSTATICCAST( ALandmarkInstance, a_rTarget.m_pTarget )->IsTriggerUnpainted( a_rTarget.m_iIndex );

			return TFALSE;
		case PriorityTarget::TARGET_INSTANCE:
			return TTRUE;
		default:
			return TFALSE;
	}
}

// $deBlob: FUNCTION 00768260
void ABlobZJumpComponent::SelectLock()
{
	PriorityTarget* pPrevLock = m_pCurrentPriorityLock;

	if ( pPrevLock == TNULL )
	{
		if ( m_lGroups.IsEmpty() )
		{
			if ( m_lTargets.IsEmpty() && m_fSearchTimer <= 0.0f )
			{
				PruneGroups();
				FindNodeTargets();
				FindEntityTargets();
				FindLandmarkTargets();
				m_fSearchTimer = 0.3f;
			}

			m_pCurrentPriorityLock = TNULL;
			HideLockElement();
		}
		else
		{
			// The best target of the first group that has one (the empty groups are removed)
			for ( auto it = m_lGroups.Begin(); it != m_lGroups.End(); )
			{
				PriorityGroup* pGroup = it;
				it                    = it.Next();

				if ( !pGroup->m_lLockList.IsEmpty() )
				{
					m_pOwner->GetSoundComponent()->PlayLockOn();
					m_pCurrentPriorityLock = pGroup->m_lLockList.Head();
					HideLockElement();
					m_pCurrentPriorityLock->m_pRectangle->SetVisible( TFALSE );
					break;
				}

				pGroup->Remove();
			}
		}
	}
	else
	{
		PriorityGroup* pGroup = &m_pGroups[ pPrevLock->m_ePriorityType ];

		if ( ( pPrevLock->m_uiFlags & PriorityTarget::FLAGS_LOCKED ) == 0 )
		{
			// Cycles to the next group with a target
			TASSERT( pGroup->IsLinked() );

			if ( pGroup->Next() == pGroup->Prev() )
				return;

			auto itEnd = m_lGroups.End();
			auto it    = T2DList<PriorityGroup>::Iterator( pGroup ).Next();

			if ( it != pGroup )
			{
				PriorityGroup* pNext;

				do
				{
					if ( m_lGroups.IsEmpty() )
						break;

					if ( it == itEnd )
						it = m_lGroups.Begin();

					PriorityGroup* pCurrent = it;
					pNext                   = TSTATICCAST( PriorityGroup, pCurrent->Next() );

					if ( !pCurrent->m_lLockList.IsEmpty() )
					{
						m_pOwner->GetSoundComponent()->PlayLockOn();
						m_pCurrentPriorityLock = pCurrent->m_lLockList.Head();
						HideLockElement();
						m_pCurrentPriorityLock->m_pRectangle->SetVisible( TFALSE );
						break;
					}

					pCurrent->Remove();
					it = pNext;
				} while ( pNext != pGroup );
			}
		}
		else
		{
			// FLAGS_LOCKED: cycles within the group
			T2DList<PriorityTarget>& rLocks = pGroup->m_lLockList;
			TASSERT( rLocks.IsInList( m_pCurrentPriorityLock ) );

			auto itEnd = rLocks.End();
			auto it    = T2DList<PriorityTarget>::Iterator( m_pCurrentPriorityLock ).Next();

			while ( !rLocks.IsEmpty() )
			{
				if ( it == itEnd )
					it = rLocks.Begin();

				PriorityTarget* pTarget = it;

				if ( IsStillLockable( *pTarget ) && IsTargetVisible( *pTarget ) )
				{
					m_pOwner->GetSoundComponent()->PlayLockOn();
					m_pCurrentPriorityLock = pTarget;
					HideLockElement();
					m_pCurrentPriorityLock->m_pRectangle->SetVisible( TFALSE );
					break;
				}

				it = it.Next();
				pTarget->Remove();
				FreeTarget( *pTarget );

				if ( pPrevLock == pTarget )
				{
					m_pCurrentPriorityLock = TNULL;
					HideLockElement();
				}
			}
		}
	}

	if ( pPrevLock != m_pCurrentPriorityLock )
		m_pOwner->GetSoundComponent()->PlayLockChange();

	if ( m_pCurrentPriorityLock != TNULL )
		ShowLock( *m_pCurrentPriorityLock );
}

// $deBlob: FUNCTION 00769230
void ABlobZJumpComponent::FindLandmarkTargets()
{
	ABlobEntity::CommonProperties* pProperties = ABlobEntity::GetCommonProperties();
	TFLOAT                         fMaxDistSq  = pProperties->m_afUnk38[ 1 ] * pProperties->m_afUnk38[ 1 ];
	ABuildingManager*              pManager    = ABuildingManager::GetSingleton();
	TFLOAT                         fMinY       = m_pOwner->GetBodyPosition().y - pProperties->m_afUnk38[ 2 ];

	for ( TINT i = 0; i < pManager->GetNumLandmarks(); i++ )
	{
		ALandmarkInstance* pLandmark = TSTATICCAST( ALandmarkInstance, pManager->GetLandmarkAt( i ) );

		if ( ( pLandmark->GetBuildingFlags() & ALandmarkInstance::Flags_TriggersHidden ) != 0 || pLandmark->GetNumTriggersLeft() <= 0 )
			continue;

		for ( TINT j = 0; j < pLandmark->GetNumTriggers(); j++ )
		{
			if ( !pLandmark->IsTriggerUnpainted( j ) )
				continue;

			TVector4 vTrigger;
			pLandmark->GetTriggerPosition( j, vTrigger );

			if ( fMinY >= vTrigger.y )
				continue;

			TFLOAT fDistSq = TVector4::DistanceSq( vTrigger, m_pOwner->GetBodyPosition() );

			if ( fDistSq >= fMaxDistSq )
				continue;

			// Already in a group or in m_lTargets?
			TBOOL                    bFound  = TFALSE;
			T2DList<PriorityTarget>& rGroup1 = m_pGroups[ 0x12 ].m_lLockList;

			for ( auto it = rGroup1.Begin(); it != rGroup1.End(); it++ )
			{
				if ( it->m_eType == PriorityTarget::TARGET_LMARK && it->m_pTarget == pLandmark && it->m_iIndex == j )
				{
					bFound = TTRUE;
					break;
				}
			}

			TBOOL                    bSkip   = TFALSE;
			T2DList<PriorityTarget>& rGroup2 = m_pGroups[ 0x13 ].m_lLockList;

			for ( auto it = rGroup2.Begin(); it != rGroup2.End(); it++ )
			{
				if ( it->m_eType == PriorityTarget::TARGET_LMARK && it->m_pTarget == pLandmark && it->m_iIndex == j )
				{
					bSkip = TTRUE;
					break;
				}
			}

			if ( bSkip || bFound )
				continue;

			if ( !m_pOwner->GetViewCamera()->IsInViewCone( vTrigger, 200.0f ) )
				continue;

			PriorityTarget* pTarget = AllocTarget( fDistSq );

			if ( pTarget == TNULL )
				continue;

			TUINT32 uiTriggerColour  = pLandmark->GetTriggerColour( j );
			pTarget->m_pTarget       = pLandmark;
			pTarget->m_uiFlags       = ( uiTriggerColour & 0xF ) | PriorityTarget::FLAGS_PAINTABLE | PriorityTarget::FLAGS_UNSORTED;
			pTarget->m_eType         = PriorityTarget::TARGET_LMARK;
			pTarget->m_iIndex        = j;
			pTarget->m_fDistance     = fDistSq;
			pTarget->m_ePriorityType = 0x13;

			// 0x12 when Blob can't paint the trigger
			ABlobEntity* pOwner = m_pOwner;

			if ( pOwner->m_iPaintPoints == 0 )
			{
				pTarget->m_ePriorityType = 0x12;
			}
			else
			{
				TUINT32 uiBlobColour = pOwner->m_pColourComponent->GetCurrentColour();

				if ( uiBlobColour & 0x10 )
					uiBlobColour = 0x10;

				if ( uiTriggerColour == 0 )
				{
					if ( uiBlobColour == 0 || 0xF < uiBlobColour )
						pTarget->m_ePriorityType = 0x12;
				}
				else if ( uiTriggerColour != uiBlobColour && ( TUINT8( pOwner->m_uiFlags >> 0x15 ) & 1 ) == 0 )
				{
					pTarget->m_ePriorityType = 0x12;
				}
			}

			pTarget->m_iPriority = GetPriority( pTarget->m_ePriorityType, m_pOwner );
			UpdateTargetFlags( *pTarget );
			InsertTarget( m_lTargets, pTarget );
		}
	}
}

// $deBlob: FUNCTION 00769620
TBOOL ABlobZJumpComponent::IsTargetVisible( PriorityTarget& a_rTarget )
{
	TASSERT( IsStillLockable( a_rTarget ) );

	TVector4 vPosition;

	if ( a_rTarget.m_eType == PriorityTarget::TARGET_INSTANCE && a_rTarget.m_ePriorityType == 0x25 )
	{
		if ( !a_rTarget.GetBasePosition( vPosition ) )
			return TFALSE;
	}
	else if ( !a_rTarget.GetPosition( vPosition ) )
	{
		return TFALSE;
	}

	TFLOAT fHeight = 0.0f;

	if ( !a_rTarget.GetHeight( &fHeight, m_pOwner->GetBoundingSphereRadius() ) )
		return TFALSE;

	// The point Blob lands at
	TVector4 vAbove = vPosition;
	vAbove.y -= fHeight;

	// The user data of the target's collision ({ 2, entity } or { 3, cell instance })
	TINT  eUserType;
	void* pUserObject;

	if ( a_rTarget.m_eType == PriorityTarget::TARGET_ENTITY )
	{
		eUserType   = 2;
		pUserObject = a_rTarget.m_pTarget;
	}
	else if ( a_rTarget.m_eType == PriorityTarget::TARGET_LMARK )
	{
		eUserType   = 3;
		pUserObject = TSTATICCAST( ALandmarkInstance, a_rTarget.m_pTarget )->m_pTerrainInstance;
	}
	else if ( a_rTarget.m_eType == PriorityTarget::TARGET_INSTANCE )
	{
		eUserType   = 3;
		pUserObject = a_rTarget.m_pTarget;
	}
	else
	{
		return TFALSE;
	}

	auto IsTargetHit = [ eUserType, pUserObject ]( const TPhysicsShapeContact& a_rContact ) {
		if ( a_rContact.m_pBody == TNULL || a_rContact.m_pBody->m_pUserData == TNULL )
			return TFALSE;

		TINT* pUserData = TSTATICCAST( TINT, a_rContact.m_pBody->m_pUserData );
		return pUserData[ 0 ] == eUserType && TREINTERPRETCAST( void*, pUserData[ 1 ] ) == pUserObject;
	};

	TPhysicsRay          oRay;
	TPhysicsShapeContact oContact;

	if ( TVector4::DistanceSq( vAbove, vPosition ) > 0.01f )
	{
		// From the landing point to the target
		oRay.m_vFrom            = vAbove;
		oRay.m_vTo              = vPosition;
		oRay.m_uiCollisionLayer = 0xA3;

		if ( TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) && !IsTargetHit( oContact ) )
			return TFALSE;
	}

	// From Blob to the landing point
	oRay.m_vFrom            = m_pOwner->GetBasePosition();
	oRay.m_vTo              = vAbove;
	oRay.m_uiCollisionLayer = 0xA1;

	if ( TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) && !IsTargetHit( oContact ) )
		return TFALSE;

	// From the camera to the target
	oRay.m_vFrom            = m_pOwner->GetViewCamera()->GetMatrix().GetTranslation();
	oRay.m_vTo              = vPosition;
	oRay.m_uiCollisionLayer = 0x10;

	if ( !TPhysicsManager::GetSingleton()->CollideRay( oRay, oContact, TNULL ) )
		return TTRUE;

	return IsTargetHit( oContact );
}

// $deBlob: FUNCTION 00769890
ABlobZJumpComponent::PriorityTarget* ABlobZJumpComponent::AllocTarget( TFLOAT a_fDistance )
{
	if ( !m_lFreeTargets.IsEmpty() )
		return m_lFreeTargets.PopBack();

	PriorityTarget* pTarget = TNULL;

	// The worst target of the last group
	while ( !m_lGroups.IsEmpty() )
	{
		if ( pTarget != TNULL )
			return pTarget;

		PriorityGroup* pGroup = m_lGroups.Tail();

		if ( !pGroup->m_lLockList.IsEmpty() )
		{
			pTarget = pGroup->m_lLockList.PopBack();
			pTarget->m_pRectangle->SetVisible( TFALSE );

			if ( m_pCurrentPriorityLock == pTarget )
				m_pCurrentPriorityLock = TNULL;

			if ( !pGroup->m_lLockList.IsEmpty() )
				continue;
		}

		pGroup->Remove();
	}

	if ( pTarget != TNULL )
		return pTarget;

	// The farthest of the targets to sort, when a_fDistance is nearer
	if ( !m_lTargets.IsEmpty() && a_fDistance < m_lTargets.Tail()->m_fDistance )
		return m_lTargets.PopBack();

	return TNULL;
}

// $deBlob: FUNCTION 00769b80
void ABlobZJumpComponent::HideLockElement()
{
	if ( m_pLockElement != TNULL )
	{
		m_pLockElement->SetVisible( TFALSE );
		m_pLockElement->GetVisual()->StopAnimation();
	}
}

// $deBlob: FUNCTION 00769c40
TFLOAT ABlobZJumpComponent::GetNodeWeight( AZJumpData::Node* a_pNode ) const
{
	if ( m_pStandNode == a_pNode )
		return 2.0f;

	if ( m_pUnk2C != TNULL )
		return ( m_pUnk2C == a_pNode ) ? 1.0f : 0.0f;

	const TMatrix44& rCamera   = m_pOwner->GetViewCamera()->GetMatrix();
	const TVector4&  rPosition = a_pNode->GetPosition();

	TVector4 vDir( rPosition.x - rCamera.m_f41, rPosition.y - rCamera.m_f42, rPosition.z - rCamera.m_f43, rPosition.w );
	vDir.Normalise();

	return ( rCamera.m_f32 * vDir.y + rCamera.m_f31 * vDir.x + rCamera.m_f33 * vDir.z ) * -1.0f + 1.0f;
}

// $deBlob: FUNCTION 00769d20
TBOOL ABlobZJumpComponent::IsPointWithoutEntities( AZJumpData::Point* a_pPoint )
{
	AZJumpData::Set* pSet = a_pPoint->m_pSet;

	for ( TINT i = 0; i < pSet->m_iNodeCount; i++ )
	{
		if ( pSet->m_pNodes[ i ].m_eType == AZJumpData::NodeType_Entity )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0076a300
TINT __stdcall ABlobZJumpComponent::CompareTargets( PriorityTarget* a_pTarget1, PriorityTarget* a_pTarget2 )
{
	// The lockable one first, then the higher priority, then the nearer
	TINT iA = a_pTarget1->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;
	TINT iB = a_pTarget2->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;

	if ( iA == iB )
	{
		if ( a_pTarget1->m_iPriority == a_pTarget2->m_iPriority )
		{
			if ( a_pTarget1->m_fDistance < a_pTarget2->m_fDistance )
				return -1;

			return a_pTarget2->m_fDistance < a_pTarget1->m_fDistance;
		}

		return a_pTarget2->m_iPriority - a_pTarget1->m_iPriority;
	}

	return iA - iB;
}

// $deBlob: FUNCTION 0076a450
TINT __stdcall ABlobZJumpComponent::CompareGroupTargets( PriorityTarget* a_pTarget1, PriorityTarget* a_pTarget2 )
{
	// The lockable one first, then the nearer
	TINT iA = a_pTarget1->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;
	TINT iB = a_pTarget2->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;

	if ( iA == iB )
	{
		if ( a_pTarget1->m_fDistance < a_pTarget2->m_fDistance )
			return -1;

		return a_pTarget2->m_fDistance < a_pTarget1->m_fDistance;
	}

	return iA - iB;
}

// $deBlob: FUNCTION 0076a350
TINT ABlobZJumpComponent::CompareGroups( PriorityGroup* a_pGroup1, PriorityGroup* a_pGroup2 )
{
	T2DList<PriorityTarget>& rList1 = a_pGroup1->m_lLockList;
	T2DList<PriorityTarget>& rList2 = a_pGroup2->m_lLockList;

	if ( !rList1.IsEmpty() && !rList2.IsEmpty() )
	{
		PriorityTarget* pBest1 = rList1.Head();
		PriorityTarget* pBest2 = rList2.Head();

		// The higher priority first, then the lockable one, then the nearer
		TUINT32 uiA = pBest1->m_iPriority;
		TUINT32 uiB = pBest2->m_iPriority;

		if ( uiA == uiB )
		{
			uiA = pBest2->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;
			uiB = pBest1->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;

			if ( uiA == uiB )
			{
				if ( pBest1->m_fDistance < pBest2->m_fDistance )
					return -1;

				return pBest2->m_fDistance < pBest1->m_fDistance;
			}
		}

		return TINT( uiB - uiA );
	}

	if ( rList1.IsEmpty() && rList2.IsEmpty() )
		return 0;

	return TINT( !rList1.IsEmpty() ) * 2 - 1;
}

// $deBlob: FUNCTION 0076ade0
void ABlobZJumpComponent::InsertGroup( T2DList<PriorityGroup>& a_rList, PriorityGroup* a_pGroup )
{
	auto it = a_rList.Begin();

	for ( ; it != a_rList.End(); it++ )
	{
		if ( CompareGroups( a_pGroup, it ) < 0 )
			break;
	}

	a_rList.InsertBefore( it, a_pGroup );
}

// $deBlob: FUNCTION 0076aec0
void ABlobZJumpComponent::InsertTarget( T2DList<PriorityTarget>& a_rList, PriorityTarget* a_pTarget )
{
	auto it = a_rList.Begin();

	for ( ; it != a_rList.End(); it++ )
	{
		TUINT32 uiUnlockable   = a_pTarget->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;
		TUINT32 uiItUnlockable = it->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;

		if ( uiUnlockable == uiItUnlockable )
		{
			// The higher priority first, then the nearer
			if ( a_pTarget->m_iPriority != it->m_iPriority )
			{
				if ( TINT( TUINT32( it->m_iPriority ) - TUINT32( a_pTarget->m_iPriority ) ) < 0 )
					break;

				continue;
			}

			if ( a_pTarget->m_fDistance < it->m_fDistance )
				break;
		}
		else if ( TINT( uiUnlockable - uiItUnlockable ) < 0 )
		{
			// The lockable ones first
			break;
		}
	}

	a_rList.InsertBefore( it, a_pTarget );
}

// $deBlob: FUNCTION 0076af90
void ABlobZJumpComponent::InsertGroupTarget( T2DList<PriorityTarget>& a_rList, PriorityTarget* a_pTarget )
{
	auto it = a_rList.Begin();

	for ( ; it != a_rList.End(); it++ )
	{
		TUINT32 uiUnlockable   = a_pTarget->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;
		TUINT32 uiItUnlockable = it->m_uiFlags & PriorityTarget::FLAGS_UNLOCKABLE;

		if ( uiUnlockable == uiItUnlockable )
		{
			if ( a_pTarget->m_fDistance < it->m_fDistance )
				break;
		}
		else if ( TINT( uiUnlockable - uiItUnlockable ) < 0 )
		{
			break;
		}
	}

	a_rList.InsertBefore( it, a_pTarget );
}

// $deBlob: FUNCTION 0076aad0
ABlobZJumpComponent::ZTargetPriority& ABlobZJumpComponent::GetZTargetPriority( TINT a_ePriorityType )
{
	TASSERT( a_ePriorityType < ZPRIORITY_COUNT );
	TASSERT( ZPRIORITY_INVALID < a_ePriorityType );
	return TSTATICCAST( ZTargetPriority, ABlobEntity::ms_pZTargetPriorities )[ a_ePriorityType ];
}

TINT ABlobZJumpComponent::GetPriority( TINT a_ePriorityType, ABlobEntity* a_pBlob )
{
	if ( ( ( a_pBlob->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) != 0 )
		return GetZTargetPriority( a_ePriorityType ).m_iInk;

	if ( a_pBlob->m_iPaintPoints == 0 )
		return GetZTargetPriority( a_ePriorityType ).m_iSmall;

	return GetZTargetPriority( a_ePriorityType ).m_iBig;
}

// $deBlob: FUNCTION 00768730
void ABlobZJumpComponent::PruneGroups()
{
	PriorityTarget* pPrevLock = m_pCurrentPriorityLock;

	for ( auto itGroup = m_lGroups.Begin(); itGroup != m_lGroups.End(); )
	{
		PriorityGroup* pGroup = itGroup;
		itGroup               = itGroup.Next();

		T2DList<PriorityTarget>& rLocks = pGroup->m_lLockList;
		auto                     itEnd  = rLocks.End();

		for ( auto it = rLocks.Begin(); it != itEnd; )
		{
			PriorityTarget* pTarget = it;
			it                      = it.Next();

			if ( IsStillLockable( *pTarget ) )
				continue;

			pTarget->Remove();
			FreeTarget( *pTarget );

			// The lock goes on to the next target of the group
			if ( m_pCurrentPriorityLock == pTarget )
			{
				if ( rLocks.IsEmpty() )
					m_pCurrentPriorityLock = TNULL;
				else if ( it == itEnd )
					m_pCurrentPriorityLock = rLocks.Head();
				else
					m_pCurrentPriorityLock = it;
			}
		}

		if ( rLocks.IsEmpty() )
			pGroup->Remove();
	}

	if ( m_pCurrentPriorityLock != TNULL && pPrevLock != m_pCurrentPriorityLock )
	{
		m_pOwner->GetSoundComponent()->PlayLockOn();
		m_pCurrentPriorityLock->m_pRectangle->SetVisible( TFALSE );
		HideLockElement();
		ShowLock( *m_pCurrentPriorityLock );
	}
}

// $deBlob: FUNCTION 007689d0
void ABlobZJumpComponent::FindNodeTargets()
{
	TINT*             pContact = TREINTERPRETCAST( TINT*, m_uiUnk10 ); // the contact body's user data { type, object }
	AZJumpData::Node* pNode    = TNULL;

	if ( pContact != TNULL )
	{
		if ( pContact[ 0 ] == 2 && TREINTERPRETCAST( ABaseEntity*, pContact[ 1 ] )->GetClass() == &TGetClass( APropPlatform ) )
		{
			// Blob stands on a platform with a Z-jump node
			pNode = TREINTERPRETCAST( APropPlatform*, pContact[ 1 ] )->GetZJumpNode();
		}
		else if ( pContact[ 0 ] == 3 && pContact[ 1 ] != 0 && ( TREINTERPRETCAST( TerrainTypes::CellInstance*, pContact[ 1 ] )->m_uiFlags & 7 ) == 7 )
		{
			// Blob stands on a Z-jump instance: a node with a point other than the one just used
			TerrainTypes::CellInstance* pInstance = TREINTERPRETCAST( TerrainTypes::CellInstance*, pContact[ 1 ] );
			TASSERT( pInstance->zjump.m_iSet >= 0 && pInstance->zjump.m_iNode >= 0 );

			AZJumpData* pData = AZJumpData::GetSingleton();
			pNode             = &pData->GetSet( pInstance->zjump.m_iSet )->m_pNodes[ pInstance->zjump.m_iNode ];

			AZJumpData::Point* pPoint = pData->GetSet( pInstance->zjump.m_iSet )->FindPoint( pNode );

			if ( pPoint == TNULL || pPoint == m_pLastPoint )
				pNode = TNULL;
		}
	}

	if ( pNode == TNULL )
	{
		m_pUnk2C     = TNULL;
		m_pStandNode = TNULL;
		return;
	}

	if ( pNode != m_pStandNode )
	{
		m_pUnk2C     = m_pStandNode;
		m_pStandNode = pNode;
	}

	if ( m_pUnk2C != TNULL && m_pUnk2C->m_pSet != m_pStandNode->m_pSet )
		m_pUnk2C = TNULL;

	TINT iPriority = GetPriority( 0x25, m_pOwner );

	if ( iPriority == -1 )
		return;

	// The instance nodes the node links to
	for ( TINT i = 0; i < 2; i++ )
	{
		AZJumpData::Node* pLink = pNode->m_apLinks[ i ];

		if ( pLink != TNULL && pLink->m_eType == AZJumpData::NodeType_Instance )
		{
			TFLOAT          fWeight = GetNodeWeight( pLink );
			PriorityTarget* pTarget = AllocTarget( fWeight );

			if ( pTarget != TNULL )
			{
				pTarget->m_eType         = PriorityTarget::TARGET_INSTANCE;
				pTarget->m_pTarget       = pLink->m_pInstance;
				pTarget->m_iPriority     = iPriority;
				pTarget->m_ePriorityType = 0x25;
				pTarget->m_uiFlags       = PriorityTarget::FLAGS_UNSORTED | PriorityTarget::FLAGS_LOCKED | PriorityTarget::FLAGS_UNK100;
				pTarget->m_fDistance     = fWeight;
				InsertTarget( m_lTargets, pTarget );
			}
		}
	}
}

// $deBlob: FUNCTION 00768bc0
void ABlobZJumpComponent::FindEntityTargets()
{
	TVector4 vBlobPosition = m_pOwner->GetBodyPosition();
	TINT     iNumEntities;

	ABaseEntity** ppEntities = AEntityHash::GetSingleton()->Query( vBlobPosition, ABlobEntity::GetCommonProperties()->m_afUnk38[ 1 ], &iNumEntities, TNULL, TTRUE, 500, TFALSE, TTRUE, TFALSE );

	TBOOL bInked = ( ( m_pOwner->m_pColourComponent->GetCurrentColour() >> 4 ) & 1 ) != 0;
	TBOOL bSmall = m_pOwner->m_iPaintPoints == 0;

	// The boss is always in reach, checked after the query's entities
	ABaseEntity* pAlwaysInReach = ABoss::GetSingleton();
	TBOOL        bQueried       = TFALSE;

	for ( TINT i = 0; i <= iNumEntities; i++ )
	{
		TBOOL        bNodeTarget = TFALSE;
		ABaseEntity* pEntity;

		if ( i == iNumEntities )
		{
			if ( pAlwaysInReach == TNULL || bQueried )
				continue;

			pEntity = pAlwaysInReach;
		}
		else
		{
			pEntity = ppEntities[ i ];

			if ( pAlwaysInReach == pEntity )
				bQueried = TTRUE;
		}

		if ( !pEntity->IsTargetable() || pEntity == m_pOwner )
			continue;

		TINT ePriorityType = TINT8( pEntity->GetCollisionGroupAndType() >> 8 );
		TINT eRetype       = -1; // the priority type the entity's old target could be in

		if ( ( TUINT8( m_pOwner->m_uiFlags >> 0x15 ) & 1 ) != 0 )
		{
			switch ( ePriorityType )
			{
				case 5:
				case 6:
				case 7:
				case 8:
				case 9:
				case 10:
				case 11:
				case 12:
				case 14:
				case 15:
					continue;
			}
		}

		if ( ePriorityType == -1 )
			continue;

		if ( ePriorityType == 1 )
		{
			// A platform with a Z-jump node: 0x25 when it's the next node of the set Blob is on
			APropPlatform* pPlatform = TSTATICCAST( APropPlatform, pEntity );
			TASSERT( pPlatform == TNULL || pPlatform->IsA( &TGetClass( APropPlatform ) ) );

			AZJumpData::Node* pNode = pPlatform->GetZJumpNode();

			if ( pNode != TNULL && m_pLastPoint != TNULL && pNode->m_pSet == m_pLastPoint->m_pSet )
				continue;

			if ( !IsNextNode( pNode, m_pStandNode ) )
			{
				eRetype = 0x25;
			}
			else
			{
				ePriorityType = 0x25;
				bNodeTarget   = TTRUE;
				eRetype       = 1;
			}
		}

		ZTargetPriority& rPriority = GetZTargetPriority( ePriorityType );
		TINT             iPriority = bInked ? rPriority.m_iInk : ( bSmall ? rPriority.m_iSmall : rPriority.m_iBig );

		if ( iPriority == -1 )
			continue;

		if ( ePriorityType == 5 )
		{
			if ( AGameStatus::GetSingleton()->GetMultiPlayerGameType() == 1 )
			{
				ABaseEntity* pPainted = APlayerManager::GetSingleton()->GetPaintedBlob();

				if ( pPainted != TNULL && pPainted != m_pOwner )
					continue;
			}
		}
		else if ( ePriorityType == 0x24 )
		{
			TASSERT( pEntity->IsFlagged( ABaseEntity::FLAG_Player ), "pEntity->IsPlayer()" );

			ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pEntity );
			TASSERT( pBlob == TNULL || pBlob->IsA( &TGetClass( ABlobEntity ) ) );

			if ( !m_pOwner->CanInkBlob( pBlob ) )
				continue;
		}

		// Not the platform (or car) Blob stands on
		const TClass* pClass = pEntity->GetClass();

		if ( ( pClass == &TGetClass( APropPlatform ) || ( pEntity->GetClass() == &TGetClass( ACar ) && TSTATICCAST( ACar, pEntity )->GetCarType() != 7 ) ) &&
		     TSTATICCAST( AVisibleEntity, pEntity )->GetPhysicsData() != TNULL )
		{
			TPhysicsObjectContainer* pBody = TSTATICCAST( AVisibleEntity, pEntity )->GetPhysicsData()->m_pBody;

			if ( pBody != TNULL && pBody == TREINTERPRETCAST( TPhysicsObjectContainer*, m_pOwner->m_oMotion.m_auiUnk280[ 0 ] ) )
				continue;
		}

		// Already a target?
		TBOOL                    bFound = TFALSE;
		T2DList<PriorityTarget>& rLocks = m_pGroups[ ePriorityType ].m_lLockList;

		for ( auto it = rLocks.Begin(); it != rLocks.End(); it++ )
		{
			if ( it->m_eType == PriorityTarget::TARGET_ENTITY && it->m_pTarget == pEntity && ( !pEntity->IsPooled() || pEntity->GetPooledID() == it->m_iIndex ) )
			{
				bFound = TTRUE;
				break;
			}
		}

		if ( eRetype != -1 )
		{
			// The platform's old target changes its priority type
			T2DList<PriorityTarget>& rOldLocks = m_pGroups[ eRetype ].m_lLockList;
			TBOOL                    bMoved    = TFALSE;

			for ( auto it = rOldLocks.Begin(); it != rOldLocks.End(); it++ )
			{
				PriorityTarget* pTarget = it;

				if ( pTarget->m_eType == PriorityTarget::TARGET_ENTITY && pTarget->m_pTarget == pEntity && ( !pEntity->IsPooled() || pEntity->GetID() == pTarget->m_iIndex ) )
				{
					pTarget->Remove();
					pTarget->m_ePriorityType = ePriorityType;
					pTarget->m_iPriority     = iPriority;

					if ( ePriorityType == 0x25 )
					{
						APropPlatform* pPlatform = TSTATICCAST( APropPlatform, pEntity );
						TASSERT( pPlatform == TNULL || pPlatform->IsA( &TGetClass( APropPlatform ) ) );

						pTarget->m_fDistance = GetNodeWeight( pPlatform->GetZJumpNode() );
					}
					else
					{
						pTarget->m_fDistance = TVector4::DistanceSq( vBlobPosition, pEntity->GetMatrix().GetTranslation() );
					}

					InsertGroupTarget( rLocks, pTarget );

					if ( !m_pGroups[ ePriorityType ].IsLinked() )
						InsertGroup( m_lGroups, &m_pGroups[ ePriorityType ] );

					bMoved = TTRUE;
					break;
				}
			}

			if ( bMoved )
				continue;
		}

		if ( bFound )
			continue;

		const TVector4& rEntityPosition = pEntity->GetMatrix().GetTranslation();
		TVector4        vDelta( vBlobPosition.x - rEntityPosition.x, vBlobPosition.y - rEntityPosition.y, vBlobPosition.z - rEntityPosition.z, vBlobPosition.w );

		if ( !bNodeTarget )
		{
			// Not too far below Blob (an enemy that Blob can't land on from below unless CanBeTargetedFromBelow)
			if ( !pEntity->IsFlagged( ABaseEntity::FLAG_Player ) )
			{
				if ( vDelta.y > m_pOwner->GetBoundingSphereRadius() && !pEntity->CanBeTargetedFromBelow() )
					continue;
			}
			else
			{
				TASSERT( pEntity->IsA( &TGetClass( ABlobEntity ) ) );

				ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pEntity );
				TASSERT( pBlob == TNULL || pBlob->IsA( &TGetClass( ABlobEntity ) ) );

				if ( vDelta.y > ( pBlob->GetBoundingSphereRadius() + m_pOwner->GetBoundingSphereRadius() ) * 2.0f )
					continue;
			}
		}

		TFLOAT fDistSq = vDelta.MagnitudeSq();

		if ( !m_pOwner->GetViewCamera()->IsInViewCone( pEntity->GetMatrix().GetTranslation(), pEntity->GetBoundingSphereRadius(), 200.0f ) )
			continue;

		PriorityTarget* pTarget = AllocTarget( fDistSq );

		if ( pTarget == TNULL )
			continue;

		pTarget->m_eType   = PriorityTarget::TARGET_ENTITY;
		pTarget->m_pTarget = pEntity;
		pTarget->m_iIndex  = pEntity->IsPooled() ? pEntity->GetPooledID() : -1;
		pTarget->m_uiFlags = pEntity->GetTargetColour();

		// The lock stays on players, platforms, pressure switches and some cars
		const TClass* pTargetClass = pEntity->GetClass();

		if ( pEntity->IsFlagged( ABaseEntity::FLAG_Player ) || pTargetClass == &TGetClass( APropPlatform ) || pEntity->GetClass() == &TGetClass( APropSwitchPressure ) ||
		     ( pEntity->GetClass() == &TGetClass( ACar ) && ( TSTATICCAST( ACar, pEntity )->GetCarType() == 0 || ( 4 < TSTATICCAST( ACar, pEntity )->GetCarType() && TSTATICCAST( ACar, pEntity )->GetCarType() < 7 ) ) ) )
			pTarget->m_uiFlags |= PriorityTarget::FLAGS_LOCKED;

		pTarget->m_iPriority     = iPriority;
		pTarget->m_ePriorityType = ePriorityType;

		switch ( ePriorityType )
		{
			case 1:
			case 2:
			case 3:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
			case 0x25:
				pTarget->m_uiFlags |= PriorityTarget::FLAGS_UNK100;
		}

		if ( pEntity->IsA( &TGetClass( APropTrigger ) ) && pEntity->GetLife() == 0 )
			pTarget->m_uiFlags |= PriorityTarget::FLAGS_UNK100;

		UpdateTargetFlags( *pTarget );

		if ( ePriorityType == 0x25 )
		{
			APropPlatform* pPlatform = TSTATICCAST( APropPlatform, pEntity );
			TASSERT( pPlatform == TNULL || pPlatform->IsA( &TGetClass( APropPlatform ) ) );

			pTarget->m_fDistance = GetNodeWeight( pPlatform->GetZJumpNode() );
		}
		else
		{
			pTarget->m_fDistance = fDistSq;
		}

		pTarget->m_uiFlags |= PriorityTarget::FLAGS_UNSORTED;
		InsertTarget( m_lTargets, pTarget );
	}
}
