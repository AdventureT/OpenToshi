#pragma once

//-----------------------------------------------------------------------------
// Purpose: the subset of the Steamworks API (steam_api.h, SDK of 2017: SteamClient017, SteamUser019,
// STEAMUSERSTATS_INTERFACE_VERSION011, STEAMAPPS_INTERFACE_VERSION008, STEAMREMOTESTORAGE_INTERFACE_VERSION013) that de
// Blob uses.
//
// NOTE: the Steamworks SDK isn't available to link against. Retail links steam_api.lib and calls the exports of
// steam_api.dll directly; we load the retail steam_api.dll at runtime (see SteamAPI.cpp) and call the same exports
// with the same arguments. The interfaces are declared with the slots of the SDK's vtables (unused slots are
// placeholders), the callback base class has the SDK's layout, so steam_api.dll sees the same objects.
// The inline steam_api.h code (CCallback) is library code and not counted.
//-----------------------------------------------------------------------------

typedef TINT32  HSteamPipe;
typedef TINT32  HSteamUser;
typedef TUINT64 SteamAPICall_t;
typedef TUINT32 AppId_t;

//-----------------------------------------------------------------------------
// CCallbackBase of steam_api.h
//-----------------------------------------------------------------------------
class CCallbackBase
{
public:
	enum
	{
		k_ECallbackFlagsRegistered = 0x01,
		k_ECallbackFlagsGameServer = 0x02
	};

	CCallbackBase()
	{
		m_nCallbackFlags = 0;
		m_iCallback      = 0;
	}

	// NOTE: same declaration order as steam_api.h, MSVC lays the overloads out the same way
	virtual void Run( void* a_pvParam )                                                    = 0;
	virtual void Run( void* a_pvParam, bool a_bIOFailure, SteamAPICall_t a_hSteamAPICall ) = 0;
	virtual int  GetCallbackSizeBytes()                                                    = 0;

	TBOOL IsRegistered() const { return m_nCallbackFlags & k_ECallbackFlagsRegistered; }

protected:
	TUINT8 m_nCallbackFlags; // 0x04
	TINT   m_iCallback;      // 0x08
};

class ISteamUser
{
public:
	virtual HSteamUser GetHSteamUser() = 0; // +0x00
	virtual bool       BLoggedOn()     = 0; // +0x04
};

class ISteamUserStats
{
public:
	virtual bool RequestCurrentStats()                     = 0; // +0x00
	virtual void Slot01()                                  = 0; // +0x04 GetStat( const char*, int32* ) (MSVC overload order)
	virtual void Slot02()                                  = 0; // +0x08 GetStat( const char*, float* )
	virtual void Slot03()                                  = 0; // +0x0C SetStat( const char*, int32 )
	virtual void Slot04()                                  = 0; // +0x10 SetStat( const char*, float )
	virtual void Slot05()                                  = 0; // +0x14 UpdateAvgRateStat
	virtual void Slot06()                                  = 0; // +0x18 GetAchievement
	virtual bool SetAchievement( const char* a_pchName )   = 0; // +0x1C
	virtual bool ClearAchievement( const char* a_pchName ) = 0; // +0x20
	virtual void Slot09()                                  = 0; // +0x24 GetAchievementAndUnlockTime
	virtual bool StoreStats()                              = 0; // +0x28
};

class ISteamApps
{
public:
	virtual bool        BIsSubscribed()          = 0; // +0x00
	virtual bool        BIsLowViolence()         = 0; // +0x04
	virtual bool        BIsCybercafe()           = 0; // +0x08
	virtual bool        BIsVACBanned()           = 0; // +0x0C
	virtual const char* GetCurrentGameLanguage() = 0; // +0x10
	virtual void        Slot05()                 = 0; // +0x14 GetAvailableGameLanguages
	virtual void        Slot06()                 = 0; // +0x18 BIsSubscribedApp
	virtual void        Slot07()                 = 0; // +0x1C BIsDlcInstalled
	virtual void        Slot08()                 = 0; // +0x20 GetEarliestPurchaseUnixTime
	virtual void        Slot09()                 = 0; // +0x24 BIsSubscribedFromFreeWeekend
	virtual void        Slot10()                 = 0; // +0x28 GetDLCCount
	virtual void        Slot11()                 = 0; // +0x2C BGetDLCDataByIndex
	virtual void        Slot12()                 = 0; // +0x30 InstallDLC
	virtual void        Slot13()                 = 0; // +0x34 UninstallDLC
	virtual void        Slot14()                 = 0; // +0x38 RequestAppProofOfPurchaseKey
	virtual void        Slot15()                 = 0; // +0x3C GetCurrentBetaName
	virtual void        Slot16()                 = 0; // +0x40 MarkContentCorrupt
	virtual void        Slot17()                 = 0; // +0x44 GetInstalledDepots
	virtual void        Slot18()                 = 0; // +0x48 GetAppInstallDir
	virtual void        Slot19()                 = 0; // +0x4C BIsAppInstalled
	virtual void        Slot20()                 = 0; // +0x50 GetAppOwner
	virtual void        Slot21()                 = 0; // +0x54 GetLaunchQueryParam
	virtual void        Slot22()                 = 0; // +0x58 GetDlcDownloadProgress
	virtual int         GetAppBuildId()          = 0; // +0x5C
};

// The Steam Cloud files (the save, SaveData.dat)
class ISteamRemoteStorage
{
public:
	virtual bool FileWrite( const char* a_pchFile, const void* a_pvData, int a_cubData ) = 0; // +0x00
	virtual int  FileRead( const char* a_pchFile, void* a_pvData, int a_cubDataToRead )  = 0; // +0x04
	virtual void Slot02()                                                                = 0; // +0x08 FileWriteAsync
	virtual void Slot03()                                                                = 0; // +0x0C FileReadAsync
	virtual void Slot04()                                                                = 0; // +0x10 FileReadAsyncComplete
	virtual void Slot05()                                                                = 0; // +0x14 FileForget
	virtual void Slot06()                                                                = 0; // +0x18 FileDelete
	virtual void Slot07()                                                                = 0; // +0x1C FileShare
	virtual void Slot08()                                                                = 0; // +0x20 SetSyncPlatforms
	virtual void Slot09()                                                                = 0; // +0x24 FileWriteStreamOpen
	virtual void Slot10()                                                                = 0; // +0x28 FileWriteStreamWriteChunk
	virtual void Slot11()                                                                = 0; // +0x2C FileWriteStreamClose
	virtual void Slot12()                                                                = 0; // +0x30 FileWriteStreamCancel
	virtual bool FileExists( const char* a_pchFile )                                     = 0; // +0x34
};

class ISteamClient
{
public:
	virtual void                 Slot00()                                                                                             = 0; // +0x00 CreateSteamPipe
	virtual void                 Slot01()                                                                                             = 0; // +0x04 BReleaseSteamPipe
	virtual void                 Slot02()                                                                                             = 0; // +0x08 ConnectToGlobalUser
	virtual void                 Slot03()                                                                                             = 0; // +0x0C CreateLocalUser
	virtual void                 Slot04()                                                                                             = 0; // +0x10 ReleaseUser
	virtual ISteamUser*          GetISteamUser( HSteamUser a_hSteamUser, HSteamPipe a_hSteamPipe, const char* a_pchVersion )          = 0; // +0x14
	virtual void                 Slot06()                                                                                             = 0; // +0x18 GetISteamGameServer
	virtual void                 Slot07()                                                                                             = 0; // +0x1C SetLocalIPBinding
	virtual void                 Slot08()                                                                                             = 0; // +0x20 GetISteamFriends
	virtual void                 Slot09()                                                                                             = 0; // +0x24 GetISteamUtils
	virtual void                 Slot10()                                                                                             = 0; // +0x28 GetISteamMatchmaking
	virtual void                 Slot11()                                                                                             = 0; // +0x2C GetISteamMatchmakingServers
	virtual void                 Slot12()                                                                                             = 0; // +0x30 GetISteamGenericInterface
	virtual ISteamUserStats*     GetISteamUserStats( HSteamUser a_hSteamUser, HSteamPipe a_hSteamPipe, const char* a_pchVersion )     = 0; // +0x34
	virtual void                 Slot14()                                                                                             = 0; // +0x38 GetISteamGameServerStats
	virtual ISteamApps*          GetISteamApps( HSteamUser a_hSteamUser, HSteamPipe a_hSteamPipe, const char* a_pchVersion )          = 0; // +0x3C
	virtual void                 Slot16()                                                                                             = 0; // +0x40 GetISteamNetworking
	virtual ISteamRemoteStorage* GetISteamRemoteStorage( HSteamUser a_hSteamUser, HSteamPipe a_hSteamPipe, const char* a_pchVersion ) = 0; // +0x44
};

//-----------------------------------------------------------------------------
// The steam_api.dll exports de Blob imports
//-----------------------------------------------------------------------------
bool       SteamAPI_Init();
void       SteamAPI_Shutdown();
bool       SteamAPI_RestartAppIfNecessary( AppId_t a_unOwnAppID );
void       SteamAPI_RunCallbacks();
void       SteamAPI_RegisterCallback( CCallbackBase* a_pCallback, int a_iCallback );
void       SteamAPI_UnregisterCallback( CCallbackBase* a_pCallback );
void       SteamAPI_SetMiniDumpComment( const char* a_pchMsg );
void       SteamAPI_WriteMiniDump( TUINT32 a_uStructuredExceptionCode, void* a_pvExceptionInfo, TUINT32 a_uBuildID );
HSteamUser SteamAPI_GetHSteamUser();
HSteamPipe SteamAPI_GetHSteamPipe();
void*      SteamInternal_CreateInterface( const char* a_szVersion );

//-----------------------------------------------------------------------------
// The accessors of steam_api.h (inline: CSteamAPIContext, library code) for the interfaces de Blob uses
//-----------------------------------------------------------------------------
ISteamUser*          SteamUser();
ISteamUserStats*     SteamUserStats();
ISteamApps*          SteamApps();
ISteamRemoteStorage* SteamRemoteStorage();

#ifdef DEBLOB_COMMUNITY_PATCH
// TTRUE if steam_api.dll was found next to the executable (see SteamAPI.cpp)
TBOOL SteamAPI_IsAvailable();
#endif
