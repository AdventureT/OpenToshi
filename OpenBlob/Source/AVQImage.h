#pragma once
#include <Toshi/TString8.h>
#include <File/TFile.h>

//-----------------------------------------------------------------------------
// Purpose: a vector quantised image: a code book of colour blocks and the block indices of the image, loaded from and
// saved to a file with a 6 word header, plus a helper writing an RGBA picture as a TGA file.
// NOTE: retail never creates one nor calls any of its functions (dead code).
//-----------------------------------------------------------------------------
class AVQImage
{
public:
	// The file header (6 words)
	struct Header
	{
		TINT    m_iWidth;        // 0x00
		TINT    m_iHeight;       // 0x04
		TUINT32 m_uiUnk08;       // 0x08
		TINT    m_iCodeBookSize; // 0x0C
		TINT    m_iImageSize;    // 0x10
		TUINT32 m_uiFormat;      // 0x14 0: the code book is RGBA (its alpha is halved when saved)
	};

public:
	AVQImage();
	virtual ~AVQImage(); // +0x00

	// Reads the file a_rFileName (FALSE if it can't be opened)
	TBOOL Load( const Toshi::TString8& a_rFileName );
	// Reads the header, the code book and the image from a_pFile (FALSE if nothing could be read)
	TBOOL Load( Toshi::TFile* a_pFile );
	// Writes the header, the code book and the image to a_rFileName
	void Save( const Toshi::TString8& a_rFileName );

	// Writes a_pRGBA (a_uiWidth * a_uiHeight pixels, swapped to BGRA in place) as a 32-bit TGA file
	static void WriteTGA( const TCHAR* a_szFileName, TUINT16 a_uiWidth, TUINT16 a_uiHeight, TUINT8* a_pRGBA );

private:
	TINT    m_iWidth;        // 0x04
	TINT    m_iHeight;       // 0x08
	TUINT32 m_uiUnk0C;       // 0x0C
	TINT    m_iImageSize;    // 0x10
	TINT    m_iCodeBookSize; // 0x14
	TUINT32 m_uiFormat;      // 0x18
	TUINT8* m_pCodeBook;     // 0x1C
	TUINT8* m_pImage;        // 0x20
};
