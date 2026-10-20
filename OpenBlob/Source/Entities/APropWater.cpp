#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "APropWater.h"
#include "ANavMarkers.h"
#include "AEntityIOPacket.h"
#include "AEntityParams.h"
#include "AEntityLoader.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionInstance2.h"
#include "Cameras/ACameraManager.h"
#include "Sound/ASoundInterface.h"

#include <Physics/Havok/TPhysicsManager.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Render/TModel.h>
#include <Shaders/WorldShader/TWorldShader.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TBOOL APropWater::ms_bTransitionMaterialsCreated = TFALSE;
TBOOL APropWater::ms_bTransitionsCleared         = TFALSE;

TSTATICASSERT( sizeof( APropWater ) == 0x164 );

// The cross fade speed of the water/ink transitions (shared by the APropWaters using a slot)
static TFLOAT s_fTransitionSpeed = 0.2f;

// $deBlob: FUNCTION 0079f500
// $deBlob: FUNCTION 0079f510
// $deBlob: FUNCTION 0079f580
// $deBlob: FUNCTION 0079f5e0
// $deBlob: FUNCTION 0079f5f0
// $deBlob: FUNCTION 00540f20
// $deBlob: FUNCTION 00859e50
TDEFINE_CLASS( APropWater );

// $deBlob: FUNCTION 0079f600
APropWater::APropWater()
{
	m_pMesh2          = TNULL;
	m_pSound          = TNULL;
	m_mBodyFull       = TMatrix44::IDENTITY;
	m_mEntityFull     = TMatrix44::IDENTITY;
	m_fBodyEmptyY     = 0.0f;
	m_fEntityEmptyY   = 0.0f;
	m_fTime           = 0.0f;
	m_fFillTime       = 0.0f;
	m_fDrainTime      = 0.0f;
	m_fWaterDepth     = 1.0f;
	m_fDelayOnTrigger = 0.0f;
	m_fDelay          = 0.0f;
	m_uiWaterFlags    = 0;
	m_pTriggerCaller  = TNULL;
	SetHash( TFALSE );
	m_uiWaterFlags = ( m_uiWaterFlags & 0xC3FF ) | 0x2000;
}

// $deBlob: FUNCTION 0079f770
// $deBlob: FUNCTION 007a17f0
// $deBlob: FUNCTION 007a17e4
APropWater::~APropWater()
{
	if ( m_pMesh2 )
		delete m_pMesh2;

	// The transition materials are shared by every APropWater
	if ( ms_bTransitionMaterialsCreated )
	{
		TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

		for ( TINT i = 0; i < TWorldShaderHAL::MAX_TRANSITIONS; i++ )
		{
			TWorldShaderHAL::TransitionRef& rRef = pShader->GetTransitionRef( i );
			rRef.m_pWaterMat->OnDestroy();
			delete rRef.m_pWaterMat;
			pShader->GetTransitionRef( i ).m_pWaterMat = TNULL;

			rRef.m_pInkMat->OnDestroy();
			delete rRef.m_pInkMat;
			pShader->GetTransitionRef( i ).m_pInkMat = TNULL;
		}

		ms_bTransitionMaterialsCreated = TFALSE;
	}
}

// $deBlob: FUNCTION 0079fce0
void APropWater::Tick( TFLOAT a_fDeltaTime )
{
	AProp::Tick( a_fDeltaTime );

	if ( 0.0f < m_fDelay )
	{
		m_fDelay -= a_fDeltaTime;

		if ( m_fDelay <= 0.0f )
		{
			m_fDelay = 0.0f;
			Trigger();
		}
	}

	// The water/ink cross fade
	if ( ( m_uiWaterFlags & WATERFLAG_TransitionMask ) != ( TRANSITION_None << 12 ) )
	{
		// NOTE: retail leaves these uninitialised (GetTransition doesn't set them when no slot is free)
		TINT                            iIndex = 0;
		TBOOL                           bNew   = TFALSE;
		TWorldShaderHAL::TransitionRef* pRef   = GetTransition( TUINT32( m_pTriggerCaller ), iIndex, bNew );

		TBOOL bDone = TTRUE;

		if ( !bNew )
		{
			TFLOAT fProgress  = ( s_fTransitionSpeed * a_fDeltaTime ) / TFLOAT( pRef->m_iNumUsers ) + pRef->m_fProgress;
			pRef->m_fProgress = fProgress;

			if ( fProgress < 1.0f )
			{
				TINT16 iProgress = -TINT16( TINT( fProgress * 32767.0f ) );

				if ( ( m_uiWaterFlags & WATERFLAG_TransitionMask ) == ( TRANSITION_ToInk << 12 ) )
				{
					pRef->m_pInkMat->SetUserDataIndex( TWorldMaterialHAL::UData_0, TINT16( iIndex ) );
					pRef->m_pWaterMat->SetUserDataIndex( TWorldMaterialHAL::UData_0, iProgress );
				}
				else
				{
					pRef->m_pWaterMat->SetUserDataIndex( TWorldMaterialHAL::UData_0, TINT16( iIndex ) );
					pRef->m_pInkMat->SetUserDataIndex( TWorldMaterialHAL::UData_0, iProgress );
				}

				bDone = TFALSE;
			}
		}

		if ( bDone )
		{
			m_uiWaterFlags  = ( m_uiWaterFlags & ~WATERFLAG_TransitionMask ) | ( TRANSITION_None << 12 );
			pRef->m_uiOwner = TUINT32( -1 );
			m_pModelComponent->GetModelInstance()->SetOverrideMaterial( TNULL );
			SetTicking( TFALSE );
		}
	}

	TUINT16 uiFlags = m_uiWaterFlags;

	switch ( ( uiFlags >> 10 ) & 3 )
	{
		case STATE_Full:
		case STATE_Empty:
			if ( m_fDelay <= 0.0f && ( uiFlags & WATERFLAG_TransitionMask ) == ( TRANSITION_None << 12 ) )
			{
				SetTicking( TFALSE );
				return;
			}

			break;
		case STATE_Filling:
		case STATE_Draining:
		{
			if ( ( uiFlags & WATERFLAG_TransitionMask ) != ( TRANSITION_None << 12 ) )
				return;

			TFLOAT fDuration = m_fDrainTime;
			m_fTime += a_fDeltaTime;

			if ( ( uiFlags & WATERFLAG_StateMask ) == ( STATE_Filling << 10 ) )
				fDuration = m_fFillTime;

			m_fTime = TMath::Min( m_fTime, fDuration );

			// 0: full, 1: empty
			TFLOAT fProgress = m_fTime / fDuration;
			TMath::Clip( fProgress, 0.0f, 1.0f );

			if ( ( m_uiWaterFlags & WATERFLAG_StateMask ) == ( STATE_Filling << 10 ) )
				fProgress = 1.0f - fProgress;

			TMatrix44 mBody   = m_mBodyFull;
			mBody.m_f42       = TMath::LERP( m_mBodyFull.m_f42, m_fBodyEmptyY, fProgress );
			TMatrix44 mEntity = m_mEntityFull;
			mEntity.m_f42     = TMath::LERP( m_mEntityFull.m_f42, m_fEntityEmptyY, fProgress );

			SetWaterMatrices( mBody, mEntity );
			UpdateSound();

			uiFlags     = m_uiWaterFlags;
			TINT eState = ( uiFlags >> 10 ) & 3;

			if ( eState == STATE_Filling && m_fFillTime <= m_fTime )
			{
				m_fTime        = 0.0f;
				m_uiWaterFlags = uiFlags & ~WATERFLAG_StateMask;
				AddSoundEmitter();
			}
			else if ( eState == STATE_Draining && m_fDrainTime <= m_fTime )
			{
				m_fTime        = 0.0f;
				m_uiWaterFlags = ( uiFlags & ~WATERFLAG_StateMask ) | ( STATE_Empty << 10 );

				if ( uiFlags & WATERFLAG_EmptyWhenDrained )
				{
					ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x67 ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), FMOD_EVENT_DEFAULT );
					EnableNavSets( TFALSE );
					UpdateSound();
					return;
				}

				AddSoundEmitter();
			}

			UpdateSound();
			break;
		}
	}
}

// $deBlob: FUNCTION 007a0600
void APropWater::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( a_iLOD & 0x200 )
	{
		RemoveSoundEmitter();

		if ( m_pSound )
		{
			m_pSound->stop( false );
			m_pSound = TNULL;
		}
	}
	else if ( ( m_uiWaterFlags & WATERFLAG_StateMask ) == 0 )
	{
		AddSoundEmitter();
	}
}

// $deBlob: FUNCTION 007a0180
// $deBlob: FUNCTION 007a1843
void APropWater::PostCreate()
{
	// The bodies' matrix when full is the one they were created with
	ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetCollisionInstance() );

	if ( pCollision != TNULL && pCollision->GetPhysicsInfo( 0 ).m_pBody != TNULL )
	{
		m_mBodyFull   = *pCollision->GetPhysicsInfo( 0 ).m_pBody->m_pMatrix;
		m_fBodyEmptyY = m_fWaterDepth + m_mBodyFull.m_f42;
	}

	TMatrix44 mBody   = m_mBodyFull;
	TMatrix44 mEntity = m_mEntityFull;

	if ( ( m_uiWaterFlags & WATERFLAG_Full ) == 0 )
	{
		mBody.m_f42    = m_fBodyEmptyY;
		mEntity.m_f42  = m_fEntityEmptyY;
		m_uiWaterFlags = m_uiWaterFlags | WATERFLAG_StateMask;
	}

	SetWaterMatrices( mBody, mEntity );

	if ( m_uiWaterFlags & WATERFLAG_Ink )
		SetInk( TTRUE, TFALSE );

	if ( m_pModelComponent != TNULL )
		m_pModelComponent->SetTransform( m_oMatrix );

	if ( m_pMesh2 != TNULL )
		m_pMesh2->SetTransform( m_oMatrix );

	TUINT16 uiFlags = m_uiWaterFlags;
	EnableNavSets( ( uiFlags & WATERFLAG_IsInk ) == 0 && ( ( uiFlags & WATERFLAG_StateMask ) != WATERFLAG_StateMask || ( uiFlags & WATERFLAG_EmptyWhenDrained ) == 0 ) );
}

// $deBlob: FUNCTION 0079f950
// $deBlob: FUNCTION 007a184e
void APropWater::ProcessParams( const AEntityParams& a_rParams )
{
	AProp::ProcessParams( a_rParams );

	m_fAmbientScale = 1.0f;

	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetAmbientScale( 1.0f );

	TBOOL bValue = TFALSE;

	a_rParams.GetBool( bValue, "AllowFilling" );
	if ( bValue ) m_uiWaterFlags |= WATERFLAG_AllowFilling;

	a_rParams.GetBool( bValue, "AllowDraining" );
	if ( bValue ) m_uiWaterFlags |= WATERFLAG_AllowDraining;

	a_rParams.GetBool( bValue, "Empty" );
	if ( !bValue ) m_uiWaterFlags |= WATERFLAG_Full;

	a_rParams.GetBool( bValue, "ConvertFromInk" );
	if ( bValue ) m_uiWaterFlags |= WATERFLAG_ConvertFromInk;

	a_rParams.GetBool( bValue, "ConvertToInk" );
	if ( bValue ) m_uiWaterFlags |= WATERFLAG_ConvertToInk;

	a_rParams.GetBool( bValue, "Ink" );
	if ( bValue ) m_uiWaterFlags |= WATERFLAG_Ink;

	// Empty when drained by default
	TBOOL bFound = a_rParams.GetValue( bValue, "EmptyWhenDrained" );
	if ( bValue == TTRUE || !bFound ) m_uiWaterFlags |= WATERFLAG_EmptyWhenDrained;

	TFLOAT fValue = 0.0f;

	a_rParams.GetFloat( fValue, "FillTime" );
	m_fFillTime = fValue;
	a_rParams.GetFloat( fValue, "DrainTime" );
	m_fDrainTime = fValue;
	a_rParams.GetFloat( fValue, "WaterDepth" );
	m_fWaterDepth = fValue;
	a_rParams.GetFloat( fValue, "DelayOnTrigger" );
	m_Flags |= 0x40000;
	m_fDelayOnTrigger = fValue;

	CreateModelCollision( TFALSE );

	// The water bodies are triggers (layer 0x20), not solid
	ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetCollisionInstance() );

	if ( pCollision != TNULL )
	{
		for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
		{
			TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

			if ( pBody != TNULL && ( pBody->m_uiCollisionFilterInfo & 8 ) == 0 )
			{
				TPhysicsManager::GetSingleton()->SetCollisionLayer( pBody, 0x20 );
				TPhysicsManager::GetSingleton()->SetCollisionGroup( pBody, 0 );
			}
		}
	}

	m_mEntityFull   = *TSTATICCAST( const AEntityLoader::EntityData, &a_rParams )->m_pMatrix;
	m_fEntityEmptyY = m_mEntityFull.m_f42 + m_fWaterDepth;

	if ( !( m_uiWaterFlags & WATERFLAG_Full ) )
		m_uiWaterFlags |= WATERFLAG_StateMask;

		// The ink model
#ifdef DEBLOB_COMMUNITY_PATCH
	const TCHAR* szMesh2 = TNULL;
#else
	// NOTE: retail reuses the a_rParams argument slot: without "Mesh2" it reads the params as the model name
	const TCHAR* szMesh2 = TREINTERPRETCAST( const TCHAR*, &a_rParams );
#endif

	if ( ( m_uiWaterFlags & ( WATERFLAG_ConvertToInk | WATERFLAG_Ink ) ) != 0 )
	{
		a_rParams.GetString( szMesh2, "Mesh2" );

		if ( szMesh2 != TNULL && *szMesh2 != 0 )
		{
			m_pMesh2 = new AEntityModelComponent( this, szMesh2 );
			m_pMesh2->GetModelInstance()->SetFlag( AModelInstance2::FLAG( 1 ), TFALSE );
		}
	}

	TBOOL bRenderToFarClip = TFALSE;

	if ( a_rParams.GetValue( bRenderToFarClip, "RenderToFarClip" ) && bRenderToFarClip )
	{
		m_Flags |= 0x4000000;

		AModelInstance2* pModel1 = ( GetModelComponent() != TNULL ) ? TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance() : TNULL;
		AModelInstance2* pModel2 = ( m_pMesh2 != TNULL ) ? m_pMesh2->GetModelInstance() : TNULL;

		if ( pModel1 != TNULL )
			pModel1->SetFlag( AModelInstance2::FLAG( 0x8000 ), TTRUE );

		if ( pModel2 != TNULL )
			pModel2->SetFlag( AModelInstance2::FLAG( 0x8000 ), TTRUE );
	}
}

// $deBlob: FUNCTION 007a0100
void APropWater::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( !( TUINT8( m_Flags >> 24 ) & 1 ) )
	{
		TINT iPacket = a_rPacket.m_eMessage;

		if ( iPacket == AEntityIOPacket::MESSAGE_TRIGGER || iPacket == AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE || iPacket == AEntityIOPacket::MESSAGE_TOGGLE )
		{
			SetTicking( TTRUE );

			if ( m_fDelay <= 0.0f )
			{
				m_pTriggerCaller = a_pCaller;

				if ( m_fDelayOnTrigger <= 0.0f )
					Trigger();
				else
					m_fDelay = m_fDelayOnTrigger;
			}
		}
	}

	AProp::HandleEntityIO( a_pCaller, a_rPacket );
}

// $deBlob: FUNCTION 007a0580
void APropWater::AddToPhysics()
{
	AVisibleEntity::AddToPhysics();

	if ( m_pModelComponent && m_pModelComponent->GetModelInstance()->GetCollisionInstance() )
		TSTATICCAST( ACollisionInstance2, m_pModelComponent->GetModelInstance()->GetCollisionInstance() )->AddToWorld( 0x60 );

	if ( m_pMesh2 && m_pMesh2->GetModelInstance()->GetCollisionInstance() )
		TSTATICCAST( ACollisionInstance2, m_pMesh2->GetModelInstance()->GetCollisionInstance() )->AddToWorld( 0x60 );
}

// $deBlob: FUNCTION 007a05c0
void APropWater::RemoveFromPhysics()
{
	AVisibleEntity::RemoveFromPhysics();

	if ( m_pModelComponent && m_pModelComponent->GetModelInstance()->GetCollisionInstance() )
		TSTATICCAST( ACollisionInstance2, m_pModelComponent->GetModelInstance()->GetCollisionInstance() )->RemoveFromWorld( 0x60 );

	if ( m_pMesh2 && m_pMesh2->GetModelInstance()->GetCollisionInstance() )
		TSTATICCAST( ACollisionInstance2, m_pMesh2->GetModelInstance()->GetCollisionInstance() )->RemoveFromWorld( 0x60 );
}

// $deBlob: FUNCTION 007a0330
void APropWater::Reset( ABaseEntity* a_pEntity )
{
	TMatrix44 mBody   = m_mBodyFull;
	TUINT16   uiFlags = m_uiWaterFlags;
	m_fTime           = 0.0f;
	m_uiWaterFlags    = uiFlags & ~WATERFLAG_StateMask;
	m_fDelay          = 0.0f;
	TMatrix44 mEntity = m_mEntityFull;

	if ( ( uiFlags & WATERFLAG_Full ) == 0 )
	{
		mBody.m_f42    = m_fBodyEmptyY;
		mEntity.m_f42  = m_fEntityEmptyY;
		m_uiWaterFlags = ( uiFlags & ~WATERFLAG_StateMask ) | WATERFLAG_StateMask;
	}

	SetInk( ( m_uiWaterFlags & WATERFLAG_Ink ) != 0, TFALSE );
	SetWaterMatrices( mBody, mEntity );

	if ( m_pModelComponent != TNULL )
		m_pModelComponent->SetTransform( m_oMatrix );

	if ( m_pMesh2 != TNULL )
		m_pMesh2->SetTransform( m_oMatrix );

	uiFlags = m_uiWaterFlags;
	EnableNavSets( ( ( uiFlags & WATERFLAG_IsInk ) == 0 && ( uiFlags & WATERFLAG_StateMask ) != WATERFLAG_StateMask ) || ( uiFlags & WATERFLAG_EmptyWhenDrained ) == 0 );
	SetTicking( TFALSE );

	if ( m_pSound )
	{
		m_pSound->stop( false );
		m_pSound = TNULL;
	}

	m_pTriggerCaller = TNULL;
	m_uiWaterFlags   = ( m_uiWaterFlags & ~WATERFLAG_TransitionMask ) | ( TRANSITION_None << 12 );

	if ( m_pModelComponent != TNULL )
		m_pModelComponent->GetModelInstance()->SetOverrideMaterial( TNULL );

	if ( m_pMesh2 != TNULL )
		m_pMesh2->GetModelInstance()->SetOverrideMaterial( TNULL );

	// Once for all the APropWaters (GetTransition clears it)
	if ( !ms_bTransitionsCleared )
	{
		TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

		for ( TINT i = 0; i < TWorldShaderHAL::MAX_TRANSITIONS; i++ )
			pShader->GetTransitionRef( i ).m_uiOwner = TUINT32( -1 );

		ms_bTransitionsCleared = TTRUE;
	}

	AProp::Reset( a_pEntity );
}

// $deBlob: FUNCTION 007a0660
void APropWater::Despawn( TBOOL a_bPlayEffect )
{
	if ( m_pSound )
	{
		m_pSound->stop( false );
		m_pSound = TNULL;
	}

	RemoveSoundEmitter();
	AVisibleEntity::Despawn( a_bPlayEffect );
}

// $deBlob: FUNCTION 007a06a0
void APropWater::SetWaterMatrices( const TMatrix44& a_rBodyMatrix, const TMatrix44& a_rEntityMatrix )
{
	AEntityModelComponent* pModel     = ( m_uiWaterFlags & WATERFLAG_IsInk ) ? m_pMesh2 : m_pModelComponent;
	ACollisionInstance2*   pCollision = TSTATICCAST( ACollisionInstance2, pModel->GetModelInstance()->GetCollisionInstance() );

	if ( pCollision == TNULL )
		return;

	for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
	{
		TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

		if ( pBody != TNULL )
		{
			*pBody->m_pMatrix = a_rBodyMatrix;
			TPhysicsManager::GetSingleton()->UpdateBodyTransform( pBody, TFALSE );
		}
	}

	SetMatrix( a_rEntityMatrix );
}

// $deBlob: FUNCTION 007a0750
void APropWater::SetInk( TBOOL a_bInk, TBOOL a_bTransition )
{
	AEntityModelComponent* pOldModel;
	TUINT16                uiFlags = m_uiWaterFlags;

	if ( ( uiFlags & WATERFLAG_IsInk ) == 0 )
	{
		if ( !a_bInk )
			return;

		m_uiWaterFlags = uiFlags | WATERFLAG_IsInk;
		EnableNavSets( TFALSE );

		if ( a_bTransition )
		{
			m_uiWaterFlags = m_uiWaterFlags & ~WATERFLAG_TransitionMask;
			SetTicking( TTRUE );
		}

		pOldModel = m_pModelComponent;
	}
	else
	{
		if ( a_bInk )
			return;

		m_uiWaterFlags = uiFlags & ~WATERFLAG_IsInk;
		EnableNavSets( ( uiFlags & WATERFLAG_StateMask ) != WATERFLAG_StateMask );

		if ( a_bTransition )
		{
			m_uiWaterFlags = ( m_uiWaterFlags & 0xDFFF ) | ( TRANSITION_ToWater << 12 );
			SetTicking( TTRUE );
		}

		pOldModel = m_pMesh2;
	}

	ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, pOldModel->GetModelInstance()->GetCollisionInstance() );

	if ( pCollision == TNULL )
		return;

	// The surface of the bodies (ink or water)
	for ( TINT i = 0; i < pCollision->GetNumPhysicsObjects(); i++ )
	{
		TPhysicsObjectContainer* pBody = pCollision->GetPhysicsInfo( i ).m_pBody;

		if ( pBody != TNULL )
			pBody->m_uiSurface = a_bInk ? 0x2000 : 0x10;
	}

	// Swaps the models
	AEntityModelComponent* pModel = m_pModelComponent;
	m_pModelComponent             = m_pMesh2;
	m_pMesh2                      = pModel;

	TBOOL bVisible = ( pModel->GetModelInstance()->GetFlags() & 1 ) != 0;
	m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG( 1 ), bVisible );
	m_pMesh2->GetModelInstance()->SetFlag( AModelInstance2::FLAG( 1 ), TFALSE );
	m_pModelComponent->SetTransform( m_oMatrix );

	TINT  iIndex      = 0;
	TBOOL bNew        = TFALSE;
	TINT  eTransition = ( m_uiWaterFlags >> 12 ) & 3;

	if ( eTransition == TRANSITION_ToWater )
	{
		TASSERT( 0 < m_pModelComponent->GetModelInstance()->GetModelInstance()->GetModel()->GetLODCount() );

		TWorldShaderHAL::TransitionRef* pRef = GetTransition( TUINT32( m_pTriggerCaller ), iIndex, bNew );
		pRef->m_iNumUsers++;
		m_pModelComponent->GetModelInstance()->SetOverrideMaterial( pRef->m_pWaterMat );
	}
	else if ( eTransition == TRANSITION_ToInk )
	{
		TASSERT( 0 < m_pModelComponent->GetModelInstance()->GetModelInstance()->GetModel()->GetLODCount() );

		TWorldShaderHAL::TransitionRef* pRef = GetTransition( TUINT32( m_pTriggerCaller ), iIndex, bNew );
		pRef->m_iNumUsers++;
		m_pModelComponent->GetModelInstance()->SetOverrideMaterial( pRef->m_pInkMat );
	}

	if ( GetLOD() != FLAG_LowLOD )
	{
		TINT eState = ( m_uiWaterFlags >> 10 ) & 3;

		if ( eState == STATE_Full || ( eState == STATE_Empty && ( m_uiWaterFlags & WATERFLAG_EmptyWhenDrained ) == 0 ) )
			AddSoundEmitter();
	}
}

// Copies a_pSource's textures and passes to a_pDest (a transition material)
static void CopyTransitionMaterial( TWorldMaterialHAL* a_pDest, TWorldMaterialHAL* a_pSource )
{
	a_pDest->SetTextureNum( a_pSource->GetTextureNum() );

	for ( TUINT i = 0; i < a_pSource->GetTextureNum(); i++ )
		a_pDest->SetTexture( i, a_pSource->GetTexture( i ) );

	TASSERT( ( a_pSource == TNULL ) || a_pSource->IsA( &TGetClass( TWorldMaterialHAL ) ) );
	TASSERT( ( a_pDest == TNULL ) || a_pDest->IsA( &TGetClass( TWorldMaterialHAL ) ) );

	for ( TINT i = 0; i < TWorldMaterialHAL::s_iMaxPasses; i++ )
	{
		a_pDest->SetTexture( i, a_pSource->GetTexture( i ) );
		a_pDest->SetUVStream( i, a_pSource->GetUVStream( i ) );
		a_pDest->SetMultiPassBlendType( i, a_pSource->GetMultiPassBlendType( i ) );
		a_pDest->SetUVMode( i, a_pSource->GetUVMode( i ) );
	}

	a_pDest->SetFlags( a_pSource->GetFlags(), TTRUE );
	a_pDest->SetMaterialName( a_pSource->GetMaterialName() );
}

// $deBlob: FUNCTION 007a0aa0
void APropWater::Trigger()
{
	// The cross fade only with a single viewport
	TBOOL bTransition = ACameraManager::GetSingleton()->GetNumActiveCameras() == 1;

	if ( ( m_uiWaterFlags & ( WATERFLAG_ConvertToInk | WATERFLAG_ConvertFromInk ) ) != 0 && !ms_bTransitionMaterialsCreated )
	{
		TWorldMaterialHAL* pWaterMat = TSTATICCAST( TWorldMaterialHAL, TWorldMaterialManager::GetSingleton()->FindMaterial( "new_watergood" ) );
		TWorldMaterialHAL* pInkMat   = TSTATICCAST( TWorldMaterialHAL, TWorldMaterialManager::GetSingleton()->FindMaterial( "ink_pool" ) );

		if ( bTransition && pWaterMat != TNULL && pInkMat != TNULL )
		{
			// The transition materials of the slots, copies of the level's water and ink pool materials
			TWorldShaderHAL*   pShader        = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );
			TWorldMaterialHAL* apSources[ 2 ] = { pWaterMat, pInkMat };

			for ( TINT i = 0; i < TWorldShaderHAL::MAX_TRANSITIONS; i++ )
			{
				pShader->GetTransitionRef( i ).m_pWaterMat = TSTATICCAST( TWorldMaterialHAL, pShader->CreateMaterial( "water_trans_mat" ) );
				pShader->GetTransitionRef( i ).m_pWaterMat->Create( 10 );
				pShader->GetTransitionRef( i ).m_pWaterMat->SetOrderTable( pShader->GetOrderTable( 10 ), 0, TTRUE );

				pShader->GetTransitionRef( i ).m_pInkMat = TSTATICCAST( TWorldMaterialHAL, pShader->CreateMaterial( "ink_trans_mat" ) );
				pShader->GetTransitionRef( i ).m_pInkMat->Create( 8 );
				pShader->GetTransitionRef( i ).m_pInkMat->SetOrderTable( pShader->GetOrderTable( 0 ), 0, TTRUE );

				pShader->GetTransitionRef( i ).m_uiOwner = TUINT32( -1 );

				for ( TINT j = 0; j < 2; j++ )
				{
					TWorldMaterialHAL* pDest = ( j == 0 ) ? pShader->GetTransitionRef( i ).m_pWaterMat : pShader->GetTransitionRef( i ).m_pInkMat;
					CopyTransitionMaterial( pDest, apSources[ j ] );
				}
			}

			ms_bTransitionMaterialsCreated = TTRUE;
		}
		else
		{
			bTransition = ms_bTransitionMaterialsCreated && bTransition;
		}
	}

	TUINT16 uiFlags = m_uiWaterFlags;

	if ( ( uiFlags & WATERFLAG_IsInk ) == 0 )
	{
		if ( uiFlags & WATERFLAG_ConvertToInk )
			SetInk( TTRUE, bTransition );
	}
	else if ( uiFlags & WATERFLAG_ConvertFromInk )
	{
		SetInk( TFALSE, bTransition );
	}

	uiFlags = m_uiWaterFlags;

	switch ( ( uiFlags >> 10 ) & 3 )
	{
		case STATE_Full:
			if ( uiFlags & WATERFLAG_AllowDraining )
			{
				ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x66 ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), FMOD_EVENT_DEFAULT );
				RemoveSoundEmitter();
				m_uiWaterFlags = ( m_uiWaterFlags & 0xFBFF ) | ( STATE_Draining << 10 );
			}

			break;
		case STATE_Filling:
			if ( uiFlags & WATERFLAG_AllowDraining )
				m_uiWaterFlags = ( uiFlags & 0xFBFF ) | ( STATE_Draining << 10 );

			// NOTE: retail swaps the times: ( Drain - t ) / Drain * Fill instead of ( Fill - t ) / Fill * Drain (same when equal)
			m_fTime = ( ( m_fDrainTime - m_fTime ) / m_fDrainTime ) * m_fFillTime;
			break;
		case STATE_Draining:
			if ( uiFlags & WATERFLAG_AllowFilling )
				m_uiWaterFlags = ( uiFlags & 0xF7FF ) | ( STATE_Filling << 10 );

			// NOTE: the same swap
			m_fTime = ( ( m_fFillTime - m_fTime ) / m_fFillTime ) * m_fDrainTime;
			break;
		case STATE_Empty:
			if ( uiFlags & WATERFLAG_AllowFilling )
			{
				m_uiWaterFlags = ( uiFlags & 0xF7FF ) | ( STATE_Filling << 10 );
				EnableNavSets( ( ( ~( m_uiWaterFlags >> 6 ) ) & 1 ) != 0 );
				ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( 0x68 ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), FMOD_EVENT_DEFAULT );
			}

			break;
	}
}

// $deBlob: FUNCTION 007a1060
void APropWater::EnableNavSets( TBOOL a_bEnable )
{
	// The water flag (1) of the markers of the nav sets
	// NOTE: retail's enabling loop reads the nav set IDs without the index assert
	for ( TINT i = 0; i < m_uiNumNavSets; i++ )
	{
		ANavMarkers::NavSet& rNavSet = ANavMarkers::GetSingleton()->GetData()->m_pNavSets[ GetNavSetID( i ) ];

		for ( TINT j = 0; j < rNavSet.m_iNumMarkers; j++ )
		{
			if ( a_bEnable )
				rNavSet.m_pMarkers[ j ].m_uiFlags |= 1;
			else
				rNavSet.m_pMarkers[ j ].m_uiFlags &= ~1;
		}
	}
}

// $deBlob: FUNCTION 007a1260
void APropWater::AddSoundEmitter()
{
	TUINT16 uiFlags = m_uiWaterFlags;

	if ( ( uiFlags & WATERFLAG_InkEmitter ) == 0 && ( uiFlags & WATERFLAG_IsInk ) != 0 )
	{
		RemoveSoundEmitter();
		APositionalSoundManager::GetSingleton()->AddEntitySound( this, 8, 0x8c );
		m_uiWaterFlags |= WATERFLAG_InkEmitter;
		return;
	}

	if ( ( uiFlags & WATERFLAG_WaterEmitter ) == 0 && ( uiFlags & WATERFLAG_IsInk ) == 0 )
	{
		RemoveSoundEmitter();
		APositionalSoundManager::GetSingleton()->AddEntitySound( this, 8, 0x9a );
		m_uiWaterFlags |= WATERFLAG_WaterEmitter;
	}
}

// $deBlob: FUNCTION 007a12e0
void APropWater::RemoveSoundEmitter()
{
	if ( m_uiWaterFlags & WATERFLAG_InkEmitter )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x8c );
		m_uiWaterFlags &= ~WATERFLAG_InkEmitter;
	}

	if ( m_uiWaterFlags & WATERFLAG_WaterEmitter )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( this, 8, 0x9a );
		m_uiWaterFlags &= ~WATERFLAG_WaterEmitter;
	}
}

// $deBlob: FUNCTION 007a1370
void APropWater::UpdateSound()
{
	TFLOAT fPitch = 0.5f;
	TFLOAT fProgress;

	switch ( ( m_uiWaterFlags >> 10 ) & 3 )
	{
		case STATE_Full:
		case STATE_Empty:
			if ( m_pSound != TNULL )
			{
				m_pSound->stop( false );
				m_pSound = TNULL;
			}

			return;
		case STATE_Filling:
			if ( m_fFillTime <= 0.0f )
				goto play;

			fProgress = m_fTime / m_fFillTime;
			fPitch    = fProgress * fProgress * 0.25f + 0.25f;
			break;
		case STATE_Draining:
			if ( m_fDrainTime <= 0.0f )
				goto play;

			fProgress = 1.0f - m_fTime / m_fDrainTime;
			fPitch    = fProgress * fProgress * 0.25f + 0.25f;
			break;
	}

play:
	if ( m_pSound == TNULL )
		m_pSound = ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID_INTERACTABLES, ASoundMap::EVENTID( ( m_uiWaterFlags & WATERFLAG_IsInk ) ? 0x69 : 0x5F ), m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ), FMOD_EVENT_DEFAULT );

	if ( m_pSound != TNULL )
		m_pSound->setPitch( fPitch, FMOD_EVENT_PITCHUNITS_RAW );
}

// $deBlob: FUNCTION 007a1490
TWorldShaderHAL::TransitionRef* APropWater::GetTransition( TUINT32 a_uiOwner, TINT& a_riIndex, TBOOL& a_rbNew )
{
	ms_bTransitionsCleared = TFALSE;

	TWorldShaderHAL* pShader = TSTATICCAST( TWorldShaderHAL, TWorldShader::GetSingleton() );

	for ( TINT i = 0; i < TWorldShaderHAL::MAX_TRANSITIONS; i++ )
	{
		if ( a_uiOwner == pShader->GetTransitionRef( i ).m_uiOwner )
		{
			a_riIndex = i;
			a_rbNew   = TFALSE;
			return &pShader->GetTransitionRef( i );
		}
	}

	for ( TINT i = 0; i < TWorldShaderHAL::MAX_TRANSITIONS; i++ )
	{
		if ( pShader->GetTransitionRef( i ).m_uiOwner == TUINT32( -1 ) )
		{
			TWorldShaderHAL::TransitionRef& rRef = pShader->GetTransitionRef( i );
			rRef.m_uiOwner                       = a_uiOwner;
			rRef.m_fProgress                     = 0.0f;
			rRef.m_pInkMat->SetUserDataIndex( TWorldMaterialHAL::UData_0, -1 );
			rRef.m_pWaterMat->SetUserDataIndex( TWorldMaterialHAL::UData_0, -1 );
			rRef.m_iNumUsers = 0;

			a_riIndex = i;
			a_rbNew   = TTRUE;
			return &rRef;
		}
	}

	// NOTE: no free slot: retail returns the first one and leaves a_riIndex and a_rbNew as they are
	return &pShader->GetTransitionRef( 0 );
}
