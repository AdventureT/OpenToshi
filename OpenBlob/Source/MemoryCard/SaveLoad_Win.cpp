#include "pch.h"
#include "SaveLoad_Win.h"
#include "AApplication.h"
#include "SteamManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The name of the save in the Steam Cloud
static const TCHAR* s_szSaveFileName = "SaveData.dat";

// $deBlob: FUNCTION 005e3fe0
// $deBlob: FUNCTION 005e3ff0
// $deBlob: FUNCTION 005e4000
// $deBlob: FUNCTION 005e4010
// $deBlob: FUNCTION 005e4020
// $deBlob: FUNCTION 0053c9a0
// $deBlob: FUNCTION 008585d0
TDEFINE_CLASS_NORUNTIME( SaveLoadSKU );

// $deBlob: FUNCTION 005e4030
SaveLoadSKU::SaveLoadSKU()
{
	m_iError     = ERROR_NONE;
	m_eOperation = OPERATION_IDLE;
	m_pSaveData  = TSTATICCAST( EnSaveData, g_oTheApp.m_pSaveData );
}

// $deBlob: FUNCTION 005e4070
void SaveLoadSKU::Update( TFLOAT a_fDeltaTime )
{
	switch ( m_eOperation )
	{
		case OPERATION_LOAD:
		case OPERATION_STARTUPLOAD:
			if ( !SteamManager::GetInstance()->FileOperation( SteamManager::FileOp_Read, s_szSaveFileName, m_pSaveData->GetBuffer(), m_pSaveData->GetBufferSize(), TNULL ) )
			{
				m_iError     = ERROR_FAILED;
				m_eOperation = OPERATION_NONE;
				break;
			}

			if ( m_pSaveData->Validate() )
			{
				m_pSaveData->GetSaveData()->m_oExtra.Sync();
				m_iError     = ERROR_OK;
				m_eOperation = OPERATION_NONE;
				break;
			}

			// NOTE: an invalid save (header, CRC) only gets the default options and counts as loaded: the profiles
			// read from the file are kept
			m_pSaveData->ResetExtra();
			m_iError     = ERROR_OK;
			m_eOperation = OPERATION_NONE;
			break;
		case OPERATION_NEWGAMELOAD:
			TASSERT( TFALSE, "OPERATION_NEWGAMELOAD not implemented" );
			break;
		case OPERATION_SAVE:
		case OPERATION_SAVEPROFILE:
		case OPERATION_5:
		case OPERATION_6:
		case OPERATION_7:
			m_pSaveData->SetHeader();
			m_pSaveData->SetCRC();

			// NOTE: Validate also clamps the camera speed; retail builds keep their asserts, ours only without
			// TOSHI_FINAL
			TASSERT( m_pSaveData->Validate() );

			if ( SteamManager::GetInstance()->FileOperation( SteamManager::FileOp_Write, s_szSaveFileName, m_pSaveData->GetBuffer(), m_pSaveData->GetBufferSize(), TNULL ) )
			{
				m_iError     = ERROR_OK;
				m_eOperation = OPERATION_NONE;
			}
			else
			{
				m_iError     = ERROR_FAILED;
				m_eOperation = OPERATION_NONE;
			}
			break;
		default:
			return;
	}
}

// $deBlob: FUNCTION 005e41d0
void SaveLoadSKU::Load()
{
	m_eOperation = OPERATION_LOAD;
}

// $deBlob: FUNCTION 005e41e0
void SaveLoadSKU::SaveProfile( TINT a_iUnk1, TINT a_iUnk2 )
{
	m_eOperation = OPERATION_SAVEPROFILE;
}

// $deBlob: FUNCTION 005e41f0
void SaveLoadSKU::Operation5()
{
	m_eOperation = OPERATION_5;
}

// $deBlob: FUNCTION 005e4200
void SaveLoadSKU::Operation6()
{
	m_eOperation = OPERATION_6;
}

// $deBlob: FUNCTION 005e4210
void SaveLoadSKU::Operation7()
{
	m_eOperation = OPERATION_7;
}

// $deBlob: FUNCTION 005e4220
void SaveLoadSKU::Operation8()
{
	m_eOperation = OPERATION_8;
}

// $deBlob: FUNCTION 005e4230
void SaveLoadSKU::StartupLoad( TINT a_iUnk )
{
	m_eOperation = OPERATION_STARTUPLOAD;
}

// $deBlob: FUNCTION 005e4240
void SaveLoadSKU::NewGameLoad()
{
	m_eOperation = OPERATION_NEWGAMELOAD;
}

// $deBlob: FUNCTION 005e4250
void SaveLoadSKU::Save()
{
	m_eOperation = OPERATION_SAVE;
}

// $deBlob: FUNCTION 005e4260
void SaveLoadSKU::Operation9()
{
	m_eOperation = OPERATION_9;
}

// $deBlob: FUNCTION 005e4280
// $deBlob: FUNCTION 005e4060
SaveLoadSKU::~SaveLoadSKU()
{
}

// $deBlob: FUNCTION 005e42b0
TINT SaveLoadSKU::GetError()
{
	return m_iError;
}
