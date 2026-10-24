#include "pch.h"
#include "ASoundTextureSwapMap.h"
#include "ASoundInterface.h"
#include "ASoundLoadHelper.h"

#include <Render/TTexture.h>
#include <Sound/TSound.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00613f30
ASoundTextureSwapMap::ASoundTextureSwapMap()
{
	if ( m_oTRB.Load( "Data/Sound/TextureSwapSoundData/SoundTextureSwap.trb" ) != TTRB::ERROR_OK )
	{
		m_pData = TNULL;
		m_oTRB.Close();
	}
	else
	{
		m_pData = TSTATICCAST( Data, m_oTRB.GetSymbolAddress( "SoundTextureSwapMap" ) );
	}
}

// $deBlob: FUNCTION 00614030
// $deBlob: FUNCTION 0056c250
ASoundTextureSwapMap::~ASoundTextureSwapMap()
{
	m_pData = TNULL;
	m_oTRB.Close();
}

// $deBlob: FUNCTION 006140d0
void ASoundTextureSwapMap::PlayTextureSounds( TTexture* a_pTexture, const TVector4& a_rcPosition )
{
	if ( m_pData == TNULL || a_pTexture == TNULL )
		return;

	for ( TINT16 i = 0; i < m_pData->m_iNumEntries; i++ )
	{
		Entry* pEntry = &m_pData->m_pEntries[ i ];

		if ( pEntry->m_pTexture == a_pTexture )
			PlayEntrySound( pEntry, a_rcPosition );
	}
}

// $deBlob: FUNCTION 00614120
void ASoundTextureSwapMap::PlayFirstTextureSound( TTexture* a_pTexture, const TVector4& a_rcPosition )
{
	if ( m_pData == TNULL )
		return;

	TINT i = 0;

	if ( m_pData->m_iNumEntries <= 0 )
		return;

	while ( m_pData->m_pEntries[ i ].m_pTexture != a_pTexture )
	{
		if ( ++i >= m_pData->m_iNumEntries )
			return;
	}

	// NOTE: the same code as PlayEntrySound
	Entry* pEntry = &m_pData->m_pEntries[ i ];

	if ( TSound::GetSingleton()->IsInitialised() )
	{
		const TCHAR*      szEvent = m_pData->m_ppStrings[ pEntry->m_iEvent ];
		FMOD::EventGroup* pGroup  = ASoundInterface::GetEventGroup( m_pData->m_ppStrings[ pEntry->m_iGroup ], 0 );
		FMOD::Event*      pEvent  = TNULL;

		if ( pGroup->getEvent( szEvent, FMOD_EVENT_DEFAULT, &pEvent ) == FMOD_OK )
		{
			FMOD_EVENT_INFO oInfo;
			pEvent->getInfo( TNULL, TNULL, &oInfo );

			if ( oInfo.lengthms != -1 )
			{
				ASoundInterface::SetPosition( pEvent, a_rcPosition );
				pEvent->start();
			}
		}
	}
}

// $deBlob: FUNCTION 006141e0
void ASoundTextureSwapMap::AddSoundGroupsToLoad()
{
	if ( m_pData == TNULL || ASoundLoadHelper::GetSingleton() == TNULL )
		return;

	for ( TINT16 i = 0; i < m_pData->m_iNumGroups; i++ )
		ASoundLoadHelper::GetSingleton()->AddSoundGroupToLoad( m_pData->m_ppStrings[ m_pData->m_pGroups[ i ] ], TNULL );
}

// $deBlob: FUNCTION 00614230
void ASoundTextureSwapMap::ResolveTextures()
{
	if ( m_pData == TNULL )
		return;

	TTextureManager* pTextureManager = TTextureManager::GetSingleton();

	for ( TINT16 i = 0; i < m_pData->m_iNumEntries; i++ )
	{
		Entry* pEntry      = &m_pData->m_pEntries[ i ];
		pEntry->m_pTexture = pTextureManager->FindTexture( m_pData->m_ppStrings[ pEntry->m_iTexture ] );
	}
}

// $deBlob: FUNCTION 00614290
void ASoundTextureSwapMap::PlayEntrySound( Entry* a_pEntry, const TVector4& a_rcPosition )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return;

	const TCHAR*      szEvent = m_pData->m_ppStrings[ a_pEntry->m_iEvent ];
	FMOD::EventGroup* pGroup  = ASoundInterface::GetEventGroup( m_pData->m_ppStrings[ a_pEntry->m_iGroup ], 0 );
	FMOD::Event*      pEvent  = TNULL;

	if ( pGroup->getEvent( szEvent, FMOD_EVENT_DEFAULT, &pEvent ) == FMOD_OK )
	{
		FMOD_EVENT_INFO oInfo;
		pEvent->getInfo( TNULL, TNULL, &oInfo );

		// Events of unknown length (looping) aren't started: nothing would stop them
		if ( oInfo.lengthms != -1 )
		{
			ASoundInterface::SetPosition( pEvent, a_rcPosition );
			pEvent->start();
		}
	}
}
