#include "ToshiPCH.h"
#include "TParticleKeyframe.h"
#include "TGenericParticles.h"
#include "TGenericParticlesSettings.h"
#include "TParticleSystem.h"
#include "TParticleEffect.h"
#include "TParticleEffectInstance.h"
#include "TParticleEffectManager.h"
#include "Math/TMath.h"
#include "Math/TMathInline.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0071b1b0
TParticleKeyframeTimeline::TParticleKeyframeTimeline( const TParticleKeyframeTimeline& a_rcOrig )
{
	m_iNumLoopKeyframes = a_rcOrig.m_iNumLoopKeyframes;

	if ( m_iNumLoopKeyframes < 1 )
		m_aLoopKeyframes = TNULL;
	else
		m_aLoopKeyframes = TSTATICCAST( TParticleKeyframe*, TMemalign( 16, m_iNumLoopKeyframes * sizeof( TParticleKeyframe* ) ) );

	for ( TINT i = 0; i < m_iNumLoopKeyframes; i++ )
	{
		m_aLoopKeyframes[ i ] = new TParticleKeyframe( *a_rcOrig.m_aLoopKeyframes[ i ] );

		if ( i > 0 )
			m_aLoopKeyframes[ i ]->m_pPrev = m_aLoopKeyframes[ i - 1 ];
	}
}

// $deBlob: FUNCTION 0071b240
// $deBlob: FUNCTION 00711a30
TParticleKeyframeTimeline::~TParticleKeyframeTimeline()
{
	for ( TINT i = 0; i < m_iNumLoopKeyframes; i++ )
	{
		if ( m_aLoopKeyframes[ i ] )
			delete m_aLoopKeyframes[ i ];
	}

	delete m_aLoopKeyframes;
}

// $deBlob: FUNCTION 0071b180
void TParticleKeyframeTimeline::InitState( TParticleKeyframeState& a_rState )
{
	a_rState.m_pCurKey  = TNULL;
	a_rState.m_iCurKey  = -1;
	a_rState.m_iLoops   = 1;
	a_rState.m_pPrevKey = TNULL;
	a_rState.m_fTime    = 0.0f;
}

// $deBlob: FUNCTION 0071b280
void TParticleKeyframeTimeline::Link()
{
	for ( TINT i = 0; i < m_iNumLoopKeyframes; i++ )
	{
		m_aLoopKeyframes[ i ]->Link();

		if ( i > 0 )
			m_aLoopKeyframes[ i ]->m_pPrev = m_aLoopKeyframes[ i - 1 ];
	}
}

// $deBlob: FUNCTION 0071bad0
TParticleKeyframe* TParticleKeyframeTimeline::GetNextKeyframe( TParticleKeyframeState& a_rState )
{
	if ( m_iNumLoopKeyframes != 0 )
	{
		if ( a_rState.m_iLoops > 0 && a_rState.m_iCurKey >= 0 &&
		     m_aLoopKeyframes[ a_rState.m_iCurKey ]->m_pData->m_iLoopType == TParticleKeyframe::LOOPTYPE_END )
		{
			// Jump back to the start of the loop
			TINT16 iNewKeyframe = 0;

			while ( m_aLoopKeyframes[ iNewKeyframe ]->m_pData->m_iLoopType != TParticleKeyframe::LOOPTYPE_START )
			{
				iNewKeyframe++;
				TASSERT( iNewKeyframe < m_iNumLoopKeyframes );
			}

			TFLOAT fTime       = a_rState.m_fTime - m_aLoopKeyframes[ a_rState.m_iCurKey ]->GetTime();
			a_rState.m_fTime   = fTime;
			a_rState.m_fTime   = m_aLoopKeyframes[ iNewKeyframe ]->GetTime() + fTime;
			a_rState.m_iCurKey = iNewKeyframe;

			return m_aLoopKeyframes[ iNewKeyframe ];
		}

		a_rState.m_iCurKey++;

		if ( a_rState.m_iCurKey < m_iNumLoopKeyframes )
			return m_aLoopKeyframes[ a_rState.m_iCurKey ];
	}

	a_rState.m_iCurKey = -1;
	return TNULL;
}

// The easing functions of the keyframes (indexed by TRBHeader::m_uiEasing)

// $deBlob: FUNCTION 0071c940
static TFLOAT Easing_Step( TFLOAT a_fT )
{
	return TFLOAT( TINT( a_fT ) );
}

// $deBlob: FUNCTION 0071c960
static TFLOAT Easing_Linear( TFLOAT a_fT )
{
	return a_fT;
}

// $deBlob: FUNCTION 0071c970
static TFLOAT Easing_QuadIn( TFLOAT a_fT )
{
	return a_fT * a_fT;
}

// $deBlob: FUNCTION 0071c980
static TFLOAT Easing_QuadOut( TFLOAT a_fT )
{
	return 1.0f - ( 1.0f - a_fT ) * ( 1.0f - a_fT );
}

// $deBlob: FUNCTION 0071c9b0
static TFLOAT Easing_CubicIn( TFLOAT a_fT )
{
	return a_fT * a_fT * a_fT;
}

// $deBlob: FUNCTION 0071c9c0
static TFLOAT Easing_CubicOut( TFLOAT a_fT )
{
	TFLOAT fInv = 1.0f - a_fT;
	return 1.0f - fInv * fInv * fInv;
}

// $deBlob: FUNCTION 0071c9f0
static TFLOAT Easing_Gain075( TFLOAT a_fT )
{
	return TMath::Gain075( a_fT );
}

// $deBlob: FUNCTION 0071ca10
static TFLOAT Easing_Gain025( TFLOAT a_fT )
{
	return TMath::Gain( 0.25f, a_fT );
}

static TFLOAT ( *s_afnEasings[] )( TFLOAT ) = {
	Easing_Step,
	Easing_Linear,
	Easing_QuadIn,
	Easing_QuadOut,
	Easing_CubicIn,
	Easing_CubicOut,
	Easing_Gain075,
	Easing_Gain025,
};

// $deBlob: FUNCTION 0071bc00
TFLOAT TParticleKeyframe::Ease( TFLOAT a_fT ) const
{
	return s_afnEasings[ m_pData->m_uiEasing ]( a_fT );
}

// $deBlob: FUNCTION 0071b390
TBOOL TParticleKeyframeTimeline::Update( TFLOAT a_fDeltaTime, TParticleKeyframeState& a_rState )
{
	if ( a_rState.m_pCurKey == TNULL )
	{
		TParticleKeyframe* pKey = GetNextKeyframe( a_rState );
		a_rState.m_pCurKey      = pKey;

		if ( pKey == TNULL )
			return TFALSE;

		pKey->m_fnStart( pKey, TNULL );
	}

	TFLOAT fT          = 0.0f;
	TFLOAT fTime       = a_rState.m_fTime + a_fDeltaTime;
	TParticleKeyframe* pCurKey = a_rState.m_pCurKey;
	a_rState.m_fTime   = fTime;

	if ( a_rState.m_pPrevKey != TNULL )
	{
		TFLOAT fPrevTime  = a_rState.m_pPrevKey->GetTime();
		TFLOAT fTimeOfKey = pCurKey->GetTime() - fPrevTime;
		TASSERT( fTimeOfKey != 0.0f );

		fT = s_afnEasings[ pCurKey->m_pData->m_uiEasing ]( ( fTime - fPrevTime ) / fTimeOfKey );
	}

	pCurKey->m_fnUpdate( pCurKey, a_fDeltaTime, fT, &a_rState );

	if ( pCurKey->GetTime() <= a_rState.m_fTime && a_rState.m_fTime != pCurKey->GetTime() )
	{
		TParticleKeyframe* pNextKey = GetNextKeyframe( a_rState );

		if ( pNextKey == TNULL )
			return TFALSE;

		TParticleKeyframe* pPrevKey = a_rState.m_pCurKey;
		a_rState.m_pPrevKey         = pPrevKey;
		a_rState.m_pCurKey          = pNextKey;
		pNextKey->m_fnStart( pNextKey, pPrevKey );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 0071b350
TBOOL TParticleKeyframeTimeline::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleKeyframeState& a_rState )
{
	TBOOL bResult = Update( a_fDeltaTime, a_rState );
	UpdateParticles( a_pParticles, a_iStart, a_iEnd );
	return bResult;
}

// $deBlob: FUNCTION 0071b2c0
void TParticleKeyframeTimeline::UpdateEvents( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TUINT8 uiEvent = a_pParticles->m_auiEventIndex[ i ];

		if ( TINT( uiEvent ) < m_iNumLoopKeyframes )
		{
			TFLOAT             fLife = a_pParticles->m_afLife[ i ];
			TParticleKeyframe* pKey  = m_aLoopKeyframes[ uiEvent ];

			if ( pKey->GetTime() <= fLife && fLife != pKey->GetTime() )
			{
				pKey->m_fnUpdateParticle( pKey, a_pParticles, 0.0f, i );
				a_pParticles->m_auiEventIndex[ i ] = uiEvent + 1;
			}
		}
	}
}

// $deBlob: FUNCTION 0071b4c0
void TParticleKeyframeTimeline::UpdateParticles( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fLife = a_pParticles->m_afLife[ i ];

		for ( TINT j = 0; j < m_iNumLoopKeyframes; j++ )
		{
			if ( fLife < m_aLoopKeyframes[ j ]->GetTime() )
			{
				TFLOAT fT = 0.0f;

				if ( j > 0 )
				{
					TFLOAT fPrevTime  = m_aLoopKeyframes[ j - 1 ]->GetTime();
					TFLOAT fTimeOfKey = m_aLoopKeyframes[ j ]->GetTime() - fPrevTime;
					TASSERT( fTimeOfKey != 0.0f );

					fT = m_aLoopKeyframes[ j ]->Ease( ( fLife - fPrevTime ) / fTimeOfKey );
				}

				TParticleKeyframe* pKey = m_aLoopKeyframes[ j ];
				pKey->m_fnUpdateParticle( pKey, a_pParticles, fT, i );
				break;
			}
		}
	}
}

// $deBlob: FUNCTION 0071b7c0
void TParticleKeyframeTimeline::InitParticles( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fLife = a_pParticles->m_afLife[ i ];

		for ( TINT j = 0; j < m_iNumLoopKeyframes; j++ )
		{
			TParticleKeyframe* pKey = m_aLoopKeyframes[ j ];

			if ( fLife < pKey->GetTime() )
			{
				TFLOAT fT = 0.0f;

				if ( j > 0 )
				{
					TFLOAT fPrevTime  = m_aLoopKeyframes[ j - 1 ]->GetTime();
					TFLOAT fTimeOfKey = pKey->GetTime() - fPrevTime;
					TASSERT( fTimeOfKey != 0.0f );

					fT = m_aLoopKeyframes[ j ]->Ease( ( fLife - fPrevTime ) / fTimeOfKey );
				}

				pKey = m_aLoopKeyframes[ j ];
				pKey->m_fnUpdateParticle( pKey, a_pParticles, fT, i );
				break;
			}

			// The particle is already past the key
			pKey->m_fnUpdateParticle( pKey, a_pParticles, 1.0f, i );
		}
	}
}

TOSHI_NAMESPACE_END
