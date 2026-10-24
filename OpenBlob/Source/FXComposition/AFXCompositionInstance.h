#pragma once
#include "FXComposition/AFXCompositionCommand.h"

#include <Math/TMatrix44.h>
#include <Toshi/TString8.h>

class AModelInstance2;

namespace Toshi
{
class TParticleEffect;
class TParticleEffectInstance;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: a composition's data (a symbol of the level's FX composition TRB, named after the composition)
//-----------------------------------------------------------------------------
struct AFXCompositionData
{
	struct Models
	{
		TINT         m_iNumModels;    // 0x00
		const TCHAR* m_aszNames[ 1 ]; // 0x04 [ m_iNumModels ]
	};

	struct ParticleSystems
	{
		TINT         m_iNumParticles; // 0x00
		const TCHAR* m_aszNames[ 1 ]; // 0x04 [ m_iNumParticles ]
	};

	Models*                m_pModels;          // 0x00
	ParticleSystems*       m_pParticleSystems; // 0x04
	TINT                   m_iNumCommands;     // 0x08
	AFXCompositionCommand* m_apCommands[ 1 ];  // 0x0C [ m_iNumCommands ] sorted by start time
};

//-----------------------------------------------------------------------------
// Purpose: a playing FX composition: the models and particle systems of an AFXCompositionData, animated by its
// commands. Registered with AFXCompositionManager.
// A name that isn't a composition is used as a particle system name (FLAGS_PARTICLE_ONLY).
//-----------------------------------------------------------------------------
class AFXCompositionInstance
{
public:
	static TCONSTEXPR_VAR TINT MAX_MODELS           = 10;
	static TCONSTEXPR_VAR TINT MAX_PARTICLEEFFECTS  = 5;
	static TCONSTEXPR_VAR TINT MAX_RUNNING_COMMANDS = 15;

	enum FLAGS : TUINT32
	{
		FLAGS_FINISHED      = 1,
		FLAGS_PARTICLE_ONLY = 2,
		FLAGS_STOPPING      = 4, // Stop: the running loops end (ENDLOOP no longer jumps back)
	};

	struct ParticleEffect
	{
		Toshi::TParticleEffectInstance* m_pEffect; // 0x00 the playing effect
		Toshi::TParticleEffect*         m_pSystem; // 0x04 the effect found by name
	};

	friend class AFXCompositionCommand;

public:
	AFXCompositionInstance();
	~AFXCompositionInstance();

	// Creates the models and finds the particle systems of the composition a_szName (or the particle system a_szName)
	TBOOL Initialize( const TCHAR* a_szName );

	// Destroys the models and particle effects
	void Destroy();

	// Starts the composition at a_rTransform
	void TriggerEffect( const Toshi::TMatrix44& a_rTransform );

	void SetTransform( const Toshi::TMatrix44& a_rTransform );

	// Lets the composition finish: stops the particle effect of a particle-only composition, ends the loops
	void Stop();
	void StopInterrupt();

	// Hides the models and kills the particle effects at once
	void StopInstant();

	// Executes the commands; returns TFALSE when it was already finished
	TBOOL Update( TFLOAT a_fDeltaTime );

	// Called by a particle effect when it ends (TParticleEffect end callback)
	static void TParticleEffectCommandEndCallback( Toshi::TParticleEffectInstance* a_pEffect, void* a_pInstance );

	void SetFinished();

	// Initialises again with the same name (after the manager reloaded the TRB).
	void Reinitialize();

	TBOOL IsFinished() const { return m_uiFlags & FLAGS_FINISHED; }

	// A composition or a particle system was found by Initialize
	// $deBlob: FUNCTION 007deae0
	TBOOL HasEffect() const { return m_pFXData != TNULL || ( m_uiFlags & FLAGS_PARTICLE_ONLY ); }

private:
	TUINT32                m_uiFlags;                                   // 0x00
	AFXCompositionData*    m_pFXData;                                   // 0x04
	TINT                   m_iNextCommand;                              // 0x08
	TFLOAT                 m_fTime;                                     // 0x0C
	Toshi::TMatrix44       m_oTransform;                                // 0x10
	AModelInstance2*       m_apModels[ MAX_MODELS ];                    // 0x50
	ParticleEffect         m_aParticleEffects[ MAX_PARTICLEEFFECTS ];   // 0x78
	AFXCompositionCommand* m_apRunningCommands[ MAX_RUNNING_COMMANDS ]; // 0xA0
	Toshi::TString8        m_strName;                                   // 0xDC
};                                                                      // 0xE8
