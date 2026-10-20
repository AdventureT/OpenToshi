#include "pch.h"
#include "APositionalSoundManager.h"
#include "ASoundInterface.h"
#include "ACurrentLevelInformation.h"
#include "Cameras/ACameraManager.h"
#include "Terrain2/TerrainTypes.h"
#include "Terrain2/ACellBucket.h"
#include "Animation/AModelInstance2.h"
#include "Collision/ACollisionInstance2.h"

#include <Sound/TSound.h>
#include <Render/TTransformObject.h>
#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Toshi/T2String8.h>

#include <fmod_event.hpp>

//-----------------------------------------------------------------------------
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The position a sound follows
// $deBlob: FUNCTION 0060ea80
// NOTE: the default target's layout is unknown: an object at 0x1C whose TTransformObject is at its 0x1C
static const TVector4* GetSoundPosition( APositionalSoundManager::SoundData* a_pData )
{
	TUINT16 uiFlags = a_pData->m_uiFlags;

	if ( uiFlags & APositionalSoundManager::FLAG_OwnPosition )
		return &a_pData->m_vPosition;

	TUINT8* pTarget = TSTATICCAST( TUINT8, a_pData->m_pTarget );

	if ( uiFlags & APositionalSoundManager::FLAG_EntityPosition )
		return TREINTERPRETCAST( const TVector4*, pTarget + 0x40 );

	if ( uiFlags & APositionalSoundManager::FLAG_MatrixPtr )
		return &( *TREINTERPRETCAST( TMatrix44**, pTarget ) )->GetTranslation();

	if ( uiFlags & APositionalSoundManager::FLAG_TargetPosition )
		return TREINTERPRETCAST( const TVector4*, pTarget + 0x18 );

	TUINT8*           pObject    = *TREINTERPRETCAST( TUINT8**, pTarget + 0x1C );
	TTransformObject* pTransform = TREINTERPRETCAST( TTransformObject*, pObject + 0x1C );

	return &pTransform->GetMatrix().GetTranslation();
}

// The collision category flags of a body
static TUINT32 GetCollisionCategoryFlags( TPhysicsObjectContainer* POC )
{
	TASSERT( POC != TNULL );
	return POC ? POC->m_uiSurface : 0;
}

// $deBlob: FUNCTION 006110e0
static TINT8 IntToInt8( TINT a_iX )
{
	TASSERT( ( a_iX >= TMath::TINT8_MIN ) && ( a_iX <= TMath::TINT8_MAX ) );
	return TINT8( a_iX );
}

// $deBlob: FUNCTION 0060ead0
TBOOL APositionalSoundManager::EventDesc::HasTarget( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID )
{
	if ( TINT16( a_iGroupID ) != -1 && TINT16( a_iEventID ) != -1 &&
	     ( TINT16( a_iGroupID ) != m_iGroupID || TINT16( a_iEventID ) != m_iEventID ) )
		return TFALSE;

	T2_FOREACH( m_oPositionalList, it )
	{
		if ( it->m_pData->m_pTarget == a_pTarget )
			return TTRUE;
	}

	return TFALSE;
}

// Picks the sounds closest to the camera (up to m_iMaxInstances, sorted to the front) and starts the ones that want
// to play, stopping farther ones when all instances are used
// $deBlob: FUNCTION 0060eb80
void APositionalSoundManager::EventDesc::Update()
{
	auto itStart = m_oPositionalList.Begin();
	auto itEnd   = m_oPositionalList.End();

	if ( itStart == itEnd )
		return;

	const TVector4& rcCameraPos = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

	TBOOL bFirst = TTRUE;

	for ( TINT i = 0; i < m_iMaxInstances; i++ )
	{
		Positional* pBest = itStart;

		for ( Positional* pIt = itStart; pIt != itEnd; pIt = TSTATICCAST( Positional, pIt->Next() ) )
		{
			if ( bFirst )
				pIt->m_pData->m_fDistanceSq = TVector4::DistanceSq( rcCameraPos, *GetSoundPosition( pIt->m_pData ) );

			if ( pBest != pIt && ( pBest->m_pData->m_uiFlags & ( FLAG_Playing | FLAG_WantsToPlay ) ) &&
			     pIt->m_pData->m_fDistanceSq < pBest->m_pData->m_fDistanceSq )
				pBest = pIt;
		}

		if ( pBest == itStart )
		{
			itStart = TSTATICCAST( Positional, itStart->Next() );
		}
		else
		{
			pBest->Remove();
			pBest->InsertBefore( itStart );
		}

		bFirst = TFALSE;

		if ( itStart == itEnd )
			break;
	}

	Positional* pFarthest = itStart;
	TINT        iNumUsed  = 0;

	for ( Positional* pIt = m_oPositionalList.Begin(); pIt != itEnd; pIt = TSTATICCAST( Positional, pIt->Next() ) )
	{
		if ( m_iMaxInstances <= iNumUsed )
			return;

		TUINT16 uiFlags = pIt->m_pData->m_uiFlags;

		if ( uiFlags & FLAG_Disabled )
			continue;

		iNumUsed++;

		if ( !( uiFlags & FLAG_WantsToPlay ) )
			continue;

		if ( m_iMaxInstances <= m_iNumInstaces )
		{
			// Stop a farther playing sound
			TBOOL bStopped = TFALSE;

			do
			{
				if ( pFarthest == itEnd )
					break;

				if ( pFarthest->m_pData->m_uiFlags & FLAG_Playing )
				{
					FMOD::Event* pEvent = pFarthest->m_pData->m_pEvent;

					if ( pEvent == TNULL )
					{
						bStopped = TFALSE;
					}
					else
					{
						pEvent->stop();
						pFarthest->m_pData->m_pEvent = TNULL;
						pFarthest->m_pData->m_uiFlags |= FLAG_Stopped | FLAG_WantsToPlay;
						pFarthest->m_pData->m_uiFlags &= ~FLAG_Playing;
						bStopped = TTRUE;
						m_iNumInstaces--;
					}
				}

				pFarthest = TSTATICCAST( Positional, pFarthest->Next() );
			} while ( !bStopped );

			if ( !bStopped )
				continue;
		}

		StartInstance( m_iGroupID, m_iEventID, pIt );
	}
}

// $deBlob: FUNCTION 0060ee60
void APositionalSoundManager::EventDesc::RestartSounds( TUINT16 a_uiFlags )
{
	for ( Positional* pIt = m_oPositionalList.Begin(); pIt != m_oPositionalList.End(); )
	{
		SoundData* pData = pIt->m_pData;

		if ( ( pData->m_uiFlags & a_uiFlags ) && ( pData->m_uiFlags & FLAG_Restart ) )
		{
			pData->m_pEvent = ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_iGroupID ), ASoundMap::EVENTID( m_iEventID ), *GetSoundPosition( pData ), 0 );
			pIt->m_pData->m_uiFlags &= FLAG_Restart;
		}

#ifdef DEBLOB_COMMUNITY_PATCH
		pIt = TSTATICCAST( Positional, pIt->Next() );
#else
		// NOTE: retail never moves to the next sound: an endless loop once the list isn't empty
#endif
	}
}

// $deBlob: FUNCTION 0060ef10
TBOOL APositionalSoundManager::EventDesc::SetPitch( void* a_pTarget, TFLOAT a_fPitch )
{
	T2_FOREACH( m_oPositionalList, it )
	{
		if ( m_iMaxInstances < 1 )
			return TFALSE;

		SoundData* pData = it->m_pData;

		if ( pData->m_pTarget == a_pTarget && pData->m_pEvent != TNULL )
		{
			pData->m_pEvent->setPitch( a_fPitch, FMOD_EVENT_PITCHUNITS_RAW );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0060ef90
TBOOL APositionalSoundManager::EventDesc::StartInstance( TINT a_iGroupID, TINT a_iEventID, Positional* a_pPositional )
{
	SoundData* pData = a_pPositional->m_pData;

	if ( pData->m_uiFlags & FLAG_Pitched )
		pData->m_uiFlags |= FLAG_Restart;
	else
		pData->m_pEvent = ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( a_iGroupID ), ASoundMap::EVENTID( a_iEventID ), *GetSoundPosition( pData ), FMOD_EVENT_DEFAULT );

	a_pPositional->m_pData->m_uiFlags |= FLAG_Playing;
	a_pPositional->m_pData->m_uiFlags &= ~( FLAG_Stopped | FLAG_WantsToPlay );
	m_iNumInstaces++;

	return TTRUE;
}

// $deBlob: FUNCTION 0060f030
TBOOL APositionalSoundManager::EventDesc::StopInstance( Positional* a_pPositional )
{
	if ( a_pPositional == TNULL || a_pPositional->m_pData->m_pEvent == TNULL )
		return TFALSE;

	a_pPositional->m_pData->m_pEvent->stop( true );
	a_pPositional->m_pData->m_pEvent = TNULL;
	a_pPositional->m_pData->m_uiFlags |= FLAG_Stopped | FLAG_WantsToPlay;
	a_pPositional->m_pData->m_uiFlags &= ~FLAG_Playing;
	m_iNumInstaces--;

	return TTRUE;
}

// $deBlob: FUNCTION 0060f090
TBOOL APositionalSoundManager::EventDesc::Remove( void* a_pTarget, T2DList<Positional>* a_pFreeList )
{
	T2_FOREACH( m_oPositionalList, it )
	{
		SoundData* pData = it->m_pData;

		if ( pData->m_pTarget != a_pTarget )
			continue;

		if ( pData->m_pEvent != TNULL )
		{
			pData->m_pEvent->stop();
			pData->m_pEvent = TNULL;
		}

		if ( pData->m_uiFlags & FLAG_Playing )
			m_iNumInstaces--;

		TASSERT( m_iNumInstaces >= 0 );

		Positional* pPositional = it;
		pPositional->Remove();
		a_pFreeList->PushBack( pPositional );
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0060f150
APositionalSoundManager::APositionalSoundManager()
{
	m_pPositionals  = TNULL;
	m_pEmitters     = TNULL;
	m_pSoundTRBData = TNULL;
	m_pEventDescs   = TNULL;
	m_uiFlags       = 0;

	m_pPositionals = new Positional[ MAX_POSITIONALS ];
	m_pEventDescs  = new EventDesc[ MAX_EVENTDESCS ];

	for ( TINT i = 0; i < MAX_POSITIONALS; i++ )
	{
		m_pPositionals[ i ].m_pData           = new SoundData;
		m_pPositionals[ i ].m_pData->m_iUnk28 = -1;
		m_oFreePositionals.PushBack( &m_pPositionals[ i ] );
	}

	for ( TINT i = 0; i < MAX_EVENTDESCS; i++ )
		m_oFreeDescs.PushBack( &m_pEventDescs[ i ] );
}

// $deBlob: FUNCTION 0060f300
// $deBlob: FUNCTION 0056c220
APositionalSoundManager::~APositionalSoundManager()
{
	Reset();
	m_oFreePositionals.Clear();
	m_oFreeDescs.Clear();

	if ( m_pPositionals )
		delete[] m_pPositionals;

	if ( m_pEventDescs )
		delete[] m_pEventDescs;
}

// The entity's matrix translation
// $deBlob: FUNCTION 0060f4a0
TBOOL APositionalSoundManager::AddEntitySound( void* a_pEntity, TINT a_iGroupID, TINT a_iEventID )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, a_pEntity ) )
		return TFALSE;

	pPositional->m_pData->m_pTarget   = a_pEntity;
	pPositional->m_pData->m_vPosition = *TREINTERPRETCAST( const TVector4*, TSTATICCAST( TUINT8, a_pEntity ) + 0x40 );
	pPositional->m_pData->m_uiFlags   = FLAG_EntityPosition | FLAG_WantsToPlay;
	return TTRUE;
}

// AddEntitySound with the entity's velocity (0x134)
// $deBlob: FUNCTION 0060f500
TBOOL APositionalSoundManager::AddMovingEntitySound( void* a_pEntity, TINT a_iGroupID, TINT a_iEventID )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, a_pEntity ) )
		return TFALSE;

	pPositional->m_pData->m_pTarget   = a_pEntity;
	pPositional->m_pData->m_vPosition = *TREINTERPRETCAST( const TVector4*, TSTATICCAST( TUINT8, a_pEntity ) + 0x40 );
	pPositional->m_pData->m_uiFlags   = FLAG_EntityPosition | FLAG_WantsToPlay;
	pPositional->m_pData->m_uiFlags |= FLAG_EntityVelocity;
	return TTRUE;
}

// $deBlob: FUNCTION 0060f450
TBOOL APositionalSoundManager::AddMatrixSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, a_pTarget ) )
		return TFALSE;

	pPositional->m_pData->m_pTarget = a_pTarget;
	pPositional->m_pData->m_uiFlags = FLAG_MatrixPtr | FLAG_WantsToPlay;
	return TTRUE;
}

// $deBlob: FUNCTION 0060f400
TBOOL APositionalSoundManager::AddTransformSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, a_pTarget ) )
		return TFALSE;

	pPositional->m_pData->m_pTarget = a_pTarget;
	pPositional->m_pData->m_uiFlags = FLAG_Unk800 | FLAG_WantsToPlay;
	return TTRUE;
}

// $deBlob: FUNCTION 0060f750
TBOOL APositionalSoundManager::AddTargetSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, TNULL ) )
		return TFALSE;

	pPositional->m_pData->m_uiFlags = FLAG_TargetPosition | FLAG_WantsToPlay;
	pPositional->m_pData->m_pTarget = a_pTarget;
	return TTRUE;
}

// $deBlob: FUNCTION 0060f6f0
TBOOL APositionalSoundManager::AddPositionSound( const TVector4* a_pPosition, TINT a_iGroupID, TINT a_iEventID )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, TNULL ) )
		return TFALSE;

	pPositional->m_pData->m_pTarget   = TNULL;
	pPositional->m_pData->m_uiFlags   = FLAG_OwnPosition | FLAG_WantsToPlay;
	pPositional->m_pData->m_vPosition = *a_pPosition;
	return TTRUE;
}

// $deBlob: FUNCTION 0060fea0
TBOOL APositionalSoundManager::AddPitchedPositionSound( const TVector4* a_pPosition, TINT a_iGroupID, TINT a_iEventID, Positional** a_ppOutPositional )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, TNULL ) )
		return TFALSE;

	pPositional->m_pData->m_pTarget   = TNULL;
	pPositional->m_pData->m_uiFlags   = FLAG_OwnPosition | FLAG_WantsToPlay;
	pPositional->m_pData->m_vPosition = *a_pPosition;
	pPositional->m_pData->m_uiFlags |= FLAG_Pitched;
	*a_ppOutPositional = pPositional;
	return TTRUE;
}

// $deBlob: FUNCTION 0060ff10
TBOOL APositionalSoundManager::AddPitchedEntitySound( void* a_pEntity, TINT a_iGroupID, TINT a_iEventID )
{
	Positional* pPositional = TNULL;

	if ( !AllocPositional( pPositional, a_iGroupID, a_iEventID, a_pEntity ) )
		return TFALSE;

	pPositional->m_pData->m_pTarget   = a_pEntity;
	pPositional->m_pData->m_vPosition = *TREINTERPRETCAST( const TVector4*, TSTATICCAST( TUINT8, a_pEntity ) + 0x40 );
	pPositional->m_pData->m_uiFlags   = FLAG_EntityPosition | FLAG_WantsToPlay;
	pPositional->m_pData->m_uiFlags |= FLAG_Pitched;
	return TTRUE;
}

// $deBlob: FUNCTION 0060f570
TBOOL APositionalSoundManager::LoadEmitters( void* a_pUnused )
{
	TASSERT( m_pSoundTRBData == TNULL );

	TTRB* pTRB      = ACurrentLevelInformation::GetSingleton()->Stream( 0xF );
	m_pSoundTRBData = TSTATICCAST( SoundEmitterHeader, pTRB->GetSymbolAddress( "SoundEmitterHeader" ) );

	TINT iNumEmitters = m_pSoundTRBData->m_iNumEmitters;

	if ( iNumEmitters > 0 )
	{
		m_pEmitters = new Positional[ iNumEmitters ];

		for ( TINT i = 0; i < m_pSoundTRBData->m_iNumEmitters; i++ )
			AddEmitter( &m_pEmitters[ i ], &m_pSoundTRBData->m_pEmitters[ i ] );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0060f7a0
TBOOL APositionalSoundManager::AddEmitter( Positional* a_pPositional, SoundData* a_pData )
{
	a_pPositional->m_pData = a_pData;
	EventDesc* pDesc       = GetEventDesc( a_pData->m_iGroupID, a_pData->m_iEventID );

	if ( a_pPositional->m_pData->m_uiFlags & FLAG_Emitter8000 )
		a_pPositional->m_pData->m_uiFlags |= FLAG_Disabled;

	a_pPositional->m_pData->m_uiFlags |= FLAG_Static | FLAG_OwnPosition | FLAG_WantsToPlay;
	pDesc->m_oPositionalList.PushFront( a_pPositional );
	return TTRUE;
}

// $deBlob: FUNCTION 0060f800
APositionalSoundManager::Positional* APositionalSoundManager::PopFreePositional()
{
	if ( m_oFreePositionals.Size() == 0 )
		return TNULL;

	TASSERT( !m_oFreePositionals.IsEmpty() );
	Positional* pPositional = m_oFreePositionals.Begin();
	pPositional->Remove();
	return pPositional;
}

// Returns the descriptor of the event, taking a free one if it has none (TNULL if none is left)
// $deBlob: FUNCTION 0060f860
APositionalSoundManager::EventDesc* APositionalSoundManager::GetEventDesc( TINT a_iGroupID, TINT a_iEventID )
{
	T2_FOREACH( m_oActivePosDesc, it )
	{
		if ( it->m_iEventID == a_iEventID && it->m_iGroupID == a_iGroupID )
			return it;
	}

	if ( m_oFreeDescs.Begin() == m_oFreeDescs.End() )
		return TNULL;

	TASSERT( !m_oFreeDescs.IsEmpty() );
	EventDesc* pDesc = m_oFreeDescs.Begin();
	TSTATICCAST( T2DList<EventDesc>::Node, pDesc )->Remove();

	pDesc->m_iGroupID      = TINT16( a_iGroupID );
	pDesc->m_iEventID      = TINT16( a_iEventID );
	FMOD::Event* pEvent    = ASoundInterface::GetEvent( ASoundMap::GROUPID( TINT16( a_iGroupID ) ), ASoundMap::EVENTID( TINT16( a_iEventID ) ), FMOD_EVENT_INFOONLY );
	pDesc->m_iMaxInstances = 0;

	if ( pEvent != TNULL )
	{
		TINT iMaxPlaybacks = 0;
		pEvent->getPropertyByIndex( FMOD_EVENTPROPERTY_MAX_PLAYBACKS, &iMaxPlaybacks, TFALSE );
		pDesc->m_iMaxInstances = IntToInt8( iMaxPlaybacks );
	}

	pDesc->m_iNumInstaces = 0;
	m_oActivePosDesc.PushBack( pDesc );
	return pDesc;
}

// $deBlob: FUNCTION 0060f960
TBOOL APositionalSoundManager::RemoveSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID )
{
	T2_FOREACH( m_oActivePosDesc, it )
	{
		if ( it->m_iEventID != a_iEventID || it->m_iGroupID != a_iGroupID )
			continue;

		EventDesc* pDesc = it;
		pDesc->Remove( a_pTarget, &m_oFreePositionals );

		if ( pDesc->m_oPositionalList.IsEmpty() )
		{
			TSTATICCAST( T2DList<EventDesc>::Node, pDesc )->Remove();
			m_oFreeDescs.PushBack( pDesc );
		}

		return TTRUE;
	}

	return TFALSE;
}

// Stops every sound and returns them to the free lists, unloads the level emitters
// $deBlob: FUNCTION 0060fa00
// $deBlob: FUNCTION 006100c0
void APositionalSoundManager::Reset()
{
	for ( auto itDesc = m_oActivePosDesc.Begin(); itDesc != m_oActivePosDesc.End(); )
	{
		EventDesc* pDesc = itDesc;

		for ( auto it = pDesc->m_oPositionalList.Begin(); it != pDesc->m_oPositionalList.End(); )
		{
			Positional* pPositional = it;
			it                      = it->Next();

			pPositional->Remove();

			if ( !( pPositional->m_pData->m_uiFlags & FLAG_Static ) )
				m_oFreePositionals.PushBack( pPositional );
			else
				pPositional->m_pData = TNULL;
		}

		TASSERT( 0 == pDesc->m_oPositionalList.Size() );

		itDesc = itDesc->Next();
		TSTATICCAST( T2DList<EventDesc>::Node, pDesc )->Remove();
		m_oFreeDescs.PushBack( pDesc );
	}

	m_pSoundTRBData = TNULL;

	if ( m_pEmitters )
		delete[] m_pEmitters;

	m_pEmitters = TNULL;

	TASSERT( 0 == m_oActivePosDesc.Size() );
}

// $deBlob: FUNCTION 0060fc50
void APositionalSoundManager::Update()
{
	if ( !m_oActivePosDesc.IsEmpty() )
		UpdatePositions();
}

// $deBlob: FUNCTION 0060fc70
void APositionalSoundManager::UpdatePositions()
{
	if ( m_uiFlags & 1 )
		return;

	T2_FOREACH( m_oActivePosDesc, itDesc )
	{
		itDesc->Update();

		T2_FOREACH( itDesc->m_oPositionalList, it )
		{
			SoundData* pData = it->m_pData;

			if ( !( pData->m_uiFlags & FLAG_Playing ) )
				continue;

			FMOD::Event*     pEvent  = pData->m_pEvent;
			FMOD_EVENT_STATE uiState = 0;

#ifdef DEBLOB_COMMUNITY_PATCH
			if ( pEvent )
				pEvent->getState( &uiState );
#else
			pEvent->getState( &uiState );
#endif

			if ( pEvent == TNULL || !( uiState & FMOD_EVENT_STATE_CHANNELSACTIVE ) )
				continue;

			FMOD_VECTOR  vPosition = ASoundInterface::GetFMODVector( *GetSoundPosition( pData ) );
			FMOD_VECTOR  vVelocity;
			FMOD_VECTOR* pVelocity;

			if ( pData->m_uiFlags & FLAG_EntityVelocity )
			{
				vVelocity = ASoundInterface::GetFMODVector( *TREINTERPRETCAST( const TVector4*, TSTATICCAST( TUINT8, pData->m_pTarget ) + 0x134 ) );
				pVelocity = &vVelocity;
			}
			else if ( pData->m_uiFlags & FLAG_TargetPosition )
			{
				TVector4 vTargetVelocity = *TREINTERPRETCAST( const TVector4*, TSTATICCAST( TUINT8, pData->m_pTarget ) + 0x28 );
				vTargetVelocity.Multiply( 2.0f );
				vVelocity = ASoundInterface::GetFMODVector( vTargetVelocity );
				pVelocity = &vVelocity;
			}
			else
			{
				pVelocity = TNULL;
			}

			pEvent->set3DAttributes( &vPosition, pVelocity );
		}
	}
}

// $deBlob: FUNCTION 0060ffc0
void APositionalSoundManager::StopPitchedSounds()
{
	T2_FOREACH( m_oActivePosDesc, itDesc )
	{
		T2_FOREACH( itDesc->m_oPositionalList, it )
		{
			SoundData* pData = it->m_pData;

			if ( ( pData->m_uiFlags & FLAG_Pitched ) && ( pData->m_uiFlags & FLAG_Playing ) && pData->m_pEvent != TNULL )
			{
				pData->m_pEvent->stop();
				it->m_pData->m_pEvent = TNULL;
				it->m_pData->m_uiFlags |= FLAG_Stopped | FLAG_WantsToPlay;
				it->m_pData->m_uiFlags &= ~FLAG_Playing;
				itDesc->m_iNumInstaces--;
			}
		}
	}
}

// $deBlob: FUNCTION 006100d0
TINT APositionalSoundManager::GetNumSounds()
{
	TINT iNumSounds = 0;

	T2_FOREACH( m_oActivePosDesc, it )
	{
		iNumSounds += it->m_oPositionalList.Size();
	}

	return iNumSounds;
}

// $deBlob: FUNCTION 00610150
TINT APositionalSoundManager::GetNumFree()
{
	return m_oFreePositionals.Size();
}

// $deBlob: FUNCTION 00610160
void APositionalSoundManager::OnCellUnloadedCallback( ACellBucket* a_pBucket )
{
	APositionalSoundManager::GetSingleton()->OnCellUnloaded( a_pBucket->GetCell() );
}

// The sounds of the cell's instances: cogs, and the instances whose collision has a sound category
// $deBlob: FUNCTION 00610180
void APositionalSoundManager::OnCellLoaded( TerrainTypes::Cell* a_pCell )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	for ( TUINT i = 0; i < a_pCell->m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = &a_pCell->m_pInstances[ i ];

		if ( pInstance == TNULL || pInstance->m_pModelInstance2 == TNULL )
			continue;

		ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance() );

		TINT  iGroupID = -1;
		TINT  iEventID = -1;
		TBOOL bDone    = TFALSE;

		if ( pCollision != TNULL )
		{
			for ( TINT k = 0; !bDone; k++ )
			{
				if ( pCollision->GetNumPhysicsObjects() <= k )
					break;

				TUINT32 uiFlags = GetCollisionCategoryFlags( pCollision->GetPhysicsInfo( k ).m_pBody );

				if ( uiFlags & 0x8000 )
				{
					iGroupID = 5;
					iEventID = 0x55;
					break;
				}

				if ( uiFlags & 0x10 )
				{
					iGroupID = 8;
					iEventID = 0x9A;
					break;
				}

				if ( uiFlags & 0x10000 )
				{
					AddMatrixSound( pInstance, 5, 0x5E );
					bDone = TTRUE;
					break;
				}

				if ( uiFlags & 0x22008 )
				{
					AddMatrixSound( pInstance, 8, 0x8C );
					bDone = TTRUE;
				}
			}

			if ( bDone )
				continue;
		}

		if ( iGroupID == -1 )
		{
			if ( pInstance->m_szModel == TNULL ||
			     ( T2String8::FindString( const_cast<TCHAR*>( pInstance->m_szModel ), "cog_large" ) == TNULL &&
			       T2String8::FindString( const_cast<TCHAR*>( pInstance->m_szModel ), "cog_small" ) == TNULL ) )
				continue;

			iGroupID = 8;
			iEventID = 0x83;
		}

		// AddMatrixSound with the descriptor taken first
		if ( !TSound::GetSingleton()->IsInitialised() )
			continue;

		Positional* pPositional = PopFreePositional();

		if ( pPositional == TNULL )
			continue;

		EventDesc* pDesc = GetEventDesc( iGroupID, iEventID );

		if ( pDesc->HasTarget( pInstance, iGroupID, iEventID ) == TTRUE )
		{
			m_oFreePositionals.PushFront( pPositional );
		}
		else
		{
			pDesc->m_oPositionalList.PushFront( pPositional );
			pPositional->m_pData->m_pTarget = pInstance;
			pPositional->m_pData->m_uiFlags = FLAG_MatrixPtr | FLAG_WantsToPlay;
		}
	}
}

// $deBlob: FUNCTION 006103f0
void APositionalSoundManager::OnCellUnloaded( TerrainTypes::Cell* a_pCell )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	for ( TUINT i = 0; i < a_pCell->m_uiNumInstances; i++ )
	{
		TerrainTypes::CellInstance* pInstance = &a_pCell->m_pInstances[ i ];

		if ( pInstance->m_pModelInstance2 == TNULL )
			continue;

		ACollisionInstance2* pCollision = TSTATICCAST( ACollisionInstance2, TSTATICCAST( AModelInstance2, pInstance->m_pModelInstance2 )->GetCollisionInstance() );
		TBOOL                bDone      = TFALSE;

		if ( pCollision != TNULL )
		{
			for ( TINT k = 0; !bDone; k++ )
			{
				if ( pCollision->GetNumPhysicsObjects() <= k )
					break;

				TUINT32 uiFlags = GetCollisionCategoryFlags( pCollision->GetPhysicsInfo( k ).m_pBody );

				if ( uiFlags & 0x8000 )
				{
					RemoveSound( pInstance, 5, 0x55 );
					bDone = TTRUE;
					break;
				}

				if ( uiFlags & 0x10 )
				{
					RemoveSound( pInstance, 8, 0x9A );
					bDone = TTRUE;
					break;
				}

				if ( uiFlags & 0x10000 )
				{
					RemoveSound( pInstance, 5, 0x5E );
					bDone = TTRUE;
					break;
				}

				if ( uiFlags & 0x22008 )
				{
					RemoveSound( pInstance, 8, 0x8C );
					bDone = TTRUE;
				}
			}

			if ( bDone )
				continue;
		}

		if ( pInstance->m_szModel != TNULL &&
		     ( T2String8::FindString( const_cast<TCHAR*>( pInstance->m_szModel ), "cog_large" ) != TNULL ||
		       T2String8::FindString( const_cast<TCHAR*>( pInstance->m_szModel ), "cog_small" ) != TNULL ) )
			RemoveSound( pInstance, 8, 0x83 );
	}
}

// $deBlob: FUNCTION 00610580
void APositionalSoundManager::SetEmitterDisabled( TINT a_iIndex, TBOOL a_bDisabled )
{
	if ( m_pSoundTRBData == TNULL || a_iIndex >= m_pSoundTRBData->m_iNumEmitters )
		return;

	Positional* pEmitter = &m_pEmitters[ a_iIndex ];

	if ( a_bDisabled )
	{
		EventDesc* pDesc = GetEventDesc( pEmitter->m_pData->m_iGroupID, pEmitter->m_pData->m_iEventID );

		if ( pEmitter->m_pData->m_uiFlags & FLAG_Playing )
			pDesc->StopInstance( pEmitter );
	}

	SoundData* pData = pEmitter->m_pData;

	if ( pData != TNULL )
	{
		if ( a_bDisabled )
			pData->m_uiFlags |= FLAG_Disabled;
		else
			pData->m_uiFlags &= ~FLAG_Disabled;
	}
}

// $deBlob: FUNCTION 00610600
void APositionalSoundManager::SetPitch( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID, TFLOAT a_fPitch )
{
	EventDesc* pDesc = GetEventDesc( TINT16( a_iGroupID ), TINT16( a_iEventID ) );

	if ( pDesc != TNULL )
		pDesc->SetPitch( a_pTarget, a_fPitch );
}

// $deBlob: FUNCTION 00610630
void APositionalSoundManager::SetPaused( TBOOL a_bPaused )
{
	if ( a_bPaused )
		m_uiFlags |= 1;
	else
		m_uiFlags &= ~1;

	T2_FOREACH( m_oActivePosDesc, itDesc )
	{
		T2_FOREACH( itDesc->m_oPositionalList, it )
		{
			if ( it->m_pData->m_pEvent != TNULL )
				it->m_pData->m_pEvent->setPaused( a_bPaused );
		}
	}
}

// Stops the emitters flagged 0x8000 and disables them, enables the others
// $deBlob: FUNCTION 00610760
void APositionalSoundManager::ResetEmitters()
{
	if ( m_pEmitters == TNULL || m_pSoundTRBData == TNULL )
		return;

	for ( TINT i = 0; i < m_pSoundTRBData->m_iNumEmitters; i++ )
	{
		Positional* pEmitter = &m_pEmitters[ i ];
		SoundData*  pData    = pEmitter->m_pData;

		if ( !( pData->m_uiFlags & FLAG_Emitter8000 ) )
		{
			if ( m_pSoundTRBData != TNULL && pData != TNULL )
				pData->m_uiFlags &= ~FLAG_Disabled;
		}
		else if ( m_pSoundTRBData != TNULL )
		{
			EventDesc* pDesc = GetEventDesc( pData->m_iGroupID, pData->m_iEventID );

			if ( pEmitter->m_pData->m_uiFlags & FLAG_Playing )
				pDesc->StopInstance( pEmitter );

			if ( pEmitter->m_pData != TNULL )
				pEmitter->m_pData->m_uiFlags |= FLAG_Disabled;
		}
	}
}

// $deBlob: FUNCTION 00610820
void APositionalSoundManager::RestartSounds( TUINT16 a_uiFlags )
{
	T2_FOREACH( m_oActivePosDesc, it )
	{
		it->RestartSounds( a_uiFlags );
	}
}

// $deBlob: FUNCTION 006108a0
void APositionalSoundManager::CheckNoPositionals()
{
	T2_FOREACH( m_oActivePosDesc, it )
	{
		TASSERT( 0 == it->m_oPositionalList.Size() );
	}
}

// $deBlob: FUNCTION 00610990
TBOOL APositionalSoundManager::AllocPositional( Positional*& a_rpPositional, TINT a_iGroupID, TINT a_iEventID, void* a_pTarget )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TFALSE;

	TASSERT( a_iGroupID >= 0 );
	TASSERT( a_iEventID >= 0 );

	Positional* pPositional = PopFreePositional();

	if ( pPositional == TNULL )
		return TFALSE;

	EventDesc* pDesc = GetEventDesc( a_iGroupID, a_iEventID );

	if ( a_pTarget == TNULL || pDesc->HasTarget( a_pTarget, a_iGroupID, a_iEventID ) != TTRUE )
	{
		pDesc->m_oPositionalList.PushFront( pPositional );
		a_rpPositional = pPositional;
		return TTRUE;
	}

	m_oFreePositionals.PushFront( pPositional );
	return TFALSE;
}
