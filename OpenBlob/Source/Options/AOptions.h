#pragma once
#include "MemoryCard/EnSaveData.h"

#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>

//-----------------------------------------------------------------------------
// Purpose: the game options: they live in the save data (EnSaveData's SaveExtra, the buffer of g_oTheApp's save data);
// the setters mark them dirty and Apply passes them to the sound and the camera.
// NOTE: the save data is the in-memory one (SaveLoad isn't ported: the options are EnSaveData::ResetExtra's defaults, like retail without a save)
//-----------------------------------------------------------------------------
class AOptions
    : public Toshi::TObject
    , public Toshi::TSingleton<AOptions>
{
public:
	TDECLARE_CLASS( AOptions, Toshi::TObject );

	static TCONSTEXPR_VAR TUINT32 OPTIONS_MAXSIZE = sizeof( EnSaveData::SaveExtra );

	typedef TUINT32 DIRTYFLAGS;
	enum DIRTYFLAGS_ : DIRTYFLAGS
	{
		DIRTYFLAGS_Sound = BITFLAG( 0 ),
		DIRTYFLAGS_Unk2  = BITFLAG( 1 ),
		DIRTYFLAGS_All   = 0xFFFFFFFF,
	};

public:
	AOptions();
	~AOptions();

	// Applies the options (the volumes only if they changed or a_bForce), clears the dirty flags
	virtual TBOOL Apply( TBOOL a_bForce ); // +0x0C

	// Sets the volumes of the sound categories: the options' ones, or a_fSFXVolume/a_fMusicVolume if >= 0
	void UpdateVolumes( TBOOL a_bForce, TFLOAT a_fSFXVolume, TFLOAT a_fMusicVolume );

	// Copies the options to/from a buffer (the save)
	TBOOL GetOptions( void* a_pOptions, TUINT32 a_uiMaxSize );
	TBOOL SetOptions( const void* a_pOptions, TUINT32 a_uiSize );

	TFLOAT GetSFXVolume() const;
	TFLOAT GetMusicVolume() const;

	TFLOAT GetCameraSpeed() const { return m_pOptions->m_fCameraSpeed; }
	TBOOL  IsCameraInverted() const { return ( m_pOptions->m_uiFlags & EnSaveData::SaveExtra::FLAG_InvertCamera ) != 0; }
	TBOOL  IsHelpEnabled() const { return ( m_pOptions->m_uiFlags & EnSaveData::SaveExtra::FLAG_HelpEnabled ) != 0; }

	// The options menu's Apply writes these straight into the options (retail has no dirty flag for them)
	void SetCameraSpeed( TFLOAT a_fSpeed ) { m_pOptions->m_fCameraSpeed = a_fSpeed; }
	void SetCameraInverted( TBOOL a_bInverted ) { m_pOptions->m_uiFlags = a_bInverted ? ( m_pOptions->m_uiFlags | EnSaveData::SaveExtra::FLAG_InvertCamera ) : ( m_pOptions->m_uiFlags & ~EnSaveData::SaveExtra::FLAG_InvertCamera ); }
	void SetHelpEnabled( TBOOL a_bEnabled ) { m_pOptions->m_uiFlags = a_bEnabled ? ( m_pOptions->m_uiFlags | EnSaveData::SaveExtra::FLAG_HelpEnabled ) : ( m_pOptions->m_uiFlags & ~EnSaveData::SaveExtra::FLAG_HelpEnabled ); }

	void            SetUnk00( TINT a_iValue );
	void            SetSFXVolume( TINT a_iVolume );
	void            SetMusicVolume( TINT a_iVolume );
	void            SetUnk14( TINT a_iValue );
	Toshi::TString8 GetUnk14Unk18String() const;
	void            SetUnk18( TINT a_iValue );

private:
	EnSaveData::SaveExtra* m_pOptions;     // 0x4 in the save data
	DIRTYFLAGS             m_uiDirtyFlags; // 0x8
};
