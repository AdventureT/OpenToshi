#pragma once
#include "Toshi/Typedefs.h"
#include "Toshi/File/TFile.h"

TOSHI_NAMESPACE_START

class TNativeFile : public TFile
{
public:
	static constexpr TUINT32 BUFFER_SIZE = 0x800;

public:
	TNativeFile(const TNativeFile& other);

	virtual size_t    Read(void* dst, size_t size) override;
	virtual size_t    Write(const void* buffer, size_t size) override;
	virtual TBOOL     Seek(TINT offset, TFile::TSEEK seek) override;
	virtual TUINT32   Tell() override;
	virtual DWORD     GetSize() override;
	virtual _FILETIME GetDate() override;
	virtual TINT      GetCChar() override;
	virtual TWCHAR    GetWChar() override;
	// $deBlob: FUNCTION 00689c30
	virtual TINT      PutCChar(TCHAR character) { return 0; }
	// $deBlob: FUNCTION 00689c60
	virtual TINT      PutWChar(TWCHAR character) { return 0; }
	virtual TINT      CPrintf(const TCHAR* format, ...);
	virtual TINT      WPrintf(const TWCHAR* format, ...);
	virtual TINT      VCPrintf(const TCHAR* format, va_list vargs);
	virtual TINT      VWPrintf(const TWCHAR* format, ...) { return 0; }

	TBOOL LoadBuffer(DWORD bufferPos);
	TINT  FlushWriteBuffer();
	TINT  ReadUnbuffered(LPVOID dst, size_t size);

	static TCHAR* SplitPath(const TString8& a_rsFullPath, TString8& a_rsDrive, TString8& a_rsDir);
	static TBOOL  DirExists(TCHAR* a_pcStr);

protected:
	TNativeFile(TNativeFileSystem* pFS);

	TBOOL Open(const TString8& a_FileName, FileMode a_Mode);
	void  Close();

	friend TNativeFileSystem;

private:
	HANDLE m_Handle;          // 0x8
	DWORD  m_Position;        // 0xC
	DWORD  m_RBufferPosition; // 0x10
	DWORD  m_PrevBufferPos;   // 0x14
	DWORD  m_LastBufferSize;  // 0x18
	TCHAR* m_RBuffer;         // 0x1C (read buffer)
	TCHAR* m_WBuffer;         // 0x20 (write buffer)
	DWORD  m_WriteBufferUsed; // 0x24
	TBOOL  m_WriteBuffered;   // 0x28
};

TOSHI_NAMESPACE_END
