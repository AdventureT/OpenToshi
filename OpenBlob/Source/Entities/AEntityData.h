#pragma once
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: the shared entity data (AEntityData.cpp): the TRBs of data\entities (entity types, poses, pose maps, states,
// projectiles, FX breakpoints), loaded once while the legal screen shows.
//-----------------------------------------------------------------------------
class AEntityData
{
public:
	AEntityData();
	~AEntityData();

	void Load();
	void Unload();

	void* GetEntityData( const TCHAR* a_szSymbol );
	void* GetPoseData( const TCHAR* a_szSymbol );
	void* GetPoseMapData( const TCHAR* a_szSymbol );
	void* GetStateData( const TCHAR* a_szSymbol );
	void* GetProjectileData( const TCHAR* a_szSymbol );
	void* GetFXBreakpointData( const TCHAR* a_szSymbol );

	// Returns the last component of a path ("a/b/ACiv" -> "ACiv")
	static const TCHAR* GetDirectoryName( const TCHAR* a_szPath );

	static AEntityData* GetSingleton() { return sm_Singleton; }

private:
	static Toshi::TTRB* LoadTRB( const TCHAR* a_szFileName );

private:
	static AEntityData* sm_Singleton;

private:
	Toshi::TTRB* m_TRBEntityData;       // 0x00
	Toshi::TTRB* m_TRBPoseData;         // 0x04
	Toshi::TTRB* m_TRBPoseMapData;      // 0x08
	Toshi::TTRB* m_TRBStateData;        // 0x0C
	Toshi::TTRB* m_TRBProjectileData;   // 0x10
	Toshi::TTRB* m_TRBFXBreakpointData; // 0x14
};
