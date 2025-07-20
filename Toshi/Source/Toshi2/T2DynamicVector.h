#pragma once

#include "T2Allocator.h"

TOSHI_NAMESPACE_START

class T2GenericDynamicVector
{
protected:
	T2GenericDynamicVector(T2Allocator* a_pAllocator, TINT a_iMaxSize, TINT a_iGrowSize, TINT a_iElementSize)
	{
		m_pAllocator   = a_pAllocator;
		m_iGrowSize    = a_iGrowSize;
		m_iNumElements = 0;
		m_iMaxSize     = a_iMaxSize;
		m_poElements   = a_iMaxSize > 0 ? a_pAllocator->Malloc(a_iMaxSize * a_iElementSize) : TNULL;
	}

	~T2GenericDynamicVector()
	{
		if (m_poElements) m_pAllocator->Free(m_poElements);
	}

	void Reallocate(TINT a_iNewSize, TINT a_iCount);
	void Grow(TINT a_iNumElements, TINT a_iCount);

	T2Allocator* m_pAllocator;   // 0x0
	TINT         m_iGrowSize;    // 0x4
	TINT         m_iNumElements; // 0x8
	TINT         m_iMaxSize;     // 0xC
	void*        m_poElements;   // 0x10
};

template <class T> class T2DynamicVector : public T2GenericDynamicVector
{
public:
	T2DynamicVector(T2Allocator* a_pAllocator, TINT a_iMaxSize, TINT a_iGrowSize)
	    : T2GenericDynamicVector(a_pAllocator, a_iMaxSize, a_iGrowSize, sizeof(T)) {}

	void Clear()
	{
		for (TINT i = 0; i < m_iNumElements; i++)
		{
			((T*)m_poElements)[m_iNumElements].~T();
		}

		m_iNumElements = 0;
	}

	void Reserve(TINT a_iSize)
	{
		if (a_iSize > m_iMaxSize)
		{
			Reallocate(a_iSize, sizeof(T));
		}
	}

	void PushBack(const T& element)
	{
		Grow(1, sizeof(T));

		T* elementArray                = (T*)m_poElements;
		elementArray[m_iNumElements++] = element;
	}

	T* Begin() const { return (T*)m_poElements; }

	T* End() const { return (T*)m_poElements + m_iNumElements; }

	TINT Size() const { return m_iNumElements; }
};

TOSHI_NAMESPACE_END
