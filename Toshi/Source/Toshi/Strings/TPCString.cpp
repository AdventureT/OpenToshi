#include "ToshiPCH.h"
#include "TPCString.h"

namespace Toshi {

	TCStringPool::TCStringPool(int a_iMaxSize, int a_iCapacity)
	{
		m_iStringCount = 0;
		m_iMaxSize = a_iMaxSize;
		m_iCapacity = a_iCapacity;

		if (m_iCapacity < 1)
		{
			m_oStringPool.m_pCStringPool = TNULL;
		}
		else
		{
			m_oStringPool.m_pCStringPool = new StringPool();
			m_oStringPool.m_pCStringPool->m_szStrings = new TCString[a_iCapacity];
			m_oStringPool.m_iCountOfPools = m_iCapacity;
		}
	}

	TCStringPool::TCStringPool(const char* a_szFileName, int unk, int unk2) : TCStringPool(unk, unk2)
	{
		ReadFile(a_szFileName);
	}

	void TCStringPool::Remove(TPooledCString& a_pcString)
	{
		TIMPLEMENT();
	}

	TBOOL TCStringPool::ReadFile(const char* a_szFileName)
	{
		TFileManager* fileManager = TFileManager::GetSingleton();
		TFile* file = fileManager->CreateFile(a_szFileName, 1);

		if (file == TNULL)
		{
			return TFALSE;
		}

		int stringCount = 0;
		file->Read(&stringCount, 4);

		int stringsSize = 0;
		file->Read(&stringsSize, 4);

		if (stringCount != 0 && stringsSize != 0)
		{
			auto strings = new char* [stringsSize];
			int read = file->Read(strings, stringsSize);
			fileManager->DestroyFile(file);

			if (read != stringsSize)
			{
				delete[] strings;
				return TFALSE;
			}

			if (m_iCapacity < stringCount)
			{
				InitStringPool(stringCount);
			}
			m_iStringCount = stringCount;

			StringPool* pool = new StringPool[m_iStringCount];

			for (size_t i = 0; i < m_iStringCount; i++)
			{
				pool[i].m_szStrings = new TCString();
				pool[i].count = 0;
			}

			TTODO("WTH");

		}

		return TFALSE;
	}

	void TCStringPool::InitStringPool(int a_iStringCount)
	{
		// 0x10021540 JPOG

		if (a_iStringCount == 0)
		{
			if (m_oStringPool.m_pCStringPool != TNULL)
			{
				delete[] m_oStringPool.m_pCStringPool;
			}
			m_oStringPool.m_pCStringPool = TNULL;
			m_iCapacity = 0;
			m_iStringCount = 0;
		}
		else
		{
			m_oStringPool.m_pCStringPool = new StringPool[a_iStringCount];
			m_oStringPool.m_iCountOfPools++;

			int stringCount = a_iStringCount < m_iStringCount ? a_iStringCount : m_iStringCount;

			for (int i = 0; i < stringCount; i++)
			{
				TIMPLEMENT();
				//m_oStringPool.m_pCStringPool[i] = new StringPool();
			}

		}

	}



	int Toshi::TPCString::Compare(const TPCString& other) const
	{
		TPooledCString* thisStringPool = GetPtr();
		TPooledCString* otherStringPool = other.GetPtr();

		if (!thisStringPool && otherStringPool) return -1;
		if (!otherStringPool) return 1;
		if (thisStringPool != otherStringPool)
		{
			return thisStringPool->m_oString.Compare(otherStringPool->m_oString, -1);
		}
		return 0;
	}

}
