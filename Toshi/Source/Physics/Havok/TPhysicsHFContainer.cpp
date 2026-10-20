#include "ToshiPCH.h"
#include "TPhysicsHFContainer.h"
#include "TPhysicsManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00734750
TPhysicsHFContainer::TPhysicsHFContainer()
{
	m_data          = TNULL;
	m_bExternalData = TFALSE;

	for ( TINT i = 0; i < MAX_MATERIALS; i++ )
	{
		m_aMaterials[ i ].m_fUnk0 = 1.0f;
		m_aMaterials[ i ].m_fUnk4 = 0.0f;
		m_aMaterials[ i ].m_fUnk8 = 0.1f;
	}
}

// $deBlob: FUNCTION 00734840
// $deBlob: FUNCTION 00735140
TPhysicsHFContainer::~TPhysicsHFContainer()
{
	if ( !m_bExternalData && m_data )
		delete[] m_data;
}

// $deBlob: FUNCTION 00734860
TPhysicsHFContainer* TPhysicsHFContainer::Create()
{
	return new ( TPhysicsManager::GetHeap() ) TPhysicsHFContainer();
}

// $deBlob: FUNCTION 00734890
void TPhysicsHFContainer::Destroy( TPhysicsHFContainer* a_pContainer )
{
	if ( a_pContainer )
		delete a_pContainer;
}

// $deBlob: FUNCTION 007348c0
void TPhysicsHFContainer::CreateData( TINT a_x, TINT a_y, TFLOAT a_fMinHeight, TFLOAT a_fMaxHeight, TFLOAT a_fSizeX, TFLOAT a_fSizeY )
{
	// NOTE: retail frees the old samples with the scalar delete and also when they're external (m_bExternalData is
	// left as it is)
	if ( m_data )
	{
		delete m_data;
		m_data = TNULL;
	}

	if ( a_x <= 0 || a_y <= 0 )
		return;

	m_fMaxHeight   = a_fMaxHeight;
	m_fMinHeight   = a_fMinHeight;
	m_fHeightRange = a_fMaxHeight - a_fMinHeight;

	if ( m_fHeightRange < 0.001f )
		m_fHeightRange = 0.001f;

	m_fSizeX = a_fSizeX;
	m_x      = a_x;
	m_y      = a_y;
	m_fSizeY = a_fSizeY;

	TINT iNumSamples = a_x * a_y;
	m_data           = new ( TPhysicsManager::GetHeap() ) TUINT16[ iNumSamples ];

	for ( TINT i = 0; i < iNumSamples; i++ )
		m_data[ i ] = 0;
}

// $deBlob: FUNCTION 007349b0
void TPhysicsHFContainer::AssertData() const
{
	TASSERT( m_data );
}

// $deBlob: FUNCTION 007349f0
void TPhysicsHFContainer::Set( const TPhysicsHFDesc& a_rDesc )
{
	m_data          = a_rDesc.m_data;
	m_x             = a_rDesc.m_x;
	m_y             = a_rDesc.m_y;
	m_fMinHeight    = a_rDesc.m_fMinHeight;
	m_fMaxHeight    = a_rDesc.m_fMaxHeight;
	m_bExternalData = TTRUE;
	m_fHeightRange  = m_fMaxHeight - m_fMinHeight;
	m_fSizeX        = TFLOAT( m_x );
	m_fSizeY        = TFLOAT( m_y );

	if ( m_fHeightRange < 0.001f )
		m_fHeightRange = 0.001f;
}

// $deBlob: FUNCTION 00734a60
void TPhysicsHFContainer::SetHeight( TINT a_x, TINT a_y, TFLOAT a_fHeight )
{
	TASSERT( m_data );

	TASSERT( a_x >= 0 );
	TASSERT( a_x < m_x );

	TASSERT( a_y >= 0 );
	TASSERT( a_y < m_y );

	// NOTE: retail overwrites the whole sample, the material index (high 3 bits) is cleared
	m_data[ a_x + a_y * m_x ] = TUINT16( TINT( ( a_fHeight - m_fMinHeight ) / m_fHeightRange * 4096.0f ) );
}

// $deBlob: FUNCTION 00734bb0
TFLOAT TPhysicsHFContainer::GetHeight( TINT a_x, TINT a_y ) const
{
	TASSERT( m_data );

	TASSERT( a_x >= 0 );
	TASSERT( a_x < m_x );

	TASSERT( a_y >= 0 );
	TASSERT( a_y < m_y );

	return TFLOAT( m_data[ a_x + a_y * m_x ] & 0x1FFF ) * ( 1.0f / 4096.0f ) * m_fHeightRange + m_fMinHeight;
}

// $deBlob: FUNCTION 00734d10
void TPhysicsHFContainer::SetMaterialIndex( TINT a_x, TINT a_y, TINT a_iIndex )
{
	TASSERT( m_data );

	TASSERT( a_x >= 0 );
	TASSERT( a_x < m_x );

	TASSERT( a_y >= 0 );
	TASSERT( a_y < m_y );

	TINT iSample = a_x + a_y * m_x;
	m_data[ iSample ] &= 0x1FFF;
	m_data[ iSample ] |= TUINT16( a_iIndex << 13 );
}

// $deBlob: FUNCTION 00734e50
TINT TPhysicsHFContainer::GetMaterialIndex( TINT a_x, TINT a_y ) const
{
	TASSERT( m_data );

	TASSERT( a_x >= 0 );
	TASSERT( a_x < m_x );

	TASSERT( a_y >= 0 );
	TASSERT( a_y < m_y );

	return m_data[ a_x + a_y * m_x ] >> 13;
}

// $deBlob: FUNCTION 00734f80
void TPhysicsHFContainer::SetMaterial( TINT a_index, TFLOAT a_fUnk0, TFLOAT a_fUnk4, TFLOAT a_fUnk8 )
{
	TASSERT( a_index >= 0 );
	TASSERT( a_index < 8 );

	m_aMaterials[ a_index ].m_fUnk0 = a_fUnk0;
	m_aMaterials[ a_index ].m_fUnk4 = a_fUnk4;
	m_aMaterials[ a_index ].m_fUnk8 = a_fUnk8;
}

// $deBlob: FUNCTION 00735020
void TPhysicsHFContainer::GetMaterial( TINT a_index, TFLOAT& a_rfUnk0, TFLOAT& a_rfUnk4, TFLOAT& a_rfUnk8 ) const
{
	TASSERT( a_index >= 0 );
	TASSERT( a_index < 8 );

	a_rfUnk0 = m_aMaterials[ a_index ].m_fUnk0;
	a_rfUnk4 = m_aMaterials[ a_index ].m_fUnk4;
	a_rfUnk8 = m_aMaterials[ a_index ].m_fUnk8;
}

// $deBlob: FUNCTION 007350c0
TVector4 TPhysicsHFContainer::GetDownVector() const
{
	return TVector4( 0.0f, -1.0f, 0.0f, 1.0f );
}

// $deBlob: FUNCTION 007350f0
TFLOAT TPhysicsHFContainer::GetHeightAt( TINT a_x, TINT a_z ) const
{
	return GetHeight( a_x, m_y - a_z - 1 );
}

// $deBlob: FUNCTION 00735110
void TPhysicsHFContainer::GetMaterialAt( TINT a_x, TINT a_z, TFLOAT& a_rfUnk0, TFLOAT& a_rfUnk4 ) const
{
	TFLOAT fUnk8;
	GetMaterial( GetMaterialIndex( a_x, m_y - a_z - 1 ), a_rfUnk0, a_rfUnk4, fUnk8 );
}

TOSHI_NAMESPACE_END
