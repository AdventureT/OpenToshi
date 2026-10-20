#include "ToshiPCH.h"
#include "TParticleInit.h"
#include "TGenericParticlesSettings.h"
#include "Math/TRandom.h"
#include "Math/TMathInline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00540060
static TRandom s_oRandom;

// $deBlob: FUNCTION 0071e9f0
void TParticleInitColour::FixedData::ConvertColourToSKUColour()
{
	m_uiColour = TGenericParticlesSettings::ConvertColourToSKUColour( m_uiColour );
}

// $deBlob: FUNCTION 0071ea00
void TParticleInitColour::MinMaxData::ConvertColourToSKUColour()
{
	*TREINTERPRETCAST( TUINT32*, m_aMin ) = TGenericParticlesSettings::ConvertColourToSKUColour( *TREINTERPRETCAST( TUINT32*, m_aMin ) );
	*TREINTERPRETCAST( TUINT32*, m_aMax ) = TGenericParticlesSettings::ConvertColourToSKUColour( *TREINTERPRETCAST( TUINT32*, m_aMax ) );
}

// $deBlob: FUNCTION 0071ea20
void __stdcall TParticleInitColour::InitFixed( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TUINT32 uiColour = TSTATICCAST( FixedData, a_pData )->m_uiColour;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		( *a_pBuffer )[ i ] = uiColour;
}

// $deBlob: FUNCTION 0071ec40
void __stdcall TParticleInitColour::InitMinMax( TParticleUIntBuffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	MinMaxData* pData = TSTATICCAST( MinMaxData, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fT = s_oRandom.GetFloat();
		TFLOAT f0 = TMath::LERP( TFLOAT( pData->m_aMin[ 0 ] ), TFLOAT( pData->m_aMax[ 0 ] ), fT );
		TFLOAT f1 = TMath::LERP( TFLOAT( pData->m_aMin[ 1 ] ), TFLOAT( pData->m_aMax[ 1 ] ), fT );
		TFLOAT f2 = TMath::LERP( TFLOAT( pData->m_aMin[ 2 ] ), TFLOAT( pData->m_aMax[ 2 ] ), fT );
		TFLOAT f3 = TMath::LERP( TFLOAT( pData->m_aMin[ 3 ] ), TFLOAT( pData->m_aMax[ 3 ] ), fT );

		TUINT32 uiColour = TUINT8( TINT( f0 ) ) | ( TUINT8( TINT( f1 ) ) << 8 ) | ( TUINT8( TINT( f2 ) ) << 16 ) | ( TUINT32( TUINT8( TINT( f3 ) ) ) << 24 );
		( *a_pBuffer )[ i ] = uiColour;
	}
}

TOSHI_NAMESPACE_END
