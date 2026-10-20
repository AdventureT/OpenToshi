#include "ToshiPCH.h"
#include "TParticleKeyframe.h"
#include "TGenericParticles.h"
#include "TGenericParticlesSettings.h"
#include "Math/TMathInline.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: unused (only its static initialiser references it)
// $deBlob: FUNCTION 0053ff20
static TRandom s_oRandom;

// The particle's life within the key (0 at the previous key, 1 at this one)
static TFLOAT GetKeyT( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TINT a_iParticle )
{
	TParticleKeyframe::TRBHeader* pData = a_pKey->m_pData;
	TFLOAT                        fLife = a_pParticles->m_afLife[ a_iParticle ];

	if ( a_pKey->m_pPrev == TNULL )
		return fLife / pData->m_fTime;

	return ( fLife - a_pKey->m_pPrev->GetTime() ) / ( pData->m_fTime - a_pKey->m_pPrev->GetTime() );
}

// $deBlob: FUNCTION 0071be20
void TParticleKeyframeColour::Link( TParticleKeyframe* a_pKey )
{
	TParticleKeyframe::ColourData* pData = a_pKey->GetData<TParticleKeyframe::ColourData>();
	a_pKey->m_fnStart                    = Keyframe_SetPrevKey;
	a_pKey->m_fnUpdate                   = Keyframe_DefaultUpdate;
	a_pKey->m_fnUpdateParticle           = UpdateParticle;
	pData->m_uiColour                    = TGenericParticlesSettings::ConvertColourToSKUColour( pData->m_uiColour );
}

// $deBlob: FUNCTION 0071c0a0
void TParticleKeyframeColour::LinkAlpha( TParticleKeyframe* a_pKey )
{
	a_pKey->m_fnStart          = Keyframe_SetPrevKey;
	a_pKey->m_fnUpdate         = Keyframe_DefaultUpdate;
	a_pKey->m_fnUpdateParticle = UpdateParticle; // NOTE: retail has a copy of it for the alpha keys
}

// When the particle gets to a new key, its colour becomes the one of the key before
// $deBlob: FUNCTION 0071bfc0
// $deBlob: FUNCTION 0071c1b0
void __stdcall TParticleKeyframeColour::UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle )
{
	TParticleKeyframe* pCurKey = TREINTERPRETCAST( TParticleKeyframe*, a_pParticles->m_apCurColourKey[ a_iParticle ] );

	if ( pCurKey == a_pKey )
		return;

	if ( pCurKey != TNULL && pCurKey->m_pData != TNULL )
	{
		TParticleKeyframe* pPrevKey = a_pKey->m_pPrev;

		if ( pCurKey->GetType() == TParticleKeyframe::KEYFRAMETYPE_ALPHA )
		{
			TParticleKeyframe* pFrom  = ( pPrevKey != TNULL ) ? pPrevKey : pCurKey;
			TUINT32            uiAlpha = pFrom->GetData<TParticleKeyframe::ColourData>()->m_uiColour;
			TUINT32            uiColour = ( a_pParticles->m_auiColour[ a_iParticle ] & 0xFFFFFF ) | ( uiAlpha << 24 );
			a_pParticles->m_auiColour[ a_iParticle ] = uiColour;
		}
		else
		{
			TParticleKeyframe* pFrom    = ( pPrevKey != TNULL ) ? pPrevKey : pCurKey;
			TUINT32            uiColour = pFrom->GetData<TParticleKeyframe::ColourData>()->m_uiColour;
			a_pParticles->m_auiColour[ a_iParticle ] = uiColour;
		}
	}

	a_pParticles->m_apCurColourKey[ a_iParticle ] = TREINTERPRETCAST( TUINT32, a_pKey );
}

// $deBlob: FUNCTION 0071be50
TUINT32 TParticleKeyframe::GetColour( TGenericParticles* a_pParticles, TINT a_iParticle )
{
	TFLOAT fT = GetKeyT( this, a_pParticles, a_iParticle );
	TMath::Clip( fT, 0.0f, 1.0f );
	fT = Ease( fT );

	TINT    iT       = TINT( lrintf( fT * 255.0f ) );
	TUINT32 uiColour = a_pParticles->m_auiColour[ a_iParticle ];
	TUINT32 uiTo     = GetData<ColourData>()->m_uiColour;

	TUINT uiB = uiColour & 0xFF;
	TUINT uiG = ( uiColour >> 8 ) & 0xFF;
	TUINT uiR = ( uiColour >> 16 ) & 0xFF;
	TUINT uiA = uiColour >> 24;

	return ( ( TINT( ( uiTo & 0xFF ) - uiB ) * iT >> 8 ) + uiB & 0xFF ) |
	    ( ( TINT( ( ( uiTo >> 16 ) & 0xFF ) - uiR ) * iT >> 8 ) + uiR & 0xFF ) << 16 |
	    ( ( TINT( ( ( uiTo >> 8 ) & 0xFF ) - uiG ) * iT >> 8 ) + uiG & 0xFF ) << 8 |
	    ( ( TINT( ( uiTo >> 24 ) - uiA ) * iT >> 8 ) + uiA ) * 0x1000000;
}

// $deBlob: FUNCTION 0071c0c0
TUINT32 TParticleKeyframe::GetAlpha( TGenericParticles* a_pParticles, TINT a_iParticle )
{
	TFLOAT fT = GetKeyT( this, a_pParticles, a_iParticle );
	TMath::Clip( fT, 0.0f, 1.0f );
	fT = Ease( fT );

	TFLOAT  fAlpha   = fT * 255.0f;
	TUINT32 uiColour = a_pParticles->m_auiColour[ a_iParticle ];
	TUINT   uiA      = uiColour >> 24;

	return ( ( ( TINT( GetData<ColourData>()->m_uiColour - uiA ) * TINT( lrintf( fAlpha ) ) ) >> 8 ) + uiA ) * 0x1000000 ^ ( uiColour & 0xFFFFFF );
}

TOSHI_NAMESPACE_END
