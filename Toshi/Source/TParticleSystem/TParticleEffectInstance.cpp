#include "ToshiPCH.h"
#include "TParticleEffectInstance.h"
#include "TGenericParticles.h"
#include "TParticleEffect.h"
#include "TParticleEffectManager.h"
#include "TParticleSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TMatrix44                TParticleEffectInstance::s_mFinalTransform;
TParticleEffectSystem*   TParticleEffectInstance::s_pActiveEffectSys;
TParticleEffectInstance* TParticleEffectInstance::s_pActiveEffectInstance;

// The instance lets the particles die (they're not emitted again nor kept alive)
static void GenericParticles_ClearOwnerFlags( TParticles* a_pParticles )
{
	TSTATICCAST( TGenericParticles, a_pParticles )->m_uiFlags &= ~( TGenericParticles::FLAGS_REEMIT | TGenericParticles::FLAGS_KEEPALIVE );
}

// $deBlob: FUNCTION 00711a70
TParticleEffectInstance::TParticleEffectInstance()
{
	m_pEffect = TNULL;
	m_oTransform.Identity();
}

// $deBlob: FUNCTION 00711a90
void TParticleEffectInstance::Init( TParticleEffect* a_pEffect )
{
	m_fnFinished = TNULL;
	m_uiFlags    = 0;
	m_oTransform.Identity();
	m_pEffect = a_pEffect;
	TUtil::MemClear( m_afnUpdateCallbacks, sizeof( m_afnUpdateCallbacks ) );
	TUtil::MemClear( m_apUpdateUserData, sizeof( m_apUpdateUserData ) );

	TASSERT( m_pEffect->GetNumSystems() < s_iMaxSettings );

	for ( TINT i = 0; i < m_pEffect->GetNumSystems(); i++ )
	{
		m_aSystems[ i ].m_pParticles = TNULL;
		m_aSystems[ i ].m_uiFlags    = 0;
		TParticleKeyframeTimeline::InitState( m_aSystems[ i ].m_oState );
	}
}

// $deBlob: FUNCTION 00711b50
TBOOL TParticleEffectInstance::Update( TFLOAT a_fDeltaTime )
{
	TBOOL bAlive            = TFALSE;
	s_pActiveEffectInstance = this;

	TASSERT( m_pEffect->GetNumSystems() < s_iMaxSettings );

	for ( TINT i = 0; i < m_pEffect->GetNumSystems(); i++ )
	{
		System& rSystem = m_aSystems[ i ];

		if ( m_afnUpdateCallbacks[ i ] != TNULL && rSystem.m_pParticles != TNULL )
			m_afnUpdateCallbacks[ i ]( rSystem.m_pParticles, m_apUpdateUserData[ i ] );

		if ( !( rSystem.m_uiFlags & SYSTEMFLAGS_FINISHED ) )
		{
			s_pActiveEffectSys = &m_pEffect->GetSystem( i );
			s_mFinalTransform.Multiply( m_oTransform, *s_pActiveEffectSys->m_pTransform );

			if ( s_pActiveEffectSys->m_pKeys->Update( a_fDeltaTime, rSystem.m_oState ) == TTRUE )
			{
				bAlive = TTRUE;
			}
			else
			{
				if ( rSystem.m_pParticles != TNULL )
				{
					GenericParticles_ClearOwnerFlags( rSystem.m_pParticles );
					TParticles* pParticles = rSystem.m_pParticles;
					rSystem.m_pParticles   = TNULL;

					if ( pParticles->m_iNumRenderingParticles == 0 )
						TParticleSystem::GetSingleton()->StopParticles( pParticles );
				}

				rSystem.m_uiFlags |= SYSTEMFLAGS_FINISHED;

				if ( rSystem.m_uiFlags & SYSTEMFLAGS_PAUSED )
					bAlive = TTRUE;
			}
		}
		else if ( rSystem.m_uiFlags & SYSTEMFLAGS_PAUSED )
		{
			bAlive = TTRUE;
		}
	}

	s_pActiveEffectInstance = TNULL;

	if ( !bAlive && m_fnFinished != TNULL )
	{
		m_fnFinished( this, m_pFinishedUserData );
		m_fnFinished = TNULL;
	}

	return bAlive;
}

// $deBlob: FUNCTION 00711d00
void TParticleEffectInstance::SetParticleSystemUpdateCallback( TINT a_iSystem, t_UpdateCallback a_fnCallback, void* a_pUserData )
{
	TASSERT( a_iSystem >= 0 );
	TASSERT( a_iSystem < m_pEffect->GetNumSystems() );
	TASSERT( a_iSystem < s_iMaxSettings );

	m_afnUpdateCallbacks[ a_iSystem ] = a_fnCallback;
	m_apUpdateUserData[ a_iSystem ]   = a_pUserData;
}

// $deBlob: FUNCTION 00711dc0
void TParticleEffectInstance::Stop()
{
	for ( TINT i = 0; i < m_pEffect->GetNumSystems(); i++ )
	{
		System& rSystem = m_aSystems[ i ];

		if ( rSystem.m_pParticles != TNULL )
		{
			GenericParticles_ClearOwnerFlags( rSystem.m_pParticles );
			TParticles* pParticles = rSystem.m_pParticles;

			if ( pParticles->m_iNumRenderingParticles != 0 )
				continue;

			rSystem.m_pParticles = TNULL;
			TParticleSystem::GetSingleton()->StopParticles( pParticles );
		}

		rSystem.m_uiFlags |= SYSTEMFLAGS_FINISHED;
	}
}

// $deBlob: FUNCTION 00711e20
void TParticleEffectInstance::StopLooping()
{
	for ( TINT i = 0; i < m_pEffect->GetNumSystems(); i++ )
		m_aSystems[ i ].m_oState.m_iLoops = 0;
}

// $deBlob: FUNCTION 00711e50
void TParticleEffectInstance::StopInstant()
{
	for ( TINT i = 0; i < m_pEffect->GetNumSystems(); i++ )
	{
		System& rSystem = m_aSystems[ i ];

		if ( TParticles* pParticles = rSystem.m_pParticles )
		{
			rSystem.m_pParticles = TNULL;
			TParticleSystem::GetSingleton()->StopParticles( pParticles );
		}

		rSystem.m_uiFlags |= SYSTEMFLAGS_FINISHED;
	}

	if ( m_fnFinished != TNULL )
	{
		m_fnFinished( this, m_pFinishedUserData );
		m_fnFinished = TNULL;
	}

	TParticleSystem::GetSingleton()->GetEffectManager()->RemoveInstanceFromUsed( this );
}

// $deBlob: FUNCTION 00711ed0
void TParticleEffectInstance::SetPaused( TBOOL a_bPaused )
{
	if ( a_bPaused == ( ( m_uiFlags >> 1 ) & 1 ) )
		return;

	if ( a_bPaused )
	{
		for ( TINT i = 0; i < m_pEffect->GetNumSystems(); i++ )
		{
			m_aSystems[ i ].m_uiFlags |= SYSTEMFLAGS_PAUSED;
			m_aSystems[ i ].m_oState.m_iLoops = 0;
		}

		m_uiFlags |= FLAGS_PAUSED;
		return;
	}

	for ( TINT i = 0; i < m_pEffect->GetNumSystems(); i++ )
	{
		System& rSystem = m_aSystems[ i ];

		if ( rSystem.m_oState.m_iCurKey < 0 )
		{
			rSystem.m_pParticles = TNULL;
			rSystem.m_uiFlags    = 0;
			TParticleKeyframeTimeline::InitState( rSystem.m_oState );
		}
		else
		{
			rSystem.m_oState.m_iLoops = 1;
		}

		rSystem.m_uiFlags &= ~( SYSTEMFLAGS_FINISHED | SYSTEMFLAGS_PAUSED );
	}

	m_uiFlags &= ~FLAGS_PAUSED;
}

// $deBlob: FUNCTION 00711fd0
void TParticleEffectInstance::GenericParticleFinished( TParticles* a_pParticles, TParticleEffectInstance* a_pInstance )
{
	for ( TINT i = 0; i < a_pInstance->m_pEffect->GetNumSystems(); i++ )
	{
		if ( a_pParticles == a_pInstance->m_aSystems[ i ].m_pParticles )
		{
			a_pInstance->m_aSystems[ i ].m_uiFlags |= SYSTEMFLAGS_FINISHED;
			a_pInstance->m_aSystems[ i ].m_pParticles = TNULL;
		}
	}
}

TOSHI_NAMESPACE_END
