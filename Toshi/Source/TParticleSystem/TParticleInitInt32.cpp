#include "ToshiPCH.h"
#include "TParticleInit.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00540030
static TRandom s_oRandom;

struct InitIntData : TParticleInit::TRBHeader
{
	TINT m_aiValues[ 2 ]; // 0x0C
};

// $deBlob: FUNCTION 0071e750
void __stdcall TParticleInitInt32::InitFixed( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TUINT32 uiValue = TSTATICCAST( InitIntData, a_pData )->m_aiValues[ 0 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		( *a_pBuffer )[ i ] = uiValue;
}

// $deBlob: FUNCTION 0071e970
void __stdcall TParticleInitInt32::InitMinMax( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TINT iMin = TSTATICCAST( InitIntData, a_pData )->m_aiValues[ 0 ];
	TINT iMax = TSTATICCAST( InitIntData, a_pData )->m_aiValues[ 1 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		// TRandom::GetInt( a_iLower, a_iUpper ) (inlined)
		TASSERT( TUINT32( iMax ) > TUINT32( iMin ) && "a_iUpper>a_iLower" );
		TINT iValue         = s_oRandom.GetInt( iMax - iMin );
		( *a_pBuffer )[ i ] = iValue + iMin;
	}
}

TOSHI_NAMESPACE_END
