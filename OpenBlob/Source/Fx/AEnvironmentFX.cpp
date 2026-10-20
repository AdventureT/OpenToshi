#include "pch.h"
#include "Entities/AEntityFactory.h"
#include "AEnvironmentFX.h"
#include "AParticleEffectHelper.h"
#include "AAssetStreaming.h"
#include "ACurrentLevelInformation.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"
#include "Render/Sky/ASky_DX11.h"
#include "Terrain2/ATerrain2Interface.h"
#include "Platform/DX11/ASkinShader_DX11.h"

#include <File/TTRB.h>
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffect.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <Platform/DX11/TWorldShader_DX11.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005b67b0
void AEnvFXColours::Lerp( const AEnvFXColours& a_rTarget, TFLOAT a_fBlend )
{
	m_vColour2.Lerp4( a_rTarget.m_vColour2, a_fBlend );
	m_vColour1.Lerp4( a_rTarget.m_vColour1, a_fBlend );
	m_vUnk30.Lerp4( a_rTarget.m_vUnk30, a_fBlend );
	m_vUnk10.Lerp4( a_rTarget.m_vUnk10, a_fBlend );
}

// The particle effect follows the camera
static void SetEffectTransform( TParticleEffectInstance* a_pInstance )
{
	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	TMatrix44 mTransform                           = TMatrix44::IDENTITY;
	mTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
	a_pInstance->GetTransform()                    = mTransform;
}

// $deBlob: FUNCTION 005b6830
AEnvironmentFX::AEnvironmentFX()
{
	m_bSkyEnabled       = TTRUE;
	m_pSettingsJob      = TNULL;
	m_pSettingsTRB      = TNULL;
	m_bOwnsSettingsTRB  = TFALSE;
	m_ppSettings        = TNULL;
	m_iNumSettings      = 0;
	m_iCurrentSettings  = 0;
	m_fBlendFrom        = 0.0f;
	m_fBlendTo          = 0.0f;
	m_bBlending         = TFALSE;
	m_fBlendTime        = 0.0f;
	m_fBlendDuration    = 0.0f;
	m_pCurrentSettings  = TNULL;
	m_bSkySpace         = TFALSE;
	m_bHasSkySpace      = TFALSE;
	m_fSkySpaceDuration = 0.0f;
	m_fSkySpaceTime     = 0.0f;
	m_pSky              = TNULL;
	m_pEffect1          = TNULL;
	m_pEffect2          = TNULL;
}

// $deBlob: FUNCTION 005b68f0
// $deBlob: FUNCTION 005c3bd0
AEnvironmentFX::~AEnvironmentFX()
{
	if ( m_pSky )
	{
		delete m_pSky;
		m_pSky = TNULL;
	}

	if ( m_pCurrentSettings )
	{
		delete m_pCurrentSettings;
		m_pCurrentSettings = TNULL;
	}

	if ( m_pEffect1 )
	{
		m_pEffect1->StopInstant();
		m_pEffect1 = TNULL;
	}

	if ( m_pEffect2 )
	{
		m_pEffect2->StopInstant();
		m_pEffect2 = TNULL;
	}

	UnloadSettings();
}

// $deBlob: FUNCTION 005b69e0
void AEnvironmentFX::CreateSky()
{
	m_pCurrentSettings = new Settings;

	TASSERT( m_pSky == TNULL );

	if ( m_bSkyEnabled )
		m_pSky = new ASky();
}

// $deBlob: FUNCTION 005b6aa0
void AEnvironmentFX::Reset()
{
	m_iCurrentSettings  = 0;
	m_fBlendFrom        = 0.0f;
	m_fBlendTo          = 0.0f;
	m_bBlending         = TFALSE;
	m_fBlendTime        = 0.0f;
	m_fBlendDuration    = 0.0f;
	m_bSkySpace         = TFALSE;
	m_fSkySpaceDuration = 0.0f;
	m_fSkySpaceTime     = 0.0f;

	if ( m_ppSettings != TNULL && m_pCurrentSettings != TNULL )
	{
		memcpy( m_pCurrentSettings, m_ppSettings[ 0 ], sizeof( Settings ) );
		ApplySettings();
	}

	if ( m_pSky )
		m_pSky->Reset();

	if ( m_pEffect1 )
	{
		m_pEffect1->StopInstant();
		m_pEffect1 = TNULL;
	}

	if ( m_pEffect2 )
	{
		m_pEffect2->StopInstant();
		m_pEffect2 = TNULL;
	}
}

// $deBlob: FUNCTION 005b6b40
void AEnvironmentFX::LoadSettings( const TCHAR* a_szFileName, TBOOL a_bDataBlock )
{
	TASSERT( ( TNULL == m_pSettingsTRB ) && ( TNULL == m_pSettingsJob ) );

	TINT iDataBlock;

	if ( TStringManager::String8CompareNoCase( a_szFileName, ACurrentLevelInformation::GetDataBlockFileName( ACurrentLevelInformation::DataBlock_EnvFXDefaultMP ) ) == 0 )
	{
		iDataBlock = ACurrentLevelInformation::DataBlock_EnvFXDefaultMP;
	}
	else if ( TStringManager::String8CompareNoCase( a_szFileName, ACurrentLevelInformation::GetDataBlockFileName( ACurrentLevelInformation::DataBlock_EnvFXDefault ) ) == 0 )
	{
		iDataBlock = ACurrentLevelInformation::DataBlock_EnvFXDefault;
	}
	else
	{
		// NOTE: a_bDataBlock isn't used, only the name says whether it's a data block
		m_pSettingsJob = AAssetStreaming::GetSingleton()->GetFreeJob();
		m_pSettingsJob->Init( new TTRB(), a_szFileName );
		AAssetStreaming::GetSingleton()->AddMainThreadJob2( m_pSettingsJob );
		m_bOwnsSettingsTRB = TTRUE;
		return;
	}

	m_pSettingsTRB     = ACurrentLevelInformation::GetSingleton()->Stream( iDataBlock );
	m_bOwnsSettingsTRB = TFALSE;
	OnSettingsLoaded();
}

// $deBlob: FUNCTION 005b6c70
TBOOL AEnvironmentFX::UpdateLoading()
{
	ATRBLoaderJob* pJob = m_pSettingsJob;

	if ( pJob == TNULL )
		return TTRUE;

	if ( pJob->m_bFinished )
	{
		m_pSettingsTRB = pJob->m_pTRB;
		AAssetStreaming::GetSingleton()->ReleaseJob( pJob );
		m_pSettingsJob = TNULL;

		return OnSettingsLoaded();
	}

	return TFALSE;
}

// $deBlob: FUNCTION 005b6cb0
void AEnvironmentFX::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bHasSkySpace && m_bSkySpace )
	{
		// Blends to the second settings while the sky goes to space
		m_fSkySpaceTime -= a_fDeltaTime;

		if ( m_fSkySpaceTime <= 0.0f )
		{
			m_fSkySpaceTime = 0.0f;
			m_bSkySpace     = TFALSE;
		}

		TFLOAT fProgress = m_fSkySpaceTime / m_fSkySpaceDuration;
		TFLOAT fBlend    = ( 1.0f - fProgress ) * ( 1.0f - fProgress );
		TMath::Clip( fBlend, 0.0f, 1.0f );

		if ( m_iNumSettings > 0 )
		{
			if ( m_iNumSettings == 1 )
				memcpy( m_pCurrentSettings, m_ppSettings[ 0 ], sizeof( Settings ) );
			else
				BlendSettings( m_pCurrentSettings, m_ppSettings[ 0 ], m_ppSettings[ 1 ], fBlend );

			m_bBlending      = TTRUE;
			m_fBlendTime     = 0.0f;
			m_fBlendDuration = 0.0f;
			m_fBlendFrom     = m_fBlendTo;
			m_fBlendTo       = fBlend;
		}
	}

	if ( m_bBlending )
	{
		m_fBlendTime += a_fDeltaTime;

		if ( m_fBlendTime >= m_fBlendDuration )
		{
			m_fBlendTime = m_fBlendDuration;
			m_bBlending  = TFALSE;
		}

		ApplySettings();
	}

	if ( m_pEffect1 )
		SetEffectTransform( m_pEffect1 );

	if ( m_pEffect2 )
		SetEffectTransform( m_pEffect2 );

	if ( m_pSky )
		m_pSky->Update( a_fDeltaTime );
}

// $deBlob: FUNCTION 005b6e90
void AEnvironmentFX::Render( void* a_pCamera )
{
	if ( m_pSky )
		m_pSky->Render( TSTATICCAST( ACamera, a_pCamera ) );
}

// $deBlob: FUNCTION 005b6eb0
TVector4* AEnvironmentFX::GetLightingScale( TVector4* a_pOut, TFLOAT a_fBlend )
{
	a_pOut->Lerp4( TWorldShaderHAL::s_vLightingScale5, TWorldShaderHAL::s_vLightingScale6, a_fBlend );
	return a_pOut;
}

// $deBlob: FUNCTION 005b6ee0
void AEnvironmentFX::SetBlend( TFLOAT a_fBlend, TFLOAT a_fDuration, TBOOL a_bForce )
{
	if ( m_bHasSkySpace && !a_bForce )
		return;

	if ( m_iNumSettings > 0 )
	{
		if ( m_iNumSettings == 1 )
			memcpy( m_pCurrentSettings, m_ppSettings[ 0 ], sizeof( Settings ) );
		else
			BlendSettings( m_pCurrentSettings, m_ppSettings[ 0 ], m_ppSettings[ 1 ], a_fBlend );

		m_fBlendDuration = a_fDuration;
		m_bBlending      = TTRUE;
		m_fBlendTime     = 0.0f;
		m_fBlendFrom     = m_fBlendTo;
		m_fBlendTo       = a_fBlend;
	}
}

// $deBlob: FUNCTION 005b6f60
void AEnvironmentFX::StartSkySpace()
{
	if ( !m_bHasSkySpace || m_bSkySpace )
		return;

	SkySpace* pSkySpace = TSTATICCAST( SkySpace, m_pSettingsTRB->GetSymbolAddress( "skyspace" ) );

	if ( pSkySpace == TNULL )
		return;

	m_iCurrentSettings  = 0;
	m_fBlendFrom        = 0.0f;
	m_fBlendTo          = 0.0f;
	m_bBlending         = TFALSE;
	m_fBlendTime        = 0.0f;
	m_fBlendDuration    = 0.0f;
	m_fSkySpaceDuration = pSkySpace->m_fDuration;
	m_fSkySpaceTime     = pSkySpace->m_fDuration;

	if ( m_pSky )
		m_pSky->StartSkySpace( pSkySpace->m_fCloudSpeed, pSkySpace->m_fCloudDescendTime, pSkySpace->m_fStarsHeight, pSkySpace->m_fStarsSpeed, pSkySpace->m_fStarsTime );

	m_bSkySpace = TTRUE;
}

// $deBlob: FUNCTION 005b6fe0
void AEnvironmentFX::UnloadSettings()
{
	if ( m_pSettingsJob )
	{
		AAssetStreaming::GetSingleton()->ReleaseJob( m_pSettingsJob );
		m_pSettingsJob = TNULL;
	}

	if ( m_pSettingsTRB )
	{
		ReleaseSettings( m_pSettingsTRB );

		if ( m_bOwnsSettingsTRB && m_pSettingsTRB )
			delete m_pSettingsTRB;

		m_bOwnsSettingsTRB = TFALSE;
		m_pSettingsTRB     = TNULL;
	}

	m_ppSettings       = TNULL;
	m_iNumSettings     = 0;
	m_iCurrentSettings = 0;
	m_fBlendFrom       = 0.0f;
	m_fBlendTo         = 0.0f;
	m_bBlending        = TFALSE;
}

// $deBlob: FUNCTION 005b7060
TBOOL AEnvironmentFX::OnSettingsLoaded()
{
	m_ppSettings = TSTATICCAST( Settings*, m_pSettingsTRB->GetSymbolAddress( "settings" ) );

	if ( m_ppSettings == TNULL )
		return TFALSE;

	m_iNumSettings     = InitSettings( m_pSettingsTRB );
	m_iCurrentSettings = 0;
	m_fBlendFrom       = 0.0f;
	m_fBlendTo         = 0.0f;
	m_bBlending        = TFALSE;
	m_fBlendTime       = 0.0f;
	m_fBlendDuration   = 0.0f;
	m_bHasSkySpace     = TFALSE;

	if ( m_pSettingsTRB->GetSymbolAddress( "skyspace" ) != TNULL )
	{
		m_bHasSkySpace = TTRUE;

		if ( m_pSky )
			m_pSky->CreateStars();
	}

	if ( m_iNumSettings > 0 && m_pCurrentSettings != TNULL )
	{
		memcpy( m_pCurrentSettings, m_ppSettings[ m_iCurrentSettings ], sizeof( Settings ) );
		ApplySettings();
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005b7100
TBOOL AEnvironmentFX::ReleaseSettings( TTRB* a_pTRB )
{
	if ( a_pTRB != TNULL )
	{
		Settings** ppSettings = TSTATICCAST( Settings*, a_pTRB->GetSymbolAddress( "settings" ) );

		if ( ppSettings == TNULL )
			return TFALSE;

		for ( ; *ppSettings != TNULL; ppSettings++ )
		{
			( *ppSettings )->m_pEffect = TNULL;

			if ( ( *ppSettings )->m_pEffectCopy )
			{
				delete ( *ppSettings )->m_pEffectCopy;
				( *ppSettings )->m_pEffectCopy = TNULL;
			}
		}
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005b7180
TINT AEnvironmentFX::InitSettings( TTRB* a_pTRB )
{
	Settings** ppSettings = TSTATICCAST( Settings*, a_pTRB->GetSymbolAddress( "settings" ) );

	if ( ppSettings == TNULL )
		return 0;

	TINT iCount = 0;

	if ( *ppSettings != TNULL )
	{
		const TFLOAT fScale = 1.0f / 255.0f;

		do
		{
			// The colours are 0-255 in the file
			Settings* pSettings = *ppSettings;
			pSettings->m_vBackgroundColour.Multiply4( fScale );
			pSettings->m_avUnk060[ 3 ].Multiply4( fScale );
			pSettings->m_avUnk060[ 5 ].Multiply4( fScale );
			pSettings->m_avUnk060[ 4 ].Multiply4( fScale );
			pSettings->m_avUnk060[ 6 ].Multiply4( fScale );
			pSettings->m_avUnk060[ 7 ].Multiply4( fScale );
			pSettings->m_avUnk060[ 9 ].Multiply4( fScale );
			pSettings->m_avUnk060[ 8 ].Multiply4( fScale );
			pSettings->m_avUnk060[ 10 ].Multiply4( fScale );
			pSettings->m_oSkyColours.m_vColour1.Multiply4( fScale );
			pSettings->m_oSkyColours.m_vColour2.Multiply4( fScale );
			pSettings->m_aCloudColours[ 0 ].m_vColour1.Multiply4( fScale );
			pSettings->m_aCloudColours[ 0 ].m_vColour2.Multiply4( fScale );
			pSettings->m_aCloudColours[ 1 ].m_vColour1.Multiply4( fScale );
			pSettings->m_aCloudColours[ 1 ].m_vColour2.Multiply4( fScale );
			pSettings->m_aCloudColours[ 2 ].m_vColour1.Multiply4( fScale );
			pSettings->m_aCloudColours[ 2 ].m_vColour2.Multiply4( fScale );
			pSettings->m_vLightingScale5.Multiply4( fScale );
			pSettings->m_vLightingScale6.Multiply4( fScale );

			if ( pSettings->m_szEffect != TNULL )
			{
				TParticleEffect* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( pSettings->m_szEffect );
				TASSERT( pEffect, "Couldn't find particle effect: %s" );

				pSettings->m_pEffect = pEffect;

				if ( pEffect != TNULL )
					pSettings->m_pEffectCopy = pEffect->Copy();
			}

			ppSettings++;
			iCount++;
		} while ( *ppSettings != TNULL );
	}

	return iCount;
}

// $deBlob: FUNCTION 005b73d0
TINT AEnvironmentFX::CountSettings( TTRB* a_pTRB )
{
	// NOTE: retail ignores a_pTRB and counts the entries of m_pSettingsTRB
	Settings** ppSettings = TSTATICCAST( Settings*, m_pSettingsTRB->GetSymbolAddress( "settings" ) );

	if ( ppSettings == TNULL )
		return 0;

	TINT iCount = 0;

	while ( *ppSettings != TNULL )
	{
		ppSettings++;
		iCount++;
	}

	return iCount;
}

// $deBlob: FUNCTION 005b7400
void AEnvironmentFX::ResetBlend()
{
	m_iCurrentSettings = 0;
	m_fBlendFrom       = 0.0f;
	m_fBlendTo         = 0.0f;
	m_bBlending        = TFALSE;
	m_fBlendTime       = 0.0f;
	m_fBlendDuration   = 0.0f;
}

// $deBlob: FUNCTION 005b7430
void AEnvironmentFX::ApplySettings()
{
	if ( m_ppSettings == TNULL )
		return;

	TFLOAT fDuration = 0.0f;
	TFLOAT fTime     = 0.0f;

	if ( m_bBlending )
	{
		fDuration = m_fBlendDuration;
		fTime     = m_fBlendTime;
	}

	Settings* pSettings = m_pCurrentSettings;
	TFLOAT    fBlend    = ( fDuration == 0.0f ) ? 1.0f : fTime / fDuration;

	if ( m_pSky )
	{
		m_pSky->SetColours( fBlend, pSettings->m_oSkyColours, pSettings->m_aCloudColours[ 0 ], pSettings->m_aCloudColours[ 1 ], pSettings->m_aCloudColours[ 2 ] );
		m_pSky->SetCloudSpeed( pSettings->m_fCloudSpeed );
	}

	// The background (clear) colour is also the fog colour
	ARenderer* pRenderer = ARenderer::GetSingleton();
	TVector4   vColour   = *TREINTERPRETCAST( TVector4*, pRenderer->m_afFogColour );
	vColour.Lerp4( pSettings->m_vBackgroundColour, fBlend );

	pRenderer->SetBackgroundColour( TUINT8( TINT( vColour.x * 255.0f ) ), TUINT8( TINT( vColour.y * 255.0f ) ), TUINT8( TINT( vColour.z * 255.0f ) ) );
	pRenderer->m_bFogEnabled                                 = TTRUE;
	*TREINTERPRETCAST( TVector4*, pRenderer->m_afFogColour ) = vColour;

	TFLOAT fFogEnd        = pRenderer->m_fFogEnd;
	pRenderer->m_fFogStart = TMath::LERP( pRenderer->m_fFogStart, pSettings->m_fFogStart, fBlend );
	pRenderer->m_fFogEnd   = TMath::LERP( fFogEnd, pSettings->m_fFogEnd, fBlend );

	TWorldShaderHAL::s_vLightingScale5.Lerp4( pSettings->m_vLightingScale5, fBlend );
	ASkinShaderHAL::s_vTevReg0Scale = TWorldShaderHAL::s_vLightingScale5;
	TWorldShaderHAL::s_vLightingScale6.Lerp4( pSettings->m_vLightingScale6, fBlend );
	ASkinShaderHAL::s_vTevReg1Scale = TWorldShaderHAL::s_vLightingScale6;

	const TFLOAT* pDistances                             = pSettings->m_afInstanceLODDistances;
	ATerrain2Interface::ms_afInstanceLODDistancesSq[ 0 ] = pDistances[ 0 ] * pDistances[ 0 ];
	ATerrain2Interface::ms_afInstanceLODDistancesSq[ 1 ] = pDistances[ 3 ] * pDistances[ 3 ];
	ATerrain2Interface::ms_afInstanceLODDistancesSq[ 2 ] = pDistances[ 1 ] * pDistances[ 1 ];
	ATerrain2Interface::ms_afInstanceLODDistancesSq[ 3 ] = pDistances[ 2 ] * pDistances[ 2 ];

	AEntityFactory::SetLODDistances( pDistances[ 1 ], pDistances[ 2 ] );

	Settings* pSettings1 = m_ppSettings[ 0 ];
	Settings* pSettings2 = m_ppSettings[ 1 ];

	if ( m_pEffect1 != TNULL && m_pEffect1->GetEffect() != pSettings1->m_pEffectCopy )
	{
		m_pEffect1->Stop();
		m_pEffect1 = TNULL;
	}

	if ( m_pEffect2 != TNULL && m_pEffect2->GetEffect() != pSettings2->m_pEffectCopy )
	{
		m_pEffect2->Stop();
		m_pEffect2 = TNULL;
	}

	if ( pSettings1->m_pEffect == TNULL && pSettings2->m_pEffect == TNULL )
		return;

	// The particle counts of the two weather effects follow the blend
	TFLOAT fWeight = TMath::LERP( m_fBlendFrom, m_fBlendTo, ( fDuration == 0.0f ) ? 1.0f : fTime / fDuration );

	TParticleEffectManager* pEffectManager = TParticleSystem::GetSingleton()->GetEffectManager();

	if ( pSettings1->m_pEffect != TNULL && pSettings1->m_pEffect->GetNumSystems() > 0 )
	{
		if ( AParticleEffectHelper::ScaleNumParticles( pSettings1->m_pEffect, pSettings1->m_pEffectCopy, 1.0f - fWeight, TNULL ) >= 1 )
		{
			if ( m_pEffect1 == TNULL )
			{
				m_pEffect1 = pEffectManager->StartEffect( pSettings1->m_pEffectCopy );

				if ( m_pEffect1 != TNULL )
					SetEffectTransform( m_pEffect1 );
			}
		}
		else if ( m_pEffect1 != TNULL )
		{
			m_pEffect1->Stop();
			m_pEffect1 = TNULL;
		}
	}

	if ( pSettings2->m_pEffect != TNULL && pSettings2->m_pEffect->GetNumSystems() > 0 )
	{
		if ( AParticleEffectHelper::ScaleNumParticles( pSettings2->m_pEffect, pSettings2->m_pEffectCopy, fWeight, TNULL ) >= 1 )
		{
			if ( m_pEffect2 == TNULL )
			{
				m_pEffect2 = pEffectManager->StartEffect( pSettings2->m_pEffectCopy );

				if ( m_pEffect2 != TNULL )
					SetEffectTransform( m_pEffect2 );
			}
		}
		else if ( m_pEffect2 != TNULL )
		{
			m_pEffect2->Stop();
			m_pEffect2 = TNULL;
		}
	}
}

// $deBlob: FUNCTION 005b78a0
void AEnvironmentFX::BlendSettings( Settings* a_pOut, const Settings* a_pFrom, const Settings* a_pTo, TFLOAT a_fBlend )
{
	TASSERT( a_fBlend >= 0.0f && a_fBlend <= 1.0f );

	const TVector4* pFrom = TREINTERPRETCAST( const TVector4*, a_pFrom );
	const TVector4* pTo   = TREINTERPRETCAST( const TVector4*, a_pTo );
	TVector4*       pOut  = TREINTERPRETCAST( TVector4*, a_pOut );

	pOut[ 0 ] = pFrom[ 0 ];
	pOut[ 1 ] = pFrom[ 1 ];
	pOut[ 2 ] = pFrom[ 2 ];
	pOut[ 3 ].Lerp4( pFrom[ 3 ], pTo[ 3 ], a_fBlend );
	pOut[ 4 ].Lerp4( pFrom[ 4 ], pTo[ 4 ], a_fBlend );
	pOut[ 5 ].Lerp4( pFrom[ 5 ], pTo[ 5 ], a_fBlend );
	pOut[ 6 ] = pFrom[ 6 ];

	for ( TINT i = 7; i < 35; i++ )
		pOut[ i ].Lerp4( pFrom[ i ], pTo[ i ], a_fBlend );

	const TFLOAT* pfFrom = TREINTERPRETCAST( const TFLOAT*, a_pFrom );
	const TFLOAT* pfTo   = TREINTERPRETCAST( const TFLOAT*, a_pTo );
	TFLOAT*       pfOut  = TREINTERPRETCAST( TFLOAT*, a_pOut );

	// 0x230-0x23C
	for ( TINT i = 140; i < 144; i++ )
		pfOut[ i ] = TMath::LERP( pfFrom[ i ], pfTo[ i ], a_fBlend );

	// NOTE: 0x240-0x24C (the instance LOD distances) are left as they are in a_pOut

	// 0x250-0x270
	for ( TINT i = 148; i < 157; i++ )
		pfOut[ i ] = TMath::LERP( pfFrom[ i ], pfTo[ i ], a_fBlend );

	// NOTE: the effect (0x274-0x27C) is left as it is in a_pOut
	a_pOut->m_uiUnk280 = a_pFrom->m_uiUnk280;
	a_pOut->m_fUnk284  = TMath::LERP( a_pFrom->m_fUnk284, a_pTo->m_fUnk284, a_fBlend );
}
