#pragma once
#include "Toshi/Utils/TSingleton.h"
#include "Toshi/Strings/TCString.h"
#include "Toshi/Core/TDList.h"

#ifdef CreateFile
#undef CreateFile
#endif

namespace Toshi
{
    class TFile;

    // JPOG Matched
    class TFileSystem : public TDList<TFileSystem>::TNode
    {
    public:
        TFileSystem(const char* name);
        TFileSystem(const TFileSystem& other);
        virtual ~TFileSystem() { UnmountFileSystem(); }

        // Override these funcs in TNativeFileSystem
        virtual TFile* CreateFile(TCString const& fn, uint32_t flags) = 0;
        virtual void DestroyFile(TFile*) = 0;
        virtual TBOOL RemoveFile(TCString const&) { return TTRUE; }
        virtual TCString MakeInternalPath(TCString const&) = 0;
        virtual TBOOL GetFirstFile(TCString const&, TCString&, unsigned int) { return TFALSE; }
        virtual TBOOL GetNextFile(TCString&, uint32_t) { return TFALSE; }
        virtual void SetPrefix(const TCString& prefix);
        virtual TBOOL MakeDirectory(TCString const&) = 0;

        inline void UnmountFileSystem() { TNode::Remove(); }

        inline TCString const& GetName() const { return m_Name; }
        inline TCString const& GetPrefix() const { return m_Prefix; }

        inline void RemoveNode() { TNode::Remove(); }
        inline TBOOL IsLinked() { return TNode::IsLinked(); }
        inline TFileSystem* PrevNode() { return TNode::Prev()->As<TFileSystem>(); }
        inline TFileSystem* NextNode() { return TNode::Next()->As<TFileSystem>(); }

        TFileSystem& operator=(TFileSystem& a_rFileSystem);

    protected:
        TCString m_Name;   // 0xC
        TCString m_Prefix; // 0x18
        HANDLE m_Handle;   // 0x1C
    };

    class TNativeFileSystem : public TFileSystem
    {
    public:
        TNativeFileSystem(const char* name);

        virtual TFile* CreateFile(TCString const& fn, uint32_t flags) override;
        virtual void DestroyFile(TFile*) override;
        virtual TCString MakeInternalPath(TCString const&) { return {}; }
        virtual TBOOL MakeDirectory(TCString const&) override;
        virtual TBOOL GetNextFile(TCString& fileName, uint32_t flags);
    };

    class TFile
    {
    public:
        enum TSEEK
        {
            TSEEK_SET,
            TSEEK_CUR,
            TSEEK_END
        };

        typedef uint16_t FileMode;
        enum FileMode_ : FileMode
        {
            FileMode_Read = BITFIELD(0),
            FileMode_Write = BITFIELD(1),
            FileMode_ReadWrite = BITFIELD(2),
            FileMode_CreateNew = BITFIELD(3),
            FileMode_NoBuffer = BITFIELD(4),
        };

        TFile(TFileSystem* pFS);
        TFile(const TFile& other);

        virtual size_t Read(void* dst, size_t size) = 0;           //0x0
        virtual size_t Write(const void* buffer, size_t size) = 0; //0x4
        virtual TBOOL Seek(int offset, TFile::TSEEK origin) = 0;    //0x8
        virtual uint32_t Tell() = 0;                               //0xC
        virtual DWORD GetSize() = 0;                               //0x10
        virtual _FILETIME GetDate() { return {}; }                 //0x14
        virtual int GetCChar() = 0;
        virtual wchar_t GetWChar() = 0;
        virtual int PutCChar(char character) = 0;
        virtual int PutWChar(wchar_t character) = 0;
        virtual int CPrintf(const char* format, ...) = 0;
        virtual int WPrintf(const wchar_t* format, ...) = 0;
        virtual int VCPrintf(const char* format, va_list* vargs) = 0;
        virtual int VWPrintf(const wchar_t* format, ...) = 0;
        virtual ~TFile() { }

        static TCString ConcatPath(const TCString& a_rcString, const TCString& a_rcString2);
        static TFile* Create(const TCString& filename, FileMode mode = FileMode_Read);
        inline TFileSystem* GetFileSystem() const { return m_pFileSystem; }
        inline TFile& operator=(const TFile& a_pFile) { m_pFileSystem = a_pFile.GetFileSystem(); return *this; }

        // FUN_00685f60
        inline void Destroy() { m_pFileSystem->DestroyFile(this); }

    private:
        TFileSystem* m_pFileSystem;
    };

    class TFileManager : public TSingleton<TFileManager>
    {
    public:
        class TSysPathIter
        {
        public:
            TSysPathIter(const TCString& str) : m_String(str), m_Position(-1) { };
            TSysPathIter(const TSysPathIter& other) : m_String(other.m_String), m_Position(other.m_Position) { };

            TBOOL First(TCString& path)
            {
                if (m_String.Length() > 0)
                {
                    m_Position = m_String.Find(';', 0);
                    path.Copy(m_String, m_Position);

                    return TTRUE;
                }
                else
                {
                    m_Position = -1;
                    return TFALSE;
                }
            }

            TBOOL Next(TCString& path)
            {
                if (m_Position >= 0)
                {
                    int strStart = m_Position + 1;
                    m_Position = m_String.Find(';', strStart);

                    path.Copy(
                        m_String.GetString(strStart),
                        (m_Position >= 0) ? (m_Position - strStart) : -1
                    );

                    return TTRUE;
                }
                else
                {
                    return TFALSE;
                }
            }

        private:
            const TCString& m_String;
            int32_t m_Position;
        };

    public:
        TFileManager() : m_WorkingDirectory("/"), m_ValidatedCount(0), m_Mutex() { InvalidateSystemPath(); }
        ~TFileManager() { Destroy(); }

        void Destroy();
        void MountFileSystem(TFileSystem* a_pFileSystem);

        TFile* CreateFile(const TCString& a_sName, uint32_t flags);

        TFileSystem* FindFileSystem(const TCString& name);
        static TFileSystem* FindFileSystem(TDList<TFileSystem>& list, const TCString& name);

        inline TCString MakeAbsolutePath(const TCString& a_cString) const { return TFile::ConcatPath(a_cString, m_WorkingDirectory); }
        inline void FileSystemRelease() { m_Mutex.Unlock(); }
        inline void FileSystemWait() { m_Mutex.Lock(); }

        inline void SetSystemPath(const TCString& name) { m_SysPath = name; InvalidateSystemPath(); }

        static void DestroyFile(TFile* pFile)
        {
            if (pFile)
            {
                pFile->Destroy();
            }
        }

        static void CreateCommon()
        {
            auto fileManager = TFileManager::CreateSingleton();
            fileManager->m_Mutex.Create();
        }

        static void DestroyCommon() { delete TFileManager::GetSingletonSafe(); }

    public:
        /*
        * Platform specific methods
        * Define them in TNativeFile_{Platform}.cpp
        */

        static TBOOL Create();

    private:
        void ValidateSystemPath();
        inline void InvalidateSystemPath() { m_IsValidated = TFALSE; }

    private:
        TBOOL m_IsValidated;                // 0x0
        TCString m_SysPath;                // 0x4
        TCString m_WorkingDirectory;       // 0x10
        uint32_t m_ValidatedCount;         // 0x1C
        TDList<TFileSystem> m_Validated;   // 0x20
        TDList<TFileSystem> m_Invalidated; // 0x28
        T2Mutex m_Mutex;                   // 0x30
    };

    class TNullFileSystem : public TFileSystem, public TSingleton<TNullFileSystem>
    {
    public:
        TNullFileSystem(const char* name) : TFileSystem(name)
        {
            TFileManager::GetSingletonSafe()->MountFileSystem(this);
        }

        // Inherited via TFileSystem
        virtual TFile* CreateFile(TCString const& fn, uint32_t flags) override;
        virtual void DestroyFile(TFile*) override;
        virtual TCString MakeInternalPath(TCString const&) override;
        virtual TBOOL MakeDirectory(TCString const&) override;
    };
}

