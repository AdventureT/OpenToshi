#include "ToshiPCH.h"
#include "TError.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00663440
TError::TError( TINT a_iStringBufferSize, TINT a_iMaxErrors )
{
	m_iStringBufferSize = a_iStringBufferSize;
	m_iMaxErrors        = a_iMaxErrors;
	m_pStringBuffer     = new TCHAR[ a_iStringBufferSize ];
	m_pErrorOffsets     = new TINT[ a_iMaxErrors ];
	m_iStringBufferUsed = 0;
	m_iNumErrors        = 0;
}

// $deBlob: FUNCTION 006634d0
TError::~TError()
{
	delete[] m_pStringBuffer;
	delete[] m_pErrorOffsets;
}

// $deBlob: FUNCTION 00663530
const TCHAR* TError::GetError( TINT a_iIndex )
{
	if ( a_iIndex < 0 || a_iIndex >= m_iNumErrors )
		return "invalid error id";

	return m_pStringBuffer + m_pErrorOffsets[ a_iIndex ];
}

// $deBlob: FUNCTION 00663560
void TError::AddError( const TCHAR* a_szFormat, ... )
{
	if ( m_iNumErrors < m_iMaxErrors && m_iStringBufferUsed < m_iStringBufferSize )
	{
		m_pErrorOffsets[ m_iNumErrors ] = m_iStringBufferUsed;

		TCHAR szError[ 1024 ];

		va_list args;
		va_start( args, a_szFormat );
		TINT iLength = T2String8::FormatV( szError, sizeof( szError ), a_szFormat, args );
		va_end( args );

		if ( m_iStringBufferUsed + iLength <= m_iStringBufferSize )
		{
#ifdef DEBLOB_COMMUNITY_PATCH
			// NOTE: retail copies the characters without the terminator, but skips a byte for it (see Docs/RetailBugs.md)
			TUtil::MemCopy( m_pStringBuffer + m_iStringBufferUsed, szError, iLength );

			if ( m_iStringBufferUsed + iLength < m_iStringBufferSize )
				m_pStringBuffer[ m_iStringBufferUsed + iLength ] = '\0';
#else
			TUtil::MemCopy( m_pStringBuffer + m_iStringBufferUsed, szError, iLength );
#endif

			if ( iLength > 0 )
			{
				m_iNumErrors++;
				m_iStringBufferUsed += iLength + 1;
			}
		}
	}
}

TOSHI_NAMESPACE_END
