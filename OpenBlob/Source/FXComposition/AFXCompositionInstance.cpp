#include "pch.h"
#include "AFXCompositionInstance.h"
#include "Console/AStaticCVars.h"
#include "AFXCompositionManager.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"

#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005b4fc0
AFXCompositionInstance::AFXCompositionInstance()
{
	m_uiFlags      = FLAGS_FINISHED;
	m_pFXData      = TNULL;
	m_iNextCommand = 0;
	m_fTime        = 0.0f;
	m_oTransform   = TMatrix44::IDENTITY;

	for ( TINT i = 0; i < MAX_RUNNING_COMMANDS; i++ )
		m_apRunningCommands[ i ] = TNULL;

	for ( TINT i = 0; i < MAX_MODELS; i++ )
		m_apModels[ i ] = TNULL;

	for ( TINT i = 0; i < MAX_PARTICLEEFFECTS; i++ )
	{
		m_aParticleEffects[ i ].m_pEffect = TNULL;
		m_aParticleEffects[ i ].m_pSystem = TNULL;
	}

	AFXCompositionManager::GetSingleton()->AddInstance( this );
}

// $deBlob: FUNCTION 005b5140
// $deBlob: FUNCTION 0076c0b0
// $deBlob: FUNCTION 0059ce60
// $deBlob: FUNCTION 007868e0
// $deBlob: FUNCTION 00786a50
AFXCompositionInstance::~AFXCompositionInstance()
{
	Destroy();
	AFXCompositionManager::GetSingleton()->RemoveInstance( this );
}

// $deBlob: FUNCTION 005b51b0
TBOOL AFXCompositionInstance::Initialize( const TCHAR* a_szName )
{
	TASSERT( TNULL == m_pFXData );

	m_uiFlags = 0;
	m_pFXData = TSTATICCAST( AFXCompositionData, AFXCompositionManager::GetSingleton()->GetTRB()->GetSymbolAddress( a_szName ) );

	if ( m_pFXData != TNULL )
	{
		m_strName = a_szName;
		TASSERT( m_pFXData->m_pModels->m_iNumModels <= MAX_MODELS );

		for ( TINT i = 0; i < m_pFXData->m_pModels->m_iNumModels; i++ )
		{
			m_apModels[ i ] = AModelFactory::GetSingleton()->CreateInstance( m_pFXData->m_pModels->m_aszNames[ i ], TTRUE, TTRUE );
			m_apModels[ i ]->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		}

		TASSERT( m_pFXData->m_pParticleSystems->m_iNumParticles <= MAX_PARTICLEEFFECTS );

		for ( TINT i = 0; i < m_pFXData->m_pParticleSystems->m_iNumParticles; i++ )
		{
			m_aParticleEffects[ i ].m_pEffect = TNULL;

			m_aParticleEffects[ i ].m_pSystem = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( m_pFXData->m_pParticleSystems->m_aszNames[ i ] );
		}

		m_uiFlags |= FLAGS_FINISHED;
		return TTRUE;
	}

	m_aParticleEffects[ 0 ].m_pSystem = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( a_szName );

	m_strName = "";
	m_uiFlags |= FLAGS_FINISHED | FLAGS_PARTICLE_ONLY;

	return m_aParticleEffects[ 0 ].m_pSystem != TNULL;
}

// $deBlob: FUNCTION 005b5360
void AFXCompositionInstance::Destroy()
{

	if ( ( m_uiFlags & FLAGS_PARTICLE_ONLY ) && m_aParticleEffects[ 0 ].m_pEffect != TNULL )
	{
		m_aParticleEffects[ 0 ].m_pEffect->SetFinishedCallback( TNULL, TNULL );
		m_aParticleEffects[ 0 ].m_pEffect->Stop();
		m_aParticleEffects[ 0 ].m_pEffect = TNULL;
		m_aParticleEffects[ 0 ].m_pSystem = TNULL;
	}

	if ( m_pFXData != TNULL )
	{
		for ( TINT i = 0; i < m_pFXData->m_pModels->m_iNumModels; i++ )
		{
			AModelFactory::GetSingleton()->DestroyInstance( m_apModels[ i ] );
			m_apModels[ i ] = TNULL;
		}

		for ( TINT i = 0; i < m_pFXData->m_pParticleSystems->m_iNumParticles; i++ )
		{
			if ( m_aParticleEffects[ i ].m_pEffect != TNULL )
			{
					m_aParticleEffects[ i ].m_pEffect->SetFinishedCallback( TNULL, TNULL );
					m_aParticleEffects[ i ].m_pEffect->Stop();
				m_aParticleEffects[ i ].m_pEffect = TNULL;
				m_aParticleEffects[ i ].m_pSystem = TNULL;
			}
		}

		m_pFXData = TNULL;
	}

	for ( TINT i = 0; i < MAX_RUNNING_COMMANDS; i++ )
		m_apRunningCommands[ i ] = TNULL;
}

// $deBlob: FUNCTION 005b5450
void AFXCompositionInstance::TriggerEffect( const TMatrix44& a_rTransform )
{
	m_oTransform = a_rTransform;

	if ( !( m_uiFlags & FLAGS_PARTICLE_ONLY ) )
	{
		if ( AStaticCVars::cvEnableFXComposition.GetBool() )
		{
			m_uiFlags &= ~( FLAGS_FINISHED | FLAGS_STOPPING );
			m_fTime        = 0.0f;
			m_iNextCommand = 0;

			for ( TINT i = 0; i < MAX_RUNNING_COMMANDS; i++ )
				m_apRunningCommands[ i ] = TNULL;

			for ( TINT i = 0; i < m_pFXData->m_pModels->m_iNumModels; i++ )
			{
				m_apModels[ i ]->SetTransform( m_oTransform );
				m_apModels[ i ]->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
				m_apModels[ i ]->SetScale( TVector4( 1.0f, 1.0f, 1.0f, 1.0f ) );
				m_apModels[ i ]->GetModelInstance()->SetForceFade( TFALSE );
			}
		}
	}
	else
	{
		StopInstant();

		if ( m_aParticleEffects[ 0 ].m_pSystem != TNULL )
		{
			if ( AStaticCVars::cvEnableParticles.GetBool() == TTRUE )
			{
				TParticleEffectInstance* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_aParticleEffects[ 0 ].m_pSystem );

				if ( pEffect != TNULL )
				{
					pEffect->GetTransform() = m_oTransform;
					pEffect->SetFinishedCallback( TParticleEffectCommandEndCallback, this );
					m_aParticleEffects[ 0 ].m_pEffect = pEffect;
					m_uiFlags &= ~FLAGS_FINISHED;
					return;
				}
			}
		}
	}
}

// $deBlob: FUNCTION 005b5630
void AFXCompositionInstance::SetTransform( const TMatrix44& a_rTransform )
{
	m_oTransform = a_rTransform;

	if ( ( m_uiFlags & FLAGS_PARTICLE_ONLY ) && m_aParticleEffects[ 0 ].m_pEffect != TNULL )
	{
		m_aParticleEffects[ 0 ].m_pEffect->GetTransform() = a_rTransform;
	}
}

// $deBlob: FUNCTION 005b56a0
void AFXCompositionInstance::Stop()
{

	if ( m_uiFlags & FLAGS_PARTICLE_ONLY )
	{
		if ( m_aParticleEffects[ 0 ].m_pEffect != TNULL )
		{
			m_aParticleEffects[ 0 ].m_pEffect->SetFinishedCallback( TNULL, TNULL );
			m_aParticleEffects[ 0 ].m_pEffect->Stop();
			m_aParticleEffects[ 0 ].m_pEffect = TNULL;
		}

		m_uiFlags |= FLAGS_FINISHED;
		return;
	}

	m_uiFlags |= FLAGS_STOPPING;
}

// $deBlob: FUNCTION 005b56e0
void AFXCompositionInstance::StopInterrupt()
{
	Stop();
	SetFinished();
}

// $deBlob: FUNCTION 005b5730
void AFXCompositionInstance::StopInstant()
{

	if ( !( m_uiFlags & FLAGS_PARTICLE_ONLY ) )
	{
		for ( TINT i = 0; i < m_pFXData->m_pParticleSystems->m_iNumParticles; i++ )
		{
			if ( m_aParticleEffects[ i ].m_pEffect != TNULL )
			{
					m_aParticleEffects[ i ].m_pEffect->StopInstant();
				m_aParticleEffects[ i ].m_pEffect = TNULL;
			}
		}

		for ( TINT i = 0; i < m_pFXData->m_pModels->m_iNumModels; i++ )
			m_apModels[ i ]->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

		for ( TINT i = 0; i < MAX_RUNNING_COMMANDS; i++ )
			m_apRunningCommands[ i ] = TNULL;
	}
	else if ( m_aParticleEffects[ 0 ].m_pEffect != TNULL )
	{
		m_aParticleEffects[ 0 ].m_pEffect->StopInstant();
		m_aParticleEffects[ 0 ].m_pEffect = TNULL;
	}

	m_uiFlags |= FLAGS_FINISHED;
}

// $deBlob: FUNCTION 005b57f0
TBOOL AFXCompositionInstance::Update( TFLOAT a_fDeltaTime )
{
	if ( AFXCompositionManager::GetSingleton()->IsReloading() == TTRUE )
		return TTRUE;

	if ( m_uiFlags & FLAGS_FINISHED )
		return TFALSE;

	if ( m_uiFlags & FLAGS_PARTICLE_ONLY )
	{
		if ( m_aParticleEffects[ 0 ].m_pEffect == TNULL )
			m_uiFlags |= FLAGS_FINISHED;

		return TTRUE;
	}

	TBOOL bRunning = TFALSE;
	m_fTime += a_fDeltaTime;

	for ( TINT i = 0; i < MAX_RUNNING_COMMANDS; i++ )
	{
		if ( m_apRunningCommands[ i ] != TNULL )
		{
			if ( m_apRunningCommands[ i ]->Execute( m_fTime, a_fDeltaTime, *this ) == TTRUE )
				m_apRunningCommands[ i ] = TNULL;
			else if ( m_apRunningCommands[ i ] != TNULL && !m_apRunningCommands[ i ]->IsPersistentCommand() )
				bRunning = TTRUE;
		}
	}

	if ( !bRunning && m_iNextCommand >= m_pFXData->m_iNumCommands )
	{
		SetFinished();
		return TTRUE;
	}

	while ( m_iNextCommand < m_pFXData->m_iNumCommands )
	{
		AFXCompositionCommand* pCommand = m_pFXData->m_apCommands[ m_iNextCommand ];

		if ( m_fTime <= pCommand->m_fTime )
			return TTRUE;

		if ( !pCommand->Execute( m_fTime, a_fDeltaTime, *this ) )
		{
			TBOOL bFoundSpot = TFALSE;

			for ( TINT i = 0; i < MAX_RUNNING_COMMANDS; i++ )
			{
				if ( m_apRunningCommands[ i ] == TNULL )
				{
					bFoundSpot               = TTRUE;
					m_apRunningCommands[ i ] = m_pFXData->m_apCommands[ m_iNextCommand ];
					break;
				}
			}

			TASSERT( bFoundSpot, "ERROR: FX Composition failed to find a free spot for a running command.  Increase MAX_RUNNING_COMMANDS.\n" );
		}

		m_iNextCommand++;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 005b59a0
void AFXCompositionInstance::TParticleEffectCommandEndCallback( TParticleEffectInstance* a_pEffect, void* a_pUserData )
{
	AFXCompositionInstance* a_pInstance = TSTATICCAST( AFXCompositionInstance, a_pUserData );

	if ( a_pInstance->m_uiFlags & FLAGS_PARTICLE_ONLY )
	{
		a_pInstance->m_aParticleEffects[ 0 ].m_pEffect = TNULL;
		return;
	}

	if ( a_pInstance->m_uiFlags & FLAGS_FINISHED )
		return;

	for ( TINT i = 0; i < a_pInstance->m_pFXData->m_pParticleSystems->m_iNumParticles; i++ )
	{
		if ( a_pEffect == a_pInstance->m_aParticleEffects[ i ].m_pEffect )
		{
			a_pInstance->m_aParticleEffects[ i ].m_pEffect = TNULL;

			for ( TINT j = 0; j < MAX_RUNNING_COMMANDS; j++ )
			{
				if ( a_pInstance->m_apRunningCommands[ j ] != TNULL && i == a_pInstance->m_apRunningCommands[ j ]->GetParticleId() )
					a_pInstance->m_apRunningCommands[ j ] = TNULL;
			}

			return;
		}
	}
}

// $deBlob: FUNCTION 005b5a20
void AFXCompositionInstance::SetFinished()
{

	if ( !( m_uiFlags & FLAGS_PARTICLE_ONLY ) )
	{
		if ( m_uiFlags & FLAGS_FINISHED )
			return;

		for ( TINT i = 0; i < m_pFXData->m_pParticleSystems->m_iNumParticles; i++ )
		{
			if ( m_aParticleEffects[ i ].m_pEffect != TNULL )
			{
				// NOTE: retail clears the end callback of the first effect for every effect it stops (and writes
				// through a null pointer once the first slot is empty)
#ifdef DEBLOB_COMMUNITY_PATCH
				m_aParticleEffects[ i ].m_pEffect->SetFinishedCallback( TNULL, TNULL );
#else
				m_aParticleEffects[ 0 ].m_pEffect->SetFinishedCallback( TNULL, TNULL );
#endif
				m_aParticleEffects[ i ].m_pEffect->Stop();
				m_aParticleEffects[ i ].m_pEffect = TNULL;
			}
		}

		for ( TINT i = 0; i < m_pFXData->m_pModels->m_iNumModels; i++ )
			m_apModels[ i ]->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

		for ( TINT i = 0; i < MAX_RUNNING_COMMANDS; i++ )
			m_apRunningCommands[ i ] = TNULL;
	}
	else if ( m_aParticleEffects[ 0 ].m_pEffect != TNULL )
	{
		m_aParticleEffects[ 0 ].m_pEffect->SetFinishedCallback( TNULL, TNULL );
		m_aParticleEffects[ 0 ].m_pEffect->Stop();
		m_aParticleEffects[ 0 ].m_pEffect = TNULL;
		m_uiFlags |= FLAGS_FINISHED;
		return;
	}

	m_uiFlags |= FLAGS_FINISHED;
}

// $deBlob: FUNCTION 005b5b00
void AFXCompositionInstance::Reinitialize()
{
	if ( m_strName.Length() != 0 )
		Initialize( m_strName );
}
