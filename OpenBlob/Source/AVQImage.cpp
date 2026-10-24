#include "pch.h"
#include "AVQImage.h"

#include <Toshi/TUtil.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005761b0
void AVQImage::WriteTGA( const TCHAR* a_szFileName, TUINT16 a_uiWidth, TUINT16 a_uiHeight, TUINT8* a_pRGBA )
{
	// The TGA header: uncompressed true colour, 32 bits, top-left origin
	TUINT8 aHeader[ 18 ];
	TUtil::MemClear( aHeader, sizeof( aHeader ) );
	aHeader[ 2 ]                                   = 2;
	*TREINTERPRETCAST( TUINT16*, &aHeader[ 12 ] ) = a_uiWidth;
	*TREINTERPRETCAST( TUINT16*, &aHeader[ 14 ] ) = a_uiHeight;
	*TREINTERPRETCAST( TUINT16*, &aHeader[ 16 ] ) = 0x2020;

	TFile* pFile = TFile::Create( a_szFileName, TFILEMODE_WRITE | TFILEMODE_CREATENEW );

	if ( pFile != TNULL )
	{
		pFile->Write( aHeader, sizeof( aHeader ) );

		// RGBA to BGRA
		TINT iSize = TINT( a_uiWidth ) * TINT( a_uiHeight ) * 4;

		for ( TINT i = 0; i < iSize; i += 4 )
		{
			TUINT8 uiRed       = a_pRGBA[ i ];
			a_pRGBA[ i ]       = a_pRGBA[ i + 2 ];
			a_pRGBA[ i + 2 ]   = uiRed;
		}

		pFile->Write( a_pRGBA, iSize );
		pFile->Destroy();
	}
}

// $deBlob: FUNCTION 005762b0
AVQImage::AVQImage()
{
	m_iWidth        = 0;
	m_iHeight       = 0;
	m_uiUnk0C       = 0;
	m_iImageSize    = 0;
	m_iCodeBookSize = 0;
	m_uiFormat      = 0;
	m_pCodeBook     = TNULL;
	m_pImage        = TNULL;
}

// $deBlob: FUNCTION 00576300
// $deBlob: FUNCTION 005766d0
AVQImage::~AVQImage()
{
	TFree( m_pCodeBook );
	TFree( m_pImage );
}

// $deBlob: FUNCTION 00576320
TBOOL AVQImage::Load( const TString8& a_rFileName )
{
	TFile* pFile = TFile::Create( a_rFileName, TFILEMODE_READ );

	if ( pFile == TNULL )
		return TFALSE;

	Load( pFile );
	pFile->Destroy();
	return TTRUE;
}

// $deBlob: FUNCTION 00576360
TBOOL AVQImage::Load( TFile* a_pFile )
{
	TFree( m_pCodeBook );
	TFree( m_pImage );
	m_pCodeBook = TNULL;
	m_pImage    = TNULL;

	TINT    iFileSize = TINT( a_pFile->GetSize() );
	TUINT8* pBuf      = new TUINT8[ iFileSize + 2 ];
	TINT    iSize     = TINT( a_pFile->Read( pBuf, iFileSize ) );

	if ( iSize == 0 )
	{
		delete[] pBuf;
		return TFALSE;
	}

	// NOTE: retail terminates one byte past the data
	pBuf[ iSize + 1 ] = 0;

	const Header* pHeader = TREINTERPRETCAST( const Header*, pBuf );
	m_iWidth              = pHeader->m_iWidth;
	m_iHeight             = pHeader->m_iHeight;
	m_uiUnk0C             = pHeader->m_uiUnk08;
	m_iCodeBookSize       = pHeader->m_iCodeBookSize;
	m_iImageSize          = pHeader->m_iImageSize;
	m_uiFormat            = pHeader->m_uiFormat;

	// NOTE: the image buffer is width * height bytes while m_iImageSize bytes are copied into it
	m_pCodeBook = TSTATICCAST( TUINT8, TMalloc( m_iCodeBookSize ) );
	m_pImage    = TSTATICCAST( TUINT8, TMalloc( m_iHeight * m_iWidth ) );

	TUINT8* pBufPos = pBuf + sizeof( Header );

	for ( TINT i = 0; i < m_iCodeBookSize; i++ )
	{
		TASSERT( ( &pBufPos[ i ] - pBuf ) < static_cast<TINT>( iSize ) );
		m_pCodeBook[ i ] = pBufPos[ i ];
	}

	pBufPos = pBuf + sizeof( Header ) + m_iCodeBookSize;

	for ( TINT i = 0; i < m_iImageSize; i++ )
	{
		TASSERT( ( &pBufPos[ i ] - pBuf ) < static_cast<TINT>( iSize ) );
		m_pImage[ i ] = pBufPos[ i ];
	}

	delete[] pBuf;
	return TTRUE;
}

// $deBlob: FUNCTION 005764e0
void AVQImage::Save( const TString8& a_rFileName )
{
	TASSERT( m_pCodeBook != TNULL );
	TASSERT( m_pImage != TNULL );

	TFile* pFile = TFile::Create( a_rFileName, TFILEMODE_CREATENEW );

	if ( pFile == TNULL )
		return;

	// Written through a 1 KB buffer that starts with the header
	TUINT8  aBuffer[ 0x400 ];
	Header* pHeader         = TREINTERPRETCAST( Header*, aBuffer );
	pHeader->m_iWidth        = m_iWidth;
	pHeader->m_iHeight       = m_iHeight;
	pHeader->m_uiUnk08       = m_uiUnk0C;
	pHeader->m_iCodeBookSize = m_iCodeBookSize;
	pHeader->m_iImageSize    = m_iImageSize;
	pHeader->m_uiFormat      = m_uiFormat;

	TUINT8* pPos = aBuffer + sizeof( Header );
	TUINT8* pEnd = aBuffer + sizeof( aBuffer );

	if ( m_uiFormat == 0 )
	{
		// RGBA code book entries, the alpha halved (rounded up)
		for ( TINT i = 0; i < m_iCodeBookSize; i += 4 )
		{
			const TUINT8* pEntry = m_pCodeBook + i;
			pPos[ 0 ]            = pEntry[ 0 ];
			pPos[ 1 ]            = pEntry[ 1 ];
			pPos[ 2 ]            = pEntry[ 2 ];
			pPos[ 3 ]            = TUINT8( ( pEntry[ 3 ] + 1 ) >> 1 );
			pPos += 4;

			if ( pPos == pEnd )
			{
				pFile->Write( aBuffer, pPos - aBuffer );
				pPos = aBuffer;
			}
		}
	}
	else
	{
		for ( TINT i = 0; i < m_iCodeBookSize; i++ )
		{
			*pPos++ = m_pCodeBook[ i ];

			if ( pPos == pEnd )
			{
				pFile->Write( aBuffer, pPos - aBuffer );
				pPos = aBuffer;
			}
		}
	}

	for ( TINT i = 0; i < m_iImageSize; i++ )
	{
		*pPos++ = m_pImage[ i ];

		if ( pPos == pEnd )
		{
			pFile->Write( aBuffer, pPos - aBuffer );
			pPos = aBuffer;
		}
	}

	if ( pPos != aBuffer )
		pFile->Write( aBuffer, pPos - aBuffer );

	pFile->Destroy();
}
