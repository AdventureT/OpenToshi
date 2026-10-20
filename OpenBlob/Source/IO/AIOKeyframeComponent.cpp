#include "pch.h"
#include "AIOKeyframeComponent.h"
#include "IO/AIOEntity.h"

#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007cc840
AIOKeyframeComponent::AIOKeyframeComponent( AIOEntity* a_pOwner )
    : AIOEntityComponent( a_pOwner, KEYFRAME_COMPONENT )
{
	m_uiFlags |= 1;
	m_iLoopCount  = 0;
	m_pTrackSet   = TNULL;
	m_eState      = STATE_Stopped;
	m_uiPlayFlags = 0;
}

// $deBlob: FUNCTION 007ccd20
// $deBlob: FUNCTION 007cc880
AIOKeyframeComponent::~AIOKeyframeComponent()
{
}

// $deBlob: FUNCTION 007cc890
void AIOKeyframeComponent::SetIOEntityData( void* a_pIOEntityData, TTRB* a_pTRB )
{
	IOEntityData* a_pData = TSTATICCAST( IOEntityData, a_pIOEntityData );
	TASSERT( KEYFRAME_COMPONENT == a_pData->componentType );

	m_pTrackSet = TSTATICCAST( ANEDKeyframeInterpolation::TrackSet, a_pTRB->GetSymbolAddress( *a_pData->pData ) );
}

// $deBlob: FUNCTION 007cc8f0
TBOOL AIOKeyframeComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	switch ( a_pMessage[ 0 ] )
	{
		case ACTION_Play:
			if ( m_eState == STATE_Stopped )
				Play( *TREINTERPRETCAST( PlayParams**, a_pMessage + 2 ) );

			return TTRUE;
		case ACTION_Pause:
			if ( m_eState == STATE_Playing )
			{
				m_eState = STATE_Paused;
				return TTRUE;
			}

			return TTRUE;
		case ACTION_Resume:
			if ( m_eState == STATE_Paused )
			{
				m_eState = STATE_Playing;
				return TTRUE;
			}

			return TTRUE;
		case ACTION_Stop:
			m_eState = STATE_Stopped;
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007cc970
void AIOKeyframeComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_eState != STATE_Playing )
		return;

	ANEDKeyframeInterpolation::TrackSet* pTrackSet = m_pTrackSet;

	for ( TINT i = 0; i < pTrackSet->iNumTracks; i++ )
	{
		ANEDKeyframeInterpolation::Track* pTrack = &pTrackSet->pTracks[ i ];

		if ( pTrack->fStartTime <= pTrackSet->fTime && pTrackSet->fTime <= pTrack->fEndTime )
		{
			for ( TINT j = 0; j < pTrack->iNumKeyLists; j++ )
				TickKeyList( &pTrack->pKeyLists[ j ], pTrackSet->fTime, pTrack->pEntity );
		}
	}

	if ( !( m_uiPlayFlags & PLAYFLAGS_Reverse ) )
	{
		pTrackSet->fTime += a_fDeltaTime;

		if ( pTrackSet->fTime <= pTrackSet->fEndTime )
			return;

		pTrackSet->fTime = pTrackSet->fEndTime;

		if ( m_uiPlayFlags & PLAYFLAGS_PingPong )
			m_uiPlayFlags |= PLAYFLAGS_Reverse;
		else if ( m_uiPlayFlags & ( PLAYFLAGS_Loop | PLAYFLAGS_Counted ) )
			pTrackSet->fTime = 0.0f;
	}
	else
	{
		pTrackSet->fTime -= a_fDeltaTime;

		if ( 0.0f <= pTrackSet->fTime )
			return;

		// NOTE: without ping pong or loop the time stays below 0
		if ( m_uiPlayFlags & PLAYFLAGS_PingPong )
		{
			pTrackSet->fTime = 0.0f;
			m_uiPlayFlags &= ~PLAYFLAGS_Reverse;
		}
		else if ( m_uiPlayFlags & ( PLAYFLAGS_Loop | PLAYFLAGS_Counted ) )
		{
			pTrackSet->fTime = pTrackSet->fEndTime;
		}
	}

	// NOTE: a counted loop (PLAYFLAGS_Counted without PLAYFLAGS_Loop) stops after one pass, its count is only
	// decremented
	if ( ( ( m_uiPlayFlags & PLAYFLAGS_Counted ) && --m_iLoopCount < 1 ) || !( m_uiPlayFlags & PLAYFLAGS_Loop ) )
		m_eState = STATE_Stopped;
}

// $deBlob: FUNCTION 007ccb40
void AIOKeyframeComponent::Unk04()
{
	ANEDKeyframeInterpolation::TrackSet* pTrackSet = m_pTrackSet;

	for ( TINT i = 0; i < pTrackSet->iNumTracks; i++ )
		pTrackSet->pTracks[ i ].pEntity = ABaseEntity::FindIDEntity( pTrackSet->pTracks[ i ].iEntityID );

	m_pOwner->SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007ccbe0
void AIOKeyframeComponent::TickKeyList( ANEDKeyframeInterpolation::KeyList* a_pKeyList, TFLOAT a_fTime, ABaseEntity* a_pEntity )
{
	TINT iKey = ANEDKeyframeInterpolation::FindKey( a_pKeyList, a_fTime );

	if ( iKey > -1 && iKey + 1 < a_pKeyList->iNumKeys )
	{
		static TMatrix44 s_oMatrix;

		ANEDKeyframeInterpolation::TransformKeyData* pKey1 = &a_pKeyList->pKeys[ iKey ];
		ANEDKeyframeInterpolation::TransformKeyData* pKey2 = &a_pKeyList->pKeys[ iKey + 1 ];

		ANEDKeyframeInterpolation::InterpolateMatrix( pKey1, pKey2, ( a_fTime - pKey1->fTime ) / ( pKey2->fTime - pKey1->fTime ), s_oMatrix );
		a_pEntity->SetMatrix( s_oMatrix );
	}
}

// $deBlob: FUNCTION 007ccc80
void AIOKeyframeComponent::Play( const PlayParams* a_pParams )
{
	PLAYFLAGS uiOldFlags = m_uiPlayFlags;
	m_uiPlayFlags        = 0;

	if ( a_pParams->iLoop != 0 )
	{
		if ( a_pParams->iCount < 1 )
		{
			m_uiPlayFlags = PLAYFLAGS_Loop;
		}
		else
		{
			m_uiPlayFlags = PLAYFLAGS_Counted;
			m_iLoopCount  = TINT16( a_pParams->iCount );
		}
	}

	if ( a_pParams->iMode == 1 )
	{
		m_uiPlayFlags |= PLAYFLAGS_Reverse;
	}
	else if ( a_pParams->iMode == 2 )
	{
		m_uiPlayFlags |= PLAYFLAGS_PingPong;
		m_eState = STATE_Playing;
		return;
	}
	else if ( a_pParams->iMode == 3 )
	{
		// Toggles the direction
		if ( !( uiOldFlags & PLAYFLAGS_Reverse ) )
		{
			m_uiPlayFlags |= PLAYFLAGS_Reverse;

			if ( a_pParams->iRestart != 0 )
			{
				m_pTrackSet->fTime = m_pTrackSet->fEndTime;
				m_eState           = STATE_Playing;
				return;
			}
		}
		else if ( a_pParams->iRestart != 0 )
		{
			m_pTrackSet->fTime = 0.0f;
			m_eState           = STATE_Playing;
			return;
		}
	}

	m_eState = STATE_Playing;
}
