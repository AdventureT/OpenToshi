#pragma once
#include <Toshi/Strings/TCString.h>
#include "Toshi/Strings/TPooledCString.h"

namespace Toshi
{
	class TPooledCString;

	class TCStringPool
	{
	public:
		TCStringPool()
		{
			TCStringPool(1024, 0);
		}

		TCStringPool(int unk, int unk2);
		TCStringPool(const char* a_szFileName, int unk, int unk2);

		struct StringPool
		{
			int count;
			TCString* m_szStrings;
		};

		struct StringPools
		{
			StringPool* m_pCStringPool;
			int m_iCountOfPools;
		};

	public:

		void Remove(TPooledCString& a_pcString);
	protected:
		TBOOL ReadFile(const char* a_szFileName);
		void InitStringPool(int a_iStringCount);

	private:
		int m_iMaxSize;           // 0x0
		int m_iStringCount;       // 0x4
		int m_iCapacity;          // 0x8
		StringPools m_oStringPool;// 0xC
	};

	class TPCString
	{
	public:
		TPCString() 
		{ 
			m_pPooledString = TNULL; 
		}

		TPCString(const TPCString& other) : TPCString()
		{
			if (other.GetStringPool())
			{
				m_pPooledString = other.m_pPooledString;
				m_pPooledString->m_iCount++;
			}
		}

		TPCString(TPooledCString* a_rPooledCString)
		{
			m_pPooledString = a_rPooledCString;
			if (m_pPooledString)
			{
				m_pPooledString->m_iCount++;
			}
		}

		~TPCString()
		{
			if (m_pPooledString)
			{
				m_pPooledString->m_iCount--;
				if (m_pPooledString->m_iCount == 0) m_pPooledString->Delete();
				m_pPooledString = TNULL;
			}
		}

		operator const TCString* () const
		{
			if (m_pPooledString)
			{
				return &GetPtr()->m_oString;
			}
			return &ms_sEmpty;
		}

		bool operator==(const TPCString& other) const
		{
			return GetPtr() == other.GetPtr();
		}

		bool operator!=(const TPCString& other) const
		{
			return GetPtr() != other.GetPtr();
		}

		TPCString& operator=(const TPCString& other)
		{
			if (m_pPooledString != other.m_pPooledString)
			{
				if (GetPtr())
				{
					m_pPooledString->m_iCount--;
					if (m_pPooledString->m_iCount == 0) m_pPooledString->Delete();
				}
				m_pPooledString = other.m_pPooledString;
				if (other.m_pPooledString)
				{
					m_pPooledString->m_iCount++;
				}
			}

			return *this;
		}

		bool operator<(const TPCString& other) const
		{
			return Compare(other) == -1;
		}

		const TCString* operator->() const
		{
			if (m_pPooledString)
			{
				return &GetPtr()->m_oString;
			}
			return &ms_sEmpty;
		}

		const TCString& operator*() const
		{
			if (m_pPooledString)
			{
				return GetPtr()->m_oString;
			}
			return ms_sEmpty;
		}


	public:

		int Compare(const TPCString& other) const;

		inline TPooledCString* GetPtr() const { return m_pPooledString; }
		inline TCStringPool* GetStringPool() const { return m_pPooledString->m_pCStringPool; }
		inline TCString& GetVolatileCString() const { TASSERT(GetPtr() != TNULL); TASSERT(GetPtr()->m_pCStringPool == TNULL); return GetPtr()->m_oString; }
		inline const TCString& GetCString() const { return !m_pPooledString ? ms_sEmpty : m_pPooledString->m_oString; }
		inline TBOOL IsEmpty() const { return !m_pPooledString ? TTRUE : m_pPooledString->m_oString.IsEmpty(); }

		TPooledCString* m_pPooledString; // 0x0

		static inline const TCString ms_sEmpty = TCString();
	};
}


