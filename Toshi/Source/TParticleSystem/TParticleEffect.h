#pragma once
#include "Math/TMatrix44.h"


TOSHI_NAMESPACE_START

class TGenericParticlesSettings;
class TParticleKeyframeTimeline;

// One particle system of an effect (0x14 bytes)
struct TParticleEffectSystem
{
	// m_uiFlags: how the generic particles follow the instance's transform
	enum FLAGS : TUINT
	{
		FLAGS_TRANSWITHXFORM    = 1 << 0, // TGenericParticles::FLAGS_TRANSWITHXFORM
		FLAGS_TRANSROTWITHXFORM = 1 << 1, // TGenericParticles::FLAGS_TRANSROTWITHXFORM
	};

	~TParticleEffectSystem();

	// $deBlob: FUNCTION 00720140
	TBOOL HasFlag( TUINT a_uiFlag ) const { return ( m_uiFlags & a_uiFlag ) != 0; }

	void Copy( const TParticleEffectSystem& a_rcOther );

	void ResolveSettings();

	TMatrix44*                 m_pTransform; // 0x00
	TGenericParticlesSettings* m_pSettings;  // 0x04
	const TCHAR*               m_szSettings; // 0x08
	TParticleKeyframeTimeline* m_pKeys;      // 0x0C
	TUINT                      m_uiFlags;    // 0x10
};

class TParticleEffect
{
public:
	static TCONSTEXPR_VAR TUINT s_LatestVer = 0x10000;

public:
	~TParticleEffect();

	void             Create();
	TParticleEffect* Copy() const;
	TINT             FindSystem( const TCHAR* a_szName ) const;

	TINT GetNumSystems() const { return m_iNumSystems; }
	TBOOL IsStealable() const { return m_iStealable != 0; }

	// $deBlob: FUNCTION 005b8b40
	TParticleEffectSystem& GetSystem( TINT a_iSystem ) const
	{
		TASSERT( a_iSystem >= 0 && a_iSystem < m_iNumSystems );
		return m_pSystems[ a_iSystem ];
	}

	// $deBlob: FUNCTION 005b8b20
	TParticleEffect* GetPrev() const { return m_pPrev; }
	// $deBlob: FUNCTION 005b8b30
	TParticleEffect* GetNext() const { return m_pNext; }
	const TCHAR* GetName() const { return m_szName; }

public:
	TUINT                  m_uiVer;       // 0x00
	TParticleEffect*       m_pNext;       // 0x04
	TParticleEffect*       m_pPrev;       // 0x08
	const TCHAR*           m_szName;      // 0x0C
	TINT16                 m_iNumSystems; // 0x10
	TINT16                 m_iStealable;  // 0x12
	TParticleEffectSystem* m_pSystems;    // 0x14
	TUINT                  m_uiUnk18;     // 0x18
};

TOSHI_NAMESPACE_END
