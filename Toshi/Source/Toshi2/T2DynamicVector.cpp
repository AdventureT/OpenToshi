#include "ToshiPCH.h"
#include "T2DynamicVector.h"

TOSHI_NAMESPACE_START

void T2GenericDynamicVector::Reallocate(TINT a_iNewSize, TINT a_iCount)
{
	if (a_iNewSize != m_iMaxSize)
	{
		TASSERT(m_pAllocator != TNULL, "Cannot reallocate a T2DynamicVector with no allocator specified!");

		void* elements = TNULL;

		if (a_iNewSize > 0)
		{
			elements = m_pAllocator->Malloc(a_iNewSize * a_iCount);
			TUtil::MemCopy(elements, m_poElements, m_iNumElements <= a_iNewSize ? m_iNumElements * a_iCount : a_iNewSize * a_iCount);
			m_pAllocator->Free(m_poElements);
			m_poElements = elements;
		}

		m_iMaxSize = a_iNewSize;
	}
}

void T2GenericDynamicVector::Grow(TINT a_iNumElements, TINT a_iCount)
{
	TINT newSize = m_iNumElements + a_iNumElements;
	TINT curSize = m_iMaxSize;

	if (curSize < newSize)
	{
		TASSERT(m_iGrowSize != 0);

		while (curSize < newSize)
		{
			if (m_iGrowSize == -1)
			{
				curSize = m_iMaxSize * 2 < 3 ? 2 : m_iMaxSize * 2;
			}
			else
			{
				curSize += m_iGrowSize;
			}
		}

		Reallocate(curSize, a_iCount);
	}
}


TOSHI_NAMESPACE_END
