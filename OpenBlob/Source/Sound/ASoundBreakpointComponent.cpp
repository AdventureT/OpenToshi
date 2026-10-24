#include "pch.h"
#include "ASoundBreakpointComponent.h"
#include "ACurrentLevelInformation.h"
#include "Animation/AModel2.h"
#include "Animation/AModelInstance2.h"
#include "Cameras/ACameraManager.h"
#include "Sound/ASoundInterface.h"
#include "Sound/TSound.h"

#include <Toshi/TString8.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>
#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>
#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00573110
ASoundBreakpointComponent::ASoundBreakpointComponent( AModelInstance2* a_pModelInstance )
{
	m_pPosition     = TNULL;
	m_uiSurfaceType = 1;
	m_uiFlags       = 0;
	m_pTable        = TNULL;
	m_pEntry        = TNULL;

	InitSounds();

	AModel2* pModel = a_pModelInstance->GetModel();

	TASSERT( pModel->GetFileName().IsIndexValid( 0 ) );
	m_szName = pModel->GetFileName();

	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( 4 );

	m_pTable = TSTATICCAST( BreakpointTable, pTRB->GetSymbolAddress( "Main" ) );
	m_pEntry = FindEntry( m_pTable, m_szName );

	m_pModelInstance = a_pModelInstance->GetModelInstance();
}

// $deBlob: FUNCTION 005731d0
ASoundBreakpointComponent::ASoundBreakpointComponent( TModelInstance* a_pModelInstance, const TCHAR* a_szName )
{
	m_pPosition     = TNULL;
	m_uiSurfaceType = 1;
	m_uiFlags       = 0;
	m_pTable        = TNULL;
	m_pEntry        = TNULL;

	InitSounds();

	m_szName         = a_szName;
	m_pModelInstance = a_pModelInstance;

	TTRB* pTRB = ACurrentLevelInformation::GetSingleton()->Stream( 4 );

	m_pTable = TSTATICCAST( BreakpointTable, pTRB->GetSymbolAddress( "Main" ) );
	m_pEntry = FindEntry( m_pTable, m_szName );
}

// $deBlob: FUNCTION 00574030
ASoundBreakpointComponent::~ASoundBreakpointComponent()
{
	if ( m_pEntry != TNULL )
		StopAndClearSounds();
}

// $deBlob: FUNCTION 00573260
void ASoundBreakpointComponent::PatchAnimIDs( TModelInstance* a_pModelInstance )
{
	if ( a_pModelInstance != TNULL )
		m_pModelInstance = a_pModelInstance;

	if ( m_pEntry != TNULL && m_pEntry->m_iNumSubEntries > 0 )
	{
		for ( TINT i = 0; i < m_pEntry->m_iNumSubEntries; i++ )
		{
			BreakpointEntry::SubEntry* pSub = &m_pEntry->m_pSubEntries[ i ];

			// The name is NULL when the index is out of the table's names
			const TCHAR* szName = GetName( pSub->m_iNameIndex );

			TSkeleton* pSkeleton   = m_pModelInstance->GetSkeletonInstance()->GetSkeleton();
			TINT       iSequenceID = pSkeleton->GetSequenceID( szName, -1 );

			pSub->m_iSequenceID = TINT16( iSequenceID );
		}
	}
}

// $deBlob: FUNCTION 00573500
void ASoundBreakpointComponent::SetMuted( TBOOL a_bMuted )
{
	if ( a_bMuted )
	{
		StopAndClearSounds();
		m_uiFlags |= FLAGS_MUTED;
	}
	else
	{
		m_uiFlags &= TUINT16( ~FLAGS_MUTED );
	}
}

// $deBlob: FUNCTION 005733c0
void ASoundBreakpointComponent::StopAndClearSounds()
{
	// NOTE: the T2Vector iterator asserts of retail (m_iIndex >= 0, < CAPACITY) are not repeated: the loop indexes
	// the array directly
	for ( TINT i = 0; i < m_iNumSounds; i++ )
	{
		if ( m_aSounds[ i ].m_pEvent != TNULL )
			m_aSounds[ i ].m_pEvent->stop( true );
	}

	ClearSounds();
}

// $deBlob: FUNCTION 00574090
void ASoundBreakpointComponent::ClearSounds()
{
	for ( TINT i = 0; i < m_iNumSounds; i++ )
		m_aSounds[ i ] = BreakpointSound();

	m_iNumSounds = 0;
}

// $deBlob: FUNCTION 00573d80
void ASoundBreakpointComponent::InitSounds()
{
	m_iNumSounds = 0;

	for ( TINT i = 0; i < TARRAYSIZE( m_aSounds ); i++ )
		m_aSounds[ i ] = BreakpointSound();
}

// $deBlob: FUNCTION 00573310
ASoundBreakpointComponent::BreakpointEntry* ASoundBreakpointComponent::FindEntry( const BreakpointTable* a_pTable, const TCHAR* a_szName )
{
	TSIZE uiNameLength = T2String8::Length( a_szName );

	if ( uiNameLength > 0 )
	{
		for ( TINT i = 0; i < a_pTable->m_iNumEntries; i++ )
		{
			BreakpointEntry* pEntry      = &a_pTable->m_pEntries[ i ];
			const TCHAR*     szEntryName = a_pTable->m_ppNames[ pEntry->m_iNameIndex ];

			if ( T2String8::Length( szEntryName ) <= TINT( uiNameLength + 4 ) )
			{
				if ( TStringManager::String8CompareNoCase( szEntryName, a_szName, uiNameLength ) == 0 )
					return pEntry;
			}
		}
	}

	return TNULL;
}

// $deBlob: FUNCTION 00573580
const TCHAR* ASoundBreakpointComponent::GetName( TINT16 a_iIndex ) const
{
	BreakpointTable* pTable = m_pTable;

	if ( a_iIndex >= 0 && a_iIndex < pTable->m_iNumNames )
		return pTable->m_ppNames[ a_iIndex ];

	return TNULL;
}

// $deBlob: FUNCTION 00573530
Toshi::TVector4* ASoundBreakpointComponent::GetPosition()
{
	if ( m_pPosition != TNULL )
		return TREINTERPRETCAST( Toshi::TVector4*, m_pPosition );

	// The row 3 of the instance's matrix
	return TREINTERPRETCAST( Toshi::TVector4*, &m_pModelInstance->GetTransform().GetMatrix().m_f41 );
}

// $deBlob: FUNCTION 00573d30
TBOOL ASoundBreakpointComponent::CheckSound( TINT16 a_iGroupName, TINT16 a_iEventName ) const
{
	for ( TINT i = 0; i < m_iNumSounds; i++ )
	{
		const BreakpointSound& rSound = m_aSounds[ i ];

		if ( rSound.m_pEvent != TNULL && rSound.m_iGroupName == a_iGroupName && rSound.m_iEventName == a_iEventName )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 00573c70
void ASoundBreakpointComponent::UpdateTrackingSounds( TINT a_iEntryIndex, TINT a_iRecordIndex )
{
	for ( TINT i = 0; i < m_iNumSounds; i++ )
	{
		BreakpointSound& rSound = m_aSounds[ i ];

		if ( rSound.m_iEntryIndex == a_iEntryIndex && rSound.m_iRecordIndex == a_iRecordIndex && rSound.m_pEvent != TNULL )
		{
			if ( ( m_pEntry->m_pSubEntries[ a_iEntryIndex ].m_pRecords[ a_iRecordIndex ].m_uiFlags & 0x10 ) != 0 )
				ASoundInterface::SetPosition( rSound.m_pEvent, *GetPosition() );
		}
	}
}

// $deBlob: FUNCTION 00573ce0
void ASoundBreakpointComponent::UpdateTrackingSounds( TINT a_iEntryIndex, TINT a_iRecordIndex, const BreakpointSound* a_pSound )
{
	if ( a_pSound->m_pEvent != TNULL && ( m_pEntry->m_pSubEntries[ a_iEntryIndex ].m_pRecords[ a_iRecordIndex ].m_uiFlags & 0x10 ) != 0 )
		ASoundInterface::SetPosition( a_pSound->m_pEvent, *GetPosition() );
}

// NOTE: the T2Vector erase/push of retail are plain array moves here; the tail slot is reset
void ASoundBreakpointComponent::EraseSound( TINT a_iIndex )
{
	for ( TINT i = a_iIndex; i < m_iNumSounds - 1; i++ )
		m_aSounds[ i ] = m_aSounds[ i + 1 ];

	m_iNumSounds--;
	m_aSounds[ m_iNumSounds ] = BreakpointSound();
}

void ASoundBreakpointComponent::PushBackSound( const BreakpointSound& a_rSound )
{
	m_aSounds[ m_iNumSounds ] = a_rSound;
	m_iNumSounds++;
}

// $deBlob: FUNCTION 005735b0
// the T2Vector iterator asserts and the TTransformObject assert latch of retail are not repeated.
void ASoundBreakpointComponent::TickImp()
{
	// NOTE: retail reads the entry without a check; the callers only tick components that have one
	if ( m_pEntry == TNULL )
		return;

	if ( !TSound::GetSingleton()->IsInitialised() || ( m_uiFlags & FLAGS_MUTED ) != 0 )
		return;

	TFLOAT fDeltaTime = TSystemManager::GetSingleton()->GetScheduler()->GetCurrentDeltaTime();

	for ( TINT i = 0; i < m_pEntry->m_iNumSubEntries; i++ )
	{
		BreakpointEntry::SubEntry* pSub = &m_pEntry->m_pSubEntries[ i ];

		if ( pSub->m_iSequenceID < 0 )
			continue;

		TAnimation* pAnim = m_pModelInstance->GetSkeletonInstance()->IsAnimating( pSub->m_iSequenceID );

		if ( pAnim == TNULL )
		{
			// The sequence isn't running: for each record, the sounds that move with the instance are repositioned,
			// the ones with the flags 0x0A are stopped and removed
			for ( TINT j = 0; j < pSub->m_iNumRecords; j++ )
			{
				TINT iIndex = 0;

				while ( iIndex < m_iNumSounds )
				{
					BreakpointSound& rSound = m_aSounds[ iIndex ];

					if ( rSound.m_iEntryIndex != i || rSound.m_iRecordIndex != j )
					{
						iIndex++;
						continue;
					}

					if ( rSound.m_pEvent != TNULL && ( pSub->m_pRecords[ j ].m_uiFlags & 0x10 ) != 0 )
						ASoundInterface::SetPosition( rSound.m_pEvent, *GetPosition() );

					if ( rSound.m_pEvent != TNULL && ( pSub->m_pRecords[ j ].m_uiFlags & 0x0A ) != 0 )
					{
						rSound.m_pEvent->stop( false );
						EraseSound( iIndex ); // the next sound takes this index
					}
					else
					{
						iIndex++;
					}
				}
			}
		}
		else
		{
			for ( TINT j = 0; j < pSub->m_iNumRecords; j++ )
			{
				BreakpointRecord& rRecord = pSub->m_pRecords[ j ];

				if ( rRecord.m_iGroupName == -1 || rRecord.m_iEventName == -1 )
					continue;

				UpdateTrackingSounds( i, j );

				TSkeletonSequence* pSequence = pAnim->GetSequencePtr();

				if ( ( rRecord.m_uiFlags & 1 ) != 0 || pAnim->GetTotalTime() <= pSequence->GetDuration() )
				{
					TFLOAT fTime = rRecord.m_fNormTime * pSequence->GetDuration();

					if ( pAnim->GetSeqTime() <= fTime && fTime <= pAnim->GetSpeed() * fDeltaTime + pAnim->GetSeqTime() )
					{
						// Flags 0x1A: the sound is tracked, it is only played when the record's sound isn't playing yet
						TBOOL bTrack = ( rRecord.m_uiFlags & 0x1A ) != 0;

						if ( !bTrack || ( m_iNumSounds < 5 && CheckSound( rRecord.m_iGroupName, rRecord.m_iEventName ) ) )
						{
							Toshi::TVector4* pPosition = GetPosition();

							if ( ( rRecord.m_uiFlags & 0x20 ) == 0 )
							{
								ACamera*     pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
								const TFLOAT fDistSq = Toshi::TVector4::DistanceSq( *pPosition, *TREINTERPRETCAST( const Toshi::TVector4*, &pCamera->GetMatrix().m_f41 ) );

								if ( rRecord.m_fMaxDistSq <= fDistSq && fDistSq != rRecord.m_fMaxDistSq )
									continue;
							}

							const TCHAR* szGroup = GetName( rRecord.m_iGroupName );
							const TCHAR* szEvent = GetName( rRecord.m_iEventName );
							FMOD::Event* pEvent  = ASoundInterface::PlayEvent( szGroup, szEvent, TNULL );

							if ( ( rRecord.m_uiFlags & 0x20 ) == 0 )
								ASoundInterface::SetPosition( pEvent, *pPosition );

							if ( bTrack )
								PushBackSound( BreakpointSound( j, i, pEvent, rRecord.m_iGroupName, rRecord.m_iEventName ) );
						}
					}
				}
			}
		}
	}

	// Removes the sounds that are paused (a failed query also removes them) or no longer playing
	TINT iIndex = 0;

	while ( iIndex < m_iNumSounds )
	{
		BreakpointSound& rSound  = m_aSounds[ iIndex ];
		bool             bPaused = false;

		if ( rSound.m_pEvent->getPaused( &bPaused ) != FMOD_OK )
		{
			EraseSound( iIndex );
			continue;
		}

		if ( !bPaused && !ASoundInterface::IsEventInState( rSound.m_pEvent, FMOD_EVENT_STATE_PLAYING ) )
		{
			EraseSound( iIndex );
			continue;
		}

		iIndex++;
	}
}
