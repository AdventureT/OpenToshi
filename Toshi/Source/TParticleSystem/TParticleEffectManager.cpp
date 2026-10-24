#include "ToshiPCH.h"
#include "TParticleEffectManager.h"
#include "TParticleEffect.h"
#include "TParticleEffectInstance.h"
#include "TGenericParticlesSettings.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00710da0
TParticleEffectManager::TParticleEffectManager( TINT a_iMaxInstances )
{
	m_iMaxInstances = a_iMaxInstances;
	m_pSettings     = TNULL;
	m_pEffects      = TNULL;

	TParticleEffectInstance* pInstances = TSTATICCAST( TParticleEffectInstance, TMemalign( 16, a_iMaxInstances * sizeof( TParticleEffectInstance ) ) );

	if ( pInstances == TNULL )
		pInstances = TNULL;
	else
	{
		for ( TINT i = a_iMaxInstances - 1; i >= 0; i-- )
			new ( &pInstances[ i ] ) TParticleEffectInstance();
	}

	m_pInstances      = pInstances;
	m_pFreeInstances  = TNULL;
	m_pUsedInstances  = TNULL;
	m_bStealInstances = TFALSE;
}

// $deBlob: FUNCTION 00710e20
// $deBlob: FUNCTION 0070ee20
TParticleEffectManager::~TParticleEffectManager()
{
	operator delete( m_pInstances );
}

// $deBlob: FUNCTION 00710e30
void TParticleEffectManager::Create()
{
	for ( TINT i = 0; i < m_iMaxInstances - 1; i++ )
		m_pInstances[ i ].m_pNext = &m_pInstances[ i + 1 ];

	for ( TINT i = 1; i < m_iMaxInstances; i++ )
		m_pInstances[ i ].m_pPrev = &m_pInstances[ i - 1 ];

	m_pInstances[ 0 ].m_pPrev                   = TNULL;
	m_pInstances[ m_iMaxInstances - 1 ].m_pNext = TNULL;

	m_pFreeInstances = m_pInstances;
	m_pUsedInstances = TNULL;
	m_pLastUsed      = TNULL;
}

// $deBlob: FUNCTION 00710ed0
void TParticleEffectManager::ClearInstanceLists()
{
	m_pFreeInstances = TNULL;
	m_pUsedInstances = TNULL;
}

// $deBlob: FUNCTION 00710ee0
TParticleEffect* TParticleEffectManager::FindEffect( const TCHAR* a_szName ) const
{
	for ( TParticleEffect* pEffect = m_pEffects; pEffect != TNULL; pEffect = pEffect->m_pNext )
	{
		if ( TStringManager::String8CompareNoCase( pEffect->m_szName, a_szName ) == 0 )
			return pEffect;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00710f20
TGenericParticlesSettings* TParticleEffectManager::FindSetting( const TCHAR* a_szName ) const
{
	for ( TGenericParticlesSettings* pSetting = m_pSettings; pSetting != TNULL; pSetting = pSetting->m_pNext )
	{
		if ( TStringManager::String8CompareNoCase( pSetting->m_szName, a_szName ) == 0 )
			return pSetting;
	}

	return TNULL;
}

// $deBlob: FUNCTION 00710f60
TParticleEffectInstance* TParticleEffectManager::StartEffect( TParticleEffect* a_pEffect )
{
	TParticleEffectInstance* pInstance = m_pFreeInstances;

	if ( pInstance == TNULL )
		return TNULL;

	m_pFreeInstances = pInstance->m_pNext;

	if ( m_pFreeInstances != TNULL )
		m_pFreeInstances->m_pPrev = TNULL;

	pInstance->Init( a_pEffect );
	AddToEndOfUsedList( pInstance );

	m_bStealInstances = m_bStealInstances || a_pEffect->m_iStealable != 0;
	return pInstance;
}

// $deBlob: FUNCTION 00710fd0
void TParticleEffectManager::Update( TFLOAT a_fDeltaTime )
{
	TParticleEffectInstance* pNext = m_pUsedInstances;

	while ( TParticleEffectInstance* pInstance = pNext )
	{
		pNext = pInstance->m_pNext;

		if ( !( pInstance->m_uiFlags & TParticleEffectInstance::FLAGS_STOLEN ) )
		{
			if ( !pInstance->Update( a_fDeltaTime ) )
				RemoveInstanceFromUsed( pInstance );
		}
		else
		{
			pInstance->StopInstant();
		}
	}
}

// $deBlob: FUNCTION 007110c0
TINT TParticleEffectManager::GetNumEffects() const
{
	TINT iCount = 0;

	for ( TParticleEffect* pEffect = m_pEffects; pEffect != TNULL; pEffect = pEffect->m_pNext )
		iCount++;

	return iCount;
}

// $deBlob: FUNCTION 007110e0
void TParticleEffectManager::StopAllEffects()
{
	TParticleEffectInstance* pNext = m_pUsedInstances;

	while ( TParticleEffectInstance* pInstance = pNext )
	{
		pNext = pInstance->m_pNext;
		pInstance->StopInstant();
	}

	pNext = m_pUsedInstances;

	while ( TParticleEffectInstance* pInstance = pNext )
	{
		pNext = pInstance->m_pNext;

		if ( !( pInstance->m_uiFlags & TParticleEffectInstance::FLAGS_STOLEN ) )
		{
			if ( !pInstance->Update( 0.0f ) )
				RemoveInstanceFromUsed( pInstance );
		}
		else
		{
			pInstance->StopInstant();
		}
	}

	TASSERT( TNULL == m_pUsedInstances );
}

// $deBlob: FUNCTION 00711180
void TParticleEffectManager::StopAllEffects( TParticleEffect* a_pEffect )
{
	TParticleEffectInstance* pNext = m_pUsedInstances;

	while ( TParticleEffectInstance* pInstance = pNext )
	{
		pNext = pInstance->m_pNext;

		if ( pInstance->m_pEffect == a_pEffect )
			pInstance->StopInstant();
	}

	pNext = m_pUsedInstances;

	while ( TParticleEffectInstance* pInstance = pNext )
	{
		pNext = pInstance->m_pNext;

		if ( !( pInstance->m_uiFlags & TParticleEffectInstance::FLAGS_STOLEN ) )
		{
			if ( !pInstance->Update( 0.0f ) )
				RemoveInstanceFromUsed( pInstance );
		}
		else
		{
			pInstance->StopInstant();
		}
	}
}

// $deBlob: FUNCTION 007111f0
void TParticleEffectManager::StopStealableEffectInstance()
{
	if ( !m_bStealInstances )
		return;

	for ( TParticleEffectInstance* pInstance = m_pUsedInstances; pInstance != TNULL; pInstance = pInstance->m_pNext )
	{
		if ( pInstance->m_pEffect->IsStealable() )
		{
			// TParticleEffectInstance::SetStolen (inlined)
			TASSERT( pInstance->GetEffect()->IsStealable() );
			pInstance->m_uiFlags |= TParticleEffectInstance::FLAGS_STOLEN;
			return;
		}
	}

	m_bStealInstances = TFALSE;
}

// $deBlob: FUNCTION 00711220
TINT TParticleEffectManager::GetNumInstances( TParticleEffect* a_pEffect ) const
{
	TINT iCount = 0;

	for ( TParticleEffectInstance* pInstance = m_pUsedInstances; pInstance != TNULL; pInstance = pInstance->m_pNext )
	{
		if ( pInstance->m_pEffect == a_pEffect )
			iCount++;
	}

	return iCount;
}

// $deBlob: FUNCTION 00711250
void TParticleEffectManager::RemoveInstanceFromUsed( TParticleEffectInstance* a_pInstance )
{
	if ( a_pInstance->m_pPrev != TNULL )
		a_pInstance->m_pPrev->m_pNext = a_pInstance->m_pNext;

	if ( a_pInstance->m_pNext != TNULL )
		a_pInstance->m_pNext->m_pPrev = a_pInstance->m_pPrev;

	if ( m_pUsedInstances == a_pInstance )
		m_pUsedInstances = a_pInstance->m_pNext;

	if ( m_pLastUsed == a_pInstance )
		m_pLastUsed = a_pInstance->m_pPrev;

	if ( a_pInstance == m_pFreeInstances )
	{
		// NOTE: retail handles an instance that is the head of the free list
		TParticleEffectInstance* pNextFree = m_pFreeInstances->m_pNext;

		if ( pNextFree == TNULL )
			m_pFreeInstances = TNULL;
		else
		{
			m_pFreeInstances   = pNextFree;
			pNextFree->m_pPrev = TNULL;
		}
	}

	a_pInstance->m_pNext = m_pFreeInstances;

	if ( m_pFreeInstances != TNULL )
		m_pFreeInstances->m_pPrev = a_pInstance;

	m_pFreeInstances     = a_pInstance;
	a_pInstance->m_pPrev = TNULL;
}

// $deBlob: FUNCTION 007112f0
void TParticleEffectManager::AddToEndOfUsedList( TParticleEffectInstance* a_pInstance )
{
	if ( m_pLastUsed == TNULL )
	{
		TASSERT( TNULL == m_pUsedInstances );
		a_pInstance->m_pPrev = TNULL;
		a_pInstance->m_pNext = TNULL;
		m_pUsedInstances     = a_pInstance;
		m_pLastUsed          = a_pInstance;
		return;
	}

	a_pInstance->m_pPrev  = m_pLastUsed;
	m_pLastUsed->m_pNext  = a_pInstance;
	a_pInstance->m_pNext  = TNULL;
	m_pLastUsed           = a_pInstance;
}

// $deBlob: FUNCTION 00712080
void TParticleEffectManager::ReleaseInstance( TParticleEffectInstance* a_pInstance )
{
	RemoveInstanceFromUsed( a_pInstance );
}

// $deBlob: FUNCTION 00711380
void TParticleEffectManager::AddSetting( TGenericParticlesSettings* a_pSetting )
{
	a_pSetting->Create();
	a_pSetting->m_pPrev = TNULL;
	a_pSetting->m_pNext = m_pSettings;

	if ( m_pSettings != TNULL )
		m_pSettings->m_pPrev = a_pSetting;

	m_pSettings = a_pSetting;
}

// $deBlob: FUNCTION 007113c0
void TParticleEffectManager::RemoveSetting( TGenericParticlesSettings* a_pSetting )
{
	for ( TGenericParticlesSettings* pSetting = m_pSettings; pSetting != TNULL; pSetting = pSetting->m_pNext )
	{
		if ( pSetting == a_pSetting )
		{
			if ( pSetting->m_pPrev != TNULL )
				pSetting->m_pPrev->m_pNext = pSetting->m_pNext;

			if ( pSetting->m_pNext != TNULL )
				pSetting->m_pNext->m_pPrev = pSetting->m_pPrev;

			if ( pSetting == m_pSettings )
				m_pSettings = m_pSettings->m_pNext;

			break;
		}
	}

	a_pSetting->m_pModel = T2ModelPtr( 0 );
}

// $deBlob: FUNCTION 00711430
void TParticleEffectManager::AddEffect( TParticleEffect* a_pEffect )
{
	a_pEffect->Create();
	a_pEffect->m_pPrev = TNULL;
	a_pEffect->m_pNext = m_pEffects;

	if ( m_pEffects != TNULL )
		m_pEffects->m_pPrev = a_pEffect;

	m_pEffects = a_pEffect;
}

// $deBlob: FUNCTION 00711470
void TParticleEffectManager::RemoveEffect( TParticleEffect* a_pEffect )
{
	for ( TParticleEffect* pEffect = m_pEffects; pEffect != TNULL; pEffect = pEffect->m_pNext )
	{
		if ( pEffect == a_pEffect )
		{
			if ( pEffect->m_pPrev != TNULL )
				pEffect->m_pPrev->m_pNext = pEffect->m_pNext;

			if ( pEffect->m_pNext != TNULL )
				pEffect->m_pNext->m_pPrev = pEffect->m_pPrev;

			if ( pEffect == m_pEffects )
				m_pEffects = m_pEffects->m_pNext;

			return;
		}
	}
}

TOSHI_NAMESPACE_END
