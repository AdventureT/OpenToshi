#include "pch.h"
#include "ACurrentLevelInformation.h"
#include "AAssetStreaming.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

ACurrentLevelInformation* ACurrentLevelInformation::sm_singleton;

const ACurrentLevelInformation::DataBlockFile ACurrentLevelInformation::sm_aDataBlockFiles[ DataBlockCount ] = {
	{ "PatternSettings.trb", TTRUE },
	{ "Common/CameraShakeCommon.trb", TTRUE },
	{ "Blob_FX/FXParticles.trb", TTRUE },
	{ "Entities/BlobProperties.trb", TTRUE },
	{ "Sound/SoundBreakPoints.trb", TTRUE },
	{ "envfx/default.trb", TTRUE },
	{ "envfx/defaultMP.trb", TTRUE },
	{ "RegionRuntimeData.trb", TFALSE },
	{ "NavMarkers.trb", TFALSE },
	{ "ZJumpData.trb", TFALSE },
	{ "Waypoints.trb", TFALSE },
	{ "NEDCamera.trb", TFALSE },
	{ "SSCamera.trb", TFALSE },
	{ "CameraShake.trb", TFALSE },
	{ "PointData.trb", TFALSE },
	{ "SoundEmitter.trb", TFALSE },
	{ "PaintableBlockData.trb", TFALSE },
	{ "Network.trb", TFALSE },
	{ "entities.trb", TFALSE },
	{ "terrain.trb", TFALSE },
	{ "ChallengeData.trb", TFALSE },
	{ "VOMapping.trb", TFALSE },
	{ "fxdata.trb", TFALSE },
	{ "MP_data.trb", TFALSE },
};

// $deBlob: FUNCTION 0056d500
ACurrentLevelInformation::ACurrentLevelInformation( ALevelInformation::LevelProperties* a_pLevelProperties )
{
	m_pLevelProperties = a_pLevelProperties;

	TASSERT( !sm_singleton );
	sm_singleton = this;

	TUtil::MemClear( m_datablocks, sizeof( m_datablocks ) );
	TUtil::MemClear( m_streamingJobs, sizeof( m_streamingJobs ) );
}

// $deBlob: FUNCTION 0056d570
// $deBlob: FUNCTION 005c3ba0
ACurrentLevelInformation::~ACurrentLevelInformation()
{
	TASSERT( sm_singleton == this );
	sm_singleton = TNULL;
}

// $deBlob: FUNCTION 0056d5b0
void ACurrentLevelInformation::Create( TBOOL a_bMultiplayer )
{
	AAssetStreaming* pAssetStreaming = AAssetStreaming::GetSingleton();
	TCHAR*           szFileName      = TStringManager::GetTempString8();

	for ( TINT i = 0; i < DataBlockCount; i++ )
	{
		if ( i == DataBlock_MPData && !a_bMultiplayer )
			continue;

		GetDataBlockPath( i, szFileName, 1024 );

		TASSERT( !m_streamingJobs[ i ] );
		TASSERT( !m_datablocks[ i ] );

		m_datablocks[ i ]    = new TTRB();
		m_streamingJobs[ i ] = pAssetStreaming->GetFreeJob();
		m_streamingJobs[ i ]->Init( m_datablocks[ i ], szFileName );
		pAssetStreaming->AddMainThreadJob2( m_streamingJobs[ i ] );
	}
}

// $deBlob: FUNCTION 0056d750
void ACurrentLevelInformation::Destroy()
{
	// NOTE: retail releases the finished jobs inside the assert (until it fired once)
	TBOOL bStreaming = IsStreaming();
	TASSERT( !bStreaming );

	for ( TINT i = 0; i < DataBlockCount; i++ )
	{
		if ( m_datablocks[ i ] != TNULL )
		{
			m_datablocks[ i ]->Close();
			delete m_datablocks[ i ];
			m_datablocks[ i ] = TNULL;
		}
	}
}

// $deBlob: FUNCTION 0056d7f0
TBOOL ACurrentLevelInformation::IsStreaming()
{
	for ( TINT i = 0; i < DataBlockCount; i++ )
	{
		ATRBLoaderJob* pJob = m_streamingJobs[ i ];

		if ( pJob != TNULL )
		{
			if ( !pJob->m_bFinished )
				return TTRUE;

			AAssetStreaming::GetSingleton()->ReleaseJob( pJob );
			m_streamingJobs[ i ] = TNULL;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0056d830
TTRB* ACurrentLevelInformation::Stream( TINT a_type )
{
	// NOTE: retail releases the finished jobs inside the assert (until it fired once)
	TBOOL bStreaming = IsStreaming();
	TASSERT( !bStreaming );

	TASSERT( a_type >= 0 && a_type < DataBlockCount );
	return m_datablocks[ a_type ];
}

// $deBlob: FUNCTION 0056d8f0
const TCHAR* ACurrentLevelInformation::GetDataBlockFileName( TINT a_type )
{
	TASSERT( a_type >= 0 && a_type < DataBlockCount );
	return sm_aDataBlockFiles[ a_type ].m_szFileName;
}

// $deBlob: FUNCTION 0056d950
void ACurrentLevelInformation::GetDataBlockPath( TINT a_type, TCHAR* a_szPath, TINT a_iSize )
{
	TASSERT( a_type >= 0 && a_type < DataBlockCount );

	if ( sm_aDataBlockFiles[ a_type ].m_bCommon )
		TStringManager::String8Format( a_szPath, a_iSize, "data/%s", sm_aDataBlockFiles[ a_type ].m_szFileName );
	else
		TStringManager::String8Format( a_szPath, a_iSize, "data/%s/%s", m_pLevelProperties->m_directory, sm_aDataBlockFiles[ a_type ].m_szFileName );
}
