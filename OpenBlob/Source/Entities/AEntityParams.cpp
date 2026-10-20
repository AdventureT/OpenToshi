#include "pch.h"
#include "AEntityParams.h"

#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT AEntityParams::FindIndex( const TCHAR* a_szName ) const
{
	for ( TINT i = 0; i < m_iNumParams; i++ )
	{
		if ( m_pParams[ i ].m_szName != TNULL && T2String8::CompareNoCase( m_pParams[ i ].m_szName, a_szName ) == 0 )
			return i;
	}

	return -1;
}

const AEntityParam* AEntityParams::FindParam( const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	return ( i >= 0 ) ? &m_pParams[ i ] : TNULL;
}

TBOOL AEntityParams::GetValue( TINT& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	a_rValue = m_pParams[ i ].m_iInt32;
	return TTRUE;
}

TBOOL AEntityParams::GetValue( TFLOAT& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	a_rValue = m_pParams[ i ].m_fFloat;
	return TTRUE;
}

// $deBlob: FUNCTION 007c44b0
TBOOL AEntityParams::GetValue( TBOOL& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	TASSERT( 0 == m_pParams[ i ].m_iInt32 || 1 == m_pParams[ i ].m_iInt32 );
	a_rValue = m_pParams[ i ].m_iInt32 != 0;
	return TTRUE;
}

TBOOL AEntityParams::GetValue( const TCHAR*& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	a_rValue = m_pParams[ i ].m_szString;
	return TTRUE;
}

// $deBlob: FUNCTION 007c45b0
TBOOL AEntityParams::GetValue( TVector4& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	TASSERT( AEntityParam::TYPE_VECTOR4 == m_pParams[ i ].m_iDataType );
	a_rValue = *m_pParams[ i ].m_pVector4;
	return TTRUE;
}

TBOOL AEntityParams::GetValue( TUINT16& a_rHigh, TUINT16& a_rLow, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	TUINT32 uiValue = TUINT32( m_pParams[ i ].m_iInt32 );
	a_rHigh         = TUINT16( uiValue >> 16 );
	a_rLow          = TUINT16( uiValue );
	return TTRUE;
}

// $deBlob: FUNCTION 007c46b0
TBOOL AEntityParams::GetValue( TUINT8& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	TASSERT( 0 <= m_pParams[ i ].m_iInt32 && 0xFF >= m_pParams[ i ].m_iInt32 );
	a_rValue = TUINT8( m_pParams[ i ].m_iInt32 );
	return TTRUE;
}

// $deBlob: FUNCTION 007c4750
TBOOL AEntityParams::GetValue( TINT8& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	TASSERT( -0x80 <= m_pParams[ i ].m_iInt32 && 0x7F >= m_pParams[ i ].m_iInt32 );
	a_rValue = TINT8( m_pParams[ i ].m_iInt32 );
	return TTRUE;
}

// $deBlob: FUNCTION 007c47f0
TBOOL AEntityParams::GetValue( TINT16& a_rValue, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return TFALSE;

	TASSERT( -0x8000 <= m_pParams[ i ].m_iInt32 && 0x7FFF >= m_pParams[ i ].m_iInt32 );
	a_rValue = TINT16( m_pParams[ i ].m_iInt32 );
	return TTRUE;
}

TINT AEntityParams::GetArray( void*& a_rData, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return -1;

	return m_pParams[ i ].GetArray( a_rData );
}

// $deBlob: FUNCTION 007c4990
TINT AEntityParams::GetArray( TUINT32*& a_rData, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return -1;

	return m_pParams[ i ].GetArray( a_rData );
}

// $deBlob: FUNCTION 007c4a30
TINT AEntityParams::GetArray( TINT*& a_rData, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return -1;

	return m_pParams[ i ].GetArray( a_rData );
}

// $deBlob: FUNCTION 007c4ad0
TINT AEntityParams::GetArray( TFLOAT*& a_rData, const TCHAR* a_szName ) const
{
	TINT i = FindIndex( a_szName );
	if ( i < 0 ) return -1;

	return m_pParams[ i ].GetArray( a_rData );
}

// $deBlob: FUNCTION 007c4b70
TBOOL AEntityParams::GetInstances( IDList*& a_rList ) const
{
	for ( TINT i = 0; i < m_iNumParams; i++ )
	{
		if ( m_pParams[ i ].m_iDataType == 5 )
		{
			a_rList = TREINTERPRETCAST( IDList*, m_pParams[ i ].m_pArray );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c4bb0
TBOOL AEntityParams::GetNavSets( IDList*& a_rList ) const
{
	for ( TINT i = 0; i < m_iNumParams; i++ )
	{
		if ( m_pParams[ i ].m_iDataType == 6 )
		{
			a_rList = TREINTERPRETCAST( IDList*, m_pParams[ i ].m_pArray );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c4bf0
TBOOL AEntityParams::GetCameraShakes( IDList*& a_rList ) const
{
	for ( TINT i = 0; i < m_iNumParams; i++ )
	{
		if ( m_pParams[ i ].m_iDataType == 7 )
		{
			a_rList = TREINTERPRETCAST( IDList*, m_pParams[ i ].m_pArray );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c4c30
TBOOL AEntityParams::GetCollision( AEntityParam::Collision**& a_rpCollision ) const
{
	for ( TINT i = 0; i < m_iNumParams; i++ )
	{
		if ( m_pParams[ i ].m_iDataType == AEntityParam::TYPE_COLLISION )
		{
			a_rpCollision = TREINTERPRETCAST( AEntityParam::Collision**, m_pParams[ i ].m_pArray );
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c4c70
TBOOL AEntityParams::GetSoundAction( void*& a_rData ) const
{
	for ( TINT i = 0; i < m_iNumParams; i++ )
	{
		if ( m_pParams[ i ].m_iDataType == AEntityParam::TYPE_SOUNDACTION )
		{
			a_rData = m_pParams[ i ].m_pArray;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007c4cb0
TBOOL AEntityParams::GetRespawnPoint( void*& a_rData ) const
{
	for ( TINT i = 0; i < m_iNumParams; i++ )
	{
		if ( m_pParams[ i ].m_iDataType == AEntityParam::TYPE_RESPAWN )
		{
			a_rData = m_pParams[ i ].m_pArray;
			return TTRUE;
		}
	}

	return TFALSE;
}
