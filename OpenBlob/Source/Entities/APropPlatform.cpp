#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "APropPlatform.h"
#include "AEntityParams.h"
#include "AEntityLoader.h"
#include "AEntityModelComponent.h"
#include "AEntityPhysicsMover.h"
#include "AEntityHash.h"
#include "ANavMarkers.h"
#include "ACharEntity.h"
#include "Enemies/AEnemy.h"
#include "AGndMotorComponent.h"
#include "AEnemyWrapper/AEnemyWrapper.h"
#include "Terrain2/TerrainTypes.h"
#include "Buildings/ABuildingInstance.h"
#include "Render/Material/AMaterialInfo.h"
#include "Render/Material/AMaterialManager.h"
#include "Render/UVAnimation/AUVAnimation.h"
#include "Shaders/ASkinShader/ASkinShader.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include <Platform/DX11/TMaterial_DX11.h>
#include <Platform/DX11/TWorldShader_DX11.h>
#include <Render/TMaterial.h>
#include <Render/TModel.h>
#include "AEntityIOPacket.h"
#include "AAttackMessage.h"
#include "ABlobEntity/ABlobEntity.h"
#include "AZJumpData.h"
#include "Collision/ACollisionInstance2.h"
#include <Physics/TBoxData.h>
#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include "Animation/AModelInstance2.h"
#include "ARandom.h"
#include "Sound/ASoundInterface.h"
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include "AEntityFactory.h"
#include <Render/TAnimation.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00793f00
// $deBlob: FUNCTION 00793f10
// $deBlob: FUNCTION 00793f80
// $deBlob: FUNCTION 00793fe0
// $deBlob: FUNCTION 00793ff0
// $deBlob: FUNCTION 00540ce0
// $deBlob: FUNCTION 00859dc0
TDEFINE_CLASS( APropPlatform );

// $deBlob: FUNCTION 00794000
APropPlatform::APropPlatform()
{
	m_vCurPointPos = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	m_vVelocity    = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	TINT16* pSounds = &m_oMovingSound.iGroup;
	for ( TINT i = 0; i < 10; i++ )
		pSounds[ i ] = -1;

	m_iIdleAnim         = -1;
	m_iTouchAnim        = -1;
	m_iCurPoint         = -1;
	m_iStartPoint       = -1;
	m_iHitPoints        = -1;
	m_uiPlatformFlags   = PLATFORMFLAG_Reverse;
	m_pIdleAnim         = TNULL;
	m_fSpeed            = 0.3f;
	m_fPathLength       = 0;
	m_pWaypointSet      = TNULL;
	m_pConveyor         = TNULL;
	m_fSplineT          = 0;
	m_fNodeDelay        = 0;
	m_fSpeedScale       = 0;
	m_fTouchCooldown    = 0;
	m_pDistanceSpring   = TNULL;
	m_fDistance         = 0;
	m_fAccelDown        = 20.0f;
	m_fAccelUp          = 1.0f;
	m_fDampeningSpeed   = 0.2f;
	m_pOscillation      = TNULL;
	m_fDamageRadius     = 0;
	m_iDamage           = 0;
	m_iRecoil           = 0;
	m_iSoundState       = 5;
	m_fAttackSlamHeight = 0.0f;
	m_pTargetOffsets    = TNULL;
	m_fDelayOnTrigger   = 0;
	m_fDelay            = 0;
	m_Flags |= 0x40;
	m_pBounce                   = TNULL;
	m_pDespawnEffect            = TNULL;
	m_pZJumpNode                = TNULL;
	m_pEffectData               = TNULL;
	m_eDelayState               = 0;
	m_uiExtraFlags              = 0;
	m_oPlatformNode.m_pPlatform = this;
	m_oPlatformNode.m_Group     = TNULL;
	m_iCollisionGroupAndType    = TINT16( ( m_iCollisionGroupAndType & 0xFF ) | ( 1 << 8 ) );
}

// $deBlob: FUNCTION 007941e0
// $deBlob: FUNCTION 0079a320
// $deBlob: FUNCTION 0079a315
APropPlatform::~APropPlatform()
{
	if ( m_pConveyor != TNULL )
	{
		if ( m_pConveyor->ppMaterials != TNULL )
		{
			// The conveyor's copies of the materials go and the meshes get theirs back
			AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
			T2ModelPtr       pModel         = pModelInstance->GetModelInstance()->GetModel();
			TModelLOD&       rLOD           = pModel->GetLOD( 0 );

			for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
			{
				TMesh* pMesh = rLOD.GetMeshes()[ i ];

				if ( pMesh->IsA( &TGetClass( ASkinMesh ) ) )
				{
					TSTATICCAST( ASkinMesh, pMesh )->SetMaterial2( TNULL );
					pModelInstance->SetFlag( AModelInstance2::FLAG( 0x100 ), TFALSE );
				}
				else
				{
					pModelInstance->SetOverrideMaterial( TNULL );
				}

				TMaterial* pMaterial = m_pConveyor->ppMaterials[ i ];

				if ( pMaterial != TNULL )
				{
					pMaterial->Destroy();
					delete pMaterial;
					m_pConveyor->ppMaterials[ i ] = TNULL;
				}
			}

			::operator delete( m_pConveyor->ppMaterials );
		}

		::operator delete( m_pConveyor->pPasses );
		::operator delete( m_pConveyor );
	}

	::operator delete( m_pTargetOffsets );

	if ( m_pOscillation != TNULL )
		::operator delete( m_pOscillation );

	if ( m_pDistanceSpring != TNULL )
		::operator delete( m_pDistanceSpring );

	if ( m_pEffectData != TNULL )
		::operator delete( m_pEffectData );

	if ( m_pBounce != TNULL )
		delete m_pBounce;

	TASSERT( !m_oPlatformNode.IsLinked() );
}

// $deBlob: FUNCTION 00794470
void APropPlatform::Tick( TFLOAT a_fDeltaTime )
{
	m_vVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	AProp::Tick( a_fDeltaTime );

	if ( 0.0f < m_fTouchCooldown )
	{
		m_fTouchCooldown -= a_fDeltaTime;

		if ( ( m_uiPlatformFlags & PLATFORMFLAG_Bounce ) && m_fTouchCooldown <= 0.0f )
		{
			if ( m_pBounce != TNULL )
				m_pBounce->oSpring.m_fRest = 0.0f;

			if ( m_pOscillation != TNULL )
				m_pOscillation->m_fAmplitude = 1.0f;
		}
	}

	// The idle animation (restarted when the touch animation ends)
	TBOOL bAnimated;

	if ( GetLOD() == FLAG_HighLOD || ( m_uiPlatformFlags & PLATFORMFLAG_IdleAnimAlways ) )
	{
		bAnimated = TTRUE;

		TBOOL bStart = TFALSE;

		if ( m_pIdleAnim == TNULL && m_iIdleAnim >= 0 )
			bStart = TTRUE;
		else if ( m_iTouchAnim >= 0 && m_iIdleAnim >= 0 && m_pIdleAnim != TNULL && m_pIdleAnim->GetSequence() == m_iTouchAnim && m_pIdleAnim->GetSequencePtr()->GetDuration() <= m_pIdleAnim->GetTotalTime() )
			bStart = TTRUE;

		if ( bStart )
		{
			m_pIdleAnim      = AddManualAnimation( m_iIdleAnim, 0.0f, 0.15f, 1.0f, 1.0f );
			TFLOAT fDuration = m_pIdleAnim->GetSequencePtr()->GetDuration();
			m_pIdleAnim->SetTime( ARandom::GetSingleton()->GetRandom().GetFloat() * fDuration );
		}
	}
	else
	{
		bAnimated = TFALSE;
	}

	if ( m_eDelayState != 0 )
	{
		if ( m_eDelayState == 1 )
		{
			// Waiting at a node
			if ( !( m_uiPlatformFlags & PLATFORMFLAG_Stopped ) && !( m_uiPlatformFlags & PLATFORMFLAG_Delayed ) )
			{
				TFLOAT      fOldDelay = m_fNodeDelay;
				EffectData* pEffect   = m_pEffectData;
				m_fNodeDelay          = fOldDelay - a_fDeltaTime;

				if ( pEffect != TNULL && pEffect->pEffect != TNULL && pEffect->fWarningTime < fOldDelay && m_fNodeDelay <= pEffect->fWarningTime )
				{
					TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( pEffect->pEffect );

					if ( m_pEffectData->oSound.iEvent != -1 && m_pEffectData->oSound.iGroup != -1 )
						ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_pEffectData->oSound.iGroup ), ASoundMap::EVENTID( m_pEffectData->oSound.iEvent ), GetMatrix().GetTranslation(), 0 );

					if ( pInstance != TNULL )
						pInstance->GetTransform() = GetMatrix();
				}

				if ( m_fNodeDelay <= 0.0f )
				{
					if ( m_pWaypointSet != TNULL && m_iSoundState != 1 )
					{
						StopIdleSound();
						PlayMovingSound();

						if ( m_iSoundState == 0 )
							PlayStartSound();

						m_iSoundState = 1;
					}

					m_eDelayState = 0;
				}
			}

			if ( m_uiPlatformFlags & PLATFORMFLAG_Delayed )
			{
				m_fDelay -= a_fDeltaTime;

				if ( m_fDelay <= 0.0f )
				{
					m_fDelay = 0.0f;

					if ( m_pWaypointSet != TNULL && m_iSoundState != 1 )
					{
						StopIdleSound();
						PlayMovingSound();

						if ( m_iSoundState == 0 )
							PlayStartSound();

						m_iSoundState = 1;
					}

					m_uiPlatformFlags &= ~PLATFORMFLAG_Delayed;
					m_eDelayState = 0;
				}
			}

			TUINT32 uiFlags = m_uiPlatformFlags;

			if ( ( uiFlags & PLATFORMFLAG_Distance ) || ( uiFlags & PLATFORMFLAG_Oscillation ) || ( uiFlags & PLATFORMFLAG_Bounce ) )
			{
				TVector4 vPosition = GetMatrix().GetTranslation();

				if ( uiFlags & PLATFORMFLAG_Distance )
				{
					vPosition.y -= m_pDistanceSpring->m_fOffset;
					vPosition.y = m_pDistanceSpring->Tick( a_fDeltaTime ) + vPosition.y;
				}

				if ( m_uiPlatformFlags & PLATFORMFLAG_Oscillation )
				{
					vPosition.y -= m_pOscillation->m_fOffset;
					vPosition.y = m_pOscillation->Tick( a_fDeltaTime ) + vPosition.y;
				}

				if ( m_pBounce != TNULL )
				{
					vPosition.y -= m_pBounce->oSpring.m_fPosition;
					m_pBounce->oSpring.Tick( a_fDeltaTime );

					if ( m_pBounce->fMaxSpringTravel < m_pBounce->oSpring.m_fPosition )
					{
						m_pBounce->oSpring.m_fPosition = m_pBounce->fMaxSpringTravel;
						m_pBounce->oSpring.m_fVelocity = 0.0f;
					}

					vPosition.y = m_pBounce->oSpring.m_fPosition + vPosition.y;
				}

				SetPosition( vPosition );
			}
		}

		UpdateConveyor();
		return;
	}

	if ( m_pWaypointSet == TNULL && ( m_uiPlatformFlags & PLATFORMFLAG_Distance ) )
	{
		// A drop without a path: the sounds follow the spring
		TFLOAT fSign = ( m_pDistanceSpring->m_uiFlags & PlatformSpring::FLAG_Rising ) ? -1.0f : 1.0f;

		if ( TMath::Abs( m_pDistanceSpring->m_fVelocity * fSign ) <= 0.01f )
		{
			if ( m_iSoundState != 0 )
			{
				StopMovingSound();
				PlayIdleSound();

				if ( m_iSoundState == 1 )
					PlayStopSound();

				m_iSoundState = 0;
			}
		}
		else if ( m_iSoundState != 1 )
		{
			StopIdleSound();
			PlayMovingSound();

			if ( m_iSoundState == 0 )
				PlayStartSound();

			m_iSoundState = 1;
		}
	}

	TVector4 vOldPos = GetMatrix().GetTranslation();
	TUINT32  uiFlags = m_uiPlatformFlags;

	if ( uiFlags & PLATFORMFLAG_Distance )
		vOldPos.y -= m_pDistanceSpring->m_fOffset;

	if ( uiFlags & PLATFORMFLAG_Oscillation )
		vOldPos.y -= m_pOscillation->m_fOffset;

	if ( ( uiFlags & PLATFORMFLAG_Bounce ) && m_pBounce != TNULL )
		vOldPos.y -= m_pBounce->oSpring.m_fPosition;

	TBOOL    bBackwards  = TFALSE;
	TBOOL    bReverse    = TFALSE;
	TBOOL    bTurnAround = TFALSE;
	TVector4 vNewPos     = vOldPos;

	if ( m_pWaypointSet != TNULL && TMath::TFLOAT_EPSILON < m_fSpeed )
	{
		TFLOAT fOldT = m_fSplineT;
		bBackwards   = ( uiFlags & PLATFORMFLAG_Backwards ) != 0;
		bReverse     = bBackwards;

		TFLOAT fStep = m_fSpeed * a_fDeltaTime * m_fSpeedScale;

		if ( !bBackwards )
			m_fSplineT = TMath::Min( fStep, 1.0f - fOldT ) + fOldT;
		else
			m_fSplineT = fOldT - TMath::Min( fStep, fOldT );

		TBOOL bBlocked = TFALSE;

		if ( QueuesBehindOthers() )
		{
			TINT   iFrom;
			TINT   iTo;
			TFLOAT fLeft;

			if ( !bBackwards )
			{
				iTo   = m_iNextPoint;
				iFrom = m_iCurPoint;
				fLeft = 1.0f - m_fSplineT;
			}
			else
			{
				iTo   = m_iCurPoint;
				iFrom = m_iNextPoint;
				fLeft = m_fSplineT;
			}

			// Another platform of the set is ahead on the same link
			bBlocked = APlatformManager::IsPathBlocked( TSTATICCAST( APlatformManager::Group, m_oPlatformNode.m_Group ), GetID(), iFrom, iTo, ( m_fPathLength * fLeft ) / m_fSpeedScale );

			if ( bBlocked )
				m_fSplineT = fOldT;
		}

		if ( !bBlocked )
		{
			if ( !( m_uiPlatformFlags & PLATFORMFLAG_Linear ) )
			{
				if ( !bAnimated && !( m_uiExtraFlags & 2 ) )
				{
					// Low LOD: heads straight for the end of the segment
					const TVector4& rTarget = bBackwards ? m_oSpline.vStart : m_oSpline.vEnd;

					TVector4 vDirection( rTarget.x - vOldPos.x, rTarget.y - vOldPos.y, rTarget.z - vOldPos.z, rTarget.w );
					TFLOAT   fDistance = vDirection.NormaliseMag();
					TFLOAT   fMove     = a_fDeltaTime * m_fSpeed * m_fPathLength;

					if ( fDistance <= fMove )
					{
						vNewPos = rTarget;
					}
					else
					{
						TVector4 vMove;
						vMove.Multiply( vDirection, fMove );
						vNewPos = TVector4( vMove.x + vOldPos.x, vMove.y + vOldPos.y, vMove.z + vOldPos.z, vOldPos.w );
					}

					if ( m_uiPlatformFlags & PLATFORMFLAG_TurnAround )
					{
						const TVector4& rFrom = bBackwards ? m_oSpline.vEnd : m_oSpline.vStart;

						TVector4 vFromStart( vNewPos.x - rFrom.x, vNewPos.y - rFrom.y, vNewPos.z - rFrom.z, vNewPos.w );
						vFromStart.Normalise();

						if ( 0.0f <= vFromStart.y * vDirection.y + vFromStart.x * vDirection.x + vFromStart.z * vDirection.z )
							m_uiPlatformFlags &= ~PLATFORMFLAG_TurnAround;
						else
							bTurnAround = TTRUE;
					}
				}
				else
				{
					TMath::HermitePosition( vNewPos, m_oSpline.vStart, m_oSpline.vStartTangent, m_oSpline.vEnd, m_oSpline.vEndTangent, m_fSplineT );

					if ( !( m_uiExtraFlags & 2 ) )
					{
						TVector4 vDirection( vNewPos.x - vOldPos.x, vNewPos.y - vOldPos.y, vNewPos.z - vOldPos.z, vNewPos.w );
						TFLOAT   fDistance = vDirection.NormaliseMag();
						TFLOAT   fMove     = a_fDeltaTime * m_fSpeed * m_fPathLength;

						if ( fMove < fDistance )
						{
							TVector4 vMove;
							vMove.Multiply( vDirection, fMove );
							vNewPos = TVector4( vMove.x + vOldPos.x, vMove.y + vOldPos.y, vMove.z + vOldPos.z, vOldPos.w );
						}

						if ( m_uiPlatformFlags & PLATFORMFLAG_TurnAround )
						{
							const TVector4& rFrom = bBackwards ? m_oSpline.vEnd : m_oSpline.vStart;

							if ( 0.0f <= ( vNewPos.y - rFrom.y ) * vDirection.y + ( vNewPos.x - rFrom.x ) * vDirection.x + ( vNewPos.z - rFrom.z ) * vDirection.z )
								m_uiPlatformFlags &= ~PLATFORMFLAG_TurnAround;
							else
								bTurnAround = TTRUE;
						}
					}
				}
			}
			else
			{
				vNewPos.Lerp4( m_oSpline.vStart, m_oSpline.vEnd, m_fSplineT );
			}
		}
	}

	if ( m_uiPlatformFlags & PLATFORMFLAG_Distance )
		vNewPos.y = m_pDistanceSpring->Tick( a_fDeltaTime ) + vNewPos.y;

	if ( m_uiPlatformFlags & PLATFORMFLAG_Oscillation )
		vNewPos.y = m_pOscillation->Tick( a_fDeltaTime ) + vNewPos.y;

	if ( ( m_uiPlatformFlags & PLATFORMFLAG_Bounce ) && m_pBounce != TNULL )
	{
		m_pBounce->oSpring.Tick( a_fDeltaTime );

		if ( m_pBounce->fMaxSpringTravel < m_pBounce->oSpring.m_fPosition )
		{
			m_pBounce->oSpring.m_fPosition = m_pBounce->fMaxSpringTravel;
			m_pBounce->oSpring.m_fVelocity = 0.0f;
		}

		vNewPos.y = m_pBounce->oSpring.m_fPosition + vNewPos.y;
	}

	SetPosition( vNewPos );

	if ( m_pWaypointSet != TNULL )
	{
		TINT16 iReachedNode = -1;

		if ( !bBackwards )
		{
			if ( 1.0f <= m_fSplineT )
				iReachedNode = TINT16( m_iNextPoint );
		}
		else if ( m_fSplineT <= 0.0f )
		{
			iReachedNode = TINT16( m_iCurPoint );
		}

		if ( iReachedNode != -1 )
		{
			// Reached the end of the segment
			ToggleNavSets( iReachedNode );

			if ( m_pWaypointSet->GetNode( iReachedNode ).uiFlags & AWaypointData::Node::FLAG_Trigger )
				ATriggerUtil::TriggerAllTargets( TNULL );

			if ( m_pWaypointSet->GetNode( iReachedNode ).uiFlags & AWaypointData::Node::FLAG_TurretChange )
				SetTurretActive( ( m_pWaypointSet->GetNode( iReachedNode ).uiFlags & AWaypointData::Node::FLAG_TurretOn ) != 0 );

			TINT                 iNextNode;
			AWaypointData::Node* pNextNode = GetNextNode( TINT16( bBackwards ? m_iCurPoint : m_iNextPoint ), bReverse, ( m_uiPlatformFlags & PLATFORMFLAG_Toggled ) != 0, &iNextNode );

			if ( pNextNode == TNULL )
			{
				TUINT32 uiPathFlags = m_uiPlatformFlags;

				if ( !( uiPathFlags & PLATFORMFLAG_Reverse ) )
				{
					if ( !( uiPathFlags & PLATFORMFLAG_Continuous ) )
					{
						if ( uiPathFlags & PLATFORMFLAG_Reset )
						{
							RestartFromStart();
							UpdateConveyor();
							return;
						}

						if ( uiPathFlags & PLATFORMFLAG_ResetToRoot )
						{
							RestartFromRoot();
							UpdateConveyor();
							return;
						}

						m_uiPlatformFlags = uiPathFlags | PLATFORMFLAG_Stopped;
						StopMoving();
					}
					else
					{
						// Continuous: goes on from the start node
						TVector4 vPrev = m_vCurPointPos;
						m_iCurPoint    = m_iNextPoint;
						m_vCurPointPos = m_vNextPointPos;
						m_iNextPoint   = m_pWaypointSet->GetStartNode();

						m_vNextPointPos = m_pWaypointSet->GetNode( TINT16( m_iNextPoint ) ).vPosition;
						AdjustNodePosition( &m_vNextPointPos, &m_vCurPointPos, &m_pWaypointSet->GetNode( TINT16( m_iCurPoint ) ).vPosition, TFALSE );

						m_fSplineT = 0.0f;
						SetSplineForward( vPrev, m_vCurPointPos, m_vNextPointPos, TINT16( m_iNextPoint ), &m_oSpline, TTRUE );
						ComputeSpeedScale( m_vCurPointPos, m_vNextPointPos );
						StartNodeDelay( TINT16( m_iCurPoint ) );
						ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 0x23 ), ASoundMap::EVENTID( 0x19A ), GetMatrix().GetTranslation(), 0 );
					}
				}
				else
				{
					// Reverse: turns around at the ends
					TBOOL bTurn = TTRUE;

					if ( !bBackwards )
					{
						StartNodeDelay( TINT16( m_iCurPoint ) );
						m_fSplineT  = 1.0f;
						uiPathFlags = m_uiPlatformFlags | PLATFORMFLAG_Backwards;
					}
					else if ( uiPathFlags & PLATFORMFLAG_Continuous )
					{
						StartNodeDelay( TINT16( m_iNextPoint ) );
						m_fSplineT  = 0.0f;
						uiPathFlags = m_uiPlatformFlags & ~PLATFORMFLAG_Backwards;
					}
					else
					{
						bTurn = TFALSE;
					}

					if ( bTurn )
					{
						m_uiPlatformFlags = uiPathFlags;

						if ( !( m_uiExtraFlags & 2 ) )
							m_uiPlatformFlags = uiPathFlags ^ PLATFORMFLAG_TurnAround;
					}
				}
			}
			else if ( !bBackwards )
			{
				TVector4 vPrev = m_vCurPointPos;
				m_iCurPoint    = m_iNextPoint;
				m_vCurPointPos = m_vNextPointPos;
				m_iNextPoint   = iNextNode;

				m_vNextPointPos = m_pWaypointSet->GetNode( TINT16( iNextNode ) ).vPosition;
				AdjustNodePosition( &m_vNextPointPos, &m_vCurPointPos, &m_pWaypointSet->GetNode( TINT16( m_iCurPoint ) ).vPosition, TFALSE );

				m_fSplineT = 0.0f;
				SetSplineForward( vPrev, m_vCurPointPos, m_vNextPointPos, TINT16( m_iNextPoint ), &m_oSpline, TTRUE );
				ComputeSpeedScale( m_vCurPointPos, m_vNextPointPos );
				StartNodeDelay( TINT16( m_iCurPoint ) );
			}
			else
			{
				TVector4 vAfter = m_vNextPointPos;
				m_iNextPoint    = m_iCurPoint;
				m_vNextPointPos = m_vCurPointPos;
				m_iCurPoint     = iNextNode;

				m_vCurPointPos = m_pWaypointSet->GetNode( TINT16( iNextNode ) ).vPosition;
				AdjustNodePosition( &m_vCurPointPos, &m_vNextPointPos, &m_pWaypointSet->GetNode( TINT16( m_iNextPoint ) ).vPosition, TFALSE );

				m_fSplineT = 1.0f;
				SetSplineReverse( vAfter, m_vCurPointPos, m_vNextPointPos, TINT16( m_iCurPoint ), &m_oSpline, TTRUE );
				ComputeSpeedScale( m_vCurPointPos, m_vNextPointPos );
				StartNodeDelay( TINT16( m_iNextPoint ) );
			}
		}
	}

	if ( bTurnAround )
		bBackwards = !bBackwards;

	if ( !bBackwards )
		m_vVelocity = TVector4( vNewPos.x - vOldPos.x, vNewPos.y - vOldPos.y, vNewPos.z - vOldPos.z, vNewPos.w );
	else
		m_vVelocity = TVector4( vOldPos.x - vNewPos.x, vOldPos.y - vNewPos.y, vOldPos.z - vNewPos.z, vOldPos.w );

	m_vVelocity.Divide( a_fDeltaTime );

	if ( m_pWaypointSet != TNULL )
	{
		if ( !bBackwards )
			UpdateOrientation( vOldPos, vNewPos );
		else
			UpdateOrientation( vNewPos, vOldPos );
	}

	UpdateConveyor();
}

// The end of Tick: the conveyor belt moves what's on it
void APropPlatform::UpdateConveyor()
{
	if ( !( m_uiPlatformFlags & PLATFORMFLAG_Conveyor ) )
		return;

	const TMatrix44& rMatrix = GetMatrix();
	m_vVelocity              = TVector4( rMatrix.m_f31, rMatrix.m_f32, rMatrix.m_f33, rMatrix.m_f34 );
	m_vVelocity.w            = 1.0f;
	m_vVelocity.Normalise();
	m_vVelocity.Multiply( m_pConveyor->fCurrentSpeed );

	// The surface velocity of the platform's bodies (what stands on them is carried)
	ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetCollisionInstance() );

	if ( pCollision == TNULL )
	{
		TPhysicsManager::GetSingleton()->SetSurfaceVelocity( m_pPhysicsData->m_pBody, m_vVelocity );
	}
	else
	{
		for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
		{
			TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

			if ( pBody != TNULL )
			{
				TPhysicsManager::GetSingleton()->SetSurfaceVelocity( pBody, m_vVelocity );
			}
		}
	}
}

// $deBlob: FUNCTION 00795520
// $deBlob: FUNCTION 0079a59d
void APropPlatform::ProcessParams( const AEntityParams& a_rParams )
{
	AProp::ProcessParams( a_rParams );

	if ( ( m_Flags >> 29 ) & 1 )
		TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetFlag( AModelInstance2::FLAG( 0x8000 ), TTRUE );

	const TCHAR* szDespawnEffect = m_pTypeData->m_pTRBData->m_szDespawnEffect;

	if ( szDespawnEffect != TNULL && *szDespawnEffect != '\0' )
	{
		m_pDespawnEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( szDespawnEffect );
		TASSERT( TDebug_IsValidAddress( m_pDespawnEffect ), "Couldn't find despawn particle effect %s for platform!" );
	}

	TBOOL bForceAlwaysHighLOD = TFALSE;
	a_rParams.GetValue( bForceAlwaysHighLOD, "ForceAlwaysHighLOD" );

	if ( bForceAlwaysHighLOD )
		m_uiPlatformFlags |= PLATFORMFLAG_IdleAnimAlways | PLATFORMFLAG_ForceAlwaysTick;

	TBOOL bForceAlwaysTick = TFALSE;
	a_rParams.GetValue( bForceAlwaysTick, "ForceAlwaysTick" );

	if ( bForceAlwaysTick )
		m_uiPlatformFlags |= PLATFORMFLAG_ForceAlwaysTick;

	TBOOL bHashed = TFALSE;

	if ( a_rParams.GetValue( bHashed, "Hashed" ) && !bHashed )
	{
		m_uiExtraFlags |= 1;
		SetHash( TFALSE );
	}

	a_rParams.GetValue( TREINTERPRETCAST( TUINT16&, m_oIdleSound.iGroup ), TREINTERPRETCAST( TUINT16&, m_oIdleSound.iEvent ), "IdleSoundID" );
	a_rParams.GetValue( TREINTERPRETCAST( TUINT16&, m_oOnActiveSound.iGroup ), TREINTERPRETCAST( TUINT16&, m_oOnActiveSound.iEvent ), "OnActiveSoundID" );
	a_rParams.GetValue( TREINTERPRETCAST( TUINT16&, m_oStartSound.iGroup ), TREINTERPRETCAST( TUINT16&, m_oStartSound.iEvent ), "StartSoundID" );
	a_rParams.GetValue( TREINTERPRETCAST( TUINT16&, m_oMovingSound.iGroup ), TREINTERPRETCAST( TUINT16&, m_oMovingSound.iEvent ), "MovingSoundID" );
	a_rParams.GetValue( TREINTERPRETCAST( TUINT16&, m_oStopSound.iGroup ), TREINTERPRETCAST( TUINT16&, m_oStopSound.iEvent ), "StopSoundID" );
	a_rParams.GetValue( m_fSpeed, "Speed" );
	a_rParams.GetValue( m_fDistance, "Distance" );
	a_rParams.GetValue( m_fAccelDown, "AccelDown" );
	a_rParams.GetValue( m_fAccelUp, "AccelUp" );
	a_rParams.GetValue( m_fDampeningSpeed, "DampeningSpeed" );

	TBOOL bValue = TFALSE;

	if ( a_rParams.GetValue( bValue, "BobOnEnd" ) && bValue == TTRUE )
		m_uiPlatformFlags |= PLATFORMFLAG_BobOnEnd;

	a_rParams.GetValue( bValue, "Linear" );
	SetPlatformFlag( PLATFORMFLAG_Linear, bValue );
	a_rParams.GetValue( bValue, "Continuous" );
	SetPlatformFlag( PLATFORMFLAG_Continuous, bValue );
	a_rParams.GetValue( bValue, "Reset" );
	SetPlatformFlag( PLATFORMFLAG_Reset, bValue );
	a_rParams.GetValue( bValue, "ResetToRoot" );
	SetPlatformFlag( PLATFORMFLAG_ResetToRoot, bValue );
	a_rParams.GetValue( bValue, "Reverse" );
	SetPlatformFlag( PLATFORMFLAG_Reverse, bValue );
	a_rParams.GetValue( bValue, "RotateYOnly" );
	SetPlatformFlag( PLATFORMFLAG_RotateYOnly, bValue );
	a_rParams.GetValue( bValue, "LockAllAxis" );
	SetPlatformFlag( PLATFORMFLAG_LockAllAxis, bValue );

	bValue = TTRUE;
	a_rParams.GetValue( bValue, "LinearSpeed" );
	SetPlatformFlag( PLATFORMFLAG_LinearSpeed, bValue );
	a_rParams.GetValue( bValue, "DamageOnTouch" );
	SetPlatformFlag( PLATFORMFLAG_DamageOnTouch, bValue );
	a_rParams.GetValue( bValue, "UnbalancePlayers" );
	SetPlatformFlag( PLATFORMFLAG_UnbalancePlayers, bValue );
	a_rParams.GetValue( bValue, "Targetable" );
	SetPlatformFlag( PLATFORMFLAG_Targetable, bValue );
	a_rParams.GetValue( bValue, "TargetableFromBelow" );
	SetPlatformFlag( PLATFORMFLAG_TargetableFromBelow, bValue );

	m_fAttackSlamHeight = 0.0f;
	a_rParams.GetValue( m_fAttackSlamHeight, "AttackSlamHeight" );
	a_rParams.GetValue( m_iHitPoints, "HitPoints" );

	if ( m_iHitPoints == -1 )
		m_uiPlatformFlags |= PLATFORMFLAG_Invulnerable;

	a_rParams.GetValue( m_fDamageRadius, "DamageRadius" );
	a_rParams.GetValue( m_iDamage, "Damage" );
	a_rParams.GetValue( m_iRecoil, "Recoil" );
	a_rParams.GetValue( bValue, "EnableConveyor" );

	if ( bValue == TTRUE )
	{
		Conveyor* pConveyor = TSTATICCAST( Conveyor, TMalloc( sizeof( Conveyor ) ) );
		m_pConveyor         = pConveyor;

		TUtil::MemClear( pConveyor, sizeof( Conveyor ) );

		SetPlatformFlag( PLATFORMFLAG_Conveyor, bValue );
		a_rParams.GetValue( pConveyor->fSpeed, "ConveyorSpeed" );
		a_rParams.GetValue( pConveyor->fScrollSpeed, "TextureScrollSpeedV" );
		a_rParams.GetValue( bValue, "StartOff" );
		SetPlatformFlag( PLATFORMFLAG_StartOff, bValue );
		a_rParams.GetValue( bValue, "ReverseWhenTriggered" );
		SetPlatformFlag( PLATFORMFLAG_ReverseWhenTriggered, bValue );
		a_rParams.GetValue( bValue, "StopWhenTriggered" );
		SetPlatformFlag( PLATFORMFLAG_StopWhenTriggered, bValue );

		if ( TUINT8( m_uiPlatformFlags >> 31 ) & 1 )
		{
			pConveyor->fCurrentSpeed  = 0.0f;
			pConveyor->fCurrentScroll = 0.0f;
		}
		else
		{
			pConveyor->fCurrentSpeed  = pConveyor->fSpeed;
			pConveyor->fCurrentScroll = pConveyor->fScrollSpeed;
		}

		if ( -1 == m_oMovingSound.iEvent && -1 == m_oMovingSound.iGroup )
		{
			m_oMovingSound.iGroup = 0x23;
			m_oMovingSound.iEvent = 0x18B;
		}
	}

	if ( a_rParams.GetValue( bValue, "PlayTouchAnimation" ) && bValue )
		m_iTouchAnim = CreateManualAnimation( "anim_touch" );

	if ( a_rParams.GetValue( bValue, "PlayIdleAnimation" ) && bValue )
		m_iIdleAnim = CreateManualAnimation( "anim_idle" );

	if ( a_rParams.GetValue( bValue, "Oscillation" ) && bValue == TTRUE )
	{
		TFLOAT fDistance = 0.0f;
		a_rParams.GetValue( fDistance, "OscillationDistance" );
		TFLOAT fSpeed = 20.0f;
		a_rParams.GetValue( fSpeed, "OscillationSpeed" );

		PlatformSpring* pSpring = new PlatformSpring();
		m_pOscillation          = pSpring;

		TUINT8 uiFlags        = pSpring->m_uiFlags;
		pSpring->m_fAccelDown = fSpeed;
		pSpring->m_fAccelUp   = fSpeed;
		pSpring->m_fDistance  = fDistance;
		pSpring->m_fFadeSpeed = 0.0f;

		if ( uiFlags & PlatformSpring::FLAG_Rising )
		{
			pSpring->m_uiFlags = uiFlags & ~PlatformSpring::FLAG_Rising;
		}
		else if ( uiFlags & PlatformSpring::FLAG_Off )
		{
			pSpring->m_fVelocity = 0.0f;
			pSpring->m_fOffset   = 0.0f;
		}

		pSpring->m_fWait   = 0.05f;
		pSpring->m_uiFlags = ( pSpring->m_uiFlags & ~( PlatformSpring::FLAG_Off | PlatformSpring::FLAG_Fading ) ) | PlatformSpring::FLAG_Unk4 | PlatformSpring::FLAG_Unk20;
		m_pOscillation->m_uiFlags |= PlatformSpring::FLAG_Oscillating;

		SetPlatformFlag( PLATFORMFLAG_Oscillation, bValue );
	}

	if ( a_rParams.GetValue( bValue, "Bounce" ) && bValue == TTRUE )
	{
		m_pBounce = new Bounce();

		TFLOAT fStiffness = 0.0f;
		a_rParams.GetValue( fStiffness, "SpringStiffness" );
		TFLOAT fDamping = 0.0f;
		a_rParams.GetValue( fDamping, "SpringDamping" );
		TFLOAT fMinCompression = 0.0f;
		a_rParams.GetValue( fMinCompression, "MinCompression" );
		TFLOAT fMaxCompression = 0.0f;
		a_rParams.GetValue( fMaxCompression, "MaxCompression" );
		TFLOAT fMaxSpringTravel = 200.0f;
		a_rParams.GetValue( fMaxSpringTravel, "MaxSpringTravel" );
		TFLOAT fMinWeightTransfer = 0.0f;
		a_rParams.GetValue( fMinWeightTransfer, "MinWeightTransfer" );
		TFLOAT fMaxWeightTransfer = 0.0f;
		a_rParams.GetValue( fMaxWeightTransfer, "MaxWeightTransfer" );

		m_pBounce->oSpring.m_fStiffness = fStiffness;
		m_pBounce->oSpring.SetDampingRatio( fDamping );
		m_pBounce->fMinCompression    = fMinCompression;
		m_pBounce->fMaxCompression    = fMaxCompression;
		m_pBounce->fMinWeightTransfer = fMinWeightTransfer;
		m_pBounce->fMaxWeightTransfer = fMaxWeightTransfer;
		m_pBounce->fMaxSpringTravel   = fMaxSpringTravel;

		SetPlatformFlag( PLATFORMFLAG_Bounce, bValue );
	}

	a_rParams.GetValue( m_fDelayOnTrigger, "DelayOnTrigger" );

	const AEntityLoader::EntityData& rEntityData = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );

	if ( 0 < rEntityData.GetWaypointSetTargetCount() )
	{
		TASSERT( rEntityData.GetWaypointSetTargetCount(), "Platform is not linked to any waypoint sets" );
		m_pWaypointSet = &AWaypointData::GetSingleton()->GetSet( rEntityData.GetWaypointSetTarget( 0, 0 ) );
		m_iStartPoint  = rEntityData.GetWaypointSetTarget( 0, 1 );
		m_fPathLength  = ComputeMaximumNodeSeperation();

		APlatformManager::GetSingleton()->AddPlatform( this, m_pWaypointSet );

		m_iCurPoint = m_iStartPoint;
		StartFromCurrentNode();
	}

	if ( GetClass() == &TGetClass( APropPlatform ) && GetModelComponent() != TNULL )
	{
		// The collision: the model's (conveyors), a collision object of the params or a box of the bounding box
		AEntityParam::Collision** ppCollision = TNULL;

		if ( !( TUINT8( m_uiPlatformFlags >> 25 ) & 1 ) )
		{
			if ( a_rParams.GetCollision( ppCollision ) )
			{
				CreatePhysics( m_pTypeData->m_pTRBData, *ppCollision, TNULL, TFALSE );
			}
			else
			{
				// A box of the bounding box (its half extents), its centre half a height below the origin
				TBoxData oBox;
				oBox.SetHalfExtents( *rEntityData.m_pBBox );
				TShapeBaseClass* pShape = &oBox;

				TMatrix44 matOffset = TMatrix44::IDENTITY;
				matOffset.m_f42 -= rEntityData.m_pBBox->y;

				CreatePhysics( m_pTypeData->m_pTRBData, &pShape, &matOffset, 1, TNULL, TFALSE );
			}

			SetPhysicsMover( new AEntityPhysicsMover( this ) );

			TBOOL bCollideWithCamera = TFALSE;
			a_rParams.GetValue( bCollideWithCamera, "CollideWithCamera" );

			if ( bCollideWithCamera )
			{
				TPhysicsObjectContainer* pBody = m_pPhysicsData->m_pBody;
				TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, TPhysicsManager::GetCollisionLayer( pBody ) | 0x10 );
			}
		}
		else
		{
			CreateModelCollision( TFALSE );

			ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetCollisionInstance() );

			if ( pCollision != TNULL )
			{
				for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
				{
					TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

					if ( pBody != TNULL )
						TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, pBody->m_uiCollisionFilterInfo & 0xFF7 );
				}
			}
		}

		if ( m_iIdleAnim != 0 && m_pPhysicsData != TNULL && m_pPhysicsData->m_pPhysicsMover != TNULL )
			TSTATICCAST( AEntityPhysicsMover, m_pPhysicsData->m_pPhysicsMover )->SetBone( 0 );
	}

	TINT iZJumpSetID = -1;

	if ( a_rParams.GetValue( iZJumpSetID, "ZJumpSetID" ) )
	{
		TINT iZJumpNodeID = -1;

		if ( a_rParams.GetValue( iZJumpNodeID, "ZJumpNodeID" ) && iZJumpSetID > -1 && iZJumpNodeID > -1 )
		{
			m_pZJumpNode = &AZJumpData::GetSingleton()->GetSet( iZJumpSetID )->m_pNodes[ iZJumpNodeID ];
			TASSERT( GetZJumpNode()->m_eType == AZJumpData::NodeType_Entity );
			GetZJumpNode()->m_pInstance = this;
		}
	}

	const TCHAR* szEffectName = TNULL;

	if ( a_rParams.GetValue( szEffectName, "Effect Name" ) && szEffectName != TNULL && *szEffectName != '\0' )
	{
		m_pEffectData = TSTATICCAST( EffectData, TMalloc( sizeof( EffectData ) ) );
		a_rParams.GetFloat( m_pEffectData->fWarningTime, "Warning Time" );
		m_pEffectData->pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( szEffectName );

		if ( T2String8::CompareNoCase( "Blob_VentSteam", szEffectName ) == 0 )
		{
			m_pEffectData->oSound.iGroup = 0x23;
			m_pEffectData->oSound.iEvent = 0x1A9;

			if ( m_pPhysicsData != TNULL && m_pPhysicsData->m_pBody != TNULL )
			{
				// NOTE: the body's m_uiSurface (the collision filter group)
				m_pPhysicsData->m_pBody->m_uiSurface = 0x800000;
			}
		}
		else
		{
			m_pEffectData->oSound.iEvent = -1;
			m_pEffectData->oSound.iGroup = -1;
		}
	}

	FinishParams();
}

// $deBlob: FUNCTION 00796390
// $deBlob: FUNCTION 007797b3
void APropPlatform::PostCreate()
{
	AProp::PostCreate();

	m_vInitialPosition = GetMatrix().GetTranslation();
	TMatrix44::MatToQuat( m_qInitialRotation, GetMatrix() );

	// The offsets of the attached targets (entities, instances, nav set nodes) from the platform
	ANavMarkers::Data* pNavData    = ANavMarkers::GetSingleton()->GetData();
	TINT               iNumOffsets = m_uiNumInstances + m_uiNumEntityTargets;

	for ( TINT i = 0; i < m_uiNumNavSets; i++ )
		iNumOffsets += pNavData->m_pNavSets[ GetNavSetID( i ) ].m_iNumMarkers;

	if ( iNumOffsets >= 1 )
	{
		TMatrix44* pOffsets = TSTATICCAST( TMatrix44, TMemalign( 16, iNumOffsets * sizeof( TMatrix44 ) ) );
		m_pTargetOffsets    = pOffsets;

		for ( TINT i = 0; i < iNumOffsets; i++ )
			pOffsets[ i ].Identity();

		TMatrix44 mInverse = m_oMatrix;
		mInverse.InvertOrthonormal();

		TINT iOffset = 0;

		for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
		{
			ABaseEntity* pEntity = GetEntityTarget( i );

			pOffsets[ iOffset ].Multiply( pEntity->GetMatrix(), mInverse );

			TVector4 vOffset( pEntity->GetMatrix().m_f41 - m_vInitialPosition.x, pEntity->GetMatrix().m_f42 - m_vInitialPosition.y, pEntity->GetMatrix().m_f43 - m_vInitialPosition.z, pEntity->GetMatrix().m_f44 );
			TMatrix44::RotateVector( vOffset, mInverse, vOffset );
			TASSERT( vOffset.w == 1.0f );
			pOffsets[ iOffset ].SetTranslation( vOffset );
			iOffset++;
		}

		for ( TINT i = 0; i < m_uiNumInstances; i++ )
		{
			TMatrix44* pInstanceMatrix = *TSTATICCAST( TMatrix44*, GetInstanceTarget( i ) );

			pOffsets[ iOffset ].Multiply( *pInstanceMatrix, mInverse );

			TVector4 vOffset( pInstanceMatrix->m_f41 - m_vInitialPosition.x, pInstanceMatrix->m_f42 - m_vInitialPosition.y, pInstanceMatrix->m_f43 - m_vInitialPosition.z, pInstanceMatrix->m_f44 );
			TASSERT( vOffset.w == 1.0f );
			pOffsets[ iOffset ].SetTranslation( vOffset );
			iOffset++;
		}

		for ( TINT i = 0; i < m_uiNumNavSets; i++ )
		{
			ANavMarkers::NavSet& rNavSet = pNavData->m_pNavSets[ GetNavSetID( i ) ];

			for ( TINT j = 0; j < rNavSet.m_iNumMarkers; j++ )
			{
				const TVector4& rPosition = rNavSet.m_pMarkers[ j ].m_vPosition;

				TVector4 vOffset( rPosition.x - m_vInitialPosition.x, rPosition.y - m_vInitialPosition.y, rPosition.z - m_vInitialPosition.z, rPosition.w );
				TASSERT( vOffset.w == 1.0f );
				pOffsets[ iOffset ].SetTranslation( vOffset );
				iOffset++;
			}
		}
	}

	if ( m_pWaypointSet != TNULL )
	{
		TVector4 vTo( m_oSpline.vStartTangent.x + m_oSpline.vStart.x, m_oSpline.vStartTangent.y + m_oSpline.vStart.y, m_oSpline.vStartTangent.z + m_oSpline.vStart.z, m_oSpline.vStart.w );
		UpdateOrientation( m_oSpline.vStart, vTo );
		SetPosition( m_oSpline.vStart );
	}

	SyncBodyTransform();

	if ( m_pModelComponent != TNULL )
		m_pModelComponent->SetTransform( GetMatrix() );
}

// $deBlob: FUNCTION 007968b0
void APropPlatform::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( TUINT8( m_Flags >> 24 ) & 1 )
		return;

	TINT iMessage = a_rPacket.m_eMessage;

	if ( iMessage == AEntityIOPacket::MESSAGE_PLAYERATTACK && !( TUINT8( m_uiPlatformFlags >> 12 ) & 1 ) )
	{
		// Damage: m_iHitPoints, splash damage and destruction
		m_iHitPoints -= TSTATICCAST( AAttackMessage, TREINTERPRETCAST( void*, a_rPacket.m_iData ) )->m_uiDamage;

		if ( m_iHitPoints <= 0 )
		{
			if ( m_fDamageRadius > 0.0f )
				SplashDamage();

			SetVisible( TFALSE );
			SetHash( TFALSE );
			RemoveFromPhysics();
		}
	}
	else if ( iMessage == AEntityIOPacket::MESSAGE_TOUCH )
	{
		// Touched: the touch animation, the bounce, unbalancing and damage on touch

		if ( a_pCaller->IsFlagged( ABaseEntity::FLAG_Player ) )
		{
			// NOTE: the contact the blob sends with MESSAGE_TOUCH: +0x14 is the y of its normal, +0x20 points at a struct with the
			// blob's velocity (a TVector4) at +4
			TUINT8* pContact = TREINTERPRETCAST( TUINT8*, a_rPacket.m_iData );

			if ( 0.64278764f < -*TREINTERPRETCAST( TFLOAT*, pContact + 0x14 ) )
			{
				if ( m_iTouchAnim >= 0 && GetPlayingManualAnimation() == TNULL && m_fTouchCooldown <= 0.0f )
					m_pIdleAnim = AddManualAnimation( m_iTouchAnim, 0.0f, 0.15f, 1.0f, 1.0f );

				if ( ( m_uiPlatformFlags & PLATFORMFLAG_Bounce ) && m_pBounce != TNULL && m_fTouchCooldown <= 0.0f )
				{
					TVector4 vContactVelocity = *TREINTERPRETCAST( TVector4*, *TREINTERPRETCAST( TUINT8**, pContact + 0x20 ) + 4 );

					if ( vContactVelocity.y > 5.0f )
					{
						ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, a_pCaller );

						TFLOAT fSpeed       = TMath::Min( vContactVelocity.y, 40.0f );
						TFLOAT fBlend       = TFLOAT( pBlob->GetSize() + 1 ) / 10.0f;
						TFLOAT fCompression = ( m_pBounce->fMaxCompression - m_pBounce->fMinCompression ) * fBlend + m_pBounce->fMinCompression;
						TFLOAT fImpulse     = ( ( m_pBounce->fMaxWeightTransfer - m_pBounce->fMinWeightTransfer ) * fBlend + m_pBounce->fMinWeightTransfer ) * fSpeed;

						m_pBounce->oSpring.AddVelocity( fImpulse );

						// The blob and the platform bounce away with the impulse as their vertical speed
						TPhysicsManager*         pPhysicsManager = TPhysicsManager::GetSingleton();
						TPhysicsObjectContainer* pBlobBody       = pBlob->GetBody();

						TVector4 vVelocity;
						pPhysicsManager->GetLinearVelocity( pBlobBody, vVelocity );
						vVelocity.y = fImpulse;
						vVelocity.w = 1.0f;
						pPhysicsManager->SetLinearVelocity( pBlobBody, vVelocity );

						pPhysicsManager->GetLinearVelocity( m_pPhysicsData->m_pBody, vVelocity );
						vVelocity.y = fImpulse;
						pPhysicsManager->SetLinearVelocity( m_pPhysicsData->m_pBody, vVelocity );

						m_pBounce->oSpring.m_fRest = fCompression;

						if ( m_iSoundState != 4 )
							PlayOnActiveSound();
					}
				}

				if ( TUINT8( m_uiPlatformFlags >> 15 ) & 1 )
				{
					AEntityIOPacket oUnbalance;
					oUnbalance.m_eMessage = AEntityIOPacket::MESSAGE_UNBALANCE;
					oUnbalance.m_iData    = 0;
					a_pCaller->HandleEntityIO( this, oUnbalance );
				}

				if ( TUINT8( m_uiPlatformFlags >> 17 ) & 1 )
					m_pDistanceSpring->Init( m_fAccelDown, m_fAccelUp, m_fDistance, m_fDampeningSpeed );

				m_fTouchCooldown = 0.2f;

				if ( m_pOscillation != TNULL )
					m_pOscillation->m_fAmplitude = 0.5f;
			}

			if ( TUINT8( m_uiPlatformFlags >> 11 ) & 1 )
				DamageCaller( a_pCaller );
		}
	}
	else if ( ( iMessage == AEntityIOPacket::MESSAGE_TRIGGER || iMessage == AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE || iMessage == AEntityIOPacket::MESSAGE_TOGGLE ) && m_eDelayState == 1 && ( TUINT8( m_uiPlatformFlags >> 10 ) & 1 ) )
	{
		// Triggered while stopped: moves on (after "DelayOnTrigger")
		if ( TUINT8( m_uiPlatformFlags >> 21 ) & 1 )
			return;

		if ( 0.0f < m_fDelayOnTrigger )
		{
			if ( m_iSoundState != 4 )
				PlayOnActiveSound();

			m_fDelay          = m_fDelayOnTrigger;
			m_uiPlatformFlags = ( m_uiPlatformFlags & ~PLATFORMFLAG_Stopped ) | PLATFORMFLAG_Delayed;
		}
		else
		{
			if ( m_iSoundState != 1 )
			{
				StopIdleSound();
				PlayMovingSound();

				if ( m_iSoundState == 0 )
					PlayStartSound();

				m_iSoundState = 1;
			}

			m_uiPlatformFlags &= ~PLATFORMFLAG_Stopped;
			m_eDelayState = 0;
			OnStartMoving();
		}
	}
	else if ( ( iMessage == AEntityIOPacket::MESSAGE_TRIGGER || iMessage == AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE || iMessage == AEntityIOPacket::MESSAGE_TOGGLE ) && ( TUINT8( m_uiPlatformFlags >> 25 ) & 1 ) )
	{
		// Triggered conveyor: on/off or reversed
		Conveyor* pConveyor = m_pConveyor;

		if ( !( TUINT8( m_uiPlatformFlags >> 29 ) & 1 ) )
		{
			if ( TUINT8( m_uiPlatformFlags >> 30 ) & 1 )
			{
				pConveyor->fCurrentSpeed  = -pConveyor->fCurrentSpeed;
				pConveyor->fCurrentScroll = -pConveyor->fCurrentScroll;
			}
		}
		else if ( TMath::Abs( pConveyor->fCurrentSpeed ) <= 0.0f )
		{
			pConveyor->fCurrentSpeed  = pConveyor->fSpeed;
			pConveyor->fCurrentScroll = pConveyor->fScrollSpeed;
			PlayStartSound();
			StartMovingSoundLoop();
		}
		else
		{
			pConveyor->fCurrentSpeed  = 0.0f;
			pConveyor->fCurrentScroll = 0.0f;
			StopMovingSoundLoop();
			PlayStopSound();
		}
	}
	else if ( iMessage == AEntityIOPacket::MESSAGE_CHALLENGERESET )
	{
		Reset( TNULL );
		return;
	}
	else if ( iMessage == AEntityIOPacket::MESSAGE_SKIP )
	{
		if ( m_eDelayState == 1 )
			m_fDelayOnTrigger = 0.0f;

		if ( m_pWaypointSet != TNULL )
		{
			TINT iNodeCount = m_pWaypointSet->iNodeCount;

			for ( TINT i = 0; i < iNodeCount; i++ )
			{
				if ( TINT8( m_pWaypointSet->GetNode( TINT16( i ) ).uiFlags ) < 0 )
				{
					m_iCurPoint = i;
					StartFromCurrentNode();
					break;
				}
			}
		}
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 00798b30
void APropPlatform::PlayOnActiveSound()
{
	if ( m_oOnActiveSound.iGroup != -1 && m_oOnActiveSound.iEvent != -1 && ( GetLOD() & FLAG_LowLOD ) == 0 )
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_oOnActiveSound.iGroup ), ASoundMap::EVENTID( m_oOnActiveSound.iEvent ), GetMatrix().GetTranslation(), 0 );
}

// $deBlob: FUNCTION 00796e40
TVector4 APropPlatform::GetReactionForce( void* a_pVelocity, void* a_pEntity )
{
	if ( !( TUINT8( m_uiPlatformFlags >> 25 ) & 1 ) )
	{
		TVector4 vVelocity;
		TPhysicsManager::GetSingleton()->GetLinearVelocity( m_pPhysicsData->m_pBody, vVelocity );
		return vVelocity;
	}

	return m_vVelocity;
}

// $deBlob: FUNCTION 00796e90
void APropPlatform::Reset( ABaseEntity* a_pEntity )
{
	SetHash( ~m_uiExtraFlags & 1 );
	m_eDelayState = 0;

	m_uiPlatformFlags &= ~( PLATFORMFLAG_Backwards | PLATFORMFLAG_Delayed | PLATFORMFLAG_TurnAround );
	m_fSpeed         = m_fBaseSpeed;
	m_iCurPoint      = -1;
	m_fSplineT       = 0.0f;
	m_fNodeDelay     = 0.0f;
	m_fTouchCooldown = 0.0f;
	m_iHitPoints     = m_iMaxHitPoints;
	m_fDelay         = 0.0f;
	m_vVelocity      = TVector4::VEC_ZERO;

	if ( m_pIdleAnim != TNULL )
	{
		m_pIdleAnim->RemoveAnimation( 0.0f );
		m_pIdleAnim = TNULL;
	}

	TMatrix44 oMatrix;
	oMatrix.SetFromQuaternion( m_qInitialRotation );
	oMatrix.m_f41 = m_vInitialPosition.x;
	oMatrix.m_f42 = m_vInitialPosition.y;
	oMatrix.m_f43 = m_vInitialPosition.z;
	oMatrix.m_f44 = m_vInitialPosition.w;
	SetMatrix( oMatrix );

	if ( m_pWaypointSet != TNULL )
	{
		m_iCurPoint = m_iStartPoint;
		m_fSpeed    = m_fSpeed / m_fPathLength;
		StartFromCurrentNode();
	}

	PlatformSpring* apSprings[ 2 ] = { m_pOscillation, m_pDistanceSpring };

	for ( TINT i = 0; i < 2; i++ )
	{
		PlatformSpring* pSpring = apSprings[ i ];

		if ( pSpring != TNULL )
		{
			pSpring->m_fVelocity  = 0.0f;
			pSpring->m_fOffset    = 0.0f;
			pSpring->m_fWait      = 0.0f;
			pSpring->m_fFade      = 1.0f;
			pSpring->m_fAmplitude = 1.0f;
			pSpring->m_uiFlags    = ( pSpring->m_uiFlags & ~( PlatformSpring::FLAG_Rising | PlatformSpring::FLAG_Unk4 | PlatformSpring::FLAG_Fading ) ) | PlatformSpring::FLAG_Off | PlatformSpring::FLAG_Unk20;
		}
	}

	if ( m_pConveyor != TNULL )
	{
		Conveyor* pConveyor = m_pConveyor;

		if ( TUINT8( m_uiPlatformFlags >> 31 ) & 1 )
		{
			pConveyor->fCurrentSpeed  = 0.0f;
			pConveyor->fCurrentScroll = 0.0f;
		}
		else
		{
			pConveyor->fCurrentSpeed  = pConveyor->fSpeed;
			pConveyor->fCurrentScroll = pConveyor->fScrollSpeed;
		}
	}

	if ( m_pBounce != TNULL )
	{
		m_pBounce->oSpring.m_fPosition = 0.0f;
		m_pBounce->oSpring.m_fRest     = 0.0f;
		m_pBounce->oSpring.m_fVelocity = 0.0f;
	}

	SetVisible( TTRUE );
	AProp::Reset( a_pEntity );
}

// $deBlob: FUNCTION 00797090
void APropPlatform::OnPoolRelease()
{
	if ( m_pWaypointSet != TNULL )
	{
		APlatformManager::GetSingleton()->RemovePlatform( this, m_pWaypointSet );
		m_pWaypointSet = TNULL;
	}
}

// $deBlob: FUNCTION 007970c0
void APropPlatform::SetPosition( const TVector4& a_rPosition )
{
	AProp::SetPosition( a_rPosition );

	if ( m_pTargetOffsets != TNULL )
	{
		// Moves the attached targets (entities, instances, nav sets) with the platform
		TMatrix44* pOffsets = TSTATICCAST( TMatrix44, m_pTargetOffsets );
		TINT       iOffset  = 0;

		for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
		{
			ABaseEntity* pTarget  = GetEntityTarget( i );
			ABaseEntity* pEnemy   = pTarget;
			ABaseEntity* pWrapper = TNULL;
			TBOOL        bMove    = TTRUE;

			if ( pTarget->IsA( &TGetClass( AEnemyWrapper ) ) )
			{
				pWrapper = pTarget;
				pEnemy   = TSTATICCAST( ABaseEntity, TSTATICCAST( AEnemyWrapper, pTarget )->GetSpawnedEnemy() );

				if ( pEnemy != TNULL )
				{
					AGndMotorComponent* pMotor = TSTATICCAST( AGndMotorComponent, TSTATICCAST( ACharEntity, pEnemy )->GetMotorComponent() );

					bMove = !pMotor->IsEnabled() || !pEnemy->IsFlagged( FLAG_HighLOD );

					pMotor->AddFlags( AGndMotorComponent::FLAG_Unk400 );
				}
			}

			TMatrix44 mMatrix;
			mMatrix.Multiply( m_oMatrix, pOffsets[ iOffset ] );

			if ( pEnemy != TNULL && bMove )
			{
				TVector4 vDelta( mMatrix.m_f41 - pEnemy->GetMatrix().m_f41, mMatrix.m_f42 - pEnemy->GetMatrix().m_f42, mMatrix.m_f43 - pEnemy->GetMatrix().m_f43, mMatrix.m_f44 );
				pEnemy->SetMatrix( mMatrix );

				if ( 36.0f < vDelta.MagnitudeSq() )
					pEnemy->SyncBodyTransform();
			}

			if ( pWrapper != TNULL )
				pWrapper->SetMatrix( mMatrix );

			iOffset++;
		}

		for ( TINT i = 0; i < m_uiNumInstances; i++ )
		{
			TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, GetInstanceTarget( i ) );
			const TMatrix44&            rOffset   = pOffsets[ iOffset ];
			iOffset++;

			TVector4 vPosition( rOffset.m_f41 + a_rPosition.x, rOffset.m_f42 + a_rPosition.y, rOffset.m_f43 + a_rPosition.z, a_rPosition.w );
			TVector4 vDelta( pInstance->m_pMatrix->m_f41 - vPosition.x, pInstance->m_pMatrix->m_f42 - vPosition.y, pInstance->m_pMatrix->m_f43 - vPosition.z, pInstance->m_pMatrix->m_f44 );
			TBOOL    bMoved = 0.01f < vDelta.MagnitudeSq();

			pInstance->m_pMatrix->m_f41 = vPosition.x;
			pInstance->m_pMatrix->m_f42 = vPosition.y;
			pInstance->m_pMatrix->m_f43 = vPosition.z;
			pInstance->m_pMatrix->m_f44 = vPosition.w;

			AModelInstance2* pModelInstance = TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 );

			// NOTE: an instance without a model instance keeps its collision instance in CellInstance+0x20 (symbol)
			ACollisionInstance2* pCollision = TNULL;

			if ( pModelInstance == TNULL )
			{
				if ( pInstance->m_pLowLODInstance != TNULL && pInstance->symbol != TNULL )
				{
					pCollision = TREINTERPRETCAST( ACollisionInstance2*, TUINTPTR( pInstance->symbol ) );
					pCollision->SetMatrix( *pInstance->m_pMatrix );

					if ( bMoved )
						pCollision->UpdateBodyTransforms();
				}
			}
			else
			{
				pModelInstance->SetPosition( vPosition );

				if ( bMoved && pModelInstance->GetCollisionInstance() != TNULL )
					TSTATICCAST( ACollisionInstance2, pModelInstance->GetCollisionInstance() )->UpdateBodyTransforms();
			}

			if ( pInstance->m_pLowLODInstance != TNULL )
			{
				TTransformObject& rTransform = pInstance->m_pLowLODInstance->GetTransform();

				TMatrix44 mMatrix = rTransform.GetMatrix();
				mMatrix.m_f41     = vPosition.x;
				mMatrix.m_f42     = vPosition.y;
				mMatrix.m_f43     = vPosition.z;
				mMatrix.m_f44     = vPosition.w;
				rTransform.SetMatrix( mMatrix );
			}

			if ( ( pInstance->m_uiFlags & 7 ) == 7 )
			{
				// A Z-jump node of the instance: the points that start from it follow
				TASSERT( pInstance->zjump.m_iSet > -1 && pInstance->zjump.m_iNode > -1 );

				AZJumpData::Set*  pSet  = AZJumpData::GetSingleton()->GetSet( pInstance->zjump.m_iSet );
				AZJumpData::Node* pNode = &AZJumpData::GetSingleton()->GetSet( pInstance->zjump.m_iSet )->m_pNodes[ pInstance->zjump.m_iNode ];

				for ( TINT j = 0; j < pSet->m_iPointCount; j++ )
				{
					if ( pSet->m_pPoints[ j ].m_pNode == pNode )
					{
						AZJumpData::GetSingleton()->UpdatePointEffect( AZJumpData::GetSingleton()->GetPointIndex( &pSet->m_pPoints[ j ] ) );
					}
				}
			}
		}

		for ( TINT i = 0; i < m_uiNumNavSets; i++ )
		{
			ANavMarkers::NavSet& rNavSet = ANavMarkers::GetSingleton()->GetData()->m_pNavSets[ GetNavSetID( i ) ];

			for ( TINT j = 0; j < rNavSet.m_iNumMarkers; j++ )
			{
				const TMatrix44& rOffset = pOffsets[ iOffset ];
				iOffset++;

				TVector4& rPosition = rNavSet.m_pMarkers[ j ].m_vPosition;
				rPosition.x         = a_rPosition.x + rOffset.m_f41;
				rPosition.y         = rOffset.m_f42 + a_rPosition.y;
				rPosition.z         = rOffset.m_f43 + a_rPosition.z;
				rPosition.w         = a_rPosition.w;
			}
		}
	}
}

// $deBlob: FUNCTION 00797740
void APropPlatform::Destroy()
{
	if ( m_iSoundState != 5 )
	{
		StopIdleSound();
		StopMovingSound();
		m_iSoundState = 5;
	}
}

// $deBlob: FUNCTION 00797770
void APropPlatform::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( m_pConveyor != TNULL )
	{
		// The conveyor's texture scroll
		if ( !( a_iLOD & FLAG_LowLOD ) && 0.0f < TMath::Abs( m_pConveyor->fCurrentSpeed ) )
			StartMovingSoundLoop();
		else
			StopMovingSoundLoop();

		return;
	}

	if ( !( TUINT8( a_iLOD >> 9 ) & 1 ) )
	{
		if ( m_eDelayState == 0 )
		{
			if ( m_pWaypointSet != TNULL )
			{
				if ( m_iSoundState == 1 )
					return;

				StopIdleSound();
				PlayMovingSound();

				if ( m_iSoundState == 0 )
					PlayStartSound();

				m_iSoundState = 1;
				return;
			}
		}
		else
		{
			if ( m_eDelayState != 1 )
				return;

			if ( 0.0f < m_fDelay )
				return;
		}

		if ( m_iSoundState != 0 )
		{
			StopMovingSound();
			PlayIdleSound();

			if ( m_iSoundState == 1 )
				PlayStopSound();

			m_iSoundState = 0;
		}
	}
	else if ( m_iSoundState != 5 )
	{
		StopIdleSound();
		StopMovingSound();
		m_iSoundState = 5;
	}
}

// $deBlob: FUNCTION 007978a0
void APropPlatform::SetWaypointSet( TINT a_iWaypointSetID, TINT a_iStartPoint, const TVector4* a_pPosition, RespawnData* a_pRespawnData )
{
	TASSERT( TNULL == m_pWaypointSet );
	TASSERT( a_iWaypointSetID >= 0 || a_pRespawnData != TNULL );

	if ( a_pRespawnData != TNULL )
	{
		m_pWaypointSet = a_pRespawnData->pWaypointSet;
		m_iNextPoint   = a_pRespawnData->iNextPoint;
		m_iCurPoint    = a_pRespawnData->iCurPoint;
		m_iStartPoint  = a_pRespawnData->iStartPoint;
	}
	else
	{
		m_pWaypointSet = &AWaypointData::GetSingleton()->GetSet( a_iWaypointSetID );
		m_iStartPoint  = a_iStartPoint;
	}

	TFLOAT fPathLength = ComputeMaximumNodeSeperation();
	m_fPathLength      = fPathLength;

	if ( FLT_MIN < TMath::Abs( m_fSpeed ) )
		m_fSpeed = m_fBaseSpeed / fPathLength;

	APlatformManager::GetSingleton()->AddPlatform( this, m_pWaypointSet );

	if ( a_pPosition != TNULL )
	{
		SetupRespawn( a_iStartPoint, a_pPosition, a_pRespawnData );
		return;
	}

	m_iCurPoint = m_iStartPoint;
	StartFromCurrentNode();
}

// OnPoolRelease's body as a non-virtual: leaves the waypoint set
// $deBlob: FUNCTION 00797a50
void APropPlatform::RemoveFromWaypointSet()
{
	if ( m_pWaypointSet != TNULL )
	{
		APlatformManager::GetSingleton()->RemovePlatform( this, m_pWaypointSet );
		m_pWaypointSet = TNULL;
	}
}

// $deBlob: FUNCTION 00797a80
void APropPlatform::SyncBodyTransform()
{
	AVisibleEntity::SyncBodyTransform();

	if ( m_pTargetOffsets != TNULL )
	{
		// Updates the attached targets
		for ( TINT i = 0; i < m_uiNumEntityTargets; i++ )
		{
			ABaseEntity* pTarget  = GetEntityTarget( i );
			ABaseEntity* pEnemy   = pTarget;
			ABaseEntity* pWrapper = TNULL;

			if ( pTarget->IsA( &TGetClass( AEnemyWrapper ) ) )
			{
				pEnemy   = TSTATICCAST( ABaseEntity, TSTATICCAST( AEnemyWrapper, pTarget )->GetSpawnedEnemy() );
				pWrapper = pTarget;
			}

			if ( pEnemy != TNULL )
				pEnemy->SyncBodyTransform();

			if ( pWrapper != TNULL )
				pWrapper->SyncBodyTransform();
		}

		for ( TINT i = 0; i < m_uiNumInstances; i++ )
		{
			TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, GetInstanceTarget( i ) );
			ACollisionInstance2*        pCollision;

			// NOTE: an instance without a model instance keeps its collision instance in CellInstance+0x20 (symbol)
			if ( pInstance->m_pModelInstance2 == TNULL )
			{
				if ( pInstance->m_pLowLODInstance == TNULL )
					continue;

				pCollision = TREINTERPRETCAST( ACollisionInstance2*, TUINTPTR( pInstance->symbol ) );
			}
			else
			{
				pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance() );
			}

			if ( pCollision != TNULL )
				pCollision->UpdateBodyTransforms();
		}
	}
}

// $deBlob: FUNCTION 007981b0
void APropPlatform::Despawn( TBOOL a_bPlayEffect )
{
	if ( m_uiPlatformFlags & PLATFORMFLAG_MovingSound )
	{
		m_uiPlatformFlags &= ~PLATFORMFLAG_MovingSound;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, m_oMovingSound.iGroup, m_oMovingSound.iEvent );
	}

	if ( m_uiPlatformFlags & PLATFORMFLAG_IdleSound )
	{
		m_uiPlatformFlags &= ~PLATFORMFLAG_IdleSound;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, m_oIdleSound.iGroup, m_oIdleSound.iEvent );
	}

	AVisibleEntity::Despawn( a_bPlayEffect );
}

// Turns the platform from a_rFrom towards a_rTo
// $deBlob: FUNCTION 00798230
void APropPlatform::SetIdleSound( TINT16 a_iGroup, TINT16 a_iEvent )
{
	m_oIdleSound.iGroup = a_iGroup;
	m_oIdleSound.iEvent = a_iEvent;
}

// $deBlob: FUNCTION 00798250
void APropPlatform::SetMovingSound( TINT16 a_iGroup, TINT16 a_iEvent )
{
	m_oMovingSound.iGroup = a_iGroup;
	m_oMovingSound.iEvent = a_iEvent;
}

// $deBlob: FUNCTION 007982b0
void APropPlatform::SetDefaultMovingSound( TINT16 a_iGroup, TINT16 a_iEvent )
{
	if ( -1 == m_oMovingSound.iGroup && -1 == m_oMovingSound.iEvent )
	{
		m_oMovingSound.iGroup = a_iGroup;
		m_oMovingSound.iEvent = a_iEvent;
	}
}

// $deBlob: FUNCTION 007982f0
void APropPlatform::UpdateOrientation( const TVector4& a_rFrom, const TVector4& a_rTo )
{
	if ( TUINT8( m_uiPlatformFlags >> 7 ) & 1 )
		return;

	TVector4 vDirection( a_rTo.x - a_rFrom.x, a_rTo.y - a_rFrom.y, a_rTo.z - a_rFrom.z, a_rTo.w );

	if ( vDirection.MagnitudeSq() > TMath::TFLOAT_EPSILON )
	{
		if ( TUINT8( m_uiPlatformFlags >> 6 ) & 1 )
			vDirection.y = 0.0f;

		vDirection.Normalise();

		TMatrix44 oMatrix = GetMatrix();
		oMatrix.LookAtDirection( vDirection, TVector4::VEC_NEGY );
		SetMatrix( oMatrix );
	}
}

// "Reset": back to the start point
// $deBlob: FUNCTION 007983d0
void APropPlatform::RestartFromStart()
{
	m_iCurPoint = m_iStartPoint;
	StartFromCurrentNode();
}

// "ResetToRoot": back to the first start node of the set
// $deBlob: FUNCTION 007983f0
void APropPlatform::RestartFromRoot()
{
	m_iCurPoint = m_pWaypointSet->GetStartNode();
	StartFromCurrentNode();
}

// Starts moving from m_iCurPoint
// $deBlob: FUNCTION 00798440
void APropPlatform::StartFromCurrentNode()
{
	AWaypointData::Node& rCurNode = m_pWaypointSet->GetNode( TINT16( m_iCurPoint ) );

	m_vCurPointPos = rCurNode.vPosition;
	AdjustNodePosition( &m_vCurPointPos, &m_vCurPointPos, &m_vCurPointPos, TTRUE );
	GetNextNode( TINT16( m_iCurPoint ), TFALSE, rCurNode.uiFlags & AWaypointData::Node::FLAG_Toggled, &m_iNextPoint );

	if ( m_iNextPoint == -1 && ( TUINT8( m_uiPlatformFlags >> 2 ) & 1 ) )
		m_iNextPoint = m_pWaypointSet->GetStartNode();

	m_vNextPointPos = m_pWaypointSet->GetNode( TINT16( m_iNextPoint ) ).vPosition;
	AdjustNodePosition( &m_vNextPointPos, &m_vCurPointPos, &m_pWaypointSet->GetNode( TINT16( m_iCurPoint ) ).vPosition, TTRUE );

	m_fSplineT = 0.0f;
	SetSplineForward( m_vCurPointPos, m_vCurPointPos, m_vNextPointPos, TINT16( m_iNextPoint ), &m_oSpline, TTRUE );

	if ( TUINT8( m_uiPlatformFlags >> 9 ) & 1 )
		m_fSpeedScale = m_fPathLength / TVector4::Distance( m_vCurPointPos, m_vNextPointPos );
	else
		m_fSpeedScale = 1.0f;

	StartNodeDelay( TINT16( m_iCurPoint ) );

	TVector4 vTo( m_oSpline.vStartTangent.x + m_oSpline.vStart.x, m_oSpline.vStartTangent.y + m_oSpline.vStart.y, m_oSpline.vStartTangent.z + m_oSpline.vStart.z, m_oSpline.vStart.w );
	UpdateOrientation( m_oSpline.vStart, vTo );
	SetPosition( m_oSpline.vStart );
	ToggleNavSets( TINT16( m_iCurPoint ) );

	AWaypointData::Node& rNode = m_pWaypointSet->GetNode( TINT16( m_iCurPoint ) );

	if ( rNode.uiFlags & AWaypointData::Node::FLAG_TurretChange )
		SetTurretActive( ( m_pWaypointSet->GetNode( TINT16( m_iCurPoint ) ).uiFlags & AWaypointData::Node::FLAG_TurretOn ) != 0 );

	SyncBodyTransform();
}

// Continues from a respawn point (or a_pPosition at the start point)
// $deBlob: FUNCTION 00798790
void APropPlatform::SetupRespawn( TINT a_iStartPoint, const TVector4* a_pPosition, RespawnData* a_pRespawnData )
{
	TBOOL bToggle = TTRUE;

	if ( a_pRespawnData == TNULL )
	{
		m_iCurPoint  = m_iStartPoint;
		m_iNextPoint = m_iStartPoint;
	}
	else
	{
		m_iCurPoint  = a_pRespawnData->iCurPoint;
		m_iNextPoint = a_pRespawnData->iNextPoint;
		bToggle      = TFALSE;

		if ( a_pRespawnData->uiFlags & 4 )
			m_uiPlatformFlags |= PLATFORMFLAG_Toggled;
		else
			m_uiPlatformFlags &= ~PLATFORMFLAG_Toggled;
	}

	m_vCurPointPos = *a_pPosition;
	AdjustNodePosition( &m_vCurPointPos, &m_vCurPointPos, &m_vCurPointPos, TTRUE );

	m_vNextPointPos = m_pWaypointSet->GetNode( TINT16( m_iNextPoint ) ).vPosition;
	AdjustNodePosition( &m_vNextPointPos, &m_vCurPointPos, &m_vCurPointPos, TTRUE );

	m_fSplineT = ( a_pRespawnData == TNULL ) ? 0.0f : a_pRespawnData->fSplineT;
	SetSplineForward( m_vCurPointPos, m_vCurPointPos, m_vNextPointPos, TINT16( m_iNextPoint ), &m_oSpline, bToggle );

	if ( TUINT8( m_uiPlatformFlags >> 9 ) & 1 )
		m_fSpeedScale = m_fPathLength / TVector4::Distance( m_vCurPointPos, m_vNextPointPos );
	else
		m_fSpeedScale = 1.0f;

	m_fNodeDelay = 0.0f;

	TVector4 vTo( m_oSpline.vStartTangent.x + m_oSpline.vStart.x, m_oSpline.vStartTangent.y + m_oSpline.vStart.y, m_oSpline.vStartTangent.z + m_oSpline.vStart.z, m_oSpline.vStart.w );
	UpdateOrientation( m_oSpline.vStart, vTo );
	SetPosition( m_oSpline.vStart );
	SyncBodyTransform();
}

// $deBlob: FUNCTION 00798970
void APropPlatform::StopMovingSound()
{
	if ( m_uiPlatformFlags & PLATFORMFLAG_MovingSound )
	{
		m_uiPlatformFlags &= ~PLATFORMFLAG_MovingSound;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, m_oMovingSound.iGroup, m_oMovingSound.iEvent );
	}
}

// $deBlob: FUNCTION 007989b0
void APropPlatform::PlayMovingSound()
{
	if ( m_uiPlatformFlags & PLATFORMFLAG_IdleSound )
	{
		m_uiPlatformFlags &= ~PLATFORMFLAG_IdleSound;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, m_oIdleSound.iGroup, m_oIdleSound.iEvent );
	}

	if ( -1 != m_oMovingSound.iGroup && -1 != m_oMovingSound.iEvent && !( m_uiPlatformFlags & PLATFORMFLAG_MovingSound ) )
	{
		if ( ASoundInterface::IsEventLooping( ASoundMap::GROUPID( m_oMovingSound.iGroup ), ASoundMap::EVENTID( m_oMovingSound.iEvent ) ) )
		{
			m_uiPlatformFlags |= PLATFORMFLAG_MovingSound;
			APositionalSoundManager::GetSingleton()->AddMovingEntitySound( this, m_oMovingSound.iGroup, m_oMovingSound.iEvent );
		}
		else
		{
			ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_oMovingSound.iGroup ), ASoundMap::EVENTID( m_oMovingSound.iEvent ), GetMatrix().GetTranslation(), 0 );
		}
	}
}

// $deBlob: FUNCTION 00798a70
void APropPlatform::PlayIdleSound()
{
	if ( m_oIdleSound.iGroup != -1 && m_oIdleSound.iEvent != -1 && !( m_uiPlatformFlags & PLATFORMFLAG_IdleSound ) )
	{
		if ( ASoundInterface::IsEventLooping( ASoundMap::GROUPID( m_oIdleSound.iGroup ), ASoundMap::EVENTID( m_oIdleSound.iEvent ) ) )
		{
			m_uiPlatformFlags |= PLATFORMFLAG_IdleSound;
			APositionalSoundManager::GetSingleton()->AddMovingEntitySound( this, m_oIdleSound.iGroup, m_oIdleSound.iEvent );
		}
		else
		{
			ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_oIdleSound.iGroup ), ASoundMap::EVENTID( m_oIdleSound.iEvent ), GetMatrix().GetTranslation(), 0 );
		}
	}
}

// $deBlob: FUNCTION 00798af0
void APropPlatform::StopIdleSound()
{
	if ( m_uiPlatformFlags & PLATFORMFLAG_IdleSound )
	{
		m_uiPlatformFlags &= ~PLATFORMFLAG_IdleSound;
		APositionalSoundManager::GetSingleton()->RemoveSound( this, m_oIdleSound.iGroup, m_oIdleSound.iEvent );
	}
}

// $deBlob: FUNCTION 00798b70
void APropPlatform::PlayStartSound()
{
	if ( m_oStartSound.iGroup != -1 && m_oStartSound.iEvent != -1 && ( GetLOD() & FLAG_LowLOD ) == 0 )
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_oStartSound.iGroup ), ASoundMap::EVENTID( m_oStartSound.iEvent ), GetMatrix().GetTranslation(), 0 );
}

// $deBlob: FUNCTION 00798bb0
void APropPlatform::PlayStopSound()
{
	if ( m_oStopSound.iGroup != -1 && m_oStopSound.iEvent != -1 && ( GetLOD() & FLAG_LowLOD ) == 0 )
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_oStopSound.iGroup ), ASoundMap::EVENTID( m_oStopSound.iEvent ), GetMatrix().GetTranslation(), 0 );
}

// $deBlob: FUNCTION 00798bf0
void APropPlatform::FinishParams()
{

	if ( m_fDistance > 0.0f )
	{
		PlatformSpring* pSpring = new PlatformSpring();
		m_uiPlatformFlags |= PLATFORMFLAG_Distance;
		m_pDistanceSpring = pSpring;

		if ( TUINT8( m_uiPlatformFlags >> 14 ) & 1 )
			pSpring->m_uiFlags |= PlatformSpring::FLAG_BobOnEnd;
	}

	TFLOAT fSpeed = m_fSpeed;
	m_fBaseSpeed  = fSpeed;

	if ( m_pWaypointSet != TNULL )
		m_fSpeed = fSpeed / m_fPathLength;

	m_iMaxHitPoints = m_iHitPoints;

	if ( TUINT8( m_uiPlatformFlags >> 25 ) & 1 )
	{
		TModelInstance* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance();
		pModelInstance->SetCustomRenderMethod( RenderConveyor, this );

		CollectConveyorPasses();
		CreateConveyorMaterials();
	}
}

// Stops at the current node
// $deBlob: FUNCTION 00798cf0
void APropPlatform::StopMoving()
{
	TBOOL bStopped = m_iSoundState == 0;
	m_eDelayState  = 1;

	if ( !bStopped )
	{
		StopMovingSound();
		PlayIdleSound();

		if ( m_iSoundState == 1 )
			PlayStopSound();

		m_iSoundState = 0;
	}
}

// Reached a_iNode: waits its delay
// $deBlob: FUNCTION 00798d30
void APropPlatform::StartNodeDelay( TINT16 a_iNode )
{
	TFLOAT fDelay = m_pWaypointSet->GetNode( a_iNode ).fDelay;
	m_fNodeDelay  = fDelay;

	if ( fDelay >= 0.0f )
	{
		if ( fDelay > TMath::TFLOAT_EPSILON )
			StopMoving();
	}
	else
	{
		m_uiPlatformFlags |= PLATFORMFLAG_Stopped;
		StopMoving();
	}
}

// The Catmull-Rom tangents of the segment from a_rCur to a_rNext
// $deBlob: FUNCTION 00798dd0
void APropPlatform::CacheSplineVectors( const TVector4& a_rPrev, const TVector4& a_rCur, const TVector4& a_rNext, const TVector4& a_rAfterNext, Spline* a_pSpline )
{
	a_pSpline->vStart = a_rCur;
	a_pSpline->vEnd   = a_rNext;

	a_pSpline->vStartTangent = TVector4( a_pSpline->vEnd.x - a_rPrev.x, a_pSpline->vEnd.y - a_rPrev.y, a_pSpline->vEnd.z - a_rPrev.z, a_pSpline->vEnd.w );
	a_pSpline->vStartTangent.Multiply( 0.5f );

	a_pSpline->vEndTangent = TVector4( a_rAfterNext.x - a_pSpline->vStart.x, a_rAfterNext.y - a_pSpline->vStart.y, a_rAfterNext.z - a_pSpline->vStart.z, a_rAfterNext.w );
	a_pSpline->vEndTangent.Multiply( 0.5f );
}

// The segment from a_rCur to a_rNext going forward (a_iNextNode is the node of a_rNext)
// $deBlob: FUNCTION 00798ea0
void APropPlatform::SetSplineForward( const TVector4& a_rPrev, const TVector4& a_rCur, const TVector4& a_rNext, TINT16 a_iNextNode, Spline* a_pSpline, TBOOL a_bToggle )
{
	AWaypointData::Node& rNode = m_pWaypointSet->GetNode( a_iNextNode );

	TINT                 iAfterNext;
	AWaypointData::Node* pAfterNext = GetNextNode( a_iNextNode, TFALSE, rNode.uiFlags & AWaypointData::Node::FLAG_Toggled, &iAfterNext );

	if ( a_bToggle )
	{
		if ( rNode.uiFlags & AWaypointData::Node::FLAG_Toggled )
			m_uiPlatformFlags |= PLATFORMFLAG_Toggled;
		else
			m_uiPlatformFlags &= ~PLATFORMFLAG_Toggled;

		if ( ARandom::GetSingleton()->GetRandom().GetInt() % 2 != 0 || ( TUINT8( m_Flags >> 23 ) & 1 ) )
			rNode.ToggleFlag();
	}

	TVector4 vAfterNext;

	if ( pAfterNext == TNULL )
	{
		vAfterNext = a_rNext;
	}
	else
	{
		vAfterNext = pAfterNext->vPosition;
		AdjustNodePosition( &vAfterNext, &a_rNext, &m_pWaypointSet->GetNode( TINT16( m_iNextPoint ) ).vPosition, TFALSE );
	}

	CacheSplineVectors( a_rPrev, a_rCur, a_rNext, vAfterNext, a_pSpline );
}

// The segment from a_rCur to a_rNext going backwards (a_iCurNode is the node of a_rCur)
// $deBlob: FUNCTION 00799040
void APropPlatform::SetSplineReverse( const TVector4& a_rAfter, const TVector4& a_rCur, const TVector4& a_rNext, TINT16 a_iCurNode, Spline* a_pSpline, TBOOL a_bToggle )
{
	AWaypointData::Node& rNode = m_pWaypointSet->GetNode( a_iCurNode );

	TINT                 iBefore;
	AWaypointData::Node* pBefore = GetNextNode( a_iCurNode, TTRUE, rNode.uiFlags & AWaypointData::Node::FLAG_Toggled, &iBefore );

	if ( a_bToggle )
	{
		if ( rNode.uiFlags & AWaypointData::Node::FLAG_Toggled )
			m_uiPlatformFlags |= PLATFORMFLAG_Toggled;
		else
			m_uiPlatformFlags &= ~PLATFORMFLAG_Toggled;

		if ( ARandom::GetSingleton()->GetRandom().GetInt() % 2 != 0 || ( TUINT8( m_Flags >> 23 ) & 1 ) )
			rNode.ToggleFlag();
	}

	TVector4 vBefore;

	if ( pBefore == TNULL )
	{
		vBefore = a_rCur;
	}
	else
	{
		vBefore = pBefore->vPosition;
		AdjustNodePosition( &vBefore, &a_rCur, &m_pWaypointSet->GetNode( TINT16( m_iCurPoint ) ).vPosition, TFALSE );
	}

	CacheSplineVectors( vBefore, a_rCur, a_rNext, a_rAfter, a_pSpline );
}

// $deBlob: FUNCTION 007991e0
AWaypointData::Node* APropPlatform::GetNextNode( TINT16 a_iNode, TBOOL a_bReverse, TBOOL a_bToggled, TINT* a_pOutNode )
{
	return FindNextNode( m_pWaypointSet, a_iNode, a_bReverse, a_bToggled, a_pOutNode );
}

// The node after a_iNode: the first link of the direction, or the second one if a_bToggled
//-----------------------------------------------------------------------------
// Purpose: writes the respawn data of the platform (its route and where it is on it), so that the ACiv of a civilian
// that leaves the view can continue from there. The flags: 0x1 the route is not a spline, 0x2 linear (PLATFORMFLAG_Linear),
// 0x4 toggled (PLATFORMFLAG_Toggled).
//-----------------------------------------------------------------------------
// $deBlob: FUNCTION 00798010
void APropPlatform::SaveRespawnData( RespawnData* a_pRespawnData )
{
	a_pRespawnData->pWaypointSet = m_pWaypointSet;
	a_pRespawnData->uiFlags      = 0;

	if ( m_pWaypointSet == TNULL )
		return;

	TASSERT( m_iStartPoint < 0x7FFF );
	TASSERT( m_iCurPoint < 0x7FFF );
	TASSERT( m_iNextPoint < 0x7FFF );

	a_pRespawnData->iStartPoint = TINT16( m_iStartPoint );
	a_pRespawnData->iCurPoint   = TINT16( m_iCurPoint );
	a_pRespawnData->iNextPoint  = TINT16( m_iNextPoint );
	a_pRespawnData->fSplineT    = m_fSplineT;

	if ( ( m_uiPlatformFlags & PLATFORMFLAG_Linear ) != 0 )
		a_pRespawnData->uiFlags |= 2;
	else
		a_pRespawnData->uiFlags &= ~2;

	if ( ( m_uiPlatformFlags & PLATFORMFLAG_Toggled ) != 0 )
		a_pRespawnData->uiFlags |= 4;
	else
		a_pRespawnData->uiFlags &= ~4;

	a_pRespawnData->fSpeed       = m_fSpeed;
	a_pRespawnData->fSpeedScale  = m_fSpeedScale;
	a_pRespawnData->fPathLength  = m_fPathLength;
	a_pRespawnData->vCurPointPos = m_vCurPointPos;

	// The route is followed by the respawned platform unless it has one of the flags that change the route
	if ( ( m_uiPlatformFlags & ( PLATFORMFLAG_Reverse | PLATFORMFLAG_Continuous | PLATFORMFLAG_Reset | PLATFORMFLAG_ResetToRoot ) ) == 0 && ( m_pWaypointSet->uiUnk02 & 1 ) != 0 )
		a_pRespawnData->uiFlags |= 1;
	else
		a_pRespawnData->uiFlags &= ~1;
}

// $deBlob: FUNCTION 00797b60
TBOOL APropPlatform::UpdateRespawnRoute( TFLOAT a_fDeltaTime, RespawnData* a_pRespawnData, TVector4* a_pPosition )
{
	if ( a_pRespawnData->pWaypointSet == TNULL )
		return TFALSE;

	if ( a_pRespawnData->iNextPoint < 0 )
		return TFALSE;

	TVector4 vPosition = *a_pPosition;

	if ( a_pRespawnData->fSpeed > 1.1920929e-07f )
	{
		// The spline T advances by the speed of the frame, up to the end of the segment
		TFLOAT fStep      = a_pRespawnData->fSpeed * a_fDeltaTime * a_pRespawnData->fSpeedScale;
		TFLOAT fRemaining = 1.0f - a_pRespawnData->fSplineT;

		a_pRespawnData->fSplineT += TMath::Min( fRemaining, fStep );

		const TVector4& rNextPos = a_pRespawnData->pWaypointSet->GetNode( a_pRespawnData->iNextPoint ).vPosition;

		if ( ( a_pRespawnData->uiFlags & 2 ) != 0 )
		{
			// Linear: the position is interpolated between the current point and the next one
			vPosition.Lerp4( a_pRespawnData->vCurPointPos, rNextPos, a_pRespawnData->fSplineT );
		}
		else
		{
			// Curved: the position moves towards the next point by the distance covered in this frame
			TVector4 vDirection( rNextPos.x - vPosition.x, rNextPos.y - vPosition.y, rNextPos.z - vPosition.z, rNextPos.w );

			TFLOAT fDistance = vDirection.NormaliseMag();
			TFLOAT fMove     = a_pRespawnData->fSpeed * a_fDeltaTime * a_pRespawnData->fPathLength;

			if ( fDistance > fMove )
			{
				vPosition.x = vDirection.x * fMove + vPosition.x;
				vPosition.y = vDirection.y * fMove + vPosition.y;
				vPosition.z = vDirection.z * fMove + vPosition.z;
			}
			else
			{
				vPosition = rNextPos;
			}
		}
	}

	*a_pPosition = vPosition;

	if ( a_pRespawnData->fSplineT < 1.0f )
		return TTRUE;

	// The next point is reached: the one it was becomes the current point and the next one is found from it
	TINT                 iNewNode = 0;
	AWaypointData::Node* pFound   = FindNextNode( a_pRespawnData->pWaypointSet, a_pRespawnData->iNextPoint, TFALSE, ( a_pRespawnData->uiFlags >> 2 ) & 1, &iNewNode );

	if ( pFound == TNULL )
	{
		a_pRespawnData->uiFlags &= ~1;
		return TTRUE;
	}

	AWaypointData::Node& rReached = a_pRespawnData->pWaypointSet->GetNode( a_pRespawnData->iNextPoint );

	a_pRespawnData->iCurPoint    = a_pRespawnData->iNextPoint;
	a_pRespawnData->vCurPointPos = rReached.vPosition;
	a_pRespawnData->iNextPoint   = TINT16( iNewNode );
	a_pRespawnData->fSplineT     = 0.0f;

	AWaypointData::Node& rNext = a_pRespawnData->pWaypointSet->GetNode( a_pRespawnData->iNextPoint );

	if ( ( rNext.uiFlags & AWaypointData::Node::FLAG_Toggled ) != 0 )
		a_pRespawnData->uiFlags |= 4;
	else
		a_pRespawnData->uiFlags &= ~4;

	rNext.ToggleFlag();

	a_pRespawnData->fSpeedScale = 1.0f;

	if ( ( a_pRespawnData->uiFlags & 2 ) != 0 )
	{
		TFLOAT fDistance            = TVector4::Distance( a_pRespawnData->vCurPointPos, rNext.vPosition );
		a_pRespawnData->fSpeedScale = a_pRespawnData->fPathLength / fDistance;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00799200
AWaypointData::Node* APropPlatform::FindNextNode( AWaypointData::Set* a_pSet, TINT16 a_iNode, TBOOL a_bReverse, TBOOL a_bToggled, TINT* a_pOutNode )
{
	AWaypointData::Node& rNode = a_pSet->GetNode( a_iNode );

	AWaypointData::Node* pLeft  = TNULL;
	AWaypointData::Node* pRight = TNULL;
	TINT                 iLeft  = -1;

	for ( TINT i = 0; i < rNode.uiLinkCount; i++ )
	{
		TBOOL bForwardLink = rNode.GetLink( i ).uiFlags & AWaypointData::Link::FLAG_Toggled;

		if ( bForwardLink ? a_bReverse : !a_bReverse )
			continue;

		if ( pLeft == TNULL )
		{
			iLeft = TUINT16( rNode.pLinks[ i ].iNode );
			pLeft = rNode.GetLinkedNode( i, a_pSet );

			if ( !a_bToggled )
				break;
		}
		else
		{
			TASSERT( !pRight );
			TUINT16 uiRight = rNode.pLinks[ i ].iNode;
			pRight          = rNode.GetLinkedNode( i, a_pSet );

			if ( a_bToggled )
			{
				*a_pOutNode = uiRight;
				return pRight;
			}
		}
	}

	*a_pOutNode = iLeft;
	return pLeft;
}

// The longest link of the set
// $deBlob: FUNCTION 00799400
TFLOAT APropPlatform::ComputeMaximumNodeSeperation()
{
	TFLOAT fMax = 0.0f;

	for ( TINT16 i = 0; i < m_pWaypointSet->iNodeCount; i++ )
	{
		AWaypointData::Node& rNode = m_pWaypointSet->GetNode( i );

		for ( TINT k = 0; k < rNode.uiLinkCount; k++ )
		{
			if ( rNode.GetLink( k ).uiFlags & AWaypointData::Link::FLAG_Toggled )
			{
				TFLOAT fDistance = TVector4::Distance( rNode.vPosition, rNode.GetLinkedNode( k, m_pWaypointSet )->vPosition );
				fMax             = TMath::Max( fMax, fDistance );
			}
		}
	}

	return fMax;
}

// $deBlob: FUNCTION 00799530
void APropPlatform::ComputeSpeedScale( const TVector4& a_rFrom, const TVector4& a_rTo )
{
	if ( TUINT8( m_uiPlatformFlags >> 9 ) & 1 )
		m_fSpeedScale = m_fPathLength / TVector4::Distance( a_rFrom, a_rTo );
	else
		m_fSpeedScale = 1.0f;
}

// Turns the nav sets of the platform on or off at a node with AWaypointData::Node::FLAG_NavSets
// $deBlob: FUNCTION 00799730
void APropPlatform::ToggleNavSets( TINT16 a_iNode )
{
	if ( !( m_pWaypointSet->GetNode( a_iNode ).uiFlags & AWaypointData::Node::FLAG_NavSets ) )
		return;

	TBOOL bEnable = ( m_pWaypointSet->GetNode( a_iNode ).uiFlags & AWaypointData::Node::FLAG_EnableNavSets ) != 0;

	for ( TINT i = 0; i < m_uiNumNavSets; i++ )
	{
		ANavMarkers::NavSet& rNavSet = ANavMarkers::GetSingleton()->GetData()->m_pNavSets[ GetNavSetID( i ) ];

		for ( TINT j = 0; j < rNavSet.m_iNumMarkers; j++ )
		{
			if ( bEnable )
				rNavSet.m_pMarkers[ j ].m_uiFlags |= 1;
			else
				rNavSet.m_pMarkers[ j ].m_uiFlags &= ~1;
		}
	}
}

// The conveyor's materials, TNULL without a conveyor
// $deBlob: FUNCTION 00797e30
TMaterial** APropPlatform::GetConveyorMaterials() const
{
	return ( m_pConveyor != TNULL ) ? m_pConveyor->ppMaterials : TNULL;
}

// $deBlob: FUNCTION 00797e40
TBOOL APropPlatform::IsBlocking( TINT a_iPlatformID, TINT a_iFrom, TINT a_iTo, TFLOAT a_fDistance )
{
	if ( a_iPlatformID == GetID() )
		return TFALSE;

	TBOOL bBackwards = ( m_uiPlatformFlags & PLATFORMFLAG_Backwards ) != 0;
	TINT  iTo;
	TINT  iFrom;

	if ( !bBackwards )
	{
		iTo   = m_iNextPoint;
		iFrom = m_iCurPoint;
	}
	else
	{
		iTo   = m_iCurPoint;
		iFrom = m_iNextPoint;
	}

	if ( iTo == a_iTo )
	{
		// Both on the same link: the distance this one has left
		TFLOAT fLeft = ( m_fPathLength * ( bBackwards ? m_fSplineT : 1.0f - m_fSplineT ) ) / m_fSpeedScale;

		if ( fLeft <= a_fDistance && ( iFrom == a_iFrom || fLeft <= m_fBoundingSphereRadius ) )
		{
			TFLOAT fGap = a_fDistance - fLeft;

			if ( fGap < m_fBoundingSphereRadius && ( fGap != 0.0f || GetID() <= a_iPlatformID ) )
				return TTRUE;
		}
	}
	else if ( iFrom == a_iTo )
	{
		// This one is on the next link: the distance it has done
		TFLOAT fDone = ( m_fPathLength * ( bBackwards ? 1.0f - m_fSplineT : m_fSplineT ) ) / m_fSpeedScale;

		if ( fDone + a_fDistance < m_fBoundingSphereRadius )
			return TTRUE;
	}

	return TFALSE;
}

// The platform's moving sound as a looping positional sound (the conveyors' belts)
// $deBlob: FUNCTION 0079a240
void APropPlatform::StartMovingSoundLoop()
{
	if ( -1 != m_oMovingSound.iGroup && -1 != m_oMovingSound.iEvent && !( m_uiPlatformFlags & PLATFORMFLAG_MovingSound ) )
	{
		APositionalSoundManager::GetSingleton()->AddEntitySound( this, m_oMovingSound.iGroup, m_oMovingSound.iEvent );
		m_uiPlatformFlags |= PLATFORMFLAG_MovingSound;
	}
}

// $deBlob: FUNCTION 0079a1f0
void APropPlatform::StopMovingSoundLoop()
{
	if ( -1 != m_oMovingSound.iGroup && -1 != m_oMovingSound.iEvent && ( m_uiPlatformFlags & PLATFORMFLAG_MovingSound ) )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( this, m_oMovingSound.iGroup, m_oMovingSound.iEvent );
		m_uiPlatformFlags &= ~PLATFORMFLAG_MovingSound;
	}
}

// $deBlob: FUNCTION 0079a390
TBOOL APropPlatform::DamageableFilter::operator()( ABaseEntity* a_pEntity )
{
	if ( m_pPlatform != a_pEntity )
	{
		if ( a_pEntity->IsFlagged( ABaseEntity::FLAG_Player ) || ( TUINT8( a_pEntity->GetFlags() >> 5 ) & 1 ) )
			return TTRUE;
	}

	return TFALSE;
}

// The destroyed platform hurts what's near it: MESSAGE_PROPATTACK with an AAttackMessage to every damageable entity
// $deBlob: FUNCTION 00799620
void APropPlatform::SplashDamage()
{
	DamageableFilter oFilter( this );
	TINT             iNumEntities;
	ABaseEntity**    ppEntities = AEntityHash::GetSingleton()->Query( GetMatrix().GetTranslation(), m_fDamageRadius, &iNumEntities, &oFilter, TTRUE, 500, TFALSE, TTRUE, TFALSE );

	for ( TINT i = 0; i < iNumEntities; i++ )
	{
		ABaseEntity* pEntity = ppEntities[ i ];

		TVector4 vDirection( pEntity->GetMatrix().m_f41 - m_oMatrix.m_f41, pEntity->GetMatrix().m_f42 - m_oMatrix.m_f42, pEntity->GetMatrix().m_f43 - m_oMatrix.m_f43, pEntity->GetMatrix().m_f44 );
		vDirection.NormaliseXZ();

		AAttackMessage oAttack( AAttackMessage::ATTACKTYPE( 0 ), GetMatrix().GetTranslation(), m_iDamage, m_iRecoil, vDirection, TFALSE );

		AEntityIOPacket oPacket;
		oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PROPATTACK;
		oPacket.m_iData    = TREINTERPRETCAST( TINT, &oAttack );
		pEntity->HandleEntityIO( this, oPacket );
	}
}

// What touches the platform gets hurt: MESSAGE_PROPATTACK with an AAttackMessage
// $deBlob: FUNCTION 00799580
void APropPlatform::DamageCaller( ABaseEntity* a_pCaller )
{
	TVector4 vDirection( a_pCaller->GetMatrix().m_f41 - m_oMatrix.m_f41, a_pCaller->GetMatrix().m_f42 - m_oMatrix.m_f42, a_pCaller->GetMatrix().m_f43 - m_oMatrix.m_f43, a_pCaller->GetMatrix().m_f44 );
	vDirection.NormaliseXZ();

	AAttackMessage oAttack( AAttackMessage::ATTACKTYPE( 0 ), GetMatrix().GetTranslation(), m_iDamage, m_iRecoil, vDirection, TFALSE );

	AEntityIOPacket oPacket;
	oPacket.m_eMessage = AEntityIOPacket::MESSAGE_PROPATTACK;
	oPacket.m_iData    = TREINTERPRETCAST( TINT, &oAttack );
	a_pCaller->HandleEntityIO( this, oPacket );
}

// The scrolling passes of the materials of LOD 0
// $deBlob: FUNCTION 00799c60
void APropPlatform::CollectConveyorPasses()
{
	T2ModelPtr pModel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModelInstance()->GetModel();
	TASSERT( pModel->GetLODCount() > 0 );
	TModelLOD& rLOD = pModel->GetLOD( 0 );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		if ( m_pConveyor->iNumPasses > 7 )
			break;

		AddConveyorPasses( rLOD.GetMeshes()[ i ]->GetMaterial() );
	}
}

// The UV offset passes of a material (animated passes of the UV offset type with Pass::m_uiVFunction 1)
// $deBlob: FUNCTION 00799d40
void APropPlatform::AddConveyorPasses( TMaterial* a_pMaterial )
{
	AMaterialInfo* pInfo = TSTATICCAST( AMaterialInfo, a_pMaterial->GetUVAnimation() );

	if ( pInfo == TNULL )
		return;

	TINT iNumPasses = pInfo->GetMaterial()->GetNumPasses();

	for ( TINT iPass = 0; iPass < iNumPasses; iPass++ )
	{
		if ( m_pConveyor->iNumPasses > 7 )
			return;

		AUVAnimation* pUVAnimation = pInfo->GetUVAnimation( iPass );

		if ( pUVAnimation != TNULL && pUVAnimation->GetType() == AUVAnimation::TYPE_UVOFFSET )
		{
			T2Material::Pass* pPass = pInfo->GetMaterial()->GetPassInfo( iPass );

			if ( pPass->m_uiVFunction == 1 )
			{
				if ( m_pConveyor->iNumPasses == 0 )
					m_pConveyor->pPasses = TSTATICCAST( ConveyorPass, TMemalign( 16, 8 * sizeof( ConveyorPass ) ) );

				m_pConveyor->pPasses[ m_pConveyor->iNumPasses ].pPass       = pPass;
				m_pConveyor->pPasses[ m_pConveyor->iNumPasses ].fOrigOffset = pPass->m_fVParam1;
				m_pConveyor->iNumPasses++;
			}
		}
	}
}

// Every mesh of LOD 0 renders with a copy of its material (so the offsets of the passes scroll)
// $deBlob: FUNCTION 007999f0
void APropPlatform::CreateConveyorMaterials()
{
	AModelInstance2* pInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	T2ModelPtr       pModel    = pInstance->GetModelInstance()->GetModel();
	TASSERT( pModel->GetLODCount() >= 1 );
	TModelLOD& rLOD       = pModel->GetLOD( 0 );
	TINT       iMeshCount = rLOD.GetMeshCount();
	TASSERT( iMeshCount >= 1 );

	m_pConveyor->ppMaterials = TSTATICCAST( TMaterial*, TMemalign( 16, iMeshCount * sizeof( TMaterial* ) ) );

	for ( TINT i = 0; i < iMeshCount; i++ )
	{
		TMesh*     pMesh     = rLOD.GetMeshes()[ i ];
		TMaterial* pOriginal = pMesh->GetMaterial();
		TMaterial* pCopy;

		if ( pOriginal->IsA( &TGetClass( ASkinMaterialHAL ) ) )
		{
			ASkinMaterial* pSkinMaterial = TSTATICCAST( ASkinMaterial, ASkinShader::GetSingleton()->CreateMaterial( "conveyor" ) );
			pSkinMaterial->Create( ASkinMaterial::BLENDMODE( 0 ) );
			pCopy = pSkinMaterial;
		}
		else
		{
			TWorldMaterial* pWorldMaterial = TWorldShader::GetSingleton()->CreateMaterial( "conveyor" );
			pWorldMaterial->Create( TWorldMaterial::BLENDMODE( 0 ) );
			pCopy = pWorldMaterial;
		}

		m_pConveyor->ppMaterials[ i ] = pCopy;
		AMaterialInfo::CopyMaterial( pOriginal, pCopy );

		if ( pMesh->IsA( &TGetClass( ASkinMesh ) ) )
		{
			TSTATICCAST( ASkinMesh, pMesh )->SetMaterial2( m_pConveyor->ppMaterials[ i ] );
			pInstance->SetFlag( AModelInstance2::FLAG( 0x100 ), TTRUE );
		}
		else
		{
			pInstance->SetOverrideMaterial( m_pConveyor->ppMaterials[ i ] );
		}
	}
}

// The conveyor's scroll and its passes' V offsets
// $deBlob: FUNCTION 00799e50
TFLOAT APropPlatform::GetConveyorScroll() const
{
	return ( m_pConveyor != TNULL ) ? m_pConveyor->fCurrentScroll : 0.0f;
}

// $deBlob: FUNCTION 00799e80
void APropPlatform::RestoreConveyorPasses()
{
	for ( TINT i = 0; i < m_pConveyor->iNumPasses; i++ )
		m_pConveyor->pPasses[ i ].pPass->m_fVParam1 = m_pConveyor->pPasses[ i ].fOrigOffset;
}

// $deBlob: FUNCTION 00799eb0
void APropPlatform::SetConveyorPassOffset( TFLOAT a_fOffset )
{
	for ( TINT i = 0; i < m_pConveyor->iNumPasses; i++ )
		m_pConveyor->pPasses[ i ].pPass->m_fVParam1 = a_fOffset;
}

// Renders the model with the conveyor's materials and the V offsets of the scrolling passes
// $deBlob: FUNCTION 00799ee0
void __stdcall APropPlatform::RenderConveyor( TModelInstance* a_pModelInstance, void* a_pUserData )
{
	APropPlatform* pPlatform = TSTATICCAST( APropPlatform, a_pUserData );
	Conveyor*      pConveyor = pPlatform->m_pConveyor;

	TFLOAT fScroll = ( pConveyor != TNULL ) ? pConveyor->fCurrentScroll : 0.0f;

	for ( TINT i = 0; i < pConveyor->iNumPasses; i++ )
		pConveyor->pPasses[ i ].pPass->m_fVParam1 = fScroll;

	AModelInstance2* pInstance = TSTATICCAST( AEntityModelComponent, pPlatform->GetModelComponent() )->GetModelInstance();

	if ( pConveyor != TNULL && pConveyor->ppMaterials != TNULL )
	{
		T2ModelPtr pModel = a_pModelInstance->GetModel();
		TASSERT( pModel->GetLODCount() > 0 );
		TModelLOD& rLOD = pModel->GetLOD( 0 );

		for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
		{
			AMaterialManager::GetSingleton()->UpdateUVs( pConveyor->ppMaterials[ i ], TFALSE, TFALSE );

			if ( rLOD.GetMeshes()[ i ]->IsA( &TGetClass( ASkinMesh ) ) )
				TSTATICCAST( ASkinMesh, rLOD.GetMeshes()[ i ] )->SetMaterial2( pConveyor->ppMaterials[ i ] );
		}
	}

	AModelInstance2::RenderGeneric( a_pModelInstance, pInstance );

	for ( TINT i = 0; i < pConveyor->iNumPasses; i++ )
		pConveyor->pPasses[ i ].pPass->m_fVParam1 = pConveyor->pPasses[ i ].fOrigOffset;
}

// $deBlob: FUNCTION 00797f40
TBOOL APropPlatform::AreInstancesColour( TUINT8 a_uiColour )
{
	TASSERT( m_pTargetOffsets );
	TASSERT( m_uiNumInstances > 0 );

	for ( TINT i = 0; i < m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = TSTATICCAST( TerrainTypes::CellInstance, GetInstanceTarget( i ) );

		if ( ( pInstance->m_uiFlags & 7 ) == 1 && a_uiColour != TSTATICCAST( ABuildingInstance, pInstance->m_pModelInstance2 )->GetCurrentColour() )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0079a100
void APropPlatform::SetSoundState( TINT a_iState )
{
	if ( m_iSoundState == a_iState )
		return;

	switch ( a_iState )
	{
		case 0:
			StopMovingSound();
			PlayIdleSound();

			if ( m_iSoundState == 1 )
			{
				PlayStopSound();
				m_iSoundState = TINT16( a_iState );
				return;
			}

			break;
		case 1:
			StopIdleSound();
			PlayMovingSound();

			if ( m_iSoundState == 0 )
			{
				PlayStartSound();
				m_iSoundState = TINT16( a_iState );
				return;
			}

			break;
		case 2:
			PlayStartSound();
			return;
		case 3:
			PlayStopSound();
			return;
		case 4:
			PlayOnActiveSound();
			return;
		case 5:
			StopIdleSound();
			StopMovingSound();
			m_iSoundState = TINT16( a_iState );
			return;
		default:
			return;
	}

	m_iSoundState = TINT16( a_iState );
}
