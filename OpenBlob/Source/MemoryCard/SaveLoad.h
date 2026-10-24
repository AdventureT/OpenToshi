#pragma once

//-----------------------------------------------------------------------------
// Purpose: the interface of the save system (one instance in
// s_instance): the operations only start an operation, Update runs it and GetError gives its result
// (0 while nothing finished yet, 1 when it succeeded, < 0 when it failed). The platform class is SaveLoadSKU
// (SaveLoad_Win.cpp).
//-----------------------------------------------------------------------------
class SaveLoad : public Toshi::TObject
{
public:
	TDECLARE_CLASS( SaveLoad, Toshi::TObject );

	enum Error : TINT
	{
		ERROR_NONE   = 0, // nothing finished yet
		ERROR_OK     = 1,
		ERROR_FAILED = -6, // reading or writing the file failed
	};

public:
	SaveLoad();
	~SaveLoad();

	virtual void Load()                                    = 0; // +0x0C
	virtual void SaveProfile( TINT a_iUnk1, TINT a_iUnk2 ) = 0; // +0x10 (the menus, the cheats)
	virtual void Operation5()                              = 0; // +0x14
	virtual void Operation6()                              = 0; // +0x18
	virtual void Operation7()                              = 0; // +0x1C
	virtual void Operation8()                              = 0; // +0x20
	virtual void StartupLoad( TINT a_iUnk )                = 0; // +0x24 (ASaveLoadStartAppState)
	virtual void NewGameLoad()                             = 0; // +0x28
	virtual void Save()                                    = 0; // +0x2C
	virtual void Operation9()                              = 0; // +0x30
	virtual TINT GetError()                                = 0; // +0x34

	// Runs the operation
	// $deBlob: FUNCTION 005e3fd0
	virtual void Update( TFLOAT a_fDeltaTime ) {} // +0x38

	TBOOL IsEnabled() const { return m_bEnabled; }

	// The menu that started the save (the menus set it before saving; NOTE: never read by SaveLoadSKU)
	void SetCaller( void* a_pCaller ) { m_pCaller = a_pCaller; }

	static SaveLoad* GetInstance() { return s_instance; }

protected:
	static SaveLoad* s_instance;

protected:
	TBOOL m_bEnabled; // 0x4 TTRUE (the menus save only if it's set)
	void* m_pCaller;  // 0x8
	TINT  m_iUnkC;    // 0xC
};
