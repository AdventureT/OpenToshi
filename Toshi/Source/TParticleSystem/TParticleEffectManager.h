#pragma once


TOSHI_NAMESPACE_START

class TParticleEffect;
class TParticleEffectInstance;
class TGenericParticlesSettings;

// Owns the effects and settings loaded from the particle TRBs and a pool of effect instances (0x20 bytes)
class TParticleEffectManager
{
public:
	TParticleEffectManager( TINT a_iMaxInstances );
	~TParticleEffectManager();

	void Create();

	TParticleEffect*           FindEffect( const TCHAR* a_szName ) const;
	// Forgets the free and used instance lists
	void                       ClearInstanceLists();
	TGenericParticlesSettings* FindSetting( const TCHAR* a_szName ) const;

	TParticleEffectInstance* StartEffect( TParticleEffect* a_pEffect );
	void                     Update( TFLOAT a_fDeltaTime );

	// $deBlob: FUNCTION 00710d70
	TGenericParticlesSettings* GetSettings() const { return m_pSettings; }
	// $deBlob: FUNCTION 00710d80
	TParticleEffect* GetEffects() const { return m_pEffects; }
	// $deBlob: FUNCTION 007114c0
	TParticleEffectInstance* GetInstances() const { return m_pInstances; }
	TINT GetNumEffects() const;
	void StopAllEffects();
	void StopAllEffects( TParticleEffect* a_pEffect );
	void StopStealableEffectInstance();
	TINT GetNumInstances( TParticleEffect* a_pEffect ) const;

	void RemoveInstanceFromUsed( TParticleEffectInstance* a_pInstance );
	void AddToEndOfUsedList( TParticleEffectInstance* a_pInstance );
	void ReleaseInstance( TParticleEffectInstance* a_pInstance );

	void AddSetting( TGenericParticlesSettings* a_pSetting );
	void RemoveSetting( TGenericParticlesSettings* a_pSetting );
	void AddEffect( TParticleEffect* a_pEffect );
	void RemoveEffect( TParticleEffect* a_pEffect );

private:
	TINT                       m_iMaxInstances;   // 0x00
	TGenericParticlesSettings* m_pSettings;       // 0x04
	TParticleEffect*           m_pEffects;        // 0x08
	TParticleEffectInstance*   m_pInstances;      // 0x0C
	TParticleEffectInstance*   m_pFreeInstances;  // 0x10
	TParticleEffectInstance*   m_pUsedInstances;  // 0x14
	TParticleEffectInstance*   m_pLastUsed;       // 0x18
	TBOOL                      m_bStealInstances; // 0x1C
};

TOSHI_NAMESPACE_END
