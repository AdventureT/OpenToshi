#pragma once
#include "SteamAPI.h"

// NOTE: 4-byte member function pointers in SteamCallback (retail's CCallbacks are 0x14 bytes), SteamManager is
// incomplete where its members are declared
class __single_inheritance SteamManager;

//-----------------------------------------------------------------------------
// CCallback of steam_api.h (library code): calls a member function of T with the callback data P.
//-----------------------------------------------------------------------------
template <class T, class P>
class SteamCallback : public CCallbackBase
{
public:
	typedef void ( T::*t_Func )( P* );

	SteamCallback( T* a_pObj, t_Func a_Func )
	    : m_pObj( TNULL ), m_Func( TNULL )
	{
		if ( a_pObj && a_Func )
		{
			m_pObj = a_pObj;
			m_Func = a_Func;
			SteamAPI_RegisterCallback( this, P::k_iCallback );
		}
	}

	~SteamCallback()
	{
		if ( IsRegistered() ) SteamAPI_UnregisterCallback( this );
	}

	virtual void Run( void* a_pvParam ) override { ( m_pObj->*m_Func )( TSTATICCAST( P, a_pvParam ) ); }
	virtual void Run( void* a_pvParam, bool, SteamAPICall_t ) override { ( m_pObj->*m_Func )( TSTATICCAST( P, a_pvParam ) ); }
	virtual int  GetCallbackSizeBytes() override { return sizeof( P ); }

private:
	T*     m_pObj; // 0x0C
	t_Func m_Func; // 0x10
};

// The callback data of steam_api.h (isteamuserstats.h)
#pragma pack( push, 8 )
struct UserStatsReceived_t
{
	enum
	{
		k_iCallback = 1101
	};
	TUINT64 m_nGameID;     // 0x00
	TINT32  m_eResult;     // 0x08
	TUINT64 m_steamIDUser; // 0x0C
};

struct UserStatsStored_t
{
	enum
	{
		k_iCallback = 1102
	};
	TUINT64 m_nGameID; // 0x00
	TINT32  m_eResult; // 0x08
};

struct UserAchievementStored_t
{
	enum
	{
		k_iCallback = 1103
	};
	TUINT64 m_nGameID;           // 0x00
	bool    m_bGroupAchievement; // 0x08
	char    m_rgchAchievementName[ 128 ];
	TUINT32 m_nCurProgress;
	TUINT32 m_nMaxProgress;
};
#pragma pack( pop )

//-----------------------------------------------------------------------------
// Purpose: de Blob's Steam integration: initialises Steam, requests the stats and achievements, gives the language of
// the Steam client.
//-----------------------------------------------------------------------------
class SteamManager
{
public:
	// de Blob's app id on Steam
	static const AppId_t APP_ID;

public:
	SteamManager();
	~SteamManager();

	// Creates the instance
	static void Create();

	// Shuts Steam down and deletes the instance
	static void Unload();

	// SteamAPI_Init, logs the build id, requests the stats. TFALSE if Steam isn't running or restarts the game
	TBOOL InitializeSteam();

	// The language of the Steam client ("english"...), TNULL without Steam
	static const TCHAR* GetSteamLanguage();

	// The operations of FileOperation
	enum FileOp : TINT
	{
		FileOp_Write = 0,
		FileOp_Read  = 1,
	};

	// The results of FileOperation (given to the callback)
	enum FileResult : TINT
	{
		FileResult_OK       = 0,
		FileResult_Failed   = -2, // no Steam, or the operation failed
		FileResult_NotFound = -4, // read: the file doesn't exist
	};

	typedef void( __stdcall* t_FileCallback )( TINT a_iResult );

	// Writes or reads a whole file of the user's Steam Cloud storage (ISteamRemoteStorage, e.g. the save
	// SaveData.dat in Steam\userdata\<user>\532320\remote). Calls a_fnCallback with the result if given, TTRUE if
	// it's FileResult_OK
	TBOOL FileOperation( FileOp a_eOperation, const TCHAR* a_szFileName, void* a_pData, TINT a_iSize, t_FileCallback a_fnCallback );

#if defined( DEBLOB_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
	// Development (-devsavedir <dir>, Debug/Release only): FileOperation reads and writes <dir>\<file> instead of
	// the Steam Cloud, so test runs never touch the user's saves. TNULL without the switch.
	static const TCHAR* GetDevSaveDir();
#endif

	// The language ids are ALocaleManager's (T2Locale::Lang).
	static TINT  GetLanguageIdFromCode( const TWCHAR* a_wszCode );
	static TINT  GetLanguageIdFromName( const TCHAR* a_szLanguage );
	static TINT  GetLanguageId();
	static TBOOL IsJapaneseLanguage();
	static TBOOL IsKoreanLanguage();
	static TBOOL IsChineseLanguage();

	// Unlocks / locks the Steam achievement a_szName ("AchievementID_..." of AAchievements) and stores the stats.
	// TFALSE until the stats were received (m_bStatsValid)
	TBOOL SetAchievement( const TCHAR* a_szName );
	TBOOL ClearAchievement( const TCHAR* a_szName );

	// $deBlob: FUNCTION 0056c6b0
	static SteamManager* GetInstance()
	{
		TASSERT( m_instance != NULL, "No existing instance" );
		return m_instance;
	}

private:
	// Requests the stats and achievements of the user (if logged on)
	TBOOL RequestStats();

	void OnUserStatsReceived( UserStatsReceived_t* a_pCallback );
	void OnUserStatsStored( UserStatsStored_t* a_pCallback );
	void OnAchievementStored( UserAchievementStored_t* a_pCallback );

private:
	static SteamManager* m_instance;

private:
	SteamCallback<SteamManager, UserStatsReceived_t>     m_CallbackUserStatsReceived; // 0x00
	SteamCallback<SteamManager, UserStatsStored_t>       m_CallbackUserStatsStored;   // 0x14
	SteamCallback<SteamManager, UserAchievementStored_t> m_CallbackAchievementStored; // 0x28
	TBOOL                                                m_bStatsValid;               // 0x3C
};
