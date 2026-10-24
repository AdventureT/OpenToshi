#include "pch.h"
#include "EnSaveData.h"
#include "Cameras/ABlobFollowCamera.h"
#include "GameController/AGameStatus.h"
#include "XUI/InGame/ABlobRewardScreen.h"

#include <File/TFile.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( EnSaveData::SlotData ) == 0x314 );
TSTATICASSERT( sizeof( EnSaveData::SaveData ) == EnSaveData::SAVE_SIZE );

// Only read by ResetExtra and the options' setters, never set
static TINT s_iUnk0099698c = 0;

// $deBlob: FUNCTION 005e3910
EnSaveData::EnSaveData()
{
	m_uiBufferSize = SAVE_SIZE;
	m_uiSize       = SAVE_SIZE;
	m_pBuffer      = TSTATICCAST( TUINT8, TMemalign( 4, SAVE_SIZE ) );
	TUtil::MemSet( m_pBuffer, 0, m_uiSize );

	m_pSaveData = TREINTERPRETCAST( SaveData*, m_pBuffer );
	TUtil::MemCopy( m_pSaveData, "BB", 2 );
	m_pSaveData->m_oHeader.m_uiSize = SAVE_SIZE;

	ResetExtra();

	TASSERT( TFALSE == m_pSaveData->m_aSaveGameSlots[ 0 ].m_bUsedSlot );
	TASSERT( TFALSE == m_pSaveData->m_aSaveGameSlots[ 1 ].m_bUsedSlot );
	TASSERT( TFALSE == m_pSaveData->m_aSaveGameSlots[ 2 ].m_bUsedSlot );
}

// $deBlob: FUNCTION 005e3a10
// $deBlob: FUNCTION 0056c2b0
EnSaveData::~EnSaveData()
{
	TFree( m_pBuffer );
}

// $deBlob: FUNCTION 005e3a20
void EnSaveData::SetHeader()
{
	TUtil::MemSet( &m_pSaveData->m_oHeader, 0, sizeof( Header ) );
	TUtil::MemCopy( m_pSaveData->m_oHeader.m_szID, "BB", 2 );
	m_pSaveData->m_oHeader.m_uiSize = SAVE_SIZE;
	m_pSaveData->m_oHeader.m_uiCRC  = 0;
}

// $deBlob: FUNCTION 005e3a60
void EnSaveData::SetCRC()
{
	TUINT32 uiCRC                  = CalculateCRC32();
	m_pSaveData->m_oHeader.m_uiCRC = uiCRC;
}

// $deBlob: FUNCTION 005e3a90
void EnSaveData::Copy( EnSaveData* a_pSaveData )
{
	TUtil::MemCopy( m_pBuffer, a_pSaveData->m_pBuffer, m_uiBufferSize );
}

// $deBlob: FUNCTION 005e3ab0
void EnSaveData::Reset()
{
	TUtil::MemSet( m_pBuffer, 0, m_uiSize );
	SetHeader();
	ResetExtra();
	SetCRC();
}

// $deBlob: FUNCTION 005e3b20
TBOOL EnSaveData::Validate()
{
	if ( TStringManager::String8CompareNoCase( m_pSaveData->m_oHeader.m_szID, "BB", 2 ) != 0 )
		return TFALSE;

	if ( m_pSaveData->m_oHeader.m_uiSize != SAVE_SIZE )
		return TFALSE;

	if ( m_pSaveData->m_oHeader.m_uiCRC != CalculateCRC32() )
		return TFALSE;

	SaveExtra& rExtra     = m_pSaveData->m_oExtra;
	TFLOAT     fCameraMin = ABlobFollowCamera::GetMINCamSpeedOption();
	TFLOAT     fCameraMax = ABlobFollowCamera::GetMAXCamSpeedOption();
	rExtra.m_fCameraSpeed = TMath::Max( TMath::Min( rExtra.m_fCameraSpeed, fCameraMax ), fCameraMin );

	for ( TINT i = 0; i < SAVEGAME_SLOT_COUNT; i++ )
	{
		SaveGameSlot& rSlot = m_pSaveData->m_aSaveGameSlots[ i ];

		if ( rSlot.m_bUsedSlot && !rSlot.m_oData.Validate() )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005e3bf0
void EnSaveData::ResetExtra()
{
	SaveExtra& rExtra = m_pSaveData->m_oExtra;

	rExtra.m_iUnk00       = 2;
	rExtra.m_iSFXVolume   = 8;
	rExtra.m_iMusicVolume = 8;
	rExtra.m_uiFlags      = 0;
	rExtra.m_uiFlags &= ~SaveExtra::FLAG_InvertCamera;
	rExtra.m_uiFlags |= 0x10;

	rExtra.m_fCameraSpeed = ABlobFollowCamera::GetMINCamSpeedOption();

	if ( s_iUnk0099698c == 0 )
	{
		TFLOAT fCameraMin     = ABlobFollowCamera::GetMINCamSpeedOption();
		TFLOAT fCameraMax     = ABlobFollowCamera::GetMAXCamSpeedOption();
		rExtra.m_fCameraSpeed = TMath::Max( TMath::Min( rExtra.m_fCameraSpeed, fCameraMax ), fCameraMin );
	}
}

// $deBlob: FUNCTION 005e36d0
EnSaveData::SaveExtra::SaveExtra()
{
	m_uiFlags = 0;
	Reset();
}

// $deBlob: FUNCTION 005e3750
void EnSaveData::SaveExtra::Reset()
{
	m_iUnk00       = 2;
	m_iSFXVolume   = 8;
	m_iMusicVolume = 8;
	m_uiFlags      = 0;
	m_uiFlags &= ~FLAG_InvertCamera;
	m_uiFlags |= 0x10;

	m_fCameraSpeed = ABlobFollowCamera::GetMINCamSpeedOption();

	if ( s_iUnk0099698c == 0 )
	{
		TFLOAT fCameraMin = ABlobFollowCamera::GetMINCamSpeedOption();
		TFLOAT fCameraMax = ABlobFollowCamera::GetMAXCamSpeedOption();
		m_fCameraSpeed    = TMath::Max( TMath::Min( fCameraMax, m_fCameraSpeed ), fCameraMin );
	}
}

// $deBlob: FUNCTION 005e37d0
TBOOL EnSaveData::SaveExtra::Validate()
{
	TFLOAT fCameraMin = ABlobFollowCamera::GetMINCamSpeedOption();
	TFLOAT fCameraMax = ABlobFollowCamera::GetMAXCamSpeedOption();
	m_fCameraSpeed    = TMath::Max( TMath::Min( fCameraMax, m_fCameraSpeed ), fCameraMin );
	return TTRUE;
}

// $deBlob: FUNCTION 005e3880
void EnSaveData::SaveData::ResetExtra()
{
	m_oExtra.Reset();
}

// $deBlob: FUNCTION 005e3c70
TUINT32 EnSaveData::CalculateCRC32()
{
	// The CRC is computed with the CRC field counted as 0
	TUINT32 uiCRC                  = m_pSaveData->m_oHeader.m_uiCRC;
	m_pSaveData->m_oHeader.m_uiCRC = 0;
	TUINT32 uiResult               = TUtil::CRC32( m_pBuffer, m_uiBufferSize );
	m_pSaveData->m_oHeader.m_uiCRC = uiCRC;

	return uiResult;
}

// $deBlob: FUNCTION 005e3ca0
void EnSaveData::GetRawDataFromFile( const TCHAR* a_szFileName, TUINT8** a_ppData, TUINT32* a_pSize )
{
	TFile* pFile = TFile::Create( a_szFileName, TFILEMODE_READ );
	TASSERT( TNULL != pFile );

	*a_pSize  = TUINT32( pFile->GetSize() );
	*a_ppData = TSTATICCAST( TUINT8, TMemalign( 0x20, *a_pSize ) );
	pFile->Read( *a_ppData, *a_pSize );
	pFile->Destroy();
}

// $deBlob: FUNCTION 005e3660
EnSaveData::SlotData::SlotData()
{
	Reset();
	Reset();
}

// $deBlob: FUNCTION 005e2ae0
// $deBlob: FUNCTION 005e3680
void EnSaveData::SlotData::Reset()
{
	m_uiUnlockedGalleryItems = 0;
	m_iUnk2F0                = 0;
	m_iUnlockedTracks        = 0;
	m_iUnlockedWorlds        = 1;
	m_iUnlockedBlobParty     = 27;
	m_iUnlockedChallenges    = 0;
	m_iUnlockedVideos        = 0;
	m_iUnlockedMovies        = 1;

	for ( TINT i = 0; i < 10; i++ )
	{
		m_auiUnlockedModes[ i ]          = 1;
		m_auiLevelFlags[ i ]             = 0;
		m_auiMedals[ i ]                 = 0;
		m_auiBestPoints[ i ]             = 0;
		m_aMedalTimes[ i ].m_fBronzeTime = 0;
		m_aMedalTimes[ i ].m_fSilverTime = 0;
		m_aMedalTimes[ i ].m_fGoldTime   = 0;
	}

	for ( TINT i = 0; i < 20; i++ )
	{
		m_auiSprintMedals[ i ]                 = 0;
		m_aSprintMedalTimes[ i ].m_fBronzeTime = 0;
		m_aSprintMedalTimes[ i ].m_fSilverTime = 0;
		m_aSprintMedalTimes[ i ].m_fGoldTime   = 0;
		m_auiSprintTotal[ i ]                  = 0;
		m_auiSprintBestCount[ i ]              = 0;
	}

	m_uiNumAwards              = 0;
	m_uiNewFlags               = 0;
	m_uiNewFlagsUnseen         = 0;
	m_uiNewFlagsExtended       = 0;
	m_uiNewFlagsExtendedUnseen = 0;
	m_auiUnk308[ 0 ]           = 0;
	m_auiUnk308[ 1 ]           = 0;
	m_auiUnk308[ 2 ]           = 0;
}

// $deBlob: FUNCTION 005e35c0
TFLOAT EnSaveData::SlotData::GetMedalTime( TINT a_iWorld ) const
{
	TUINT32 uiMedals = m_auiMedals[ a_iWorld ];

	if ( uiMedals & AWARD_Gold )
		return m_aMedalTimes[ a_iWorld ].m_fGoldTime;

	if ( uiMedals & AWARD_Silver )
		return m_aMedalTimes[ a_iWorld ].m_fSilverTime;

	if ( uiMedals & AWARD_Bronze )
		return m_aMedalTimes[ a_iWorld ].m_fBronzeTime;

	return 0.0f;
}

// $deBlob: FUNCTION 005e3610
TFLOAT EnSaveData::SlotData::GetSprintMedalTime( TINT a_iSlot ) const
{
	TUINT32 uiMedals = m_auiSprintMedals[ a_iSlot ];

	if ( uiMedals & AWARD_Gold )
		return m_aSprintMedalTimes[ a_iSlot ].m_fGoldTime;

	if ( uiMedals & AWARD_Silver )
		return m_aSprintMedalTimes[ a_iSlot ].m_fSilverTime;

	if ( uiMedals & AWARD_Bronze )
		return m_aSprintMedalTimes[ a_iSlot ].m_fBronzeTime;

	return 0.0f;
}

// $deBlob: FUNCTION 005e31c0
void EnSaveData::SlotData::AddMedals( TINT a_iWorld, TUINT32 a_uiMedals, ABlobRewardScreen* a_pRewardScreen )
{
	// Only the best medal (gold 4, silver 2, bronze 1) is kept
	m_auiMedals[ a_iWorld ] |= a_uiMedals;
	TUINT32 uiMedals = m_auiMedals[ a_iWorld ];

	if ( uiMedals & AWARD_Gold )
		m_auiMedals[ a_iWorld ] = uiMedals & ~( AWARD_Bronze | AWARD_Silver );
	else if ( uiMedals & AWARD_Silver )
		m_auiMedals[ a_iWorld ] = uiMedals & ~( AWARD_Bronze | AWARD_Gold );
	else if ( uiMedals & AWARD_Bronze )
		m_auiMedals[ a_iWorld ] = uiMedals & ~( AWARD_Silver | AWARD_Gold );

	// And only the best paint award (Paint100 0x80, Paint75 0x40, Paint50 0x20, Paint25 0x10)
	uiMedals = m_auiMedals[ a_iWorld ];

	if ( uiMedals & AWARD_Paint100 )
		m_auiMedals[ a_iWorld ] = uiMedals & ~( AWARD_Paint25 | AWARD_Paint50 | AWARD_Paint75 );
	else if ( uiMedals & AWARD_Paint75 )
		m_auiMedals[ a_iWorld ] = uiMedals & ~( AWARD_Paint25 | AWARD_Paint50 | AWARD_Paint100 );
	else if ( uiMedals & AWARD_Paint50 )
		m_auiMedals[ a_iWorld ] = uiMedals & ~( AWARD_Paint25 | AWARD_Paint75 | AWARD_Paint100 );
	else if ( uiMedals & AWARD_Paint25 )
		m_auiMedals[ a_iWorld ] = uiMedals & ~( AWARD_Paint50 | AWARD_Paint75 | AWARD_Paint100 );

	CountAwards();
}

// $deBlob: FUNCTION 005e3290
void EnSaveData::SlotData::RecordBest( TINT a_iWorld, TUINT8 a_uiMode, TFLOAT a_fTime, TUINT32 a_uiPoints, TINT a_iMedal )
{
	m_auiLevelFlags[ a_iWorld ] |= TUINT8( 1 << a_uiMode );

	if ( m_auiBestPoints[ a_iWorld ] < a_uiPoints )
		m_auiBestPoints[ a_iWorld ] = a_uiPoints;

	// The time of a medal is kept only while no better medal was won
	if ( a_iMedal == 0 )
	{
		if ( m_auiMedals[ a_iWorld ] & 4 )
			return;

		if ( m_auiMedals[ a_iWorld ] & 2 )
			return;
	}
	else if ( a_iMedal == 1 && ( m_auiSprintMedals[ a_iWorld ] & 4 ) )
	{
		// NOTE: the retail tests m_auiSprintMedals (the sprint medals) here, not m_auiMedals
		return;
	}

	if ( a_iMedal == 0 )
	{
		TFLOAT fTime = m_aMedalTimes[ a_iWorld ].m_fBronzeTime;

		if ( fTime == 0.0f || a_fTime < fTime )
			m_aMedalTimes[ a_iWorld ].m_fBronzeTime = a_fTime;
	}
	else if ( a_iMedal == 1 )
	{
		TFLOAT fTime = m_aMedalTimes[ a_iWorld ].m_fSilverTime;

		if ( fTime == 0.0f || a_fTime < fTime )
			m_aMedalTimes[ a_iWorld ].m_fSilverTime = a_fTime;
	}
	else if ( a_iMedal == 2 )
	{
		TFLOAT fTime = m_aMedalTimes[ a_iWorld ].m_fGoldTime;

		if ( fTime == 0.0f || a_fTime < fTime )
			m_aMedalTimes[ a_iWorld ].m_fGoldTime = a_fTime;
	}
}

// $deBlob: FUNCTION 005e3250
void EnSaveData::SlotData::AddSprintMedals( TINT a_iSlot, TUINT32 a_uiMedals )
{
	m_auiSprintMedals[ a_iSlot ] |= a_uiMedals;
	TUINT32 uiMedals = m_auiSprintMedals[ a_iSlot ];

	// Only the best medal is kept
	if ( uiMedals & AWARD_Gold )
		m_auiSprintMedals[ a_iSlot ] = uiMedals & ~( AWARD_Bronze | AWARD_Silver );
	else if ( uiMedals & AWARD_Silver )
		m_auiSprintMedals[ a_iSlot ] = uiMedals & ~( AWARD_Bronze | AWARD_Gold );
	else if ( uiMedals & AWARD_Bronze )
		m_auiSprintMedals[ a_iSlot ] = uiMedals & ~( AWARD_Silver | AWARD_Gold );
}

// $deBlob: FUNCTION 005e3390
void EnSaveData::SlotData::RecordSprintMedalTime( TINT a_iSlot, TFLOAT a_fTime, TINT a_iMedal )
{
	// The time of a medal is kept only while no better medal was won
	if ( a_iMedal == 0 )
	{
		if ( m_auiSprintMedals[ a_iSlot ] & 4 )
			return;

		if ( m_auiSprintMedals[ a_iSlot ] & 2 )
			return;
	}
	else if ( a_iMedal == 1 && ( m_auiSprintMedals[ a_iSlot ] & 4 ) )
	{
		return;
	}

	if ( a_iMedal == 0 )
	{
		TFLOAT fTime = m_aSprintMedalTimes[ a_iSlot ].m_fBronzeTime;

		if ( fTime == 0.0f || a_fTime < fTime )
			m_aSprintMedalTimes[ a_iSlot ].m_fBronzeTime = a_fTime;
	}
	else if ( a_iMedal == 1 )
	{
		TFLOAT fTime = m_aSprintMedalTimes[ a_iSlot ].m_fSilverTime;

		if ( fTime == 0.0f || a_fTime < fTime )
			m_aSprintMedalTimes[ a_iSlot ].m_fSilverTime = a_fTime;
	}
	else if ( a_iMedal == 2 )
	{
		TFLOAT fTime = m_aSprintMedalTimes[ a_iSlot ].m_fGoldTime;

		if ( fTime == 0.0f || a_fTime < fTime )
			m_aSprintMedalTimes[ a_iSlot ].m_fGoldTime = a_fTime;
	}
}

// NOTE: retail's assert names Source\MemoryCard\ASaveOptions.cpp
// $deBlob: FUNCTION 005e3470
void EnSaveData::SlotData::RecordSprintBest( TINT a_iLevel, TUINT32 a_uiCount, TUINT32 a_uiTotal )
{
	TASSERT( a_iLevel >= 0 && a_iLevel < AGameStatus::SprintLevel_NUMOF );

	if ( a_uiCount > m_auiSprintBestCount[ a_iLevel ] )
		m_auiSprintBestCount[ a_iLevel ] = a_uiCount;

	m_auiSprintTotal[ a_iLevel ] = a_uiTotal;
}

// $deBlob: FUNCTION 005e3550
TINT EnSaveData::SlotData::CountAwards()
{
	m_uiNumAwards = 0;

	for ( TINT i = 0; i < 10; i++ )
	{
		for ( TINT j = 0; j < AGameStatus::s_iNumAwards; j++ )
		{
			if ( AGameStatus::s_aAwards[ j ].m_uiFlag & m_auiMedals[ i ] )
				m_uiNumAwards++;
		}
	}

	return m_uiNumAwards;
}

// $deBlob: FUNCTION 005e2c00
void EnSaveData::SlotData::UnlockWorld( TINT a_iWorld, ABlobRewardScreen* a_pRewardScreen )
{
	TUINT32 uiBit = 1 << a_iWorld;

	if ( ( uiBit & m_iUnlockedWorlds ) == 0 )
	{
		if ( TUINT( a_iWorld ) < 10 )
		{
			m_uiNewFlags |= NEWFLAG_World << a_iWorld;
			m_uiNewFlagsUnseen |= NEWFLAG_World << a_iWorld;
		}

		m_iUnlockedWorlds |= uiBit;
		a_pRewardScreen->AddReward( ABlobRewardScreen::RewardType_World, a_iWorld );
	}
}

// $deBlob: FUNCTION 005e2d50
void EnSaveData::SlotData::UnlockWorldMode( TINT a_iWorld, TINT a_iMode, ABlobRewardScreen* a_pRewardScreen )
{
	TUINT8 uiModes = TUINT8( m_auiUnlockedModes[ a_iWorld ] );
	TUINT8 uiBit   = TUINT8( 1 << a_iMode );

	if ( ( uiBit & uiModes ) == 0 )
	{
		m_auiUnlockedModes[ a_iWorld ] = TUINT8( uiBit | uiModes );

		if ( a_iMode == 1 )
		{
			if ( TUINT( a_iWorld ) < 10 )
			{
				m_uiNewFlags |= ( NEWFLAG_FreePaintWorld << a_iWorld ) | NEWFLAG_FreePaint;
				m_uiNewFlagsUnseen |= ( NEWFLAG_FreePaintWorld << a_iWorld ) | NEWFLAG_FreePaint;
			}

			a_pRewardScreen->AddReward( ABlobRewardScreen::RewardType_WorldMode, a_iWorld );
		}
	}
}

// $deBlob: FUNCTION 005e2f40
void EnSaveData::SlotData::UnlockChallenge( TINT a_iWorld, TINT a_iChallenge, ABlobRewardScreen* a_pRewardScreen )
{
	TUINT32 uiBit = 1 << a_iChallenge;

	if ( ( uiBit & m_iUnlockedChallenges ) == 0 )
	{
		if ( TUINT( a_iWorld ) < 10 )
		{
			m_uiNewFlags |= NEWFLAG_World << a_iWorld;
			m_uiNewFlagsUnseen |= NEWFLAG_World << a_iWorld;
		}

		m_auiUnk308[ 0 ] |= uiBit;
		m_auiUnk308[ 1 ] |= uiBit;
		m_iUnlockedChallenges |= uiBit;
		a_pRewardScreen->AddReward( ABlobRewardScreen::RewardType_Challenge, a_iChallenge );
	}
}

// $deBlob: FUNCTION 005e30e0
void EnSaveData::SlotData::UnlockMovie( TINT a_iMovie, ABlobRewardScreen* a_pRewardScreen )
{
	TUINT32 uiBit = 1 << a_iMovie;

	if ( ( uiBit & m_iUnlockedMovies ) == 0 )
	{
		m_uiNewFlags |= NEWFLAG_Extras | NEWFLAG_Cutscenes;
		m_uiNewFlagsUnseen |= NEWFLAG_Extras | NEWFLAG_Cutscenes;
		m_iUnlockedMovies |= uiBit;
		a_pRewardScreen->AddReward( ABlobRewardScreen::RewardType_Movie, a_iMovie );
	}
}

// $deBlob: FUNCTION 005e30a0
void EnSaveData::SlotData::UnlockVideo( TINT a_iVideo, ABlobRewardScreen* a_pRewardScreen )
{
	TUINT32 uiBit = 1 << a_iVideo;

	if ( ( uiBit & m_iUnlockedVideos ) == 0 )
	{
		m_uiNewFlags |= NEWFLAG_Extras | NEWFLAG_Movies;
		m_uiNewFlagsUnseen |= NEWFLAG_Extras | NEWFLAG_Movies;
		m_iUnlockedVideos |= uiBit;
		a_pRewardScreen->AddReward( ABlobRewardScreen::RewardType_Video, a_iVideo );
	}
}

// $deBlob: FUNCTION 005e3120
void EnSaveData::SlotData::UnlockGalleryItem( TINT a_iItem, ABlobRewardScreen* a_pRewardScreen )
{
	TUINT32 uiItems = m_uiUnlockedGalleryItems;

	if ( ( ( 1 << a_iItem ) & uiItems ) == 0 )
	{
		m_uiNewFlags |= NEWFLAG_Extras | NEWFLAG_Gallery;
		m_uiNewFlagsUnseen |= NEWFLAG_Extras | NEWFLAG_Gallery;
		m_uiUnlockedGalleryItems = TUINT16( uiItems | ( 1 << a_iItem ) );
		a_pRewardScreen->AddReward( ABlobRewardScreen::RewardType_Gallery, a_iItem );
	}
}

// $deBlob: FUNCTION 005e3170
void EnSaveData::SlotData::UnlockBlobParty( TINT a_iItem, ABlobRewardScreen* a_pRewardScreen )
{
	TUINT32 uiBit = 1 << a_iItem;

	if ( ( uiBit & m_iUnlockedBlobParty ) == 0 )
	{
		m_iUnlockedBlobParty |= uiBit;
		m_uiNewFlagsExtended |= uiBit | NEWFLAGEXT_BlobParty;
		m_uiNewFlagsExtendedUnseen |= uiBit | NEWFLAGEXT_BlobParty;
		a_pRewardScreen->AddReward( ABlobRewardScreen::RewardType_BlobParty, a_iItem );
	}
}
