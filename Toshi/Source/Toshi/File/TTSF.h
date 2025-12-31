#pragma once
#include "Toshi/File/TCompress_Decompress.h"
#include "Toshi/File/TCompress_Compress.h"
#include "Toshi/File/TTRB.h"

TOSHI_NAMESPACE_START

class TTSF
{
public:
	static constexpr TUINT32 IDMAGICB = TFourCC("TSFB");
	static constexpr TUINT32 IDMAGICL = TFourCC("TSFL");

	typedef TUINT8 Endianess;
	enum Endianess_ : Endianess
	{
		Endianess_Little,
		Endianess_Big,
	};

	struct Header
	{
		TUINT32 Magic;
		TUINT32 FileSize;
	};

	struct FileInfo
	{
		TUINT32 FileStartOffset; // offset to TRBF
		TUINT32 FileSize;        // just the size
	};

	struct Hunk
	{
		TUINT32 Name;
		TUINT32 Size;
	};

public:
	TTSF()
	    : m_Endianess(Endianess_Little), m_pFile(TNULL) {}

	Endianess GetEndianess() const { return m_Endianess; }

protected:
	Endianess m_Endianess; // 0x0
	TFile*    m_pFile;     // 0x4
};

class TTSFI : public TTSF
{
public:
	friend TTRB;

public:
	TTSFI();
	~TTSFI() { Close(); }

	// $deBlob: FUNCTION 00688220
	size_t ReadAlignmentPad()
	{
		TASSERT(m_pFile != TNULL, "File is TNULL");
		static TCHAR s_AlignBuffer[4] = { 0, 0, 0, 0 };
		TUINT8       alignValue       = 4 - (m_pFile->Tell() & 3);

		if (alignValue != 4)
		{
			return m_pFile->Read(s_AlignBuffer, alignValue);
		}

		return 0;
	}

	TTRB::ERROR Open(TFile* a_pFile);

	TTRB::ERROR PushForm();
	TTRB::ERROR PopForm();

	template <class T> void Read(T* dst) { m_ReadPos += m_pFile->Read(dst, sizeof(T)); }

	void ReadRaw(void* dst, TUINT32 size) { m_ReadPos += m_pFile->Read(dst, size); }

	// Sections related stuff
	TTRB::ERROR ReadHunk();
	TTRB::ERROR SkipHunk();
	TTRB::ERROR ReadFORM(TTRB::SectionFORM* section);
	TTRB::ERROR ReadHunkData(void* dest);

	void Close(TBOOL free = TTRUE);

	void ReadCompressed(void* buffer, TUINT32 size);
	void CompressSection(TFile* file, TCHAR* unk, TUINT32 unk2, TUINT32 unk3) { TCompress_Compress::Compress(file, unk, unk2, unk3, m_Endianess); }

	void LogUnknownSection();

	const Hunk& GetCurrentHunk() { return m_CurrentHunk; }

private:
	TUINT32  m_FileInfoCount; // 0x8
	FileInfo m_FileInfo[32];  // 0xC
	Header   m_Header;        // 0x10C
	TUINT32  m_Magic;         // 0x114
	Hunk     m_CurrentHunk;   // 0x118
	TUINT32  m_ReadPos;       // 0x120
};

class TTSFO : public TTSF
{
public:
	typedef TUINT8 ERROR;
	enum ERROR_ : ERROR
	{
		ERROR_OK,
		ERROR_UNKNOWN,
		ERROR_FILE
	};

	struct HunkMark
	{
		TUINT32 Name;
		TUINT32 Pos;
	};

public:
	TTSFO()
	    : m_PositionCount(0), m_Positions() {}
	~TTSFO() { Close(); }

	/**
        * Aligns current position to 4.
        *
        * @return number of written bytes
        */
	size_t WriteAlignmentPad()
	{
		TASSERT(m_pFile != TNULL, "TTSFO is not created");
		static TCHAR s_AlignBuffer[4] = { 0, 0, 0, 0 };
		TUINT8       alignValue       = 4 - (m_pFile->Tell() & 3);

		if (alignValue != 4)
		{
			return m_pFile->Write(s_AlignBuffer, alignValue);
		}

		return 0;
	}

	/**
        * Creates file for writing.
        *
        * @param filepath - path to the file
        * @param magic - magic value which is placed after the first hunk
        * @return TTRB::ERROR_OK if successful
        */
	TTSFO::ERROR Create(const TCHAR* filepath, const TCHAR* magic = "TRBF", Endianess endianess = Endianess_Little);

	/**
        * Closes the file.
        *
        */
	void Close();

	/**
        * Begin a new form and saves it's info
        *
        * @param name - 4 bytes long value which will be written after the hunk
        * @return number of written bytes
        */
	size_t BeginForm(const TCHAR* name);

	/**
        * Ends the current form if it exists.
        *
        * @return size of the form
        */
	size_t EndForm();

	/**
        * Opens new hunk.
        *
        * @param hunkMark - pointer to TTSFO::HunkMark which has to be opened
        * @param hunkName - 4 bytes long name of the hunk
        * @return always TTRUE
        */
	TBOOL OpenHunk(HunkMark* hunkMark, const TCHAR* hunkName);

	/**
        * Closes the hunk.
        *
        * @param hunkMark - pointer to TTSFO::HunkMark which has to be closed
        * @return always TTRUE
        */
	TBOOL CloseHunk(HunkMark* hunkMark);

	/**
        * Writes a hunk of data.
        *
        * @param hunkName - magic value that is used to identificate hunk (use TMAKEFOUR to get it)
        * @param buffer - pointer to the buffer of data (can be TNULL)
        * @param bufferSize - size of the buffer (can be 0)
        * @return number of written bytes
        */
	size_t WriteHunk(TUINT32 hunkName, void* buffer = TNULL, size_t bufferSize = 0);

	template <class T> size_t Write(const T& value)
	{
		TASSERT(m_pFile != TNULL, "TTSFO is not created");
		return m_pFile->Write(&value, sizeof(T));
	}

	// $deBlob: FUNCTION 00688ed0
	size_t WriteRaw(const void* buffer, size_t size)
	{
		TASSERT(m_pFile != TNULL, "TTSFO is not created");
		return m_pFile->Write(buffer, size);
	}

	// $deBlob: FUNCTION 00688ee0
	size_t WriteCompressed(const void* buffer, size_t size)
	{
		TASSERT(m_pFile != TNULL, "TTSFO is not created");
		size_t writtenSize = TCompress_Compress::Compress(m_pFile, (TCHAR*)buffer, size, 0, m_Endianess == Endianess_Big);
		WriteAlignmentPad();
		return writtenSize;
	}

	// $deBlob: FUNCTION 00688f10
	void WriteBool(TBOOL value) { Write(value); }
	void WriteInt8(int8_t value) { Write(value); }
	void WriteUInt8(TUINT8 value) { Write(value); }
	void WriteUInt16(TUINT16 value) { Write(value); }
	// $deBlob: FUNCTION 00688f30
	void WriteInt32(TINT32 value) { Write(value); }
	// $deBlob: FUNCTION 00688f50
	void WriteUInt32(TUINT32 value) { Write(value); }
	void WriteFloat(TFLOAT value) { Write(value); }
	void WriteVector3(TVector3& value) { Write(value); }
	void WriteVector4(TVector4& value) { Write(value); }
	void WriteQuaternion(TQuaternion& value) { Write(value); }

private:
	size_t m_PositionCount;
	size_t m_Positions[32];
};

TOSHI_NAMESPACE_END
