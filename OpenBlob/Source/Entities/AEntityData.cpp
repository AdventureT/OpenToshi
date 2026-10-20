#include "pch.h"
#include "AEntityData.h"
#include "ACiv.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AEntityData* AEntityData::sm_Singleton;

// $deBlob: FUNCTION 007586b0
AEntityData::AEntityData()
{
	// NOTE: retail doesn't clear m_TRBFXBreakpointData (Load sets it)
	m_TRBEntityData     = TNULL;
	m_TRBPoseData       = TNULL;
	m_TRBPoseMapData    = TNULL;
	m_TRBStateData      = TNULL;
	m_TRBProjectileData = TNULL;

	TASSERT( !sm_Singleton );
	sm_Singleton = this;
}

// $deBlob: FUNCTION 00758720
// $deBlob: FUNCTION 0056bf80
AEntityData::~AEntityData()
{
	TASSERT( this == sm_Singleton );
	sm_Singleton = TNULL;

	TASSERT( 0 == m_TRBEntityData );
	TASSERT( 0 == m_TRBPoseData );
	TASSERT( 0 == m_TRBPoseMapData );
	TASSERT( 0 == m_TRBStateData );
	TASSERT( 0 == m_TRBProjectileData );
}

// $deBlob: FUNCTION 00758860
void AEntityData::Load()
{
	TASSERT( !m_TRBEntityData );
	m_TRBEntityData       = LoadTRB( "data/entities/EntityData.trb" );
	m_TRBPoseData         = LoadTRB( "data/entities/PoseData.trb" );
	m_TRBPoseMapData      = LoadTRB( "data/entities/PoseMaps.trb" );
	m_TRBStateData        = LoadTRB( "data/entities/StateData.trb" );
	m_TRBProjectileData   = LoadTRB( "data/entities/ProjectileData.trb" );
	m_TRBFXBreakpointData = LoadTRB( "data/entities/FXBreakPoints.trb" );

	ACiv::LoadPoseMaps();
}

// $deBlob: FUNCTION 00758900
void AEntityData::Unload()
{
	ACiv::UnloadPoseMaps();

	if ( m_TRBEntityData ) delete m_TRBEntityData;
	m_TRBEntityData = TNULL;

	if ( m_TRBPoseData ) delete m_TRBPoseData;
	m_TRBPoseData = TNULL;

	if ( m_TRBPoseMapData ) delete m_TRBPoseMapData;
	m_TRBPoseMapData = TNULL;

	if ( m_TRBStateData ) delete m_TRBStateData;
	m_TRBStateData = TNULL;

	if ( m_TRBProjectileData ) delete m_TRBProjectileData;
	m_TRBProjectileData = TNULL;

	if ( m_TRBFXBreakpointData ) delete m_TRBFXBreakpointData;
	m_TRBFXBreakpointData = TNULL;
}

// $deBlob: FUNCTION 007589c0
void* AEntityData::GetEntityData( const TCHAR* a_szSymbol )
{
	return m_TRBEntityData->GetSymbolAddress( a_szSymbol );
}

// $deBlob: FUNCTION 007589d0
void* AEntityData::GetFXBreakpointData( const TCHAR* a_szSymbol )
{
	return m_TRBFXBreakpointData->GetSymbolAddress( a_szSymbol );
}

// $deBlob: FUNCTION 007589e0
void* AEntityData::GetPoseMapData( const TCHAR* a_szSymbol )
{
	return m_TRBPoseMapData->GetSymbolAddress( a_szSymbol );
}

// $deBlob: FUNCTION 007589f0
void* AEntityData::GetPoseData( const TCHAR* a_szSymbol )
{
	return m_TRBPoseData->GetSymbolAddress( a_szSymbol );
}

// $deBlob: FUNCTION 00758a00
void* AEntityData::GetStateData( const TCHAR* a_szSymbol )
{
	return m_TRBStateData->GetSymbolAddress( a_szSymbol );
}

// $deBlob: FUNCTION 00758a10
void* AEntityData::GetProjectileData( const TCHAR* a_szSymbol )
{
	return m_TRBProjectileData->GetSymbolAddress( a_szSymbol );
}

// $deBlob: FUNCTION 00758a20
const TCHAR* AEntityData::GetDirectoryName( const TCHAR* a_szPath )
{
	for ( TINT i = TINT( TStringManager::String8Length( a_szPath ) ) - 1; i >= 0; i-- )
	{
		if ( a_szPath[ i ] == '/' || a_szPath[ i ] == '\\' )
			return &a_szPath[ i + 1 ];
	}

	return a_szPath;
}

// $deBlob: FUNCTION 00758a50
TTRB* AEntityData::LoadTRB( const TCHAR* a_szFileName )
{
	TTRB*       pTRB  = new TTRB();
	TTRB::ERROR ret  = pTRB->Load( a_szFileName, 0 );
	TASSERT( ret == TTRB::ERROR_OK );

	return pTRB;
}
