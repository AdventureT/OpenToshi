#include "ToshiPCH.h"
#include "TParticleKeyframe.h"
#include "TGenericParticles.h"
#include "Math/TMathInline.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: unused (only its static initialiser references it)
// $deBlob: FUNCTION 0053ff60
static TRandom s_oRandom;

//-----------------------------------------------------------------------------
// TParticleKeyframeSize
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0071c270
void TParticleKeyframeSize::Link( TParticleKeyframe* a_pKey )
{
	a_pKey->m_fnStart          = Keyframe_SetPrevKey;
	a_pKey->m_fnUpdate         = Keyframe_DefaultUpdate;
	a_pKey->m_fnUpdateParticle = UpdateParticle;
}

// When the particle gets to a new key, its size becomes the one of the key it leaves
// $deBlob: FUNCTION 0071c2a0
void __stdcall TParticleKeyframeSize::UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle )
{
	TParticleKeyframe* pCurKey = TREINTERPRETCAST( TParticleKeyframe*, a_pParticles->m_apCurSizeKey[ a_iParticle ] );

	if ( pCurKey == a_pKey )
		return;

	if ( pCurKey != TNULL )
	{
		TParticleKeyframe::FloatData* pCurData = pCurKey->GetData<TParticleKeyframe::FloatData>();

		if ( pCurData->m_eType == TParticleKeyframe::KEYFRAMETYPE_SIZEMULT )
		{
			TFLOAT& rSize = a_pParticles->m_afSize[ a_iParticle ];
			rSize         = pCurData->m_fValue * rSize;
		}
		else
		{
			a_pParticles->m_afSize[ a_iParticle ] = pCurData->m_fValue;
		}
	}

	a_pParticles->m_apCurSizeKey[ a_iParticle ] = TREINTERPRETCAST( TUINT32, a_pKey );
}

// $deBlob: FUNCTION 0071c330
TFLOAT TParticleKeyframe::GetSize( TGenericParticles* a_pParticles, TINT a_iParticle )
{
	FloatData* pData = GetData<FloatData>();
	TFLOAT     fLife = a_pParticles->m_afLife[ a_iParticle ];
	TFLOAT     fT;

	if ( m_pPrev == TNULL )
		fT = fLife / pData->m_fTime;
	else
		fT = ( fLife - m_pPrev->GetTime() ) / ( pData->m_fTime - m_pPrev->GetTime() );

	if ( fT <= 0.0f )
		fT = 0.0f;
	else if ( 1.0f <= fT )
		fT = 1.0f;

	TFLOAT fEased = Ease( fT );
	TFLOAT fSize  = a_pParticles->m_afSize[ a_iParticle ];
	TFLOAT fTo    = pData->m_fValue;

	if ( m_pData->m_eType == KEYFRAMETYPE_SIZEMULT )
		fTo = fTo * fSize;

	TFLOAT fResult = ( fTo - fSize ) * fEased + fSize;

	if ( fResult == 0.0f )
		fResult = 0.01f;

	return fResult;
}

//-----------------------------------------------------------------------------
// TParticleKeyframeMass
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0071fe20
void TParticleKeyframeMass::Link( TParticleKeyframe* a_pKey )
{
	a_pKey->m_fnStart          = Keyframe_DefaultStart;
	a_pKey->m_fnUpdate         = Keyframe_DefaultUpdate;
	a_pKey->m_fnUpdateParticle = UpdateParticle;
}

// $deBlob: FUNCTION 0071fe50
void __stdcall TParticleKeyframeMass::UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle )
{
	TParticleKeyframe::FloatData* pData = a_pKey->GetData<TParticleKeyframe::FloatData>();
	TFLOAT                        fFrom;
	TFLOAT                        fTo;

	if ( a_pKey->m_pPrev == TNULL )
	{
		fTo   = pData->m_fValue;
		fFrom = a_pParticles->m_afMass[ a_iParticle ];
	}
	else
	{
		fTo   = pData->m_fValue;
		fFrom = a_pKey->m_pPrev->GetData<TParticleKeyframe::FloatData>()->m_fValue;
	}

	TFLOAT fMass = TMath::LERP( fFrom, fTo, a_fT );

	if ( fMass == 0.0f )
		fMass = 0.01f;

	a_pParticles->m_afMass[ a_iParticle ] = fMass;
}

//-----------------------------------------------------------------------------
// TParticleKeyframeUV
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0071fdb0
void TParticleKeyframeUV::Link( TParticleKeyframe* a_pKey )
{
	a_pKey->m_fnStart          = Keyframe_DefaultStart;
	a_pKey->m_fnUpdate         = Keyframe_DefaultUpdate;
	a_pKey->m_fnUpdateParticle = UpdateParticle;
}

// $deBlob: FUNCTION 0071fde0
void __stdcall TParticleKeyframeUV::UpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle )
{
	if ( a_pKey->m_pPrev != TNULL )
	{
		TFLOAT fUVIndex                        = a_pKey->m_pPrev->GetData<TParticleKeyframe::UVData>()->m_fUVIndex;
		a_pParticles->m_aiUVIndex[ a_iParticle ] = TINT( fUVIndex );
	}
}

TOSHI_NAMESPACE_END
