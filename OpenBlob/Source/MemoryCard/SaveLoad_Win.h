#pragma once
#include "SaveLoad.h"
#include "EnSaveData.h"

//-----------------------------------------------------------------------------
// Purpose: the save system on PC: the application's save data
// (EnSaveData, 0xC88 bytes) is read and written as is as the Steam Cloud file SaveData.dat
// (SteamManager::FileOperation). Every operation finishes in the next Update.
//-----------------------------------------------------------------------------
class SaveLoadSKU : public SaveLoad
{
public:
	TDECLARE_CLASS( SaveLoadSKU, SaveLoad );

	enum Operation : TINT
	{
		OPERATION_NONE        = -1, // the last operation finished
		OPERATION_LOAD        = 0,
		OPERATION_NEWGAMELOAD = 1, // not implemented on PC
		OPERATION_STARTUPLOAD = 2,
		OPERATION_SAVE        = 3,
		OPERATION_SAVEPROFILE = 4,
		OPERATION_5           = 5, // saves
		OPERATION_6           = 6, // saves
		OPERATION_7           = 7, // saves
		OPERATION_8           = 8, // does nothing
		OPERATION_9           = 9, // does nothing
		OPERATION_IDLE        = 10,
	};

public:
	SaveLoadSKU();
	~SaveLoadSKU();

	//-----------------------------------------------------------------------------
	// SaveLoad
	//-----------------------------------------------------------------------------
	virtual void Load() override;                                    // +0x0C
	virtual void SaveProfile( TINT a_iUnk1, TINT a_iUnk2 ) override; // +0x10
	virtual void Operation5() override;                              // +0x14
	virtual void Operation6() override;                              // +0x18
	virtual void Operation7() override;                              // +0x1C
	virtual void Operation8() override;                              // +0x20
	virtual void StartupLoad( TINT a_iUnk ) override;                // +0x24
	virtual void NewGameLoad() override;                             // +0x28
	virtual void Save() override;                                    // +0x2C
	virtual void Operation9() override;                              // +0x30
	virtual TINT GetError() override;                                // +0x34
	virtual void Update( TFLOAT a_fDeltaTime ) override;             // +0x38

private:
	TINT        m_iError;     // 0x10 SaveLoad::Error
	Operation   m_eOperation; // 0x14
	EnSaveData* m_pSaveData;  // 0x18 the application's save data
};
