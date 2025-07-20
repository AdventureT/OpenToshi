#pragma once

TOSHI_NAMESPACE_START

template <class T> class TArray
{
public:
	class Storage
	{
	public:
		friend class TArray;

	public:
		Storage(TINT a_iGrowSize = 10, TINT a_iSize = 0)
		{
			m_iGrowSize         = a_iGrowSize;
			m_iNumAllocElements = a_iSize;
			m_iNumElements      = 0;

			if (a_iSize > 0)
			{
				m_pData = TSTATICCAST(T, TMemalign(16, a_iSize * sizeof(T)));
			}
			else
			{
				TASSERT(m_iGrowSize != 0);
				m_pData = TNULL;
			}
		}

		~Storage()
		{
			if (m_pData)
			{
				TFree(m_pData);
			}
		}

		void Clear()
		{
			if (m_iNumAllocElements < 0)
			{
				if (m_pData)
				{
					TFree(m_pData);
					m_pData = TNULL;
				}

				m_iNumAllocElements = 0;
			}

			m_iNumElements = 0;
		}

		void Bind(TArray& a_oArray)
		{
			a_oArray.m_poArray = this;
			a_oArray.m_iIndex  = (m_iNumElements == 0) ? -1 : 0;
		}

		TINT GetNumElements() const { return m_iNumElements; }

		TINT GetNumAllocElements() const { return m_iNumAllocElements; }

		TINT GetGrowSize() const { return m_iGrowSize; }

		void SetGrowSize(TINT a_iGrowSize) { m_iGrowSize = a_iGrowSize; }

		void Push(const T& element)
		{
			GrowBy(1);
			(*this)[m_iNumElements++] = element;
		}

		T& Pop()
		{
			TASSERT(m_iNumElements >= 1);
			return m_pData[--m_iNumElements];
		}

		T& operator[](TINT a_iIndex)
		{
			TASSERT(a_iIndex >= 0);
			TASSERT(a_iIndex < m_iNumElements);
			return m_pData[a_iIndex];
		}

		const T& operator[](TINT a_iIndex) const
		{
			TASSERT(a_iIndex >= 0);
			TASSERT(a_iIndex < m_iNumElements);
			return m_pData[a_iIndex];
		}

	private:
		void GrowBy(TINT a_iGrowBy)
		{
			if (m_iNumAllocElements < m_iNumElements + a_iGrowBy)
			{
				TASSERT(m_iGrowSize != 0);

				auto iNewSize = TMath::Max(m_iNumAllocElements + m_iGrowSize, m_iNumElements + a_iGrowBy);
				Allocate(iNewSize);
			}
		}

		void Allocate(TINT a_iNewSize)
		{
			if (a_iNewSize != 0)
			{
				T*     pNewBuffer = TSTATICCAST(T, TMemalign(16, a_iNewSize * sizeof(T)));
				size_t uiCopySize = TMath::Min(m_iNumElements, a_iNewSize);

				for (size_t i = 0; i < uiCopySize; i++) pNewBuffer[i] = m_pData[i];

				m_iNumAllocElements = a_iNewSize;
				TASSERT(m_iNumElements <= m_iNumAllocElements);

				if (m_pData) TFree(m_pData);
				m_pData = pNewBuffer;
			}
			else
			{
				if (m_pData) TFree(m_pData);
				m_pData             = TNULL;
				m_iNumAllocElements = 0;
				m_iNumElements      = 0;
			}
		}

	private:
		TINT m_iGrowSize;         // 0x0
		TINT m_iNumElements;      // 0x4
		TINT m_iNumAllocElements; // 0x8
		T*   m_pData;             // 0xC
	};

public:
	TArray()
	{
		m_iIndex  = 0;
		m_poArray = TNULL;
	}

	TArray(const TArray& other)
	{
		m_poArray = other.m_poArray;
		m_iIndex  = 0;
	}

	TArray(Storage& a_poArray)
	{
		m_iIndex  = 0;
		m_poArray = &a_poArray;
	}

	TArray(TINT a_iIndex, Storage& a_poArray)
	{
		m_iIndex  = a_iIndex;
		m_poArray = &a_poArray;
	}

	void SetCurrentIndex(TINT a_iIndex) const
	{
		TASSERT(m_poArray);
		TASSERT(a_iIndex < m_poArray->GetNumElements());
		m_iIndex = a_iIndex;
	}

	TINT GetCurrentIndex() const { return m_iIndex; }

	TINT& IncrementSafe()
	{
		m_iIndex++;
		TASSERT(m_poArray);

		if (m_poArray->m_iNumElements <= m_iIndex || m_iIndex == 0) m_iIndex = -1;

		return m_iIndex;
	}

	void Push(const T& element)
	{
		TASSERT(m_poArray);
		m_poArray->Push(element);
	}

	T* operator->() const
	{
		TASSERT(m_iIndex >= 0);
		TASSERT(m_poArray);
		TASSERT(m_iIndex < m_poArray->m_iNumElements);
		return &m_poArray->m_pData[m_iIndex];
	}

	T& Get()
	{
		TASSERT(m_iIndex >= 0);
		TASSERT(m_poArray);
		TASSERT(m_iIndex < m_poArray->m_iNumElements);
		return m_poArray->m_pData[m_iIndex];
	}

	TBOOL IsOver() const { return (m_iIndex >= m_poArray->GetNumElements() || m_iIndex == -1); }

	const T& Get() const
	{
		TASSERT(m_iIndex >= 0);
		TASSERT(m_poArray);
		TASSERT(m_iIndex < m_poArray->m_iNumElements);
		return m_poArray->m_pData[m_iIndex];
	}

	TArray& operator++()
	{
		m_iIndex++;
		return *this;
	}

	TArray operator++(TINT)
	{
		TArray temp = *this;
		m_iIndex++;
		return temp;
	}

	TArray& operator--()
	{
		m_iIndex--;
		return *this;
	}

	TArray operator--(TINT)
	{
		TArray temp = *this;
		m_iIndex--;
		return temp;
	}

	TArray operator+(TINT a_iValue) const { return TArray(m_iIndex + a_iValue, m_poArray); }

	TArray operator-(TINT a_iValue) const { return TArray(m_iIndex - a_iValue, m_poArray); }

private:
	TINT     m_iIndex;  // 0x0
	Storage* m_poArray; // 0x4
};

TOSHI_NAMESPACE_END
