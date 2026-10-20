/**
 * @file TFile.h
 * Core file system interface for the Toshi engine
 * 
 * This file defines the base file interface that provides:
 * - File I/O operations (read/write/seek)
 * - File system abstraction
 * - File path handling
 * - File stream management
 */

#pragma once
#include "Toshi/TSingleton.h"
#include "Toshi/TDList.h"
#include "Toshi/TString8.h"

#include "Thread/TMutexLock.h"

#undef CreateFile
#undef FindFirstFile

TOSHI_NAMESPACE_START

TDECLARE_POINTER_HANDLE( TFileLookupHandle );

class TFile;

enum TSEEK
{
	TSEEK_SET,
	TSEEK_CUR,
	TSEEK_END
};

typedef TUINT16 TFILEMODE;
enum TFILEMODE_ : TFILEMODE
{
	TFILEMODE_READ      = BITFLAG( 0 ),
	TFILEMODE_WRITE     = BITFLAG( 1 ),
	TFILEMODE_READWRITE = BITFLAG( 2 ),
	TFILEMODE_CREATENEW = BITFLAG( 3 ),
	TFILEMODE_NOBUFFER  = BITFLAG( 4 ),
};

typedef TUINT8 TFINDFILE;
enum TFINDFILE_ : TFINDFILE
{
	TFINDFILE_FILE      = BITFLAG( 0 ),
	TFINDFILE_DIRECTORY = BITFLAG( 1 ),
};

struct TNativeFileInfo
{
	TString8 InternalPath;
	TUINT32  Offset;
	TUINT32  Length;
};

// JPOG Matched
class TFileSystem : public TDList<TFileSystem>::TNode
{
public:
	TFileSystem( const TCHAR* a_szName );
	TFileSystem( const TFileSystem& other );

	virtual ~TFileSystem(); // +0x00

	/**
         * Creates file.
         * @param a_rcFileName path to the file to open
         * @param a_eFileMode mode to open the file with
         * @return pointer to the opened file or TNULL if couldn't open
         */
	virtual TFile* CreateFile( const TString8& a_rcFileName, TFILEMODE a_eFileMode ) = 0; // +0x04

	/**
         * Destroys specified file.
         */
	virtual void DestroyFile( TFile* a_pFile ) = 0; // +0x08

	/**
         * Removes specified file.
         */
	// $deBlob: FUNCTION 00685d80
	virtual TBOOL RemoveFile( const TString8& a_rcFileName ) { return TTRUE; } // +0x0C

	/**
         * Not documented.
         */
	virtual TString8 MakeInternalPath( const TString8& a_rcPath ) = 0; // +0x10

	/**
         * Finds out what's the first file at the specified path.
         */
	// $deBlob: FUNCTION 00685d90
	virtual TBOOL GetFirstFile( const TString8& a_rcPath, TString8& a_rOutFileName, TFINDFILE a_ui8Flags = TFINDFILE_DIRECTORY | TFINDFILE_FILE ) { return TFALSE; } // +0x14

	/**
         * Not documented.
         */
	// $deBlob: FUNCTION 00685da0
	virtual TBOOL GetNextFile( TString8& a_rOutFileName, TFINDFILE a_ui8Flags = TFINDFILE_DIRECTORY | TFINDFILE_FILE ) { return TFALSE; } // +0x18

	/**
         * Sets the directory prefix to work with.
         */
	virtual void SetPrefix( const TString8& a_rcPrefix ); // +0x1C

	/**
         * Fetches info about the path and returns internal path.
         */
	virtual TBOOL ToNative( const TString8& a_rcPath, TNativeFileInfo& a_rOutInfo ); // +0x20

	// $deBlob: FUNCTION 00686500
	const TString8& GetName() const { return m_Name; }
	const TString8& GetPrefix() const { return m_Prefix; }

	TFileSystem& operator=( TFileSystem& a_rFileSystem );

protected:
	TString8 m_Name;   // 0x0C
	TString8 m_Prefix; // 0x18
	                   // NOTE: de Blob: 0x24 bytes (TNullFileSystem is allocated with 36); the find handle lives in TNativeFileSystem
};

class TFile
{
public:
	TFile( TFileSystem* a_pFS );
	TFile( const TFile& a_rOther );

	/**
         * Reads specified number of bytes from the file into the buffer.
         * @param a_pDst destination buffer
         * @param a_uiSize number of bytes to read
         * @return the number of read bytes
         */
	virtual TSIZE Read( void* a_pDst, TSIZE a_uiSize ) = 0;

	/**
         * Writes specified number of bytes to the file from the buffer.
         * @param a_pSrc sourde buffer
         * @param a_uiSize number of bytes to write
         * @return the number of written bytes
         */
	virtual TSIZE Write( const void* a_pSrc, TSIZE a_uiSize ) = 0;

	/**
         * Shifts current file cursor based on the specified offset and origin.
         * @param a_iOffset number of characters to shift the position
         * @param a_eOrigin position to which a_iOffset is added
         * @return TTRUE if succeeded
         */
	virtual TBOOL Seek( TINT a_iOffset, TSEEK a_eOrigin = TSEEK_CUR ) = 0;

	/**
         * @return current position of the file cursor
         */
	virtual TSIZE Tell() = 0;

	/**
          * Flushes write buffers.
          */
	// $deBlob: FUNCTION 006864d0
	virtual void FlushBuffers() {}

	/**
         * @return size of the file
         */
	virtual TSIZE GetSize() = 0;

	/**
         * @return last write time
         */

	// $deBlob: FUNCTION 006864e0
	virtual TUINT64 GetDate() { return 0; }

	/**
         * Reads one character of type TCHAR from the file.
         * @return the read character
         */
	virtual TINT GetCChar() = 0;

	/**
         * Reads one character of type wchar from the file.
         * @return the read character
         */
	virtual TINT GetWChar() = 0;

	/**
         * Writes one character of type TCHAR to the file.
         * @return code of the written character
         */
	virtual TINT PutCChar( TCHAR a_cCharacter ) = 0;

	/**
         * Writes one character of type wchar to the file.
         * @return number of written bytes
         */
	virtual TINT PutWChar( TWCHAR a_wcCharacter ) = 0;

	/**
         * Analogue of printf but writes result to the file.
         * @return number of characters written
         */
	virtual TINT VCPrintf( const TCHAR* a_szFormat, va_list a_vargs ) = 0;

	/**
         * Analogue of wprintf but writes result to the file.
         * @return number of characters written
         */
	virtual TINT VWPrintf( const TWCHAR* a_wszFormat, va_list a_vargs ) = 0;

	virtual ~TFile();

	TINT CPrintf( const TCHAR* a_szFormat, ... );

	// $deBlob: FUNCTION 00685ff0
	TINT WPrintf( const TWCHAR* a_wszFormat, ... )
	{
		va_list args;
		va_start( args, a_wszFormat );
		TINT iResult = VWPrintf( a_wszFormat, args );
		va_end( args );

		return iResult;
	}

	// $deBlob: FUNCTION 006864f0
	TFileSystem* GetFileSystem() const
	{
		return m_pFileSystem;
	}

	void Destroy();

	// Reads a line (skipping empty lines) into a_pBuffer, returns the number of read characters
	TINT ReadLine( TCHAR* a_pBuffer, TINT a_iSize );

public:
	static TString8 ConcatPath( const TString8& a_rcPath1, const TString8& a_rcPath2 );
	static TString8 SimplifyPath( const TString8& a_rcPath );
	static void     SplitPath( const TString8& a_rcPath, TString8& a_rDirectory, TString8& a_rFileName );

	static TFile* Create( const TString8& a_rcFilename, TFILEMODE a_eMode = TFILEMODE_READ );
	static void   Destroy( TFile* a_pFile );
	static TBOOL  ToNative( const TString8& a_rcFileName, TNativeFileInfo& a_rNativeInfo );
	static void   PrintFileAccess( TBOOL a_bPrint );

private:
	TFileSystem* m_pFileSystem; // 0x04
};

class TFileManager : public TSingleton<TFileManager>
{
public:
	class TSysPathIter
	{
	public:
		TSysPathIter( const TString8& str );
		;
		TSysPathIter( const TSysPathIter& other );
		;

		TBOOL First( TString8& path );

		TBOOL Next( TString8& path );

	private:
		const TString8& m_String;
		int32_t         m_Position;
	};

public:
	TFileManager();
	~TFileManager();

	static void Destroy();
	void        MountFileSystem( TFileSystem* a_pFileSystem );

	// NOTE: retail calls this on the manager (the body doesn't use it)
	void UnmountFileSystem( TFileSystem* a_pFileSystem );

	TFile* CreateFile( const TString8& a_sName, TFILEMODE flags );
	TBOOL  ToNative( const TString8& a_sFileName, TNativeFileInfo& a_rNativeInfo );

	TFileSystem*        FindFileSystem( const TString8& name );
	static TFileSystem* FindFileSystem( TDList<TFileSystem>& list, const TString8& name );

	TString8 MakeAbsolutePath( const TString8& a_rcRelativePath ) const;
	void     SetWorkingDirectory( const TString8& a_rcPath );

	// $deBlob: FUNCTION 00685600
	static void DestroyFile( TFile* a_pFile )
	{
		if ( a_pFile )
			a_pFile->GetFileSystem()->DestroyFile( a_pFile );
	}

	// $deBlob: FUNCTION 006857a0
	// $Barnyard: FUNCTION 006be1a0
	void SetSystemPath( const TString8& name )
	{
		m_SysPath = name;
		InvalidateSystemPath();
	}

	// $deBlob: FUNCTION 00685790
	const TString8& GetSystemPath() const { return m_SysPath; }
	// $deBlob: FUNCTION 006857c0
	const TString8& GetWorkingDirectory() const { return m_WorkingDirectory; }

public:
	/*
        * Platform specific methods
        * Define them in TFileManager_{Platform}.cpp
        */

	static TBOOL Create();

public:
	// $deBlob: FUNCTION 00685990
	void FileSystemRelease() { m_pMutex->Unlock(); }

	// $deBlob: FUNCTION 00685980
	void FileSystemWait() { m_pMutex->Lock(); }

private:
	void ValidateSystemPath();
	void InvalidateSystemPath() { m_IsValidated = TFALSE; }

	static TBOOL CreateCommon();
	static void DestroyCommon();

private:
	TBOOL               m_IsValidated;      // 0x0
	TString8            m_SysPath;          // 0x4
	TString8            m_WorkingDirectory; // 0x10
	TUINT32             m_ValidatedCount;   // 0x1C
	TDList<TFileSystem> m_Validated;        // 0x20
	TDList<TFileSystem> m_Invalidated;      // 0x28
	TMutex*             m_pMutex;           // 0x30
};

TOSHI_NAMESPACE_END
