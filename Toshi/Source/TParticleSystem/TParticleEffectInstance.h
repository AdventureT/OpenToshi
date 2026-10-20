#pragma once
#include "Math/TMatrix44.h"
#include "TParticleKeyframe.h"


TOSHI_NAMESPACE_START

class TParticleEffect;
struct TParticleEffectSystem;
class TParticles;

// A running particle effect (0x198 bytes), pooled by TParticleEffectManager
class TParticleEffectInstance
{
public:
	static TCONSTEXPR_VAR TINT s_iMaxSettings = 10;

	using t_FinishedCallback = void ( * )( TParticleEffectInstance* a_pInstance, void* a_pUserData );
	using t_UpdateCallback   = void ( * )( TParticles* a_pParticles, void* a_pUserData );

	enum FLAGS : TUINT
	{
		FLAGS_STOLEN = 1 << 0,
		FLAGS_PAUSED = 1 << 1,
	};

	enum SYSTEMFLAGS : TUINT
	{
		SYSTEMFLAGS_FINISHED = 1 << 0,
		SYSTEMFLAGS_PAUSED   = 1 << 1,
	};

	struct System
	{
		TParticleKeyframeState m_oState;     // 0x00
		TParticles*            m_pParticles; // 0x10
		TUINT                  m_uiFlags;    // 0x14
	};

public:
	TParticleEffectInstance();

	void  Init( TParticleEffect* a_pEffect );
	TBOOL Update( TFLOAT a_fDeltaTime );
	void  SetParticleSystemUpdateCallback( TINT a_iSystem, t_UpdateCallback a_fnCallback, void* a_pUserData );
	void  Stop();
	void  StopLooping();
	void  StopInstant();
	void  SetPaused( TBOOL a_bPaused );

	static void GenericParticleFinished( TParticles* a_pParticles, TParticleEffectInstance* a_pInstance );

	void SetFinishedCallback( t_FinishedCallback a_fnCallback, void* a_pUserData )
	{
		m_fnFinished        = a_fnCallback;
		m_pFinishedUserData = a_pUserData;
	}

	TParticleEffect* GetEffect() const { return m_pEffect; }
	TMatrix44&       GetTransform() { return m_oTransform; }
	System&          GetSystem( TINT a_iSystem ) { return m_aSystems[ a_iSystem ]; }

public:
	static TMatrix44                s_mFinalTransform;
	static TParticleEffectSystem*   s_pActiveEffectSys;
	static TParticleEffectInstance* s_pActiveEffectInstance;

public:
	TMatrix44                m_oTransform;                          // 0x000
	TParticleEffect*         m_pEffect;                             // 0x040
	t_FinishedCallback       m_fnFinished;                          // 0x044
	void*                    m_pFinishedUserData;                   // 0x048
	TUINT                    m_uiFlags;                             // 0x04C
	System                   m_aSystems[ s_iMaxSettings ];          // 0x050
	t_UpdateCallback         m_afnUpdateCallbacks[ s_iMaxSettings ]; // 0x140
	void*                    m_apUpdateUserData[ s_iMaxSettings ];   // 0x168
	TParticleEffectInstance* m_pNext;                               // 0x190
	TParticleEffectInstance* m_pPrev;                               // 0x194
};

TSTATICASSERT( sizeof( TParticleEffectInstance ) == 0x198 );

TOSHI_NAMESPACE_END
