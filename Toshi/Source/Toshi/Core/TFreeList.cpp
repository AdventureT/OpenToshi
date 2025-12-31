#include "ToshiPCH.h"
#include "TFreeList.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006f2b30
// $Barnyard: FUNCTION 006c1840
TFreeList::TFreeList(TUINT a_uiItemSize, TINT a_iInitialSize, TINT a_iGrowSize, const TCHAR* a_pchName)
{
	m_uiItemSize  = a_uiItemSize;
	m_iCapacity   = 0;
	m_Unk         = 0;
	m_pMemoryHeap = TNULL;
	TASSERT(m_iGrowSize >= 0);
	TASSERT(a_iInitialSize >= 0);
	SetGrowSize(a_iGrowSize);
	m_iCapacity  = a_iInitialSize;
	m_pPrevList  = ms_pLastList;
	ms_pLastList = this;
}

// $Barnyard: FUNCTION 006c18b0
TFreeList::Node* TFreeList::Allocate(TINT a_iNumber, TINT a_iSize)
{
	TASSERT(a_iNumber > 0);
	m_iCapacity += a_iNumber;

	Node* pNewNode = TREINTERPRETCAST(Node*, m_pMemoryHeap->Malloc(a_iNumber * a_iSize + sizeof(Node)));

	pNewNode->pNext  = m_RootNode.pNext;
	m_RootNode.pNext = pNewNode;

	auto  pData = pNewNode + 1;
	Node* pNext = TNULL;

	for (TINT i = a_iNumber - 1; i != 0; i--)
	{
		pData->pNext = pNext;
		pNext        = pData;

		pData = TREINTERPRETCAST(Node*, TREINTERPRETCAST(uintptr_t, pData) + a_iSize);
	}

	m_LastNode.pNext = pNext;
	return pData;
}

// $deBlob: FUNCTION 006f2d10
// $Barnyard: FUNCTION 006c1990
void TFreeList::SetCapacity(TINT a_iNewCapacity)
{
	if (m_iCapacity < a_iNewCapacity)
	{
		auto pNode = Allocate(a_iNewCapacity - m_iCapacity, m_uiItemSize);

		pNode->pNext     = m_LastNode.pNext;
		m_LastNode.pNext = pNode;
	}
}

// $deBlob: FUNCTION 006f2c80
// $Barnyard: FUNCTION 006c1950
void* TFreeList::New(TUINT a_uiSize)
{
	if (a_uiSize != m_uiItemSize)
	{
		return TMalloc(a_uiSize);
	}

	if (Node* pNode = m_LastNode.pNext)
	{
		m_LastNode.pNext = pNode->pNext;
		return pNode;
	}

	TASSERT(0 < m_iGrowSize, "Tried to grow TFreeList with 0 grow size\n");
	return Allocate(m_iGrowSize, a_uiSize);
}

// $deBlob: FUNCTION 006f2cf0
// $Barnyard: FUNCTION 006c1880
void TFreeList::Delete(void* a_Ptr)
{
	Node* pNode = TSTATICCAST(Node, a_Ptr);

	if (m_LastNode.pNext != TNULL)
	{
		pNode->pNext     = m_LastNode.pNext;
		m_LastNode.pNext = pNode;
	}
	else
	{
		m_LastNode.pNext = pNode;
		pNode->pNext     = TNULL;
	}
}

TOSHI_NAMESPACE_END
