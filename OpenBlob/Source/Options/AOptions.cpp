#include "pch.h"
#include "AOptions.h"
#include "Cameras/ABlobFollowCamera.h"
#include "AApplication.h"
#include "Sound/AMusicManager.h"
#include "Sound/ASoundInterface.h"
#include "Sound/ASoundLoadHelper.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005e8300
// $deBlob: FUNCTION 005e8310
// $deBlob: FUNCTION 005e8380
// $deBlob: FUNCTION 005e83e0
// $deBlob: FUNCTION 005e83f0
// $deBlob: FUNCTION 0053c9e0
// $deBlob: FUNCTION 008585e0
TDEFINE_CLASS( AOptions );

// NOTE: the options are the ones of the save data in memory: SaveLoad isn't ported, so they are EnSaveData's
// defaults (ResetExtra), as in retail when there's no save; nothing reads or writes the options file
// $deBlob: FUNCTION 005e8400
AOptions::AOptions()
{
	m_uiDirtyFlags = 0;
	m_pOptions     = &TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData )->GetSaveData()->m_oExtra;
	m_uiDirtyFlags = DIRTYFLAGS_All;
}

// $deBlob: FUNCTION 005e8470
// $deBlob: FUNCTION 005e8940
AOptions::~AOptions()
{
}

// $deBlob: FUNCTION 005e84d0
TBOOL AOptions::Apply( TBOOL a_bForce )
{
	UpdateVolumes( a_bForce, -1.0f, -1.0f );

	ABlobFollowCamera::SetCamSpeedOption( m_pOptions->m_fCameraSpeed );
	ABlobFollowCamera::SetInvertCamera( ( m_pOptions->m_uiFlags >> 1 ) & 1 );

	m_uiDirtyFlags = 0;
	return TTRUE;
}

// $deBlob: FUNCTION 005e8530
void AOptions::UpdateVolumes( TBOOL a_bForce, TFLOAT a_fSFXVolume, TFLOAT a_fMusicVolume )
{
	if ( TSound::GetSingleton()->IsInitialised() && ( ( m_uiDirtyFlags & DIRTYFLAGS_Sound ) || a_bForce ) )
	{
		TMath::Clip( m_pOptions->m_iSFXVolume, 0, 10 );
		TMath::Clip( m_pOptions->m_iMusicVolume, 0, 10 );

		TFLOAT fMusicVolume = m_pOptions->m_iMusicVolume / 10.0f;
		TFLOAT fSFXVolume   = m_pOptions->m_iSFXVolume / 10.0f;

		if ( 0.0f <= a_fSFXVolume )
			fSFXVolume = a_fSFXVolume;

		if ( 0.0f <= a_fMusicVolume )
			fMusicVolume = a_fMusicVolume;

		if ( ASoundLoadHelper::GetSingleton()->GetState() != ASoundLoadHelper::UNLOADED )
		{
			ASoundInterface::SetCategoryVolume( ASoundMap::CATEGORYID_INGAME, fSFXVolume );
			ASoundInterface::SetCategoryVolume( ASoundMap::CATEGORYID_SPEECH, fSFXVolume );
			ASoundInterface::SetCategoryVolume( ASoundMap::CATEGORYID_GUI, fSFXVolume );
			AMusicManager::GetSingleton()->SetVolume( fMusicVolume, 1 );
			AMusicManager::GetSingleton()->SetVolume( fSFXVolume, 2 );
			AMusicManager::GetSingleton()->UpdateVolumes();
			ASoundInterface::SetCategoryVolume( ASoundMap::CATEGORYID_MUSIC, fMusicVolume );
		}
	}
}

// $deBlob: FUNCTION 005e8660
TBOOL AOptions::GetOptions( void* a_pOptions, TUINT32 a_uiMaxSize )
{
	TASSERT( OPTIONS_MAXSIZE <= a_uiMaxSize );
	TUtil::MemCopy( a_pOptions, m_pOptions, OPTIONS_MAXSIZE );
	return TTRUE;
}

// $deBlob: FUNCTION 005e86b0
TBOOL AOptions::SetOptions( const void* a_pOptions, TUINT32 a_uiSize )
{
	TASSERT( a_uiSize == OPTIONS_MAXSIZE );
	TUtil::MemCopy( m_pOptions, a_pOptions, OPTIONS_MAXSIZE );
	m_uiDirtyFlags = DIRTYFLAGS_All;
	return TTRUE;
}

// $deBlob: FUNCTION 005e8710
TFLOAT AOptions::GetSFXVolume() const
{
	return m_pOptions->m_iSFXVolume / 10.0f;
}

// $deBlob: FUNCTION 005e8740
TFLOAT AOptions::GetMusicVolume() const
{
	return m_pOptions->m_iMusicVolume / 10.0f;
}

// $deBlob: FUNCTION 005e8770
void AOptions::SetUnk00( TINT a_iValue )
{
	if ( a_iValue != m_pOptions->m_iUnk00 )
	{
		m_pOptions->m_iUnk00 = a_iValue;
		m_uiDirtyFlags |= DIRTYFLAGS_Sound;
	}
}

// $deBlob: FUNCTION 005e8790
void AOptions::SetSFXVolume( TINT a_iVolume )
{
	if ( a_iVolume != m_pOptions->m_iSFXVolume )
	{
		m_pOptions->m_iSFXVolume = a_iVolume;
		m_uiDirtyFlags |= DIRTYFLAGS_Sound;
	}
}

// $deBlob: FUNCTION 005e87b0
void AOptions::SetMusicVolume( TINT a_iVolume )
{
	if ( a_iVolume != m_pOptions->m_iMusicVolume )
	{
		m_pOptions->m_iMusicVolume = a_iVolume;
		m_uiDirtyFlags |= DIRTYFLAGS_Sound;
	}
}

// $deBlob: FUNCTION 005e87d0
void AOptions::SetUnk14( TINT a_iValue )
{
	if ( a_iValue != m_pOptions->m_iUnk14 )
	{
		m_pOptions->m_iUnk14 = a_iValue;
		m_uiDirtyFlags |= DIRTYFLAGS_Unk2;
	}
}

// "<m_iUnk14>x<m_iUnk18>" (a resolution?)
// $deBlob: FUNCTION 005e8810
TString8 AOptions::GetUnk14Unk18String() const
{
	TString8 strResult;
	strResult.Format( "%dx%d", m_pOptions->m_iUnk14, m_pOptions->m_iUnk18 );
	return strResult;
}

// $deBlob: FUNCTION 005e87f0
void AOptions::SetUnk18( TINT a_iValue )
{
	if ( a_iValue != m_pOptions->m_iUnk18 )
	{
		m_pOptions->m_iUnk18 = a_iValue;
		m_uiDirtyFlags |= DIRTYFLAGS_Unk2;
	}
}
