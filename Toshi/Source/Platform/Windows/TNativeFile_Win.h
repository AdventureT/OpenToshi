#pragma once
#include "File/TFile.h"

TOSHI_NAMESPACE_START

// TFileManager::Create allocates 40 bytes for it
class TNativeFileSystem : public TFileSystem
{
public:
	TNativeFileSystem( const TCHAR* name );
	virtual ~TNativeFileSystem();                                                                                                                        // +0x00

	virtual TFile*   CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode ) OVERRIDE;                                                       // +0x04
	virtual void     DestroyFile( TFile* a_pFile ) OVERRIDE;                                                                                            // +0x08
	virtual TBOOL    RemoveFile( const TString8& a_rcFileName ) OVERRIDE;                                                                               // +0x0C
	virtual TString8 MakeInternalPath( const TString8& a_rcPath ) OVERRIDE;                                                                             // +0x10
	virtual TBOOL    GetFirstFile( const TString8& a_rcPath, TString8& a_rOutFileName, TFINDFILE a_ui8Flags = TFINDFILE_DIRECTORY | TFINDFILE_FILE ) OVERRIDE; // +0x14
	virtual TBOOL    GetNextFile( TString8& a_rOutFileName, TFINDFILE a_ui8Flags = TFINDFILE_DIRECTORY | TFINDFILE_FILE ) OVERRIDE;                     // +0x18

	virtual TBOOL MakeDirectory( const TString8& a_rcDirectory ); // +0x24

protected:
	TFileLookupHandle m_NextFileHandle; // 0x24
};

class TNativeFile : public TFile
{
public:
	static TCONSTEXPR_VAR TUINT32 BUFFER_SIZE = 0x800;

public:
	TNativeFile( TNativeFileSystem* a_pFileSystem );
	TNativeFile( const TNativeFile& other );
	~TNativeFile();

	//-----------------------------------------------------------------------------
	// TFile
	//-----------------------------------------------------------------------------
	virtual TSIZE   Read( void* a_pDst, TSIZE a_uiSize ) OVERRIDE;
	virtual TSIZE   Write( const void* a_pSrc, TSIZE a_uiSize ) OVERRIDE;
	virtual TBOOL   Seek( TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR ) OVERRIDE;
	virtual TSIZE   Tell() OVERRIDE;
	virtual void    FlushBuffers() OVERRIDE;
	virtual TSIZE   GetSize() OVERRIDE;
	virtual TUINT64 GetDate() OVERRIDE;
	virtual TINT    GetCChar() OVERRIDE;
	virtual TINT    GetWChar() OVERRIDE;
	virtual TINT    PutCChar( TCHAR a_cCharacter ) OVERRIDE;
	virtual TINT    PutWChar( TWCHAR a_wcCharacter ) OVERRIDE;
	virtual TINT    VCPrintf( const TCHAR* a_szFormat, va_list a_vargs ) OVERRIDE;
	virtual TINT    VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs ) OVERRIDE;

	TBOOL Open( const TString8& a_FileName, TFILEMODE a_Mode );

	void Close();

private:
	TBOOL LoadBuffer( TUINT32 bufferPos );
	TINT  FlushWriteBuffer();
	TINT  ReadUnbuffered( void* dst, TUINT size );

protected:
	void*   m_Handle;            // 0x08
	TINT    m_Position;          // 0x0C
	TINT    m_RBufferPosition;   // 0x10
	TUINT32 m_PrevBufferPos;     // 0x14
	TINT    m_LastBufferSize;    // 0x18
	TCHAR*  m_pBuffer;           // 0x1C (read buffer)
	TCHAR*  m_WBuffer;           // 0x20 (write buffer)
	TINT    m_iWriteBufferUsed;  // 0x24
	TBOOL   m_bWriteBuffered;    // 0x28
};

TOSHI_NAMESPACE_END
