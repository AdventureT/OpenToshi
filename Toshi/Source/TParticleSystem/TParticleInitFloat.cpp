#include "ToshiPCH.h"
#include "TParticleInit.h"
#include "TGenericParticles.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00540000
static TRandom s_oRandom;

struct InitFloatData : TParticleInit::TRBHeader
{
	TFLOAT m_afValues[ 2 ]; // 0x0C
};

// $deBlob: FUNCTION 0071e240
void __stdcall TParticleInitFloat::InitFixed( TParticleFloatBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TFLOAT fValue = TSTATICCAST( InitFloatData, a_pData )->m_afValues[ 0 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		( *a_pBuffer )[ i ] = fValue;
}

// $deBlob: FUNCTION 0071e460
void __stdcall TParticleInitFloat::InitMinMax( TParticleFloatBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TFLOAT fMin = TSTATICCAST( InitFloatData, a_pData )->m_afValues[ 0 ];
	TFLOAT fMax = TSTATICCAST( InitFloatData, a_pData )->m_afValues[ 1 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fValue       = s_oRandom.GetFloatMinMax( fMin, fMax );
		( *a_pBuffer )[ i ] = fValue;
	}
}

// $deBlob: FUNCTION 0071e500
void __stdcall TParticleInitFloat::InitFromSize( TParticleFloatBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TGenericParticles* pParticles = TParticleInit::sm_pCurParticles;
	TFLOAT             fScale     = TSTATICCAST( InitFloatData, a_pData )->m_afValues[ 0 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fSize        = pParticles->m_afSize[ i ];
		( *a_pBuffer )[ i ] = fSize * fScale;
	}
}

TOSHI_NAMESPACE_END
