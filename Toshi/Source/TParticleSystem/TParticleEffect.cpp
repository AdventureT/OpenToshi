#include "ToshiPCH.h"
#include "TParticleEffect.h"
#include "TParticleSystem.h"
#include "TParticleEffectManager.h"
#include "TParticleKeyframe.h"
#include "TGenericParticlesSettings.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00711900
// $deBlob: FUNCTION 00711500
TParticleEffectSystem::~TParticleEffectSystem()
{
	TParticleSystem::GetSingleton()->StopAllGenericParticles( m_pSettings );
	delete m_pTransform;

	if ( m_pSettings )
		delete m_pSettings;

	if ( m_pKeys )
		delete m_pKeys;
}

// $deBlob: FUNCTION 00711550
void TParticleEffectSystem::ResolveSettings()
{
	if ( m_pSettings == TNULL )
		m_pSettings = TParticleSystem::GetSingleton()->GetEffectManager()->FindSetting( m_szSettings );
}

// $deBlob: FUNCTION 00711570
void TParticleEffectSystem::Copy( const TParticleEffectSystem& a_rcOther )
{
	m_pTransform  = new TMatrix44;
	*m_pTransform = *a_rcOther.m_pTransform;

	m_pSettings = new TGenericParticlesSettings( *a_rcOther.m_pSettings );
	m_pKeys     = new TParticleKeyframeTimeline( *a_rcOther.m_pKeys );

	m_szSettings = m_pSettings->GetName();
	m_uiFlags    = a_rcOther.m_uiFlags;

	for ( TINT i = 0; i < m_pKeys->GetNumKeyframes(); i++ )
	{
		TASSERT( m_pKeys->GetKeyframe( i )->GetType() == TParticleKeyframe::KEYFRAMETYPE_PARTICLESYSTEM && "m_apSettingsKeys->m_aLoopKeyframes[i]->GetType() == TKeyframeParticles::KEYFRAMETYPE_PARTICLESYSTEM" );
		m_pKeys->GetKeyframe( i )->GetData<TParticleKeyframe::ParticleSystemData>()->m_pSettings = m_pSettings;
	}
}

// $deBlob: FUNCTION 00711670
// $deBlob: FUNCTION 0057b1e0
TParticleEffect::~TParticleEffect()
{
	TParticleSystem::GetSingleton()->GetEffectManager()->StopAllEffects( this );

	if ( m_pSystems )
		delete[] m_pSystems;
}

// $deBlob: FUNCTION 007116a0
void TParticleEffect::Create()
{
	TASSERT( m_uiVer == s_LatestVer );

	for ( TINT16 i = 0; i < m_iNumSystems; i++ )
	{
		TParticleEffectSystem& rSystem = GetSystem( i );
		rSystem.ResolveSettings();
		rSystem.m_pKeys->Link();

		TParticleKeyframeTimeline* pKeys = rSystem.m_pKeys;

		if ( pKeys->GetNumKeyframes() > 0 )
			pKeys->GetKeyframe( 0 )->GetData<TParticleKeyframe::ParticleSystemData>()->m_pSettings = rSystem.m_pSettings;

		for ( TINT j = 1; j < pKeys->GetNumKeyframes(); j++ )
		{
			TParticleKeyframe::ParticleSystemData* pData = pKeys->GetKeyframe( j )->GetData<TParticleKeyframe::ParticleSystemData>();
			pData->m_pPrevKey                   = pKeys->GetKeyframe( j - 1 );
			pData->m_pSettings                  = rSystem.m_pSettings;
		}
	}
}

// $deBlob: FUNCTION 007117c0
TParticleEffect* TParticleEffect::Copy() const
{
	TParticleEffect* pCopy = new TParticleEffect;
	pCopy->m_uiVer         = m_uiVer;
	pCopy->m_szName        = m_szName;
	pCopy->m_iNumSystems   = m_iNumSystems;
	pCopy->m_iStealable    = m_iStealable;
	pCopy->m_uiUnk18       = m_uiUnk18;
	pCopy->m_pSystems      = new TParticleEffectSystem[ m_iNumSystems ];

	for ( TINT i = 0; i < m_iNumSystems; i++ )
		pCopy->m_pSystems[ i ].Copy( m_pSystems[ i ] );

	pCopy->Create();
	return pCopy;
}

// $deBlob: FUNCTION 00711880
TINT TParticleEffect::FindSystem( const TCHAR* a_szName ) const
{
	for ( TINT i = 0; i < m_iNumSystems; i++ )
	{
		if ( TStringManager::String8Compare( GetSystem( i ).m_szSettings, a_szName ) == 0 )
			return i;
	}

	return -1;
}

TOSHI_NAMESPACE_END
